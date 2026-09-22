@echo off
rem OGABOOGA CODER launcher -- double-click me!
setlocal
cd /d "%~dp0"
set "PYTHONPATH=%~dp0src"
set "PY=python"
where python >nul 2>nul || set "PY=py -3"
%PY% -m ogabooga
if errorlevel 1 pause