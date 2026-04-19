# 改进的 Qt 6 UI 组件检查脚本

$results = @()

Write-Host "正在扫描 QML 文件..." -ForegroundColor Cyan

Get-ChildItem -Path src/ui -Recurse -Filter "*.qml" | ForEach-Object {
    $file = $_.FullName
    $relativePath = $file.Replace((Get-Location).Path + "\", "")
    $content = Get-Content $file -Raw
    
    $issues = @()
    
    # 1. 检查是否在 Layout 上下文中使用 Column/Row 而不是 ColumnLayout/RowLayout
    # 更精确的检查：查找 Layout 的子项使用 Column/Row
    if ($content -match '(ColumnLayout|RowLayout|GridLayout)\s*\{[^}]*\n[^}]*(Column|Row)\s*\{' -and 
        $content -notmatch 'delegate:') {
        $issues += "在 Layout 子项中使用 Column/Row 而非 ColumnLayout/RowLayout"
    }
    
    # 2. 检查 Flickable 是否缺少 ScrollBar
    if ($content -match '\bFlickable\s*\{' -and 
        $content -notmatch 'ScrollBar\.(vertical|horizontal)' -and 
        $content -notmatch 'ScrollView') {
        $issues += "Flickable 缺少 ScrollBar（建议使用 ScrollView）"
    }
    
    # 3. 检查是否缺少 pragma ComponentBehavior: Bound
    if ($content -match 'delegate:' -and 
        $content -notmatch 'pragma ComponentBehavior: Bound' -and
        $content -match 'required property') {
        $issues += "缺少 'pragma ComponentBehavior: Bound'（Qt 6.2+ 最佳实践）"
    }
    
    # 4. 检查是否在 Layout 子项中使用了 width: parent.width 而不是 Layout.fillWidth
    if ($content -match '(ColumnLayout|RowLayout)\s*\{[^}]*\n[^}]*width:\s*parent\.width' -and
        $content -notmatch 'Layout\.fillWidth') {
        $issues += "在 Layout 子项中使用 width: parent.width 而非 Layout.fillWidth"
    }
    
    # 5. 检查是否使用了老式的 import 语句
    if ($content -match 'import QtQuick 2\.' -or $content -match 'import QtQuick\.Controls 2\.') {
        $issues += "使用 Qt 5 风格的 import（应使用 Qt 6 风格）"
    }
    
    # 6. 检查 Column/Row 的子项是否使用了 Layout 属性（这是错误的）
    if ($content -match '\bColumn\s*\{[^}]*\n[^}]*Layout\.(fillWidth|fillHeight|preferredWidth|preferredHeight)' -or
        $content -match '\bRow\s*\{[^}]*\n[^}]*Layout\.(fillWidth|fillHeight|preferredWidth|preferredHeight)') {
        $issues += "Column/Row 的子项使用了 Layout 属性（应改用 ColumnLayout/RowLayout）"
    }
    
    if ($issues.Count -gt 0) {
        $results += [PSCustomObject]@{
            File = $relativePath
            Issues = $issues -join "; "
        }
    }
}

Write-Host "`n检查完成！" -ForegroundColor Green
Write-Host "发现 $($results.Count) 个文件需要优化`n" -ForegroundColor Yellow

$results | Format-Table -AutoSize -Wrap

# 生成详细报告
$reportPath = "QT6_UI_AUDIT_REPORT_FINAL.md"
$report = @"
# Qt 6 UI 组件全面核查报告（最终版）

生成时间: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

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

"@

if ($results.Count -eq 0) {
    $report += "`n所有文件都符合 Qt 6 最佳实践！✓`n"
} else {
    foreach ($result in $results) {
        $report += "`n### $($result.File)`n`n"
        foreach ($issue in ($result.Issues -split "; ")) {
            $report += "- $issue`n"
        }
    }
}

$report += @"

## Qt 6 最佳实践总结

### 1. Layout 系统的正确使用

**✓ 正确示例 - 在 Layout 上下文中使用 ColumnLayout**
``````qml
ColumnLayout {
    spacing: 8
    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 100
    }
}
``````

**✓ 正确示例 - 在非 Layout 上下文中使用 Column**
``````qml
Rectangle {
    Column {
        anchors.fill: parent
        spacing: 8
        Text { text: "Item 1" }
        Text { text: "Item 2" }
    }
}
``````

**✗ 错误示例 - Column 的子项使用 Layout 属性**
``````qml
Column {
    Rectangle {
        Layout.fillWidth: true  // ✗ 错误！Column 不支持 Layout 属性
    }
}
``````

### 2. ScrollView vs Flickable

**✓ 推荐 - 使用 ScrollView**
``````qml
ScrollView {
    anchors.fill: parent
    contentWidth: availableWidth
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    
    ColumnLayout {
        width: parent.availableWidth
        // 内容
    }
}
``````

**✓ 可接受 - Flickable + ScrollBar**
``````qml
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
``````

### 3. pragma ComponentBehavior: Bound

**✓ 推荐 - 在使用 delegate 的文件中添加**
``````qml
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
``````

### 4. Qt 6 风格的 import

**✓ 推荐**
``````qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
``````

**✗ 不推荐（Qt 5 风格）**
``````qml
import QtQuick 2.15
import QtQuick.Controls 2.15
``````

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

"@

$report | Out-File -FilePath $reportPath -Encoding utf8
Write-Host "详细报告已保存到: $reportPath" -ForegroundColor Green
