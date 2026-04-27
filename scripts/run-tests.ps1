param(
    [string]$Preset = "dev",
    [switch]$Verbose,
    [switch]$Rerun
)

$ErrorActionPreference = "Stop"

Write-Host "=== FlipGearbox 测试运行器 ===" -ForegroundColor Cyan
Write-Host "使用 preset: $Preset" -ForegroundColor Yellow

if (-not (Test-Path "build")) {
    Write-Host "错误：build 目录不存在，请先编译项目" -ForegroundColor Red
    exit 1
}

Set-Location build

$ctestArgs = @("--preset", $Preset)
if ($Verbose) { $ctestArgs += "--verbose" }
if ($Rerun) { $ctestArgs += "--rerun-failed" }

Write-Host "执行命令: ctest $($ctestArgs -join ' ')" -ForegroundColor Green
& ctest @ctestArgs

$exitCode = $LASTEXITCODE
Set-Location ..

if ($exitCode -eq 0) {
    Write-Host "`n✅ 所有测试通过" -ForegroundColor Green
} else {
    Write-Host "`n❌ 有测试失败 (exit code: $exitCode)" -ForegroundColor Red
}

exit $exitCode
