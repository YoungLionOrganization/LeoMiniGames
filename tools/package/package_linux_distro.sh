#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="$(realpath "${1:?usage: package_linux_distro.sh BUILD_DIR deb|rpm|arch}")"
FORMAT="${2:?Specify deb, rpm or arch}"
VERSION="${LMG_VERSION:-0.7.3}"
SUFFIX="${LMG_PLATFORM_SUFFIX:?Set the target distro/architecture label}"
DIST="$ROOT/dist/linux-native"
mkdir -p "$DIST"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
DESTDIR="$WORK/stage" cmake --install "$BUILD_DIR" --prefix /usr --component Unspecified
[[ -x "$WORK/stage/usr/bin/LeoMiniGames" ]] || { echo 'Installed binary missing.' >&2; exit 2; }
case "$FORMAT" in
  deb)
    ARCH="$(dpkg --print-architecture)"
    mkdir -p "$WORK/stage/DEBIAN" "$WORK/debian"
    printf 'Source: leominigames\nSection: games\nPriority: optional\nMaintainer: YoungLion <contact@younglion.xyz>\n\nPackage: leominigames\nArchitecture: any\nDescription: Modular Qt mini-game platform\n' > "$WORK/debian/control"
    DEPS="$(cd "$WORK" && dpkg-shlibdeps -O -e stage/usr/bin/LeoMiniGames | sed -n 's/^shlibs:Depends=//p')"
    [[ -n "$DEPS" ]] || { echo 'Native dependency detection failed.' >&2; exit 3; }
    cat > "$WORK/stage/DEBIAN/control" <<EOF
Package: leominigames
Version: $VERSION-1
Section: games
Priority: optional
Architecture: $ARCH
Maintainer: YoungLion <contact@younglion.xyz>
Depends: $DEPS, qml6-module-qtquick, qml6-module-qtquick-window, qml6-module-qtquick-controls, qml6-module-qtquick-layouts, qml6-module-qtquick-templates, qml6-module-qtqml-workerscript, qml6-module-qtmultimedia, libsecret-1-0
Description: Modular Qt mini-game platform
 Six builtin games, downloadable QML games and themes, and Developer Lab.
EOF
    dpkg-deb --root-owner-group --build "$WORK/stage" "$DIST/LeoMiniGames-v${VERSION}-${SUFFIX}.deb"
    ;;
  rpm)
    mkdir -p "$WORK/rpm"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
    cat > "$WORK/rpm/SPECS/leominigames.spec" <<EOF
Name: leominigames
Version: $VERSION
Release: 1
Summary: Modular Qt mini-game platform
License: LicenseRef-LMG-SAPEL-1.0
URL: https://github.com/YoungLionOrganization/LeoMiniGames
Requires: qt6-qtbase, qt6-qtdeclarative, qt6-qtsvg, qt6-qtmultimedia, libsecret
%description
Six builtin games, downloadable QML games and themes, and Developer Lab.
%install
mkdir -p %{buildroot}
cp -a "$WORK/stage/." %{buildroot}/
%files
/usr/bin/LeoMiniGames
/usr/share/applications/xyz.younglion.leominigames.desktop
/usr/share/metainfo/xyz.younglion.leominigames.metainfo.xml
/usr/share/icons/hicolor/512x512/apps/xyz.younglion.leominigames.png
%doc /usr/share/doc/leominigames
EOF
    rpmbuild --define "_topdir $WORK/rpm" --define '_build_id_links none' \
      --define 'debug_package %{nil}' -bb "$WORK/rpm/SPECS/leominigames.spec"
    RPM="$(find "$WORK/rpm/RPMS" -type f -name '*.rpm' -print -quit)"
    [[ -n "$RPM" ]] || exit 3
    cp "$RPM" "$DIST/LeoMiniGames-v${VERSION}-${SUFFIX}.rpm"
    ;;
  arch)
    [[ "$(id -u)" != 0 ]] || { echo 'makepkg must run as an unprivileged user.' >&2; exit 2; }
    tar -C "$WORK/stage" -czf "$WORK/payload.tar.gz" .
    cp "$ROOT/packaging/arch/PKGBUILD" "$WORK/PKGBUILD"
    sed -i "s/'SKIP'/'$(sha256sum "$WORK/payload.tar.gz" | cut -d' ' -f1)'/" "$WORK/PKGBUILD"
    (cd "$WORK" && LMG_VERSION="$VERSION" makepkg --force --nodeps --noconfirm)
    ARCHIVE="$(find "$WORK" -maxdepth 1 -name '*.pkg.tar.zst' -print -quit)"
    [[ -n "$ARCHIVE" ]] || exit 3
    cp "$ARCHIVE" "$DIST/LeoMiniGames-v${VERSION}-${SUFFIX}.pkg.tar.zst"
    ;;
  *) echo "Unsupported format: $FORMAT" >&2; exit 2;;
esac
