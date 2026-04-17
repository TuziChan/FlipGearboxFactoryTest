# Tasks

## 阶段一：后端服务搭建

- [ ] Task 1: 创建 RecipeService 配方文件管理服务
  - [ ] 1.1 创建 `src/infrastructure/services/RecipeService.h` — 定义 `RecipeService` 类，提供 `loadAll()`, `save()`, `remove()`, `exportTo()`, `importFrom()` 方法
  - [ ] 1.2 创建 `src/infrastructure/services/RecipeService.cpp` — 实现扫描 `config/recipes/*.json`，使用 `RecipeConfig::fromJson()` 解析，`RecipeConfig::toJson()` 序列化
  - [ ] 1.3 暴露为 QML context property `recipeService`

- [ ] Task 2: 创建 HistoryService 历史记录持久化服务
  - [ ] 2.1 创建 `src/infrastructure/services/HistoryService.h` — 定义 `HistoryService` 类，提供 `loadAll()`, `addRecord()`, `exportRecord()`, `exportAll()`, `filteredModel()` 方法
  - [ ] 2.2 创建 `src/infrastructure/services/HistoryService.cpp` — 使用 `data/history.jsonl`（JSON Lines 格式）追加写入，启动时全量加载
  - [ ] 2.3 在 `GearboxTestEngine::onTestCompleted` 中调用 `HistoryService::addRecord()` 自动保存测试结果
  - [ ] 2.4 暴露为 QML context property `historyService`

## 阶段二：RecipePage 真实化

- [ ] Task 3: 重构 RecipePage.qml — 数据源和表单绑定
  - [ ] 3.1 移除 `recipeListModel` 的 `Component.onCompleted` 中全部 3 个硬编码配方
  - [ ] 3.2 添加 `Component.onCompleted` 调用 `recipeService.loadAll()` 填充 `recipeListModel`
  - [ ] 3.3 表单所有字段（占空比、转速、电流、扭矩、角度序列、公差等）绑定到 `recipeListModel.get(selectedIndex)` 的对应属性
  - [ ] 3.4 实现 `saveRecipe()` 调用 `recipeService.save()`
  - [ ] 3.5 实现 `exportRecipe()` 调用 `recipeService.exportTo()`
  - [ ] 3.6 实现 `importRecipe()` 调用 `recipeService.importFrom()`

## 阶段三：HistoryPage 真实化

- [ ] Task 4: 重构 HistoryPage.qml — 数据源和详情绑定
  - [ ] 4.1 移除 `historyModel` 的 `Component.onCompleted` 中全部 5 条硬编码记录
  - [ ] 4.2 添加 `Component.onCompleted` 调用 `historyService.loadAll()` 填充 `historyModel`
  - [ ] 4.3 详情面板所有字段（SN、配方名、判定、操作员、时间、各项指标）绑定到选中记录的数据
  - [ ] 4.4 实现 `getFilteredModel()` 调用 `historyService.filteredModel()` 或在前端根据 filterVerdict/filterDateRange 过滤
  - [ ] 4.5 实现 `exportRecord()` 调用 `historyService.exportRecord()`
  - [ ] 4.6 实现 `exportAll()` 调用 `historyService.exportAll()`

## 阶段四：TestExecutionPage 真实化

- [ ] Task 5: 重构 TestExecutionPage.qml — 动态测试模型
  - [ ] 5.1 移除 `initializeModels()` 中硬编码的 `steps` 数组，从 ViewModel 的当前配方动态生成步骤列表
  - [ ] 5.2 移除硬编码的 `angleRows` 数组，从配方的 `anglePositions` 和 `angleTolerance` 动态生成
  - [ ] 5.3 移除硬编码的 `loadModel`，从配方的负载测试参数动态生成
  - [ ] 5.4 移除硬编码的 `phaseDurations` 数组，从配方获取各阶段预估时长
  - [ ] 5.5 移除 `commandBar.serialNumber = "GBX42A-20260417-001"` 和 `commandBar.backlashValue = "1.2"` 默认值
  - [ ] 5.6 `ai2Value` 绑定到 ViewModel 的真实 AI2 属性

## 阶段五：DeviceConfigPage 真实化

- [ ] Task 6: 重构 DeviceConfigPage.qml — 真实连接测试
  - [ ] 6.1 `testConnection()` 改为调用 `deviceConfigService.testConnection(portName)` — 在 C++ 端尝试打开串口并发送 Modbus 读命令
  - [ ] 6.2 `testDevice()` 改为调用 `deviceConfigService.testDevice(deviceKey)` — 尝试读取设备的基本寄存器验证从站响应

## 阶段六：小修复与 ViewModel 改进

- [ ] Task 7: 修复 DiagnosticsPage 和 TestExecutionViewModel 硬编码
  - [ ] 7.1 DiagnosticsPage.qml: `refreshInterval` 从配置获取
  - [ ] 7.2 DiagnosticsPage.qml: 制动电流输入框默认值从配方获取
  - [ ] 7.3 TestExecutionViewModel.cpp: 默认型号从 `StationConfig.defaultRecipe` 获取

## 阶段七：编译与测试验证

- [ ] Task 8: 更新 CMakeLists.txt 并编译验证
  - [ ] 8.1 将 `RecipeService.h/.cpp` 和 `HistoryService.h/.cpp` 添加到 CMakeLists.txt
  - [ ] 8.2 更新测试用例覆盖新服务
  - [ ] 8.3 使用 `run.bat` 编译验证通过

# Task Dependencies
- [Task 3] depends on [Task 1]
- [Task 4] depends on [Task 2]
- [Task 5] depends on [Task 1] (需要配方数据结构)
- [Task 6] 无外部依赖
- [Task 7] 无外部依赖
- [Task 8] depends on [Task 1] ~ [Task 7]
