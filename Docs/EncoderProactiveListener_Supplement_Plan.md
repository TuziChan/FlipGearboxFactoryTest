# EncoderProactiveListener 补充方案

> 角色：头脑风暴（Brainstorm）  
> 任务：EncoderProactiveListener 补充方案（94a998f3-d258-46dc-8287-aa975aab91d3）  
> 分析范围：`EncoderProactiveListener.h/cpp`、`SingleTurnEncoderDevice.h/cpp`  
> 目标：解决当前主动监听器仅支持单圈模式、资源泄漏、协议解析不完整等 P0 级缺陷

---

## 执行摘要

当前 `EncoderProactiveListener` 仅支持 **单圈值（Mode 2）** 的主动上传解析，且存在 **析构未释放 listener** 的内存泄漏风险。本方案提出对 `EncoderProactiveListener` 进行**协议模式化重构**，使其完整支持编码器的三种主动上报模式（单圈值 / 虚拟多圈值 / 角速度），并修复 `SingleTurnEncoderDevice` 的资源生命周期管理缺陷。

**关键决策**：在硬件主动上报帧格式未完全确认前，建议**默认使用 Modbus 查询模式（Mode 0）**作为安全基线，主动模式仅在经过硬件验证的测试环境中启用。

| 问题 | 当前状态 | 修复方案 | 优先级 |
|------|---------|---------|--------|
| 仅支持 Mode 2 单圈解析 | 硬编码 2 字节 uint16 角度 | 引入 `ProtocolMode` 枚举，分模式解析帧 | P0 |
| 无虚拟多圈 / 角速度缓存 | `latestAngle()` 唯一出口 | 增加 `latestMultiTurn()` / `latestVelocity()` | P0 |
| `SingleTurnEncoderDevice` 析构泄漏 | `~SingleTurnEncoderDevice() = default` | 显式析构：`stop()` + `delete` | P0 |
| `readVirtualMultiTurn` / `readAngularVelocity` 未读缓存 | 仅 `readAngle` 检查 listener | 三方法均按当前 mode 分发到 listener 或 Modbus | P1 |
| 帧缓冲区 break 导致处理不完整 | `break` 中断 while 循环 | 移除 `break`，处理所有完整帧 | P1 |
| 高数据率下信号连接泄漏 | `start()` 重复 connect 无防护 | 增加 `m_running` 原子标志防护 | P2 |

---

## 1. 编码器主动上报模式协议分析

### 1.1 三种主动上报模式对比

根据 `Docs/单圈编码器.md` 寄存器定义 (`0x0006`)：

| 模式值 | 名称 | 数据含义 | 预期数据类型 | 推测帧长度 |
|--------|------|---------|-------------|-----------|
| `0x01` (Mode 2) | 自动回传单圈值 | 0 ~ resolution-1 | 无符号整数 | 2 bytes (uint16 BE) |
| `0x04` (Mode 3) | 自动回传虚拟多圈值 | 0 ~ 0xFFFFFF (累积计数) | 无符号整数 | 3~4 bytes (uint24/32 BE) |
| `0x05` (Mode 4) | 自动回传角速度值 | 有符号速度值 | 有符号整数 | 2 bytes (int16 BE) |

> ⚠️ **硬件帧格式待确认**：编码器手册中未明确给出主动上报模式的**裸数据帧格式**（无 Modbus ADU 封装时的字节序列）。上述帧长度基于寄存器位宽和 Modbus 寄存器映射推断。

### 1.2 当前实现与期望实现的差距

```
当前实现（缺陷）                           期望实现
┌─────────────────────────┐               ┌─────────────────────────┐
│ EncoderProactiveListener │               │ EncoderProactiveListener │
│                         │               │  - ProtocolMode mode    │
│  latestAngle()          │               │  - latestAngle()        │
│                         │               │  - latestMultiTurn()    │
│  onReadyRead():         │               │  - latestVelocity()     │
│    while (>=2) {        │               │                         │
│      parse uint16       │               │  onReadyRead():         │
│      m_buffer.remove(2) │               │    switch(mode) {       │
│      break;  ← 缺陷     │               │      case SingleTurn:   │
│    }                    │               │        parse 2 bytes    │
│                         │               │      case MultiTurn:    │
│  start(): connect       │               │        parse 3~4 bytes  │
│  stop(): disconnect     │               │      case Velocity:     │
│                         │               │        parse 2 bytes    │
└─────────────────────────┘               │    }                    │
                                          └─────────────────────────┘
```

---

## 2. 重构设计方案

### 2.1 EncoderProactiveListener 重构设计

#### 2.1.1 类定义重构

```cpp
#ifndef ENCODERPROACTIVELISTENER_H
#define ENCODERPROACTIVELISTENER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <atomic>

namespace Infrastructure {
namespace Devices {

/**
 * @brief Listener for encoder proactive upload modes
 *
 * Handles auto-report modes:
 * - SingleTurn (Mode 2): 2-byte uint16 big-endian angle count
 * - MultiTurn  (Mode 3): 3~4-byte uint32 big-endian accumulated count
 * - Velocity   (Mode 4): 2-byte int16 big-endian angular velocity
 */
class EncoderProactiveListener : public QObject {
    Q_OBJECT

public:
    enum class ProtocolMode {
        SingleTurn = 0,   // Mode 2 (0x01): auto-report single-turn value
        MultiTurn  = 1,   // Mode 3 (0x04): auto-report virtual multi-turn value
        Velocity   = 2    // Mode 4 (0x05): auto-report angular velocity
    };

    explicit EncoderProactiveListener(QSerialPort* serialPort,
                                       ProtocolMode mode,
                                       uint16_t resolution = 4096,
                                       QObject* parent = nullptr);
    ~EncoderProactiveListener() override;

    void start();
    void stop();

    // Data accessors (thread-safe, atomic)
    double latestAngle() const;       // Valid in SingleTurn mode
    double latestMultiTurn() const;   // Valid in MultiTurn mode (total degrees)
    double latestVelocity() const;    // Valid in Velocity mode (RPM)
    bool isValid() const;
    ProtocolMode mode() const;

signals:
    void dataReceived(double value);
    void errorOccurred(const QString& error);

private slots:
    void onReadyRead();

private:
    // Frame parsing per mode
    void parseSingleTurnFrame();
    void parseMultiTurnFrame();
    void parseVelocityFrame();

    QSerialPort* m_serialPort;
    ProtocolMode m_mode;
    QByteArray m_buffer;
    uint16_t m_resolution;

    std::atomic<bool> m_running{false};
    std::atomic<bool> m_valid{false};

    // Cached values (atomic for thread safety with Poller)
    std::atomic<double> m_latestAngle{0.0};
    std::atomic<double> m_latestMultiTurn{0.0};
    std::atomic<double> m_latestVelocity{0.0};
};

} // namespace Devices
} // namespace Infrastructure

#endif // ENCODERPROACTIVELISTENER_H
```

#### 2.1.2 实现要点

```cpp
// Constructor
EncoderProactiveListener::EncoderProactiveListener(QSerialPort* serialPort,
                                                    ProtocolMode mode,
                                                    uint16_t resolution,
                                                    QObject* parent)
    : QObject(parent)
    , m_serialPort(serialPort)
    , m_mode(mode)
    , m_resolution(resolution)
{
}

// Destructor: guaranteed cleanup
EncoderProactiveListener::~EncoderProactiveListener() {
    stop();
}

// Start with duplicate protection
void EncoderProactiveListener::start() {
    if (m_running.load()) {
        qWarning() << "EncoderProactiveListener already running, ignoring duplicate start()";
        return;
    }
    if (!m_serialPort || !m_serialPort->isOpen()) {
        emit errorOccurred("Serial port is not open");
        return;
    }

    m_buffer.clear();
    m_valid.store(false);
    m_running.store(true);

    connect(m_serialPort, &QSerialPort::readyRead,
            this, &EncoderProactiveListener::onReadyRead,
            Qt::UniqueConnection);  // Prevent duplicate connections

    qDebug() << "EncoderProactiveListener started, mode:" << static_cast<int>(m_mode)
             << "resolution:" << m_resolution;
}

// Stop: disconnect and reset state
void EncoderProactiveListener::stop() {
    if (!m_running.load()) {
        return;
    }
    m_running.store(false);

    if (m_serialPort) {
        disconnect(m_serialPort, &QSerialPort::readyRead,
                   this, &EncoderProactiveListener::onReadyRead);
    }
    m_buffer.clear();
    m_valid.store(false);
}

// Unified read handler: dispatch by mode
void EncoderProactiveListener::onReadyRead() {
    if (!m_serialPort || !m_running.load()) {
        return;
    }

    m_buffer.append(m_serialPort->readAll());

    switch (m_mode) {
        case ProtocolMode::SingleTurn:
            parseSingleTurnFrame();
            break;
        case ProtocolMode::MultiTurn:
            parseMultiTurnFrame();
            break;
        case ProtocolMode::Velocity:
            parseVelocityFrame();
            break;
    }

    // Prevent buffer overflow
    const int MAX_BUFFER_SIZE = 64;
    if (m_buffer.size() > MAX_BUFFER_SIZE) {
        qWarning() << "EncoderProactiveListener buffer overflow, dropping" << m_buffer.size() - MAX_BUFFER_SIZE / 2 << "bytes";
        m_buffer.remove(0, m_buffer.size() - MAX_BUFFER_SIZE / 2);
    }
}

// Mode 2: Single-turn value (2 bytes, uint16 BE)
void EncoderProactiveListener::parseSingleTurnFrame() {
    while (m_buffer.size() >= 2) {
        uint8_t highByte = static_cast<uint8_t>(m_buffer[0]);
        uint8_t lowByte  = static_cast<uint8_t>(m_buffer[1]);
        uint16_t rawAngle = (static_cast<uint16_t>(highByte) << 8) | lowByte;

        double angleDeg = (static_cast<double>(rawAngle) / static_cast<double>(m_resolution)) * 360.0;

        m_latestAngle.store(angleDeg);
        m_valid.store(true);
        emit dataReceived(angleDeg);

        m_buffer.remove(0, 2);
    }
}

// Mode 3: Virtual multi-turn value
// Note: Frame length assumption — see "硬件验证建议" section below
void EncoderProactiveListener::parseMultiTurnFrame() {
    // Assumption A: 3 bytes (24-bit) based on register max 0xFFFFFF
    // Assumption B: 4 bytes (32-bit, padded) if encoder sends full uint32
    // Default implementation uses 4 bytes for safety; adjust after HW test.
    const int FRAME_SIZE = 4;  // TODO: verify with hardware

    while (m_buffer.size() >= FRAME_SIZE) {
        uint32_t rawValue = 0;
        for (int i = 0; i < FRAME_SIZE; ++i) {
            rawValue = (rawValue << 8) | static_cast<uint8_t>(m_buffer[i]);
        }

        // If 4-byte frame and encoder only uses 24-bit, mask upper byte
        if (FRAME_SIZE == 4 && rawValue > 0xFFFFFF) {
            rawValue &= 0xFFFFFF;
        }

        // Convert accumulated count to total degrees
        double totalAngleDeg = (static_cast<double>(rawValue) / static_cast<double>(m_resolution)) * 360.0;

        m_latestMultiTurn.store(totalAngleDeg);
        m_valid.store(true);
        emit dataReceived(totalAngleDeg);

        m_buffer.remove(0, FRAME_SIZE);
    }
}

// Mode 4: Angular velocity (2 bytes, int16 BE)
void EncoderProactiveListener::parseVelocityFrame() {
    while (m_buffer.size() >= 2) {
        uint8_t highByte = static_cast<uint8_t>(m_buffer[0]);
        uint8_t lowByte  = static_cast<uint8_t>(m_buffer[1]);
        int16_t rawVelocity = static_cast<int16_t>((static_cast<uint16_t>(highByte) << 8) | lowByte);

        // Scaling: raw velocity -> RPM (device-specific; verify with manual)
        // Default: direct value as RPM. Adjust if manual specifies different scaling.
        double velocityRpm = static_cast<double>(rawVelocity);

        m_latestVelocity.store(velocityRpm);
        m_valid.store(true);
        emit dataReceived(velocityRpm);

        m_buffer.remove(0, 2);
    }
}
```

### 2.2 SingleTurnEncoderDevice 修复设计

#### 2.2.1 析构函数修复（P0 — 资源泄漏）

```cpp
// SingleTurnEncoderDevice.h
~SingleTurnEncoderDevice() override;  // Remove "= default"

// SingleTurnEncoderDevice.cpp
SingleTurnEncoderDevice::~SingleTurnEncoderDevice() {
    if (m_proactiveListener) {
        m_proactiveListener->stop();
        delete m_proactiveListener;
        m_proactiveListener = nullptr;
    }
}
```

> **注意**：`setAutoReportMode()` 中使用了 `deleteLater()`，这是 Qt 的延迟删除机制（在事件循环中执行）。析构函数中应直接使用 `delete` 以确保同步释放，避免 `StationRuntime` 重建时旧 listener 仍在事件队列中存活。

#### 2.2.2 initialize() 模式分发

```cpp
bool SingleTurnEncoderDevice::initialize() {
    if (!m_busController || !m_busController->isOpen()) {
        m_lastError = "Bus controller is not open";
        emit errorOccurred(m_lastError);
        return false;
    }

    if (m_communicationMode == 0) {
        // Mode 0: Query mode
        double angle;
        if (!readAngle(angle)) {
            m_lastError = QString("Failed to verify encoder communication: %1").arg(m_lastError);
            return false;
        }
        qDebug() << "Encoder initialized in query mode, slave ID:" << m_slaveId
                 << "Resolution:" << m_resolution << "Angle:" << angle << "deg";
    } else {
        // Modes 2, 3, 4: Auto-report modes
        uint16_t modeValue;
        EncoderProactiveListener::ProtocolMode listenerMode;
        QString modeName;

        switch (m_communicationMode) {
            case 2:
                modeValue = 0x01;
                listenerMode = EncoderProactiveListener::ProtocolMode::SingleTurn;
                modeName = "auto-report single-turn";
                break;
            case 3:
                modeValue = 0x04;
                listenerMode = EncoderProactiveListener::ProtocolMode::MultiTurn;
                modeName = "auto-report multi-turn";
                break;
            case 4:
                modeValue = 0x05;
                listenerMode = EncoderProactiveListener::ProtocolMode::Velocity;
                modeName = "auto-report velocity";
                break;
            default:
                m_lastError = QString("Invalid communication mode: %1").arg(m_communicationMode);
                return false;
        }

        // Write auto-report mode
        if (!writeRegister(REG_AUTO_REPORT_MODE, modeValue)) {
            m_lastError = QString("Failed to set encoder auto-report mode: %1").arg(m_lastError);
            return false;
        }

        // Write interval
        if (!writeRegister(REG_AUTO_REPORT_INTERVAL, static_cast<uint16_t>(m_autoReportIntervalMs))) {
            m_lastError = QString("Failed to set encoder auto-report interval: %1").arg(m_lastError);
            emit errorOccurred(m_lastError);
            return false;
        }

        QThread::msleep(200);

        QSerialPort* serialPort = m_busController->underlyingSerialPort();
        if (!serialPort) {
            m_lastError = "Bus controller does not provide underlying serial port access";
            return false;
        }

        // Create listener with correct protocol mode
        m_proactiveListener = new EncoderProactiveListener(serialPort, listenerMode, m_resolution, this);
        m_proactiveListener->start();

        qDebug() << "Encoder initialized in" << modeName << "mode, slave ID:" << m_slaveId
                 << "Resolution:" << m_resolution << "Interval:" << m_autoReportIntervalMs << "ms";
    }

    return true;
}
```

#### 2.2.3 读方法按模式分发

```cpp
bool SingleTurnEncoderDevice::readAngle(double& angleDeg) {
    // Only use listener cache in SingleTurn proactive mode (Mode 2)
    if (m_communicationMode == 2 && m_proactiveListener && m_proactiveListener->isValid()) {
        angleDeg = m_proactiveListener->latestAngle();
        return true;
    }

    // Fallback to Modbus polling for Mode 0, 3, 4
    QVector<uint16_t> values;
    if (!readRegisters(REG_ANGLE, 1, values)) {
        m_lastError = QString("Failed to read angle: %1").arg(m_lastError);
        return false;
    }
    angleDeg = (static_cast<double>(values[0]) / static_cast<double>(m_resolution)) * 360.0;
    return true;
}

bool SingleTurnEncoderDevice::readVirtualMultiTurn(double& totalAngleDeg) {
    // Only use listener cache in MultiTurn proactive mode (Mode 3)
    if (m_communicationMode == 3 && m_proactiveListener && m_proactiveListener->isValid()) {
        totalAngleDeg = m_proactiveListener->latestMultiTurn();
        return true;
    }

    // Fallback to Modbus polling
    QVector<uint16_t> values;
    if (!readRegisters(REG_VIRTUAL_MULTITURN, 2, values)) {
        m_lastError = QString("Failed to read virtual multi-turn: %1").arg(m_lastError);
        return false;
    }

    uint32_t combined = (static_cast<uint32_t>(values[0]) << 16) | static_cast<uint32_t>(values[1]);
    totalAngleDeg = (static_cast<double>(combined) / static_cast<double>(m_resolution)) * 360.0;
    return true;
}

bool SingleTurnEncoderDevice::readAngularVelocity(double& velocityRpm) {
    // Only use listener cache in Velocity proactive mode (Mode 4)
    if (m_communicationMode == 4 && m_proactiveListener && m_proactiveListener->isValid()) {
        velocityRpm = m_proactiveListener->latestVelocity();
        return true;
    }

    // Fallback to Modbus polling
    QVector<uint16_t> values;
    if (!readRegisters(REG_ANGULAR_VELOCITY, 1, values)) {
        m_lastError = QString("Failed to read angular velocity: %1").arg(m_lastError);
        return false;
    }

    int16_t rawVelocity = static_cast<int16_t>(values[0]);
    velocityRpm = static_cast<double>(rawVelocity);
    return true;
}
```

### 2.3 setAutoReportMode() 同步修正

```cpp
bool SingleTurnEncoderDevice::setAutoReportMode(uint16_t mode, int intervalMs) {
    // Validate mode
    if (mode != 0x00 && mode != 0x01 && mode != 0x04 && mode != 0x05) {
        m_lastError = QString("Invalid auto-report mode: %1").arg(mode);
        return false;
    }

    // Write mode register
    if (!writeRegister(REG_AUTO_REPORT_MODE, mode)) {
        m_lastError = QString("Failed to set auto-report mode: %1").arg(m_lastError);
        return false;
    }

    // Write interval register
    if (!writeRegister(REG_AUTO_REPORT_INTERVAL, static_cast<uint16_t>(intervalMs))) {
        m_lastError = QString("Failed to set auto-report interval: %1").arg(m_lastError);
        return false;
    }

    m_autoReportIntervalMs = intervalMs;

    // Synchronize listener lifecycle
    if (mode == 0x00) {
        // Query mode: stop and delete listener
        if (m_proactiveListener) {
            m_proactiveListener->stop();
            delete m_proactiveListener;  // Use direct delete for immediate cleanup
            m_proactiveListener = nullptr;
        }
        m_communicationMode = 0;
        qDebug() << "Encoder switched to query mode";
    } else {
        // Map register value to communicationMode
        int newCommMode = 0;
        EncoderProactiveListener::ProtocolMode listenerMode;
        if (mode == 0x01) {
            newCommMode = 2;
            listenerMode = EncoderProactiveListener::ProtocolMode::SingleTurn;
        } else if (mode == 0x04) {
            newCommMode = 3;
            listenerMode = EncoderProactiveListener::ProtocolMode::MultiTurn;
        } else if (mode == 0x05) {
            newCommMode = 4;
            listenerMode = EncoderProactiveListener::ProtocolMode::Velocity;
        }

        m_communicationMode = newCommMode;

        // Restart listener with new mode if needed
        if (m_proactiveListener) {
            if (m_proactiveListener->mode() == listenerMode) {
                // Same mode, just ensure running
                m_proactiveListener->start();
            } else {
                // Mode changed: recreate listener
                m_proactiveListener->stop();
                delete m_proactiveListener;
                m_proactiveListener = nullptr;
            }
        }

        if (!m_proactiveListener) {
            QSerialPort* serialPort = m_busController->underlyingSerialPort();
            if (!serialPort) {
                m_lastError = "Bus controller does not provide underlying serial port access";
                return false;
            }
            m_proactiveListener = new EncoderProactiveListener(serialPort, listenerMode, m_resolution, this);
        }
        m_proactiveListener->start();

        qDebug() << "Encoder auto-report mode set to" << mode
                 << "(communicationMode=" << m_communicationMode << ")"
                 << "interval=" << intervalMs << "ms";
    }

    return true;
}
```

---

## 3. 关键设计决策与理由

### 3.1 为什么引入 `ProtocolMode` 枚举而不是用 `communicationMode` 直接映射？

| 方案 | 优点 | 缺点 | 决策 |
|------|------|------|------|
| A. 直接使用 `communicationMode` (2/3/4) | 无额外映射层 | `SingleTurnEncoderDevice` 的 mode 值（2/3/4）与寄存器值（0x01/0x04/0x05）不一致，直接在 listener 中使用会导致混淆 | ❌ 拒绝 |
| B. 引入 `ProtocolMode` 枚举 | 语义清晰（SingleTurn/MultiTurn/Velocity），与寄存器值解耦 | 增加一个映射层 | ✅ 采纳 |

### 3.2 为什么 `parseMultiTurnFrame` 默认假设 4 字节帧？

编码器手册中 `0x0000`（单圈值）和 `0x0000-0x0001`（虚拟多圈）的取值范围为 `0 ~ 0xFFFFFF`（24-bit）。但大多数 Modbus 设备在主动上传时仍按 **寄存器对齐** 发送数据（即 16-bit 倍数）。因此：
- **保守假设**：4 字节（32-bit，高 8 位补 0）。
- **验证后调整**：若硬件实际发送 3 字节，将 `FRAME_SIZE` 改为 3 并移除掩码逻辑。

### 3.3 为什么 `readAngle` 仅在 `communicationMode == 2` 时使用 listener 缓存？

当前代码的 `readAngle()` 在**任何** proactive 模式下都返回 `latestAngle()`，这会导致：
- Mode 3（多圈模式）下，`readAngle()` 仍返回 listener 缓存，但 listener 缓存的是多圈原始计数而非单圈角度，数据语义错误。
- Mode 4（速度模式）下，`readAngle()` 返回的是速度值而非角度，完全错误。

**修复策略**：严格按 mode 分发，每个读方法只在对应 proactive 模式下使用缓存，其他模式回退到 Modbus 轮询。

### 3.4 为什么析构函数用 `delete` 而非 `deleteLater()`？

`setAutoReportMode()` 使用 `deleteLater()` 是合理的（它在事件循环中调用，需要确保槽函数执行完毕后再删除）。但析构函数发生在对象生命周期的终点，必须**同步释放**资源。若使用 `deleteLater()`，在 `StationRuntime` 重建期间（`unique_ptr::reset()` → `~SingleTurnEncoderDevice()`），旧的 listener 可能仍在事件队列中处理 `readyRead` 信号，导致崩溃。

---

## 4. 单元测试设计

建议新增 `tests/devices/EncoderProactiveListenerTests.cpp`：

| 测试用例 | 目的 | 验证点 |
|---------|------|--------|
| `testSingleTurnParsing` | 验证 Mode 2 帧解析 | 2-byte BE uint16 → 正确角度 |
| `testMultiTurnParsing` | 验证 Mode 3 帧解析 | 4-byte BE uint32 → 正确总角度 |
| `testVelocityParsing` | 验证 Mode 4 帧解析 | 2-byte BE int16 → 正确速度 |
| `testBufferAccumulation` | 验证分片帧处理 | 分两次写入 1+1 bytes，应正确拼成一帧 |
| `testBufferOverflow` | 验证缓冲区保护 | 写入 100 bytes，缓冲区不超过上限 |
| `testDestructorCleanup` | 验证资源释放 | listener delete 后，serialPort 无 dangling signal |
| `testDuplicateStartProtection` | 验证重复 start 防护 | 两次 start() 不导致重复 connect |
| `testModeSwitchRecreate` | 验证模式切换 | SingleTurn → Velocity 切换后，listener 重建为新 mode |

---

## 5. 硬件验证建议

在代码合入前，必须通过真实硬件验证以下假设：

| 验证项 | 方法 | 期望结果 |
|--------|------|---------|
| Mode 2 帧长度 | 串口抓包工具监听 RS485 | 确认单圈模式发送 **2 字节** |
| Mode 3 帧长度 | 串口抓包工具监听 RS485 | 确认多圈模式发送 **3 或 4 字节** |
| Mode 4 帧格式 | 串口抓包工具监听 RS485 | 确认速度模式发送 **2 字节有符号值** |
| 多圈值范围 | 手动旋转编码器多圈后读取 | 确认累积计数递增且不回绕 |
| 速度值方向 | 正转/反转分别测试 | 确认正转为正、反转为负 |
| 速度值缩放 | 与已知转速对比 | 确认 raw value 是否直接等于 RPM，或有固定缩放因子 |

> 若硬件验证发现帧格式与假设不符（如多圈模式实际发送 3 字节），只需修改 `parseMultiTurnFrame()` 中的 `FRAME_SIZE` 常量，其他代码无需变更。

---

## 6. 实施计划

### 第一波：止血（必须立即完成）

| 序号 | 任务 | 文件 | 影响 | 预估工作量 |
|------|------|------|------|-----------|
| 1 | 修复 `SingleTurnEncoderDevice` 析构函数 | `.h`, `.cpp` | 消除内存泄漏 + 悬空信号 | 10 min |
| 2 | 为 `EncoderProactiveListener` 增加 `m_running` 原子标志和重复 start 防护 | `.h`, `.cpp` | 消除重复 connect 风险 | 15 min |
| 3 | 严格化 `readAngle` / `readVirtualMultiTurn` / `readAngularVelocity` 的 mode 检查 | `SingleTurnEncoderDevice.cpp` | 防止跨模式数据语义错误 | 20 min |
| 4 | 编译验证 + 现有测试回归 | CMake/build | 确保无编译错误 | 30 min |

### 第二波：协议模式重构（本周完成）

| 序号 | 任务 | 文件 | 影响 | 预估工作量 |
|------|------|------|------|-----------|
| 5 | 引入 `ProtocolMode` 枚举并重构 `EncoderProactiveListener` | `.h`, `.cpp` | 完整支持三模式 | 2 h |
| 6 | 更新 `SingleTurnEncoderDevice::initialize()` 和 `setAutoReportMode()` 传递 mode | `SingleTurnEncoderDevice.cpp` | 与 listener 新接口对接 | 30 min |
| 7 | 编写 `EncoderProactiveListenerTests` | `tests/devices/` | 回归验证三模式解析 | 2 h |
| 8 | 硬件验证（Mode 2/3/4 帧格式） | 测试台 | 确认帧长度和字节序 | 1 h |

### 第三波：配置化与优化（后续迭代）

| 序号 | 任务 | 说明 |
|------|------|------|
| 9 | `encoderAutoReportIntervalMs` 配置化 | 从 `StationRuntimeFactory` 硬编码 `20` 改为从 `StationConfig` 读取 |
| 10 | 主动模式性能基准测试 | 对比 Mode 0 (Modbus 轮询) 与 Mode 2/3/4 (主动上传) 的延迟和 CPU 占用 |
| 11 | 与 DYN200 ProactiveListener 统一抽象 | 评估提取 `IProactiveListener` 基类，统一 listener 生命周期管理 |

---

## 7. 风险与缓解

| 风险 | 可能性 | 影响 | 缓解措施 |
|------|--------|------|---------|
| 多圈模式帧长度假设错误（3 vs 4 bytes） | 中 | 数据解析错位 | 代码中使用 `FRAME_SIZE` 常量集中定义，硬件验证后立即调整；默认 4 字节更安全（3 字节帧在 4 字节解析下会累积错位，但不会崩溃） |
| 速度值缩放因子与假设不符 | 中 | 速度读数数值错误 | `parseVelocityFrame()` 中缩放因子提取为常量 `VELOCITY_SCALE`，硬件验证后调整 |
| `underlyingSerialPort()` 返回 nullptr | 低 | listener 创建失败 | 已在代码中检查并返回错误，但需确保 `ModbusRtuBusController` 始终实现该方法 |
| 主动模式下 Modbus 命令与主动数据冲突 | 中 | 帧解析混乱 | 当前 `ModbusRtuBusController::sendRequest()` 与 listener 共享 `QSerialPort`，已在 `Dyn200` 中验证可行性；建议添加串口访问互斥锁 |

---

## 8. 附录：修改文件清单

| 文件 | 修改类型 | 修改内容 |
|------|---------|---------|
| `src/infrastructure/devices/EncoderProactiveListener.h` | 重构 | 增加 `ProtocolMode` 枚举、`latestMultiTurn()`、`latestVelocity()`、`m_running` 标志 |
| `src/infrastructure/devices/EncoderProactiveListener.cpp` | 重构 | 分模式解析帧、重复 start 防护、缓冲区溢出保护 |
| `src/infrastructure/devices/SingleTurnEncoderDevice.h` | 修复 | 析构函数声明改为非默认 |
| `src/infrastructure/devices/SingleTurnEncoderDevice.cpp` | 重构+修复 | 显式析构、`initialize` 和 `setAutoReportMode` 传递 `ProtocolMode`、三读方法严格 mode 检查 |
| `tests/devices/EncoderProactiveListenerTests.cpp` | 新增 | 8 个单元测试覆盖三模式解析和资源生命周期 |

---

*方案版本：v1.0*  
*基于代码库 HEAD (88e97bd)*  
*硬件验证项需在代码合入前完成，否则建议默认禁用主动模式（强制 Mode 0）*
