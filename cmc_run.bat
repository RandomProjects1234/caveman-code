@echo off
rem Run a CMC program. Drag any .cmc file onto this file, or double-click
rem to start the talk-back REPL.
setlocal
cd /d "%~dp0"
if not exist "bin\cmc.exe" (
    echo The CMC compiler is not built yet.
    echo Build it with:  native\build.bat
    pause
    exit /b 1
)
if "%~1"=="" (
    "bin\cmc.exe"
) else (
    "bin\cmc.exe" run "%~1"
)
if errorlevel 1 pause
