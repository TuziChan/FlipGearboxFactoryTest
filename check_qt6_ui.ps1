# Qt 6 UI 组件全面核查脚本

$results = @()

Write-Host "正在扫描 QML 文件..." -ForegroundColor Cyan

Get-ChildItem -Path src/ui -Recurse -Filter "*.qml" | ForEach-Object {
    $file = $_.FullName
    $relativePath = $file.Replace((Get-Location).Path + "\", "")
    $content = Get-Content $file -Raw
    
    $issues = @()
    
    # 1. 检查是否在 Layout 上下文中使用 Column/Row 而不是 ColumnLayout/RowLayout
    if ($content -match 'Layout\.' -and $content -match '\b(Column|Row)\s*\{' -and $content -notmatch 'ColumnLayout|RowLayout') {
        if ($content -match '(?<!umn)Layout\s*\{' -or $content -match 'Layout\.(fillWidth|fillHeight|preferredWidth|preferredHeight)') {
            $issues += "使用 Column/Row 而非 ColumnLayout/RowLayout（在 Layout 上下文中）"
        }
    }
    
    # 2. 检查 Flickable 是否缺少 ScrollBar
    if ($content -match '\bFlickable\s*\{' -and $content -notmatch 'ScrollBar\.(vertical|horizontal)' -and $content -notmatch 'ScrollView') {
        $issues += "Flickable 缺少 ScrollBar"
    }
    
    # 3. 检查是否应该使用 ScrollView 而不是 Flickable
    if ($content -match '\bFlickable\s*\{[^}]*contentWidth[^}]*contentHeight' -and $content -notmatch 'ScrollBar') {
        $issues += "建议使用 ScrollView 替代 Flickable"
    }
    
    # 4. 检查是否使用了过时的 anchors.fill 而不是 Layout
    if ($content -match 'RowLayout|ColumnLayout|GridLayout' -and $content -match 'anchors\.fill:\s*parent' -and $content -notmatch 'Layout\.(fillWidth|fillHeight)') {
        $issues += "在 Layout 中使用 anchors.fill 而非 Layout.fillWidth/fillHeight"
    }
    
    # 5. 检查是否缺少 pragma ComponentBehavior: Bound
    if ($content -match 'delegate:' -and $content -notmatch 'pragma ComponentBehavior: Bound') {
        $issues += "缺少 'pragma ComponentBehavior: Bound'（Qt 6.2+ 最佳实践）"
    }
    
    # 6. 检查是否使用了 width: parent.width 而不是 Layout.fillWidth
    if ($content -match 'ColumnLayout|RowLayout' -and $content -match 'width:\s*parent\.width' -and $content -notmatch 'Layout\.fillWidth') {
        $issues += "使用 width: parent.width 而非 Layout.fillWidth"
    }
    
    # 7. 检查是否使用了老式的 import 语句
    if ($content -match 'import QtQuick 2\.' -or $content -match 'import QtQuick\.Controls 2\.') {
        $issues += "使用 Qt 5 风格的 import（应使用 Qt 6 风格）"
    }
    
    # 8. 检查是否使用了 Item 作为根元素而可以使用更具体的类型
    if ($content -match '^\s*Item\s*\{' -and $content -match 'ColumnLayout|RowLayout' -and $relativePath -notmatch 'AppShell') {
        $issues += "根元素使用 Item，可能应该使用 ColumnLayout/RowLayout"
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
$reportPath = "QT6_UI_AUDIT_REPORT.md"
$report = @"
# Qt 6 UI 组件全面核查报告

生成时间: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

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
```qml
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
```

### 2. 使用 ScrollView
```qml
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
```

### 3. 使用 pragma ComponentBehavior: Bound
```qml
// ✓ 推荐
pragma ComponentBehavior: Bound

Repeater {
    model: myModel
    delegate: Rectangle {
        required property int index
        required property string name
    }
}
```

### 4. 使用 Qt 6 风格的 import
```qml
// ✓ 推荐
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// ✗ 不推荐
import QtQuick 2.15
import QtQuick.Controls 2.15
```

"@

$report | Out-File -FilePath $reportPath -Encoding utf8
Write-Host "详细报告已保存到: $reportPath" -ForegroundColor Green
