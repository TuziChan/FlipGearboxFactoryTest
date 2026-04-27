# 串口线程安全修复方案

## 问题诊断

从日志分析出三个严重问题：

### 1. QTimer 跨线程错误
```
QObject::startTimer: Timers cannot be started from another thread
```
**原因**：`QSerialPort` 在主线程创建，但 `EncoderPoller` 等工作线程直接调用其方法，触发内部 QTimer。

### 2. Modbus 通信不稳定
```
Timeout waiting for bytes to be written
Timeout: incomplete response (received 6 bytes, expected 7)
```
**原因**：多个 Poller 线程并发访问同一串口，没有互斥保护，导致帧错乱。

### 3. 违反 Qt 线程模型
Qt 文档明确规定：**QObject 及其子类（包括 QSerialPort）不能跨线程直接调用方法**。

## 解决方案：Worker Thread Pattern

### 架构改进

```
之前（错误）:
┌─────────────┐
│ Main Thread │
│  QSerialPort├──┐
└─────────────┘  │
                 │ 跨线程调用（违规）
┌─────────────┐  │
│ Poller 1    ├──┤
└─────────────┘  │
┌─────────────┐  │
│ Poller 2    ├──┤
└─────────────┘  │
┌─────────────┐  │
│ Poller 3    ├──┘
└─────────────┘

现在（正确）:
┌─────────────┐     信号槽（线程安全）
│ Main Thread ├────────────────────┐
└─────────────┘                    │
                                   ▼
┌─────────────┐              ┌──────────────┐
│ Poller 1    ├─────────────►│ Serial Thread│
└─────────────┘   排队请求    │  QSerialPort │
┌─────────────┐              │  (独占访问)  │
│ Poller 2    ├─────────────►│              │
└─────────────┘              └──────────────┘
┌─────────────┐
│ Poller 3    ├─────────────►
└─────────────┘
```

### 核心改动

#### 1. 新增 `SerialWorker` 类
- 拥有 `QSerialPort` 对象
- 运行在专用线程
- 通过信号槽接收请求

#### 2. 重构 `ModbusRtuBusController`
- 创建 Worker 线程
- 使用 `QMetaObject::invokeMethod` + `QMutex` 实现同步调用
- 所有串口操作委托给 Worker

#### 3. 关键代码片段

**SerialWorker.h**
```cpp
class SerialWorker : public QObject {
    Q_OBJECT
public slots:
    void openPort(const QString& portName, int baudRate, ...);
    void sendRequest(const QByteArray& request, int requestId);
signals:
    void portOpened(bool success, const QString& error);
    void responseReceived(int requestId, bool success, ...);
private:
    QSerialPort* m_serialPort; // 在 Worker 线程中拥有
};
```

**ModbusRtuBusController.cpp**
```cpp
// 构造函数
m_worker->moveToThread(m_workerThread);
m_workerThread->start();

// 发送请求（主线程调用）
bool sendRequest(const QByteArray& request, QByteArray& response) {
    QMutexLocker locker(&m_requestMutex);
    
    // 异步调用 Worker
    QMetaObject::invokeMethod(m_worker, "sendRequest", 
                              Qt::QueuedConnection,
                              Q_ARG(QByteArray, request),
                              Q_ARG(int, requestId));
    
    // 等待响应（通过信号槽返回）
    m_requestCondition.wait(&m_requestMutex, timeout);
    
    response = m_pendingRequest.response;
    return m_pendingRequest.success;
}
```

## 优势

1. **线程安全**：QSerialPort 只在 Worker 线程访问
2. **自动序列化**：Qt 信号槽队列自动排队请求
3. **符合 Qt 规范**：遵循官方推荐的 Worker Thread 模式
4. **向后兼容**：`ModbusRtuBusController` 接口不变

## 测试建议

1. 运行程序，观察日志中 `QObject::startTimer` 警告是否消失
2. 检查 Modbus 通信是否稳定（无超时/不完整响应）
3. 验证多设备并发采集是否正常

## 参考文档

- Qt 官方文档：[QObject Thread Affinity](https://doc.qt.io/qt-6/threads-qobject.html)
- Qt 官方文档：[Thread-Support in Qt Modules](https://doc.qt.io/qt-6/threads-modules.html)
- Qt 示例：[Blocking Fortune Client](https://doc.qt.io/qt-6/qtnetwork-blockingfortuneclient-example.html)
