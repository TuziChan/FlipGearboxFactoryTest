# Qt6 复刻 motor.html 的 MVVM 设计

本设计文档用于指导在 `F:\Work\产品出厂程序qt版` 中使用 Qt6 和 `Qt Quick/QML` 复刻 [motor.html](/F:/Work/产品出厂程序qt版/Docs/motor.html) 的“齿轮箱测试系统”页面，并将当前的前端演示稿升级为可切换“仿真模式 / 真实设备模式”的工业测试程序。目标不是只做一个外观相似的界面，而是构建一个能够管理多条 RS485 总线、在一台 PC 上协调多个从站设备、并用 MVVM 架构承载后续扩展的正式桌面程序。

## 目标

程序首版必须完成三件事。第一，高保真复刻 `motor.html` 的视觉结构和主要交互，包括三栏布局、步骤推进、指标卡片、图表、判定栏、状态栏和消息提示。第二，建立清晰的 MVVM 分层，使界面展示、测试流程、业务判定和设备通信相互隔离。第三，支持双运行模式：没有硬件时可在仿真模式下完整演示测试流程；接入真实硬件并配置好通信参数后，可切换到真实模式，从多条 RS485 总线上读取和控制实际设备。

## 范围

首版聚焦 `motor.html` 对应的“测试执行”页面，页面中的流程包括“准备/找零、空载正反转、角度定位、负载上升、回零结束”。首版同时建立顶部导航的基本骨架，但“配方管理、历史记录、设备配置、I/O 诊断”只需要占位页面或简单骨架，不在首版完成完整业务。

真实设备侧以 `Docs` 目录中的文档为依据，首版默认支持的设备类型和事实如下：

- [AQMD3610NS-A2.md](/F:/Work/产品出厂程序qt版/Docs/AQMD3610NS-A2.md)：直流驱动器，支持 `RS485 + Modbus RTU`，用于启停、正反转、速度/电流相关控制与状态读取。
- [DYN-200使用手册印刷V3.7版本中性(12)(3)_2025-08-02-13_23_22.md](/F:/Work/产品出厂程序qt版/Docs/DYN-200使用手册印刷V3.7版本中性(12)(3)_2025-08-02-13_23_22.md)：动态扭矩传感器，支持 `RS485`，并支持 `Modbus RTU` 或主动上传协议，首版优先按 `Modbus RTU` 集成。
- [单圈编码器.md](/F:/Work/产品出厂程序qt版/Docs/单圈编码器.md)：单圈编码器，支持 `RS485 + Modbus RTU`，适合读取绝对角度、角速度和置零。
- [双通道数控电源用户手册.md](/F:/Work/产品出厂程序qt版/Docs/双通道数控电源用户手册.md)：双通道数控电源，支持 `RS485 + Modbus RTU`，可设置电压/电流并读取回读值。

## 总体方案

采用 `QML + Qt Quick` 构建界面，采用 `C++` 承载 ViewModel、流程引擎、总线管理和设备驱动适配层。首版按“一台 PC 管理多条 RS485 总线，每条总线一主多从”的拓扑设计。系统内部必须抽象“总线控制器”和“设备能力接口”，这样首版先支持单主多从，后续如需扩展成更复杂的多主站协调，只需在基础设施层增加协调器，不必推翻页面层与流程层。

推荐的总体分层如下：

- `View`：QML 页面和组件，只做显示与交互绑定。
- `ViewModel`：`QObject` 属性、命令、列表模型，给 QML 提供直接可用的数据。
- `Domain`：测试流程定义、判定规则、测试结果、报告数据、工位配置等纯业务对象。
- `Infrastructure`：串口、Modbus RTU、总线轮询、设备适配器、仿真实现。

## 项目结构

建议工程结构如下：

    F:\Work\产品出厂程序qt版
    ├─ CMakeLists.txt
    ├─ src
    │  ├─ app
    │  │  ├─ main.cpp
    │  │  ├─ ApplicationBootstrapper.h
    │  │  └─ ApplicationBootstrapper.cpp
    │  ├─ ui
    │  │  ├─ pages
    │  │  │  ├─ TestExecutionPage.qml
    │  │  │  ├─ RecipePage.qml
    │  │  │  ├─ HistoryPage.qml
    │  │  │  ├─ DeviceConfigPage.qml
    │  │  │  └─ IoDiagnosticsPage.qml
    │  │  └─ components
    │  │     ├─ TitleBar.qml
    │  │     ├─ NavRail.qml
    │  │     ├─ CommandBar.qml
    │  │     ├─ StepListPanel.qml
    │  │     ├─ MetricsGrid.qml
    │  │     ├─ RealtimeChartPanel.qml
    │  │     ├─ PhaseTableAngle.qml
    │  │     ├─ PhaseTableLoad.qml
    │  │     ├─ VerdictPanel.qml
    │  │     └─ StatusBar.qml
    │  ├─ viewmodels
    │  │  ├─ MainViewModel.h/.cpp
    │  │  ├─ TestExecutionViewModel.h/.cpp
    │  │  ├─ CommandBarViewModel.h/.cpp
    │  │  ├─ StepListViewModel.h/.cpp
    │  │  ├─ MetricsViewModel.h/.cpp
    │  │  ├─ ChartViewModel.h/.cpp
    │  │  ├─ JudgmentViewModel.h/.cpp
    │  │  └─ DeviceStatusViewModel.h/.cpp
    │  ├─ domain
    │  │  ├─ TestPhase.h
    │  │  ├─ TestResult.h
    │  │  ├─ StationConfig.h
    │  │  ├─ LimitsProfile.h
    │  │  ├─ TestSequenceEngine.h/.cpp
    │  │  └─ ReportBuilder.h/.cpp
    │  └─ infrastructure
    │     ├─ bus
    │     │  ├─ IBusController.h
    │     │  ├─ ModbusRtuBusController.h/.cpp
    │     │  └─ BusManager.h/.cpp
    │     ├─ devices
    │     │  ├─ IMotorDriveDevice.h
    │     │  ├─ ITorqueSensorDevice.h
    │     │  ├─ IEncoderDevice.h
    │     │  ├─ IPowerSupplyDevice.h
    │     │  ├─ AqmdMotorDriveAdapter.h/.cpp
    │     │  ├─ Dyn200TorqueSensorAdapter.h/.cpp
    │     │  ├─ BriterEncoderAdapter.h/.cpp
    │     │  └─ DualPowerSupplyAdapter.h/.cpp
    │     ├─ simulation
    │     │  ├─ SimulatedMotorDriveDevice.h/.cpp
    │     │  ├─ SimulatedTorqueSensorDevice.h/.cpp
    │     │  ├─ SimulatedEncoderDevice.h/.cpp
    │     │  └─ SimulatedPowerSupplyDevice.h/.cpp
    │     └─ config
    │        ├─ ConfigLoader.h/.cpp
    │        └─ StationRuntimeFactory.h/.cpp
    └─ config
       ├─ app.json
       └─ stations.json

## 页面与组件设计

`motor.html` 不应被翻译成一个巨大的 QML 文件。主页面 `TestExecutionPage.qml` 只负责三栏总布局和区域装配，内部拆成独立组件。每个组件通过绑定某个 ViewModel 或子 ViewModel 渲染，不直接包含设备通信逻辑。

页面拆分如下：

- `TitleBar.qml`：显示系统名、工位号、总线在线状态。
- `NavRail.qml`：左侧导航，首版仅“测试执行”页完整，其它页保留骨架。
- `CommandBar.qml`：型号、SN、回差补偿、开始测试、急停。
- `StepListPanel.qml`：阶段列表、状态颜色、阶段耗时和总耗时。
- `MetricsGrid.qml`：转速、扭矩、功率、电机电流、制动电流、角度等卡片。
- `PhaseTableAngle.qml`：角度定位阶段的明细表格。
- `PhaseTableLoad.qml`：负载上升阶段的明细表格。
- `RealtimeChartPanel.qml`：实时波形、多通道开关、采样条。
- `VerdictPanel.qml`：整机判定、分项判定、传感器状态、复制报告、重置。
- `StatusBar.qml`：底部设备在线、时钟、心跳状态。

## ViewModel 划分

ViewModel 必须按“页面聚合 + 子模型”方式设计，避免把所有逻辑堆进一个类里。

- `MainViewModel`
  - 管理全局导航、当前页面、工位信息、当前运行模式、系统时钟、总线汇总状态。
- `TestExecutionViewModel`
  - 当前页面的核心入口。
  - 暴露 `startTest()`、`emergencyStop()`、`resetTest()`、`copyReport()` 等命令。
  - 聚合命令栏、步骤、指标、图表、判定等子 ViewModel。
- `CommandBarViewModel`
  - 提供型号列表、选中型号、SN、回差补偿、表单锁定状态、输入校验错误。
- `StepListViewModel`
  - 提供阶段列表、当前阶段、阶段状态、阶段耗时。
- `MetricsViewModel`
  - 提供各指标卡片的显示值、单位、来源说明和格式化文本。
- `ChartViewModel`
  - 提供图表点列、当前可见通道、采样进度和滚动窗口。
- `JudgmentViewModel`
  - 提供整机结论、分项判定、AI1/AI2 状态和报告文本。
- `DeviceStatusViewModel`
  - 提供多总线、多设备在线状态摘要和诊断信息。

QML 层禁止做以下事情：直接访问串口、直接做 Modbus 读写、直接计算测试判定、直接生成报告文本。QML 只允许绑定属性、调用命令和做少量视觉层状态切换。

## 测试流程引擎

测试流程不沿用 `motor.html` 中前端定时器驱动的方式，而是由 `TestSequenceEngine` 在 `Domain` 层统一调度。该引擎必须把整个流程拆成可验证的阶段状态机，每个阶段都有明确的输入、前置条件、采样时段、超时、结果和失败原因。

首版阶段固定为：

1. 准备/找零
2. 空载正反转
3. 角度定位
4. 负载上升
5. 回零结束

每个阶段输出：

- 阶段状态：`wait / running / done / fail`
- 起止时间与耗时
- 实时指标快照
- 阶段明细结果
- 失败原因，区分通信失败、流程失败、判定失败

`TestSequenceEngine` 只依赖抽象设备能力接口，不直接依赖串口对象。这样同一套流程既能跑仿真设备，也能跑真实设备。

## 双模式设计

系统必须支持同一套页面和同一套流程入口下的双模式切换：

- 仿真模式
  - 使用 `simulation` 目录下的模拟设备实现。
  - 没有硬件时也可以完整演示测试流程、图表刷新、步骤推进与判定。
- 真实模式
  - 使用 `bus` 与 `devices` 目录下的 Modbus RTU 设备适配器。
  - 从真实总线轮询读取数据，并向设备写入控制命令。

切换规则如下：

- 应用启动时从 `config/app.json` 读取默认模式。
- 页面上可以展示当前模式，但不允许 QML 自己决定创建哪种设备对象。
- `ApplicationBootstrapper` 或 `StationRuntimeFactory` 负责根据配置创建仿真或真实运行时。
- `TestExecutionViewModel` 对当前模式透明，不应包含 `if (simulation) ... else ...` 这样的分支扩散。

## 多总线与工位模型

系统首版必须支持“一台 PC 同时管理多条 RS485 总线，每条总线一主多从”。实现上引入三个概念：

- 总线 `Bus`
  - 一条物理串口线路，例如 `COM3`、`COM4`。
  - 每条总线由一个 `IBusController` 实例管理。
- 从站设备 `Device`
  - 某条总线上的一个 Modbus 从站，包含设备类型、站号、超时、轮询周期。
- 工位运行时 `StationRuntime`
  - 一个业务工位的完整设备组合，不直接暴露“串口”，只暴露“电机驱动器、扭矩传感器、编码器、电源”等能力接口。

首版推荐的 `stations.json` 配置思想如下：

    {
      "stations": [
        {
          "id": "station-03",
          "displayName": "工位 #03",
          "mode": "real",
          "devices": {
            "motorDrive": { "bus": "bus-a", "address": 1, "type": "aqmd3610ns-a2" },
            "torqueSensor": { "bus": "bus-a", "address": 2, "type": "dyn200" },
            "encoder": { "bus": "bus-b", "address": 1, "type": "briter-single-turn" },
            "powerSupply": { "bus": "bus-c", "address": 3, "type": "sydp-24x2-240-p4-iso" }
          }
        }
      ]
    }

## 设备能力接口

设备抽象按“能力”而不是“页面控件”拆分。首版至少需要以下接口：

    class IMotorDriveDevice {
    public:
        virtual ~IMotorDriveDevice() = default;
        virtual Result startForward(int rpm) = 0;
        virtual Result startReverse(int rpm) = 0;
        virtual Result stop() = 0;
        virtual Result brake() = 0;
        virtual MotorDriveTelemetry readTelemetry() = 0;
    };

    class ITorqueSensorDevice {
    public:
        virtual ~ITorqueSensorDevice() = default;
        virtual Result<TorqueTelemetry> readTelemetry() = 0;
        virtual Result zero() = 0;
    };

    class IEncoderDevice {
    public:
        virtual ~IEncoderDevice() = default;
        virtual Result<EncoderTelemetry> readTelemetry() = 0;
        virtual Result zero() = 0;
    };

    class IPowerSupplyDevice {
    public:
        virtual ~IPowerSupplyDevice() = default;
        virtual Result setChannelVoltage(int channel, double volts) = 0;
        virtual Result setChannelCurrent(int channel, double amps) = 0;
        virtual Result setChannelEnabled(int channel, bool enabled) = 0;
        virtual Result<PowerSupplyTelemetry> readTelemetry() = 0;
    };

以上签名是方向性约束，具体 `Result`、`Telemetry` 类型名可在实现时细化，但工程中最终必须存在与这些职责等价的接口。

## 真实设备集成策略

根据 `Docs` 中的信息，首版统一按 `Modbus RTU` 接入。理由如下：

- 编码器、电源、AQMD 驱动器均明确支持 Modbus RTU。
- DYN-200 既支持主动上传也支持 Modbus RTU；首版优先选 Modbus RTU，减少协议分叉。
- 首版目标是先打通完整流程，尽量减少混合协议带来的时序复杂度。

总线层实现要点：

- 使用 `QSerialPort` 打开串口。
- 自行实现或封装 `Modbus RTU` 报文读写、CRC 校验、超时重试、轮询节流。
- 每条总线维护自己的命令队列和轮询循环，避免不同串口互相阻塞。
- 设备适配器只关心寄存器映射，不关心串口调度细节。
- 真实模式下通信错误必须被分类和上传，不得静默吞掉。

## 判定与错误分类

现场最容易混淆的问题是“产品 NG”和“通信挂了”。因此首版必须把失败原因分为三类：

- `通信失败`
  - 超时、CRC 错误、串口断开、站号无响应
- `流程失败`
  - 阶段前置条件不满足、动作未完成、阶段超时、采样不足
- `判定失败`
  - 采集到数据，但数据超出上下限，最终判为 `NG`

界面中需要明确显示是哪一类失败。总判定区域不能只显示红色结果而不解释原因。

## 配置文件

首版使用本地 JSON 配置即可，不引入数据库。

- `config/app.json`
  - 默认运行模式
  - 默认工位
  - 日志级别
  - UI 分辨率偏好或窗口恢复信息
- `config/stations.json`
  - 总线列表
  - 工位与设备映射
  - 设备站号
  - 轮询周期
  - 通信超时

配置必须做到“改配置即可换串口、站号和工位组合”，不依赖重新编译。

## 多分辨率支持

多分辨率支持是首版硬要求，不允许在实现后期再补。页面不能按 HTML 固定像素照抄，必须按 `1920x1080` 为设计基准构建自适应布局。

首版重点验证分辨率为：

- `1366x768`
- `1600x900`
- `1920x1080`
- `2560x1440`

设计原则如下：

- 主布局优先使用 `RowLayout`、`ColumnLayout`、`GridLayout` 和比例宽度，而不是把控件位置写死。
- 左栏和右栏可以设置合理的最小宽度和最大宽度，但不能固定死导致低分辨率挤压。
- 字号、间距、卡片高度使用分级缩放策略，而不是整页线性拉伸。
- 图表和表格优先保证可读性；在高度不足的分辨率下，左右栏允许内部滚动。
- 顶部命令栏、总判定、底部状态栏应保持稳定可见，不应因窗口缩小而被挤掉。
- 低于 `1366x768` 的分辨率不承诺最佳体验，但程序仍必须可用，不得出现核心控件遮挡或无法操作。

## 视觉复刻原则

首版视觉风格对齐 `motor.html`，但不要求像素级复制浏览器默认渲染。必须保留以下视觉特征：

- 浅色工业界面
- 蓝色强调色与状态色语义
- 三栏布局和圆角卡片
- 流程步骤状态色
- 采样进度条
- 整机结论大标签
- 底部设备在线状态点

字体、图标、阴影和动画节奏以 Qt 环境下可稳定复现为准，不为了还原浏览器细节而牺牲可维护性。

## 验收标准

首版完成后，应能用以下方式验证：

1. 在无硬件环境下启动程序，进入仿真模式。
2. 打开“测试执行”页面，看到布局与 `motor.html` 结构一致。
3. 输入型号、SN、回差补偿，点击开始测试。
4. 观察左侧步骤推进、中间指标刷新、图表滚动、角度表/负载表切换、右侧总判定变化。
5. 测试结束后可复制报告并重置测试。
6. 切换到真实模式并配置有效串口、站号后，程序能识别多条 RS485 总线上的设备在线状态。
7. 真实测试时，数据来自实际设备；若某条总线或某个从站故障，界面明确显示通信失败。
8. 在 `1366x768`、`1600x900`、`1920x1080`、`2560x1440` 四档分辨率下，界面无关键控件遮挡，流程可完整操作。

## 非首版内容

以下内容不纳入首版完整实现，只保留架构接口或骨架页面：

- 完整配方管理
- 历史记录持久化与查询
- 完整 I/O 诊断工具
- 权限系统
- 多工位并发调度
- 真正的多主站同总线冲突协调

## 设计结论

最终采用的方案是“分层 MVVM + 总线管理器 + 设备适配器”。这是三种候选方案中最适合当前任务的路径，因为它既能高保真复刻 `motor.html` 的 UI，又能承接多总线、多从站、双模式和后续设备扩展。相比把全部逻辑堆在单一 ViewModel 中，它更利于长期维护；相比把流程逻辑塞进 QML/JavaScript，它更符合工业测试程序对可测试性、稳定性和通信边界的要求。

## 变更记录

2026-04-15：初版建立。根据当前仓库仅含 `Docs` 文档的现状，明确首版采用 `Qt Quick/QML + C++ MVVM`，默认真实通信协议统一按 `RS485 + Modbus RTU` 接入，并把“一台 PC 管理多条 RS485 总线、每条总线一主多从、支持仿真/真实双模式、支持多分辨率”写入正式设计范围。
