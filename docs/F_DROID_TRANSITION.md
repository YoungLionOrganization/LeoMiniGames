# F-Droid Transition

The official F-Droid main repository requires FLOSS applications.

`LicenseRef-LMG-SAPEL-1.0` is intentionally source-available and restricts
redistribution. Therefore a version governed by this license should not claim
eligibility for the official F-Droid main repository.

## Repository cleanup

After adopting the new license:

- remove or archive claims that LeoMiniGames is `GPL-3.0-or-later`;
- remove/disable official-F-Droid submission metadata and readiness claims;
- do not submit the source-available edition to fdroiddata;
- rename any "F-Droid build" feature whose purpose is actually "tracker-free"
  or "privacy build" so it does not falsely imply official F-Droid eligibility.

A separate YoungLion-controlled Android repository can still distribute
authorized LeoMiniGames builds if desired, subject to Android, signing, Qt/LGPL,
and third-party requirements.
