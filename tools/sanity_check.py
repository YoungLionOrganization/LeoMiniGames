from pathlib import Path
import json, sys

root = Path(__file__).resolve().parents[1]
errors = []

required = [
    "CMakeLists.txt", "src/main.cpp", "src/sdk/IGamePlugin.h",
    "qml/Main.qml", "qml/LibraryPage.qml", "qml/XoxPage.qml",
    "qml/BlackjackPage.qml", "qml/MinesweeperPage.qml",
    "qml/TwentyFortyEightPage.qml", "qml/MemoryMatchPage.qml",
    "qml/ReactionTapPage.qml", "src/core/AudioManager.cpp"
]

for item in required:
    if not (root / item).exists():
        errors.append(f"missing: {item}")

for meta in root.glob("plugins/builtin/*/metadata.json"):
    try:
        data = json.loads(meta.read_text(encoding="utf-8"))
        for key in ("id", "name", "version", "apiVersion"):
            if key not in data:
                errors.append(f"{meta}: missing {key}")
    except Exception as exc:
        errors.append(f"{meta}: {exc}")

main = (root / "src/main.cpp").read_text(encoding="utf-8")
for plugin_class in (
    "XoxPlugin", "BlackjackPlugin", "MinesweeperPlugin",
    "TwentyFortyEightPlugin", "MemoryMatchPlugin", "ReactionTapPlugin"
):
    if f"Q_IMPORT_PLUGIN({plugin_class})" not in main:
        errors.append(f"missing static import: {plugin_class}")

pm = (root / "src/core/PluginManager.cpp").read_text(encoding="utf-8")
if "staticInstances()" not in pm:
    errors.append("static plugin discovery missing")

for qml in root.glob("qml/*.qml"):
    text = qml.read_text(encoding="utf-8")
    if text.count("{") != text.count("}"):
        errors.append(f"{qml}: brace mismatch")

print("LeoMiniGames sanity check")
if errors:
    print("\n".join("ERROR: " + error for error in errors))
    sys.exit(1)
print("OK")
