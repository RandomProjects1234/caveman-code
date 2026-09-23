@echo off
setlocal
pushd "%~dp0"

if not exist "C:\Users\milov\emsdk\emsdk_env.bat" (
    echo Emscripten is not installed yet. Run the emsdk install first.
    popd
    exit /b 1
)
call "C:\Users\milov\emsdk\emsdk_env.bat" >nul 2>&1

set OUT=%~dp0..\..\web\app\public\wasm
if not exist "%OUT%" mkdir "%OUT%"

em++ -O2 -std=c++17 -fexceptions ^
    -I..\src\engine -I..\src\ide -I. ^
    ..\src\engine\errors.cpp ^
    ..\src\engine\textutil.cpp ^
    ..\src\engine\values.cpp ^
    ..\src\engine\words.cpp ^
    ..\src\engine\words_data.cpp ^
    ..\src\engine\lexer.cpp ^
    ..\src\engine\parser.cpp ^
    ..\src\engine\interpreter.cpp ^
    ..\src\engine\builtins.cpp ^
    ..\src\engine\codegen.cpp ^
    ..\src\engine\draw.cpp ^
    ..\src\engine\paths.cpp ^
    ..\src\ide\ide_blocks.cpp ^
    wasm_api.cpp ^
    --js-library wasm_js.js ^
    -sMODULARIZE=1 -sEXPORT_ES6=1 -sEXPORT_NAME=createCmcModule ^
    -sENVIRONMENT=web,node -sALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=2MB ^
    -sASYNCIFY=1 -sASYNCIFY_IMPORTS=cmc_js_ask_async ^
    -sDISABLE_EXCEPTION_CATCHING=0 ^
    -sEXPORTED_FUNCTIONS=_cmc_version,_cmc_run,_cmc_compile,_cmc_check,_cmc_blocks_specs,_cmc_blocks_from_source,_cmc_blocks_to_source,_cmc_blocks_check,_cmc_dictionary,_cmc_last,_cmc_free,_malloc,_free ^
    -sEXPORTED_RUNTIME_METHODS=cwrap,UTF8ToString,lengthBytesUTF8,stringToUTF8 ^
    -o "%OUT%\cmc.js"

if errorlevel 1 (
    echo WASM BUILD FAILED
    popd
    exit /b 1
)
echo Built %OUT%\cmc.js and cmc.wasm
popd
