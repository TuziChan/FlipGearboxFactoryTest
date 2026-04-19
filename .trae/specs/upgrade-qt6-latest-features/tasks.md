# Tasks

- [ ] Task 1: AppButton — MouseArea → TapHandler + HoverHandler
  - [ ] 1.1: 读取 `src/ui/components/AppButton.qml`，将底部 `MouseArea` 替换为 `TapHandler { enabled: !root.disabled; onTapped: root.clicked() }` + `HoverHandler { id: hoverHandler; enabled: !root.disabled }`
  - [ ] 1.2: 验证编译通过，确认点击和焦点环行为正常

- [ ] Task 2: AppSwitch — MouseArea → TapHandler
  - [ ] 2.1: 读取 `src/ui/components/AppSwitch.qml`，将 `MouseArea` 替换为 `TapHandler { onTapped: { root.checked = !root.checked; root.toggled(root.checked) } }`
  - [ ] 2.2: 验证编译通过

- [ ] Task 3: AppCheckbox — MouseArea → TapHandler
  - [ ] 3.1: 读取 `src/ui/components/AppCheckbox.qml`，将底部 `MouseArea` 替换为 `TapHandler { enabled: !root.disabled; cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor; onTapped: root.toggle() }`
  - [ ] 3.2: 验证编译通过

- [ ] Task 4: AppSelect — MouseArea → TapHandler/HoverHandler
  - [ ] 4.1: 读取 `src/ui/components/AppSelect.qml`
  - [ ] 4.2: 将 trigger 区域的 `MouseArea` 替换为 `TapHandler { enabled: root.enabled; onTapped: root.openPopup() }`
  - [ ] 4.3: 将 delegate 内的 `MouseArea` 替换为 `TapHandler { onTapped: root.selectIndex(itemDelegate.index) }` + `HoverHandler { onHoveredChanged: if (hovered) root.highlightedIndex = itemDelegate.index }`
  - [ ] 4.4: 验证编译通过

- [ ] Task 5: AppCombobox — SearchField + SortFilterProxyModel 重写
  - [ ] 5.1: 读取 `src/ui/components/AppCombobox.qml`
  - [ ] 5.2: 在 CMakeLists.txt 的 find_package 中确认 Qt6 已包含 QuickControls2（已有）
  - [ ] 5.3: 重写 AppCombobox：使用 `SearchField` 替代手写 TextField + ListView 过滤逻辑，配合 `SortFilterProxyModel` + `FunctionFilter` 做搜索过滤
  - [ ] 5.4: 保留 `activated(int index, string text)` 信号和 `currentIndex`/`currentText` 属性接口不变
  - [ ] 5.5: 验证编译通过

- [ ] Task 6: AppDropdownMenu — delegate MouseArea → TapHandler/HoverHandler
  - [ ] 6.1: 读取 `src/ui/components/AppDropdownMenu.qml`
  - [ ] 6.2: 将 trigger `AppButton` 内部已由 Task 1 处理
  - [ ] 6.3: 将 delegate 内 `MouseArea` 替换为 `TapHandler { onTapped: root.selectIndex(itemDelegate.index) }` + `HoverHandler { onHoveredChanged: if (hovered) root.highlightedIndex = itemDelegate.index }`
  - [ ] 6.4: 验证编译通过

- [ ] Task 7: AppContextMenu — delegate MouseArea → TapHandler/HoverHandler
  - [ ] 7.1: 读取 `src/ui/components/AppContextMenu.qml`
  - [ ] 7.2: 将 delegate 内 `MouseArea` 替换为 `TapHandler { enabled: itemDelegate.optionEnabled; onTapped: root.selectIndex(itemDelegate.index) }` + `HoverHandler { onHoveredChanged: if (hovered) root.highlightedIndex = itemDelegate.index }`
  - [ ] 7.3: 验证编译通过

- [ ] Task 8: AppAccordion — MouseArea → TapHandler
  - [ ] 8.1: 读取 `src/ui/components/AppAccordion.qml`
  - [ ] 8.2: 将 delegate 内 triggerArea `MouseArea` 替换为 `TapHandler { enabled: !itemRoot.disabled; cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor; onTapped: root.toggleIndex(itemRoot.index) }` + `HoverHandler { id: triggerHover; enabled: !itemRoot.disabled }`
  - [ ] 8.3: 更新 `hovered` 属性绑定从 `triggerArea.containsMouse` 到 `triggerHover.hovered`
  - [ ] 8.4: 验证编译通过

- [ ] Task 9: AppCollapsible — 补 pragma + MouseArea → TapHandler
  - [ ] 9.1: 在文件第一行添加 `pragma ComponentBehavior: Bound`
  - [ ] 9.2: 将 triggerArea `MouseArea` 替换为 `TapHandler { enabled: !root.disabled; cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor; onTapped: root.toggle() }` + `HoverHandler { id: triggerHover; enabled: !root.disabled }`
  - [ ] 9.3: 更新 `hovered`/`pressed` 属性绑定
  - [ ] 9.4: 验证编译通过

- [ ] Task 10: AppCarousel — MouseArea → TapHandler
  - [ ] 10.1: 读取 `src/ui/components/AppCarousel.qml`
  - [ ] 10.2: 将 indicator delegate 中 `MouseArea` 替换为 `TapHandler { cursorShape: Qt.PointingHandCursor; onTapped: root.goTo(index) }`
  - [ ] 10.3: 验证编译通过

- [ ] Task 11: AppNavigationMenu — MouseArea → TapHandler
  - [ ] 11.1: 读取 `src/ui/components/AppNavigationMenu.qml`
  - [ ] 11.2: 将 navItemDelegate 内 `MouseArea` 替换为 `TapHandler { onTapped: root.selectItem(navItemDelegate.index) }` + `HoverHandler { id: itemHover }`
  - [ ] 11.3: 更新 `containsMouse` 引用为 `itemHover.hovered`
  - [ ] 11.4: 验证编译通过

- [ ] Task 12: AppDialog/AppSheet/AppDrawer — overlay MouseArea → TapHandler
  - [ ] 12.1: 读取 `AppDialog.qml`，将 overlay `MouseArea` 替换为 `TapHandler { onTapped: root.closeDialog() }`
  - [ ] 12.2: 读取 `AppSheet.qml`，将 overlay `MouseArea` 替换为 `TapHandler { onTapped: root.closeSheet() }`
  - [ ] 12.3: 读取 `AppDrawer.qml`，将 overlay `MouseArea` 替换为 `TapHandler { onTapped: root.closeDrawer() }`
  - [ ] 12.4: 验证编译通过

- [ ] Task 13: ChartPanel — MouseArea → TapHandler
  - [ ] 13.1: 读取 `src/ui/components/ChartPanel.qml`
  - [ ] 13.2: 将 channel toggle delegate 中 `MouseArea` 替换为 `TapHandler { onTapped: root.onToggleChannel(modelData.key) }`
  - [ ] 13.3: 验证编译通过

- [ ] Task 14: SidebarNav/SidebarNavItem — MouseArea → TapHandler
  - [ ] 14.1: 读取 `src/ui/components/SidebarNav.qml`，将折叠按钮 `MouseArea` 替换为 `TapHandler { onTapped: root.expanded = !root.expanded }`
  - [ ] 14.2: 读取 `src/ui/components/SidebarNavItem.qml`，将 `MouseArea` 替换为 `TapHandler { onTapped: root.triggered() }` + `HoverHandler`
  - [ ] 14.3: 验证编译通过

- [ ] Task 15: 其他小组件 MouseArea 排查替换
  - [ ] 15.1: 读取 `FlowList.qml`, `FlowStepItem.qml`, `InspectorSection.qml`, `ThemeSwitcherPanel.qml`, `AlertStrip.qml`, `DataTableCard.qml`
  - [ ] 15.2: 逐一将发现的 `MouseArea` 替换为 `TapHandler`/`HoverHandler`
  - [ ] 15.3: 验证编译通过

- [ ] Task 16: HistoryPage — SortFilterProxyModel 替换手写过滤
  - [ ] 16.1: 读取 `src/ui/pages/HistoryPage.qml`
  - [ ] 16.2: 删除 `getFilteredModel()` 函数
  - [ ] 16.3: 添加 `import Qt.labs.qmlmodels` 或确认 Qt 6.10 的 `SortFilterProxyModel` 模块导入
  - [ ] 16.4: 在 `historyModel` 下方声明 `SortFilterProxyModel { id: filteredModel; model: historyModel; filters: [FunctionFilter { ... }] }`
  - [ ] 16.5: 将 `historyListView.model` 从 `root.getFilteredModel()` 改为 `filteredModel`
  - [ ] 16.6: 验证编译通过

- [ ] Task 17: AppSlider — Synchronizer 双向绑定
  - [ ] 17.1: 读取 `src/ui/components/AppSlider.qml`
  - [ ] 17.2: 移除 `_syncing` 标志位及相关 `onValueChanged`/`onValuesChanged` 中的防抖逻辑
  - [ ] 17.3: 使用 `import Qt.labs.synchronizer` 和 `Synchronizer` 替代
  - [ ] 17.4: 验证编译通过

- [ ] Task 18: AppProgress — 增加 indeterminate 模式
  - [ ] 18.1: 读取 `src/ui/components/AppProgress.qml`
  - [ ] 18.2: 添加 `property bool indeterminate: false` 属性
  - [ ] 18.3: 当 `indeterminate: true` 时，使用 `SequentialAnimation` 驱动一个循环滑块动画
  - [ ] 18.4: 当 `indeterminate: false` 时，保持原有确定进度条行为
  - [ ] 18.5: 验证编译通过

- [ ] Task 19: AppScrollArea — ScrollView 简化
  - [ ] 19.1: 读取 `src/ui/components/AppScrollArea.qml`
  - [ ] 19.2: 将 `Flickable` + `ScrollBar.vertical: ScrollBar { ... }` 替换为 `ScrollView { ... }`
  - [ ] 19.3: 保留 `atVerticalEnd`、`scrollToEnd` 功能
  - [ ] 19.4: 验证编译通过

- [ ] Task 20: Main.qml — Window → ApplicationWindow
  - [ ] 20.1: 读取 `Main.qml`
  - [ ] 20.2: 添加 `import QtQuick.Controls`，将 `Window { ... }` 替换为 `ApplicationWindow { ... }`
  - [ ] 20.3: 验证编译通过，确认弹出框层级正常

- [ ] Task 21: AppShell — 页面懒加载
  - [ ] 21.1: 读取 `src/ui/pages/AppShell.qml`
  - [ ] 21.2: 将 `StackLayout` 内的 6 个页面组件各用 `Loader { active: StackLayout.isCurrentItem; sourceComponent: ... }` 包裹
  - [ ] 21.3: 验证编译通过，确认页面切换正常

- [ ] Task 22: 全局编译验证
  - [ ] 22.1: 运行完整构建，确认所有 QML 文件无语法错误
  - [ ] 22.2: 运行 QML 冒烟测试（如果可用）

# Task Dependencies
- [Task 1] 是其他 MouseArea 替换任务的参考模板，应最先完成
- [Task 5 (AppCombobox SearchField)] 独立，不依赖其他任务
- [Task 16 (HistoryPage SortFilterProxyModel)] 独立，不依赖其他任务
- [Task 17 (AppSlider Synchronizer)] 独立，不依赖其他任务
- [Task 18 (AppProgress indeterminate)] 独立，不依赖其他任务
- [Task 19 (AppScrollArea ScrollView)] 独立，不依赖其他任务
- [Task 20 (ApplicationWindow)] 应在 Task 12 (Dialog overlay) 之后完成
- [Task 21 (AppShell 懒加载)] 独立，不依赖其他任务
- [Task 22 (全局验证)] 依赖所有其他任务完成
- [Task 2-15] 各 MouseArea 替换任务互相独立，可并行执行
