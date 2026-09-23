@echo off
rem OGABOOGA CODER launcher -- double-click me!
setlocal
cd /d "%~dp0"
if not exist "bin\ogabooga.exe" (
    echo OGABOOGA CODER is not built yet.
    echo Build it with:  native\build.bat
    pause
    exit /b 1
)
"bin\ogabooga.exe"
if errorlevel 1 pause
