# Qt6 最新特性升级 — UI 组件全面现代化

## Why
项目使用 Qt 6.11.0，但 UI 组件仍大量采用 Qt5 时代的 `MouseArea`、手动过滤、手写搜索等模式。Qt 6.8-6.10 引入了 `TapHandler`/`HoverHandler`、`SearchField`、`SortFilterProxyModel`、`Synchronizer`、`ApplicationWindow` 等新特性，采用它们可以减少代码量、提升运行时性能、改善可访问性。

## What Changes
- 全部交互组件的 `MouseArea` → `TapHandler` + `HoverHandler`
- `AppCombobox` 内部搜索逻辑 → `SearchField` + `SortFilterProxyModel`（Qt 6.10）
- `HistoryPage` 手写过滤 → `SortFilterProxyModel`（Qt 6.10）
- `Main.qml` 的 `Window` → `ApplicationWindow`
- `AppShell` 页面全量实例化 → `Loader` 懒加载
- `AppCollapsible` 补齐 `pragma ComponentBehavior: Bound`
- `AppSlider` 的 `_syncing` 防抖 → `Synchronizer`（Qt 6.10 Tech Preview）
- `AppProgress` 增加 `indeterminate` 模式
- `AppScrollArea` 简化，使用 `ScrollView` 替代手动 `Flickable` + `ScrollBar`

## Impact
- Affected specs: 无其他 spec 受影响
- Affected code:
  - `src/ui/components/AppButton.qml` — MouseArea 替换
  - `src/ui/components/AppSwitch.qml` — MouseArea 替换
  - `src/ui/components/AppCheckbox.qml` — MouseArea 替换
  - `src/ui/components/AppSelect.qml` — MouseArea 替换 + delegate 内 MouseArea
  - `src/ui/components/AppCombobox.qml` — SearchField + SortFilterProxyModel 重写
  - `src/ui/components/AppSlider.qml` — Synchronizer 双向绑定
  - `src/ui/components/AppProgress.qml` — indeterminate 模式
  - `src/ui/components/AppScrollArea.qml` — ScrollView 简化
  - `src/ui/components/AppAccordion.qml` — MouseArea 替换
  - `src/ui/components/AppCarousel.qml` — MouseArea 替换
  - `src/ui/components/AppCollapsible.qml` — 补 pragma + MouseArea 替换
  - `src/ui/components/AppDropdownMenu.qml` — delegate 内 MouseArea
  - `src/ui/components/AppContextMenu.qml` — delegate 内 MouseArea
  - `src/ui/components/AppNavigationMenu.qml` — MouseArea 替换
  - `src/ui/components/AppHoverCard.qml` — 无交互变更
  - `src/ui/components/AppPopover.qml` — 无交互变更
  - `src/ui/components/AppTooltip.qml` — 无交互变更
  - `src/ui/components/AppDrawer.qml` — overlay MouseArea 替换
  - `src/ui/components/AppSheet.qml` — overlay MouseArea 替换
  - `src/ui/components/AppDialog.qml` — overlay MouseArea 替换
  - `src/ui/components/ChartPanel.qml` — MouseArea 替换
  - `src/ui/components/SidebarNav.qml` — MouseArea 替换
  - `src/ui/components/SidebarNavItem.qml` — MouseArea 替换
  - `src/ui/components/FlowList.qml` — 可能有 MouseArea
  - `src/ui/components/FlowStepItem.qml` — 可能有 MouseArea
  - `src/ui/components/VerdictPanel.qml` — 无交互变更
  - `src/ui/components/MetricCard.qml` — 无交互变更
  - `src/ui/components/SectionCard.qml` — 无交互变更
  - `src/ui/components/InspectorSection.qml` — 可能有 MouseArea
  - `src/ui/components/ThemeSwitcherPanel.qml` — 可能有 MouseArea
  - `src/ui/components/CommandBar.qml` — 无交互变更
  - `src/ui/components/BottomStatusBar.qml` — 无交互变更
  - `src/ui/components/TopToolbar.qml` — 无交互变更
  - `src/ui/components/AlertStrip.qml` — 可能有 MouseArea
  - `src/ui/components/AppTable.qml` — delegate 内 MouseArea
  - `src/ui/pages/HistoryPage.qml` — SortFilterProxyModel 替换
  - `src/ui/pages/AppShell.qml` — Loader 懒加载
  - `Main.qml` — Window → ApplicationWindow

## ADDED Requirements

### Requirement: TapHandler/HoverHandler 交互统一
系统 SHALL 在所有可交互组件中使用 `TapHandler` 和 `HoverHandler` 替代 `MouseArea`，以获得更好的事件传播、触控支持和可访问性。

#### Scenario: 按钮点击
- **WHEN** 用户点击 `AppButton`
- **THEN** `TapHandler.onTapped` 触发 `clicked` 信号，且不影响父级事件传播

#### Scenario: 悬停状态
- **WHEN** 鼠标悬停在 `AppButton` 上
- **THEN** `HoverHandler` 的 `hovered` 属性变为 `true`

#### Scenario: delegate 内点击
- **WHEN** 用户点击 `AppSelect` / `AppDropdownMenu` / `AppContextMenu` 的列表项
- **THEN** `TapHandler` 正确触发 `selectIndex`，且支持键盘导航

### Requirement: SearchField 搜索控件
`AppCombobox` SHALL 使用 Qt 6.10 的 `SearchField` 控件替代手写的 `TextField` + `ListView` 搜索逻辑，减少代码量并使用原生搜索体验。

#### Scenario: 搜索过滤
- **WHEN** 用户在 `AppCombobox` 中输入搜索文本
- **THEN** 下拉列表仅显示匹配项

#### Scenario: 选择确认
- **WHEN** 用户从搜索结果中选中一项
- **THEN** `currentIndex`、`currentText` 更新，`activated` 信号触发，弹出框关闭

### Requirement: SortFilterProxyModel 声明式过滤
`HistoryPage` SHALL 使用 Qt 6.10 的 `SortFilterProxyModel` + `FunctionFilter` 替代手写 `getFilteredModel()` 函数。

#### Scenario: 按判定过滤
- **WHEN** 用户选择过滤条件（全部/合格/不合格）
- **THEN** `SortFilterProxyModel` 自动过滤 `historyModel`，ListView 显示过滤后结果

#### Scenario: 文本搜索
- **WHEN** 用户在搜索框输入 SN 或配方名称
- **THEN** `FunctionFilter` 实时过滤，无需重建 ListModel

### Requirement: ApplicationWindow
`Main.qml` SHALL 使用 `ApplicationWindow` 替代 `Window`，以获得内置的 `header`/`footer`/`Overlay` 支持和与 `Popup`/`Dialog` 更好的配合。

#### Scenario: 弹出框层级
- **WHEN** 在页面中打开 `AppDialog` 或 `AppDrawer`
- **THEN** 弹出框正确显示在内容之上，且背景遮罩正常工作

### Requirement: 页面懒加载
`AppShell` SHALL 使用 `Loader` + `StackLayout.isCurrentItem` 实现页面懒加载，避免启动时实例化所有页面。

#### Scenario: 首次加载
- **WHEN** 应用启动
- **THEN** 仅实例化当前活跃页面（默认为 "测试执行" 页面）

#### Scenario: 页面切换
- **WHEN** 用户点击导航切换到新页面
- **THEN** 新页面首次被实例化并显示

### Requirement: pragma ComponentBehavior 一致性
所有 QML 组件 SHALL 包含 `pragma ComponentBehavior: Bound`，以启用 Qt6 编译优化。

#### Scenario: 缺失 pragma
- **WHEN** 检查所有 `.qml` 文件
- **THEN** 每个组件文件第一行均为 `pragma ComponentBehavior: Bound`

### Requirement: Synchronizer 双向绑定
`AppSlider` SHALL 使用 Qt 6.10 的 `Synchronizer` 替代 `_syncing` 标志位防抖逻辑。

#### Scenario: 外部值变更
- **WHEN** 外部代码设置 `AppSlider.value`
- **THEN** 滑块位置同步更新，不触发循环

#### Scenario: 用户拖动
- **WHEN** 用户拖动滑块
- **THEN** `value` 和 `values` 属性同步更新，`valueCommitted` 信号触发一次

### Requirement: AppProgress 不确定模式
`AppProgress` SHALL 支持 `indeterminate` 属性，当 `indeterminate: true` 时显示无限循环动画。

#### Scenario: 确定模式
- **WHEN** `indeterminate: false`
- **THEN** 显示 0 到 `maximum` 的进度条

#### Scenario: 不确定模式
- **WHEN** `indeterminate: true`
- **THEN** 显示来回循环的进度条动画

### Requirement: AppScrollArea 简化
`AppScrollArea` SHALL 使用 `ScrollView` 替代手动 `Flickable` + `ScrollBar` 组合，减少代码量。

#### Scenario: 内容滚动
- **WHEN** 内容超出视口高度
- **THEN** 自动显示垂直滚动条，支持鼠标滚轮和触摸滚动
