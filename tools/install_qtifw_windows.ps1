param(
  [Parameter(Mandatory=$false)]
  [ValidateSet('x64','arm64')]
  [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'

$RepositoryArch = if ($Architecture -eq 'arm64') { 'windows_arm64' } else { 'windows_x86' }
$BaseUrl = "https://download.qt.io/online/qtsdkrepository/$RepositoryArch/ifw/tools_ifw_411/qt.tools.ifw.411/"
$InstallRoot = Join-Path $env:RUNNER_TEMP "QtIFW-$Architecture"
$ArchivePath = Join-Path $env:RUNNER_TEMP "qtifw-$Architecture.7z"
$ShaPath = "$ArchivePath.sha1"

Write-Host "Discovering Qt Installer Framework 4.11 package for $RepositoryArch..."
$Index = (Invoke-WebRequest -Uri $BaseUrl -UseBasicParsing).Content
$Candidates = [regex]::Matches($Index, 'href="([^"]+\.7z)"') |
  ForEach-Object { $_.Groups[1].Value } |
  Where-Object { $_ -match 'ifw-win-.*\.7z$' -and $_ -notmatch 'meta\.7z$' }

$ArchiveName = $Candidates | Select-Object -First 1
if (-not $ArchiveName) {
  throw "QtIFW 4.11 archive was not found in $BaseUrl"
}

Write-Host "Downloading $ArchiveName"
Invoke-WebRequest -Uri ($BaseUrl + $ArchiveName) -OutFile $ArchivePath -UseBasicParsing
Invoke-WebRequest -Uri ($BaseUrl + $ArchiveName + '.sha1') -OutFile $ShaPath -UseBasicParsing

$ExpectedSha1 = ((Get-Content $ShaPath -Raw).Trim() -split '\s+')[0].ToLowerInvariant()
$ActualSha1 = (Get-FileHash -Path $ArchivePath -Algorithm SHA1).Hash.ToLowerInvariant()
if ($ExpectedSha1 -ne $ActualSha1) {
  throw "QtIFW SHA1 mismatch. Expected $ExpectedSha1, got $ActualSha1"
}

Remove-Item $InstallRoot -Recurse -Force -ErrorAction SilentlyContinue
New-Item $InstallRoot -ItemType Directory -Force | Out-Null

$SevenZip = (Get-Command 7z.exe -ErrorAction SilentlyContinue).Source
if (-not $SevenZip) {
  $SevenZip = (Get-Command 7z -ErrorAction Stop).Source
}

& $SevenZip x $ArchivePath "-o$InstallRoot" -y | Out-Host
if ($LASTEXITCODE -ne 0) {
  throw "7-Zip failed to extract QtIFW archive: $LASTEXITCODE"
}

$Creator = Get-ChildItem $InstallRoot -Filter binarycreator.exe -Recurse | Select-Object -First 1
if (-not $Creator) {
  throw "binarycreator.exe not found after extracting $ArchiveName"
}

"QT_IFW_BIN=$($Creator.DirectoryName)" | Out-File $env:GITHUB_ENV -Encoding utf8 -Append
Write-Host "QtIFW binarycreator: $($Creator.FullName)"
