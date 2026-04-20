# 编码器主动监听集成修复

## 问题描述
EncoderProactiveListener 类已创建但未集成到 SingleTurnEncoderDevice 中，导致主动模式数据冲突问题未解决。

## 修复内容

### 1. SingleTurnEncoderDevice.h
- 添加 `#include "EncoderProactiveListener.h"`
- 添加成员变量 `EncoderProactiveListener* m_proactiveListener;`

### 2. SingleTurnEncoderDevice.cpp

#### 构造函数
- 初始化 `m_proactiveListener(nullptr)`

#### initialize() 方法
在主动模式（mode 2/3/4）下：
```cpp
// 获取底层串口
QSerialPort* serialPort = m_busController->underlyingSerialPort();
if (!serialPort) {
    m_lastError = "Bus controller does not provide underlying serial port access";
    return false;
}

// 创建并启动 listener
m_proactiveListener = new EncoderProactiveListener(serialPort, this);
m_proactiveListener->start();
```

#### readAngle() 方法
优先从 listener 获取数据：
```cpp
// 主动模式：从 listener 获取
if (m_proactiveListener && m_proactiveListener->isValid()) {
    angleDeg = m_proactiveListener->latestAngle();
    return true;
}

// 查询模式：使用 Modbus 轮询
QVector<uint16_t> values;
if (!readRegisters(REG_ANGLE, 1, values)) {
    // ...
}
```

#### setAutoReportMode() 方法
支持运行时切换模式：
```cpp
if (mode == 0x00) {
    // 切换到查询模式 - 停止 listener
    if (m_proactiveListener) {
        m_proactiveListener->stop();
        m_proactiveListener->deleteLater();
        m_proactiveListener = nullptr;
    }
} else {
    // 切换到主动模式 - 启动 listener
    if (!m_proactiveListener) {
        QSerialPort* serialPort = m_busController->underlyingSerialPort();
        // ...
        m_proactiveListener = new EncoderProactiveListener(serialPort, this);
        m_proactiveListener->start();
    }
}
```

### 3. CMakeLists.txt
在以下目标中添加 EncoderProactiveListener 源文件：
- appFlipGearboxFactoryTest
- SimulationRuntimeTests
- GearboxSimulationIntegrationTests

## 工作原理

### 查询模式（Mode 0）
- `m_proactiveListener` 为 nullptr
- `readAngle()` 使用 Modbus 轮询读取寄存器
- 无数据冲突

### 主动模式（Mode 2/3/4）
- `m_proactiveListener` 监听串口主动上报数据
- `readAngle()` 从 listener 的缓存获取最新数据
- 避免了 Modbus 轮询与主动上报的冲突

### 模式切换
- `setAutoReportMode()` 支持运行时切换
- 切换到查询模式时停止并销毁 listener
- 切换到主动模式时创建并启动 listener

## 依赖关系
- 依赖 `IBusController::underlyingSerialPort()` 方法（已在 P0 修复中添加）
- listener 作为 SingleTurnEncoderDevice 的子对象，生命周期自动管理

## 测试建议
1. 测试查询模式（mode 0）下的正常读取
2. 测试主动模式（mode 2）下从 listener 获取数据
3. 测试运行时模式切换
4. 测试 listener 无效时的降级处理
