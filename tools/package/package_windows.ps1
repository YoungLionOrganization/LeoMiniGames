param(
  [Parameter(Mandatory=$true)][string]$BuildDir,
  [string]$QtBin = $env:QT_BIN,
  [string]$IfwBin = $env:QT_IFW_BIN,
  [string]$Version = $env:LMG_VERSION
)
$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($Version)) { $Version = '0.7.2' }
$Root = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$Suffix = if ($env:LMG_PLATFORM_SUFFIX) { $env:LMG_PLATFORM_SUFFIX } else { 'x86_64' }
$WindowsArch = if ($env:LMG_WINDOWS_ARCH) { $env:LMG_WINDOWS_ARCH.ToLowerInvariant() } elseif ($Suffix -ieq 'ARM64') { 'arm64' } else { 'x86_64' }
$CpuProfile = if ($env:LMG_WINDOWS_CPU_PROFILE) { $env:LMG_WINDOWS_CPU_PROFILE.ToLowerInvariant() } elseif ($Suffix -match 'AVX2') { 'avx2' } else { 'baseline' }
if ($WindowsArch -notin @('x86_64','arm64')) { throw "Unsupported LMG_WINDOWS_ARCH: $WindowsArch" }
if ($CpuProfile -notin @('baseline','avx2')) { throw "Unsupported LMG_WINDOWS_CPU_PROFILE: $CpuProfile" }
if ($WindowsArch -eq 'arm64' -and $CpuProfile -ne 'baseline') { throw 'ARM64 packages must use the baseline CPU profile.' }
if ($WindowsArch -eq 'arm64' -and $Suffix -cne 'ARM64') { throw "ARM64 packages must use LMG_PLATFORM_SUFFIX=ARM64, got $Suffix" }
if ($WindowsArch -eq 'x86_64' -and $CpuProfile -eq 'avx2' -and $Suffix -cne 'x86_64-AVX2') { throw "AVX2 packages must use LMG_PLATFORM_SUFFIX=x86_64-AVX2, got $Suffix" }
if ($WindowsArch -eq 'x86_64' -and $CpuProfile -eq 'baseline' -and $Suffix -cne 'x86_64') { throw "Baseline x64 packages must use LMG_PLATFORM_SUFFIX=x86_64, got $Suffix" }
if ($WindowsArch -eq 'x86_64' -and $CpuProfile -eq 'avx2') {
  $BuildLabel = 'Windows x64 — Intel/AMD AVX2 optimized'
} elseif ($WindowsArch -eq 'arm64') {
  $BuildLabel = 'Windows ARM64'
} else {
  $BuildLabel = 'Windows x64 — Intel/AMD baseline'
}
$ReleaseMetadataPath = Join-Path $Root 'release/release.json'
$CanonicalTrack = 'stable'
if (Test-Path $ReleaseMetadataPath) {
  $ReleaseMetadata = Get-Content $ReleaseMetadataPath -Raw -Encoding UTF8 | ConvertFrom-Json
  if ($ReleaseMetadata.update_track) { $CanonicalTrack = [string]$ReleaseMetadata.update_track }
}
$UpdateTrack = if ($env:LMG_UPDATE_TRACK) { $env:LMG_UPDATE_TRACK.ToLowerInvariant() } else { $CanonicalTrack.ToLowerInvariant() }
if ($UpdateTrack -notin @('stable','preview')) { throw "Invalid LMG_UPDATE_TRACK: $UpdateTrack" }
$Dist = Join-Path $Root 'dist/windows'
$Stage = Join-Path $Dist 'portable'
$IfwWork = Join-Path $Dist 'ifw-work'
$UpdateRepository = Join-Path $Dist ("update-repository/{0}" -f $Suffix)
Remove-Item $Dist -Recurse -Force -ErrorAction SilentlyContinue
New-Item $Stage -ItemType Directory -Force | Out-Null

$Exe = Get-ChildItem $BuildDir -Filter LeoMiniGames.exe -Recurse | Select-Object -First 1
if (-not $Exe) { throw 'LeoMiniGames.exe not found in build directory.' }
Copy-Item $Exe.FullName $Stage

$LegalFiles = @('LICENSE','NOTICE','COPYRIGHT','LICENSING.md','LICENSE_HISTORY.md','LICENSE_METADATA.json')
foreach ($Legal in $LegalFiles) { Copy-Item (Join-Path $Root $Legal) $Stage }
New-Item (Join-Path $Stage 'licenses') -ItemType Directory -Force | Out-Null
Copy-Item (Join-Path $Root 'licenses/YOUNGLION_LMG_SDK_LICENSE_1.0.txt') (Join-Path $Stage 'licenses')
Copy-Item (Join-Path $Root 'licenses/YOUNGLION_PACKAGE_LICENSE_1.0.txt') (Join-Path $Stage 'licenses')
New-Item (Join-Path $Stage 'Legal') -ItemType Directory -Force | Out-Null
Copy-Item (Join-Path $Root 'docs/THIRD_PARTY_NOTICES.md') (Join-Path $Stage 'Legal')
Copy-Item (Join-Path $Root 'docs/QT_LGPL_COMPLIANCE.md') (Join-Path $Stage 'Legal')

$BuildInfo = [ordered]@{
  schema = 1
  product = 'LeoMiniGames'
  version = $Version
  platform = 'windows'
  architecture = $WindowsArch
  cpu_profile = $CpuProfile
  build_label = $BuildLabel
}
$BuildInfo | ConvertTo-Json -Depth 4 | Set-Content -Path (Join-Path $Stage 'build-info.json') -Encoding UTF8

$Deploy = if ($QtBin) { Join-Path $QtBin 'windeployqt.exe' } else { (Get-Command windeployqt.exe -ErrorAction Stop).Source }
& $Deploy --release --qmldir (Join-Path $Root 'qml') (Join-Path $Stage 'LeoMiniGames.exe')
if ($LASTEXITCODE -ne 0) { throw "windeployqt failed: $LASTEXITCODE" }

$Portable = Join-Path $Dist "LeoMiniGames-v$Version-Windows-$Suffix.zip"
Compress-Archive -Path (Join-Path $Stage '*') -DestinationPath $Portable -Force

if ($IfwBin) {
  $Creator = Join-Path $IfwBin 'binarycreator.exe'
  $Repogen = Join-Path $IfwBin 'repogen.exe'
} else {
  $Creator = (Get-Command binarycreator.exe -ErrorAction Stop).Source
  $Repogen = (Get-Command repogen.exe -ErrorAction Stop).Source
}
if (-not (Test-Path $Creator)) { throw "binarycreator.exe not found: $Creator" }
if (-not (Test-Path $Repogen)) { throw "repogen.exe not found: $Repogen" }

$ConfigDir = Join-Path $IfwWork 'config'
$PackagesDir = Join-Path $IfwWork 'packages'
$PackageMeta = Join-Path $PackagesDir 'xyz.younglion.leominigames/meta'
$PackageData = Join-Path $PackagesDir 'xyz.younglion.leominigames/data'
New-Item $ConfigDir -ItemType Directory -Force | Out-Null
New-Item $PackageMeta -ItemType Directory -Force | Out-Null
New-Item $PackageData -ItemType Directory -Force | Out-Null
Copy-Item (Join-Path $Root 'installer/config/*') $ConfigDir -Recurse -Force
Copy-Item (Join-Path $Root 'installer/packages/xyz.younglion.leominigames/meta/*') $PackageMeta -Recurse -Force
Copy-Item (Join-Path $Stage '*') $PackageData -Recurse -Force

$ConfigPath = Join-Path $ConfigDir 'config.xml'
$PackageXmlPath = Join-Path $PackageMeta 'package.xml'
$ControlScriptPath = Join-Path $ConfigDir 'control.qs'
$InstallScriptPath = Join-Path $PackageMeta 'installscript.qs'
$RepoUrl = "https://leominigames.younglion.xyz/updates/qtifw/$UpdateTrack/windows/$Suffix"
$ReleaseDate = if ($env:LMG_RELEASE_DATE) { $env:LMG_RELEASE_DATE } else { (Get-Date).ToUniversalTime().ToString('yyyy-MM-dd') }

$ConfigText = Get-Content $ConfigPath -Raw -Encoding UTF8
$ConfigText = [regex]::Replace($ConfigText, '<Version>[^<]+</Version>', "<Version>$Version</Version>", 1)
$ConfigText = [regex]::Replace($ConfigText, '<Title>[^<]+</Title>', "<Title>LeoMiniGames $Version Setup</Title>", 1)
$ConfigText = $ConfigText.Replace('@LMG_UPDATE_REPOSITORY_URL@', $RepoUrl)
[System.IO.File]::WriteAllText($ConfigPath, $ConfigText, (New-Object System.Text.UTF8Encoding($false)))

$Utf8NoBom = New-Object System.Text.UTF8Encoding($false)
$TemplateReplacements = @{
  '__LMG_WINDOWS_ARCH__' = $WindowsArch
  '__LMG_WINDOWS_CPU_PROFILE__' = $CpuProfile
  '__LMG_WINDOWS_BUILD_LABEL__' = $BuildLabel
  '__LMG_UPDATE_REPOSITORY_URL__' = $RepoUrl
}
foreach ($TemplatePath in @($ControlScriptPath, $InstallScriptPath)) {
  if (-not (Test-Path $TemplatePath)) { throw "Missing installer script template: $TemplatePath" }
  $TemplateText = Get-Content $TemplatePath -Raw -Encoding UTF8
  foreach ($Key in $TemplateReplacements.Keys) { $TemplateText = $TemplateText.Replace($Key, $TemplateReplacements[$Key]) }
  if ($TemplateText -match '__LMG_[A-Z0-9_]+__') { throw "Unresolved installer placeholder in $TemplatePath" }
  [System.IO.File]::WriteAllText($TemplatePath, $TemplateText, $Utf8NoBom)
}

$PackageText = Get-Content $PackageXmlPath -Raw -Encoding UTF8
$PackageText = [regex]::Replace($PackageText, '<Version>[^<]+</Version>', "<Version>$Version</Version>", 1)
$PackageText = [regex]::Replace($PackageText, '<ReleaseDate>[^<]+</ReleaseDate>', "<ReleaseDate>$ReleaseDate</ReleaseDate>", 1)
[System.IO.File]::WriteAllText($PackageXmlPath, $PackageText, (New-Object System.Text.UTF8Encoding($false)))

Remove-Item $UpdateRepository -Recurse -Force -ErrorAction SilentlyContinue
New-Item (Split-Path $UpdateRepository -Parent) -ItemType Directory -Force | Out-Null
& $Repogen -p $PackagesDir $UpdateRepository
if ($LASTEXITCODE -ne 0 -or -not (Test-Path (Join-Path $UpdateRepository 'Updates.xml'))) {
  throw 'QtIFW update repository generation failed.'
}

$Setup = Join-Path $Dist "LeoMiniGames-v$Version-Windows-$Suffix-Setup.exe"
& $Creator --hybrid -c $ConfigPath -p $PackagesDir --include xyz.younglion.leominigames $Setup
if ($LASTEXITCODE -ne 0 -or -not (Test-Path $Setup)) { throw 'QtIFW hybrid installer build failed.' }

Remove-Item $IfwWork -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "Portable: $Portable"
Write-Host "Installer: $Setup"
Write-Host "Update repository: $UpdateRepository"
Write-Host "Remote repository URL: $RepoUrl"
