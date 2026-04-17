# Tasks

## 阶段一：安全与稳定性（紧急）

- [x] Task 1: BrakePowerSupplyDevice 电流安全上限
  - [x] 1.1 在 `setCurrent()` 方法中添加 `constexpr double MAX_CURRENT_A = 5.0` 上限校验
  - [x] 1.2 超限时设置 `m_lastError` 并返回 false，不执行硬件写入
  - [x] 1.3 在 `TestExecutionVerification.cpp` 中添加边界测试用例（正常值、上限值、超限值、负值）

- [x] Task 2: 消除 GearboxTestEngine 中全部 QThread::msleep()
  - [x] 2.1 在 `TestSubState` 枚举中新增延时子状态：`SettlingForwardDelay`、`SettlingReverseDelay`、`SettlingPosition2Delay`、`SettlingPosition1ReturnDelay`、`SettlingPosition3Delay`、`SettlingZeroDelay`、`SettlingLoadForwardDelay`、`SettlingLoadReverseDelay`
  - [x] 2.2 为每个延时子状态实现 handler：检查 `phaseElapsedMs >= 目标延时`，满足则转换到下一状态
  - [x] 2.3 替换 `handleSampleForward()` 中的 `QThread::msleep(500)` → `transitionToSubState(SettlingForwardDelay)`
  - [x] 2.4 替换 `handleSampleReverse()` 中的 `QThread::msleep(500)` → `transitionToSubState(SettlingReverseDelay)`
  - [x] 2.5 替换 `handleMoveToPosition2()` 中的 `QThread::msleep(200)` → `transitionToSubState(SettlingPosition2Delay)`
  - [x] 2.6 替换 `handleMoveBackToPosition1()` 中的 `QThread::msleep(200)` → `transitionToSubState(SettlingPosition1ReturnDelay)`
  - [x] 2.7 替换 `handleMoveToPosition3()` 中的 `QThread::msleep(200)` → `transitionToSubState(SettlingPosition3Delay)`
  - [x] 2.8 替换 `handleMoveBackToZero()` 中的 `QThread::msleep(500)` → `transitionToSubState(SettlingZeroDelay)`
  - [x] 2.9 替换 `handleRampBrakeForward()` 中的 `QThread::msleep(500)` → `transitionToSubState(SettlingLoadForwardDelay)`
  - [x] 2.10 替换 `handleRampBrakeReverse()` 中的 `QThread::msleep(500)` → `transitionToSubState(SettlingLoadReverseDelay)`
  - [ ] 2.11 更新 `TestExecutionVerification.cpp` 确保状态转换测试覆盖新增子状态

- [x] Task 3: 修复 DevicePoller 读取失败时覆盖 buffer
  - [x] 3.1 `MotorPoller::run()` — 仅在 `ok=true` 时写入 `currentA`/`ai1Level`
  - [x] 3.2 `TorquePoller::run()` — 仅在 `ok=true` 时写入 `torqueNm`/`speedRpm`/`powerW`
  - [x] 3.3 `EncoderPoller::run()` — 仅在 `ok=true` 时写入 `angleDeg`/`magnetDetected`
  - [x] 3.4 `BrakePoller::run()` — 仅在 `ok=true` 时写入 `currentA`/`ai1Level`
  - [x] 3.5 所有 Poller — `valid.store(ok)` 替代 `valid.store(true)`

## 阶段二：QML 页面功能修复

- [x] Task 4: 修复 HistoryPage.qml
  - [x] 4.1 声明 `property string selectedRecordId: ""`
  - [x] 4.2 `selectRecord()` 函数同时设置 `selectedRecordId` 和 `selectedRecordIndex`
  - [x] 4.3 详情面板所有字段绑定到 `currentRecord()` 的真实数据（替换硬编码字符串）
  - [x] 4.4 `deleteRecord()` 调用 `historyService.deleteRecord(selectedRecordId)` 持久化删除
  - [x] 4.5 `HistoryService` 添加 `deleteRecord(id)` 方法（从 JSONL 文件中删除指定行）
  - [x] 4.6 筛选功能连接到 ListView — `ListView.model` 绑定到筛选后的模型

- [x] Task 5: 修复 RecipePage.qml
  - [x] 5.1 编辑输入框添加 `onTextChanged` 回写到临时编辑对象
  - [x] 5.2 `saveRecipe()` 从临时编辑对象取值而非 `currentRecipe()`
  - [x] 5.3 修复新建配方流程 — `createNewRecipe()` 创建临时空对象，`saveRecipe()` 支持新建模式
  - [x] 5.4 修复角度序列拼接 Bug — 第 3 个位置使用正确字段（`position2ToleranceDeg`）
  - [x] 5.5 描述字段从 `fileName` 改为 `description`
  - [x] 5.6 `saveRecipe()` 检查 `recipeService.save()` 返回值，失败时提示用户
  - [x] 5.7 保存成功后同步更新 `recipeListModel` 中对应条目

- [x] Task 6: DiagnosticsPage.qml 安全防护
  - [x] 6.1 电机正/反转按钮添加确认对话框（使用 `AppAlertDialog` 组件）
  - [x] 6.2 制动电流输入添加前端范围校验（0~5.0A），显示错误提示

- [x] Task 7: DeviceConfigPage.qml 范围校验
  - [x] 7.1 `slaveId` 输入添加 `onTextChanged` 校验，限制 1-247 范围
  - [ ] 7.2 使用 `SpinBox` 替代自由输入的 `AppInput`（可选方案，暂不实施）

## 阶段三：工程配置可移植性

- [ ] Task 8: CMakeLists.txt 路径修复
  - [ ] 8.1 `QML_SMOKE_TEST_PATH` 改为从环境变量 `QT_HOST_PATH` 或 CMake 变量获取
  - [ ] 8.2 MACOSX_BUNDLE 设置改为 `if(APPLE)` 条件化
  - [ ] 8.3 测试文件 `QmlSmokeTests.cpp` 中硬编码 `F:/Work/...` 路径改为相对路径或临时目录

## 阶段四：编译与回归验证

- [ ] Task 9: 编译验证
  - [ ] 9.1 确保 CMake 配置成功
  - [ ] 9.2 确保全部 C++ 源码编译通过
  - [ ] 9.3 运行 `TestExecutionVerification` 全部测试用例通过
  - [ ] 9.4 运行 QML 冒烟测试通过

# Task Dependencies
- [Task 2] 无外部依赖（但改动量最大，建议优先完成）
- [Task 1] 无外部依赖
- [Task 3] 无外部依赖
- [Task 4] 依赖 [Task 4.5]（HistoryService 需先添加 deleteRecord）
- [Task 5] 无外部依赖
- [Task 6] 建议在 [Task 1] 之后（C++ 端上限就绪后前端也加防护）
- [Task 7] 无外部依赖
- [Task 8] 无外部依赖
- [Task 9] depends on [Task 1] ~ [Task 8]
