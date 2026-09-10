$ErrorActionPreference = 'Stop'
$env:PSModulePath = "$PSHOME\Modules"
$tutorPort = 38761
$tutorUrl = "http://127.0.0.1:$tutorPort"
function Test-Tutor {
    try {
        $tutorStatus = Invoke-RestMethod -Uri "$tutorUrl/api/session" -TimeoutSec 2
        return $tutorStatus.app -eq 'BlasterLabTutor'
    } catch { return $false }
}
try {
    if (-not (Test-Tutor)) {
        $tutorNode = (Get-Command node.exe -ErrorAction SilentlyContinue).Source
        if (-not $tutorNode) { throw 'Node.js 22+ is required. Install Node.js, then run this launcher again.' }
        if ([int]((& $tutorNode --version) -replace '^v(\d+).*','$1') -lt 22) { throw 'Please use Node.js 22 or later.' }
        $tutorLogDir = Join-Path $env:LOCALAPPDATA 'BlasterLab/assistant'
        New-Item -ItemType Directory -Force -Path $tutorLogDir | Out-Null
        $tutorScript = Join-Path $PSScriptRoot 'tutor-server.cjs'
        $tutorProcess = Start-Process -FilePath $tutorNode -ArgumentList @('"' + $tutorScript + '"') -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $tutorLogDir 'server.log') -RedirectStandardError (Join-Path $tutorLogDir 'server-error.log')
        $tutorReady = $false
        for ($attempt = 0; $attempt -lt 30; $attempt++) {
            Start-Sleep -Milliseconds 300
            if (Test-Tutor) { $tutorReady = $true; break }
            if ($tutorProcess.HasExited) { break }
        }
        if (-not $tutorReady) { throw "The tutor did not start. Port $tutorPort may be occupied. See $tutorLogDir/server-error.log." }
    }
    Start-Process $tutorUrl
    Write-Host 'Blaster Lab is open. Click the character, then API settings to enter your key.'
} catch {
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}
