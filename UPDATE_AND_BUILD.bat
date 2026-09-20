@echo off
setlocal
title CHEAT GLOBAL - Update and Build
cd /d "%~dp0"

echo.
echo  CHEAT GLOBAL - UPDATE AND BUILD
echo  ===============================
echo.

powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\Update-Offsets.ps1"
if errorlevel 1 (
    echo.
    echo [!] Offset update failed. Build was cancelled.
    echo.
    pause
    exit /b 1
)

echo.
echo [*] Offsets updated. Starting Release x64 build...
echo.
call "%~dp0BUILD_RELEASE.bat"
exit /b %ERRORLEVEL%
