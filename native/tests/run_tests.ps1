param(
    [switch]$Update
)

$ErrorActionPreference = "Stop"

$testsDir = $PSScriptRoot
$root = Split-Path -Parent (Split-Path -Parent $testsDir)
$exe = Join-Path $root "bin\cmc.exe"
$ide = Join-Path $root "bin\ogabooga.exe"
$examples = Join-Path $root "examples"
$expectedDir = Join-Path $testsDir "expected"
New-Item -ItemType Directory -Force -Path $expectedDir | Out-Null

function Invoke-Process {
    param([string]$FileName, [string]$Arguments, [string]$WorkDir, [string]$StdinText,
          [string]$ExtraEnvName, [string]$ExtraEnvValue)
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $FileName
    $psi.Arguments = $Arguments
    $psi.WorkingDirectory = $WorkDir
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.RedirectStandardInput = $true
    $psi.StandardOutputEncoding = [System.Text.Encoding]::UTF8
    $psi.StandardErrorEncoding = [System.Text.Encoding]::UTF8
    if ($ExtraEnvName) { $psi.EnvironmentVariables[$ExtraEnvName] = $ExtraEnvValue }
    $p = [System.Diagnostics.Process]::Start($psi)
    if ($null -ne $StdinText) { $p.StandardInput.Write($StdinText) }
    $p.StandardInput.Close()
    $outTask = $p.StandardOutput.ReadToEndAsync()
    $errTask = $p.StandardError.ReadToEndAsync()
    $timedOut = $false
    if (-not $p.WaitForExit(120000)) {
        $timedOut = $true
        try { $p.Kill() } catch { }
        $p.WaitForExit()
    }
    return [pscustomobject]@{
        Out = $outTask.Result
        Err = $errTask.Result
        Code = $p.ExitCode
        TimedOut = $timedOut
    }
}

function Normalize {
    param([string]$Text)
    if ($null -eq $Text) { return "" }
    return ($Text -replace "`r`n", "`n")
}

$pass = 0
$fail = 0

Write-Host "checking cmc.exe ..."
$version = Invoke-Process -FileName $exe -Arguments "version" -WorkDir $root -StdinText ""
if ($version.Code -ne 0 -or $version.Out -notmatch "Cave Man Code \(CMC\) v") {
    Write-Host "FAIL  cmc.exe version" -ForegroundColor Red
    $fail++
} else {
    Write-Host ("PASS  " + (Normalize $version.Out).Trim())
    $pass++
}

Write-Host "checking ogabooga.exe selftest ..."
$selftest = Invoke-Process -FileName $ide -Arguments "--selftest" -WorkDir $root -StdinText ""
if ($selftest.Code -ne 0) {
    Write-Host "FAIL  ogabooga --selftest" -ForegroundColor Red
    $fail++
} else {
    Write-Host "PASS  ogabooga --selftest"
    $pass++
}

Write-Host ""
Write-Host "running every example ..."
$files = Get-ChildItem -Path $examples -Filter "*.cmc" | Sort-Object Name
foreach ($file in $files) {
    $run = Invoke-Process -FileName $exe -Arguments ('run "' + $file.FullName + '"') -WorkDir $root `
        -StdinText "" -ExtraEnvName "CMC_NO_DRAW" -ExtraEnvValue "1"
    $out = Normalize $run.Out
    $err = Normalize $run.Err

    $outFile = Join-Path $expectedDir ($file.BaseName + ".out")
    $errFile = Join-Path $expectedDir ($file.BaseName + ".err")
    $codeFile = Join-Path $expectedDir ($file.BaseName + ".code")

    if ($Update) {
        [IO.File]::WriteAllText($outFile, $out)
        [IO.File]::WriteAllText($errFile, $err)
        [IO.File]::WriteAllText($codeFile, [string]$run.Code)
        Write-Host ("SNAP  " + $file.Name)
        continue
    }

    if ($run.TimedOut) {
        Write-Host ("FAIL  " + $file.Name + " (timeout)") -ForegroundColor Red
        $fail++
        continue
    }

    $source = [IO.File]::ReadAllText($file.FullName)
    if ($source -match "munga") {
        $wantErrRandom = Normalize ([IO.File]::ReadAllText($errFile))
        $wantCodeRandom = ([IO.File]::ReadAllText($codeFile)).Trim()
        if (([string]$run.Code) -eq $wantCodeRandom -and $err -eq $wantErrRandom) {
            Write-Host ("PASS  " + $file.Name + " (random output, checked exit and errors)")
            $pass++
        } else {
            Write-Host ("FAIL  " + $file.Name + " (random output)") -ForegroundColor Red
            Write-Host ("  exit: got " + $run.Code + " want " + $wantCodeRandom)
            Write-Host ("  stderr got : " + ($err -replace "`n", " | "))
            Write-Host ("  stderr want: " + ($wantErrRandom -replace "`n", " | "))
            $fail++
        }
        continue
    }

    $wantOut = Normalize ([IO.File]::ReadAllText($outFile))
    $wantErr = Normalize ([IO.File]::ReadAllText($errFile))
    $wantCode = ([IO.File]::ReadAllText($codeFile)).Trim()
    if ($out -eq $wantOut -and $err -eq $wantErr -and ([string]$run.Code) -eq $wantCode) {
        Write-Host ("PASS  " + $file.Name)
        $pass++
    } else {
        Write-Host ("FAIL  " + $file.Name) -ForegroundColor Red
        if (([string]$run.Code) -ne $wantCode) {
            Write-Host ("  exit: got " + $run.Code + " want " + $wantCode)
        }
        if ($out -ne $wantOut) {
            Write-Host ("  stdout got : " + ($out -replace "`n", " | "))
            Write-Host ("  stdout want: " + ($wantOut -replace "`n", " | "))
        }
        if ($err -ne $wantErr) {
            Write-Host ("  stderr got : " + ($err -replace "`n", " | "))
            Write-Host ("  stderr want: " + ($wantErr -replace "`n", " | "))
        }
        $fail++
    }
}

Write-Host ""
if ($Update) {
    Write-Host ("snapshots written: " + (Get-ChildItem $expectedDir -File).Count)
} else {
    Write-Host ("passed: " + $pass + "  failed: " + $fail)
    if ($fail -gt 0) { exit 1 }
}
