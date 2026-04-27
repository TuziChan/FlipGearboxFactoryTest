<#
.SYNOPSIS
    Automated build and test pipeline for FlipGearboxFactoryTest
.DESCRIPTION
    Configures CMake, builds the project, runs all tests, and generates reports.
    Designed for CI/CD and AI team automation.
.PARAMETER QtRoot
    Qt installation root directory (default: D:\Qt\6.11.0\mingw_64)
.PARAMETER BuildType
    CMake build type: Debug, Release, RelWithDebInfo (default: Debug)
.PARAMETER Clean
    Perform a clean build
.PARAMETER SkipTests
    Skip test execution
.PARAMETER Generator
    CMake generator (default: auto-detect)
.PARAMETER OutputDir
    Directory for test reports (default: build/test-reports)
.PARAMETER MonitorSession
    Enable team execution monitoring
.EXAMPLE
    .\Build-And-Test.ps1 -QtRoot "D:\Qt\6.11.0\mingw_64" -Clean
#>
param(
    [string]$QtRoot = "D:\Qt\6.11.0\mingw_64",
    [string]$BuildType = "Debug",
    [switch]$Clean,
    [switch]$SkipTests,
    [string]$Generator = "",
    [string]$OutputDir = "",
    [switch]$MonitorSession
)

$ErrorActionPreference = "Stop"
$scriptStart = Get-Date

# Resolve paths
$SourceDir = Resolve-Path (Split-Path -Parent $MyInvocation.MyCommand.Path)
if (-not $SourceDir) {
    $SourceDir = Get-Location
}
$BuildDir = Join-Path $SourceDir "build"
if ([string]::IsNullOrEmpty($OutputDir)) {
    $OutputDir = Join-Path $BuildDir "test-reports"
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "FlipGearboxFactoryTest Build Pipeline" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Source:    $SourceDir"
Write-Host "Build:     $BuildDir"
Write-Host "Qt Root:   $QtRoot"
Write-Host "BuildType: $BuildType"
Write-Host "Clean:     $Clean"
Write-Host "SkipTests: $SkipTests"
Write-Host "Output:    $OutputDir"
Write-Host "========================================"

# Verify Qt exists
if (-not (Test-Path $QtRoot)) {
    Write-Error "Qt directory not found: $QtRoot"
    exit 1
}

$QtBin = Join-Path $QtRoot "bin"
$QtCMake = Join-Path $QtRoot "lib\cmake\Qt6"
if (-not (Test-Path $QtCMake)) {
    Write-Warning "Qt6 CMake config not found at expected path: $QtCMake"
}

# Add Qt to PATH
$env:PATH = "$QtBin;$env:PATH"
$env:QTDIR = $QtRoot
$env:Qt6_DIR = $QtCMake

# Auto-detect generator if not specified
if ([string]::IsNullOrEmpty($Generator)) {
    $ninja = Get-Command "ninja" -ErrorAction SilentlyContinue
    $mingwMake = Get-Command "mingw32-make" -ErrorAction SilentlyContinue
    if ($ninja) {
        $Generator = "Ninja"
    } elseif ($mingwMake) {
        $Generator = "MinGW Makefiles"
    } else {
        $Generator = "Ninja Multi-Config"
    }
    Write-Host "Auto-detected generator: $Generator" -ForegroundColor Green
}

# Clean build directory if requested
if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

# Create directories
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

# Create monitoring log directory
$MonitorDir = Join-Path $BuildDir "monitor-logs"
if ($MonitorSession) {
    New-Item -ItemType Directory -Force -Path $MonitorDir | Out-Null
}

# Stage 1: Configure
Write-Host ""
Write-Host "[1/4] Configuring CMake..." -ForegroundColor Cyan
$configureStart = Get-Date

try {
    $cmakeArgs = @(
        "-S", $SourceDir,
        "-B", $BuildDir,
        "-G", $Generator,
        "-DCMAKE_BUILD_TYPE=$BuildType",
        "-DCMAKE_PREFIX_PATH=$QtRoot"
    )

    & cmake @cmakeArgs 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed with exit code $LASTEXITCODE"
    }
} catch {
    Write-Error "Configuration failed: $_"
    exit 1
}
$configureDuration = (Get-Date) - $configureStart
Write-Host "Configuration completed in $($configureDuration.TotalSeconds.ToString('F1'))s" -ForegroundColor Green

# Stage 2: Build
Write-Host ""
Write-Host "[2/4] Building project..." -ForegroundColor Cyan
$buildStart = Get-Date

try {
    $buildArgs = @(
        "--build", $BuildDir,
        "--config", $BuildType,
        "--parallel"
    )

    & cmake @buildArgs 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE"
    }
} catch {
    Write-Error "Build failed: $_"
    exit 1
}
$buildDuration = (Get-Date) - $buildStart
Write-Host "Build completed in $($buildDuration.TotalSeconds.ToString('F1'))s" -ForegroundColor Green

# Stage 3: Tests
if (-not $SkipTests) {
    Write-Host ""
    Write-Host "[3/4] Running tests..." -ForegroundColor Cyan
    $testStart = Get-Date

    # Set Qt environment for tests
    $env:QT_QPA_PLATFORM = "offscreen"
    $env:QT_QUICK_CONTROLS_STYLE = "Basic"

    try {
        $ctestArgs = @(
            "--test-dir", $BuildDir,
            "--output-on-failure",
            "-C", $BuildType
        )

        & ctest @ctestArgs 2>&1
        $testExitCode = $LASTEXITCODE
    } catch {
        Write-Warning "CTest execution had issues: $_"
        $testExitCode = 1
    }

    $testDuration = (Get-Date) - $testStart
    Write-Host "Tests completed in $($testDuration.TotalSeconds.ToString('F1'))s" -ForegroundColor Green

    # Run unified test runner for detailed reports
    $UnifiedRunner = Join-Path $BuildDir "$BuildType\UnifiedTestRunner.exe"
    if (-not (Test-Path $UnifiedRunner)) {
        $UnifiedRunner = Join-Path $BuildDir "UnifiedTestRunner.exe"
    }

    if (Test-Path $UnifiedRunner) {
        Write-Host "Generating detailed test reports..." -ForegroundColor Cyan
        & $UnifiedRunner `
            --build-dir $BuildDir `
            --output-dir $OutputDir `
            --format json,html,junit,md `
            --qt-path $QtBin 2>&1
    } else {
        Write-Warning "UnifiedTestRunner not found at $UnifiedRunner"
    }

    if ($testExitCode -ne 0) {
        Write-Warning "Some tests failed (exit code: $testExitCode)"
    }
} else {
    Write-Host "[3/4] Skipping tests (--SkipTests specified)" -ForegroundColor Yellow
}

# Stage 4: Generate summary report
Write-Host ""
Write-Host "[4/4] Generating pipeline report..." -ForegroundColor Cyan

$scriptDuration = (Get-Date) - $scriptStart
$reportPath = Join-Path $OutputDir "pipeline_report.json"

$report = @{
    timestamp = (Get-Date -Format "o")
    success = $true
    stages = @{
        configure = @{
            duration_seconds = [math]::Round($configureDuration.TotalSeconds, 2)
            success = $true
        }
        build = @{
            duration_seconds = [math]::Round($buildDuration.TotalSeconds, 2)
            success = $true
        }
        test = @{
            duration_seconds = if ($SkipTests) { 0 } else { [math]::Round($testDuration.TotalSeconds, 2) }
            success = if ($SkipTests) { $true } else { ($testExitCode -eq 0) }
            exit_code = if ($SkipTests) { -1 } else { $testExitCode }
        }
    }
    total_duration_seconds = [math]::Round($scriptDuration.TotalSeconds, 2)
    report_output_dir = $OutputDir
    build_dir = $BuildDir
    qt_root = $QtRoot
} | ConvertTo-Json -Depth 5

$report | Out-File -FilePath $reportPath -Encoding utf8
Write-Host "Pipeline report saved to: $reportPath" -ForegroundColor Green

# Final summary
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "PIPELINE SUMMARY" -ForegroundColor Cyan
Write-Host "========================================"
Write-Host "Configure:  $($configureDuration.TotalSeconds.ToString('F1'))s"
if (-not $SkipTests) {
    Write-Host "Build:      $($buildDuration.TotalSeconds.ToString('F1'))s"
    Write-Host "Tests:      $($testDuration.TotalSeconds.ToString('F1'))s (exit: $testExitCode)"
} else {
    Write-Host "Build:      $($buildDuration.TotalSeconds.ToString('F1'))s"
    Write-Host "Tests:      SKIPPED"
}
Write-Host "Total:      $($scriptDuration.TotalSeconds.ToString('F1'))s"
Write-Host "Reports:    $OutputDir"
Write-Host "========================================"

exit 0
