@echo off
setlocal enabledelayedexpansion

REM ============================================================
REM FlipGearboxFactoryTest - Automated Test Runner
REM ============================================================
REM Usage: run_tests.bat [options]
REM   Options:
REM     --all          Run all tests (default)
REM     --unit         Run unit tests only
REM     --integration  Run integration tests only
REM     --mock         Run mock/simulation tests only
REM     --stability    Run stability tests only
REM     --report       Generate test report after run
REM     --verbose      Verbose output
REM     --build-dir    Specify build directory
REM ============================================================

set "RUN_ALL=1"
set "RUN_UNIT=0"
set "RUN_INTEGRATION=0"
set "RUN_MOCK=0"
set "RUN_STABILITY=0"
set "GENERATE_REPORT=0"
set "VERBOSE=0"
set "BUILD_DIR="
set "TEST_REPORT_FILE=test_results.json"

REM Parse arguments
:parse_args
if "%~1"=="" goto :done_parse
if /I "%~1"=="--all" set "RUN_ALL=1"
if /I "%~1"=="--unit" set "RUN_ALL=0" & set "RUN_UNIT=1"
if /I "%~1"=="--integration" set "RUN_ALL=0" & set "RUN_INTEGRATION=1"
if /I "%~1"=="--mock" set "RUN_ALL=0" & set "RUN_MOCK=1"
if /I "%~1"=="--stability" set "RUN_ALL=0" & set "RUN_STABILITY=1"
if /I "%~1"=="--report" set "GENERATE_REPORT=1"
if /I "%~1"=="--verbose" set "VERBOSE=1"
if /I "%~1"=="--build-dir" (
    set "BUILD_DIR=%~2"
    shift
)
shift
goto :parse_args
:done_parse

echo ========================================
echo FlipGearboxFactoryTest - Test Runner
echo ========================================

REM Auto-detect Qt
set "QT_DIR="
for %%D in (D E F C) do (
    if exist "%%D:\Qt\6.11.0\mingw_64\bin" (
        set "QT_DIR=%%D:\Qt\6.11.0\mingw_64"
        goto :qt_found
    )
)
if defined QT_DIR (
    if exist "%QT_DIR%\bin" goto :qt_found
)
echo [ERROR] Qt not found
exit /b 1
:qt_found
set "PATH=%QT_DIR%\bin;%QT_DIR%\..\..\Tools\mingw1310_64\bin;%PATH%"
set "QT_QPA_PLATFORM=offscreen"
set "QT_QUICK_CONTROLS_STYLE=Basic"

REM Auto-detect build directory
if "%BUILD_DIR%"=="" (
    for /d %%d in (build\Desktop_Qt_*) do (
        if exist "%%d\CMakeCache.txt" (
            set "BUILD_DIR=%%d"
            goto :build_found
        )
    )
)
:build_found
if "%BUILD_DIR%"=="" (
    echo [ERROR] Build directory not found! Run build_project.bat first.
    exit /b 1
)

echo Build Directory: %BUILD_DIR%
echo QT_QPA_PLATFORM: %QT_QPA_PLATFORM%
echo.

cd "%BUILD_DIR%"

REM Define test categories
set "UNIT_TESTS=ModbusCrcTests ProtocolLayerTests DomainEngineTests DomainEngineAdvancedTests JudgementLogicTests BrakePowerConstantVoltageTest HistoryViewModelTests RecipeViewModelTests TestExecutionPageTests AutoTestFrameworkTests PerformanceMonitorTests TestReportGeneratorTests"
set "INTEGRATION_TESTS=TestExecutionVerification GearboxSimulationIntegrationTests AngleTestMagnetIntegrationTests MagnetDetectionMockTests BoundaryProtectionTests"
set "MOCK_TESTS=SimulationRuntimeTests MockMotorMagnetDetectionTests MockTestRunnerTests"
set "STABILITY_TESTS=LongRunningStabilityTest"
set "ALL_TESTS=%UNIT_TESTS% %INTEGRATION_TESTS% %MOCK_TESTS% %STABILITY_TESTS%"

REM Determine which tests to run
set "TESTS_TO_RUN="
if %RUN_ALL%==1 set "TESTS_TO_RUN=%ALL_TESTS%"
if %RUN_UNIT%==1 set "TESTS_TO_RUN=%UNIT_TESTS%"
if %RUN_INTEGRATION%==1 set "TESTS_TO_RUN=%INTEGRATION_TESTS%"
if %RUN_MOCK%==1 set "TESTS_TO_RUN=%MOCK_TESTS%"
if %RUN_STABILITY%==1 set "TESTS_TO_RUN=%STABILITY_TESTS%"

set /a TOTAL=0
set /a PASSED=0
set /a FAILED=0
set /a SKIPPED=0

set "RESULT_LOG=test_run_results.log"
set "DETAIL_LOG=test_run_details.log"

REM Clear previous logs
echo. > "%RESULT_LOG%"
echo. > "%DETAIL_LOG%"

echo [*] Starting test execution...
echo.

for %%T in (%TESTS_TO_RUN%) do (
    set /a TOTAL+=1
    set "TEST_NAME=%%T"
    echo ----------------------------------------
    echo Running: !TEST_NAME!
    echo ----------------------------------------

    if exist "!TEST_NAME!.exe" (
        if %VERBOSE%==1 (
            "!TEST_NAME!.exe" -o - -txt 2>&1
        ) else (
            "!TEST_NAME!.exe" -o - -txt > "tmp_test_out.txt" 2>&1
        )
        set "TEST_RESULT=!ERRORLEVEL!"

        if !TEST_RESULT!==0 (
            echo [PASS] !TEST_NAME!
            set /a PASSED+=1
            echo PASS:!TEST_NAME! >> "%RESULT_LOG%"
        ) else (
            echo [FAIL] !TEST_NAME! (exit code: !TEST_RESULT!)
            if %VERBOSE%==0 (
                echo --- Output ---
                type "tmp_test_out.txt"
                echo --- End Output ---
            )
            set /a FAILED+=1
            echo FAIL:!TEST_NAME! >> "%RESULT_LOG%"
        )
    ) else (
        echo [SKIP] !TEST_NAME! (executable not found)
        set /a SKIPPED+=1
        echo SKIP:!TEST_NAME! >> "%RESULT_LOG%"
    )
    echo.
)

if exist "tmp_test_out.txt" del "tmp_test_out.txt"

echo ========================================
echo Test Execution Summary
echo ========================================
echo Total:   %TOTAL%
echo Passed:  %PASSED%
echo Failed:  %FAILED%
echo Skipped: %SKIPPED%
echo ========================================

REM Generate JSON report if requested
if %GENERATE_REPORT%==1 (
    echo [*] Generating test report...
    (
        echo {
        echo   "project": "FlipGearboxFactoryTest",
        echo   "timestamp": "%date% %time%",
        echo   "summary": {
        echo     "total": %TOTAL%,
        echo     "passed": %PASSED%,
        echo     "failed": %FAILED%,
        echo     "skipped": %SKIPPED%
        echo   },
        echo   "results": [
    ) > "%TEST_REPORT_FILE%"

    set "FIRST=1"
    for /F "tokens=1,2 delims=:" %%A in (%RESULT_LOG%) do (
        if !FIRST!==1 (
            set "FIRST=0"
        ) else (
            echo     , >> "%TEST_REPORT_FILE%"
        )
        (
            echo     {
            echo       "name": "%%B",
            echo       "status": "%%A"
            echo     }
        ) >> "%TEST_REPORT_FILE%"
    )

    (
        echo
        echo   ]
        echo }
    ) >> "%TEST_REPORT_FILE%"

    echo [OK] Report saved to: %BUILD_DIR%\%TEST_REPORT_FILE%
)

cd ..\..

if %FAILED% gtr 0 (
    echo [ERROR] Some tests failed!
    exit /b 1
)

exit /b 0
