# Qt 6 UI 组件全面核查报告

生成时间: 2026-04-19 17:29:15

## 概述

扫描了 src/ui 目录下的所有 QML 文件，检查是否符合 Qt 6 最佳实践。

## 检查项目

1. **Layout 上下文中使用 Column/Row** - 应使用 ColumnLayout/RowLayout
2. **Flickable 缺少 ScrollBar** - 应添加 ScrollBar 或使用 ScrollView
3. **应使用 ScrollView** - 替代 Flickable + ScrollBar 组合
4. **在 Layout 中使用 anchors.fill** - 应使用 Layout.fillWidth/fillHeight
5. **缺少 pragma ComponentBehavior: Bound** - Qt 6.2+ 最佳实践
6. **使用 width: parent.width** - 应使用 Layout.fillWidth
7. **使用 Qt 5 风格的 import** - 应更新为 Qt 6 风格
8. **根元素使用 Item** - 可能应该使用更具体的 Layout 类型

## 发现的问题

### src\ui\components\AlertStrip.qml

- 在 Layout 中使用 anchors.fill 而非 Layout.fillWidth/fillHeight

### src\ui\components\AppButton.qml

- 在 Layout 中使用 anchors.fill 而非 Layout.fillWidth/fillHeight

### src\ui\components\AppButtonGroup.qml

- 在 Layout 中使用 anchors.fill 而非 Layout.fillWidth/fillHeight

### src\ui\components\AppPagination.qml

- 在 Layout 中使用 anchors.fill 而非 Layout.fillWidth/fillHeight

### src\ui\components\AppToggle.qml

- 在 Layout 中使用 anchors.fill 而非 Layout.fillWidth/fillHeight

### src\ui\components\FlowList.qml

- 缺少 'pragma ComponentBehavior: Bound'（Qt 6.2+ 最佳实践）

### src\ui\components\StatusList.qml

- 缺少 'pragma ComponentBehavior: Bound'（Qt 6.2+ 最佳实践）

### src\ui\pages\ComponentGalleryPage.qml

- Flickable 缺少 ScrollBar
- 建议使用 ScrollView 替代 Flickable

## 优先级建议

### 高优先级
- 在 Layout 上下文中使用 Column/Row 改为 ColumnLayout/RowLayout
- Flickable 缺少 ScrollBar

### 中优先级
- 添加 pragma ComponentBehavior: Bound
- 使用 ScrollView 替代 Flickable

### 低优先级
- 使用 Layout.fillWidth 替代 width: parent.width
- 优化根元素类型

## Qt 6 最佳实践总结

### 1. 使用 Layout 系统
`qml
// ✓ 推荐
ColumnLayout {
    spacing: 8
    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 100
    }
}

// ✗ 不推荐（在 Layout 上下文中）
Column {
    spacing: 8
    Rectangle {
        width: parent.width  // Layout 属性被忽略
        height: 100
    }
}
`

### 2. 使用 ScrollView
`qml
// ✓ 推荐
ScrollView {
    width: 200
    height: 200
    ColumnLayout {
        // 内容
    }
}

// ✗ 不推荐
Flickable {
    width: 200
    height: 200
    contentWidth: width
    contentHeight: column.height
    // 缺少 ScrollBar
}
`

### 3. 使用 pragma ComponentBehavior: Bound
`qml
// ✓ 推荐
pragma ComponentBehavior: Bound

Repeater {
    model: myModel
    delegate: Rectangle {
        required property int index
        required property string name
    }
}
`

### 4. 使用 Qt 6 风格的 import
`qml
// ✓ 推荐
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// ✗ 不推荐
import QtQuick 2.15
import QtQuick.Controls 2.15
`

