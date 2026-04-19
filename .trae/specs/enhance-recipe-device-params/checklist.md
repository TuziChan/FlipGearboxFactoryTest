# Checklist

## 设备接口层
- [x] IBrakePowerDevice.h 新增 setVoltage、readVoltage、readPower、readMode 纯虚方法
- [x] BrakePowerSupplyDevice 新增电压设定寄存器 REG_CH1/CH2_SET_VOLTAGE
- [x] BrakePowerSupplyDevice 新增电压回读寄存器 REG_CH1/CH2_READ_VOLTAGE
- [x] BrakePowerSupplyDevice 新增功率回读寄存器 REG_CH1/CH2_READ_POWER
- [x] BrakePowerSupplyDevice 新增模式读取寄存器 REG_MODE
- [x] setVoltage 实现安全限幅（0-24V），超限返回 false
- [x] readVoltage 正确解码 Input Register（×0.01V）
- [x] readPower 正确解码 Input Register（×0.01W）
- [x] readMode 正确返回 1=CV / 0=CC
- [x] MockBrakePowerDevice 实现所有新增接口方法

## 测试用例
- [x] 电压正常范围设定验证（0.0, 5.0, 12.0, 23.99, 24.0V 被接受）
- [x] 电压超限拒绝验证（24.01, 30.0, 100.0V 被拒绝）
- [x] 电压负值拒绝验证（-0.01, -1.0V 被拒绝）
- [x] 功率回读验证
- [x] 模式读取验证
- [x] 配方序列化新增字段往返测试（JSON -> TestRecipe -> JSON 一致）
- [x] 旧配方加载兼容性测试（缺少新字段时使用默认值）

## 数据模型
- [x] TestRecipe.h 新增 brakeMode 字段（默认 "CC"）
- [x] TestRecipe.h 新增 brakeRampStartVoltage 字段（默认 0.0）
- [x] TestRecipe.h 新增 brakeRampEndVoltage 字段（默认 12.0）
- [x] RecipeConfig::fromJson 支持读取新字段，缺失时使用默认值
- [x] RecipeConfig::toJson 支持写入新字段

## 测试引擎
- [x] GearboxTestEngine CC 模式行为与现有完全一致（回归）
- [x] GearboxTestEngine CV 模式使用电压斜坡代替电流斜坡
- [x] acquireTelemetry 增加制动电压和功率采集

## 配方 UI
- [x] RecipePage.qml 展示全部归零参数（4个）
- [x] RecipePage.qml 展示全部空载参数（21个：5个时序 + 16个限值）
- [x] RecipePage.qml 展示全部角度参数（9个）
- [x] RecipePage.qml 展示全部负载参数（18个：2个通用 + 3个制动模式 + 4个斜坡 + 4个锁止 + 8个限值）
- [x] 制动模式切换时 CC/CV 参数联动显示/隐藏
- [x] 参数分组可折叠
- [x] 新建配方默认值正确
- [x] 编辑已有配方加载所有字段正确
- [x] 保存配方序列化所有字段正确
- [x] 导入配方映射所有字段正确

## 诊断页
- [x] DiagnosticsViewModel 新增 setBrakeVoltage 方法
- [x] DiagnosticsViewModel buildBrakeStatus 包含电压、功率、模式信息
- [x] DiagnosticsPage.qml 增加电压输入框和设置按钮

## 构建验证
- [x] 全部测试通过（QT_QPA_PLATFORM=offscreen）
- [x] 构建编译通过无错误
