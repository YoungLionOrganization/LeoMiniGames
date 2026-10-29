# LeoMiniGames installation guide

This guide covers v0.7.3 packages. Download published packages from [GitHub Releases](https://github.com/YoungLionOrganization/LeoMiniGames/releases). Actions artifacts are test candidates until publication. Running the application does not require Qt, CMake or a compiler.

## Choose a package

Full filenames begin with `LeoMiniGames-v0.7.3-`.

| System | Package selection |
| --- | --- |
| Windows, Intel/AMD 64-bit | `Windows-x86_64-Setup.exe` or portable `.zip`; separate `x86_64-AVX2` builds require AVX2 |
| Windows on ARM | `Windows-ARM64-Setup.exe` or matching `.zip` |
| Ubuntu 22.04/24.04 x86_64 | Matching `Linux-Ubuntu-22.04-x86_64` / `Linux-Ubuntu-24.04-x86_64` AppImage, tar.gz or Setup.run |
| Ubuntu 24.04 ARM64 | `Linux-Ubuntu-24.04-arm64` AppImage, tar.gz or Setup.run |
| Debian 13 | `Debian-13-x86_64.deb` or `Debian-13-arm64.deb` |
| Fedora 43 | `Fedora-43-x86_64.rpm` or `Fedora-43-arm64.rpm` |
| Arch Linux x86_64 | `Arch-x86_64.pkg.tar.zst`; Flatpak and compatible portable builds are alternatives |
| Linux with Flatpak | `Linux-x86_64.flatpak` or `Linux-aarch64.flatpak` |
| macOS | `macOS-arm64` for Apple Silicon, `macOS-x86_64` for Intel, or `macOS-universal` |
| Android | Usually `Android-arm64-v8a.apk`; choose the larger `Android-universal.apk` when unsure |

On Linux, `uname -m` reports `x86_64` for Intel/AMD or `aarch64` for ARM64. There is no official native Arch ARM package. Flatpak bundles are downloaded directly; no AUR or Flathub store listing is claimed.

## Verify the download

Compare the downloaded file's SHA-256 value with its entry in the release description's **SHA-256** section.

```sh
sha256sum LeoMiniGames-v0.7.3-Arch-x86_64.pkg.tar.zst
```

Windows PowerShell example:

```powershell
Get-FileHash .\LeoMiniGames-v0.7.3-Windows-x86_64-Setup.exe -Algorithm SHA256
```

## Windows

Run Setup and choose the installation location and shortcuts. Existing installations use their Maintenance Tool for update, modify/repair and uninstall. Its shortcut remains available after installation.

For a portable ZIP, extract the entire archive into a dedicated directory and run `LeoMiniGames.exe`. Keep the accompanying Qt libraries and runtime files with it. The baseline package does not require AVX2. No 32-bit Windows package is produced.

## Arch Linux

Install the native package on an up-to-date Arch system:

```sh
sudo pacman -Syu
sudo pacman -U ./LeoMiniGames-v0.7.3-Arch-x86_64.pkg.tar.zst
LeoMiniGames
```

Use the same `pacman -U` method for a newer package. Remove with `sudo pacman -R leominigames`; user saves remain separate.

## Flatpak on Arch and other distributions

On Arch, install Flatpak with `sudo pacman -S flatpak` if needed. Other distributions provide their own Flatpak package. The first installation downloads the required KDE runtime; the application bundle does not include that complete runtime.

```sh
flatpak remote-add --user --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
flatpak install --user ./LeoMiniGames-v0.7.3-Linux-x86_64.flatpak
flatpak run xyz.younglion.leominigames
```

On ARM64, choose the filename containing `aarch64`. To replace a local bundle, remove the application without deleting its data, then install the new bundle:

```sh
flatpak uninstall --user xyz.younglion.leominigames
flatpak install --user ./LeoMiniGames-v0.7.3-Linux-x86_64.flatpak
```

Do not use `--delete-data` for an ordinary upgrade: it explicitly deletes data. Flatpak stores data under `~/.var/app/xyz.younglion.leominigames/`. Native installation data is not migrated there automatically. Use the file portal for external file selection.

## Debian and Fedora

Debian 13 x86_64:

```sh
sudo apt install ./LeoMiniGames-v0.7.3-Debian-13-x86_64.deb
```

Fedora 43 x86_64:

```sh
sudo dnf install ./LeoMiniGames-v0.7.3-Fedora-43-x86_64.rpm
```

Choose the `arm64` file on ARM64 systems. Remove with `sudo apt remove leominigames` or `sudo dnf remove leominigames`, respectively. These native packages use distribution Qt libraries. For a different distribution/version, use Flatpak or a compatible portable build instead of forcing a mismatched native package.

## Linux user installer

Ubuntu 24.04 x86_64 example:

```sh
bash ./LeoMiniGames-v0.7.3-Linux-Ubuntu-24.04-x86_64-Setup.run
```

Setup runs without root and defaults to `~/.local/opt/LeoMiniGames`. Append `--prefix /absolute/path/LeoMiniGames` for another dedicated application directory, or `--no-integration` to skip the launcher/menu entry. Do not select a directory containing unrelated software.

For an upgrade, run the newer installer with the same prefix. Failed upgrades restore the previous payload and integration files. If restoration also fails, keep the reported recovery directory.

Uninstall the default installation with:

```sh
bash ~/.local/opt/LeoMiniGames/uninstall.sh --uninstall
```

For a custom prefix, use its own `uninstall.sh`.

## AppImage and portable tar.gz

```sh
chmod +x ./LeoMiniGames-v0.7.3-Linux-Ubuntu-24.04-x86_64.AppImage
./LeoMiniGames-v0.7.3-Linux-Ubuntu-24.04-x86_64.AppImage
```

If FUSE is unavailable, try `--appimage-extract-and-run` or use the portable tar.gz. Delete an AppImage to remove it; user data remains separate.

```sh
tar -xzf ./LeoMiniGames-v0.7.3-Linux-Ubuntu-24.04-x86_64.tar.gz
./LeoMiniGames.AppDir/AppRun
```

Keep the complete AppDir together. Files ending in `-native.tar.gz` are diagnostic archives linked to distribution Qt libraries, not general portable bundles.

## Android and macOS

On Android, open the APK with the device's package installer. Same-identity, same-key upgrades preserve data. If a signature mismatch occurs, preserve the existing installation and select the correctly signed package. The minimum version is Android 9 / API 28. AAB files are not directly installable.

On macOS, open the matching DMG and drag the app to Applications, or use the ZIP. Release notes describe signing/notarization status. Verify the source and checksum when assessing Gatekeeper warnings. Unsigned iOS/iPadOS ZIPs are developer test outputs requiring an appropriate signing environment. v0.7.3 does not contain a HarmonyOS HAP.

## Preserve saves and report problems

Close the application before backing up its complete user data directory. Native Linux defaults to `~/.local/share/YoungLion/LeoMiniGames`; Flatpak uses its sandbox. `XDG_DATA_HOME` changes the native Linux location. Changing package formats does not automatically transfer saves; v0.7.3 has no Save Transfer wizard.

After installation, open and close a builtin game, check that its state survives reopening, and check catalog/theme access. For missing sound, check the system output and application settings. A locked/unavailable keyring can prevent Developer Lab credential persistence; exclude credentials from reports.

Include OS/version, architecture, package filename, application version and reproduction steps in a [bug report](https://github.com/YoungLionOrganization/LeoMiniGames/issues/new/choose). Catalog HTTP 500 and local startup failures have different causes. The [technical Linux package guide](https://github.com/YoungLionOrganization/LeoMiniGames/blob/main/docs/LINUX_PACKAGES.md) describes build and servicing verification.
