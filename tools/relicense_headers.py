#!/usr/bin/env python3
"""
LeoMiniGames license-header migration helper.

DRY-RUN by default.
Use --apply only after reviewing all reported files.

This script intentionally does NOT modify:
- LICENSE / legal text files
- .git
- third-party/vendor directories
- designated SDK/template paths unless explicitly mapped below
"""

from pathlib import Path
import argparse

ROOT = Path(__file__).resolve().parents[1]

HOST_OLD = "SPDX-License-Identifier: GPL-3.0-or-later"
HOST_NEW = "SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0"
SDK_NEW = "SPDX-License-Identifier: LicenseRef-YoungLion-LMG-SDK-1.0"

SKIP_PARTS = {
    ".git", "build", "build-fdroid", "build-privacy", "deployment",
}
SKIP_PREFIXES = (
    "licenses/",
    "docs/",
)
SKIP_EXACT = {
    "LICENSE", "NOTICE", "COPYRIGHT", "LICENSING.md", "LICENSE_HISTORY.md",
    "LICENSE_METADATA.json", "MIGRATION_FROM_GPL.md", "LEGAL_CHANGELOG.md",
    "CONTRIBUTOR_LICENSE_AGREEMENT.md", "CONTRIBUTING.md",
    "tools/relicense_headers.py",
}

# Explicit SDK/template files that may be moved to the special SDK license
# only if Project Owner copyright is confirmed.
SDK_EXACT = {
    "src/sdk/IGamePlugin.h",
}

TEXT_SUFFIXES = {
    ".h", ".hpp", ".cpp", ".cxx", ".cc", ".qml", ".py", ".sh", ".bat",
    ".cmake", ".txt", ".md", ".json", ".yml", ".yaml"
}

def classify(path: Path):
    rel = path.relative_to(ROOT).as_posix()
    if any(part in SKIP_PARTS for part in path.parts):
        return None
    if rel.startswith(SKIP_PREFIXES) or rel in SKIP_EXACT:
        return None
    if path.suffix.lower() not in TEXT_SUFFIXES and path.name != "CMakeLists.txt":
        return None
    if rel in SDK_EXACT:
        return "sdk"
    return "host"

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--apply", action="store_true")
    args = ap.parse_args()

    changed = []
    for path in ROOT.rglob("*"):
        if not path.is_file():
            continue
        kind = classify(path)
        if not kind:
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        lines = text.splitlines(keepends=True)
        header_indexes = [i for i in range(min(3, len(lines))) if HOST_OLD in lines[i]]
        if not header_indexes:
            continue

        replacement = SDK_NEW if kind == "sdk" else HOST_NEW
        for i in header_indexes:
            lines[i] = lines[i].replace(HOST_OLD, replacement)
        new = "".join(lines)

        rel = path.relative_to(ROOT).as_posix()
        print(f"{'[APPLY]' if args.apply else '[DRY]'} {rel}: {HOST_OLD} -> {replacement}")
        changed.append(rel)

        if args.apply:
            path.write_text(new, encoding="utf-8", newline="\n")

    print(f"\nFiles matched: {len(changed)}")
    if not args.apply:
        print("No files were changed. Re-run with --apply after legal/source review.")

if __name__ == "__main__":
    main()
