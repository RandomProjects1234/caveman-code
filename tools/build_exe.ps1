# Build stand-alone .exe files with PyInstaller (optional).
# First install PyInstaller once:  pip install pyinstaller
# Then run:                        powershell -File tools/build_exe.ps1
$ErrorActionPreference = "Stop"
Set-Location (Split-Path -Parent $PSScriptRoot)

python -c "import PyInstaller" 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Host "PyInstaller is not installed. Run this first:"
    Write-Host "    pip install pyinstaller"
    exit 1
}

Write-Host "Building OgaboogaCoder.exe (this takes a minute)..."
python -m PyInstaller --noconfirm --onefile --windowed `
    --name OgaboogaCoder `
    --paths src `
    --add-data "examples;examples" `
    --add-data "docs;docs" `
    tools/entry_ogabooga.py
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Building cmc.exe ..."
python -m PyInstaller --noconfirm --onefile --console `
    --name cmc `
    --paths src `
    --add-data "examples;examples" `
    tools/entry_cmc.py
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host ""
Write-Host "Done! Look in the dist folder:"
Write-Host "    dist\OgaboogaCoder.exe"
Write-Host "    dist\cmc.exe"