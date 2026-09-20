@echo off
setlocal EnableExtensions
title CHEAT GLOBAL - Release Builder
cd /d "%~dp0"

set "PROJECT=%~dp0CGRobloxExternal\CGRobloxExternal.vcxproj"
set "OUTPUT=%~dp0CGRobloxExternal\x64\Release\CGRobloxExternal.exe"
set "MSBUILD="

for /f "delims=" %%I in ('where msbuild.exe 2^>nul') do (
    if not defined MSBUILD set "MSBUILD=%%I"
)

if not defined MSBUILD if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=%ProgramFiles%\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
)
if not defined MSBUILD if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=%ProgramFiles%\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
)
if not defined MSBUILD if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
)
if not defined MSBUILD if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
)

echo.
echo  CHEAT GLOBAL - RELEASE X64 BUILD
echo  ================================
echo.

if not exist "%PROJECT%" (
    echo [!] Project file not found:
    echo     %PROJECT%
    echo.
    pause
    exit /b 2
)

if not defined MSBUILD (
    echo [!] MSBuild was not found.
    echo     Install Visual Studio 2022 or Build Tools with:
    echo     "Desktop development with C++"
    echo.
    pause
    exit /b 3
)

echo [*] Builder: %MSBUILD%
echo [*] Project: %PROJECT%
echo.

"%MSBUILD%" "%PROJECT%" /m /t:Build /p:Configuration=Release /p:Platform=x64 /v:minimal
if errorlevel 1 (
    echo.
    echo [!] BUILD FAILED
    echo.
    pause
    exit /b 1
)

if not exist "%OUTPUT%" (
    echo.
    echo [!] Build completed but the output file was not found.
    echo.
    pause
    exit /b 4
)

echo.
echo [+] BUILD SUCCESSFUL
echo [+] Output:
echo     %OUTPUT%
echo.
pause
exit /b 0
