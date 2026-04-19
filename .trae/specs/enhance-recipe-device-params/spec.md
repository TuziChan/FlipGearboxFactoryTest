# 增强配方设备参数控制 Spec

## Why
当前配方 UI 仅暴露了 TestRecipe 数据模型约 30% 的参数，且制动电源设备（双通道数控电源 SYDP-24x2）仅支持恒流模式下的电流设定，硬件手册中的电压设定/回读、功率回读、恒压模式等能力完全未实现。测试工程师无法在配方中精细控制制动器电源的完整参数，也无法选择恒压模式进行测试。

## What Changes
- **扩展 IBrakePowerDevice 接口**：增加 `setVoltage()`、`readVoltage()`、`readPower()`、`setBrakeMode()`、`readBrakeMode()` 方法
- **扩展 BrakePowerSupplyDevice 实现**：实现电压设定（Holding Register 0x0000/0x0002）、电压回读（Input Register 0x0000/0x0003）、功率回读（Input Register 0x0002/0x0005）、模式读取（Input Register 0x0009）
- **扩展 TestRecipe 数据模型**：新增制动电源工作模式（恒流/恒压）、制动电压设定字段
- **扩展 RecipeConfig 序列化**：支持新增字段的 JSON 读写
- **扩展 RecipePage.qml 配方 UI**：将 TestRecipe 全部 ~50 个参数分组展示，包括归零、空载、角度、负载/制动四大参数组
- **扩展 GearboxTestEngine**：在负载测试阶段根据配方中制动模式参数选择恒流或恒压控制策略
- **扩展 DiagnosticsPage/DiagnosticsViewModel**：增加电压设置和模式读取的调试能力
- **扩展 MockBrakePowerDevice**：在测试中 mock 新增接口方法
- **更新测试用例**：验证新增参数的序列化、设备控制、UI 交互

## Impact
- Affected specs: TestRecipe 配方数据模型、IBrakePowerDevice 设备接口、GearboxTestEngine 测试引擎
- Affected code:
  - `src/domain/TestRecipe.h` — 新增字段
  - `src/domain/GearboxTestEngine.h/cpp` — 使用新制动参数
  - `src/infrastructure/devices/IBrakePowerDevice.h` — 接口扩展
  - `src/infrastructure/devices/BrakePowerSupplyDevice.h/cpp` — 实现扩展
  - `src/infrastructure/config/RecipeConfig.cpp` — 序列化扩展
  - `src/ui/pages/RecipePage.qml` — UI 全量参数展示
  - `src/viewmodels/DiagnosticsViewModel.h/cpp` — 调试页扩展
  - `src/ui/pages/DiagnosticsPage.qml` — 调试页 UI 扩展
  - `tests/TestExecutionVerification.cpp` — 测试用例扩展

## ADDED Requirements

### Requirement: 制动电源恒压模式支持
系统 SHALL 支持制动电源设备在恒流（CC）和恒压（CV）两种模式下工作。

#### Scenario: 配方选择恒流模式（默认）
- **WHEN** 配方中 `brakeMode` 为 "CC"
- **THEN** GearboxTestEngine 在负载测试阶段使用 `brakeRampStartCurrentA` 至 `brakeRampEndCurrentA` 的线性电流斜坡

#### Scenario: 配方选择恒压模式
- **WHEN** 配方中 `brakeMode` 为 "CV"
- **THEN** GearboxTestEngine 在负载测试阶段使用 `brakeRampStartVoltage` 至 `brakeRampEndVoltage` 的线性电压斜坡

#### Scenario: 恒压模式参数缺失
- **WHEN** 配方选择了恒压模式但未设定电压范围
- **THEN** 系统使用默认电压参数（起始 0.0V，终止 12.0V）

### Requirement: 制动电源设备接口扩展
IBrakePowerDevice 接口 SHALL 扩展以下方法：
- `setVoltage(int channel, double voltageV)` — 设定输出电压（0-24V）
- `readVoltage(int channel, double& voltageV)` — 读取实际输出电压
- `readPower(int channel, double& powerW)` — 读取实际输出功率
- `readMode(int channel, int& mode)` — 读取当前工作模式（1=恒压，0=恒流）

#### Scenario: 电压安全限幅
- **WHEN** 调用 `setVoltage()` 传入超过 24.0V 的值
- **THEN** 方法返回 false 并设置错误信息

#### Scenario: 功率回读
- **WHEN** 调用 `readPower()`
- **THEN** 返回 Input Register 中的功率值（×0.01W）

### Requirement: 配方数据模型新增字段
TestRecipe 结构体 SHALL 新增以下字段：
- `brakeMode`（QString）— 制动模式 "CC" 或 "CV"，默认 "CC"
- `brakeRampStartVoltage`（double）— 恒压斜坡起始电压，默认 0.0V
- `brakeRampEndVoltage`（double）— 恒压斜坡终止电压，默认 12.0V

#### Scenario: 向后兼容旧配方
- **WHEN** 加载不包含 `brakeMode` 字段的旧 JSON 配方文件
- **THEN** `brakeMode` 默认为 "CC"，`brakeRampStartVoltage` 默认为 0.0，`brakeRampEndVoltage` 默认为 12.0

### Requirement: 配方 UI 全量参数暴露
RecipePage.qml SHALL 将 TestRecipe 中的全部参数按以下分组展示：

**归零参数组**：归零占空比(%)、前进占空比(%)、编码器零点角度(°)、归零超时(ms)

**空载测试参数组**：空载占空比(%)、正转启动稳定时间(ms)、正转采样窗口(ms)、反转启动稳定时间(ms)、反转采样窗口(ms)、正转电流平均Min/Max(A)、正转电流峰值Min/Max(A)、正转转速平均Min/Max(RPM)、正转转速峰值Min/Max(RPM)、反转对应参数对称

**角度定位参数组**：角度测试占空比(%)、位1目标角度(°)、位1容差(°)、位2目标角度(°)、位2容差(°)、位3目标角度(°)、位3容差(°)、回零容差(°)、角度超时(ms)

**负载测试参数组**：负载占空比(%)、电机稳定时间(ms)、制动模式(CC/CV)、制动起始电流(A)、制动终止电流(A)、制动起始电压(V)、制动终止电压(V)、斜坡时间(ms)、锁止转速阈值(RPM)、锁止角度窗口(ms)、锁止角度变化量(°)、锁止确认时长(ms)、正转电流Min/Max(A)、正转扭矩Min/Max(N·m)、反转电流Min/Max(A)、反转扭矩Min/Max(N·m)

#### Scenario: 恒压模式参数联动
- **WHEN** 用户在配方 UI 中选择制动模式为 "CV"
- **THEN** 显示制动电压参数（起始/终止电压），隐藏电流参数
- **WHEN** 用户选择 "CC"
- **THEN** 显示制动电流参数（起始/终止电流），隐藏电压参数

#### Scenario: 参数分组可折叠
- **WHEN** 配方参数分组过多导致页面过长
- **THEN** 每个参数组可通过标题栏点击展开/折叠

### Requirement: 诊断页制动电源增强
DiagnosticsPage.qml 的制动电源控制区域 SHALL 增加电压输入框和设置按钮，并显示当前工作模式。

#### Scenario: 诊断页设置电压
- **WHEN** 用户在诊断页输入电压值并点击"设置电压"
- **THEN** DiagnosticsViewModel 调用 `setVoltage()` 并显示操作日志

### Requirement: 测试用例扩展
TestExecutionVerification SHALL 新增以下测试：
- 恒压模式电压设定正常范围
- 恒压模式电压设定超限拒绝
- 恒压模式电压设定负值拒绝
- 功率回读验证
- 模式读取验证
- 配方序列化新增字段往返测试

## MODIFIED Requirements

### Requirement: BrakePowerSupplyDevice 寄存器映射
制动电源设备实现 SHALL 新增以下寄存器常量：
- `REG_CH1_SET_VOLTAGE = 0x0000`（Holding Register，×0.01V）
- `REG_CH2_SET_VOLTAGE = 0x0002`（Holding Register，×0.01V）
- `REG_CH1_READ_VOLTAGE = 0x0000`（Input Register，×0.01V）
- `REG_CH2_READ_VOLTAGE = 0x0003`（Input Register，×0.01V）
- `REG_CH1_READ_POWER = 0x0002`（Input Register，×0.01W）
- `REG_CH2_READ_POWER = 0x0006`（Input Register，×0.01W）
- `REG_MODE = 0x0009`（Input Register，1=CV，0=CC）

### Requirement: GearboxTestEngine 负载测试阶段
负载测试阶段 SHALL 根据配方 `brakeMode` 字段决定控制策略：
- CC 模式：沿用现有 `brakeRampStartCurrentA` → `brakeRampEndCurrentA` 电流斜坡
- CV 模式：使用 `brakeRampStartVoltage` → `brakeRampEndVoltage` 电压斜坡

### Requirement: RecipeConfig 序列化
RecipeConfig 的 `fromJson()` 和 `toJson()` SHALL 支持新增的 `brakeMode`、`brakeRampStartVoltage`、`brakeRampEndVoltage` 字段。
