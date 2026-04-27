# DYN200 参数配置功能 — 架构设计与集成问题分析报告

> 角色：头脑风暴（Brainstorm）  
> 任务：DYN200集成架构分析（cfc68607-8ded-4cb1-92e4-741b7a74a29f）  
> 分析范围：DYN200扭矩传感器在整个FlipGearboxFactoryTest项目中的参数配置、模块集成、耦合关系与可扩展性  

---

## 执行摘要

DYN200扭矩传感器已**完整集成**到项目的五层架构中（UI → ViewModel → 领域引擎 → 设备抽象 → 总线协议），支持4种通信模式（Modbus RTU轮询 + 3种主动上传模式），具备模拟/真实硬件双模式运行时切换能力。整体架构设计遵循了依赖倒置原则和工厂模式，但在**配置一致性**、**通信模式回退机制**、**跨层数据契约**和**运行时重配置**四个方面存在可改进空间。

| 维度 | 当前状态 | 风险等级 |
|------|---------|---------|
| 模块间耦合度 | 中等（抽象接口隔离良好，但配置层存在隐式耦合） | 🟡 中 |
| 可扩展性 | 良好（新设备可通过ITorqueSensorDevice扩展） | 🟢 低 |
| 配置管理机制 | 合格（JSON文件+StationConfig结构体，但缺少校验层） | 🟡 中 |
| 通信模式管理 | 存在陷阱（主动模式切回Modbus需手动恢复出厂设置） | 🔴 高 |
| 测试覆盖 | 充分（模拟设备+领域引擎+运行时集成测试均有覆盖） | 🟢 低 |

---

## 1. 架构概览与模块划分

### 1.1 DYN200相关模块拓扑图

```
┌─────────────────────────────────────────────────────────────────────────────┐
│  UI/QML Layer (QML)                                                         │
│  ├── DiagnosticsPage.qml ──扭矩遥测卡片 (torqueTelemetry)                    │
│  ├── DeviceConfigPage.qml ──dyn200配置表单 (portName/baudRate/commMode等)    │
│  └── ...                                                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│  ViewModel Layer (C++)                                                      │
│  ├── DiagnosticsViewModel ──buildTorqueStatus() / torqueTelemetryChanged    │
│  └── TestExecutionViewModel ──torque/speed/power Q_PROPERTY                  │
├─────────────────────────────────────────────────────────────────────────────┤
│  Domain Layer (C++)                                                         │
│  ├── GearboxTestEngine ──acquireTelemetry() / evaluateLoadResult()          │
│  ├── TelemetrySnapshot ──dynTorqueNm / dynSpeedRpm / dynPowerW              │
│  └── TestRecipe ──loadForwardTorqueMin/Max等判定限值                         │
├─────────────────────────────────────────────────────────────────────────────┤
│  Infrastructure Layer (C++)                                                 │
│  ├── Devices                                                                │
│  │   ├── ITorqueSensorDevice (抽象接口)                                     │
│  │   ├── Dyn200TorqueSensorDevice (实现) ──Modbus RTU + 主动上传双模         │
│  │   ├── Dyn200ProactiveListener (协议解析) ──Hex6/Hex8/ASCII               │
│  │   └── SimulatedTorqueDevice (模拟实现)                                  │
│  ├── Bus                                                                    │
│  │   ├── IBusController (抽象接口)                                          │
│  │   └── ModbusRtuBusController (实现) ──underlyingSerialPort()            │
│  ├── Acquisition                                                            │
│  │   ├── AcquisitionScheduler ──setTorqueDevice(pollIntervalUs)            │
│  │   └── TorquePoller (QThread) ──readAll()循环采集                         │
│  └── Config                                                                 │
│      ├── StationConfig ──dyn200Config (DeviceConfig结构体)                  │
│      ├── DeviceConfigService ──load/save dyn200配置到station.json           │
│      ├── ConfigLoader ──JSON反序列化到StationConfig                         │
│      ├── RuntimeManager ──switchMode() 模拟/真实切换                        │
│      ├── StationRuntime ──运行时设备组装与生命周期管理                        │
│      └── StationRuntimeFactory ──根据StationConfig创建运行时实例              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 1.2 DYN200参数配置的数据流

```
station.json (磁盘持久化)
    │
    ▼
DeviceConfigService::loadDeviceConfig() ──QVariantMap {dyn200: {...}}
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
StationRuntimeFactory::create() ──读取 config.dyn200Config 创建设备
    │
    ▼
Dyn200TorqueSensorDevice 构造函数 (busController, slaveId, communicationMode)
    │
    ▼
Dyn200TorqueSensorDevice::initialize() ──Modbus验证或切换主动模式+启动Listener
```

**关键发现**：配置存在**两条独立路径**——`DeviceConfigService`（QML直接读写JSON的QVariantMap路径）和 `ConfigLoader`（C++强类型StationConfig路径）。两者在字段映射上保持了人工一致性，但缺少自动化校验机制。

---

## 2. 模块间耦合度分析

### 2.1 耦合度评估矩阵

| 模块对 | 耦合类型 | 强度 | 说明 |
|--------|---------|------|------|
| Dyn200TorqueSensorDevice ↔ IBusController | 接口依赖 | 弱 ✅ | 通过IBusController抽象，可替换总线实现 |
| Dyn200TorqueSensorDevice ↔ ModbusRtuBusController | 实现泄漏 | 中 🟡 | `getSerialPort()` 调用 `underlyingSerialPort()`，暴露QSerialPort细节 |
| Dyn200TorqueSensorDevice ↔ Dyn200ProactiveListener | 组合+生命周期管理 | 中 🟡 | 析构时stop+delete，通信模式切换时动态创建 |
| DiagnosticsViewModel ↔ StationRuntime | 直接依赖 | 中 🟡 | 通过原始指针访问，Runtime重建时需重新连接 |
| DiagnosticsViewModel ↔ ITorqueSensorDevice | 间接依赖 | 弱 ✅ | 通过StationRuntime转发，不直接引用DYN200类型 |
| AcquisitionScheduler ↔ ITorqueSensorDevice | 接口依赖 | 弱 ✅ | 通过setTorqueDevice注入，轮询器仅调用readAll() |
| GearboxTestEngine ↔ TelemetrySnapshot | 数据耦合 | 弱 ✅ | 通过快照获取DYN200数据，与具体设备解耦 |
| DeviceConfigService ↔ StationConfig | 结构耦合 | 中 🟡 | 需手动保持QVariantMap字段与DeviceConfig字段一致 |
| QML ↔ deviceConfigService | 运行时上下文注入 | 弱 ✅ | 通过setContextProperty暴露，QML无编译时依赖 |

### 2.2 关键耦合问题：串口抽象泄漏

**问题描述**：`Dyn200TorqueSensorDevice::getSerialPort()` 直接调用 `IBusController::underlyingSerialPort()`，将 `QSerialPort*` 传递给 `Dyn200ProactiveListener`。这破坏了总线控制器的封装边界。

**代码位置**：
- `src/infrastructure/devices/Dyn200TorqueSensorDevice.cpp:250-253`
- `src/infrastructure/bus/IBusController.h:67`

**影响**：
- 若未来使用非串口总线（如TCP Modbus、CAN总线），主动上传模式将不可用
- `IBusController` 被迫暴露 `underlyingSerialPort()` 这一与抽象语义不符的方法

**可行动建议**：
- **协议与设备开发** (`protocol-dev`)：建议将主动上传模式的数据接收抽象为独立接口 `IProactiveDataSource`，由 `ModbusRtuBusController` 实现该接口，而非直接暴露 `QSerialPort*`
- **领域引擎开发** (`domain-dev`)：当前无直接影响，但需记录此约束——DYN200的主动模式绑定到串口总线

### 2.3 关键耦合问题：ViewModel与Runtime生命周期绑定

**问题描述**：`DiagnosticsViewModel` 和 `TestExecutionViewModel` 持有 `StationRuntime*` 原始指针。当 `RuntimeManager::switchMode()` 触发 `runtimeRecreated` 时，ViewModel需手动更新指针并重新连接信号。

**代码位置**：
- `src/viewmodels/DiagnosticsViewModel.cpp:64-68`（连接runtimeRecreated信号）
- `src/viewmodels/TestExecutionViewModel.cpp:367-369`（updateRuntime）

**影响**：
- 存在悬空指针风险（虽然当前通过信号机制处理，但逻辑分散）
- 新增ViewModel时容易遗漏runtime重建处理

**可行动建议**：
- **ViewModel开发** (`viewmodel-dev`)：考虑引入 `QPointer<StationRuntime>` 替代原始指针，或让 `RuntimeManager` 提供稳定的间接访问层（如 `runtime()` 始终返回当前有效实例，旧实例延迟析构）

---

## 3. 可扩展性评估

### 3.1 设备类型扩展性：优秀 ✅

通过 `ITorqueSensorDevice` 接口，新增扭矩传感器类型（如非DYN200型号）只需：
1. 实现 `ITorqueSensorDevice` 接口
2. 在 `StationRuntimeFactory` 中根据配置条件创建对应实例
3. （可选）添加对应QML诊断卡片

**当前已实现的可扩展点**：
- `SimulatedTorqueDevice`：完整的模拟实现，支持SimulationContext物理模型
- `MockTorqueDevice`：测试专用，支持手动设置mockSpeedRpm/mockTorqueNm

### 3.2 通信模式扩展性：受限 🟡

DYN200支持4种通信模式，但扩展受限：

| 模式 | 实现位置 | 扩展难度 |
|------|---------|---------|
| 0 Modbus RTU | Dyn200TorqueSensorDevice::readAll() | 低（基于ModbusFrame通用协议） |
| 1 HEX 6-byte | Dyn200ProactiveListener::parseHex6ByteFrame() | 中（需修改Listener的switch-case） |
| 2 HEX 8-byte | Dyn200ProactiveListener::parseHex8ByteFrame() | 中 |
| 3 ASCII | Dyn200ProactiveListener::parseAsciiFrame() | 中 |

**限制**：
- `Dyn200ProactiveListener` 使用 `enum ProtocolMode` 硬编码3种协议，新增协议需修改枚举和switch-case
- `Dyn200TorqueSensorDevice::initialize()` 中 modeValue 到 listenerMode 的映射是硬编码的（case 1/2/3）

**可行动建议**：
- **协议与设备开发** (`protocol-dev`)：建议将 `ProtocolMode` 重构为策略模式，每种协议实现 `IProactiveFrameParser` 接口，通过工厂注册。新增协议时无需修改Listener核心逻辑

### 3.3 配置字段扩展性：合格 🟡

**StationConfig/DeviceConfig 结构**：
- 新增通用字段（如所有设备共有的 `retryCount`、`heartbeatInterval`）：需修改 `DeviceConfig` 结构体、`DeviceConfigService` 的load/save逻辑、QML表单、ConfigLoader
- 新增DYN200专用字段（如 `torqueFilterWindow`、`zeroPointOffset`）：需修改 `DeviceConfig`（影响所有设备）或引入设备专用配置结构

**当前问题**：`DeviceConfig` 是通用结构体，DYN200专用参数（如 `communicationMode`）被放入通用结构中，造成语义污染（编码器也有 `communicationMode`，但含义完全不同）。

**可行动建议**：
- **基础设施/服务开发** (`infra-dev`)：考虑将 `DeviceConfig` 改为基类结构，各设备继承并添加专用字段；或引入 `QVariantMap deviceSpecific` 扩展字段桶，避免频繁修改结构体

---

## 4. 配置管理机制分析

### 4.1 配置分层架构

```
Layer 1: 磁盘持久化 (station.json / recipes/*.json)
    │
Layer 2: 服务封装 (DeviceConfigService / RecipeService)
    │         ├─ Q_INVOKABLE QVariantMap loadDeviceConfig()  [QML可见]
    │         └─ Q_INVOKABLE bool saveDeviceConfig(...)      [QML可见]
    │
Layer 3: 强类型配置 (StationConfig / TestRecipe)
    │         ├─ ConfigLoader::loadStationConfig() ── main.cpp启动加载
    │         └─ RecipeConfig::fromJson() / toJson()
    │
Layer 4: 运行时实例 (StationRuntime / Dyn200TorqueSensorDevice)
    │         └─ StationRuntimeFactory::create() 从StationConfig创建设备
```

### 4.2 DYN200配置字段清单与一致性检查

| 字段 | station.json键 | StationConfig路径 | QML绑定 | 运行时消费 |
|------|---------------|-------------------|---------|-----------|
| portName | dyn200.portName | dyn200Config.portName | ✅ DeviceConfigPage | StationRuntimeFactory |
| baudRate | dyn200.baudRate | dyn200Config.baudRate | ✅ | StationRuntimeFactory |
| slaveId | dyn200.slaveId | dyn200Config.slaveId | ✅ | StationRuntimeFactory |
| timeout | dyn200.timeout | dyn200Config.timeout | ✅ | StationRuntimeFactory |
| parity | dyn200.parity | dyn200Config.parity | ✅ | StationRuntimeFactory |
| stopBits | dyn200.stopBits | dyn200Config.stopBits | ✅ | StationRuntimeFactory |
| enabled | dyn200.enabled | dyn200Config.enabled | ✅ | StationRuntimeFactory |
| pollIntervalUs | dyn200.pollIntervalUs | dyn200Config.pollIntervalUs | ✅ | AcquisitionScheduler |
| communicationMode | dyn200.communicationMode | dyn200Config.communicationMode | ✅ | Dyn200TorqueSensorDevice |

**一致性状态**：✅ 全部9个字段在4个层级（JSON/结构体/QML/运行时）中保持映射一致。

### 4.3 配置管理机制的缺陷

#### 缺陷A：缺少配置校验层

**描述**：`DeviceConfigService::saveDeviceConfig()` 和 `ConfigLoader::loadStationConfig()` 均不校验字段合法性。例如：
- `baudRate` 可以是任意整数（非法值如 12345 不会报错）
- `communicationMode` 可以是任意整数（只有0/1/2/3有效，但4不会报错，直到 `initialize()` 才失败）
- `slaveId` 可以是0或大于247的值

**代码位置**：
- `src/infrastructure/config/DeviceConfigService.cpp:90-139`（save无校验）
- `src/infrastructure/config/ConfigLoader.cpp:34-91`（load无校验）

**风险**：
- 用户在前端保存非法配置后，运行时初始化失败，错误信息延迟暴露
- 非法JSON结构可能导致ConfigLoader静默回退到默认值，用户无感知

**可行动建议**：
- **基础设施/服务开发** (`infra-dev`)：在 `StationConfigValidator` 中添加 `validateDyn200Config(const DeviceConfig&)` 方法，校验 baudRate∈{9600,19200,38400,57600,115200}、slaveId∈[1,247]、communicationMode∈[0,3]、stopBits∈{1,2}、parity∈{"None","Even","Odd"}
- **UI/QML开发** (`ui-dev`)：在DeviceConfigPage的输入控件上添加即时校验（如baudRate使用下拉选择而非自由输入）

#### 缺陷B：运行时重配置能力缺失

**描述**：修改DYN200配置（如baudRate或communicationMode）后，必须重启应用或切换模拟/真实模式才能生效。`StationRuntime` 不支持运行时重新初始化单个设备。

**代码验证**：
- `DeviceConfigPage.qml` 的 `saveConfig()` 仅写JSON，无重启Runtime逻辑
- `main.cpp:74` 的 `RuntimeManager` 在启动时创建，后续仅支持 `switchMode()`（整体重建）

**风险**：
- 现场调试时修改串口参数需要重启应用，影响生产效率
- communicationMode切换后若写入DYN200寄存器失败，设备可能进入不可控状态

**可行动建议**：
- **基础设施/服务开发** (`infra-dev`)：为 `StationRuntime` 添加 `reinitializeDevice(DeviceType)` 方法，支持单设备热重启；或添加 `applyConfig(const StationConfig&)` 方法进行差异比对和局部重建
- **协议与设备开发** (`protocol-dev`)：`Dyn200TorqueSensorDevice` 需支持 `deinitialize()` 方法（当前析构仅stop Listener），确保通信模式切换前能安全关闭当前模式

#### 缺陷C：communicationMode切换陷阱

**描述**：DYN200的主动上传模式（1/2/3）切换到Modbus RTU模式（0）需要**手动恢复出厂设置**（通过传感器物理按键）。代码中的注释已警告此限制，但配置页面允许用户自由切换此值。

**代码位置**：
- `src/infrastructure/devices/Dyn200TorqueSensorDevice.h:29-31`
- `src/infrastructure/devices/Dyn200TorqueSensorDevice.cpp:79`

**风险**：🔴 **高** — 用户在UI中将communicationMode从1改回0并保存后，DYN200实际仍工作在主动上传模式，但软件按Modbus RTU初始化，导致通信完全失败。由于需要物理按键恢复，现场工程师可能无法快速解决。

**可行动建议**：
- **UI/QML开发** (`ui-dev`)：在DeviceConfigPage中，communicationMode字段旁添加**显式警告标签**，说明模式1/2/3切换后无法通过软件恢复为模式0
- **协议与设备开发** (`protocol-dev`)：在 `Dyn200TorqueSensorDevice::initialize()` 中，当检测到从主动模式切换回Modbus RTU的尝试时，返回明确的错误信息（如"无法通过通信命令将DYN200从主动上传模式切回Modbus RTU，请使用传感器物理按键恢复出厂设置"）
- **ViewModel开发** (`viewmodel-dev`)：在save配置时，若communicationMode发生变化，弹出确认对话框说明风险

---

## 5. 潜在架构问题识别

### 问题1：主动上传模式与Modbus轮询的串口竞争 🟡

**描述**：在主动上传模式下，`Dyn200ProactiveListener` 直接连接 `QSerialPort::readyRead` 信号。如果此时 `Dyn200TorqueSensorDevice` 的 `readAll()` 被调用（Modbus RTU请求），两者会在同一条串线上竞争数据。

**当前缓解措施**：主动模式下，`readAll()` 直接从 `m_proactiveListener->latestTorque()` 读取缓存值，不发送Modbus请求（见 `Dyn200TorqueSensorDevice.cpp:99-103`）。

**残留风险**：
- `initialize()` 中先通过 `writeRegister(REG_COMM_MODE, modeValue)` 写寄存器（Modbus命令），然后立即启动Listener。若DYN200在收到写命令后立刻开始主动上传，Modbus响应帧和主动上传帧可能混淆
- `readTorque()` / `readSpeed()` / `readPower()` 在主动模式下各自独立调用缓存，但 `readAll()` 同时读取三个值，存在时间不一致性（torque和speed可能来自不同帧）

**可行动建议**：
- **协议与设备开发** (`protocol-dev`)：在 `writeRegister(REG_COMM_MODE)` 后增加足够延迟（如200ms），确保Modbus响应完整接收后再启动Listener；或考虑在 `readAll()` 中返回Listener最近一次完整解析的帧的时间戳，让调用方判断数据新鲜度

### 问题2：TorquePoller的线程模型隐患 🟡

**描述**：`TorquePoller` 继承 `QThread` 并在 `run()` 中循环调用 `m_device->readAll()`。`Dyn200TorqueSensorDevice::readAll()` 在主动模式下访问 `m_proactiveListener` 的成员变量（`latestTorque()` 等），无显式同步机制。

**代码位置**：
- `src/infrastructure/acquisition/DevicePoller.h:66-111`（TorquePoller）
- `src/infrastructure/devices/Dyn200TorqueSensorDevice.cpp:117-136`（readAll）
- `src/infrastructure/devices/Dyn200ProactiveListener.h:46-51`（latestTorque/latestSpeed，非线程安全）

**风险**：`Dyn200ProactiveListener::onReadyRead()` 在主线程（串口事件线程）执行，更新 `m_latestTorque`；`TorquePoller` 在独立线程读取 `m_latestTorque`。Qt信号槽默认是队列连接，但直接成员访问无原子保护。

**可行动建议**：
- **协议与设备开发** (`protocol-dev`)：为 `Dyn200ProactiveListener` 的 `m_latestTorque` / `m_latestSpeed` 添加 `std::atomic<double>` 保护，或改用QMutex；或在Listener中通过信号将数据发送到TorquePoller线程（但会增加开销）

### 问题3：DiagnosticsViewModel与AcquisitionScheduler的数据来源不一致 🟡

**描述**：`DiagnosticsViewModel::buildTorqueStatus()` 直接调用 `m_runtime->torque()->readAll()`（同步设备读取），而 `TestExecutionViewModel` 和 `GearboxTestEngine` 使用 `AcquisitionScheduler::snapshot()`（异步缓冲数据）。两者数据来源不同，可能在高速场景下显示不一致的遥测值。

**代码位置**：
- `src/viewmodels/DiagnosticsViewModel.cpp:315-338`（直接调用readAll）
- `src/domain/GearboxTestEngine.cpp:132`（通过acquireTelemetry获取snapshot）

**可行动建议**：
- **ViewModel开发** (`viewmodel-dev`)：统一DiagnosticsViewModel的遥测来源，优先使用 `AcquisitionScheduler::snapshot()`，确保诊断页面与测试引擎看到的数据一致；或在UI上明确标注诊断页面的数据是"实时直读"而非"调度缓冲"

### 问题4：模拟模式与真实模式的配置分叉 🟡

**描述**：`StationRuntimeFactory::create()` 在模拟模式下使用硬编码的虚拟配置（`{"SIM_DYN200", 9600, 1, 1000, "None", 1}`），忽略用户配置的 `dyn200Config` 参数。这导致模拟模式下无法验证配置是否正确（如baudRate不匹配真实设备）。

**代码位置**：
- `src/infrastructure/config/StationRuntimeFactory.cpp:36-39`

**可行动建议**：
- **领域引擎开发** (`domain-dev`)：模拟模式下仍读取用户配置的baudRate等参数，在SimulatedBusController中模拟该参数下的通信时序（如故意在错误baudRate下模拟超时），帮助用户预验证配置

---

## 6. 改进建议与可行动方案

### 优先级矩阵

| 优先级 | 改进项 | 负责角色 | 工作量 | 影响 |
|--------|--------|---------|--------|------|
| P0 | communicationMode切换陷阱防护 | ui-dev + protocol-dev | 小 | 避免现场设备锁死 |
| P1 | 配置校验层（StationConfigValidator） | infra-dev | 中 | 提升配置可靠性 |
| P1 | TorquePoller线程安全修复 | protocol-dev | 小 | 消除数据竞争 |
| P2 | 统一遥测数据来源 | viewmodel-dev | 中 | 消除诊断/执行数据不一致 |
| P2 | 运行时单设备热重配置 | infra-dev | 大 | 提升调试效率 |
| P3 | 主动上传协议策略模式化 | protocol-dev | 中 | 提升通信模式扩展性 |
| P3 | 串口抽象泄漏修复 | protocol-dev | 中 | 支持非串口总线 |
| P3 | 模拟模式配置仿真 | domain-dev | 中 | 提升模拟真实性 |

### 6.1 P0：communicationMode切换陷阱防护（立即行动）

**方案**：三层防护

1. **UI层** (`ui-dev`)：
   ```qml
   // DeviceConfigPage.qml 中 dyn200 卡片
   Components.AppAlert {
       visible: root.deviceConfig.dyn200.communicationMode !== 0
       variant: "warning"
       text: "警告：DYN200当前处于主动上传模式。切回Modbus RTU(模式0)需要手动恢复出厂设置！"
   }
   ```

2. **ViewModel层** (`viewmodel-dev`)：
   在保存配置前检查 `communicationMode` 变更，若从非0变为0，弹出确认对话框。

3. **设备层** (`protocol-dev`)：
   在 `Dyn200TorqueSensorDevice::initialize()` 中，主动模式下检测是否尝试写 `REG_COMM_MODE` 为Modbus模式，若是则返回明确错误。

### 6.2 P1：配置校验层

**方案**：扩展 `StationConfigValidator`

```cpp
// src/infrastructure/config/StationConfigValidator.h 新增
class DeviceConfigValidator {
public:
    static bool validate(const DeviceConfig& config, QStringList& errors);
    static bool validateDyn200(const DeviceConfig& config, QStringList& errors);
};
```

校验规则：
- `portName`: 非空（真实模式下）
- `baudRate`: ∈ {9600, 19200, 38400, 57600, 115200}
- `slaveId`: [1, 247]
- `parity`: "None" | "Even" | "Odd"
- `stopBits`: 1 | 2
- `communicationMode`: 0 | 1 | 2 | 3
- `timeout`: [100, 10000]
- `pollIntervalUs`: [1000, 100000]

**集成点**：
- `DeviceConfigService::saveDeviceConfig()` 保存前调用校验
- `ConfigLoader::loadStationConfig()` 加载后调用校验，错误写入日志
- `StationRuntimeFactory::create()` 创建设备前调用校验，提前失败

### 6.3 P1：TorquePoller线程安全

**方案**：最小修改

```cpp
// Dyn200ProactiveListener.h
std::atomic<double> m_latestTorque;
std::atomic<double> m_latestSpeed;
```

由于 `double` 的 `std::atomic` 在部分平台上需要支持（C++20通常支持），若编译器不支持，可使用 `QAtomicDouble` 或 `QReadWriteLock`。

### 6.4 P2：统一遥测数据来源

**方案**：让DiagnosticsViewModel使用AcquisitionScheduler

```cpp
// DiagnosticsViewModel::buildTorqueStatus() 修改建议
QVariantMap DiagnosticsViewModel::buildTorqueStatus() {
    if (!m_runtime || !m_runtime->acquisitionScheduler()) {
        return buildOfflineStatus("DYN200", "调度器未初始化");
    }
    auto snap = m_runtime->acquisitionScheduler()->snapshot();
    bool online = m_runtime->torque() && m_runtime->dyn200Bus() && m_runtime->dyn200Bus()->isOpen();
    return {
        {"name", "DYN200 扭矩传感器"},
        {"status", online ? "online" : "offline"},
        {"torqueNm", snap.dynTorqueNm},
        {"speedRpm", snap.dynSpeedRpm},
        {"powerW", snap.dynPowerW},
        {"lastUpdate", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")},
        {"summary", online ? QString("扭矩 %1 N·m / 转速 %2 RPM / 功率 %3 W")
                                    .arg(snap.dynTorqueNm, 0, 'f', 2)
                                    .arg(snap.dynSpeedRpm, 0, 'f', 0)
                                    .arg(snap.dynPowerW, 0, 'f', 1)
                           : QString("通信失败: %1").arg(humanizeError(m_runtime->torque()->lastError()))},
        {"errorCount", online ? 0 : 1}
    };
}
```

**注意**：需确保 `AcquisitionScheduler` 在DiagnosticsViewModel首次刷新前已启动（`StationRuntime::initialize()` Phase 3已保证）。

### 6.5 P2：运行时单设备热重配置

**方案**：差异比对+局部重建

```cpp
// RuntimeManager 新增
Q_INVOKABLE bool reconfigureDevice(const QString& deviceKey, const QVariantMap& newConfig);

// 实现逻辑：
// 1. 对比newConfig与当前StationConfig的差异
// 2. 若dyn200配置变化：
//    a. 停止AcquisitionScheduler
//    b. shutdown Dyn200TorqueSensorDevice（关闭Listener，释放串口）
//    c. 关闭dyn200Bus（ModbusRtuBusController）
//    d. 用新配置重新open bus
//    e. 重新创建Dyn200TorqueSensorDevice
//    f. 重新initialize
//    g. 重新设置到AcquisitionScheduler
//    h. 启动AcquisitionScheduler
// 3. 更新StationConfig并保存到JSON
```

**复杂度**：高。涉及运行时状态迁移、设备生命周期重入安全、QML属性重新绑定。建议作为独立迭代任务。

---

## 7. 跨角色协作建议

### 7.1 协议与设备开发 (`protocol-dev`)

**当前输入需求**：
- 实现 `Dyn200ProactiveListener` 的线程安全修复（原子变量或锁）
- 评估 `IProactiveFrameParser` 策略模式重构的可行性
- 为 `IBusController` 设计 `IProactiveDataSource` 抽象，消除 `underlyingSerialPort()` 泄漏

**约束条件**：
- DYN200主动上传模式的CRC算法（Modbus CRC16）已在 `calculateCRC16()` 中实现，重构时需保持兼容性
- 三种帧格式（Hex6/Hex8/ASCII）的解析逻辑需与设备手册完全一致

### 7.2 领域引擎开发 (`domain-dev`)

**当前输入需求**：
- `GearboxTestEngine` 的 `acquireTelemetry()` 已通过 `AcquisitionScheduler::snapshot()` 获取DYN200数据，无需修改
- 建议关注 `lockSpeedThresholdRpm` 和 `lockAngleDeltaDeg` 的配方配置合理性（当前默认值2.0 RPM和5.0°来自2026-04-17文档）

**约束条件**：
- TelemetrySnapshot中的DYN200字段（dynTorqueNm/dynSpeedRpm/dynPowerW）已被TestExecutionViewModel和报告生成器消费，修改字段名需同步更新多处

### 7.3 ViewModel开发 (`viewmodel-dev`)

**当前输入需求**：
- 统一DiagnosticsViewModel与TestExecutionViewModel的DYN200数据来源（均使用AcquisitionScheduler snapshot）
- 在保存DYN200配置时增加communicationMode变更确认对话框
- 考虑使用 `QPointer<StationRuntime>` 替代原始指针，增强生命周期安全

**约束条件**：
- `torqueTelemetry` Q_PROPERTY 的数据结构（{"torqueNm", "speedRpm", "powerW", "online"}）已被DiagnosticsPage.qml硬编码消费，修改结构需同步更新QML

### 7.4 UI/QML开发 (`ui-dev`)

**当前输入需求**：
- 在DeviceConfigPage的DYN200卡片中增加communicationMode警告标签
- 将baudRate、parity、stopBits等字段从自由输入改为下拉选择，减少配置错误
- 在连接测试按钮中增加DYN200专用测试逻辑（如依次测试Modbus通信、验证扭矩读取）

**约束条件**：
- `deviceConfig` 的JavaScript结构体字段名（dyn200.portName等）与C++层QVariantMap和JSON键必须保持一致

### 7.5 基础设施/服务开发 (`infra-dev`)

**当前输入需求**：
- 实现 `DeviceConfigValidator` 校验层，优先覆盖DYN200的9个配置字段
- 评估 `StationRuntime` 单设备热重配置的技术可行性
- 确保 `DeviceConfigService` 的save/load与 `ConfigLoader` 的字段映射持续一致（建议提取共享字段常量）

**约束条件**：
- station.json的文件路径在main.cpp中通过 `../../config/station.json` 相对路径定位，发布版需验证路径正确性

---

## 8. 结论

DYN200参数配置功能在架构层面**已正确集成**，具备生产使用的基础条件。项目采用了清晰的分层架构和抽象接口，模拟/真实双模式切换机制成熟，测试覆盖充分。

**核心优势**：
1. **接口隔离**：`ITorqueSensorDevice` / `IBusController` 抽象使DYN200实现与上层完全解耦
2. **工厂装配**：`StationRuntimeFactory` 将配置到运行时的装配逻辑集中管理
3. **双模式支持**：模拟设备 `SimulatedTorqueDevice` 与真实设备共享接口，测试覆盖度高
4. **多协议支持**：DYN200的4种通信模式均有实现，覆盖不同采集频率需求

**必须修复的高风险项（P0）**：
- **communicationMode切换陷阱**：当前允许用户通过UI将DYN200从主动上传模式切回Modbus RTU，但设备无法通过软件命令完成此切换，会导致通信完全中断且需物理按键恢复。建议立即在UI、ViewModel、设备层三层添加防护。

**建议优先实施的改进（P1-P2）**：
- 配置校验层（避免非法值导致运行时失败）
- TorquePoller线程安全修复（消除数据竞争隐患）
- 统一遥测数据来源（消除诊断页与测试执行页的数据不一致）

以上分析可作为代码审查和迭代规划的输入，各角色可根据优先级矩阵认领对应任务。

---

*报告生成时间：基于代码库当前HEAD状态*  
*分析涉及文件：~25个核心源文件 + CMakeLists.txt + 测试文件引用*
