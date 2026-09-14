from pathlib import Path
import json, re, sys, xml.etree.ElementTree as ET

root = Path(__file__).resolve().parents[1]
errors = []

required = [
    "CMakeLists.txt", "qml/CMakeLists.txt", "qml/Main.qml", "qml/Theme.qml",
    "src/main.cpp", "src/sdk/IGamePlugin.h", "src/core/PluginManager.cpp",
    "src/core/AudioManager.cpp", "resources/resources.qrc"
]
for rel in required:
    if not (root / rel).is_file():
        errors.append(f"Missing: {rel}")

ids = set()
for meta in root.glob("plugins/builtin/*/metadata.json"):
    try:
        data = json.loads(meta.read_text(encoding="utf-8"))
    except Exception as exc:
        errors.append(f"Invalid JSON {meta.relative_to(root)}: {exc}")
        continue
    pid = data.get("id", "")
    if not re.fullmatch(r"[a-z0-9][a-z0-9_.-]{1,63}", pid):
        errors.append(f"Invalid plugin id: {pid}")
    if pid in ids:
        errors.append(f"Duplicate plugin id: {pid}")
    ids.add(pid)
    if data.get("apiVersion") != 1:
        errors.append(f"Unexpected API version for {pid}")

qml_cmake = (root / "qml/CMakeLists.txt").read_text(encoding="utf-8")
for qml in (root / "qml").glob("*.qml"):
    if qml.name not in qml_cmake:
        errors.append(f"QML file not listed in qml/CMakeLists.txt: {qml.name}")

try:
    tree = ET.parse(root / "resources/resources.qrc")
    for elem in tree.findall(".//file"):
        rel = (elem.text or "").strip()
        if rel and not (root / "resources" / rel).is_file():
            errors.append(f"QRC target missing: resources/{rel}")
except Exception as exc:
    errors.append(f"Invalid resources.qrc: {exc}")

for lang in ("az", "tr"):
    path = root / f"resources/i18n/{lang}.json"
    try:
        obj = json.loads(path.read_text(encoding="utf-8"))
        if not isinstance(obj, dict):
            errors.append(f"{lang}.json is not an object")
    except Exception as exc:
        errors.append(f"Invalid translation JSON {lang}: {exc}")

main_cpp = (root / "src/main.cpp").read_text(encoding="utf-8")
top_cmake = (root / "CMakeLists.txt").read_text(encoding="utf-8")
plugins = (
    "XoxPlugin", "BlackjackPlugin", "MinesweeperPlugin",
    "TwentyFortyEightPlugin", "MemoryMatchPlugin", "ReactionTapPlugin"
)
for cls in plugins:
    if f"Q_IMPORT_PLUGIN({cls})" not in main_cpp:
        errors.append(f"Missing Q_IMPORT_PLUGIN for {cls}")
    if cls not in top_cmake:
        errors.append(f"Missing plugin target link/reference: {cls}")

manifest = (root / "android/AndroidManifest.xml").read_text(encoding="utf-8")
for token in ('%%INSERT_VERSION_NAME%%', '%%INSERT_VERSION_CODE%%', '%%INSERT_APP_NAME%%', 'android:appCategory="game"', 'android.intent.category.GAME'):
    if token not in manifest:
        errors.append(f"Android manifest missing expected token: {token}")
for token in ('QT_ANDROID_PACKAGE_NAME "xyz.younglion.leominigames"', 'QT_ANDROID_VERSION_NAME "${PROJECT_VERSION}"', 'set(LEOMINIGAMES_ANDROID_VERSION_CODE 700)'):
    if token not in top_cmake:
        errors.append(f"CMake Android metadata missing expected token: {token}")

for qml in root.rglob("*.qml"):
    text = qml.read_text(encoding="utf-8")
    if re.search(r"^\s*import\s+LeoMiniGames\b", text, flags=re.MULTILINE):
        errors.append(f"Deprecated self QML import remains: {qml.relative_to(root)}")

if 'QT_ANDROID_VERSION_CODE ${LEOMINIGAMES_ANDROID_VERSION_CODE}' not in top_cmake:
    errors.append("CMake Android version code is not wired to the release constant")

if errors:
    print("\n".join(errors))
    sys.exit(1)

print("Project validation OK")
print("Built-in plugins:", ", ".join(sorted(ids)))
print("QML files:", len(list((root / "qml").glob("*.qml"))))
