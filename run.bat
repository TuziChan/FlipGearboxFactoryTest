@echo off
setlocal

set "QT_BIN=F:\Qt\6.11.0\llvm-mingw_64\bin"
set "MINGW_BIN=F:\Qt\Tools\llvm-mingw1706_64\bin"
set "PATH=%QT_BIN%;%MINGW_BIN%;%PATH%"

set "PROJECT_ROOT=%~dp0"
set "BUILD_DIR=%PROJECT_ROOT%build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug"
set "EXE_NAME=appFlipGearboxFactoryTest.exe"

echo ============================================
echo   FlipGearboxFactoryTest - Build and Run
echo ============================================
echo.

tasklist /FI "IMAGENAME eq %EXE_NAME%" 2>nul | find /i "%EXE_NAME%" >nul
if %errorlevel% equ 0 (
    echo [!] Closing existing %EXE_NAME% process...
    taskkill /F /IM "%EXE_NAME%" >nul 2>&1
    timeout /t 2 /nobreak >nul
    echo [OK] Existing process closed.
    echo.
)

if not exist "%BUILD_DIR%" (
    echo [ERROR] Build directory does not exist: %BUILD_DIR%
    echo         Run the initial CMake configure step in Qt Creator first.
    exit /b 1
)

echo [*] Building...
echo.
cmake --build "%BUILD_DIR%"
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed.
    exit /b 1
)

echo.
echo [OK] Build succeeded.
echo.

echo [*] Launching application...
echo.
pushd "%BUILD_DIR%"
start "" "%EXE_NAME%"
popd

echo [OK] Application launched.
echo.
echo Notes:
echo   1. Serial port connection errors are expected without hardware.
echo   2. The UI can still be opened and tested.
echo   3. Close the app window to exit.
echo.

exit /b 0
