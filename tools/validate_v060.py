#!/usr/bin/env python3
from pathlib import Path
import json
import re
import sys
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
errors = []

def require(path: str, needle: str | None = None):
    p = ROOT / path
    if not p.is_file():
        errors.append(f"missing: {path}")
        return
    if needle is not None and needle not in p.read_text(encoding="utf-8"):
        errors.append(f"{path}: missing token {needle!r}")

for path in [
    "LICENSE", "README.md", "CHANGELOG.md", "CONTRIBUTING.md", "SECURITY.md",
    "docs/BUILDING.md", "docs/PLUGIN_API.md", "docs/PLUGIN_FORMAT.md", "docs/F_DROID_TRANSITION.md",
    "docs/DEVELOPER_GUIDE.md", "docs/SAVE_SYSTEM.md", "docs/SETTINGS_SYSTEM.md",
    "docs/MIGRATION_COMPATIBILITY.md", "docs/TRUSTED_NATIVE.md", "VALIDATION_REPORT.md", "licenses/YOUNGLION_PACKAGE_LICENSE_1.0.txt",
    "schemas/plugin-manifest.schema.json", "src/core/GameRegistry.cpp", "src/core/GameSave.cpp",
    "src/core/GameSettings.cpp", "src/core/GameLifecycle.cpp", "src/core/GameInput.cpp",
    "src/core/GameViewport.cpp", "src/core/GameAudio.cpp", "src/core/GameI18n.cpp",
    "src/core/GameStats.cpp", "src/core/Achievements.cpp", "src/core/PluginDiagnostics.cpp",
    "mod-sdk/ExampleModernMod/manifest.json"
]:
    require(path)

require("CMakeLists.txt", "project(LeoMiniGames VERSION 0.7.0")
require("CMakeLists.txt", "LEOMINIGAMES_PRIVACY_BUILD")
require("src/main.cpp", 'setContextProperty(QStringLiteral("Games")')
require("src/main.cpp", 'setContextProperty(QStringLiteral("GameSave")')
require("src/main.cpp", 'setContextProperty(QStringLiteral("GameSettings")')
require("src/main.cpp", 'setContextProperty(QStringLiteral("Audio")')  # legacy API preserved
require("src/main.cpp", 'setContextProperty(QStringLiteral("Settings")')  # legacy API preserved
require("qml/LibraryPage.qml", "model: Games")
require("qml/LibraryPage.qml", "Games.query")
require("qml/ModsPage.qml", "ModsExplore")
require("qml/ModsPage.qml", "ModsInstalled")
require("src/core/PluginManager.cpp", "trusted-native.json")
require("src/core/PluginManager.cpp", "QCryptographicHash::Sha256")
require("src/core/ModManager.cpp", "Previous version restored")
require("src/core/ModManager.cpp", "publisher_verified")
require("src/core/GameSave.cpp", 'QByteArray kMagic("LMGSAVE\\0", 8)')
require("src/core/GameSave.cpp", "QSaveFile")
require("src/core/GameSave.cpp", "QCryptographicHash::Sha256")
require("src/sdk/GameLocalStats.h", "migrate v0.5.x legacy keys once")

# App-side source headers use the current host identifier, except explicitly licensed SDK files.
for base in (ROOT / "src", ROOT / "plugins/builtin", ROOT / "qml"):
    for p in base.rglob("*"):
        if p.suffix in {".cpp", ".h", ".qml"}:
            text = "\n".join(p.read_text(encoding="utf-8").splitlines()[:3])
            expected = (
                "SPDX-License-Identifier: LicenseRef-YoungLion-LMG-SDK-1.0"
                if p.relative_to(ROOT).as_posix() == "src/sdk/IGamePlugin.h"
                else "SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0"
            )
            if expected not in text:
                errors.append(f"missing SPDX header: {p.relative_to(ROOT)}")

# Built-ins must not continue using QSettings for scores/progress.
for p in (ROOT / "plugins/builtin").rglob("*Game.cpp"):
    text = p.read_text(encoding="utf-8")
    if "QSettings" in text:
        errors.append(f"built-in game still persists state with QSettings: {p.relative_to(ROOT)}")

# RCC v1 compatibility examples.
for rel in ("mod-sdk/ExampleHelloMod/manifest.json", "mod-sdk/ExampleModernMod/manifest.json"):
    try:
        manifest = json.loads((ROOT / rel).read_text(encoding="utf-8"))
    except Exception as exc:
        errors.append(f"invalid manifest {rel}: {exc}")
        continue
    if manifest.get("package_format", "rcc-v1") != "rcc-v1":
        errors.append(f"{rel}: package format changed")
    for key in ("id", "name", "version", "entry"):
        if key not in manifest:
            errors.append(f"{rel}: missing legacy field {key}")

# New schema cannot require new post-legacy fields.
try:
    schema = json.loads((ROOT / "schemas/plugin-manifest.schema.json").read_text(encoding="utf-8"))
    required = set(schema.get("required", []))
    if not required.issubset({"id", "name", "version", "entry"}):
        errors.append(f"manifest schema makes new fields mandatory: {sorted(required)}")
except Exception as exc:
    errors.append(f"invalid manifest schema: {exc}")

# QML brace smoke test and no self-import in app/private or mod examples.
for p in list((ROOT / "qml").glob("*.qml")) + list((ROOT / "mod-sdk").rglob("*.qml")):
    text = p.read_text(encoding="utf-8")
    if text.count("{") != text.count("}"):
        errors.append(f"QML brace mismatch: {p.relative_to(ROOT)}")
    if re.search(r"^\s*import\s+LeoMiniGames\b", text, re.MULTILINE):
        errors.append(f"private self import in plugin/QML: {p.relative_to(ROOT)}")

# JSON/XML parse smoke test.
for p in ROOT.rglob("*.json"):
    try:
        json.loads(p.read_text(encoding="utf-8"))
    except Exception as exc:
        errors.append(f"invalid JSON {p.relative_to(ROOT)}: {exc}")
try:
    ET.parse(ROOT / "android/AndroidManifest.xml")
except Exception as exc:
    errors.append(f"invalid AndroidManifest.xml: {exc}")

# RCC launch URL must not be short-circuited by a duplicate empty return.
mod_manager_text = (ROOT / "src/core/ModManager.cpp").read_text(encoding="utf-8")
if "return QUrl{};\n        return QUrl{};" in mod_manager_text:
    errors.append("ModManager entryUrl contains an unconditional duplicate empty return")

# Android release identity is generated by Qt/CMake in current source; the template keeps required permissions.
manifest_text = (ROOT / "android/AndroidManifest.xml").read_text(encoding="utf-8")
cmake_text = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
for token in ('QT_ANDROID_PACKAGE_NAME "xyz.younglion.leominigames"',
              'LEOMINIGAMES_ANDROID_VERSION_CODE 700',
              'QT_ANDROID_VERSION_NAME "${PROJECT_VERSION}"'):
    if token not in cmake_text:
        errors.append(f"CMake Android identity missing {token}")
if 'android.permission.VIBRATE' not in manifest_text:
    errors.append('Android manifest missing android.permission.VIBRATE')

if errors:
    print("LeoMiniGames v0.6.x compatibility validation FAILED")
    for error in errors:
        print("ERROR:", error)
    sys.exit(1)

print("LeoMiniGames v0.6.x compatibility validation OK")
