# 真实设备接入与齿轮箱测试流程落地设计

本文档把当前仓库从“Qt/QML 界面骨架”推进到“可控制真实设备完成齿轮箱出厂测试”的设计方案固定下来。它建立在现有设计文档 [2026-04-15-motor-qt6-mvvm-design.md](/F:/Work/FlipGearboxFactoryTest/Docs/superpowers/specs/2026-04-15-motor-qt6-mvvm-design.md) 之上，但不假设读者了解那份文档的细节；本文会把本次实现必须知道的硬件事实、测试口径、模块边界和异常规则完整写清楚。

目标不是只把模拟页面替换成真实串口读写，而是让同一套 Qt 程序具备以下能力：第一，按既定工艺驱动 AQMD、电磁粉末制动器、DYN200 和单圈编码器完成整套测试；第二，把“通信失败”“流程失败”“判定失败”清楚地区分开；第三，把所有关键阈值和限值放进可配置的配方或工位配置，而不是散落在 QML 或临时脚本里。

## 用户可见结果

完成后，操作员在“测试执行”页输入产品信息并点击开始，程序会先自动找零，再做空载正反转、角度定位、负载锁止和最终回零。界面中部实时显示转速、扭矩、功率、电机电流、制动电流和编码器角度；左侧显示阶段推进；右侧显示分项判定与总判定。若某个设备掉线或寄存器读写失败，界面明确标出通信失败；若设备工作正常但角度或扭矩超限，则标出判定失败。

## 当前仓库上下文

当前仓库 `F:\Work\FlipGearboxFactoryTest` 已有 Qt 6 + QML 工程骨架，主入口为 [main.cpp](/F:/Work/FlipGearboxFactoryTest/main.cpp) 和 [Main.qml](/F:/Work/FlipGearboxFactoryTest/Main.qml)。页面层位于 `src/ui/pages` 与 `src/ui/components`，其中 [TestExecutionPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/TestExecutionPage.qml) 仍以定时器和假数据驱动演示流程，[TestExecutionWorkspace.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/TestExecutionWorkspace.qml) 负责测试页主体布局。当前仓库还没有真正的 `domain`、`infrastructure`、`viewmodels` 目录，因此本次设计会新增这些层。

仓库 `Docs` 目录下已经收录本次接入所需的设备手册：

- [AQMD3610NS-A2.md](/F:/Work/FlipGearboxFactoryTest/Docs/AQMD3610NS-A2.md)
- [DYN-200使用手册印刷V3.7版本中性(12)(3)_2025-08-02-13_23_22.md](/F:/Work/FlipGearboxFactoryTest/Docs/DYN-200使用手册印刷V3.7版本中性(12)(3)_2025-08-02-13_23_22.md)
- [单圈编码器.md](/F:/Work/FlipGearboxFactoryTest/Docs/单圈编码器.md)
- [双通道数控电源用户手册.md](/F:/Work/FlipGearboxFactoryTest/Docs/双通道数控电源用户手册.md)

本文档中的寄存器规划和设备职责必须以这些仓库内文档为准。

## 硬件事实与正式测试口径

本次设计采用以下已经确认的现场事实，这些事实在实现和测试中不得擅自改动。

AQMD 驱动器由上位机通过 `RS485/Modbus RTU` 直接控制启停、方向和占空比，不依赖 PLC 或额外外部控制器完成主流程动作。AQMD 同时承担“电机电流读取”和“磁铁事件输入读取”两项职责。

减速器输出齿上有 3 个磁铁，使用单个感应器接到 AQMD 的 `AI1`。感应到磁铁时输出低电平，没有感应到磁铁时输出高电平。正式测试只认 `AI1` 的“高到低跳变”，也就是 falling edge。只要 `AI1` 一直保持低电平，不允许重复视为新的磁铁命中事件。

位置定义采用以下规则。电机沿测试方向依次遇到 3 个磁铁事件，`AI1` 的第 1、2、3 次高到低跳变分别对应位置①、位置②、位置③。角度判定并不是“到某个编码器角度就算到位”，而是“在 `AI1` 读到高到低的那一刻，立即锁存当前编码器角度，并用该角度判定”。

零点存在固定机械关系。磁铁相对于减速器轴真正零点有 `3°` 偏差，因此正式找零流程为：先低占空比驱动减速器，直到 `AI1` 命中磁铁事件，此位置视为“机械零点前 3°”；随后保持同一方向继续走到配方配置的“编码器绝对角度设定零点值”。这里的回差补偿只在初始找零阶段做一次，后续角度测试不再重复消隙。

第一步空载测试的正式口径为：电机先运行 `3 s`，再读取随后 `2 s` 内的空载电流与空载转速。电流由 AQMD 提供，转速由 DYN200 提供。每个方向都同时记录平均值和最大值，并都要参与判定。正反转分别独立判定。

第二步角度测试的正式顺序为：`① -> ② -> 回到① -> ③ -> 回到零点`。位置①、位置②、位置③的目标角度分别为 `3° ± 3°`、`49° ± 3°`、`113.5° ± 3°`。位置①要记录两次，一次来自初次前进命中，一次来自从②回到①后的再次命中，这两次都属于正式结果。

第三步负载测试的正式口径为：电机先运行 `3 s`，随后在 `2 s` 内逐步提高磁粉制动器电流，直到齿轮箱被锁住。锁止成立必须同时满足两条条件：第一，DYN200 转速绝对值 `<= 2 RPM`；第二，连续 `100 ms` 内编码器角度变化 `<= 5°`，并且这种状态持续 `500 ms`。一旦锁止成立，立即读取当时的制动电流值和扭矩值，正反转都要做并按限值判定。若 2 秒爬升结束仍未锁止，记为流程失败而不是判定失败。

最后一步正式口径为：正转驱动电机回到编码器设定零点值，作为测试收尾动作。

## 总体架构

本次设计继续沿用 Qt/QML + C++ 的 MVVM 分层，但将“真实设备接入”和“测试流程状态机”落到清晰的层级里。

- `src/ui`
  继续保留纯展示层。QML 只能绑定 ViewModel 属性和触发命令，不允许直接访问串口、直接计算判定、直接决定状态机跳转。
- `src/viewmodels`
  把 domain 层产出的运行状态投影成页面数据。它要替换掉 [TestExecutionPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/TestExecutionPage.qml) 里当前那套本地定时器模拟逻辑。
- `src/domain`
  实现真实的测试流程引擎、采样窗口、事件锁存、判定和报告对象。它只依赖设备抽象接口，不依赖具体串口实现。
- `src/infrastructure`
  实现串口、Modbus RTU、设备寄存器适配器、配置解析和运行时装配。它负责把“寄存器与电平”转换成 domain 可消费的设备能力。

## 目录规划

建议新增如下目录和核心文件。现有 QML 文件继续保留，只在接线时替换绑定来源。

    src/
      domain/
        GearboxTestEngine.h
        GearboxTestEngine.cpp
        TestRecipe.h
        TestRunState.h
        TestResults.h
        TelemetrySnapshot.h
        FailureReason.h
        ReportBuilder.h
        ReportBuilder.cpp
      infrastructure/
        bus/
          IBusController.h
          ModbusRtuBusController.h
          ModbusRtuBusController.cpp
          ModbusFrame.h
          ModbusFrame.cpp
          BusManager.h
          BusManager.cpp
        devices/
          IMotorDriveDevice.h
          ITorqueSensorDevice.h
          IEncoderDevice.h
          IBrakePowerDevice.h
          AqmdMotorDriveDevice.h
          AqmdMotorDriveDevice.cpp
          Dyn200TorqueSensorDevice.h
          Dyn200TorqueSensorDevice.cpp
          SingleTurnEncoderDevice.h
          SingleTurnEncoderDevice.cpp
          BrakePowerSupplyDevice.h
          BrakePowerSupplyDevice.cpp
        config/
          AppConfig.h
          StationConfig.h
          RecipeConfig.h
          ConfigLoader.h
          ConfigLoader.cpp
          StationRuntime.h
          StationRuntimeFactory.h
          StationRuntimeFactory.cpp
      viewmodels/
        TestExecutionViewModel.h
        TestExecutionViewModel.cpp
        CommandBarViewModel.h
        CommandBarViewModel.cpp
        StepListViewModel.h
        StepListViewModel.cpp
        MetricsViewModel.h
        MetricsViewModel.cpp
        AngleResultsViewModel.h
        AngleResultsViewModel.cpp
        LoadResultsViewModel.h
        LoadResultsViewModel.cpp
        VerdictViewModel.h
        VerdictViewModel.cpp

## 设备职责与接口边界

### AQMD 驱动器

AQMD 对本项目不仅是“驱动器”，还是磁铁事件来源，因此它需要暴露以下能力：

- 设定方向：正转 / 反转
- 设定占空比
- 启动 / 停止 / 刹车
- 读取电机电流
- 读取 `AI1` 当前电平
- 生成 `AI1` 高到低命中事件

这里的“事件”不能靠 QML 推导，必须在 C++ 层按轮询前后状态比较生成，否则同一磁铁会被重复消费。

### DYN200 扭矩传感器

DYN200 需要至少暴露三项遥测：

- 转速 RPM
- 扭矩 N·m
- 功率 W

本项目默认按 `Modbus RTU` 集成，不采用主动上传协议。这样总线层可以统一调度，也更符合当前工程的单主站控制方式。

### 单圈编码器

编码器主要职责是：

- 提供绝对角度
- 支持设置零点或写入当前位置为零点的操作（若工艺最终采用“只用绝对角度不改内部零点”，该能力仍建议保留）

本项目实际判定依据是“磁铁事件时刻锁存的编码器角度”，因此编码器读数必须具有稳定的时间戳和统一的数据缩放规则。

### 数控电源 / 磁粉制动器

数控电源负责：

- 按通道设定输出电流
- 打开或关闭输出
- 回读实际电流、电压、功率

虽然现场对象是“磁粉制动器”，但从软件接口上看，它是“可控电流负载源”，因此本项目在设备层用 `IBrakePowerDevice` 建模更准确。

## 流程引擎设计

核心流程对象命名建议为 `GearboxTestEngine`。它是一个状态机，负责根据设备遥测和时间推进整个测试。

### 主阶段

主阶段固定为：

1. `PrepareAndHome`
2. `IdleRun`
3. `AnglePositioning`
4. `LoadRampAndLock`
5. `ReturnToZero`
6. `Completed` 或 `Failed`

### 关键子状态

为了避免一个阶段内部塞满隐含逻辑，每个主阶段再拆成明确子状态。

找零阶段：

- `SeekingMagnet`
- `AdvancingToEncoderZero`
- `HomeSettled`

空载阶段：

- `SpinupForward`
- `SampleForward`
- `SpinupReverse`
- `SampleReverse`

角度阶段：

- `MoveToPosition1`
- `HoldAfterPosition1`
- `MoveToPosition2`
- `HoldAfterPosition2`
- `MoveBackToPosition1`
- `HoldAfterPosition1Return`
- `MoveToPosition3`
- `HoldAfterPosition3`
- `MoveBackToZero`

负载阶段：

- `SpinupLoadForward`
- `RampBrakeForward`
- `ConfirmLockForward`
- `SpinupLoadReverse`
- `RampBrakeReverse`
- `ConfirmLockReverse`

回零阶段：

- `ReturnFinalZero`
- `FinalZeroSettled`

### 统一遥测输入

引擎每个周期接收一份 `TelemetrySnapshot`，至少包含以下字段：

- `motorCurrentA`
- `aqmdAi1Level`
- `dynSpeedRpm`
- `dynTorqueNm`
- `dynPowerW`
- `encoderAngleDeg`
- `brakeCurrentA`
- `timestamp`

引擎输出一份 `TestRunState`，供 ViewModel 与日志系统使用。该状态至少包含：

- 当前主阶段
- 当前子状态
- 当前方向
- 运行耗时
- 关键实时指标
- 分项中间结果
- 最终总判定
- 失败类别
- 失败原因文本

## 每个测试步骤的正式行为

### 1. 找零

程序开始测试后，首先做设备在线预检。预检通过后，AQMD 以配方中的“找零低占空比”正转运行。流程引擎监控 `AI1` 电平，只要检测到高到低跳变，就记下“磁铁命中”。命中后不反向、不刹停，而是沿当前方向继续前进，直到编码器角度到达配方中的“编码器绝对角度设定零点值”。此时停止 AQMD，零位建立完成。

这里的回差补偿通过“先命中磁铁，再同向走到编码器零点”的单次动作消化。之后所有角度测试与最终回零都以这个零位为基准，不再在每个位置重复做越位或反向逼近。

### 2. 空载正反转

引擎切到正转，AQMD 按配方占空比运行 `3 s` 预热，然后进入 `2 s` 采样窗。采样窗中持续读取 AQMD 电流与 DYN200 转速，保留整窗样本。结束时计算：

- 正转空载电流平均值
- 正转空载电流最大值
- 正转空载转速平均值
- 正转空载转速最大值

并分别与配方限值比较。

随后引擎切到反转，重复相同动作，得出四个反转空载结果。

### 3. 角度定位

角度定位阶段不以“接近目标角度即停”作为命中，而是以磁铁事件为正式命中条件。每次等待命中时，引擎记录进入该子状态时的 `AI1` 电平，后续只有检测到新的高到低跳变时，才视为命中目标位置。在同一周期内锁存当前编码器角度，并立刻计算偏差与判定。

执行顺序固定为：

1. 首次命中位置①，锁存角度，应满足 `3° ± 3°`
2. 下一次命中位置②，锁存角度，应满足 `49° ± 3°`
3. 回程再次命中位置①，锁存角度，应满足 `3° ± 3°`
4. 再次前进命中位置③，锁存角度，应满足 `113.5° ± 3°`
5. 最后回到编码器设定零点

所有命中结果都进入正式结果表，不允许只保留最后一次。

### 4. 负载上升与锁止

负载测试先让 AQMD 在当前方向运行 `3 s`，再打开制动通道输出，并在接下来的 `2 s` 内按设定步长或斜率逐步提高电流。引擎持续轮询 DYN200 转速、编码器角度和制动电流。

锁止成立条件是同时满足：

- `abs(speedRpm) <= 2`
- 最近连续 `100 ms` 的编码器角度变化 `<= 5°`
- 上述两项状态持续 `500 ms`

一旦成立，立即冻结以下值：

- 当前制动电流
- 当前扭矩

然后与正转或反转的锁止配方限值比较，得到正式判定。

若 `2 s` 爬升时间结束仍未建立锁止条件，记为流程失败。此时不能因为“扭矩不够大”而直接归类为判定失败，因为正式锁止时刻根本没有形成。

### 5. 最终回零

所有正反转分项完成后，AQMD 正转回到编码器设定零点值。成功到位则流程完成并生成报告；若超时或通信中断则记为失败。

## 失败分类

本项目必须把失败类型清楚分成三类，因为这是现场排障和质量判责的核心。

### 通信失败

以下情况归为通信失败：

- 串口打开失败
- Modbus 帧 CRC 错误
- 设备超时无响应
- 站号配置错误导致目标从站不可达
- 某阶段中途设备掉线

通信失败意味着“软件没有拿到可信数据”，因此不能把结果算作产品 NG。

### 流程失败

以下情况归为流程失败：

- 找零阶段在超时内未等到磁铁事件
- 已命中磁铁但未能走到编码器零点
- 空载采样窗样本不足
- 角度阶段等待目标磁铁事件超时
- 负载爬升结束仍未锁止
- 最终回零超时

流程失败意味着硬件或控制动作没有把测试带到可判定状态。

### 判定失败

以下情况归为判定失败：

- 空载平均值或最大值超出配方范围
- 任一角度命中值超出容差
- 锁止时制动电流或扭矩超出配方范围

判定失败意味着数据可信，但产品或工艺结果不合格。

## 配置模型

### 配方配置

配方应至少包含以下项目。

找零相关：

- `homeDutyCycle`
- `homeAdvanceDutyCycle`
- `encoderZeroAngleDeg`
- `homeTimeoutMs`

空载相关：

- `idleForwardSpinupMs`
- `idleForwardSampleMs`
- `idleReverseSpinupMs`
- `idleReverseSampleMs`
- 正转空载电流平均值上下限
- 正转空载电流最大值上下限
- 正转空载转速平均值上下限
- 正转空载转速最大值上下限
- 反转同类四组限值

角度相关：

- `angleTestDutyCycle`
- `position1TargetDeg = 3.0`
- `position1ToleranceDeg = 3.0`
- `position2TargetDeg = 49.0`
- `position2ToleranceDeg = 3.0`
- `position3TargetDeg = 113.5`
- `position3ToleranceDeg = 3.0`
- `returnZeroToleranceDeg`
- 各角度动作超时

负载相关：

- `loadSpinupMs = 3000`
- `loadRampMs = 2000`
- `lockSpeedThresholdRpm = 2`
- `lockAngleWindowMs = 100`
- `lockAngleDeltaDeg = 5`
- `lockHoldMs = 500`
- `brakeRampStartCurrentA`
- `brakeRampEndCurrentA` 或 `brakeRampStepA`
- 正转锁止电流上下限
- 正转锁止扭矩上下限
- 反转锁止电流上下限
- 反转锁止扭矩上下限

### 工位 / 通信配置

工位配置至少包含：

- AQMD、DYN200、编码器、数控电源各自所在总线
- 站号
- 波特率
- 校验位 / 停止位
- 超时
- 轮询周期
- 数控电源使用的通道编号

## ViewModel 与页面联动

页面结构可继续沿用当前 [TestExecutionPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/TestExecutionPage.qml) 与 [TestExecutionWorkspace.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/TestExecutionWorkspace.qml) 的三栏布局，但数据必须改为绑定真实 ViewModel。

命令栏建议正式提供：

- 配方选择
- SN
- 编码器设定零点显示或编辑
- 开始测试
- 急停
- 复位

指标区建议正式显示：

- AQMD 电流
- DYN200 转速
- DYN200 扭矩
- DYN200 功率
- 编码器角度
- 制动电流
- `AI1` 状态

角度表正式列建议为：

- 目标位
- 目标角度
- 锁存角度
- 偏差
- 容差
- 判定

负载表正式列建议为：

- 方向
- 锁止制动电流
- 锁止扭矩
- 电流限值
- 扭矩限值
- 判定

右侧判定区不仅显示 `OK / NG`，还应单独显示失败类别和失败原因。

## 报告与日志

报告至少包含：

- 时间、SN、配方名、工位
- 找零结果与磁铁命中时间点
- 编码器设定零点值
- 空载正反转平均值、最大值、限值、判定
- 角度定位五个结果项
- 正反转锁止电流与扭矩结果
- 总判定
- 若失败，失败类别与失败原因

日志系统至少要记录：

- 每次设备命令下发
- 每次 `AI1` 命中事件
- 每次阶段跳转
- 每次正式结果锁存
- 失败发生时的原始上下文快照

## 测试策略

本次实现必须先写测试再写正式代码。推荐的测试层次如下：

1. domain 测试
   验证找零状态机、空载采样窗口、磁铁事件锁存、锁止判定逻辑、失败分类和总判定规则。
2. infrastructure 测试
   验证 Modbus CRC、帧编码、AQMD/DYN200/编码器/数控电源寄存器映射和缩放。
3. ViewModel 测试
   验证 UI 能正确投影阶段、指标、分项结果和失败类型。
4. UI smoke 测试
   验证测试页仍可正常加载，且关键区域与正式 ViewModel 连通。

## 设计结论

本次真实设备接入不应通过在 QML 页面里追加串口调用来完成，而应通过“设备适配层 + 流程引擎 + 配方/结果模型”的正式结构落地。这样可以把磁铁事件、空载采样、角度锁存和负载锁止这些容易出错的逻辑集中在可测试的 C++ 层，同时保留现有 QML 页面作为稳定的操作界面。

对现场最重要的三个口径已经固定：第一，正式角度判定以 `AI1` 高到低沿那一刻的编码器角度为准；第二，回差补偿只在初始找零阶段执行一次；第三，负载测试的“锁止”必须同时满足转速阈值和角度变化阈值，并保持足够时间。后续实现、测试和验收都必须以这三个口径为中心。



## 变更记录

2026-04-17：初版建立。根据当前仓库已有 Qt/QML UI 骨架和仓库内设备手册，固化 AQMD、DYN200、单圈编码器、数控电源的职责分工，写明正式测试工艺、事件口径、失败分类、模块边界和配置模型，用于后续 ExecPlan 编写与真实实现。

2026-04-17：补充完善。新增设备寄存器映射表、通信调度与轮询策略、事件检测与去抖动机制、超时值定义、制动电流爬升策略、异常恢复与重试策略、数据采样与滤波、ViewModel 更新策略、性能指标与时序要求等 8 个关键章节，覆盖所有实现细节。

2026-04-17：**重大修正**。发现补充章节中的设备寄存器映射表与仓库手册严重不符（P0 级别错误），已创建独立修正文档 `2026-04-17-device-registers-correction.md`，修正所有寄存器地址、数据类型、通信时序和去抖参数。**后续实现必须以修正文档为准**。

## 修正文档说明

**⚠️ 重要：** 本文档中"设备寄存器映射表"等章节包含严重错误，已被以下修正文档替代：

📄 **修正文档路径：** `F:\Work\FlipGearboxFactoryTest\Docs\superpowers\specs\2026-04-17-device-registers-correction.md`

**修正内容包括：**
- ✅ AQMD3610NS-A2 寄存器映射（P0）
- ✅ DYN200 扭矩传感器寄存器映射（P0）
- ✅ 单圈编码器寄存器映射（P0）
- ✅ 数控电源寄存器映射（P0）
- ✅ 通信调度与轮询策略（P1）
- ✅ AI1 去抖参数与轴速限制（P1）
- ✅ 总线架构优化（P1）

**后续 ExecPlan 编写和代码实现必须以修正文档为准，忽略本文档中的错误内容。**
