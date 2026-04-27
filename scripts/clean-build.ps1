#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Clean build directory with multiple modes

.DESCRIPTION
    Supports three cleaning modes:
    - Full: Remove entire build/ directory
    - CacheOnly: Remove cache files (CMakeFiles, *_autogen, .qt, .rcc, Desktop_*, *.tmp)
    - KeepExe: Remove everything except .exe and .a files

.PARAMETER Full
    Remove entire build/ directory

.PARAMETER CacheOnly
    Remove only cache and temporary files (default)

.PARAMETER KeepExe
    Remove everything except executables and libraries

.EXAMPLE
    .\clean-build.ps1 -Full
    .\clean-build.ps1 -CacheOnly
    .\clean-build.ps1 -KeepExe
#>

param(
    [switch]$Full,
    [switch]$CacheOnly,
    [switch]$KeepExe
)

$ErrorActionPreference = "Stop"
$buildDir = Join-Path $PSScriptRoot "..\build"

if (-not (Test-Path $buildDir)) {
    Write-Host "✓ build/ directory does not exist, nothing to clean" -ForegroundColor Green
    exit 0
}

function Get-DirectorySize {
    param([string]$Path)
    if (Test-Path $Path) {
        $size = (Get-ChildItem -Path $Path -Recurse -File -ErrorAction SilentlyContinue | Measure-Object -Property Length -Sum).Sum
        return [math]::Round($size / 1MB, 2)
    }
    return 0
}

$initialSize = Get-DirectorySize $buildDir

if ($Full) {
    Write-Host "🗑️  Full clean mode: removing entire build/ directory..." -ForegroundColor Yellow
    Remove-Item -Path $buildDir -Recurse -Force
    Write-Host "✓ Removed build/ directory (freed ${initialSize}MB)" -ForegroundColor Green
    exit 0
}

if ($KeepExe) {
    Write-Host "🗑️  Keep-exe mode: removing all except .exe and .a files..." -ForegroundColor Yellow

    $tempDir = Join-Path $PSScriptRoot "..\build-temp-backup"
    New-Item -ItemType Directory -Path $tempDir -Force | Out-Null

    Get-ChildItem -Path $buildDir -Filter "*.exe" | Copy-Item -Destination $tempDir
    Get-ChildItem -Path $buildDir -Filter "*.a" | Copy-Item -Destination $tempDir

    Remove-Item -Path $buildDir -Recurse -Force
    New-Item -ItemType Directory -Path $buildDir -Force | Out-Null

    Get-ChildItem -Path $tempDir | Move-Item -Destination $buildDir
    Remove-Item -Path $tempDir -Recurse -Force

    $finalSize = Get-DirectorySize $buildDir
    $freed = $initialSize - $finalSize
    Write-Host "✓ Kept executables and libraries (freed ${freed}MB)" -ForegroundColor Green
    exit 0
}

# Default: CacheOnly mode
Write-Host "🗑️  Cache-only mode: removing cache and temporary files..." -ForegroundColor Yellow

$itemsToRemove = @(
    "CMakeFiles",
    ".qt",
    ".rcc",
    ".ninja_deps",
    ".ninja_log"
)

$patternsToRemove = @(
    "*_autogen",
    "Desktop_*",
    "*.tmp",
    "*.log"
)

$removedCount = 0

foreach ($item in $itemsToRemove) {
    $path = Join-Path $buildDir $item
    if (Test-Path $path) {
        Remove-Item -Path $path -Recurse -Force -ErrorAction SilentlyContinue
        $removedCount++
        Write-Host "  - Removed $item" -ForegroundColor Gray
    }
}

foreach ($pattern in $patternsToRemove) {
    $items = Get-ChildItem -Path $buildDir -Filter $pattern -ErrorAction SilentlyContinue
    foreach ($item in $items) {
        Remove-Item -Path $item.FullName -Recurse -Force -ErrorAction SilentlyContinue
        $removedCount++
        Write-Host "  - Removed $($item.Name)" -ForegroundColor Gray
    }
}

$finalSize = Get-DirectorySize $buildDir
$freed = $initialSize - $finalSize

Write-Host "✓ Cache cleaned: removed $removedCount items, freed ${freed}MB (${finalSize}MB remaining)" -ForegroundColor Green
