# Downloads otelcol-contrib (the OpenTelemetry Collector Contrib Windows
# binary) used as the Windows BDD oracle, verifies its SHA-256, and extracts
# the executable into Bdd/otel/bin/.
#
# Idempotent: if the binary is already present, the script does nothing.
# Used by both the developer inner loop and the bdd-windows CI job.

[CmdletBinding()]
param(
    [string]$Version = '0.150.1',
    [string]$Sha256  = '53466D9A16B0D8BC6CB9A7A24EDA306751AFBD28BB115BEE1D2B11E9AF65A334',
    [string]$BinDir
)

$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrEmpty($BinDir))
{
    $BinDir = Join-Path $PSScriptRoot 'bin'
}

$exePath       = Join-Path $BinDir 'otelcol-contrib.exe'
$versionMarker = Join-Path $BinDir 'otelcol-contrib.version'

# Fast-path only when both the exe and a matching version marker exist;
# otherwise re-download so a stale or hand-replaced binary can't silently
# bypass the pinned $Version / $Sha256.
if ((Test-Path $exePath) -and (Test-Path $versionMarker) -and
    ((Get-Content $versionMarker -Raw).Trim() -eq $Version))
{
    Write-Host "otelcol-contrib v$Version already installed at $exePath"
    return
}

New-Item -ItemType Directory -Force -Path $BinDir | Out-Null

$archive    = "otelcol-contrib_${Version}_windows_amd64.tar.gz"
$url        = "https://github.com/open-telemetry/opentelemetry-collector-releases/releases/download/v${Version}/${archive}"
$tarballDst = Join-Path $BinDir $archive

Write-Host "Downloading $url -> $tarballDst"
Invoke-WebRequest -Uri $url -OutFile $tarballDst -UseBasicParsing

$actual = (Get-FileHash -Algorithm SHA256 $tarballDst).Hash
if ($actual -ne $Sha256)
{
    Remove-Item $tarballDst -Force
    throw "SHA-256 mismatch for $archive`nExpected: $Sha256`nActual:   $actual"
}

# Use the system tar (Windows ships bsdtar at C:\Windows\System32\tar.exe);
# Git Bash's tar gets shadowed in PATH when running from a bash-launched
# PowerShell and mishandles Windows-style -C arguments.
$systemTar = Join-Path $env:SystemRoot 'System32\tar.exe'
Write-Host "Extracting otelcol-contrib.exe via $systemTar"
& $systemTar -xzf $tarballDst -C $BinDir otelcol-contrib.exe
if (-not (Test-Path $exePath))
{
    throw "Extraction failed: $exePath not present"
}

Remove-Item $tarballDst -Force
Set-Content -Path $versionMarker -Value $Version -NoNewline
Write-Host "Installed otelcol-contrib v$Version at $exePath"
