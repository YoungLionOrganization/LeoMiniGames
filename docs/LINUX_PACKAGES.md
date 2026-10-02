# Linux package choices

| Format | Targets | Install / remove |
| --- | --- | --- |
| AppImage | Ubuntu 22.04/24.04 x86_64, Ubuntu 24.04 ARM64; compatible newer distros | `chmod +x *.AppImage`, launch; delete the file to remove. |
| Portable tar.gz | Same as AppImage | Extract, launch `LeoMiniGames.AppDir/AppRun`. |
| Setup.run | Same as AppImage | `bash <package>-Setup.run`; `bash ~/.local/opt/LeoMiniGames/uninstall.sh --uninstall`. |
| DEB | Debian 13 x86_64/ARM64 | `sudo apt install ./<package>.deb`; `sudo apt remove leominigames`. |
| RPM | Fedora 43 x86_64/ARM64 | `sudo dnf install ./<package>.rpm`; `sudo dnf remove leominigames`. |
| Arch package | Arch Linux x86_64 | `sudo pacman -U ./<package>.pkg.tar.zst`; `sudo pacman -R leominigames`. |
| Flatpak bundle | x86_64/aarch64, including Arch | `flatpak install --user ./<package>.flatpak`; `flatpak run xyz.younglion.leominigames`; `flatpak uninstall --user xyz.younglion.leominigames`. |
| Native tar.gz | Debian 13, Fedora 43, Arch | Diagnostic archive linked to distro Qt packages. |

Download the candidate/release package for your architecture. ARM64/aarch64 and x86_64 are separate builds. Flatpak uses the KDE 6.10 runtime from Flathub; first run `flatpak remote-add --user --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo`. This repository builds official single-file bundles; no Flathub listing or AUR submission has been made. Arch supports its native package, Flatpak and compatible portable builds.

The user installer needs Bash, tar and coreutils, and runs without root. The default prefix is `~/.local/opt/LeoMiniGames`. `--prefix /absolute/path` chooses another dedicated directory; use that same prefix to upgrade or uninstall. `--no-integration` skips the launcher/menu entry. Reinstall replaces a managed payload and preserves user saves/settings; unrelated directories and symlinks are rejected. Use native package managers for system installation.

Flatpak data lives under `~/.var/app/xyz.younglion.leominigames`. The sandbox grants network, graphics/audio and keyring access, and uses file portals instead of granting the entire home directory. Native L3 plugins must match the runtime ABI. Native and Flatpak builds have separate data directories. Uninstallation normally keeps user data; `flatpak uninstall --delete-data` explicitly deletes it.

Updates use the same format/package manager. In-app update discovery opens the backend download page. QtIFW Maintenance Tool servicing remains Windows-only.

## Build and verification

After [building](BUILDING.md), use `LMG_PLATFORM_SUFFIX=Debian-13-x86_64 bash tools/package/package_linux_distro.sh build deb`. Use `rpm` on Fedora or `arch` as an unprivileged user on Arch. `package_linux_native.sh` emits diagnostic tarballs; `package_linux.sh` deploys Qt, produces AppImage/tar.gz and a `.run` installer. `bash tools/package/package_flatpak.sh` builds a bundle against the declared KDE SDK.

`python3 tests/test_linux_installer.py` tests fresh install, upgrade, launch, uninstall, data retention, corrupt payloads and unrelated targets. Set `LMG_TEST_APPDIR="$PWD/dist/linux/LeoMiniGames.AppDir"` to exercise the actual deployed application.

Reusable native/Flatpak workflows run in CI and Build Release Artifacts. Native packages are installed, launched with Xvfb/xcb, reinstalled and removed. Flatpak is built, installed, launched inside its sandbox, reinstalled and removed. Portable artifact jobs test the installer with the deployed AppDir. The smoke test opens/closes all six builtin games and rejects QML/lifecycle failures. Physical audio, interactive keyrings/desktops and older distributions still require release QA.
