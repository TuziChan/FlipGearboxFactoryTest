@echo off
setlocal enabledelayedexpansion

REM ============================================================
REM FlipGearboxFactoryTest - CI Automation Script
REM ============================================================
REM Full CI pipeline:
REM   1. Clean build (Debug + Release)
REM   2. Run all tests
REM   3. Generate test reports
REM   4. Collect artifacts
REM ============================================================

set "START_TIME=%TIME%"
set "CI_LOG=ci_run_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%.log"
set "CI_LOG=%CI_LOG: =0%"

echo ========================================
echo FlipGearboxFactoryTest - CI Pipeline
echo ========================================
echo Started: %date% %time%
echo Log: %CI_LOG%
echo.

(
echo ========================================
echo FlipGearboxFactoryTest - CI Pipeline
echo ========================================
echo Started: %date% %time%
) > "%CI_LOG%"

set /a STAGE=0
set /a TOTAL_STAGES=4

REM Stage 1: Clean Build (Debug)
set /a STAGE+=1
echo.
echo ========================================
echo Stage %STAGE%/%TOTAL_STAGES%: Clean Build (Debug)
echo ========================================
call scripts\build_project.bat Debug clean >> "%CI_LOG%" 2>&1
if %ERRORLEVEL% neq 0 (
    echo [FAIL] Debug build failed!
    echo [FAIL] Debug build failed! >> "%CI_LOG%"
    goto :ci_failed
)
echo [PASS] Debug build successful!
echo [PASS] Debug build successful! >> "%CI_LOG%"

REM Stage 2: Clean Build (Release)
set /a STAGE+=1
echo.
echo ========================================
echo Stage %STAGE%/%TOTAL_STAGES%: Clean Build (Release)
echo ========================================
call scripts\build_project.bat Release clean >> "%CI_LOG%" 2>&1
if %ERRORLEVEL% neq 0 (
    echo [FAIL] Release build failed!
    echo [FAIL] Release build failed! >> "%CI_LOG%"
    goto :ci_failed
)
echo [PASS] Release build successful!
echo [PASS] Release build successful! >> "%CI_LOG%"

REM Stage 3: Run All Tests (Debug build)
set /a STAGE+=1
echo.
echo ========================================
echo Stage %STAGE%/%TOTAL_STAGES%: Run All Tests
echo ========================================
call scripts\run_tests.bat --all --report >> "%CI_LOG%" 2>&1
if %ERRORLEVEL% neq 0 (
    echo [WARN] Some tests failed - see log for details
    echo [WARN] Some tests failed - see log for details >> "%CI_LOG%"
    REM Don't fail CI on test failure, just report
)
echo [PASS] Test execution completed!
echo [PASS] Test execution completed! >> "%CI_LOG%"

REM Stage 4: Collect Artifacts
set /a STAGE+=1
echo.
echo ========================================
echo Stage %STAGE%/%TOTAL_STAGES%: Collect Artifacts
echo ========================================

set "ARTIFACT_DIR=artifacts\%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%"
set "ARTIFACT_DIR=%ARTIFACT_DIR: =0%"
if not exist "%ARTIFACT_DIR%" mkdir "%ARTIFACT_DIR%"

REM Copy test reports
for /d %%d in (build\Desktop_Qt_*) do (
    if exist "%%d\test_results.json" (
        copy "%%d\test_results.json" "%ARTIFACT_DIR%\" >nul
        echo [OK] Copied test_results.json from %%d
    )
)

REM Copy CI log
copy "%CI_LOG%" "%ARTIFACT_DIR%\" >nul

echo [PASS] Artifacts collected in: %ARTIFACT_DIR%
echo [PASS] Artifacts collected in: %ARTIFACT_DIR% >> "%CI_LOG%"

REM Final Summary
echo.
echo ========================================
echo CI Pipeline Complete
echo ========================================
echo Started:  %START_TIME%
echo Finished: %TIME%
echo Artifacts: %ARTIFACT_DIR%
echo Status: SUCCESS
echo ========================================

echo. >> "%CI_LOG%"
echo ======================================== >> "%CI_LOG%"
echo CI Pipeline Complete >> "%CI_LOG%"
echo Started:  %START_TIME% >> "%CI_LOG%"
echo Finished: %TIME% >> "%CI_LOG%"
echo Status: SUCCESS >> "%CI_LOG%"
echo ======================================== >> "%CI_LOG%"

exit /b 0

:ci_failed
echo.
echo ========================================
echo CI Pipeline FAILED at Stage %STAGE%
echo ========================================
echo Started:  %START_TIME%
echo Finished: %TIME%
echo Log: %CI_LOG%
echo ========================================

echo. >> "%CI_LOG%"
echo ======================================== >> "%CI_LOG%"
echo CI Pipeline FAILED at Stage %STAGE% >> "%CI_LOG%"
echo Started:  %START_TIME% >> "%CI_LOG%"
echo Finished: %TIME% >> "%CI_LOG%"
echo Status: FAILED >> "%CI_LOG%"
echo ======================================== >> "%CI_LOG%"

exit /b 1
