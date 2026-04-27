$components = Get-ChildItem -Path src/ui/components -Filter "*.qml"
$issues = @()

foreach ($file in $components) {
    $content = Get-Content $file.FullName -Raw
    $fileName = $file.Name
    
    # Check 1: Column in Layout context
    if ($content -match 'Layout' -and $content -match '\bColumn\s*\{') {
        $issues += [PSCustomObject]@{
            File = $fileName
            Issue = "May need ColumnLayout"
            Priority = "Medium"
        }
    }
    
    # Check 2: Row in Layout context
    if ($content -match 'Layout' -and $content -match '\bRow\s*\{') {
        $issues += [PSCustomObject]@{
            File = $fileName
            Issue = "May need RowLayout"
            Priority = "Medium"
        }
    }
    
    # Check 3: Flickable without ScrollBar
    if ($content -match '\bFlickable\s*\{' -and $content -notmatch 'ScrollBar') {
        $issues += [PSCustomObject]@{
            File = $fileName
            Issue = "Flickable missing ScrollBar"
            Priority = "High"
        }
    }
    
    # Check 4: Using anchors in Layout
    if ($content -match 'Layout\.' -and $content -match 'anchors\.(fill|left|right|top|bottom)') {
        $issues += [PSCustomObject]@{
            File = $fileName
            Issue = "Mixed anchors in Layout"
            Priority = "High"
        }
    }
    
    # Check 5: Missing pragma ComponentBehavior
    if ($content -notmatch 'pragma ComponentBehavior: Bound' -and $content -match 'required property') {
        $issues += [PSCustomObject]@{
            File = $fileName
            Issue = "Missing pragma ComponentBehavior: Bound"
            Priority = "Low"
        }
    }
    
    # Check 6: Using MouseArea instead of TapHandler
    if ($content -match '\bMouseArea\s*\{') {
        $issues += [PSCustomObject]@{
            File = $fileName
            Issue = "Consider TapHandler/PointHandler (Qt 6)"
            Priority = "Low"
        }
    }
}

$issues | Format-Table -AutoSize
Write-Host ""
Write-Host "Total issues found: $($issues.Count)"
