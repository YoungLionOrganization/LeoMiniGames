param(
  [Parameter(Mandatory=$true)][string]$Suffix,
  [Parameter(Mandatory=$true)][string]$InstallerSourceDir,
  [string]$DistDir = 'dist/windows',
  [string]$OutDir = 'windows-package-smoke'
)
$ErrorActionPreference = 'Stop'
if ($env:GITHUB_ACTIONS -ne 'true' -or !$env:RUNNER_TEMP) {
  throw 'Run installer servicing only on a disposable GitHub Windows runner.'
}
$Version = (Get-Content (Join-Path $PSScriptRoot '../../release/release.json') -Raw | ConvertFrom-Json).version
$Dist = (Resolve-Path -LiteralPath $DistDir).Path
$Output = (New-Item -ItemType Directory -Force -Path $OutDir).FullName
$Work = (New-Item -ItemType Directory -Path (Join-Path $env:RUNNER_TEMP ("lmg-servicing-" + [guid]::NewGuid()))).FullName
$Prefix = Join-Path $Work 'installed app'
$Zip = Join-Path $Dist "LeoMiniGames-v$Version-Windows-$Suffix.zip"
$Setup = Join-Path $Dist "LeoMiniGames-v$Version-Windows-$Suffix-Setup.exe"
$Repository = ([uri](Join-Path $Dist "update-repository/$Suffix")).AbsoluteUri
$Cache = Join-Path $Work 'repository-cache'

function Invoke-Checked([string]$Exe, [string[]]$Arguments, [string]$Name, [int[]]$AllowedExitCodes = @(0)) {
  $Info = [System.Diagnostics.ProcessStartInfo]::new()
  $Info.FileName = $Exe
  $Info.UseShellExecute = $false
  $Info.CreateNoWindow = $true
  $Info.RedirectStandardOutput = $true
  $Info.RedirectStandardError = $true
  if ([IO.Path]::GetFileName($Exe) -eq 'LeoMiniGames.exe') {
    $Info.Environment['QT_QPA_PLATFORM'] = 'windows'
  }
  foreach ($Argument in $Arguments) { $Info.ArgumentList.Add($Argument) }
  $Process = [System.Diagnostics.Process]::new()
  $Process.StartInfo = $Info
  if (!$Process.Start()) { throw "Could not start $Name" }
  $Stdout = $Process.StandardOutput.ReadToEndAsync()
  $Stderr = $Process.StandardError.ReadToEndAsync()
  $TimedOut = !$Process.WaitForExit(240000)
  if ($TimedOut) { $Process.Kill($true); $Process.WaitForExit() }
  $Text = $Stdout.GetAwaiter().GetResult() + $Stderr.GetAwaiter().GetResult()
  [IO.File]::WriteAllText((Join-Path $Output "$Name.log"), $Text)
  if ($TimedOut -or $Process.ExitCode -notin $AllowedExitCodes) {
    Write-Host $Text
    throw "$Name failed (timeout=$TimedOut, exit=$($Process.ExitCode))"
  }
  return $Text
}

# Exclude the development kit from child runtime discovery so missing deployment
# dependencies cannot be hidden by the runner's installed Qt.
$QtRoot = $env:QT_ROOT_DIR
if ($QtRoot) {
  $QtFull = [IO.Path]::GetFullPath($QtRoot).TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
  $env:PATH = (($env:PATH -split ';') | Where-Object {
    $_ -and !([IO.Path]::GetFullPath($_).StartsWith($QtFull, [StringComparison]::OrdinalIgnoreCase))
  }) -join ';'
}
foreach ($Variable in @('QT_PLUGIN_PATH','QT_QPA_PLATFORM_PLUGIN_PATH','QML2_IMPORT_PATH','QML_IMPORT_PATH')) {
  [Environment]::SetEnvironmentVariable($Variable, $null, 'Process')
}
[Environment]::SetEnvironmentVariable('QT_QPA_PLATFORM', $null, 'Process')
$env:QT_QUICK_BACKEND = 'software'
$env:QT_FORCE_STDERR_LOGGING = '1'
$env:APPDATA = Join-Path $Work 'user-data/roaming'
$env:LOCALAPPDATA = Join-Path $Work 'user-data/local'
$UserData = New-Item -ItemType Directory -Force -Path (Join-Path $env:APPDATA 'YoungLion/LeoMiniGames')
$Sentinel = Join-Path $UserData.FullName 'servicing-sentinel'
[IO.File]::WriteAllText($Sentinel, 'preserve-user-save')

Expand-Archive -LiteralPath $Zip -DestinationPath (Join-Path $Work 'portable')
$Portable = @(Get-ChildItem (Join-Path $Work 'portable') -Recurse -Filter LeoMiniGames.exe)
if ($Portable.Count -ne 1) { throw 'Expected one portable executable' }
$Text = Invoke-Checked $Portable[0].FullName @('--smoke-test') 'portable-launch'
if (!$Text.Contains('PASS: six builtin QML sessions opened and closed')) { throw 'Portable launch evidence missing' }

# The older fixture uses this candidate's binary with previous package metadata.
# It tests QtIFW upgrade operations; it is not a v0.7.2 gameplay/device claim.
$Fixture = Join-Path $Work 'older-fixture'
Copy-Item -LiteralPath $InstallerSourceDir -Destination $Fixture -Recurse
foreach ($Xml in @('config/config.xml','packages/xyz.younglion.leominigames/meta/package.xml')) {
  $Path = Join-Path $Fixture $Xml
  $Text = [IO.File]::ReadAllText($Path).Replace("<Version>$Version</Version>", '<Version>0.7.2</Version>')
  [IO.File]::WriteAllText($Path, $Text)
}
$Older = Join-Path $Work 'older-setup.exe'
$null = Invoke-Checked (Join-Path $env:QT_IFW_BIN 'binarycreator.exe') @('--offline-only','-c',"$Fixture/config/config.xml",'-p',"$Fixture/packages",$Older) 'fixture-create'
$Options = @('--default-answer','--accept-licenses','--confirm-command','--cache-path',$Cache,'--set-temp-repository',$Repository)
$null = Invoke-Checked $Older ($Options + @('--root',$Prefix,'install')) 'older-install'
$Maintenance = Join-Path $Prefix 'LeoMiniGamesMaintenance.exe'
if (!(Test-Path -LiteralPath $Maintenance)) { throw 'Maintenance Tool was not installed' }
# QtIFW returns EssentialUpdated (6) after successfully updating our essential
# app component. The next invocation restarts the tool and checks its version.
$null = Invoke-Checked $Maintenance ($Options + @('update')) 'candidate-upgrade' @(0,6)
$Text = Invoke-Checked $Maintenance @('list') 'installed-components'
if (!$Text.Contains($Version)) { throw 'Candidate version missing after upgrade' }
$Text = Invoke-Checked (Join-Path $Prefix 'LeoMiniGames.exe') @('--smoke-test') 'installed-launch'
if (!$Text.Contains('PASS: six builtin QML sessions opened and closed')) { throw 'Installed launch evidence missing' }

# Exercise add/remove through the shipped Maintenance Tool using a disposable
# optional component; the real application remains a forced component.
$Extra = Join-Path $Work 'extra/packages/lmg.qa.optional'
New-Item -ItemType Directory -Force -Path "$Extra/meta","$Extra/data" | Out-Null
[IO.File]::WriteAllText("$Extra/meta/package.xml", '<Package><DisplayName>QA optional fixture</DisplayName><Description>Disposable servicing fixture</Description><Version>1.0.0</Version><ReleaseDate>2026-10-30</ReleaseDate><Default>false</Default></Package>')
[IO.File]::WriteAllText("$Extra/data/qa-optional-marker", 'optional')
$ExtraRepo = Join-Path $Work 'extra/repository'
$null = Invoke-Checked (Join-Path $env:QT_IFW_BIN 'repogen.exe') @('-p',"$Work/extra/packages",$ExtraRepo) 'optional-repository'
$ExtraOptions = @('--default-answer','--accept-licenses','--confirm-command','--cache-path',"$Work/extra/cache",'--set-temp-repository',([uri]$ExtraRepo).AbsoluteUri)
$null = Invoke-Checked $Maintenance ($ExtraOptions + @('install','lmg.qa.optional')) 'modify-install'
if (!(Test-Path "$Prefix/qa-optional-marker")) { throw 'Optional component was not installed' }
$null = Invoke-Checked $Maintenance ($ExtraOptions + @('remove','lmg.qa.optional')) 'modify-remove'
if ((Test-Path "$Prefix/qa-optional-marker") -or !(Test-Path "$Prefix/LeoMiniGames.exe")) { throw 'Modify damaged component ownership' }
$null = Invoke-Checked $Maintenance @('--default-answer','--confirm-command','purge') 'uninstall'
for ($Attempt = 0; $Attempt -lt 30 -and (Test-Path $Maintenance); ++$Attempt) { Start-Sleep -Milliseconds 200 }
if ((Test-Path "$Prefix/LeoMiniGames.exe") -or (Test-Path $Maintenance)) { throw 'Uninstall left application/maintenance executable' }
if ([IO.File]::ReadAllText($Sentinel) -ne 'preserve-user-save') { throw 'Servicing changed user data' }

# Test the exact public installer as well as the versioned upgrade fixture.
$null = Invoke-Checked $Setup ($Options + @('--root',"$Work/fresh candidate",'install')) 'candidate-fresh-install'
$FreshMaintenance = "$Work/fresh candidate/LeoMiniGamesMaintenance.exe"
if (!(Test-Path $FreshMaintenance)) { throw 'Exact candidate Setup did not install' }
$Text = Invoke-Checked "$Work/fresh candidate/LeoMiniGames.exe" @('--smoke-test') 'candidate-fresh-launch'
if (!$Text.Contains('PASS: six builtin QML sessions opened and closed')) { throw 'Exact Setup payload launch failed' }
$null = Invoke-Checked $FreshMaintenance @('--default-answer','--confirm-command','purge') 'candidate-fresh-uninstall'
for ($Attempt = 0; $Attempt -lt 30 -and (Test-Path $FreshMaintenance); ++$Attempt) { Start-Sleep -Milliseconds 200 }
if ((Test-Path "$Work/fresh candidate/LeoMiniGames.exe") -or (Test-Path $FreshMaintenance)) { throw 'Fresh candidate uninstall left executables' }
if ([IO.File]::ReadAllText($Sentinel) -ne 'preserve-user-save') { throw 'Fresh candidate servicing changed user data' }

$Result = @{
  source_sha = $env:GITHUB_SHA
  version = $Version
  target = "Windows-$Suffix"
  environment = "GitHub $env:RUNNER_OS $env:RUNNER_ARCH; QtIFW CLI; previous-version metadata fixture"
  evidence = "https://github.com/$env:GITHUB_REPOSITORY/actions/runs/$env:GITHUB_RUN_ID"
  tested_at = [DateTime]::UtcNow.ToString('o')
  tester = 'GitHub Actions windows_package_smoke.ps1'
  portable_launch = $true
  fresh_install_launch_uninstall = $true
  installer_upgrade_modify_uninstall = $true
  user_data_preserved = $true
  tested_artifacts = @{}
}
foreach ($Artifact in @($Zip,$Setup)) { $Result.tested_artifacts[[IO.Path]::GetFileName($Artifact)] = (Get-FileHash -LiteralPath $Artifact -Algorithm SHA256).Hash.ToLowerInvariant() }
$Result | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $Output 'servicing.json') -Encoding utf8
Write-Host 'PASS: portable and Setup launch; QtIFW fixture upgrade, modify and uninstall; user data preserved'
