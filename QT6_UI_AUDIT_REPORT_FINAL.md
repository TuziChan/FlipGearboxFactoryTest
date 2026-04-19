# Qt 6 UI 组件全面核查报告（最终版）

生成时间: 2026-04-19 17:31:23

## 已完成的优化

1. ✓ SectionCard.qml - Column 改为 ColumnLayout
2. ✓ AppCardHeader.qml - Column 改为 ColumnLayout
3. ✓ FlowStepItem.qml - Column 改为 ColumnLayout
4. ✓ DataTableCard.qml - Column 改为 ColumnLayout
5. ✓ ComponentGalleryPage.qml - Flickable 改为 ScrollView
6. ✓ FlowList.qml - 添加 pragma ComponentBehavior: Bound
7. ✓ StatusList.qml - 添加 pragma ComponentBehavior: Bound
8. ✓ run.bat - 自动搜索 Qt 目录（C-F 盘）

## 当前检查结果

### src\ui\components\AppAccordion.qml

- Column/Row 的子项使用了 Layout 属性（应改用 ColumnLayout/RowLayout）

### src\ui\components\AppCalendar.qml

- Column/Row 的子项使用了 Layout 属性（应改用 ColumnLayout/RowLayout）

### src\ui\components\AppCollapsible.qml

- 在 Layout 子项中使用 Column/Row 而非 ColumnLayout/RowLayout
- Column/Row 的子项使用了 Layout 属性（应改用 ColumnLayout/RowLayout）

### src\ui\components\CommandBar.qml

- 在 Layout 子项中使用 Column/Row 而非 ColumnLayout/RowLayout

### src\ui\components\StatusList.qml

- Column/Row 的子项使用了 Layout 属性（应改用 ColumnLayout/RowLayout）

### src\ui\pages\ComponentGalleryPage.qml

- Column/Row 的子项使用了 Layout 属性（应改用 ColumnLayout/RowLayout）

## Qt 6 最佳实践总结

### 1. Layout 系统的正确使用

**✓ 正确示例 - 在 Layout 上下文中使用 ColumnLayout**
```qml
ColumnLayout {
    spacing: 8
    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 100
    }
}
```

**✓ 正确示例 - 在非 Layout 上下文中使用 Column**
```qml
Rectangle {
    Column {
        anchors.fill: parent
        spacing: 8
        Text { text: "Item 1" }
        Text { text: "Item 2" }
    }
}
```

**✗ 错误示例 - Column 的子项使用 Layout 属性**
```qml
Column {
    Rectangle {
        Layout.fillWidth: true  // ✗ 错误！Column 不支持 Layout 属性
    }
}
```

### 2. ScrollView vs Flickable

**✓ 推荐 - 使用 ScrollView**
```qml
ScrollView {
    anchors.fill: parent
    contentWidth: availableWidth
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    
    ColumnLayout {
        width: parent.availableWidth
        // 内容
    }
}
```

**✓ 可接受 - Flickable + ScrollBar**
```qml
Flickable {
    anchors.fill: parent
    contentWidth: width
    contentHeight: content.height
    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
    
    Column {
        id: content
        // 内容
    }
}
```

### 3. pragma ComponentBehavior: Bound

**✓ 推荐 - 在使用 delegate 的文件中添加**
```qml
pragma ComponentBehavior: Bound

import QtQuick

Repeater {
    model: myModel
    delegate: Rectangle {
        required property int index
        required property string name
        // 使用 index 和 name
    }
}
```

### 4. Qt 6 风格的 import

**✓ 推荐**
```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
```

**✗ 不推荐（Qt 5 风格）**
```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
```

## 关键区别说明

### Column vs ColumnLayout

- **Column**: 简单的定位器（Positioner），用于垂直堆叠项目
  - 不支持 Layout 属性
  - 适用于简单的垂直排列
  - 子项使用 width/height 设置尺寸

- **ColumnLayout**: 布局管理器，用于响应式布局
  - 支持 Layout.fillWidth、Layout.preferredHeight 等属性
  - 适用于需要动态调整尺寸的场景
  - 子项使用 Layout 属性参与布局协商

### 何时使用哪个？

1. **使用 Column/Row**:
   - 简单的静态布局
   - 不需要响应式尺寸调整
   - 子项尺寸固定或基于内容

2. **使用 ColumnLayout/RowLayout**:
   - 需要响应窗口大小变化
   - 子项需要填充可用空间
   - 需要使用 Layout 属性控制尺寸

## 测试建议

1. 在 Qt Creator 中重新构建项目
2. 测试 ComponentGalleryPage 的滚动功能
3. 验证所有使用 Layout 的组件布局正常
4. 测试窗口缩放时的响应式行为

