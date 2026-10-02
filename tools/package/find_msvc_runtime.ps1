# SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
function Find-LmgMsvcRuntime {
  param(
    [Parameter(Mandatory=$true)][string[]]$RedistRoots,
    [Parameter(Mandatory=$true)][ValidateSet('x64','arm64')][string]$Architecture
  )
  foreach ($RedistRoot in ($RedistRoots | Select-Object -Unique)) {
    $ArchDir = Join-Path $RedistRoot $Architecture
    if (-not (Test-Path $ArchDir)) { continue }
    $Crt = Get-ChildItem $ArchDir -Directory -Filter 'Microsoft.VC*.CRT' |
      Sort-Object Name -Descending | Select-Object -First 1
    if ($Crt) { return $Crt }
  }
  throw "MSVC runtime missing for $Architecture; searched: $($RedistRoots -join ', ')"
}
