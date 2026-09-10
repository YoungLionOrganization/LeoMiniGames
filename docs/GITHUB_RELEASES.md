# GitHub build artifacts and manual Releases

## CI

`ci.yml` runs on pushes and pull requests. GitHub CI uses Qt 6.10.2 because the
currently pinned `aqtinstall` path used by `install-qt-action` does not reliably
resolve Qt 6.11.x repository metadata. This does **not** lower the supported local
Qt version; Qt 6.11.1 remains a supported/recommended development kit and CMake's
minimum remains Qt 6.5.

## Build Release Artifacts

`build-artifacts.yml` is manual (`workflow_dispatch`) and intentionally does not
automatically create a GitHub Release. Open **Actions → Build Release Artifacts →
Run workflow**, enter the application version, and start it from the commit/branch
you want to package.

Expected downloadable files include:

- Source ZIP + SHA-256 sidecar
- Windows portable ZIP + QtIFW Setup EXE
- Linux portable tar.gz + AppImage
- macOS arm64/x86_64 ZIP + DMG (unsigned unless signing is added)
- Android arm64-v8a signed APK + signed AAB
- iOS unsigned simulator ZIP

Each single-file artifact uses GitHub's direct artifact upload mode, so an EXE,
APK, AAB, DMG, AppImage or prebuilt ZIP is not wrapped inside another ZIP.

## Android signing secrets

Never commit a `.jks`/`.keystore` file or passwords. Configure repository Actions
secrets with these exact names:

- `ANDROID_KEYSTORE_BASE64`
- `ANDROID_KEY_ALIAS`
- `ANDROID_KEYSTORE_PASSWORD`
- `ANDROID_KEY_PASSWORD`

On Windows PowerShell, convert your existing keystore to one-line Base64:

```powershell
$bytes = [IO.File]::ReadAllBytes("C:\path\to\your-upload-key.jks")
[Convert]::ToBase64String($bytes) | Set-Content -NoNewline .\keystore.base64.txt
```

Copy the entire contents of `keystore.base64.txt` into the
`ANDROID_KEYSTORE_BASE64` GitHub secret. The workflow decodes it only into the
runner's temporary directory, sets file permissions, supplies Qt's signing
environment variables, builds the signed APK/AAB and then the hosted runner is
discarded.

If this application has already been distributed outside Google Play, retain the
same app-signing key when the update mechanism requires signature continuity. If
Google Play App Signing is enabled, the CI key should normally be the registered
**upload key**, not Google's protected app-signing key.

## Publishing a Release manually

After all required artifact jobs succeed, download the files from the workflow
run, verify their names/checksums, create or edit the GitHub Release manually,
and attach the files. Do not mark unsigned macOS/iOS artifacts as notarized or
App-Store-ready.
