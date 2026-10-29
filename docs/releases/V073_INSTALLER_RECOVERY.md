# v0.7.3 Linux installer failure recovery

17 October 2026. Follows [runtime fixes](V073_RUNTIME_FOLLOWUPS.md) and [session/save integrity fixes](V073_SESSION_INTEGRITY.md).

The user Setup.run installer previously retired the installed payload before writing its launcher and desktop entry. A failed integration write then triggered unconditional temporary-directory cleanup, deleting the previous installation. A failed rollback after payload promotion had the same problem.

Installation now commits only after payload replacement and integration records succeed. Before replacement it stages the new integration content and snapshots existing launcher/menu files. A failure restores both the previous payload and integration files, including a partially written destination. An incomplete fresh installation is removed. If restoration itself fails, the recovery directory is retained and its path is reported instead of deleting the last working payload. Upgrades without integration preserve the uninstall records of an existing installation.

Six added installer regressions cover fresh-install failure, partial launcher/menu writes during upgrade, failed payload promotion, failed rollback retaining recovery files, interruption during payload promotion, and an upgrade without integration followed by uninstall. They run alongside the existing five installer lifecycle regressions in Linux CI and against packaged Setup.run artifacts. INT/TERM interruptions trigger rollback; state is recorded before filesystem moves so interruption immediately after a move cannot delete the recovery payload.

Local fault injection under MSYS Bash on Windows reproduced the old installer deleting the previous payload after a launcher-write failure. The corrected fixture restored that payload; a second fixture verified restoration after a partial desktop-entry write. This checks shell transaction behavior; native Linux CI remains the installation/runtime verification authority. Current commit results are tracked in PR #7.

The production catalog remains a separate blocker: the 15 October CI/artifact runs completed all non-Intel-Android targets, while each Intel Android emulator TLS probe received HTTP 500 from the catalog. Direct catalog requests still returned 500 on 17 October, with transport health responding 200. The check is retained, and no production backend deployment is included.
