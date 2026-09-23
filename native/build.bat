@echo off
setlocal enabledelayedexpansion
pushd "%~dp0"

if not exist "..\bin" mkdir "..\bin"

set "ENGINE="
for /r "src\engine" %%f in (*.cpp) do set "ENGINE=!ENGINE! "%%f""
set "CLISRC="
for /r "src\cli" %%f in (*.cpp) do set "CLISRC=!CLISRC! "%%f""
set "IDESRC="
for /r "src\ide" %%f in (*.cpp) do set "IDESRC=!IDESRC! "%%f""

echo %ENGINE% %CLISRC% > cli_sources.rsp
windres -I win win\cmc.rc -o cmc_res.o
if errorlevel 1 (
    echo CMC RESOURCES FAILED
    popd
    exit /b 1
)
g++ -std=c++17 -O2 -Wall -Wextra -static -s -Isrc\engine -Isrc\ide @cli_sources.rsp cmc_res.o -o "..\bin\cmc.exe" -luser32 -lgdi32 -lshell32
if errorlevel 1 (
    echo CMC BUILD FAILED
    popd
    exit /b 1
)

echo %ENGINE% %IDESRC% > ide_sources.rsp
windres -I win win\ogabooga.rc -o ogabooga_res.o
if errorlevel 1 (
    echo OGABOOGA RESOURCES FAILED
    popd
    exit /b 1
)
g++ -std=c++17 -O2 -Wall -Wextra -static -s -mwindows -Isrc\engine -Isrc\ide @ide_sources.rsp ogabooga_res.o -o "..\bin\ogabooga.exe" -luser32 -lgdi32 -lcomdlg32 -lcomctl32 -lshell32 -lole32
if errorlevel 1 (
    echo OGABOOGA BUILD FAILED
    popd
    exit /b 1
)

del cli_sources.rsp ide_sources.rsp cmc_res.o ogabooga_res.o >nul 2>nul
echo Built %~dp0..\bin\cmc.exe
echo Built %~dp0..\bin\ogabooga.exe
popd