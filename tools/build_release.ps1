# Build the CMC download zips. Usage:
#   powershell -File tools/build_release.ps1
$ErrorActionPreference = "Stop"
Set-Location (Split-Path -Parent $PSScriptRoot)
python tools/build_release.py
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host ""
Write-Host "Zips are in dist\ and website\downloads\"
Write-Host "Next: python tools\build_site.py"