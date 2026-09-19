param([string]$RepoPath = ".")
$ErrorActionPreference = "Stop"
$Source = Split-Path -Parent $MyInvocation.MyCommand.Path
$Repo = (Resolve-Path $RepoPath).Path
if (-not (Test-Path (Join-Path $Repo ".git"))) { throw "Target is not a Git repository: $Repo" }
Write-Host "Applying LeoMiniGames source/licensing/workflow repair to $Repo"
$exclude = @('.git','build','dist','.qt','.idea','.vs')
Get-ChildItem -LiteralPath $Source -Force | Where-Object { $exclude -notcontains $_.Name -and $_.Name -ne 'APPLY_TO_EXISTING_REPO.ps1' -and $_.Name -ne 'APPLY_TO_EXISTING_REPO.sh' } | ForEach-Object {
  $dst = Join-Path $Repo $_.Name
  if ($_.PSIsContainer) { Copy-Item $_.FullName $dst -Recurse -Force }
  else { Copy-Item $_.FullName $dst -Force }
}
Push-Location $Repo
try {
  python tools/validate_distribution.py
  python tools/validate_v070.py
  Write-Host "Repair applied. Review 'git status', commit, then push."
} finally { Pop-Location }
