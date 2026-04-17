# 全面质量整改计划 Spec

## Why
基于代码审查发现的 4 个严重缺陷、5 个高优先级问题、12 个中低优先级问题，需要系统性修复安全漏洞、消除阻塞调用、补全 QML 页面功能、提升工程规范性。当前最紧迫的风险是：制动电流无上限可能导致设备损坏，状态机中的 `QThread::msleep()` 阻塞急停响应，QML 新页面存在编辑功能完全失效等问题。

## What Changes
- **BrakePowerSupplyDevice**: `setCurrent()` 添加安全上限校验（5A），防止设备过流
- **GearboxTestEngine**: 消除全部 8 处 `QThread::msleep()` 调用，改用非阻塞延时子状态
- **DevicePoller**: 修复读取失败时仍覆盖 buffer 并标记 valid=true 的错误
- **DiagnosticsPage.qml**: 电机控制添加确认对话框，制动电流输入添加前端范围校验
- **HistoryPage.qml**: 修复 `selectedRecordId` 未声明、详情面板硬编码、删除不持久化、筛选未连接
- **RecipePage.qml**: 修复编辑不回写、新建流程不完整、角度拼接 Bug、描述字段绑定错误
- **DeviceConfigPage.qml**: slaveId 输入添加范围校验
- **CMakeLists.txt**: 移除硬编码 `F:/Qt` 路径，条件化 MACOSX_BUNDLE
- **路径管理**: 统一使用 `QStandardPaths` 替代 `cdUp()/cdUp()` 相对路径

## Impact
- Affected code: GearboxTestEngine.cpp, BrakePowerSupplyDevice.cpp, DevicePoller.h, 4 个 QML 页面, CMakeLists.txt, HistoryService.cpp
- Affected behavior: 测试流程时序（msleep → 非阻塞子状态），制动电流上限，QML 页面交互
- **BREAKING**: `GearboxTestEngine` 状态机新增约 6 个延时子状态（SettlingDelay），`TestSubState` 枚举扩展

## ADDED Requirements

### Requirement: 制动电流安全上限
系统 SHALL 在 `BrakePowerSupplyDevice::setCurrent()` 中强制校验电流值不超过 5.0A。超出范围时返回 false 并记录警告日志。

#### Scenario: 正常电流值
- **WHEN** 调用 `setCurrent(1, 3.5)`
- **THEN** 正常执行写入，返回 true

#### Scenario: 超限电流值
- **WHEN** 调用 `setCurrent(1, 8.0)`
- **THEN** 返回 false，`lastError()` 包含超限信息，不执行任何硬件操作

#### Scenario: 负值电流
- **WHEN** 调用 `setCurrent(1, -1.0)`
- **THEN** 返回 false，`lastError()` 包含超限信息

### Requirement: 状态机非阻塞延时
系统 SHALL 使用延时子状态替代所有 `QThread::msleep()` 调用。每个延时点新增一个 `Settling*Delay` 子状态，在定时器回调中检查 `phaseElapsedMs` 达到目标后转换到下一状态。

#### Scenario: 空载正向采样完成后的延时
- **WHEN** 采样完成，需要 500ms 稳定时间再切换到反向
- **THEN** 转入 `SettlingForwardDelay` 子状态，定时器继续运行，500ms 后转入 `SpinupReverse`，期间急停仍然可响应

#### Scenario: 角度位置到达后的延时
- **WHEN** 磁铁事件触发且角度评估完成
- **THEN** 转入 `SettlingPosition*Delay` 子状态，200ms 后转入下一角度位置

#### Scenario: 负载锁定后的延时
- **WHEN** 正向/反向负载锁定确认
- **THEN** 转入 `SettlingLoad*Delay` 子状态，500ms 后转入下一阶段

### Requirement: DevicePoller 读取失败保护
系统 SHALL 在设备读取失败时保留 buffer 中的上一次有效值，仅标记 `valid=false`。

#### Scenario: 设备读取失败
- **WHEN** `readCurrent()` 返回 false
- **THEN** buffer 中 `currentA` 保持上次成功值不变，`valid` 标记为 false，`errorCount` 递增

### Requirement: DiagnosticsPage 安全防护
系统 SHALL 为危险操作提供前端防护层。

#### Scenario: 电机控制确认
- **WHEN** 用户点击"正转"或"反转"按钮
- **THEN** 弹出确认对话框，用户确认后才执行操作

#### Scenario: 制动电流前端校验
- **WHEN** 用户输入制动电流并点击设置
- **THEN** 前端校验值在 0~5.0A 范围内，超出范围显示错误提示且不发送指令

### Requirement: HistoryPage 功能完整性
系统 SHALL 修复 HistoryPage 的所有功能缺陷。

#### Scenario: 选中记录后显示详情
- **WHEN** 用户点击某条历史记录的"查看"按钮
- **THEN** `selectedRecordId` 被正确设置，详情面板显示该记录的真实数据（SN、配方名、判定、时间、各项指标）

#### Scenario: 删除记录持久化
- **WHEN** 用户删除一条记录
- **THEN** 前端 ListModel 和后端存储同时删除，刷新后记录不再出现

#### Scenario: 筛选结果展示
- **WHEN** 用户选择过滤条件
- **THEN** ListView 仅显示符合条件的结果

### Requirement: RecipePage 编辑功能
系统 SHALL 使配方编辑功能完整可用。

#### Scenario: 编辑并保存配方
- **WHEN** 用户进入编辑模式，修改参数，点击保存
- **THEN** 修改后的值正确回写到 ListModel 并通过 RecipeService 持久化到文件

#### Scenario: 新建配方
- **WHEN** 用户点击"新建"并填写参数后保存
- **THEN** 系统创建新配方文件并添加到列表

### Requirement: 工程配置可移植性
系统 SHALL 移除所有硬编码的绝对路径。

#### Scenario: 不同开发者的 Qt 安装路径
- **WHEN** 开发者 A 的 Qt 安装在 `C:/Qt/6.11.0`，开发者 B 安装在 `/opt/Qt/6.11.0`
- **THEN** CMake 配置能自动检测，无需手动修改 CMakeLists.txt

## MODIFIED Requirements

### Requirement: TestSubState 枚举
扩展 `TestSubState` 枚举，新增约 6 个延时子状态：`SettlingForwardDelay`、`SettlingReverseDelay`、`SettlingPosition2Delay`、`SettlingPosition1ReturnDelay`、`SettlingPosition3Delay`、`SettlingLoadForwardDelay`、`SettlingLoadReverseDelay`。

### Requirement: DeviceConfigPage slaveId 校验
`slaveId` 输入限制为 1-247 范围，超出范围自动钳位。

## REMOVED Requirements

### Requirement: GearboxTestEngine 阻塞式延时
**Reason**: 阻塞主线程/事件循环，影响急停响应和 UI 流畅性
**Migration**: 所有 `QThread::msleep()` 替换为非阻塞子状态

### Requirement: DevicePoller 无条件覆盖 buffer
**Reason**: 读取失败时用无效值覆盖上一次有效数据，导致上层获取到错误遥测
**Migration**: 仅在读取成功时更新 buffer 值
