param(
  [Parameter(Mandatory=$true)][string]$BuildDir,
  [string]$QtBin = $env:QT_BIN,
  [string]$IfwBin = $env:QT_IFW_BIN,
  [string]$Version = $env:LMG_VERSION
)
$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($Version)) { $Version = '0.7.0' }
$Root = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$Suffix = if ($env:LMG_PLATFORM_SUFFIX) { $env:LMG_PLATFORM_SUFFIX } else { 'x86_64' }
$Dist = Join-Path $Root 'dist/windows'
$Stage = Join-Path $Dist 'portable'
Remove-Item $Dist -Recurse -Force -ErrorAction SilentlyContinue
New-Item $Stage -ItemType Directory -Force | Out-Null
$Exe = Get-ChildItem $BuildDir -Filter LeoMiniGames.exe -Recurse | Select-Object -First 1
if (-not $Exe) { throw 'LeoMiniGames.exe not found in build directory.' }
Copy-Item $Exe.FullName $Stage
$LegalFiles = @('LICENSE','NOTICE','COPYRIGHT','LICENSING.md')
foreach ($Legal in $LegalFiles) { Copy-Item (Join-Path $Root $Legal) $Stage }
New-Item (Join-Path $Stage 'licenses') -ItemType Directory -Force | Out-Null
Copy-Item (Join-Path $Root 'licenses/LEOMINIGAMES_PLUGIN_EXCEPTION_1.0.txt') (Join-Path $Stage 'licenses')
Copy-Item (Join-Path $Root 'licenses/YOUNGLION_MOD_LICENSE_1.0.txt') (Join-Path $Stage 'licenses')
$Deploy = if ($QtBin) { Join-Path $QtBin 'windeployqt.exe' } else { (Get-Command windeployqt.exe -ErrorAction Stop).Source }
& $Deploy --release --qmldir (Join-Path $Root 'qml') (Join-Path $Stage 'LeoMiniGames.exe')
if ($LASTEXITCODE -ne 0) { throw "windeployqt failed: $LASTEXITCODE" }
$Portable = Join-Path $Dist "LeoMiniGames-v$Version-Windows-$Suffix.zip"
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
if (-not (Test-Path $Creator)) { throw "binarycreator.exe not found: $Creator" }
$Setup = Join-Path $Dist "LeoMiniGames-v$Version-Windows-$Suffix-Setup.exe"
& $Creator --offline-only -c (Join-Path $Root 'installer/config/config.xml') -p (Join-Path $Root 'installer/packages') --include xyz.younglion.leominigames $Setup
if ($LASTEXITCODE -ne 0 -or -not (Test-Path $Setup)) { throw 'QtIFW installer build failed.' }
Write-Host "Portable: $Portable"
Write-Host "Installer: $Setup"
