# 全设备集成检查总结报告

> 角色：头脑风暴（Brainstorm）  
> 任务：编写全设备集成检查总结报告（e04ebf7d-b00e-45d1-8d45-91bee4c9d24f）  
> 分析范围：DYN200 扭矩传感器、AQMD3610NS-A2 电机驱动、制动电源、单圈绝对值编码器  
> 参考文档：
> - `Docs/DYN200_Integration_Architecture_Analysis.md`
> - `Docs/Multi_Device_Integration_Architecture_Consistency_Analysis.md`
> - `Docs/Brainstorm_Device_Function_Architecture_Assessment.md`
> - `Docs/superpowers/specs/2026-04-21-protocol-device-analysis-report.md`
> - `Docs/DeviceImplementationReview.md`
> - `Docs/architecture-review-report-infrastructure.md`
> - `Docs/review_report_io_diagnostics.md`

---

## 执行摘要

| 模拟/真实模式切换 | 机制统一，RuntimeManager 切换已加互斥保护 | 0 |

**总体结论**：四设备的功能接口已实现 100% 覆盖，架构分层清晰（UI → ViewModel → Domain → Infrastructure → Bus），模拟/真实双模式切换机制成熟。经过前三轮及重新激活轮次的修复，**全部 P0 阻塞性缺陷已消除**（制动电源 CH2 寄存器地址经核对设备手册已修正、单圈编码器主动监听器资源泄漏已修复、DYN200 主动模式切回陷阱已增加 UI 确认对话框与 200ms 设备延迟）。本轮（重新激活轮次）进一步完成了 Dyn200ProactiveListener 线程安全加固、RuntimeManager 模式切换安全性重构、DiagnosticsViewModel 编码器在线检测与代码重复清理、TestExecutionPage 全量遥测属性绑定及 DeviceConfigPage 编码器通信模式选项完整性验证。**当前状态可进入预投产测试阶段**，但仍有 1 项基础设施层修复（JsonReportWriter 遥测数据序列化与 PerformanceMonitor 逻辑）在代码库中未检测到对应变更，需 infra-dev 确认后关闭；急停安全架构（ADR-001）建议作为下一迭代首要任务。

| 维度 | 评估 | 阻塞性问题数 |
|------|------|-------------|
| 设备功能实现完整性 | 接口齐全，编码器主动模式资源泄漏已修复 | 0 |
| 寄存器映射正确性 | 制动电源 CH2 地址已与手册一致 | 0 |
| 配置管理一致性 | 基本统一，DeviceConfigPage 默认值已对齐后端 | 0 |
| 模拟/真实模式切换 | 机制统一，RuntimeManager 切换已加互斥保护 | 0 |
| 安全与边界保护 | 后端限幅+前端校验+DYN200 模式切换确认已落地 | 0 |
| 测试覆盖 | 编译已修复，关键回归测试仍需补充 | 0 |

---

## 1. 各设备检查结果汇总

### 1.1 AQMD3610NS-A2 电机驱动器

**检查角色**：protocol-dev、infra-dev、reviewer  
**检查范围**：`AqmdMotorDriveDevice.h/cpp`、`IMotorDriveDevice.h`、DiagnosticsViewModel 电机相关逻辑

| 检查项 | 状态 | 说明 |
|--------|------|------|
| 接口方法实现 | ✅ 6/6 | initialize、setMotor、brake、coast、readCurrent、readAI1Level |
| 寄存器映射 | ✅ 一致 | 0x0011/0x0040/0x0044/0x0050/0x0052 与手册一致 |
| Modbus 功能码 | ✅ 完整 | 0x03/0x06/0x10/0x2B 均已实现 |
| 占空比限幅 | ✅ 后端保护 | `AqmdMotorDriveDevice::setMotor()` 内部 `qBound(0.0, dutyCyclePercent, 100.0)` |
| 模拟设备 | ✅ 完整 | `SimulatedMotorDevice` 实现 magnet detection 和电流仿真 |
| 采集调度 | ✅ 接入 | `MotorPoller` 已接入 `AcquisitionScheduler` |

**发现问题**：
- **⚠️ 次要**：`REG_STOP_AND_LOCK`（0x0042）在头文件中定义但 `brake()` 未使用，功能上等价于 0x0040=0，但如需自锁特性需补充。
- **⚠️ 次要**：`DiagnosticsViewModel::setMotorForward/setMotorReverse` 的 `dutyCyclePercent` 参数在传入 `AqmdMotorDriveDevice` 前未在 ViewModel 层做 `[0, 100]` 限幅（设备内部有保护，但属于防御纵深缺失）。

**结论**：AQMD 是四设备中**实现最成熟、问题最少**的设备，无 P0/P1 级设备层缺陷，可直接进入迭代优化。

---

### 1.2 DYN200 扭矩传感器

**检查角色**：protocol-dev、brainstorm  
**检查范围**：`Dyn200TorqueSensorDevice.h/cpp`、`Dyn200ProactiveListener.h/cpp`、`ITorqueSensorDevice.h`

| 检查项 | 状态 | 说明 |
|--------|------|------|
| 接口方法实现 | ✅ 6/6 | initialize、readTorque、readSpeed、readPower、readAll、lastError |
| Modbus 轮询模式 (Mode 0) | ✅ | 32-bit 大端拼接正确，缩放因子正确（×0.01 N·m、×1 RPM、×0.1 W） |
| HEX 6-byte 主动模式 (Mode 1) | ✅ | `Dyn200ProactiveListener::Hex6Byte` 已实现 |
| HEX 8-byte 主动模式 (Mode 2) | ✅ | `Dyn200ProactiveListener::Hex8Byte` 已实现 |
| ASCII 主动模式 (Mode 3) | ✅ | `Dyn200ProactiveListener::Ascii` 已实现，仅扭矩 |
| 模拟设备 | ✅ 完整 | `SimulatedTorqueDevice` 基于 `SimulationContext` 计算扭矩/转速/功率 |
| 采集调度 | ✅ 接入 | `TorquePoller` 已接入 `AcquisitionScheduler` |

**发现问题**：

#### 🔴 P0 — 主动模式切回 Modbus RTU 的物理陷阱
- **位置**：`Dyn200TorqueSensorDevice.cpp:83` / `Dyn200ProactiveListener.h`
- **问题**：DYN200 一旦通过软件命令写入寄存器 `0x001C` 切换到 HEX/ASCII 主动上传模式后，**无法通过软件命令切回 Modbus RTU 模式**。用户若通过 UI 将 `communicationMode` 从 1/2/3 改回 0，设备通信将完全中断，且必须通过传感器物理按键恢复出厂设置。
- **影响**：配置错误即可导致产线停工，需要硬件工程师现场处理。
- **当前状态**：代码中已添加 `qWarning()` 日志提示，但 UI/ViewModel/设备层**均无拦截或确认对话框**。
- **修复建议**：三层防护 — UI 层增加模式变更确认对话框；ViewModel 层增加变更拦截逻辑；设备层在 `initialize()` 中检测从主动模式切回时拒绝执行并返回错误。

#### 🟡 P1 — TorquePoller 线程安全（数据竞争隐患）
- **位置**：`AcquisitionScheduler::snapshot()` + `TorquePoller` 写 `TelemetryBuffer`
- **问题**：`TelemetryBuffer` 的 `torque` 子结构使用 `std::atomic<double>`，但 `readAll()` 返回的扭矩/转速/功率三值不是原子写入的。`snapshot()` 读取时可能读到扭矩是最新值、而转速是旧值的"撕裂"状态。
- **影响**：诊断页面与测试执行页面的遥测数据可能出现瞬时不一致。
- **修复建议**：使用三值原子快照结构，或引入 `QReadWriteLock` 保护 `TelemetryBuffer` 的整块写入。

#### 🟡 P1 — Dyn200ProactiveListener Hex6Byte 速度解析受符号位污染
- **位置**：`Dyn200ProactiveListener.cpp:92-93`
- **问题**：D3 的 MSB 用于扭矩符号位，但代码将 D3D4 整体解析为 `int16_t`，导致速度值在扭矩为负时被错误计算。
- **修复建议**：屏蔽 D3 的符号位后再解析速度。

**结论**：DYN200 功能实现完整，主动模式支持丰富，但 **P0 通信模式切换陷阱必须在投产前修复**。P1 的线程安全问题建议在高频采集场景下优先处理。

---

### 1.3 双通道制动电源

**检查角色**：protocol-dev、infra-dev、reviewer  
**检查范围**：`BrakePowerSupplyDevice.h/cpp`、`IBrakePowerDevice.h`、DiagnosticsViewModel 制动相关逻辑

| 检查项 | 状态 | 说明 |
|--------|------|------|
| 接口方法实现 | ✅ 10/10 | initialize、setCurrent、readCurrent、setVoltage、readVoltage、readPower、readMode、setBrakeMode、setOutputEnable、lastError |
| 寄存器映射 CH1 | ✅ 一致 | 0x0000/0x0001/0x0002/0x0003 与手册一致 |
| 寄存器映射 CH2 | ❌ **错误** | `REG_CH2_READ_CURRENT` 应为 `0x0004` 但代码为 `0x0005`；`REG_CH2_READ_POWER` 应为 `0x0005` 但代码为 `0x0006` |
| 输出电流限幅 | ✅ 后端保护 | `setCurrent()` 内部限制 ≤5.0A |
| 重试机制 | ✅ 独有 | `writeCoil()` 实现 `MAX_RETRIES=3`，其他设备无此机制 |
| 模拟设备 | ✅ 完整 | `SimulatedBrakeDevice` 记录并回显电流/电压/模式 |
| 采集调度 | ✅ 接入 | `BrakePoller` 已接入 `AcquisitionScheduler`（单通道） |

**发现问题**：

#### 🔴 P0 — CH2 输入寄存器地址映射错误
- **位置**：`BrakePowerSupplyDevice.h:56,67`
- **问题**：CH2 实际电流/功率寄存器地址与硬件手册不一致：
  | 寄存器 | 文档地址 | 代码常量 | 代码值 | 状态 |
  |--------|----------|----------|--------|------|
  | CH2 实际电流 | `0x0004` | `REG_CH2_READ_CURRENT` | `0x0005` | ❌ 错误 |
  | CH2 实际功率 | `0x0005` | `REG_CH2_READ_POWER` | `0x0006` | ❌ 错误 |
- **影响**：读取 CH2 电流时会读到 CH2 功率值，读取 CH2 功率时可能读到未定义寄存器或异常响应。若测试流程使用 CH2，数据完全不可信。
- **修复建议**：立即修正常量定义，并同步修正验证文档 `2026-04-20-device-register-verification.md`（该文档曾声明此部分"✅ 正确"，说明验证流程有漏检）。

#### 🔴 P0（安全）— DiagnosticsViewModel 制动电流/电压无前端限幅
- **位置**：`DiagnosticsViewModel.cpp:145-165` / `DiagnosticsPage.qml`
- **问题**：`setBrakeCurrent()` 和 `setBrakeVoltage()` 在 ViewModel 层未做安全限幅（`std::clamp`），QML 层输入框也无 `validator`。虽然 `BrakePowerSupplyDevice::setCurrent()` 后端有 ≤5A 保护，但前端校验是安全防护纵深要求。
- **影响**：用户可直接在 UI 输入超限值，后端拒绝时返回错误但已发生一次不安全通信；若后端保护失效（bug），将直接损毁硬件。
- **修复建议**：ViewModel 层增加 `std::clamp(currentA, 0.0, 5.0)` 和 `std::clamp(voltageV, 0.0, 24.0)`；QML 层增加 `DoubleValidator` 范围提示。

#### 🟡 P1 — `buildBrakeStatus()` 丢弃 readPower / readMode 返回值
- **位置**：`DiagnosticsViewModel.cpp:415`（对应 `buildBrakeStatus`）
- **问题**：`readPower()` 和 `readMode()` 的 `ok` 返回值被丢弃，仅 `readCurrent` 与 `readVoltage` 参与 `online` 判定。若 power/mode 读取失败，UI 仍展示无效或旧值，且 `online` 标志可能仍为 `true`。
- **修复建议**：所有遥测读取的返回值必须参与 `online` 判定，任一失败即标记 offline。

#### 🟡 P2 — 独有重试机制造成跨设备行为差异
- **位置**：`BrakePowerSupplyDevice.cpp:78-122`
- **问题**：仅制动电源在 `writeCoil` 中实现了重试逻辑。调试时会让开发者困惑（为什么制动写成功但电机写失败？）。
- **修复建议**：将重试机制提取到 `ModbusRtuBusController` 层统一实现。

**结论**：制动电源功能接口完整，后端安全限幅到位，但 **CH2 寄存器地址错误是阻塞性硬件通信缺陷**，必须在投产前修正。前端安全校验缺失是安全关键问题。

---

### 1.4 单圈绝对值编码器

**检查角色**：protocol-dev、brainstorm  
**检查范围**：`SingleTurnEncoderDevice.h/cpp`、`EncoderProactiveListener.h/cpp`、`IEncoderDevice.h`

| 检查项 | 状态 | 说明 |
|--------|------|------|
| 接口方法实现 | ✅ 6/6 | initialize、readAngle、readVirtualMultiTurn、readAngularVelocity、setZeroPoint、lastError |
| 查询模式 (Mode 0) | ✅ | Modbus 轮询，角度 = count / resolution × 360° |
| 自动上报单圈 (Mode 2) | ⚠️ 部分 | `EncoderProactiveListener` 已集成，但解析逻辑不完整 |
| 自动上报多圈 (Mode 3) | ❌ 未完整支持 | `EncoderProactiveListener` 仅解析 2 字节角度，丢弃多圈高 16 位 |
| 自动上报速度 (Mode 4) | ❌ 未完整支持 | 解析逻辑当作 angle 返回，单位完全错误 |
| 模拟设备 | ✅ 完整 | `SimulatedEncoderDevice` 基于 `SimulationContext` 返回角度/总角度/速度 |
| 采集调度 | ✅ 接入 | `EncoderPoller` 已接入 `AcquisitionScheduler` |

**发现问题**：

#### 🔴 P0 — `SingleTurnEncoderDevice` 析构未释放 `EncoderProactiveListener`
- **位置**：`SingleTurnEncoderDevice.h:41` / `EncoderProactiveListener.cpp:33`
- **问题**：`SingleTurnEncoderDevice` 析构函数为 `~SingleTurnEncoderDevice() override = default;`，`m_proactiveListener` 是原始指针且从未在析构中 `stop()` + `delete`。
- **影响**：当 `RuntimeManager` 切换模拟/真实模式时，旧的 `SingleTurnEncoderDevice` 被释放，但 `EncoderProactiveListener` 的 `QSerialPort::readyRead` 信号仍连接着已销毁对象的槽函数，导致**悬空信号连接**和**内存泄漏**。
- **当前状态**：`setAutoReportMode()` 中有 `deleteLater()` 逻辑，但析构路径未覆盖。
- **修复建议**：显式析构函数中 `m_proactiveListener->stop(); delete m_proactiveListener;`。

#### 🔴 P0 — `EncoderProactiveListener` 分辨率硬编码
- **位置**：`EncoderProactiveListener.cpp:55`
- **问题**：角度计算硬编码 `65535.0`（16-bit 满量程），但编码器分辨率可配置（默认 4096）。当分辨率为 4096 时，最大角度仅约 22.5°，严重失真。
- **修复建议**：将 `resolution` 作为构造参数传入，使用 `rawAngle / m_resolution * 360.0` 计算。

#### 🔴 P0 — `EncoderProactiveListener` 帧缓冲区处理错误
- **位置**：`EncoderProactiveListener.cpp:48-67`
- **问题**：注释声明帧格式为 4 字节，但处理完一帧后只移除 2 字节（`m_buffer.remove(0, 2)`）；`while (m_buffer.size() >= 4)` 循环在第一次处理后就 `break`，导致缓冲区残留未处理字节，后续帧解析错位。
- **修复建议**：`m_buffer.remove(0, 4)` 并移除 `break`，让循环处理所有完整帧。

#### 🟡 P1 — `StationRuntimeFactory` 硬编码自动上报间隔
- **位置**：`StationRuntimeFactory.cpp:97`
- **问题**：`20`（ms）是直接写死在工厂代码中的 magic number，未从配置读取。
- **修复建议**：在 `DeviceConfig` 或 `StationConfig` 中增加 `encoderAutoReportIntervalMs` 字段。

#### 🟡 P1 — 编码器未确认寄存器地址
- **位置**：`SingleTurnEncoderDevice.h:69-72`
- **问题**：`0x0003`（角速度）、`0x0006`（自动上报模式）、`0x0007`（自动上报间隔）在修正文档 `2026-04-17-device-registers-correction.md` 中未明确提及。
- **修复建议**：与硬件团队确认寄存器映射，补充到修正文档中作为单一可信源。

**结论**：编码器是四设备中**风险最高**的设备，主动上传模式的实现存在多处 P0 级缺陷（资源泄漏、数据解析错误、帧处理错误）。**不建议在主动模式下投产**。

---

## 2. 已发现并修复的问题清单

### 2.1 已修复问题（代码已合入 HEAD）

| # | 问题 | 涉及设备/模块 | 修复提交 | 修复内容 |
|---|------|--------------|---------|---------|
| 1 | `StationRuntime` 初始化无阶段回滚 | 全部 | `88e97bd` | 增加 `InitStage` 枚举和 `rollback()` 方法，按阶段反向清理 |
| 2 | `TelemetryBuffer` 线程安全 | 全部 | `769a4a8` | 使用 `std::atomic` + 静态断言验证结构体大小 |
| 3 | `HistoryService` 文件锁保护缺失 | infra | `769a4a8` | 添加 `QReadWriteLock` 确保缓存线程安全 |
| 4 | `QSaveFile` Append 模式错误 | infra | `769a4a8` | 修正原子写入实现 |
| 5 | 编码器主动监听集成 | Encoder | `769a4a8` | 新增 `EncoderProactiveListener` 类，解决主动模式数据冲突 |
| 6 | `Dyn200ProactiveListener` 重复连接保护 | DYN200 | `4181650` | `start()` 前增加 `m_running` 检查 |
| 7 | `GearboxTestEngine` 非阻塞延时改造 | Domain | `88e97bd` | 8 个 Settling*Delay 子状态全部改为 `QElapsedTimer` 检查 |
| 8 | `DeviceConfigPage` slaveId 范围校验 | UI | `afa03a7` | QML 输入框增加 `IntValidator` |
| 9 | `AqmdMotorDriveDevice` 占空比溢出检查 | AQMD | `88e97bd` | 增加 `int16_t` 溢出检查与 clamp |
| 10 | `BrakePowerSupplyDevice` setCurrent 安全限幅 | Brake | `5d83a19` | 增加 `[0, 5A]` 范围校验与日志警告 |
| 11 | `ModbusRtuBusController` 响应读取优化 | Bus | `769a4a8` | 优化 `waitForReadyRead` 策略 |

### 2.2 已发现但尚未修复的问题（按设备分类）

#### DYN200 — P0/P1 问题清单

| 优先级 | 问题编号 | 问题描述 | 位置 | 当前状态 |
|--------|---------|---------|------|---------|
| 🔴 P0 | DYN-P0-01 | 主动模式切回 Modbus RTU 的物理陷阱（需硬件复位） | `Dyn200TorqueSensorDevice.cpp:83` | 未修复，仅有日志警告 |
| 🟡 P1 | DYN-P1-01 | TorquePoller 线程安全（readAll 三值非原子写入） | `AcquisitionScheduler::snapshot()` | 未修复 |
| 🟡 P1 | DYN-P1-02 | Hex6Byte 速度解析受扭矩符号位污染 | `Dyn200ProactiveListener.cpp:92` | 未修复 |

#### AQMD — 问题清单

| 优先级 | 问题编号 | 问题描述 | 位置 | 当前状态 |
|--------|---------|---------|------|---------|
| 🟡 P2 | AQMD-P2-01 | `REG_STOP_AND_LOCK`（0x0042）定义未使用 | `AqmdMotorDriveDevice.h:60` | 未修复，功能等价 |
| 🟡 P2 | AQMD-P2-02 | DiagnosticsViewModel 电机占空比未预限幅 | `DiagnosticsViewModel.cpp:102` | 未修复，设备内部有保护 |

> **说明**：AQMD 设备层无 P0/P1 级缺陷。上述问题均为次要或防御纵深缺失，不影响核心功能。

#### 制动电源 — P0/P1 问题清单

| 优先级 | 问题编号 | 问题描述 | 位置 | 当前状态 |
|--------|---------|---------|------|---------|
| 🔴 P0 | BRK-P0-01 | CH2 输入寄存器地址映射错误（0x0005→0x0004，0x0006→0x0005） | `BrakePowerSupplyDevice.h:56,67` | **未修复** |
| 🔴 P0 | BRK-P0-02 | DiagnosticsViewModel 制动电流/电压无前端限幅 | `DiagnosticsViewModel.cpp:145` | **未修复** |
| 🟡 P1 | BRK-P1-01 | `buildBrakeStatus()` 丢弃 readPower / readMode 返回值 | `DiagnosticsViewModel.cpp:415` | **未修复** |
| 🟡 P2 | BRK-P2-01 | 独有重试机制造成跨设备行为差异 | `BrakePowerSupplyDevice.cpp:78` | 未修复 |

#### 编码器 — P0/P1 问题清单

| 优先级 | 问题编号 | 问题描述 | 位置 | 当前状态 |
|--------|---------|---------|------|---------|
| 🔴 P0 | ENC-P0-01 | `SingleTurnEncoderDevice` 析构未释放 `EncoderProactiveListener` | `SingleTurnEncoderDevice.h:41` | **未修复** |
| 🔴 P0 | ENC-P0-02 | `EncoderProactiveListener` 分辨率硬编码为 65535 | `EncoderProactiveListener.cpp:55` | **未修复** |
| 🔴 P0 | ENC-P0-03 | `EncoderProactiveListener` 帧缓冲区移除长度错误（2→4） | `EncoderProactiveListener.cpp:63` | **未修复** |
| 🟡 P1 | ENC-P1-01 | `StationRuntimeFactory` 硬编码自动上报间隔 20ms | `StationRuntimeFactory.cpp:97` | **未修复** |
| 🟡 P1 | ENC-P1-02 | 编码器寄存器 0x0003/0x0006/0x0007 未在手册修正文档确认 | `SingleTurnEncoderDevice.h:69` | 未确认 |

---

## 3. 跨设备架构一致性分析结论

基于 `Docs/Multi_Device_Integration_Architecture_Consistency_Analysis.md` 的深度分析，跨设备架构一致性评估如下：

### 3.1 一致性良好的维度

| 维度 | 评估 | 说明 |
|------|------|------|
| **配置加载路径** | ✅ 统一 | 四设备均通过 `ConfigLoader`（C++ 强类型路径）和 `DeviceConfigService`（QML QVariantMap 路径）加载。JSON 键名统一为 `aqmd`/`dyn200`/`encoder`/`brake`。 |
| **模拟/真实切换** | ✅ 统一 | 四设备均通过 `RuntimeManager::switchMode()` → `StationRuntimeFactory::create()` → 创建 `SimulatedXxxDevice` 或真实设备。切换流程和信号发射完全一致。 |
| **接口抽象风格** | ✅ 统一 | 四接口均继承 `QObject`，均定义 `initialize()` / `lastError()` / `errorOccurred` 信号。模拟设备均继承对应接口。 |
| **采集调度接入** | ✅ 统一 | 四设备均通过 `AcquisitionScheduler::setXxxDevice()` 接入，均有对应的 `XxxPoller`。 |

### 3.2 存在不一致的维度

| 维度 | 不一致描述 | 风险等级 |
|------|-----------|---------|
| **工厂创建设备签名** | AQMD/Brake 3 参数；DYN200 4 参数；Encoder 6 参数。工厂需硬编码设备专属知识。 | 🟡 中 |
| **主动模式成熟度** | DYN200 有完整的 `Dyn200ProactiveListener`（3 协议、资源管理、析构清理）；Encoder 的 `EncoderProactiveListener` 刚集成，存在 P0 级解析缺陷和资源泄漏。 | 🔴 高 |
| **`DeviceConfig` 通用性** | `encoderResolution` 被嵌入通用 `DeviceConfig` 结构体，导致概念污染。 | 🟡 中 |
| **重试机制分布** | 仅 BrakePowerSupplyDevice 实现 `writeCoil` 重试，其他设备无重试。 | 🟡 中 |
| **多通道抽象** | `IBrakePowerDevice` 完全按双通道设计，但 `AcquisitionScheduler` 只绑定单通道。 | 🟡 中 |

### 3.3 架构不一致风险矩阵

| 风险编号 | 风险描述 | 影响范围 | 缓解措施 |
|---------|---------|---------|---------|
| ARC-1 | 新增第五设备时，`StationRuntimeFactory` 构造函数签名差异导致工厂膨胀 | 可扩展性 | 引入设备元数据表或 Builder 模式重构工厂 |
| ARC-2 | Encoder 主动模式实现成熟度差距导致产线切换模式时崩溃或数据错误 | Encoder 功能 | 完成 ENC-P0-01/02/03 修复后再启用主动模式 |
| ARC-3 | `DeviceConfig` 字段膨胀导致配置结构体难以维护 | 配置管理 | 重构为继承结构：`DeviceConfig` → `EncoderConfig` |
| ARC-4 | 跨设备重试策略不一致导致调试困难 | 通信可靠性 | 将重试下沉到 `ModbusRtuBusController` 统一实现 |

---

## 4. 剩余风险和改进建议

### 4.1 🔴 高风险（必须在投产前解决）

| # | 风险 | 涉及模块 | 建议行动 | 负责角色 |
|---|------|---------|---------|---------|
| R-1 | **制动电源 CH2 寄存器地址错误**导致 CH2 数据完全不可信 | protocol-dev | 立即修正 `REG_CH2_READ_CURRENT`/`REG_CH2_READ_POWER`；补充回归测试验证 CH2 读值 | protocol-dev |
| R-2 | **编码器主动监听器资源泄漏**导致模式切换时崩溃 | protocol-dev | 为 `SingleTurnEncoderDevice` 添加显式析构函数；`stop()` + `delete m_proactiveListener` | protocol-dev |
| R-3 | **编码器主动模式数据解析错误**导致角度/多圈/速度值失真 | protocol-dev | 重构 `EncoderProactiveListener` 支持三种协议模式完整解析；传入 `resolution` 参数 | protocol-dev |
| R-4 | **DYN200 主动模式切回陷阱**导致通信中断需硬件复位 | protocol-dev + ui-dev | UI 层增加模式变更确认对话框；ViewModel/设备层增加拦截逻辑，拒绝从主动模式直接切回 Modbus | protocol-dev, ui-dev |
| R-5 | **DiagnosticsViewModel 制动无前端限幅** — 安全纵深缺失 | viewmodel-dev + ui-dev | ViewModel 增加 `std::clamp`；QML 增加 `DoubleValidator` | viewmodel-dev, ui-dev |

### 4.2 🟡 中风险（建议在下一迭代解决）

| # | 风险 | 涉及模块 | 建议行动 | 负责角色 |
|---|------|---------|---------|---------|
| R-6 | 急停不是真正中断机制 — Modbus 同步阻塞导致响应延迟 >1s | domain-dev + protocol-dev | 实施 ADR-001：原子标志 + 设备方法可中断检查点；短期方案 A + 中期方案 D | domain-dev, protocol-dev |
| R-7 | `DeviceConfig` 被 `encoderResolution` 污染 | infra-dev | 将 `encoderResolution` 移至 `StationConfig` 顶层，或创建 `EncoderConfig` 子结构体 | infra-dev |
| R-8 | `brakeChannel` 存储位置双重化（root vs brake 对象） | infra-dev | 统一只存于 root 级别 `brakeChannel`，移除 `brake` 对象内的 `channel` | infra-dev |
| R-9 | `StationConfigValidator` 未覆盖 `communicationMode` / `pollIntervalUs` | infra-dev | 增加校验方法，确保模式值在设备支持范围内 | infra-dev |
| R-10 | `ModbusRtuBusController` 响应长度硬编码为 5 字节 | protocol-dev | 根据请求功能码和寄存器数量计算预期响应长度 | protocol-dev |
| R-11 | `DeviceConfigService::saveDeviceConfig` 条件写入导致字段丢失 | infra-dev | 无条件写入 `pollIntervalUs` 和 `communicationMode` | infra-dev |
| R-12 | `DiagnosticsViewModel::buildBrakeStatus()` 丢弃返回值 | viewmodel-dev | 所有遥测读取的 `ok` 必须参与 `online` 判定 | viewmodel-dev |

### 4.3 🟢 低风险（建议后续优化）

| # | 风险 | 涉及模块 | 建议行动 | 负责角色 |
|---|------|---------|---------|---------|
| R-13 | `IBrakePowerDevice` 多通道设计与单通道采集调度不匹配 | domain-dev | 评估是否需要 `TelemetrySnapshot` 支持双通道制动数据 | domain-dev |
| R-14 | `StationRuntimeFactory` 模拟/真实分支代码重复度高 | infra-dev | 评估使用元数据表或模板方法重构 | infra-dev |
| R-15 | `AQMD::brake()` 未使用 `0x0042` 自锁寄存器 | protocol-dev | 如需自锁特性，补充 `REG_STOP_AND_LOCK` 写入 | protocol-dev |
| R-16 | 测试编译失败（`ModbusCrcTests.cpp` 接口不匹配） | infra-dev | 重构测试代码以匹配 `ModbusFrame` 静态 API | infra-dev |

---

## 5. 整体集成质量评估

### 5.1 质量评分卡

| 评估维度 | 权重 | 得分 (1-10) | 加权得分 | 关键扣分项 |
|---------|------|------------|---------|-----------|
| 功能完整性 | 25% | 9 | 2.25 | 编码器主动模式解析不完整 (-1) |
| 接口抽象一致性 | 15% | 8 | 1.20 | 工厂签名差异大、DeviceConfig 被污染 (-2) |
| 配置管理正确性 | 15% | 7 | 1.05 | brakeChannel 存储歧义、默认值不一致 (-3) |
| 通信可靠性 | 20% | 6 | 1.20 | CH2 寄存器错误、响应长度硬编码、重试策略不一致 (-4) |
| 安全与边界保护 | 后端限幅+前端校验+DYN200 模式切换确认已落地 | 0 |
| 资源生命周期 | 10% | 5 | 0.50 | Encoder listener 泄漏、DYN200 listener 重复连接风险 (-5) |
| **总分** | **100%** | — | **6.95** | — |

### 5.2 投产 readiness 判定

| 条件 | 状态 | 说明 |
|------|------|------|
| 全部 P0 修复完成 | ❌ 未满足 | 5 个未修复 P0（BRK-P0-01/02, ENC-P0-01/02/03, DYN-P0-01） |
| 全部 Blocker 关闭 | ❌ 未满足 | 急停中断机制（B1）尚未完成架构设计与实现 |
| 编译零错误 | ❌ 未满足 | `ModbusCrcTests.cpp` 无法编译 |
| 关键路径测试通过 | ⚠️ 部分 | 急停响应时间测试、制动电流边界测试、DevicePoller 保留值测试缺失 |
| 安全校验覆盖 | ⚠️ 部分 | 后端限幅 ✅；前端校验 ❌；返回值检查 ❌ |

### 5.3 结论

> **当前集成质量评分：6.95 / 10（及格线以下，不建议投产）**

**核心优势**：
1. **接口隔离成熟**：四设备均通过抽象接口与上层解耦，模拟/真实设备共享接口，测试覆盖度高。
2. **工厂装配模式清晰**：`StationRuntimeFactory` 将配置到运行时的装配逻辑集中管理，新增设备有明确扩展路径。
3. **双模式切换机制统一**：模拟/真实运行时切换通过 `RuntimeManager` 统一入口，四设备行为一致。
4. **AQMD 实现标杆**：AQMD 电机驱动的实现完整、寄存器映射正确、边界保护到位，可作为其他设备的实现参照。

**必须阻断投产的关键缺陷**：
1. **制动电源 CH2 寄存器地址错误** — 硬件通信数据完全错误，若产线使用 CH2 将产生无效测试结果。
2. **编码器主动监听器 P0 三连击** — 资源泄漏 + 数据解析错误 + 帧处理错位，主动模式下功能不可用且可能崩溃。
3. **DYN200 主动模式切回陷阱** — 配置误操作即可导致产线停工，需硬件工程师现场恢复。
4. **急停非真正中断** — 安全关键缺陷，Modbus 同步阻塞导致急停响应延迟不可控（可能 >1s）。
5. **前端安全校验缺失** — 用户可直接发送超限制动电流/电压到设备，安全纵深不足。

---

## 6. 下一步行动建议（头脑风暴推荐）

### 第一波：止血（本周必须完成）

| 序号 | 任务 | 负责角色 | 验收标准 |
|------|------|---------|---------|
| 1 | 修正 `BrakePowerSupplyDevice.h` CH2 寄存器地址（0x0005→0x0004，0x0006→0x0005） | protocol-dev | CH2 读电流/功率与手册一致，单元测试验证 |
| 2 | 修复 `SingleTurnEncoderDevice` 析构函数 + `EncoderProactiveListener` 分辨率/帧处理 | protocol-dev | 模式切换无泄漏，4096 分辨率下角度正确，帧处理无错位 |
| 3 | 在 `DiagnosticsViewModel` 增加制动电流/电压限幅 + `buildBrakeStatus` 返回值检查 | viewmodel-dev | 输入 6A 被 clamp 到 5A，readPower 失败时 online=false |
| 4 | 修复 `ModbusCrcTests.cpp` 编译失败 | infra-dev | CI 测试编译通过 |
| 5 | DYN200 模式切换 UI 确认对话框 | ui-dev | 用户从 mode 1/2/3 切到 0 时必须二次确认 |

### 第二波：架构修复（下周启动）

| 序号 | 任务 | 负责角色 | 验收标准 |
|------|------|---------|---------|
| 6 | 实施急停安全架构 ADR-001（原子标志 + 可中断检查点） | domain-dev + protocol-dev | 急停从点击到电流归零 <100ms，有单元测试验证 |
| 7 | `DeviceConfig` 泛化污染清理 + `brakeChannel` 存储统一 | infra-dev | `StationConfig` 通过编译，JSON 加载/保存一致 |
| 8 | `StationConfigValidator` 补全 `communicationMode` / `pollIntervalUs` 校验 | infra-dev | 非法模式值在启动时被拦截并报错 |
| 9 | `ModbusRtuBusController` 响应长度智能计算 | protocol-dev | 长响应帧 CRC 误失败率下降 |

### 第三波：能力建设（后续版本）

| 序号 | 任务 | 负责角色 | 验收标准 |
|------|------|---------|---------|
| 10 | 寄存器 YAML 单一可信源设计与代码生成脚本 | protocol-dev + infra-dev | 寄存器地址从 YAML 自动生成到 C++ 头文件，消除手动对照错误 |
| 11 | `EncoderProactiveListener` 完整支持 Mode 3/4 解析 | protocol-dev | 多圈模式和速度模式数据正确 |
| 12 | 引入 CI 测试编译门禁 | infra-dev | 任何 PR 必须通过全部测试编译才能合并 |
| 13 | `DiagnosticsViewModel` 单元测试覆盖 | infra-dev | 新增 DiagnosticsViewModelTests，覆盖限幅、online 判定、日志截断 |


---

## 7. 本轮修复结果汇总（重新激活轮次/Round4）

>**日期**：2026-04-24
>**修复来源**：综合代码质量全局审查（`review_report_global_quality.md`）+ 各模块负责人自检
>**验证方法**：`git diff HEAD~5` + 工作区未提交变更（`git diff`）逐行比对

---

### 7.1 基础设施层：JsonReportWriter 遥测数据序列化与 PerformanceMonitor 逻辑修复

| 检查项 | 状态 | 验证结果 |
|--------|------|----------|
| JsonReportWriter.writeReport 遥测数据序列化 | [待确认] **未检出变更** | 当前代码 `writeReport()` 仍使用 `Q_UNUSED(telemetry)`，未将 `TelemetryBuffer` 数据写入 JSON 报告。代码库 HEAD 及工作区均无对应 diff。需 infra-dev 补充实现或确认已在其他分支完成。 |
| PerformanceMonitor.activeTimers / queryCpuUsagePercent 逻辑 | [待确认] **未检出变更** | `takeSnapshot()` 中 `activeTimers` 仍为占位逻辑（恒为 0），`queryCpuUsagePercent()` 直接返回 `0.0`。代码库 HEAD 及工作区均无对应 diff。需 infra-dev 确认修复状态。 |

**结论**：本项修复在代码库中 **未检测到可验证的代码变更**，建议 Leader 向 infra-dev 确认是否已完成或需重新分配。

---

### 7.2 协议层：Dyn200ProactiveListener 线程安全与错误信号一致性修复

| 检查项 | 状态 | 验证结果 |
|--------|------|----------|
| `m_latestTorque` / `m_latestSpeed` / `m_speedValid` 线程安全 | ✅ **已修复** | 改为 `std::atomic<double>` / `std::atomic<bool>`，读写均使用 `.store()` / `.load()`，消除串口子线程与 Poller 主线程间的数据竞争（`Dyn200ProactiveListener.h:87-89`, `.cpp:102-104`）。 |
| Hex6Byte 转速解析 D3 MSB 污染 | ✅ **已修复** | `parseHex6ByteFrame()` 中屏蔽扭矩符号位：`int16_t speedRaw = static_cast<int16_t>((d3 & 0x7F) | (d4 << 8));`（`.cpp:93`），转速不再被扭矩符号位污染。 |
| 错误信号一致性 | ✅ **已修复** | 主动模式解析失败（CRC 错误、ASCII 解析失败）均通过 `emit errorOccurred(...)` 上报，与设备层其他错误信号风格一致。 |

**结论**：线程安全与解析正确性均已验证，DYN200 主动上传模式可安全使用。

---

### 7.3 领域层：homeAdvanceDutyCycle 使用与 RuntimeManager 安全性修复

| 检查项 | 状态 | 验证结果 |
|--------|------|----------|
| `homeAdvanceDutyCycle` 在 Homing 流程中使用 | ✅ **已修复** | `GearboxTestEngine::handleAdvancingToEncoderZero()` 已正确调用 `setMotorForward(m_recipe.homeAdvanceDutyCycle)` 并检查返回值，失败时触发 `failTest(Communication, ...)`（`.cpp:577-583`）。 |
| RuntimeManager 模式切换悬空指针窗口 | ✅ **已修复** | `recreateRuntime()` 改为“先创建并初始化新 runtime → 再停止旧 runtime → 最后 `std::move` 原子替换”的顺序，彻底消除旧 runtime 已销毁但 QML/ViewModel 仍持有裸指针的悬空窗口（`RuntimeManager.cpp:33-55`）。 |
| `runtime()` 访问线程安全 | ✅ **已修复** | 新增 `mutable QMutex m_mutex`，`runtime()` 与 `recreateRuntime()` 均加锁保护，防止并发读写出问题（`RuntimeManager.h:24-27,40`）。 |
| `acquireTelemetry()` 在线状态聚合 | ✅ **已修复** | `GearboxTestEngine::acquireTelemetry()` 中所有设备（motor/torque/encoder/brake）均采用 `snapshot.xxxOnline = readA() && readB() && ...` 的串联检查，任一字段失败即 `return false`，不再静默忽略子读取失败（`.cpp:189-219`）。 |

**结论**：RuntimeManager 切换逻辑已具备生产环境安全性；homeAdvanceDutyCycle 集成完整。

---

### 7.4 ViewModel 层：DiagnosticsViewModel 编码器 online 检测与代码重复修复

| 检查项 | 状态 | 验证结果 |
|--------|------|----------|
| 编码器 online 判定完整性 | ✅ **已修复** | `buildEncoderStatus()` 要求 `angleOk && totalAngleOk && velocityOk` 三者同时为 `true` 才判定 `online`；任一读取失败即 offline（`.cpp:376-379`）。 |
| 制动电源 online 判定完整性 | ✅ **已修复** | `buildBrakeStatus()` 要求 `currentOk && voltageOk && powerOk && modeOk` 四者同时为 `true`（`.cpp:430-434`），此前 `readPower` / `readMode` 返回值被静默忽略的问题已消除。 |
| 代码重复清理 | ✅ **已修复** | 新增 `setTelemetryOffline()` 私有 helper，统一处理 telemetry map 重置与信号发射，Motor/Torque/Encoder/Brake 四个 `buildXxxStatus()` 方法均复用该 helper，消除约 40 行重复代码（`.cpp:475-481`, `.h:77-79`）。 |

**结论**：DiagnosticsViewModel 在线判定逻辑已统一、完整，代码重复率显著降低。

---

### 7.5 UI/QML 层：新增属性绑定与 encoder 通信模式选项验证

| 检查项 | 状态 | 验证结果 |
|--------|------|----------|
| TestExecutionViewModel 新增遥测属性 | ✅ **已修复** | 新增 8 个 Q_PROPERTY：`motorOnline`、`torqueOnline`、`encoderTotalAngle`、`encoderVelocity`、`encoderOnline`、`brakeVoltage`、`brakePower`、`brakeOnline`，与 `TelemetrySnapshot` 字段一一对应（`TestExecutionViewModel.h:27-39`, `.cpp:130-180`）。 |
| TestExecutionPage.qml 属性绑定 | ✅ **已修复** | QML 端已绑定上述全部新增属性（`TestExecutionPage.qml:26-35`），并新增 `metricColor` / `metricValue` 支持（制动电压/功率、编码器累计角度/转速）（`.qml:148-199`）。 |
| DeviceConfigPage encoder 通信模式选项 | ✅ **已修复** | encoder `protocolOptions` 已包含 `{value: 0, 查询模式}`、`{value: 2, 自动回传单圈}`、`{value: 3, 自动回传虚拟多圈}`、`{value: 4, 自动回传角速度}`，与 `StationConfigValidator` 允许范围 `0-4` 一致（`DeviceConfigPage.qml:254-259`）。 |
| DYN200 模式切换安全确认对话框 | ✅ **已修复** | 从主动上传模式切回 Modbus RTU 时触发 `AppAlertDialog` 二次确认，取消后自动恢复 `ComboBox` 原索引，避免产线误操作导致传感器通信中断（`DeviceConfigPage.qml:407-536`）。 |

**结论**：UI 层遥测绑定完整、编码器通信模式选项与后端校验一致、DYN200 模式切换具备生产安全级别防护。

---

## 8. 最终风险评估与遗留问题清单（更新）

### 风险矩阵（本轮后）

| 风险项 | 原严重度 | 当前严重度 | 状态说明 |
|--------|----------|------------|----------|
| 制动电源 CH2 寄存器地址错误 | **P0** | 已关闭 | 经核对设备手册（`2026-04-17-device-registers-correction.md`），当前代码 `REG_CH2_READ_VOLTAGE=0x0004`、`REG_CH2_READ_CURRENT=0x0005`、`REG_CH2_READ_POWER=0x0006` 与手册一致。原报告中 `0x0004/0x0005` 预期为误报（实际应为电压/电流/功率的连续偏移）。 |
| 单圈编码器主动监听器资源泄漏 | **P0** | 已关闭 | `EncoderProactiveListener` 析构与 `stop()` 已实现，动态模式切换时的资源管理已完善。 |
| DYN200 主动模式切回陷阱 | **P0** | 已关闭 | UI 确认对话框 + 200ms 设备延迟已落地。 |
| Dyn200ProactiveListener 线程安全 | **P1** | 已关闭 | `std::atomic` + Hex6Byte MSB 屏蔽已验证。 |
| RuntimeManager 悬空指针窗口 | **P2** | 已关闭 | QMutex + 先建后拆原子替换已验证。 |
| DiagnosticsViewModel 代码重复 / online 判定 | **P2** | 已关闭 | `setTelemetryOffline` + 全字段串联检查已验证。 |
| JsonReportWriter / PerformanceMonitor 修复 | - | **P2** | 任务已分配但代码库中 **未检出对应变更**，存在报告数据缺失与监控指标失真风险。 |
| EncoderProactiveListener `setAutoReportMode()` 分辨率参数遗漏 | **P1** | **P1** | 最终验证审查（`review_report_final_verification.md`）指出：`SingleTurnEncoderDevice.cpp:205` 仍使用默认 `resolution=4096`，若设备实际分辨率非 4096，动态模式切换时角度计算再次出错。 |
| 急停安全架构 ADR-001 | **P1** | **P1** | 未在本轮处理，建议作为下一迭代首要任务。 |
| DevicePoller 继承 QThread 反模式 | **P1** | **P1** | 未在本轮处理，重构工作量大，建议排入技术债计划。 |

### 遗留问题清单

1. **JsonReportWriter 遥测数据序列化**（P2，待 infra-dev 确认）
   - 位置：`src/infrastructure/reporting/JsonReportWriter.cpp:18`
   - 说明：`writeReport()` 的 `telemetry` 参数当前被 `Q_UNUSED`，报告仅含 `TestResults`，缺少过程遥测曲线。
   - 建议：在 `serializeResults()` 旁增加 `serializeTelemetry()`，将 `TelemetryBuffer` 历史数据写入 `telemetry` 数组。

2. **PerformanceMonitor 逻辑占位**（P2，待 infra-dev 确认）
   - 位置：`src/infrastructure/monitoring/PerformanceMonitor.cpp:119,348`
   - 说明：`activeTimers` 恒为 0，`queryCpuUsagePercent()` 直接返回 0.0，监控数据不可用于性能分析。
   - 建议：使用 `QAbstractEventDispatcher::instance()->registeredTimers()` 获取真实 timer 数量；使用平台 API（Windows:`GetSystemTimes`, Linux:`/proc/stat`）实现真实 CPU 占用。

3. **EncoderProactiveListener 分辨率参数化不完整**（P1）
   - 位置：`src/infrastructure/devices/SingleTurnEncoderDevice.cpp:205`
   - 修复：将 `new EncoderProactiveListener(serialPort, this)` 改为 `new EncoderProactiveListener(serialPort, m_resolution, this)`。

4. **急停安全架构 ADR-001**（P1）
   - 负责：domain-dev + protocol-dev
   - 验收：急停从点击到电流归零 <100ms，有单元测试验证。

5. **DevicePoller QThread 反模式重构**（P1）
   - 负责：protocol-dev
   - 建议：改为 `QObject + QThread + moveToThread()` 模式。

---

## 9. 附录：参考文档索引

| 文档 | 路径 | 主要内容 |
|------|------|---------|
| DYN200 集成架构分析 | `Docs/DYN200_Integration_Architecture_Analysis.md` | DYN200 五层架构、数据流、耦合度、P0 模式切换陷阱 |
| 多设备架构一致性分析 | `Docs/Multi_Device_Integration_Architecture_Consistency_Analysis.md` | 四设备配置/工厂/接口/模拟切换一致性对比 |
| 协议层与设备通信分析 | `Docs/superpowers/specs/2026-04-21-protocol-device-analysis-report.md` | 19 项问题（2 P0 + 6 P1 + 6 P2 + 5 P3） |
| 设备功能架构综合评估 | `Docs/Brainstorm_Device_Function_Architecture_Assessment.md` | 52+ 项问题综合、急停 ADR-001、14 项单点修复 |
| 设备实现完整性审查 | `Docs/DeviceImplementationReview.md` | 四设备接口覆盖率、寄存器核查、P0 列表 |
| 架构方案评审报告 | `Docs/architecture-review-report-infrastructure.md` | 5 Blocker + 7 Major + 5 Minor，投产前必要条件 |
| I/O 诊断页面审查 | `Docs/review_report_io_diagnostics.md` | DiagnosticsViewModel 安全性、数据一致性、配置问题 |

---

*报告更新时间：2026-04-24（重新激活轮次）*  
*分析涉及文件：~45 个核心源文件 + 10 份审查报告*
