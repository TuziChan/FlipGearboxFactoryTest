$components = Get-ChildItem -Path src/ui/components -Filter "*.qml"
$highPriority = @()
$mediumPriority = @()
$lowPriority = @()

foreach ($file in $components) {
    $content = Get-Content $file.FullName -Raw
    $fileName = $file.Name
    
    # High Priority: Mixed anchors in Layout
    if ($content -match 'Layout\.' -and $content -match 'anchors\.(fill|left|right|top|bottom)') {
        $highPriority += $fileName
    }
    
    # Medium Priority: Column/Row in Layout context
    if ($content -match 'Layout' -and ($content -match '\bColumn\s*\{' -or $content -match '\bRow\s*\{')) {
        if ($mediumPriority -notcontains $fileName) {
            $mediumPriority += $fileName
        }
    }
    
    # High Priority: Flickable without ScrollBar
    if ($content -match '\bFlickable\s*\{' -and $content -notmatch 'ScrollBar') {
        if ($highPriority -notcontains $fileName) {
            $highPriority += $fileName
        }
    }
}

Write-Host "=== Qt 6 Optimization Analysis ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "HIGH PRIORITY (Mixed anchors in Layout): $($highPriority.Count) files" -ForegroundColor Red
$highPriority | Sort-Object | ForEach-Object { Write-Host "  - $_" }
Write-Host ""
Write-Host "MEDIUM PRIORITY (Column/Row in Layout): $($mediumPriority.Count) files" -ForegroundColor Yellow
$mediumPriority | Sort-Object | ForEach-Object { Write-Host "  - $_" }
Write-Host ""
Write-Host "Total files needing attention: $($highPriority.Count + $mediumPriority.Count)" -ForegroundColor Cyan
