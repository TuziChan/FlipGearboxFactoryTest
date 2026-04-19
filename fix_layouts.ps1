# Qt 6 Auto-fix Script for common Layout issues

function Fix-LayoutAnchors {
    param($filePath)
    
    $content = Get-Content $filePath -Raw
    $modified = $false
    
    # Pattern 1: ColumnLayout/RowLayout with anchors.fill and anchors.margins
    if ($content -match '(ColumnLayout|RowLayout)\s*\{[^}]*anchors\.fill:\s*parent[^}]*anchors\.margins:\s*(\d+)') {
        Write-Host "  Found Layout with anchors.fill in $filePath" -ForegroundColor Yellow
        # This needs manual review as the fix depends on context
    }
    
    # Pattern 2: Layout items without Layout.fillWidth when they should have it
    if ($content -match 'ColumnLayout\s*\{[^}]*visible:.*spacing:') {
        Write-Host "  Found ColumnLayout that may need Layout.fillWidth in $filePath" -ForegroundColor Yellow
    }
    
    return $modified
}

$files = @(
    "src/ui/components/SectionCard.qml",
    "src/ui/components/FlowStepItem.qml",
    "src/ui/components/DataTableCard.qml",
    "src/ui/components/MetricCard.qml",
    "src/ui/components/BottomStatusBar.qml"
)

Write-Host "=== Analyzing High Priority Files ===" -ForegroundColor Cyan
Write-Host ""

foreach ($file in $files) {
    if (Test-Path $file) {
        Write-Host "Checking: $file" -ForegroundColor Green
        Fix-LayoutAnchors $file
    }
}

Write-Host ""
Write-Host "Analysis complete. Manual fixes required for complex cases." -ForegroundColor Cyan
