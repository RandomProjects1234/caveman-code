@echo off
rem Run a CMC program. Drag any .cmc file onto this file, or double-click
rem to start the talk-back REPL.
setlocal
cd /d "%~dp0"
set "PYTHONPATH=%~dp0src"
set "PY=python"
where python >nul 2>nul || set "PY=py -3"
if "%~1"=="" (
    %PY% -m cmc
) else (
    %PY% -m cmc run "%~1"
)
if errorlevel 1 pause