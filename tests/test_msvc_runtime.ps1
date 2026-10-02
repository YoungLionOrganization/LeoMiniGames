# SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '../tools/package/find_msvc_runtime.ps1')
$Fixture = Join-Path ([IO.Path]::GetTempPath()) ('lmg-redist-' + [guid]::NewGuid())
try {
  $NewRoot = Join-Path $Fixture 'v145'
  $OldRoot = Join-Path $Fixture 'v143'
  $X64 = Join-Path $NewRoot 'x64/Microsoft.VC145.CRT'
  $Arm64 = Join-Path $OldRoot 'arm64/Microsoft.VC143.CRT'
  New-Item $X64,$Arm64 -ItemType Directory -Force | Out-Null
  $Result = Find-LmgMsvcRuntime -RedistRoots @($NewRoot,$OldRoot) -Architecture arm64
  if ($Result.FullName -ne (Get-Item $Arm64).FullName) { throw 'ARM64 fallback did not select v143.' }
  $Result = Find-LmgMsvcRuntime -RedistRoots @($NewRoot,$OldRoot) -Architecture x64
  if ($Result.FullName -ne (Get-Item $X64).FullName) { throw 'x64 did not select the current runtime.' }
  $Rejected = $false
  try { Find-LmgMsvcRuntime -RedistRoots @($NewRoot) -Architecture arm64 | Out-Null }
  catch { $Rejected = $true }
  if (-not $Rejected) { throw 'Missing ARM64 runtime was silently accepted.' }
  Write-Host 'PASS: 3 MSVC runtime discovery regressions'
} finally {
  Remove-Item $Fixture -Recurse -Force -ErrorAction SilentlyContinue
}
