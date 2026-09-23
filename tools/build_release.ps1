# Build the CMC download zips from the native C++ build.
# Usage:
#   native\build.bat
#   powershell -File tools\build_release.ps1
$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

$match = Select-String -Path "native\src\engine\words.cpp" -Pattern 'VERSION = "([^"]+)"'
if (-not $match) { throw "Could not find the CMC version." }
$version = $match.Matches[0].Groups[1].Value

$dist = Join-Path $root "dist"
$stage = Join-Path $dist "_stage"
Remove-Item $stage -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $stage | Out-Null

$startHere = @"
CAVE MAN CODE (CMC) v$version
================================

Everything here is built with C++.  You do not need Python.

QUICK START
-----------
1. Double-click  OGABOOGA CODER.exe  to open the friendly
   block + text coding cave.

2. To run a program, drag any .cmc file onto  cmc.exe .
   Double-click cmc.exe with no file to talk to CMC
   line by line (the REPL).

3. Both programs live right here:
      OGABOOGA CODER.exe   the IDE
      cmc.exe              the compiler and runner

BUILD IT YOURSELF (optional)
----------------------------
You need a C++17 compiler (g++ or clang).  The full package
includes the source in  native\ .  Then run:

      native\build.bat

WHAT IS WHERE
-------------
  OGABOOGA CODER.exe   the IDE
  cmc.exe              the compiler and runner
  examples\            example .cmc programs
  docs\                the manual, tutorial, and language spec
  website\             the offline wiki (open website\index.html)
  native\              (full package) the C++ source and build files
  tools\               (full package) release builder

LEARN CMC
---------
Read  docs\02_TUTORIAL_30_MINUTES.md  or open  website\index.html .

MIT License.  Go make something silly.
"@
Set-Content -Path (Join-Path $stage "START_HERE.txt") -Value $startHere -Encoding UTF8

$commonFiles = @("README.md", "LICENSE", "CHEATSHEET.md", "START_HERE.txt")

function Stage-Package {
    param(
        [string]$Name,
        [string[]]$Directories,
        [string[]]$Files,
        [string[]]$RemovePaths = @()
    )
    $target = Join-Path $stage $Name
    New-Item -ItemType Directory -Force -Path $target | Out-Null
    foreach ($file in $Files) {
        $source = Join-Path $root $file
        if (-not (Test-Path $source)) { $source = Join-Path $stage $file }
        Copy-Item $source (Join-Path $target $file) -Force
    }
    Copy-Item (Join-Path $root "bin\ogabooga.exe") (Join-Path $target "OGABOOGA CODER.exe") -Force
    Copy-Item (Join-Path $root "bin\cmc.exe") (Join-Path $target "cmc.exe") -Force
    foreach ($dir in $Directories) {
        $source = Join-Path $root $dir
        if (Test-Path $source) {
            Copy-Item $source (Join-Path $target $dir) -Recurse -Force
        }
    }
    foreach ($remove in $RemovePaths) {
        Remove-Item (Join-Path $target $remove) -Recurse -Force -ErrorAction SilentlyContinue
    }
    Get-ChildItem $target -Recurse -Directory -Filter "__pycache__" -ErrorAction SilentlyContinue |
        Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
    Get-ChildItem $target -Recurse -File -Include "*.pyc", "*.pyo" -ErrorAction SilentlyContinue |
        Remove-Item -Force -ErrorAction SilentlyContinue

    $zip = Join-Path $dist ($Name + ".zip")
    Remove-Item $zip -Force -ErrorAction SilentlyContinue
    Compress-Archive -Path $target -DestinationPath $zip -CompressionLevel Optimal
    Write-Host ("Made " + $zip)
}

Stage-Package -Name ("CaveManCode-v" + $version) `
    -Directories @("native", "examples", "docs", "tools", "site_content", "website") `
    -Files $commonFiles `
    -RemovePaths @("website\downloads", "native\cli_sources.rsp", "native\ide_sources.rsp")

Stage-Package -Name ("CMC-Compiler-v" + $version) `
    -Directories @("examples", "docs", "website") `
    -Files $commonFiles `
    -RemovePaths @("website\downloads")

Stage-Package -Name ("OGABOOGA-CODER-v" + $version) `
    -Directories @("examples", "docs", "website") `
    -Files $commonFiles `
    -RemovePaths @("website\downloads")

$downloads = Join-Path $root "website\downloads"
New-Item -ItemType Directory -Force -Path $downloads | Out-Null
Get-ChildItem (Join-Path $dist "*.zip") | ForEach-Object {
    Copy-Item $_.FullName (Join-Path $downloads $_.Name) -Force
}

Remove-Item $stage -Recurse -Force -ErrorAction SilentlyContinue
Write-Host ""
Write-Host "Zips are in dist\ and website\downloads\"
