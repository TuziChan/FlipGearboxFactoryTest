# 多设备集成架构一致性分析报告

> 角色：头脑风暴（Brainstorm）  
> 任务：分析多设备集成架构一致性（ea434ed3-a1cc-450a-9f4b-5fb779ef0853）  
> 分析范围：DYN200扭矩传感器、AQMD电机驱动、制动电源、单圈编码器  
> 参考文档：Docs/DYN200_Integration_Architecture_Analysis.md

---

## 执行摘要

项目中的四个设备（AQMD、DYN200、Encoder、Brake）已**全部集成**到统一的分层架构中（UI → ViewModel → 领域引擎 → 设备抽象 → 总线协议），且均支持**模拟/真实双模式运行时切换**。整体架构设计遵循了依赖倒置原则和工厂模式，但在**配置结构体泛化污染**、**工厂创建设备签名不一致**、**主动模式实现成熟度差异**、**资源生命周期管理**四个方面存在显著的跨设备不一致风险。

| 维度 | 当前状态 | 风险等级 |
|------|---------|---------|
| 配置加载路径统一性 | 基本统一，但 DeviceConfig 被 encoderResolution 污染 | 🟡 中 |
| 工厂创建设备逻辑一致性 | 模式一致，但构造函数签名差异大，工厂硬编码设备专属参数 | 🟡 中 |
| 接口抽象统一性 | 风格一致（均继承 QObject + initialize + lastError），但功能粒度差异大 | 🟢 低 |
| 模拟/真实模式切换一致性 | 完全统一，四设备均通过 RuntimeManager + StationRuntimeFactory 切换 | 🟢 低 |
| 主动上传模式实现成熟度 | DYN200 成熟；Encoder 存在内存泄漏、协议解析不完整、运行时不一致 | 🔴 高 |
| 资源生命周期管理 | DYN200 正确释放 proactive listener；Encoder 泄漏 + 悬空信号风险 | 🔴 高 |
| 配置校验覆盖度 | StationConfigValidator 未覆盖 communicationMode / pollIntervalUs / autoReportIntervalMs | 🟡 中 |

---

## 1. 架构概览与模块拓扑

### 1.1 四设备集成拓扑图

```
┌─────────────────────────────────────────────────────────────────────────────┐
│  UI/QML Layer (QML)                                                         │
│  ├── DeviceConfigPage.qml ──设备配置表单（四设备卡片）                        │
│  └── DiagnosticsPage.qml ──遥测卡片（motor/torque/encoder/brake）            │
├─────────────────────────────────────────────────────────────────────────────┤
│  ViewModel Layer (C++)                                                      │
│  ├── DiagnosticsViewModel ──build*Status() / *TelemetryChanged              │
│  └── TestExecutionViewModel ──Q_PROPERTY 暴露遥测数据                        │
├─────────────────────────────────────────────────────────────────────────────┤
│  Domain Layer (C++)                                                         │
│  ├── GearboxTestEngine ──evaluate*Result() / acquireTelemetry()             │
│  └── TelemetrySnapshot ──motorCurrentA / dynTorqueNm / encoderAngleDeg ...   │
├─────────────────────────────────────────────────────────────────────────────┤
│  Infrastructure Layer (C++)                                                 │
│  ├── Devices                                                                │
│  │   ├── ITorqueSensorDevice / IMotorDriveDevice / IEncoderDevice / IBrakePowerDevice│
│  │   ├── Dyn200TorqueSensorDevice ──Modbus RTU + 3种主动上传                 │
│  │   ├── AqmdMotorDriveDevice ──Modbus RTU 轮询                             │
│  │   ├── SingleTurnEncoderDevice ──Modbus RTU + 3种自动上报                  │
│  │   ├── BrakePowerSupplyDevice ──Modbus RTU 轮询 (+ 重试机制)               │
│  │   ├── Dyn200ProactiveListener ──Hex6/Hex8/ASCII 多协议解析                │
│  │   └── EncoderProactiveListener ──单协议固定帧解析 (4 bytes)               │
│  ├── Bus                                                                    │
│  │   ├── IBusController / ModbusRtuBusController / SimulatedBusController   │
│  ├── Acquisition                                                            │
│  │   └── AcquisitionScheduler ──MotorPoller/TorquePoller/EncoderPoller/BrakePoller│
│  ├── Config                                                                 │
│  │   ├── StationConfig ──aqmdConfig/dyn200Config/encoderConfig/brakeConfig  │
│  │   ├── DeviceConfigService ──QML 直接读写 station.json                    │
│  │   ├── ConfigLoader ──JSON 反序列化到 StationConfig                       │
│  │   ├── RuntimeManager ──switchMode() 切换模拟/真实模式                    │
│  │   ├── StationRuntimeFactory ──根据配置创建运行时实例（四设备统一入口）      │
│  │   └── StationConfigValidator ──配置校验                                  │
│  └── Simulation                                                             │
│      ├── SimulatedTorqueDevice / SimulatedMotorDevice                       │
│      ├── SimulatedEncoderDevice / SimulatedBrakeDevice                      │
│      └── SimulationContext ──跨设备状态共享                                 │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 1.2 配置数据流（四设备对比）

| 设备 | JSON 键 | DeviceConfig 字段 | 专属额外字段 | C++ 创建参数 |
|------|---------|-------------------|-------------|-------------|
| AQMD | `aqmd` | portName, slaveId, baudRate, timeout, parity, stopBits, enabled, pollIntervalUs, communicationMode | 无 | bus, slaveId, runtime |
| DYN200 | `dyn200` | 同上 | 无 | bus, slaveId, **communicationMode**, runtime |
| Encoder | `encoder` | 同上 + **encoderResolution** | resolution | bus, slaveId, **resolution**, **communicationMode**, **autoReportIntervalMs(20)**, runtime |
| Brake | `brake` | 同上 | 无 (channel 存于 root) | bus, slaveId, runtime |

**关键发现**：`DeviceConfig` 本应是设备无关的通用结构体，但被 `encoderResolution` 污染，违反了通用性原则。`brakeChannel` 虽不在 `DeviceConfig` 中，但在保存时被混入 `brake` JSON 对象并回写 root，逻辑上更混乱。

---

## 2. 配置加载路径一致性分析

### 2.1 加载路径对比矩阵

| 路径 | AQMD | DYN200 | Encoder | Brake | 一致性评级 |
|------|------|--------|---------|-------|-----------|
| `ConfigLoader::loadStationConfig` | ✅ 加载全部通用字段 | ✅ 同上 | ✅ 加载全部 + `resolution` | ✅ 加载全部通用字段 + `channel` | 高 ✅ |
| `DeviceConfigService::loadDeviceConfig` | ✅ QVariantMap 读取 | ✅ 同上 | ✅ 同上 + `resolution` | ✅ 同上 + `channel` | 高 ✅ |
| `DeviceConfigService::saveDeviceConfig` | ✅ QVariantMap 写入 | ✅ 同上 | ✅ 同上 + `resolution` | ✅ 同上 + `channel` + `brakeChannel` root | 中 🟡 |
| 默认值来源 | `StationConfig()` 构造函数 | `StationConfig()` 构造函数 | `StationConfig()` 构造函数 | `StationConfig()` 构造函数 | 高 ✅ |

### 2.2 具体不一致点

**[IC-1] `encoderResolution` 污染通用结构体**
- **位置**：`StationConfig.h:24`
- **问题**：`DeviceConfig` 是四设备通用的结构体，但 `encoderResolution` 只有编码器使用。这导致 AQMD/DYN200/Brake 的 `DeviceConfig` 实例也携带无意义的 `encoderResolution` 字段。
- **影响**：概念污染、未来新增设备时字段膨胀、JSON 中其他设备也可能意外携带 resolution。
- **建议**：将 `encoderResolution` 从 `DeviceConfig` 移除，改为 `StationConfig` 的顶层字段 `encoderResolution`，或创建 `EncoderConfig : DeviceConfig` 子结构体。

**[IC-2] `brakeChannel` 存储位置不一致**
- **位置**：`ConfigLoader.cpp:92` / `DeviceConfigService.cpp:122`
- **问题**：
  - 加载时：`brakeChannel` 从 JSON root 读取（`json["brakeChannel"]`）。
  - 保存时：`brakeChannel` 被同时写入 `brake` 对象的 `channel` 字段 **和** JSON root 的 `brakeChannel` 字段。
- **影响**：QML 编辑 brake channel 时，数据来源（root vs brake.channel）取决于 UI 实现，容易不一致。
- **建议**：统一只存于 root 的 `brakeChannel`，或完全内嵌于 `brake` 对象并移除 root 字段。

**[IC-3] `DeviceConfigService` 对 `pollIntervalUs` / `communicationMode` 的保存为可选字段**
- **位置**：`DeviceConfigService.cpp:117-118`
- **问题**：`saveDeviceConfig` 使用 `if (map.contains("pollIntervalUs"))` 和 `if (map.contains("communicationMode"))` 条件写入。如果 QML 未传递这些键，它们会从 JSON 中消失，下次加载回退到默认值。
- **影响**：DYN200/Encoder 的主动模式配置可能在保存后丢失。
- **建议**：无条件写入所有标准字段，或确保 QML 始终传递完整 Map。

---

## 3. StationRuntimeFactory 创建设备逻辑一致性

### 3.1 工厂代码片段对比

```cpp
// AQMD（最简单）
runtime->m_motor = std::make_unique<Devices::AqmdMotorDriveDevice>(
    runtime->m_aqmdBus.get(), config.aqmdConfig.slaveId, runtime.get()
);

// DYN200（增加 communicationMode）
runtime->m_torque = std::make_unique<Devices::Dyn200TorqueSensorDevice>(
    runtime->m_dyn200Bus.get(), config.dyn200Config.slaveId,
    config.dyn200Config.communicationMode, runtime.get()
);

// Encoder（增加 resolution + communicationMode + hardcoded interval）
runtime->m_encoder = std::make_unique<Devices::SingleTurnEncoderDevice>(
    runtime->m_encoderBus.get(), config.encoderConfig.slaveId,
    config.encoderConfig.encoderResolution,
    config.encoderConfig.communicationMode,
    20, // Default auto-report interval: 20ms ← 硬编码！
    runtime.get()
);

// Brake（最简单，同 AQMD）
runtime->m_brake = std::make_unique<Devices::BrakePowerSupplyDevice>(
    runtime->m_brakeBus.get(), config.brakeConfig.slaveId, runtime.get()
);
```

### 3.2 工厂不一致风险清单

| 编号 | 问题描述 | 涉及设备 | 风险等级 |
|------|---------|---------|---------|
| IF-1 | **构造函数签名数量不一致**：AQMD/Brake 3 个参数；DYN200 4 个；Encoder 6 个。工厂必须硬编码设备专属知识。 | 全部 | 🟡 中 |
| IF-2 | **`communicationMode` 参数化不一致**：DYN200 和 Encoder 接收该参数；AQMD 和 Brake 不支持任何模式切换。如果未来 AQMD 也增加主动模式，工厂签名需再次修改。 | DYN200, Encoder | 🟡 中 |
| IF-3 | **Encoder 自动上报间隔硬编码**：`20` 直接写死在 `StationRuntimeFactory.cpp:97`，未从 `config.encoderConfig` 读取。用户无法通过配置调整上报频率。 | Encoder | 🔴 高 |
| IF-4 | **`runtime.get()` 作为 parent QObject 传递**：所有设备都接收 `runtime.get()` 作为 parent，但如果 `StationRuntimeFactory::create` 返回的 `unique_ptr` 被移动，parent 指针可能悬空（虽然 Qt 对象树会在 parent delete 时析构子对象，但 unique_ptr 也拥有所有权，存在双重释放风险）。 | 全部 | 🟡 中 |
| IF-5 | **模拟设备构造签名不一致**：模拟设备统一为 `(simContext, runtime)`，而真实设备为 `(bus, slaveId, ...)`。这导致工厂中模拟分支和真实分支完全分裂，无法通过统一接口创建。 | 全部 | 🟡 中 |

---

## 4. 设备接口抽象统一性分析

### 4.1 接口契约对比

| 维度 | ITorqueSensorDevice | IMotorDriveDevice | IEncoderDevice | IBrakePowerDevice |
|------|---------------------|-------------------|----------------|-------------------|
| 基类 | QObject | QObject | QObject | QObject |
| `initialize()` | ✅ | ✅ | ✅ | ✅ |
| `lastError()` | ✅ | ✅ | ✅ | ✅ |
| `errorOccurred` 信号 | ✅ | ✅ | ✅ | ✅ |
| 纯虚读方法数 | 4 (torque, speed, power, readAll) | 2 (current, AI1Level) | 4 (angle, multiTurn, velocity, zeroPoint) | 7 (setCurrent, readCurrent, setVoltage, readVoltage, readPower, readMode, setBrakeMode) |
| 纯虚写/控制方法数 | 0 | 3 (setMotor, brake, coast) | 1 (setZeroPoint) | 4 (setOutputEnable, setCurrent, setVoltage, setBrakeMode) |
| 专属枚举/类型 | 无 | `Direction` | 无 | 无 |
| 跨通道/多轴支持 | 无 | 无 | 无 | ✅ `int channel` 参数 |

### 4.2 接口抽象一致性问题

**[II-1] 主动模式状态未在接口层抽象**
- **问题**：`ITorqueSensorDevice` 和 `IEncoderDevice` 均没有 `isProactiveMode()` 或 `setCommunicationMode()` 的接口方法。主动模式完全是实现层的私有能力，但 `AcquisitionScheduler` 的 `TorquePoller`/`EncoderPoller` 却需要感知数据刷新来源（Modbus 轮询 vs 主动上传缓存）。
- **影响**：Poller 在主动模式下仍按固定频率调用 `readAll()` / `readAngle()`，虽然设备内部会返回缓存值，但这造成了不必要的线程唤醒和潜在竞争。
- **建议**：在接口层增加 `bool isUsingProactiveMode() const` 或类似查询方法，使调度层可以优化采集策略。

**[II-2] `IBrakePowerDevice` 的多通道设计与单通道假设冲突**
- **问题**：`IBrakePowerDevice` 接口完全按双通道设计（所有方法带 `int channel`），但 `StationConfig` 只有一个 `brakeChannel`（默认 1），且 `AcquisitionScheduler::setBrakeDevice` 只接受单一 channel。
- **影响**：如果用户配置使用通道 2，但 `AcquisitionScheduler` 只轮询通道 1，`TelemetrySnapshot` 只会读到通道 1 的数据。
- **建议**：要么将 `brakeChannel` 扩展为数组支持多通道，要么在 `IBrakePowerDevice` 之上再抽象一个 `ISingleChannelBrakeDevice` 用于调度层。

**[II-3] `IMotorDriveDevice::Direction` 枚举位置**
- **问题**：`Direction` 枚举定义在 `IMotorDriveDevice` 类内部，这是好的做法。但其他设备没有类似的专属枚举（如 DYN200 的通信模式在配置层定义，不在设备接口层）。
- **影响**：`Dyn200TorqueSensorDevice` 的 `communicationMode` 语义分散在 `StationConfig`（int 值）和 `Dyn200ProactiveListener::ProtocolMode`（enum）之间，缺少统一映射。

---

## 5. 模拟/真实模式切换一致性

### 5.1 切换机制对比矩阵

| 维度 | AQMD | DYN200 | Encoder | Brake | 一致性 |
|------|------|--------|---------|-------|--------|
| 模拟实现类 | SimulatedMotorDevice | SimulatedTorqueDevice | SimulatedEncoderDevice | SimulatedBrakeDevice | ✅ 统一 |
| 模拟基类 | IMotorDriveDevice | ITorqueSensorDevice | IEncoderDevice | IBrakePowerDevice | ✅ 统一 |
| 模拟构造参数 | `(context, runtime)` | `(context, runtime)` | `(context, runtime)` | `(context, runtime)` | ✅ 统一 |
| 真实构造参数 | `(bus, slaveId, runtime)` | `(bus, slaveId, mode, runtime)` | `(bus, slaveId, res, mode, intv, runtime)` | `(bus, slaveId, runtime)` | 🟡 不一致 |
| 运行时切换方式 | RuntimeManager::switchMode → recreateRuntime → StationRuntimeFactory | 同上 | 同上 | 同上 | ✅ 统一 |
| 切换时旧资源清理 | shutdown → bus close → unique_ptr reset | 同上 | 同上 | 同上 | ✅ 统一 |
| 模拟 `initialize()` 行为 | `return true` | `return true` | `return true` | `return true` | ✅ 统一 |

### 5.2 模拟/真实切换的潜在风险

**[IS-1] 切换时 proactive listener 未正确停止（Encoder）**
- **问题**：`Dyn200TorqueSensorDevice` 的析构函数会 `stop()` 并 `delete` `m_proactiveListener`。但 `SingleTurnEncoderDevice` 的析构函数声明为 `~SingleTurnEncoderDevice() override = default`，其 `m_proactiveListener` 是原始指针，从未被删除，`stop()` 也从未被调用。
- **影响**：当 RuntimeManager 切换模拟/真实模式时，旧的 `SingleTurnEncoderDevice` 被 `unique_ptr::reset()` 释放。由于 parent 是 `StationRuntime`，Qt 对象树会 `delete` 该对象，但 `EncoderProactiveListener` 的 `stop()` 不会被调用，`QSerialPort::readyRead` 信号仍连接着已销毁对象的槽函数，导致**悬空信号连接**和**内存泄漏**。
- **风险等级**：🔴 **高**

**[IS-2] DYN200 主动模式切换回 Modbus 的物理陷阱**
- **问题**：已在前述 DYN200 报告中标记为 P0。此处确认：**Encoder 不存在同样的物理陷阱**——编码器的主动模式可以通过写入寄存器 `0x0006` 关闭（值 `0x00`），再切回 Modbus 轮询。但当前代码的 `setAutoReportMode` 支持此操作，`initialize()` 仅在 `communicationMode != 0` 时设置主动模式，没有显式处理从主动模式切回查询模式的逻辑。
- **风险等级**：🟡 **中**（逻辑缺失，非硬件陷阱）

---

## 6. 跨设备架构不一致风险详单

### 🔴 高风险（P0-P1）

#### R-1 [P0] Encoder 主动监听器内存泄漏与悬空信号（设备层）
- **位置**：`SingleTurnEncoderDevice.h:41` / `EncoderProactiveListener.cpp:33`
- **根因**：`SingleTurnEncoderDevice` 析构函数为默认实现，未释放 `m_proactiveListener`，也未调用其 `stop()`。
- **触发条件**：RuntimeManager 切换模拟/真实模式，或 StationRuntime 重建。
- **后果**：
  1. `EncoderProactiveListener` 对象泄漏。
  2. `QSerialPort::readyRead` 信号仍连接到已销毁 `EncoderProactiveListener` 实例的 `onReadyRead` 槽，程序崩溃或行为未定义。
- **修复建议**：
  ```cpp
  // SingleTurnEncoderDevice.h
  ~SingleTurnEncoderDevice() override;
  
  // SingleTurnEncoderDevice.cpp
  SingleTurnEncoderDevice::~SingleTurnEncoderDevice() {
      if (m_proactiveListener) {
          m_proactiveListener->stop();
          delete m_proactiveListener;
      }
  }
  ```

#### R-2 [P1] Encoder 自动上报间隔硬编码，不可配置（工厂层）
- **位置**：`StationRuntimeFactory.cpp:97`
- **根因**：`20` 是硬编码的 magic number，未从 `config.encoderConfig` 读取。
- **修复建议**：在 `DeviceConfig` 或 `StationConfig` 中增加 `encoderAutoReportIntervalMs` 字段，或在 `encoderConfig` 中使用 `pollIntervalUs` 作为语义等价配置（注意单位转换：ms vs us）。

#### R-3 [P1] Encoder 主动模式解析协议不完整（协议层）
- **位置**：`EncoderProactiveListener.cpp:48-67`
- **根因**：监听器只解析单圈角度的 2 字节数据，但编码器有三种主动模式：
  - Mode 2 (0x01): 单圈值 → 当前解析基本正确（假设 4 字节帧，只取前 2 字节）。
  - Mode 3 (0x04): 虚拟多圈值 → 需要 32-bit 数据，当前解析丢弃了高 16 位。
  - Mode 4 (0x05): 角速度 → 需要 signed 16-bit，当前解析当作 angle 返回，单位完全错误。
- **后果**：用户配置 Encoder 为 mode 3 或 mode 4 时，`readVirtualMultiTurn()` 和 `readAngularVelocity()` 在 proactive 模式下返回错误数据。
- **修复建议**：参照 `Dyn200ProactiveListener` 的设计，为 `EncoderProactiveListener` 增加 `ProtocolMode` 枚举，根据模式选择解析逻辑；或在 `SingleTurnEncoderDevice` 的 read 方法中拒绝 proactive 缓存值，回退到 Modbus 轮询。

### 🟡 中风险（P2）

#### R-4 [P2] `DeviceConfig` 被 `encoderResolution` 污染（配置层）
- **位置**：`StationConfig.h:24`
- **修复建议**：重构为继承结构或从 `StationConfig` 顶层拆分：
  ```cpp
  struct DeviceConfig { /* 通用字段 */ };
  struct StationConfig {
      DeviceConfig aqmdConfig, dyn200Config, encoderConfig, brakeConfig;
      uint16_t encoderResolution = 4096;  // 移出 DeviceConfig
      int brakeChannel = 1;
  };
  ```

#### R-5 [P2] `brakeChannel` 存储位置双重化（配置层）
- **位置**：`DeviceConfigService.cpp:120-122`
- **修复建议**：统一 JSON Schema，将 `brakeChannel` 仅保存在 root 级别，QML 保存时不再往 `brake` 对象内写入 `channel`。

#### R-6 [P2] StationConfigValidator 未覆盖设备专属字段（校验层）
- **位置**：`StationConfigValidator.cpp`
- **缺失校验**：
  - `communicationMode` 范围（DYN200: 0-3；Encoder: 0,2-4）。
  - `pollIntervalUs` 合理范围（例如 > 1000us）。
  - `encoderResolution` > 0（虽然已校验，但应在 `DeviceConfig` 移除后调整路径）。
- **修复建议**：增加 `validateCommunicationMode(device, mode)` 和 `validatePollInterval(interval)` 方法。

#### R-7 [P2] BrakePowerSupplyDevice 独有的重试机制造成行为差异（设备层）
- **位置**：`BrakePowerSupplyDevice.cpp:78-122`
- **根因**：仅制动电源设备在 `writeCoil` 中实现了 `MAX_RETRIES=3` 和 `RETRY_DELAY_MS=50` 的重试逻辑。其他设备的 Modbus 写操作失败即返回。
- **影响**：在弱电磁干扰环境下，制动电源的写操作成功率可能高于其他设备，但这是一种隐式行为差异，调试时会让开发者困惑。
- **修复建议**：要么将重试机制提取到 `IBusController` 或 `ModbusFrame` 层统一实现，要么为所有设备增加可选的重试策略参数。

#### R-8 [P2] 配置保存的字段条件写入导致数据丢失（服务层）
- **位置**：`DeviceConfigService.cpp:117-118`
- **根因**：`pollIntervalUs` 和 `communicationMode` 使用 `if (map.contains(...))` 条件写入。
- **修复建议**：无条件写入所有标准字段，删除 `contains` 判断。

### 🟢 低风险（P3）

#### R-9 [P3] 工厂中模拟/真实分支代码重复度高（工厂层）
- **问题**：`StationRuntimeFactory::create` 的 mock 分支和 real 分支各自创建 4 个 bus + 4 个 device，代码结构对称但重复。可以考虑引入设备元数据表或模板方法进一步抽象。

#### R-10 [P3] `IBrakePowerDevice` 多通道设计与单通道采集调度不匹配（调度层）
- **问题**：`AcquisitionScheduler` 的 `BrakePoller` 只绑定单一 `m_brakeChannel`，但 `IBrakePowerDevice` 原生支持双通道。
- **当前影响低**：因为当前测试场景只使用一个制动通道。但若未来扩展为双制动器测试，需要重构调度层。

---

## 7. 可行动建议（按模块分派）

### 7.1 协议与设备开发 (`protocol-dev`)

**输入需求**：
- **R-1（P0）**：修复 `SingleTurnEncoderDevice` 析构函数，确保 `EncoderProactiveListener` 被正确 `stop()` + `delete`。
- **R-3（P1）**：扩展 `EncoderProactiveListener` 以支持三种自动上报协议的完整解析（单圈 / 虚拟多圈 / 角速度），或至少增加模式不匹配时的安全回退。
- **R-7（P2）**：评估将 `BrakePowerSupplyDevice` 的重试机制下沉到 `ModbusRtuBusController` 层，使所有设备共享统一的重试策略。

**约束条件**：
- `EncoderProactiveListener` 的协议帧格式需与设备手册核对，确认 mode 3/4 的帧长度和数据类型。
- 修改 `Dyn200ProactiveListener` 时要保持向后兼容三种既有协议。

### 7.2 领域引擎开发 (`domain-dev`)

**输入需求**：
- **R-10（P3）**：评估 `TelemetrySnapshot` 是否需要同时记录双通道制动数据（`brakeCh1CurrentA`, `brakeCh2CurrentA`）。
- **R-2（P1）**：如果 `encoderAutoReportIntervalMs` 需要成为可配置参数，请在 `TelemetrySnapshot` 或相关领域模型中预留字段。

**约束条件**：
- `TelemetrySnapshot` 的结构变更会影响 `JsonReportWriter` 和测试报告生成。

### 7.3 ViewModel 开发 (`viewmodel-dev`)

**输入需求**：
- **R-2（P1）**：在 `DeviceConfigPage` 的 Encoder 卡片中增加 "自动上报间隔 (ms)" 输入框（如果配置层增加该字段）。
- **R-6（P2）**：保存配置时增加校验反馈——如果 `communicationMode` 超出设备支持范围，弹出错误提示。
- **R-8（P2）**：确保 QML 传递给 `saveDeviceConfig` 的 `deviceConfig` Map 包含 `pollIntervalUs` 和 `communicationMode` 键，避免条件写入导致的数据丢失。

**约束条件**：
- QML 中 `deviceConfig.encoder.resolution` 的字段名必须与 C++ 层保持一致。

### 7.4 UI/QML 开发 (`ui-dev`)

**输入需求**：
- **R-4（P2）**：Encoder 配置卡片中，当用户选择主动模式（Mode 2/3/4）时，显示警告提示："主动模式下 EncoderProactiveListener 仅完整支持 Mode 2（单圈值），Mode 3/4 的解析尚未完全实现"。
- **R-5（P2）**：统一 `brakeChannel` 的编辑和显示数据源（推荐统一使用 root 级别 `brakeChannel`，不再显示 `brake.channel`）。

**约束条件**：
- `DeviceConfigService` 的 `saveDeviceConfig` 接口签名不变，QML 只需确保 Map 完整。

### 7.5 基础设施/服务开发 (`infra-dev`)

**输入需求**：
- **R-4（P2）**：重构 `StationConfig.h`，将 `encoderResolution` 从 `DeviceConfig` 移至 `StationConfig` 顶层，避免通用结构体被污染。
- **R-5（P2）**：统一 `brakeChannel` 的 JSON 存储位置（仅 root）。
- **R-6（P2）**：增强 `StationConfigValidator`，增加 `communicationMode` 和 `pollIntervalUs` 的校验。
- **R-8（P2）**：修改 `DeviceConfigService::saveDeviceConfig`，无条件写入 `pollIntervalUs` 和 `communicationMode`。
- **R-9（P3）**：评估使用元数据表或工厂模板重构 `StationRuntimeFactory`，减少模拟/真实分支的重复代码。

**约束条件**：
- `station.json` 的路径和格式变更需考虑向后兼容（已有用户的旧配置文件）。
- `StationConfig` 的结构变更会影响 `ConfigLoader` 和 `DeviceConfigService`，需同步修改。

### 7.6 代码审查 (`reviewer`)

**审查重点**：
- [ ] `SingleTurnEncoderDevice` 析构函数是否正确释放了 `m_proactiveListener`（R-1）。
- [ ] `EncoderProactiveListener` 是否支持全部三种主动上报协议（R-3）。
- [ ] `StationRuntimeFactory.cpp:97` 的硬编码 `20` 是否已被替换为配置读取（R-2）。
- [ ] `DeviceConfigService::saveDeviceConfig` 是否还存在条件写入（R-8）。
- [ ] `StationConfigValidator` 是否覆盖了所有四设备的 `communicationMode`（R-6）。

---

## 8. 结论

四设备在**配置加载路径**和**模拟/真实模式切换机制**上保持了良好的一致性，说明项目的整体架构设计是合理的。然而，在**设备专属参数向通用结构体的泄漏**、**工厂创建设备的签名膨胀**、**Encoder 主动上传实现的成熟度差距**这三个方面，存在显著的架构不一致风险。

**最紧急的修复项（P0）**：
1. **Encoder proactive listener 内存泄漏与悬空信号**（R-1）—— 每次模式切换都会泄漏资源并可能崩溃。

**建议优先实施的改进（P1-P2）**：
1. 将 `encoderAutoReportIntervalMs` 配置化（R-2）。
2. 完整实现 Encoder 三种主动上报协议的解析（R-3）。
3. 清理 `DeviceConfig` 的泛化污染和 `brakeChannel` 存储位置（R-4, R-5）。
4. 补全配置校验层对 `communicationMode` 和 `pollIntervalUs` 的覆盖（R-6）。

以上分析可作为代码审查和迭代规划的输入，各角色可根据优先级矩阵认领对应任务。

---

*报告生成时间：基于代码库当前 HEAD 状态*  
*分析涉及文件：~30 个核心源文件 + CMakeLists.txt*
