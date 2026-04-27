# 多设备集成架构一致性分析报告

> 角色：头脑风暴（Brainstorm）
> 日期：2026-04-24
> 范围：DYN200扭矩传感器、AQMD电机驱动、制动电源、单圈编码器
> 状态：已完成架构审查，待修复问题清单已生成

---

## 执行摘要

本次分析对项目中四个核心设备（DYN200扭矩传感器、AQMD电机驱动、制动电源、单圈编码器）的集成架构进行了全面审查。重点检查了配置加载路径、设备创建逻辑、接口抽象模式、模拟/真实模式切换等关键维度。

**总体结论**：四个设备都已完整集成到项目的五层架构中（UI → ViewModel → 领域引擎 → 设备抽象 → 总线协议），整体架构设计遵循了依赖倒置原则和工厂模式。但在配置一致性、工厂创建设备签名、主动上传模式实现成熟度、资源生命周期管理等方面存在架构不一致风险。

| 维度 | 当前状态 | 风险等级 |
|------|---------|---------|
| 模块间耦合度 | 中等（抽象接口隔离良好，但配置层存在隐式耦合） | 🟡 中 |
| 可扩展性 | 良好（新设备可通过ITorqueSensorDevice等扩展） | 🟢 低 |
| 配置管理机制 | 合格（JSON文件+StationConfig结构体，但缺少校验层） | 🟡 中 |
| 主动上传模式实现 | DYN200成熟；Encoder存在内存泄漏、协议解析不完整 | 🔴 高 |
| 资源生命周期管理 | DYN200正确释放；Encoder存在泄漏+悬空信号风险 | 🔴 高 |
| 测试覆盖 | 充分（模拟设备+领域引擎+运行时集成测试均有覆盖） | 🟢 低 |

---

## 1. 架构概览与模块划分

### 1.1 四设备相关模块拓扑图

```
┌─────────────────────────────────────────────────────────────────────────────┐
│  UI/QML Layer (QML)                                                         │
│  ├── DiagnosticsPage.qml ──遥测卡片 (motor/torque/encoder/brake)            │
│  ├── DeviceConfigPage.qml ──设备配置表单 (四设备卡片)                        │
│  └── ...                                                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│  ViewModel Layer (C++)                                                      │
│  ├── DiagnosticsViewModel ──build*Status() / *TelemetryChanged              │
│  └── TestExecutionViewModel ──motorCurrentA / dynTorqueNm / encoderAngleDeg │
├─────────────────────────────────────────────────────────────────────────────┤
│  Domain Layer (C++)                                                         │
│  ├── GearboxTestEngine ──acquireTelemetry() / evaluate*Result()             │
│  ├── TelemetrySnapshot ──dynTorqueNm / dynSpeedRpm / encoderAngleDeg        │
│  └── TestRecipe ──判定限值                                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│  Infrastructure Layer (C++)                                                 │
│  ├── Devices                                                                │
│  │   ├── ITorqueSensorDevice (抽象接口)                                     │
│  │   ├── IMotorDriveDevice (抽象接口)                                       │
│  │   ├── IEncoderDevice (抽象接口)                                          │
│  │   ├── IBrakePowerDevice (抽象接口)                                       │
│  │   ├── Dyn200TorqueSensorDevice (实现) ──Modbus RTU + 3种主动上传         │
│  │   ├── AqmdMotorDriveDevice (实现) ──Modbus RTU 轮询                      │
│  │   ├── SingleTurnEncoderDevice (实现) ──Modbus RTU + 3种自动上报          │
│  │   ├── BrakePowerSupplyDevice (实现) ──Modbus RTU 轮询 (+重试机制)        │
│  │   ├── Dyn200ProactiveListener (协议解析) ──Hex6/Hex8/ASCII               │
│  │   └── EncoderProactiveListener (协议解析) ──单协议固定帧解析              │
│  ├── Bus                                                                    │
│  │   ├── IBusController (抽象接口)                                          │
│  │   └── ModbusRtuBusController (实现) ──underlyingSerialPort()            │
│  ├── Acquisition                                                            │
│  │   └── AcquisitionScheduler ──set*Device(pollIntervalUs)                 │
│  ├── Config                                                                 │
│  │   ├── StationConfig ──aqmdConfig/dyn200Config/encoderConfig/brakeConfig  │
│  │   ├── DeviceConfigService ──load/save 配置到station.json                 │
│  │   ├── ConfigLoader ──JSON反序列化到StationConfig                         │
│  │   ├── RuntimeManager ──switchMode() 模拟/真实切换                        │
│  │   ├── StationRuntime ──运行时设备组装与生命周期管理                        │
│  │   └── StationRuntimeFactory ──根据StationConfig创建运行时实例              │
│  └── Simulation                                                             │
│      ├── SimulatedTorqueDevice / SimulatedMotorDevice                       │
│      ├── SimulatedEncoderDevice / SimulatedBrakeDevice                      │
│      └── SimulationContext ──跨设备状态共享                                 │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 1.2 配置数据流

```
station.json (磁盘持久化)
    │
    ▼
DeviceConfigService::loadDeviceConfig() ──QVariantMap {aqmd/dyn200/encoder/brake}
    │
    ▼
QML (DeviceConfigPage) 双向绑定编辑
    │
    ▼
DeviceConfigService::saveDeviceConfig() ──写回station.json
    │
    ▼
ConfigLoader::loadStationConfig() (main.cpp启动时) ──反序列化到StationConfig
    │
    ▼
StationRuntimeFactory::create() ──读取各设备配置创建设备
    │
    ▼
各设备构造函数 (busController, slaveId, [communicationMode], ...)
    │
    ▼
各设备::initialize() ──Modbus验证或切换主动模式+启动Listener
```

---

## 2. 配置加载路径一致性分析

### 2.1 加载路径对比矩阵

| 路径 | AQMD | DYN200 | Encoder | Brake | 一致性评级 |
|------|------|--------|---------|-------|-----------|
| ConfigLoader::loadStationConfig | ✅ 加载全部通用字段 | ✅ 同上 | ✅ 加载全部 + resolution | ✅ 加载全部通用字段 + channel | 高 ✅ |
| DeviceConfigService::loadDeviceConfig | ✅ QVariantMap读取 | ✅ 同上 | ✅ 同上 + resolution | ✅ 同上 + channel | 高 ✅ |
| DeviceConfigService::saveDeviceConfig | ✅ QVariantMap写入 | ✅ 同上 | ✅ 同上 + resolution | ✅ 同上 + channel + brakeChannel root | 中 🟡 |
| 默认值来源 | StationConfig()构造函数 | StationConfig()构造函数 | StationConfig()构造函数 | StationConfig()构造函数 | 高 ✅ |

### 2.2 配置不一致问题详单

#### IC-1: `encoderResolution` 污染通用结构体
- **位置**：`StationConfig.h:24`
- **问题**：`DeviceConfig` 是四设备通用结构体，但 `encoderResolution` 只有编码器使用。
- **影响**：概念污染、未来新增设备时字段膨胀。
- **建议**：将 `encoderResolution` 移至 `StationConfig` 顶层，或创建 `EncoderConfig : DeviceConfig` 子结构体。

#### IC-2: `brakeChannel` 存储位置不一致
- **位置**：`ConfigLoader.cpp:92` / `DeviceConfigService.cpp:122`
- **问题**：加载时从 JSON root 读取；保存时同时写入 `brake` 对象的 `channel` 字段和 JSON root 的 `brakeChannel`。
- **影响**：QML 编辑 brake channel 时数据来源不一致。
- **建议**：统一只存于 root 级别 `brakeChannel`。

#### IC-3: 配置保存的字段条件写入
- **位置**：`DeviceConfigService.cpp:117-118`
- **问题**：`pollIntervalUs` 和 `communicationMode` 使用条件写入。
- **影响**：主动模式配置可能在保存后丢失。
- **建议**：无条件写入所有标准字段。

---

## 3. StationRuntimeFactory 创建设备逻辑一致性

### 3.1 工厂代码对比

```cpp
// AQMD（3参数）
runtime->m_motor = std::make_unique<Devices::AqmdMotorDriveDevice>(
    runtime->m_aqmdBus.get(), config.aqmdConfig.slaveId, runtime.get()
);

// DYN200（4参数，增加communicationMode）
runtime->m_torque = std::make_unique<Devices::Dyn200TorqueSensorDevice>(
    runtime->m_dyn200Bus.get(), config.dyn200Config.slaveId,
    config.dyn200Config.communicationMode, runtime.get()
);

// Encoder（6参数，增加resolution + communicationMode + hardcoded interval）
runtime->m_encoder = std::make_unique<Devices::SingleTurnEncoderDevice>(
    runtime->m_encoderBus.get(), config.encoderConfig.slaveId,
    config.encoderConfig.encoderResolution,
    config.encoderConfig.communicationMode,
    20, // 硬编码！未从配置读取
    runtime.get()
);

// Brake（3参数）
runtime->m_brake = std::make_unique<Devices::BrakePowerSupplyDevice>(
    runtime->m_brakeBus.get(), config.brakeConfig.slaveId, runtime.get()
);
```

### 3.2 工厂不一致风险清单

| 编号 | 问题描述 | 涉及设备 | 风险等级 |
|------|---------|---------|---------|
| IF-1 | 构造函数签名数量不一致：AQMD/Brake 3参数；DYN200 4参数；Encoder 6参数 | 全部 | 🟡 中 |
| IF-2 | `communicationMode` 参数化不一致：DYN200和Encoder接收；AQMD和Brake不支持 | DYN200, Encoder | 🟡 中 |
| IF-3 | **Encoder自动上报间隔硬编码**：`20`直接写死在代码中 | Encoder | 🔴 高 |
| IF-4 | `runtime.get()` 作为parent传递：unique_ptr移动后可能悬空 | 全部 | 🟡 中 |
| IF-5 | 模拟设备构造签名不一致：模拟`(context, runtime)` vs 真实`(bus, slaveId, ...)` | 全部 | 🟡 中 |

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
| 跨通道支持 | 无 | 无 | 无 | ✅ `int channel` 参数 |

### 4.2 接口抽象问题

#### II-1: 主动模式状态未在接口层抽象
- **问题**：`ITorqueSensorDevice` 和 `IEncoderDevice` 均无 `isProactiveMode()` 或 `setCommunicationMode()` 方法。
- **影响**：Poller 在主动模式下仍按固定频率调用 `readAll()`，造成不必要的线程唤醒。
- **建议**：在接口层增加 `bool isUsingProactiveMode() const` 查询方法。

#### II-2: `IBrakePowerDevice` 多通道设计与单通道采集调度冲突
- **问题**：接口完全按双通道设计，但 `AcquisitionScheduler` 只绑定单一 `brakeChannel`。
- **影响**：若使用通道2，调度层只轮询通道1。
- **建议**：要么扩展 `brakeChannel` 为数组，要么抽象 `ISingleChannelBrakeDevice`。

---

## 5. 模拟/真实模式切换一致性

### 5.1 切换机制对比矩阵

| 维度 | AQMD | DYN200 | Encoder | Brake | 一致性 |
|------|------|--------|---------|-------|--------|
| 模拟实现类 | SimulatedMotorDevice | SimulatedTorqueDevice | SimulatedEncoderDevice | SimulatedBrakeDevice | ✅ 统一 |
| 模拟基类 | IMotorDriveDevice | ITorqueSensorDevice | IEncoderDevice | IBrakePowerDevice | ✅ 统一 |
| 模拟构造参数 | (context, runtime) | (context, runtime) | (context, runtime) | (context, runtime) | ✅ 统一 |
| 真实构造参数 | (bus, slaveId, runtime) | (bus, slaveId, mode, runtime) | (bus, slaveId, res, mode, intv, runtime) | (bus, slaveId, runtime) | 🟡 不一致 |
| 运行时切换方式 | RuntimeManager::switchMode → recreateRuntime → StationRuntimeFactory | 同上 | 同上 | 同上 | ✅ 统一 |
| 切换时旧资源清理 | shutdown → bus close → unique_ptr reset | 同上 | 同上 | 同上 | ✅ 统一 |
| 模拟 `initialize()` 行为 | `return true` | `return true` | `return true` | `return true` | ✅ 统一 |

### 5.2 切换的潜在风险

#### IS-1: 切换时 proactive listener 未正确停止（Encoder）— P0
- **位置**：`SingleTurnEncoderDevice.h:41` / `EncoderProactiveListener.cpp`
- **问题**：`~SingleTurnEncoderDevice() = default`，`m_proactiveListener` 原始指针从未被 `delete`，`stop()` 从未被调用。
- **影响**：模式切换时 `QSerialPort::readyRead` 信号仍连接已销毁对象，导致**悬空信号连接**和**内存泄漏**。
- **修复建议**：显式析构函数中 `m_proactiveListener->stop(); delete m_proactiveListener;`。

#### IS-2: DYN200 主动模式切回 Modbus 的物理陷阱
- **问题**：从主动上传模式切回 Modbus RTU 需物理按键恢复出厂设置。
- **风险等级**：🔴 **高**（已在 DYN200 分析报告中标记为 P0）
- **建议**：UI/ViewModel/设备层三层添加防护。

---

## 6. 跨设备架构不一致风险详单

### 🔴 高风险（P0-P1）

#### R-1 [P0] Encoder 主动监听器内存泄漏与悬空信号
- **位置**：`SingleTurnEncoderDevice.h:41`
- **根因**：析构函数为默认实现，未释放 `m_proactiveListener`。
- **触发条件**：RuntimeManager 切换模拟/真实模式。
- **修复建议**：
  ```cpp
  SingleTurnEncoderDevice::~SingleTurnEncoderDevice() {
      if (m_proactiveListener) {
          m_proactiveListener->stop();
          delete m_proactiveListener;
      }
  }
  ```

#### R-2 [P0] Encoder 分辨率硬编码 + 帧处理错误
- **位置**：`EncoderProactiveListener.cpp:55,63`
- **根因**：硬编码 `65535.0`；`m_buffer.remove(0, 2)` 与 `>= 4` 条件不匹配；`break` 中断循环。
- **修复建议**：传入 `m_resolution`；修正 `remove(0, 4)` 并移除 `break`。

#### R-3 [P1] Encoder 自动上报间隔硬编码
- **位置**：`StationRuntimeFactory.cpp:97`
- **根因**：`20` 是硬编码 magic number。
- **修复建议**：在 `DeviceConfig` 中增加 `encoderAutoReportIntervalMs` 字段。

#### R-4 [P1] Encoder 主动模式解析协议不完整
- **位置**：`EncoderProactiveListener.cpp:48-67`
- **根因**：只解析单圈角度，不支持 Mode 3（多圈）和 Mode 4（速度）。
- **修复建议**：参照 `Dyn200ProactiveListener` 增加 `ProtocolMode` 枚举和分模式解析。

### 🟡 中风险（P2）

#### R-5 [P2] `DeviceConfig` 被 `encoderResolution` 污染
- **位置**：`StationConfig.h:24`
- **修复建议**：重构为继承结构或从 `StationConfig` 顶层拆分。

#### R-6 [P2] `brakeChannel` 存储位置双重化
- **位置**：`DeviceConfigService.cpp:120-122`
- **修复建议**：统一 JSON Schema，仅保存在 root 级别。

#### R-7 [P2] StationConfigValidator 未覆盖设备专属字段
- **位置**：`StationConfigValidator.cpp`
- **缺失校验**：`communicationMode` 范围、`pollIntervalUs` 合理范围。
- **修复建议**：增加对应校验方法。

#### R-8 [P2] BrakePowerSupplyDevice 独有重试机制
- **位置**：`BrakePowerSupplyDevice.cpp:78-122`
- **根因**：仅制动电源实现 `MAX_RETRIES=3`。
- **修复建议**：提取到 `ModbusRtuBusController` 层统一实现。

#### R-9 [P2] 配置保存字段条件写入导致数据丢失
- **位置**：`DeviceConfigService.cpp:117-118`
- **修复建议**：无条件写入所有标准字段。

### 🟢 低风险（P3）

#### R-10 [P3] 工厂模拟/真实分支代码重复度高
- **建议**：评估使用元数据表或模板方法重构。

#### R-11 [P3] `IBrakePowerDevice` 多通道设计与单通道调度不匹配
- **建议**：评估 `TelemetrySnapshot` 是否需要双通道制动数据。

---

## 7. 各设备模块耦合度评估

| 模块对 | 耦合类型 | 强度 | 说明 |
|--------|---------|------|------|
| AQMD ↔ IBusController | 接口依赖 | 弱 ✅ | 通过 IBusController 抽象，可替换总线实现 |
| DYN200 ↔ IBusController | 接口依赖 | 弱 ✅ | 同上；额外依赖 `underlyingSerialPort()` 用于主动模式 |
| Encoder ↔ IBusController | 接口依赖 | 弱 ✅ | 同上；额外依赖 `underlyingSerialPort()` 用于主动模式 |
| Brake ↔ IBusController | 接口依赖 | 弱 ✅ | 同上 |
| StationRuntimeFactory ↔ 各设备 | 工厂依赖 | 中 🟡 | 工厂硬编码各设备构造函数签名，新增设备需修改工厂 |
| DeviceConfigService ↔ station.json | 文件依赖 | 中 🟡 | 字段映射保持人工一致，缺少自动化校验 |
| ConfigLoader ↔ StationConfig | 数据依赖 | 弱 ✅ | 强类型结构体，编译期可检查 |
| AcquisitionScheduler ↔ 各设备 | 接口依赖 | 弱 ✅ | 通过设备抽象接口访问，不依赖具体类型 |
| SimulatedXxxDevice ↔ SimulationContext | 数据依赖 | 中 🟡 | 原始指针传递，但生命周期由 StationRuntime 管理 |

---

## 8. 可扩展性评估

### 8.1 新增第五设备的扩展路径

当前架构支持通过以下步骤扩展新设备：

1. **定义抽象接口**：`INewDevice.h`（继承 `QObject`，定义 `initialize()` + `lastError()`）
2. **实现真实设备**：`NewDevice.h/cpp`（继承接口，实现 Modbus 通信逻辑）
3. **实现模拟设备**：`SimulatedNewDevice.h/cpp`（继承接口，基于 `SimulationContext`）
4. **扩展 `StationConfig`**：在 `DeviceConfig`（或子结构体）中增加配置字段
5. **扩展 `ConfigLoader`**：增加 JSON 反序列化逻辑
6. **扩展 `DeviceConfigService`**：增加 QVariantMap 读写逻辑
7. **扩展 `StationRuntimeFactory`**：增加新设备的创建逻辑
8. **扩展 `StationRuntime`**：增加新设备的 unique_ptr 成员和 getter
9. **扩展 `AcquisitionScheduler`**：增加 `setNewDevice()` 和 `NewPoller`
10. **扩展 `TelemetryBuffer`**：增加新设备的缓存子结构
11. **扩展 `TelemetrySnapshot`**：增加新设备的遥测字段
12. **扩展 ViewModel 和 QML**：增加遥测显示和配置 UI

### 8.2 扩展瓶颈

| 瓶颈 | 原因 | 改进建议 |
|------|------|---------|
| StationRuntimeFactory 膨胀 | 每新增设备需增加 2 处创建逻辑（真实 + 模拟） | 引入设备元数据表 + 模板工厂 |
| ConfigLoader / DeviceConfigService 重复 | JSON 字段映射需两处同步修改 | 提取共享字段常量或反射生成 |
| TelemetryBuffer / Snapshot 膨胀 | 每设备增加一组 atomic 字段 | 使用泛型缓存模板 `DeviceCache<T>` |
| QML 硬编码索引 | `deviceStatuses[0/1/2/3]` 直接索引 | ViewModel 提供具名属性或枚举 |

---

## 9. 可行动建议（按模块分派）

### 9.1 协议与设备开发 (`protocol-dev`)

**当前输入需求**：
- 修复 `SingleTurnEncoderDevice` 析构函数，确保 `EncoderProactiveListener` 被正确 `stop()` + `delete`。
- 扩展 `EncoderProactiveListener` 支持三种自动上报协议完整解析（单圈 / 虚拟多圈 / 角速度）。
- 将 `BrakePowerSupplyDevice` 重试机制下沉到 `ModbusRtuBusController` 层。

**约束条件**：
- `EncoderProactiveListener` 的协议帧格式需与设备手册核对。
- 修改 `Dyn200ProactiveListener` 时要保持向后兼容。

### 9.2 领域引擎开发 (`domain-dev`)

**当前输入需求**：
- 评估 `TelemetrySnapshot` 是否需要同时记录双通道制动数据。
- 如果 `encoderAutoReportIntervalMs` 需要可配置，在领域模型中预留字段。

**约束条件**：
- `TelemetrySnapshot` 的结构变更会影响 `JsonReportWriter` 和测试报告生成。

### 9.3 ViewModel 开发 (`viewmodel-dev`)

**当前输入需求**：
- 在 `DeviceConfigPage` 的 Encoder 卡片中增加 "自动上报间隔 (ms)" 输入框。
- 保存配置时增加校验反馈 —— `communicationMode` 超出范围时弹出错误提示。
- 确保 QML 传递给 `saveDeviceConfig` 的 Map 包含 `pollIntervalUs` 和 `communicationMode`。

**约束条件**：
- QML 中 `deviceConfig.encoder.resolution` 的字段名必须与 C++ 层保持一致。

### 9.4 UI/QML 开发 (`ui-dev`)

**当前输入需求**：
- Encoder 配置卡片中，选择主动模式时显示警告："主动模式下 EncoderProactiveListener 仅完整支持 Mode 2"。
- 统一 `brakeChannel` 的编辑和显示数据源（推荐统一使用 root 级别 `brakeChannel`）。

**约束条件**：
- `DeviceConfigService` 的 `saveDeviceConfig` 接口签名不变。

### 9.5 基础设施/服务开发 (`infra-dev`)

**当前输入需求**：
- 重构 `StationConfig.h`，将 `encoderResolution` 从 `DeviceConfig` 移至 `StationConfig` 顶层。
- 统一 `brakeChannel` 的 JSON 存储位置（仅 root）。
- 增强 `StationConfigValidator`，增加 `communicationMode` 和 `pollIntervalUs` 校验。
- 修改 `DeviceConfigService::saveDeviceConfig`，无条件写入 `pollIntervalUs` 和 `communicationMode`。
- 评估使用元数据表重构 `StationRuntimeFactory`。

**约束条件**：
- `station.json` 的格式变更需考虑向后兼容。
- `StationConfig` 的结构变更会影响 `ConfigLoader` 和 `DeviceConfigService`。

---

## 10. 结论

四设备在**配置加载路径**和**模拟/真实模式切换机制**上保持了良好的一致性，说明项目的整体架构设计是合理的。然而，在**设备专属参数向通用结构体的泄漏**、**工厂创建设备的签名膨胀**、**Encoder 主动上传实现的成熟度差距**三个方面，存在显著的架构不一致风险。

**最紧急的修复项（P0）**：
1. **Encoder proactive listener 内存泄漏与悬空信号**（R-1）—— 每次模式切换都会泄漏资源并可能崩溃。
2. **Encoder 分辨率硬编码 + 帧处理错误**（R-2）—— 主动模式下角度数据完全错误。

**建议优先实施的改进（P1-P2）**：
1. 将 `encoderAutoReportIntervalMs` 配置化（R-3）。
2. 完整实现 Encoder 三种主动上报协议解析（R-4）。
3. 清理 `DeviceConfig` 的泛化污染和 `brakeChannel` 存储位置（R-5, R-6）。
4. 补全配置校验层对 `communicationMode` 和 `pollIntervalUs` 的覆盖（R-7）。

以上分析可作为代码审查和迭代规划的输入，各角色可根据优先级矩阵认领对应任务。

---

*报告生成时间：基于代码库当前 HEAD 状态*  
*分析涉及文件：~30 个核心源文件 + 7 份审查报告*
