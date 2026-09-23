@echo off
rem Build the web IDE and copy it into website\ide for GitHub Pages.
rem Usage:  web\build.bat
setlocal
pushd "%~dp0.."

echo === 1/4 building the C++ engine as WebAssembly ===
call native\wasm\build_wasm.bat
if errorlevel 1 (
    echo WebAssembly build failed
    popd
    exit /b 1
)

echo === 2/4 refreshing the example programs ===
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$items = Get-ChildItem 'examples' -Filter *.cmc | Sort-Object Name | ForEach-Object { [pscustomobject]@{ name = $_.BaseName; file = $_.Name; source = [IO.File]::ReadAllText($_.FullName) } }; [IO.File]::WriteAllText('web\app\public\examples\examples.json', ($items | ConvertTo-Json -Depth 3))"
if errorlevel 1 (
    echo Example refresh failed
    popd
    exit /b 1
)

echo === 3/4 building the React app ===
pushd web\app
if not exist node_modules (
    call npm install
    if errorlevel 1 (
        popd
        popd
        exit /b 1
    )
)
call npm run build
if errorlevel 1 (
    popd
    popd
    exit /b 1
)
popd

echo === 4/4 copying into website\ide ===
if exist "website\ide" rmdir /s /q "website\ide"
xcopy "web\app\dist" "website\ide" /e /i /q >nul
if errorlevel 1 (
    echo Copy failed
    popd
    exit /b 1
)

echo.
echo Web IDE is ready in website\ide\
echo Open website\ide\index.html or push to GitHub Pages.
popd
