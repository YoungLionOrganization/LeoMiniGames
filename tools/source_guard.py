#!/usr/bin/env python3
from pathlib import Path
import json
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
errors = []

for path in list(ROOT.rglob("*.cpp")) + list(ROOT.rglob("*.h")):
    text = path.read_text(encoding="utf-8")
    for lineno, line in enumerate(text.splitlines(), 1):
        if re.search(r"=\s*\{\s*\}\s*;", line):
            errors.append(f"{path.relative_to(ROOT)}:{lineno}: bare braced assignment is forbidden")
        if re.search(r"\breturn\s*\{\s*\}\s*;", line):
            errors.append(f"{path.relative_to(ROOT)}:{lineno}: bare braced return is forbidden")

# QML object declarations are separated by whitespace/newlines, not semicolons.
# Catch the exact class of error that qmlcachegen reports as "Unexpected token `;`".
qml_object_separator = re.compile(r"}\s*;\s*[A-Z][A-Za-z0-9_]*\s*{")
for path in ROOT.rglob("*.qml"):
    text = path.read_text(encoding="utf-8")
    for match in qml_object_separator.finditer(text):
        lineno = text.count("\n", 0, match.start()) + 1
        errors.append(
            f"{path.relative_to(ROOT)}:{lineno}: semicolon between QML object declarations is forbidden"
        )


# Build-time regressions previously caught only by a real Qt 6.11 build.
cmake_text = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
if "qt_add_executable(LeoMiniGames" in cmake_text and "qt_finalize_executable(LeoMiniGames)" in cmake_text and "MANUAL_FINALIZATION" not in cmake_text:
    errors.append("CMakeLists.txt: explicit qt_finalize_executable would double-finalize an automatically finalized target")

external_runtime = (ROOT / "src/core/ExternalGameRuntime.cpp").read_text(encoding="utf-8")
if "#include <QNetworkAccessManagerFactory>" in external_runtime:
    errors.append("src/core/ExternalGameRuntime.cpp: use <QQmlNetworkAccessManagerFactory>, not nonexistent <QNetworkAccessManagerFactory>")

# Catch QObject classes that declare an out-of-line constructor but forget its definition.
for header in (ROOT / "src/core").glob("*.h"):
    htext = header.read_text(encoding="utf-8")
    class_match = re.search(r"class\s+(\w+)(?:\s+final)?\s*:\s*public\s+QObject", htext)
    if not class_match:
        continue
    class_name = class_match.group(1)
    ctor_decl = re.search(rf"\b(?:explicit\s+)?{re.escape(class_name)}\s*\([^{{;]*\)\s*;", htext)
    if not ctor_decl:
        continue
    source = header.with_suffix(".cpp")
    if not source.exists():
        errors.append(f"{header.relative_to(ROOT)}: out-of-line constructor declared but {source.name} is missing")
        continue
    ctext = source.read_text(encoding="utf-8")
    if not re.search(rf"\b{re.escape(class_name)}::{re.escape(class_name)}\s*\(", ctext):
        errors.append(f"{source.relative_to(ROOT)}: constructor for {class_name} is declared in the header but not defined")

game_audio = (ROOT / "src/core/GameAudio.cpp").read_text(encoding="utf-8")
if "releasePrefix(QUrl(" in game_audio:
    errors.append("src/core/GameAudio.cpp: AudioManager::releasePrefix expects QString, not QUrl")

dev_lab = (ROOT / "qml/DeveloperLabPage.qml").read_text(encoding="utf-8")
if "pragma ComponentBehavior: Bound" not in dev_lab:
    errors.append("qml/DeveloperLabPage.qml: nested components must use pragma ComponentBehavior: Bound")
if re.search(r"(?<![A-Za-z0-9_.])Developer\.", dev_lab) or re.search(r"(?<![A-Za-z0-9_.])GameLogger\.", dev_lab):
    errors.append("qml/DeveloperLabPage.qml: direct unqualified Developer/GameLogger context access is forbidden; use required root properties")

# Publisher trust regression: canonical legacy admin catalogs must retain Official
# provenance, while third-party and modern explicit-unverified rows stay fail-closed.
resolver_path = ROOT / "src/core/PublisherTrustResolver.cpp"
if not resolver_path.is_file():
    errors.append("src/core/PublisherTrustResolver.cpp: missing centralized publisher trust resolver")
else:
    resolver = resolver_path.read_text(encoding="utf-8")
    for needle in ("authoritativeCatalog", "legacyCatalogIsAdminOnly", "legacyAdminFallback", "publisher_type"):
        if needle not in resolver:
            errors.append(f"src/core/PublisherTrustResolver.cpp: missing trust invariant {needle}")
for rel in ("src/core/ModManager.cpp", "src/core/ThemeCatalogManager.cpp"):
    manager_text = (ROOT / rel).read_text(encoding="utf-8")
    if "resolvePublisherTrust(object, authoritativeCatalog, true)" not in manager_text:
        errors.append(f"{rel}: catalog publisher trust must use centralized legacy-compatible resolver")

for path in ROOT.rglob("*.json"):
    try:
        json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:
        errors.append(f"{path.relative_to(ROOT)}: invalid JSON: {exc}")

plugins = (
    "XoxPlugin",
    "BlackjackPlugin",
    "MinesweeperPlugin",
    "TwentyFortyEightPlugin",
    "MemoryMatchPlugin",
    "ReactionTapPlugin",
)
main = (ROOT / "src/main.cpp").read_text(encoding="utf-8")
for name in plugins:
    if f"Q_IMPORT_PLUGIN({name})" not in main:
        errors.append(f"src/main.cpp: missing Q_IMPORT_PLUGIN({name})")

if errors:
    print("Source guard FAILED")
    print("\n".join(errors))
    sys.exit(1)
print("Source guard OK")
