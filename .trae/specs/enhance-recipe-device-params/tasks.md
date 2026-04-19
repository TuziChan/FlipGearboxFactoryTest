# Tasks

- [x] Task 1: 扩展 IBrakePowerDevice 接口和 BrakePowerSupplyDevice 实现
  - [x] SubTask 1.1: 在 IBrakePowerDevice.h 中新增 setVoltage、readVoltage、readPower、readMode 纯虚方法
  - [x] SubTask 1.2: 在 BrakePowerSupplyDevice.h 中新增电压/功率/模式寄存器常量
  - [x] SubTask 1.3: 在 BrakePowerSupplyDevice.cpp 中实现 setVoltage（含安全限幅 0-24V）、readVoltage、readPower、readMode
  - [x] SubTask 1.4: 在 MockBrakePowerDevice 中实现新增接口方法
  - [x] SubTask 1.5: 编写测试用例验证电压正常范围、超限拒绝、负值拒绝、功率回读、模式读取

- [x] Task 2: 扩展 TestRecipe 数据模型和 RecipeConfig 序列化
  - [x] SubTask 2.1: 在 TestRecipe.h 新增 brakeMode(QString)、brakeRampStartVoltage(double)、brakeRampEndVoltage(double) 字段及默认值
  - [x] SubTask 2.2: 在 RecipeConfig.cpp 的 fromJson/toJson 中添加新字段的序列化/反序列化，确保旧配方向后兼容
  - [x] SubTask 2.3: 编写配方序列化往返测试，验证新增字段和旧配方兼容性

- [x] Task 3: 扩展 GearboxTestEngine 支持恒压模式
  - [x] SubTask 3.1: 在 handleRampBrakeForward/Reverse 中根据 m_recipe.brakeMode 选择电流斜坡或电压斜坡
  - [x] SubTask 3.2: 复用现有方法通过模式分支处理 CV/CC 斜坡
  - [x] SubTask 3.3: 在 acquireTelemetry 中增加制动电压和功率的遥测采集
  - [x] SubTask 3.4: 验证恒流模式行为不变（回归测试通过）

- [x] Task 4: 全量暴露 RecipePage.qml 配方参数 UI
  - [x] SubTask 4.1: 重构 RecipePage.qml 数据模型，将 editingRecipe 从简化字段映射改为完整的 TestRecipe 字段映射
  - [x] SubTask 4.2: 实现归零参数组完整 UI（归零占空比、前进占空比、零点角度、超时）
  - [x] SubTask 4.3: 实现空载测试参数组完整 UI（占空比、正/反转稳定时间和采样窗口、16个限值）
  - [x] SubTask 4.4: 实现角度定位参数组完整 UI（占空比、3个目标角度+容差、回零容差、超时）
  - [x] SubTask 4.5: 实现负载测试参数组完整 UI（占空比、制动模式选择、CC/CV条件参数、斜坡参数、锁止参数、正/反转限值）
  - [x] SubTask 4.6: 实现参数分组可折叠功能（内联实现）
  - [x] SubTask 4.7: 更新 recipeListModel 的 loadAll/append/save 数据映射，与完整字段对齐

- [x] Task 5: 扩展 DiagnosticsViewModel 和 DiagnosticsPage
  - [x] SubTask 5.1: 在 DiagnosticsViewModel 中新增 setBrakeVoltage 方法
  - [x] SubTask 5.2: 在 buildBrakeStatus 中增加电压、功率、模式信息
  - [x] SubTask 5.3: 在 DiagnosticsPage.qml 制动电源区域增加电压输入框和设置按钮

- [x] Task 6: 集成测试与构建验证
  - [x] SubTask 6.1: 运行全部现有测试确保无回归（35 passed, 0 failed）
  - [x] SubTask 6.2: 运行构建确保编译通过（195/195 链接成功）
  - [x] SubTask 6.3: 验证配方 JSON 文件向后兼容加载（testRecipeConfigBackwardCompatibility 通过）

# Task Dependencies
- [Task 2] depends on [Task 1]（TestRecipe 新增字段需要设备接口先定义好）
- [Task 3] depends on [Task 1] and [Task 2]（引擎使用新接口和新配方字段）
- [Task 4] depends on [Task 2]（UI 展示需要数据模型字段确定）
- [Task 5] depends on [Task 1]（诊断页使用新接口方法）
- [Task 6] depends on [Task 1-5] all complete
