param(
  [Parameter(Mandatory=$true)][string]$BuildDir,
  [string]$QtBin = $env:QT_BIN,
  [string]$IfwBin = $env:QT_IFW_BIN
)
$ErrorActionPreference = 'Stop'
$Root = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$Dist = Join-Path $Root 'dist/windows'
$Stage = Join-Path $Dist 'portable'
Remove-Item $Dist -Recurse -Force -ErrorAction SilentlyContinue
New-Item $Stage -ItemType Directory -Force | Out-Null
$Exe = Get-ChildItem $BuildDir -Filter LeoMiniGames.exe -Recurse | Select-Object -First 1
if (-not $Exe) { throw 'LeoMiniGames.exe not found in build directory.' }
Copy-Item $Exe.FullName $Stage
$Deploy = if ($QtBin) { Join-Path $QtBin 'windeployqt.exe' } else { (Get-Command windeployqt.exe -ErrorAction Stop).Source }
& $Deploy --release --qmldir (Join-Path $Root 'qml') (Join-Path $Stage 'LeoMiniGames.exe')
if ($LASTEXITCODE -ne 0) { throw "windeployqt failed: $LASTEXITCODE" }
$Portable = Join-Path $Dist 'LeoMiniGames-v0.7.0-Windows-x86_64.zip'
Compress-Archive -Path (Join-Path $Stage '*') -DestinationPath $Portable -Force

$PkgData = Join-Path $Root 'installer/packages/xyz.younglion.leominigames/data'
Remove-Item $PkgData -Recurse -Force -ErrorAction SilentlyContinue
New-Item $PkgData -ItemType Directory -Force | Out-Null
Copy-Item (Join-Path $Stage '*') $PkgData -Recurse
if ($IfwBin) {
  $Creator = Join-Path $IfwBin 'binarycreator.exe'
} else {
  $Creator = (Get-Command binarycreator.exe -ErrorAction Stop).Source
}
$Setup = Join-Path $Dist 'LeoMiniGames-v0.7.0-Windows-x86_64-Setup.exe'
& $Creator --offline-only -c (Join-Path $Root 'installer/config/config.xml') -p (Join-Path $Root 'installer/packages') --include xyz.younglion.leominigames $Setup
if ($LASTEXITCODE -ne 0 -or -not (Test-Path $Setup)) { throw 'QtIFW installer build failed.' }
Write-Host "Portable: $Portable"
Write-Host "Installer: $Setup"
