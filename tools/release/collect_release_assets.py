#!/usr/bin/env python3
from __future__ import annotations

import argparse
import fnmatch
import json
import shutil
from pathlib import Path


def load_policy(root: Path) -> dict:
    policy = json.loads((root / "release/assets.json").read_text(encoding="utf-8"))
    if policy.get("schema") != 1:
        raise SystemExit("unsupported release/assets.json schema")
    return policy


def collect_public(root: Path, artifact_root: Path, output: Path) -> list[str]:
    policy = load_policy(root)
    version = policy["version"]
    files = [p for p in artifact_root.rglob("*") if p.is_file()]
    selected: dict[str, Path] = {}

    for entry in policy["assets"]:
        pattern = entry["pattern"].format(version=version)
        matches = sorted(p for p in files if fnmatch.fnmatchcase(p.name, pattern))
        min_count = int(entry.get("min_count", 1 if entry.get("required", False) else 0))
        max_count = int(entry.get("max_count", max(min_count, 1)))
        if len(matches) < min_count or len(matches) > max_count:
            raise SystemExit(
                f"asset policy mismatch for {entry['id']}: pattern={pattern!r}, "
                f"expected {min_count}..{max_count}, found {len(matches)}"
            )
        for source in matches:
            existing = selected.get(source.name)
            if existing is not None:
                if existing.read_bytes() != source.read_bytes():
                    raise SystemExit(f"conflicting duplicate release filename: {source.name}")
                continue
            selected[source.name] = source

    if output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True)
    for name, source in sorted(selected.items()):
        shutil.copy2(source, output / name)

    forbidden = policy.get("forbidden_public_patterns", [])
    for path in output.iterdir():
        if not path.is_file():
            raise SystemExit(f"release-assets root must contain files only: {path.name}")
        for pattern in forbidden:
            if fnmatch.fnmatchcase(path.name, pattern):
                raise SystemExit(f"forbidden public release asset: {path.name} ({pattern})")

    if not selected:
        raise SystemExit("no public release assets were collected")
    return sorted(selected)


def collect_update_repositories(artifact_root: Path, output: Path) -> list[str]:
    if output.exists():
        shutil.rmtree(output)
    copied: list[str] = []
    seen: set[str] = set()
    for updates_xml in artifact_root.rglob("Updates.xml"):
        repo_dir = updates_xml.parent
        try:
            marker_index = list(repo_dir.parts).index("update-repository")
        except ValueError:
            continue
        suffix_parts = repo_dir.parts[marker_index + 1 :]
        if len(suffix_parts) != 1:
            continue
        suffix = suffix_parts[0]
        if suffix in seen:
            raise SystemExit(f"duplicate QtIFW update repository for {suffix}")
        seen.add(suffix)
        target = output / "windows" / suffix
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(repo_dir, target)
        copied.append(f"windows/{suffix}")

    expected = {"windows/x86_64", "windows/ARM64"}
    missing = expected.difference(copied)
    if missing:
        raise SystemExit(f"missing QtIFW update repositories: {sorted(missing)}")
    return sorted(copied)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--artifact-root", required=True, type=Path)
    parser.add_argument("--public-output", required=True, type=Path)
    parser.add_argument("--update-output", required=True, type=Path)
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[2]
    public = collect_public(root, args.artifact_root.resolve(), args.public_output.resolve())
    update_repos = collect_update_repositories(args.artifact_root.resolve(), args.update_output.resolve())
    print("Public release assets:")
    for name in public:
        print(f"  {name}")
    print("QtIFW update repositories:")
    for name in update_repos:
        print(f"  {name}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
