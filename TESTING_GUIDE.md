# 串口线程安全修复 - 测试指南

## 修复内容

已完成以下修改以解决 Qt 线程违规问题：

### 1. 新增文件
- `src/infrastructure/bus/SerialWorker.h` - 串口工作线程类
- `src/infrastructure/bus/SerialWorker.cpp` - 实现
- `test_serial_worker.cpp` - 独立测试程序

### 2. 修改文件
- `src/infrastructure/bus/ModbusRtuBusController.h` - 重构为线程安全版本
- `src/infrastructure/bus/ModbusRtuBusController.cpp` - 使用 Worker 模式
- `CMakeLists.txt` - 添加 SerialWorker 源文件

## 关键改进

### 信号槽参数类型
**重要**：Qt 的 `QMetaObject::invokeMethod` 和跨线程信号槽要求参数类型必须是**值类型**，不能是引用：

```cpp
// ❌ 错误 - 引用类型无法跨线程传递
void openPort(const QString& portName, ...);
signals:
    void portOpened(bool success, const QString& error);

// ✓ 正确 - 值类型可以安全复制
void openPort(QString portName, ...);
signals:
    void portOpened(bool success, QString error);
```

### 显式连接类型
跨线程信号槽必须使用 `Qt::QueuedConnection`：

```cpp
connect(m_worker, &SerialWorker::portOpened, 
        this, &ModbusRtuBusController::onPortOpened, 
        Qt::QueuedConnection);  // 必须显式指定
```

## 测试步骤

### 方法 1：运行主程序

1. 重新编译项目
2. 运行 `appFlipGearboxFactoryTest.exe`
3. 在设备配置页面启用编码器
4. 观察日志输出

**预期结果**：
```
[ModbusRtuBusController] Worker thread started: QThread(0x...)
[ModbusRtuBusController::open] Requesting to open port "COM23"
[SerialWorker::openPort] Called with port: "COM23" baudRate: 115200
[SerialWorker::openPort] Port opened successfully
[SerialWorker::openPort] Emitted portOpened signal
[ModbusRtuBusController::onPortOpened] Called with success: true
[ModbusRtuBusController::open] Port opened successfully
[Modbus TX] 01 03 00 00 00 01 84 0A
[Modbus RX] 01 03 02 77 48 9F 82
```

**不应出现**：
- ❌ `QObject::startTimer: Timers cannot be started from another thread`
- ❌ `Timeout waiting for port to open`
- ❌ `Timeout waiting for bytes to be written`

### 方法 2：运行独立测试

如果要单独测试串口功能：

1. 将 `test_serial_worker.cpp` 添加到 CMakeLists.txt：
```cmake
qt_add_executable(test_serial_worker test_serial_worker.cpp)
target_link_libraries(test_serial_worker PRIVATE FlipGearboxInfra Qt6::Core Qt6::SerialPort)
```

2. 编译并运行：
```bash
cd build/Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug
./test_serial_worker.exe
```

## 故障排查

### 问题 1：仍然超时
**症状**：`Timeout waiting for port to open`

**可能原因**：
1. Worker 线程未启动 - 检查 `m_workerThread->isRunning()` 日志
2. 信号槽未连接 - 检查是否有 `onPortOpened` 调用日志
3. COM 口被占用 - 关闭其他串口工具

**调试**：
```cpp
// 在 ModbusRtuBusController 构造函数中添加
qDebug() << "Worker thread running:" << m_workerThread->isRunning();
qDebug() << "Worker thread ID:" << m_workerThread->currentThreadId();
```

### 问题 2：信号未触发
**症状**：看到 `[SerialWorker::openPort] Emitted portOpened signal` 但没有 `onPortOpened` 日志

**可能原因**：信号槽连接失败

**调试**：
```cpp
// 检查连接是否成功
bool connected = connect(m_worker, &SerialWorker::portOpened, 
                        this, &ModbusRtuBusController::onPortOpened, 
                        Qt::QueuedConnection);
qDebug() << "Signal connection success:" << connected;
```

### 问题 3：编译错误
**症状**：`no matching function for call to 'QMetaObject::invokeMethod'`

**原因**：参数类型不匹配

**解决**：确保所有信号槽参数都是值类型（`QString` 而非 `const QString&`）

## 性能验证

修复后应观察到：

1. **稳定性提升**：
   - Modbus 超时次数显著减少
   - 不再出现不完整响应
   - 多设备并发采集稳定

2. **线程安全**：
   - 无 Qt 线程警告
   - 无数据竞争
   - 无随机崩溃

3. **性能影响**：
   - 轻微延迟增加（~1-2ms，因为信号槽排队）
   - CPU 使用率略降（无并发冲突）

## 回滚方案

如果修复导致问题，可以回滚到原始实现：

```bash
git checkout HEAD~1 -- src/infrastructure/bus/ModbusRtuBusController.*
git checkout HEAD~1 -- CMakeLists.txt
rm src/infrastructure/bus/SerialWorker.*
```

## 参考资料

- [Qt 线程基础](https://doc.qt.io/qt-6/thread-basics.html)
- [QObject 线程亲和性](https://doc.qt.io/qt-6/threads-qobject.html)
- [Qt 信号槽跨线程](https://doc.qt.io/qt-6/threads-qobject.html#signals-and-slots-across-threads)
