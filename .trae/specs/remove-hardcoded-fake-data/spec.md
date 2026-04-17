# 移除所有页面硬编码假数据 Spec

## Why
除 ComponentGalleryPage 外，5 个功能页面中存在大量硬编码假数据（配方参数、历史记录、设备配置、测试步骤/角度序列等）和未实现的功能操作（保存/导入/导出/过滤）。这些假数据掩盖了真实业务逻辑，使页面在无硬件环境下展示虚假的正常状态，可能导致错误的产品判断。

## What Changes
- **RecipePage.qml**: 移除 3 个硬编码配方，改为从 `config/recipes/` 目录动态加载 JSON 配方文件；实现保存/导入/导出功能；表单字段绑定到选中配方数据
- **HistoryPage.qml**: 移除 5 条硬编码历史记录，创建 `HistoryService` 后端服务提供持久化存储和查询；详情面板绑定到选中记录；实现过滤和导出功能
- **TestExecutionPage.qml**: 移除硬编码的步骤列表、角度序列、负载测试参数、默认 SN 和回差值，全部从当前配方动态生成；AI2 通道连接到 ViewModel
- **DeviceConfigPage.qml**: `testConnection()` 和 `testDevice()` 改为调用真实的串口连接验证
- **DiagnosticsPage.qml**: 刷新间隔和制动电流默认值从配置获取
- **TestExecutionViewModel.cpp**: 默认型号从配置文件获取而非硬编码 `"GBX-42A"`

## Impact
- Affected pages: RecipePage, HistoryPage, TestExecutionPage, DeviceConfigPage, DiagnosticsPage
- Affected viewmodels: TestExecutionViewModel, DiagnosticsViewModel
- New infrastructure: `HistoryService` (历史记录持久化服务), `RecipeService` (配方文件管理服务)
- Affected code: 配方加载逻辑、历史记录存储逻辑、测试步骤生成逻辑

## ADDED Requirements

### Requirement: 配方文件动态加载
The system SHALL 从 `config/recipes/` 目录动态加载所有 `.json` 配方文件作为配方列表数据源。

#### Scenario: 配方目录有多个文件
- **WHEN** RecipePage 加载
- **THEN** 系统扫描 `config/recipes/*.json`，解析每个文件为配方对象，展示在列表中

#### Scenario: 配方目录为空
- **WHEN** `config/recipes/` 目录不存在或为空
- **THEN** 配方列表显示空状态提示，不显示任何硬编码数据

### Requirement: 配方 CRUD 操作
The system SHALL 提供配方的创建、编辑、保存、导入、导出功能。

#### Scenario: 保存配方
- **WHEN** 用户编辑配方参数后点击保存
- **THEN** 系统将修改后的配方数据序列化为 JSON 并写入对应文件

#### Scenario: 导出配方
- **WHEN** 用户点击导出
- **THEN** 系统弹出文件保存对话框，将配方 JSON 写入用户指定路径

#### Scenario: 导入配方
- **WHEN** 用户点击导入并选择文件
- **THEN** 系统解析 JSON 文件，验证格式，添加到配方列表并保存到 `config/recipes/`

### Requirement: 配方表单数据绑定
The system SHALL 配方编辑表单的所有字段绑定到当前选中配方的数据。

#### Scenario: 选中配方后表单填充
- **WHEN** 用户在配方列表中选中一个配方
- **THEN** 表单中所有字段（占空比、转速、电流、扭矩、角度序列、公差等）自动填充为该配方的实际参数值

### Requirement: 历史记录持久化
The system SHALL 将每次测试结果持久化存储到本地文件（JSON 格式）。

#### Scenario: 测试完成后保存记录
- **WHEN** 一次测试流程完成（PASS 或 FAIL）
- **THEN** 系统自动将完整测试结果（SN、配方、时间、各项指标、判定）序列化为 JSON 追加到历史记录文件

#### Scenario: HistoryPage 加载历史列表
- **WHEN** 用户切换到历史记录页面
- **THEN** 系统从持久化存储加载所有历史记录并按时间倒序展示

### Requirement: 历史记录详情绑定
The system SHALL 历史记录详情面板的所有字段绑定到当前选中记录的数据。

#### Scenario: 选中历史记录
- **WHEN** 用户在历史列表中选中一条记录
- **THEN** 详情面板显示该记录的真实 SN、配方名、判定、操作员、时间、各项指标数据

### Requirement: 历史记录过滤
The System SHALL 支持按判定结果（PASS/FAIL/ALL）和日期范围过滤历史记录。

#### Scenario: 按判定过滤
- **WHEN** 用户选择过滤条件为 "FAIL"
- **THEN** 列表仅显示判定结果为 FAIL 的记录

### Requirement: 历史记录导出
The system SHALL 支持导出单条记录和批量导出全部记录。

#### Scenario: 导出单条记录
- **WHEN** 用户在历史记录详情面板点击导出
- **THEN** 系统将该记录的完整数据导出为文件

### Requirement: 测试步骤从配方动态生成
The system SHALL 测试执行页的步骤列表、角度测试序列、负载测试参数全部从当前加载的配方动态生成。

#### Scenario: 加载配方后更新测试模型
- **WHEN** 测试执行页加载配方 GBX-42A
- **THEN** 角度测试表根据配方的 `anglePositions` 和 `angleTolerance` 动态生成行；负载测试表根据 `loadTorqueMin` 等参数生成行

### Requirement: 移除硬编码默认 SN 和回差值
The system SHALL 序列号和回差补偿值输入框初始为空，不预设假数据。

#### Scenario: 页面初始加载
- **WHEN** 测试执行页首次加载
- **THEN** 序列号输入框为空，回差补偿值输入框为空

### Requirement: 设备连接测试真实性
The system SHALL 设备配置页的"测试连接"和"测试设备"按钮执行真实的串口连接验证。

#### Scenario: 测试连接成功
- **WHEN** 用户点击"测试连接"且串口可用
- **THEN** 系统尝试打开串口、发送 Modbus 读取命令，根据响应结果显示连接状态

#### Scenario: 测试连接失败
- **WHEN** 用户点击"测试连接"但串口不存在或设备无响应
- **THEN** 显示连接失败状态和错误原因

## MODIFIED Requirements

### Requirement: DiagnosticsPage 刷新间隔
刷新间隔默认值从硬编码 1000ms 改为从 `StationConfig` 或用户偏好获取。

### Requirement: TestExecutionViewModel 默认型号
默认选中型号从硬编码 `"GBX-42A"` 改为从 `StationConfig.defaultRecipe` 获取。

## REMOVED Requirements

### Requirement: RecipePage 硬编码配方数据
**Reason**: 配方应从文件动态加载
**Migration**: 删除 `Component.onCompleted` 中所有硬编码的 ListModel 数据

### Requirement: HistoryPage 硬编码历史记录
**Reason**: 历史记录应从持久化存储加载
**Migration**: 删除 `Component.onCompleted` 中所有硬编码的历史记录数据

### Requirement: TestExecutionPage 硬编码角度/负载测试参数
**Reason**: 测试参数应从配方动态获取
**Migration**: 删除 `angleRows`、`loadModel` 中的硬编码数据
