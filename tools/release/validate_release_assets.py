#!/usr/bin/env python3
from __future__ import annotations

import argparse
import fnmatch
import json
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("directory", type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    policy = json.loads((root / "release/assets.json").read_text(encoding="utf-8"))
    version = policy["version"]
    directory = args.directory.resolve()
    files = [p for p in directory.iterdir() if p.is_file()]
    if any(p.is_dir() for p in directory.iterdir()):
        raise SystemExit("release-assets must not contain directories")

    expected_names: set[str] = set()
    for entry in policy["assets"]:
        pattern = entry["pattern"].format(version=version)
        matches = [p for p in files if fnmatch.fnmatchcase(p.name, pattern)]
        minimum = int(entry.get("min_count", 1 if entry.get("required") else 0))
        maximum = int(entry.get("max_count", max(1, minimum)))
        if not minimum <= len(matches) <= maximum:
            raise SystemExit(f"{entry['id']}: expected {minimum}..{maximum}, got {len(matches)}")
        expected_names.update(p.name for p in matches)

    actual_names = {p.name for p in files}
    extras = sorted(actual_names - expected_names)
    if extras:
        raise SystemExit(f"unexpected public assets: {extras}")
    for name in actual_names:
        for pattern in policy.get("forbidden_public_patterns", []):
            if fnmatch.fnmatchcase(name, pattern):
                raise SystemExit(f"forbidden public asset: {name}")
    print(f"PASS: {len(files)} validated public release assets")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
