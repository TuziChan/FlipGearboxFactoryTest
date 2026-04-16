# Shadcn-Inspired, Qt-Native Design System

这份文档定义了一套可同时服务于 HTML 草稿和 Qt/QML 正式实现的设计系统。目标不是照搬 `shadcn/ui` 的 React/Tailwind 技术实现，而是借用它“开放源码、组件语义清晰、token 驱动”的方法，建立一套能被 QML 完整复刻的组件库。

适用范围：

- `Docs/drafts/apple-industrial-prototype/` 下的 HTML 原型
- `src/ui/components/` 下的 Qt/QML 组件
- 后续任何新的页面或变体

## Design Principles

1. 组件优先，不在页面中重复写视觉细节。
2. token 优先，颜色/圆角/边框/字号/间距必须通过统一变量控制。
3. 语义命名，不使用平台绑定命名。
4. HTML 和 QML 使用同一套组件职责，只允许实现方式不同。
5. 业务页面只负责布局与状态组合，不负责重新发明按钮、卡片、边栏或表格样式。

## Naming Convention

命名统一规则如下：

- 原子组件统一使用 `App*`
- 组合组件使用 `语义名 + 类型`
- 列表类使用 `*List`
- 单项类使用 `*Item`
- 区块容器优先使用 `*Section` 或 `*Panel`
- 页面专用旧名保留，但只作为过渡层，不再扩展

推荐命名：

- `AppButton`
- `AppInput`
- `AppSelect`
- `AppBadge`
- `AppIcon`
- `AppSeparator`
- `SidebarNav`
- `SidebarNavItem`
- `FlowList`
- `FlowStepItem`
- `SectionCard`
- `InspectorSection`
- `AlertStrip`
- `BottomStatusBar`

过渡层旧名与新名映射：

- `NavIcon` -> `AppIcon`
- `NavRail` -> `SidebarNav`
- `StepPanel` -> `FlowList`
- `StatusBar` -> `BottomStatusBar`
- `TitleBar + CommandBar` -> `TopToolbar`（后续合并目标）
- `VerdictPanel` -> `InspectorPanel` / `InspectorSection`（后续拆分目标）

后续新增组件必须优先使用新命名，不再引入新的页面专用命名。

## Tokens

以下 token 是系统基线，HTML 已在 `css/tokens.css` 使用，QML 应映射到 `AppTheme.qml`。

### Color Tokens

- `accent`
- `accent-weak`
- `accent-strong`
- `ok`
- `ok-weak`
- `warn`
- `warn-weak`
- `danger`
- `danger-weak`
- `bg`
- `surface`
- `card`
- `panel`
- `divider`
- `stroke`
- `text-1`
- `text-2`
- `text-3`

### Radius Tokens

- `radius-s`
- `radius-m`
- `radius-l`

### Spacing Tokens

- `space-1`
- `space-2`
- `space-3`
- `space-4`
- `space-5`
- `space-6`

### Elevation Tokens

- `shadow-soft`

## Component Taxonomy

组件分为三层：

### 1. Foundation

这些不直接出现在业务页面里，但被所有组件依赖。

- `Tokens`
- `IconSet`
- `Typography`
- `SurfaceStyle`
- `StateStyle`

### 2. Primitive Components

这些是跨页面的基础积木。

- `AppButton`
- `AppInput`
- `AppSelect`
- `AppBadge`
- `AppIcon`
- `AppSeparator`

### 3. Composite Components

这些是业务无关但布局语义明确的组合组件。

- `TopToolbar`
- `SidebarNav`
- `SidebarNavItem`
- `FlowList`
- `FlowStepItem`
- `MetricCard`
- `MetricGrid`
- `SectionCard`
- `DataTableCard`
- `ChartPanel`
- `InspectorPanel`
- `VerdictBadge`
- `StatusList`
- `AlertStrip`
- `BottomStatusBar`

## Canonical Components

下面是推荐的组件定义方式。HTML 和 QML 都必须沿用这些语义边界。

### AppButton

职责：

- 统一按钮形状、间距、字号和状态

状态：

- default
- primary
- danger
- secondary
- disabled

QML 对应：

- `AppButton.qml`

HTML 当前位置：

- `components.css` 中 `.btn`

### AppInput / AppSelect

职责：

- 所有字段输入的统一外观

QML 对应：

- `AppInput.qml`
- `AppSelect.qml`

HTML 当前位置：

- `components.css` 中 `.field input` / `.field select`

### AppBadge

职责：

- 工位标签、状态 pill、轻量标签

QML 对应：

- `AppBadge.qml`

HTML 当前位置：

- `components.css` 中 `.pill`

### SidebarNav / SidebarNavItem

职责：

- 页面级导航，不承担流程状态

行为：

- 可展开/收起
- 当前页面高亮
- hover 轻反馈

QML 对应：

- `NavRail.qml`
- `NavIcon.qml`

HTML 当前位置：

- `index.html` 中 `aside.sidebar`
- `components.css` 中 `.sidebar`, `.nav-item`

### FlowList / FlowStepItem

职责：

- 测试执行阶段的主线流程

行为：

- pending / current / done / interrupted
- 展示步骤编号、名称、耗时、说明

QML 对应：

- `StepPanel.qml`

HTML 当前位置：

- `#flowList`
- `.flow-item`

### MetricCard

职责：

- 展示单个测试指标

规则：

- 标签次级
- 数值一级
- 颜色只在语义需要时使用

QML 对应：

- `MetricCard.qml`

HTML 当前位置：

- `.metric-card`

### DataTableCard

职责：

- 展示阶段性明细表

规则：

- 未进入相关阶段时不显示
- 结果列支持 OK / NG / Pending

QML 对应：

- `DataTableCard.qml`

HTML 当前位置：

- `.detail-table`

### ChartPanel

职责：

- 展示实时趋势线和可切换 series

行为：

- series 开关
- 空状态说明
- 网格线

QML 对应：

- `ChartPanel.qml`

HTML 当前位置：

- `#chartCanvas`
- `.chart-chip`

### InspectorPanel

职责：

- 展示结论、判据、设备状态和次级操作

规则：

- 不是第二主内容区
- 视觉权重低于主工作区

QML 对应：

- `VerdictPanel.qml`

HTML 当前位置：

- `.inspector-panel`

### AlertStrip

职责：

- 展示异常或中断

规则：

- 仅在异常场景显示
- 必须和 verdict / flow 同步

QML 对应：

- `AlertStrip.qml`

HTML 当前位置：

- `#alertBar`

### BottomStatusBar

职责：

- 展示心跳、设备在线状态、当前时间

QML 对应：

- `BottomStatusBar.qml`

HTML 当前位置：

- `.statusbar`

## HTML to QML Mapping

以下映射必须保持一致：

- `sidebar` -> `SidebarNav`
- `nav-item` -> `SidebarNavItem`
- `toolbar` -> `TopToolbar`
- `field` -> `AppField`
- `btn` -> `AppButton`
- `flow-item` -> `FlowStepItem`
- `metric-card` -> `MetricCard`
- `detail-card` -> `SectionCard`
- `detail-table` -> `DataTableCard`
- `chart-card` -> `ChartPanel`
- `verdict-card` -> `VerdictBadge + InspectorSection`
- `summary-card` -> `InspectorSection`
- `device-card` -> `StatusList`
- `alert-bar` -> `AlertStrip`
- `statusbar` -> `BottomStatusBar`

## Recommended Qt/QML File Targets

建议后续在 `src/ui/components/` 继续补齐这批正式组件：

- `AppButton.qml`
- `AppInput.qml`
- `AppSelect.qml`
- `AppBadge.qml`
- `AppSeparator.qml`
- `TopToolbar.qml`
- `SidebarNav.qml`
- `SidebarNavItem.qml`
- `FlowList.qml`
- `FlowStepItem.qml`
- `MetricGrid.qml`
- `SectionCard.qml`
- `InspectorSection.qml`
- `AlertStrip.qml`
- `BottomStatusBar.qml`

现有组件应逐步向这些命名归一，而不是持续扩展页面专用命名。

## Implementation Strategy

推荐顺序：

1. 固化 tokens
2. 统一 primitive components
3. 重构 composite components
4. 页面只保留布局和状态连接
5. 最后再做动效和细节 polish

## What Not to Port Literally from shadcn/ui

以下内容不要直接照搬：

- React hooks
- Tailwind utility class 结构
- Radix 的 DOM/portal/focus trap 实现
- `data-state` 驱动样式方式
- 依赖浏览器事件模型的复杂组件逻辑

应移植的是：

- 组件边界
- 命名方式
- token 系统
- 状态语义
- 可组合结构

## Current Draft Ownership

当前草稿目录：

- [index.html](/F:/Work/FlipGearboxFactoryTest/Docs/drafts/apple-industrial-prototype/index.html)
- [tokens.css](/F:/Work/FlipGearboxFactoryTest/Docs/drafts/apple-industrial-prototype/css/tokens.css)
- [base.css](/F:/Work/FlipGearboxFactoryTest/Docs/drafts/apple-industrial-prototype/css/base.css)
- [layout.css](/F:/Work/FlipGearboxFactoryTest/Docs/drafts/apple-industrial-prototype/css/layout.css)
- [components.css](/F:/Work/FlipGearboxFactoryTest/Docs/drafts/apple-industrial-prototype/css/components.css)
- [data.js](/F:/Work/FlipGearboxFactoryTest/Docs/drafts/apple-industrial-prototype/js/data.js)
- [state.js](/F:/Work/FlipGearboxFactoryTest/Docs/drafts/apple-industrial-prototype/js/state.js)
- [app.js](/F:/Work/FlipGearboxFactoryTest/Docs/drafts/apple-industrial-prototype/js/app.js)

这套文件目前是 HTML 参考实现，同时也是未来 QML 复刻的视觉基线。
