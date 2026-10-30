# SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
# Exercise process status handling without running or installing any package.
$ErrorActionPreference = 'Stop'
$Tokens = $null
$ParseErrors = $null
$Script = [System.Management.Automation.Language.Parser]::ParseFile(
  (Join-Path $PSScriptRoot '../tools/ci/windows_package_smoke.ps1'),
  [ref]$Tokens, [ref]$ParseErrors)
if ($ParseErrors) { throw ($ParseErrors | Out-String) }
$Function = $Script.Find({ param($Node)
  $Node -is [System.Management.Automation.Language.FunctionDefinitionAst] -and $Node.Name -eq 'Invoke-Checked'
}, $true)
if (!$Function) { throw 'Process helper was not found.' }
Invoke-Expression $Function.Extent.Text
$Output = (New-Item -ItemType Directory -Path (Join-Path ([IO.Path]::GetTempPath()) ('lmg-process-test-' + [guid]::NewGuid()))).FullName
$Shell = (Get-Process -Id $PID).Path
$null = Invoke-Checked $Shell @('-NoProfile','-NonInteractive','-Command','Write-Output fixture; exit 0') 'success'
$null = Invoke-Checked $Shell @('-NoProfile','-NonInteractive','-Command','Write-Output fixture; exit 6') 'essential-update' @(0,6)
foreach ($ExitCode in @(1,3,4,5,6,7)) {
  $Rejected = $false
  try {
    $null = Invoke-Checked $Shell @('-NoProfile','-NonInteractive','-Command',"exit $ExitCode") "rejected-$ExitCode"
  } catch {
    if ($_.Exception.Message -notmatch "exit=$ExitCode\)") { throw }
    $Rejected = $true
  }
  if (!$Rejected) { throw "Unexpected status accepted: $ExitCode" }
}
Write-Output 'PASS: success and explicit essential-update status accepted; failures remain rejected'
