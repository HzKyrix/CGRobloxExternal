@echo off
setlocal
title CHEAT GLOBAL - Offset Updater
cd /d "%~dp0"

echo.
echo  CHEAT GLOBAL - OFFSET UPDATER
echo  =============================
echo.

powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\Update-Offsets.ps1" %*
set "RESULT=%ERRORLEVEL%"

echo.
if not "%RESULT%"=="0" (
    echo [!] Offset update failed. Existing offsets were preserved.
) else (
    echo [+] Offset update completed successfully.
)

echo.
pause
exit /b %RESULT%
