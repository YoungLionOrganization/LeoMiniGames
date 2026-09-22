#!/usr/bin/env python3
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ERRORS: list[str] = []


def text(path: str) -> str:
    p = ROOT / path
    if not p.is_file():
        ERRORS.append(f"missing required file: {path}")
        return ""
    return p.read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        ERRORS.append(message)


def main() -> int:
    release_path = ROOT / "release/release.json"
    assets_path = ROOT / "release/assets.json"
    publishers_path = ROOT / "release/authorized_publishers.json"
    for p in (release_path, assets_path, publishers_path):
        require(p.is_file(), f"missing release metadata: {p.relative_to(ROOT)}")
    if ERRORS:
        return finish()

    release = json.loads(release_path.read_text(encoding="utf-8"))
    assets = json.loads(assets_path.read_text(encoding="utf-8"))
    publishers = json.loads(publishers_path.read_text(encoding="utf-8"))
    require(release.get("schema") == 1, "release/release.json schema must be 1")
    require(release.get("version") == "0.7.1", "canonical release version must be 0.7.1")
    require(release.get("tag") == "v0.7.1", "canonical release tag must be v0.7.1")
    require(release.get("target_branch") == "main", "release target branch must be main")
    require(release.get("update_track") in {"stable", "preview"}, "invalid update track")
    require(bool(publishers.get("publishers")), "authorized publisher allowlist must not be empty")
    require(assets.get("version") == release.get("version"), "release/assets.json version mismatch")

    cmake = text("CMakeLists.txt")
    main_cpp = text("src/main.cpp")
    package_xml = text("installer/packages/xyz.younglion.leominigames/meta/package.xml")
    config_xml = text("installer/config/config.xml")
    package_windows = text("tools/package/package_windows.ps1")
    update_h = text("src/core/UpdateService.h")
    update_cpp = text("src/core/UpdateService.cpp")
    settings_qml = text("qml/SettingsPage.qml")
    build_workflow = text(".github/workflows/build-artifacts.yml")
    publish_workflow = text(".github/workflows/publish-release.yml")
    ci_workflow = text(".github/workflows/ci.yml")

    package_source = text("tools/package/package_source.py")

    require("project(LeoMiniGames VERSION 0.7.1" in cmake, "CMake version is not 0.7.1")
    require("LEOMINIGAMES_ANDROID_VERSION_CODE 701" in cmake, "Android versionCode is not 701")
    require('setApplicationVersion(QStringLiteral("0.7.1"))' in main_cpp, "runtime app version is not 0.7.1")
    game_runtime_h = text("src/core/GameRuntime.h")
    game_runtime_cpp = text("src/core/GameRuntime.cpp")
    require('QStringLiteral("0.7.0")' not in game_runtime_h, "GameRuntime still hardcodes the previous app version")
    require("QCoreApplication::applicationVersion()" in game_runtime_cpp, "GameRuntime version is not bound to the canonical application version")
    require("<Version>0.7.1</Version>" in package_xml, "QtIFW package version is not 0.7.1")
    require("<Version>0.7.1</Version>" in config_xml, "QtIFW config version is not 0.7.1")

    require("<RemoteRepositories>" in config_xml, "QtIFW RemoteRepositories missing")
    require("@LMG_UPDATE_REPOSITORY_URL@" in config_xml, "QtIFW repository placeholder missing")
    require("--hybrid" in package_windows, "Windows installer is not built as QtIFW hybrid")
    require("repogen" in package_windows.lower(), "Windows package script does not generate an update repository")
    require("LeoMiniGamesMaintenance" in package_windows or "LeoMiniGamesMaintenance" in config_xml,
            "Maintenance Tool configuration missing")

    require("class UpdateService" in update_h, "UpdateService declaration missing")
    require("https://leominigames.younglion.xyz/api/v1/updates/v1/check" in update_cpp,
            "UpdateService backend gateway endpoint missing")
    require("api.github.com/repos/YoungLionOrganization/LeoMiniGames/releases" not in update_cpp
            and "raw.githubusercontent.com/YoungLionOrganization/LeoMiniGames/updates" not in update_cpp,
            "UpdateService must not talk directly to the GitHub update provider")
    require("NoLessSafeRedirectPolicy" in update_cpp, "UpdateService safe redirect policy missing")
    require("kMaxUpdateMetadataBytes" in update_cpp, "Update metadata size guard missing")
    require("request.setTransferTimeout" not in update_cpp,
            "UpdateService uses QNetworkRequest::setTransferTimeout, which requires Qt 6.7 but the project baseline is Qt 6.5")
    require("QTimer *timeout" in update_cpp and "QNetworkReply::abort" in update_cpp,
            "Qt 6.5-compatible network timeout guard missing")
    require("const QUrl finalUrl = reply->url()" in update_cpp and "isAllowedUpdateUrl(finalUrl)" in update_cpp,
            "final redirect host validation missing")
    require("provider_contract" in update_cpp and "leominigames-update-v1" in update_cpp,
            "backend update contract validation missing")
    require("leominigames.younglion.xyz" in update_cpp,
            "YoungLion update domain validation missing")
    require("https://leominigames.younglion.xyz/updates/qtifw/$UpdateTrack/windows/$Suffix" in package_windows,
            "Windows QtIFW repository is not routed through the LeoMiniGames backend")
    require("raw.githubusercontent.com/YoungLionOrganization/LeoMiniGames/updates" not in package_windows,
            "Windows package script still embeds the GitHub update repository directly")
    require(re.search(r"#if defined\(Q_OS_WIN\)\s*#include <QProcess>\s*#endif", update_cpp) is not None,
            "QProcess include must be Windows-only so iOS/iPadOS builds remain portable")
    launch_block = update_cpp.split("bool UpdateService::launchMaintenance()", 1)[1].split("bool UpdateService::openUpdate()", 1)[0]
    require("#if defined(Q_OS_WIN)" in launch_block and "QProcess::startDetached" in launch_block and "#else" in launch_block,
            "launchMaintenance must compile QProcess code only on Windows")
    require("const qsizetype common = qMin(" in update_cpp,
            "SemVer prerelease comparison should avoid Apple 64-bit narrowing warnings")
    require("UpdateService" in main_cpp and 'setContextProperty(QStringLiteral("Updates")' in main_cpp,
            "UpdateService is not exposed to QML")
    require("Updates.checkForUpdates()" in settings_qml, "Settings update-check action missing")
    require("Updates.openUpdate()" in settings_qml, "Settings update action missing")

    require("release-assets:" in build_workflow, "final release-assets job missing")
    require("collect_release_assets.py" in build_workflow, "release asset allowlist collector missing")
    require("update-repositories" in build_workflow, "QtIFW update-repositories artifact missing")
    require("tools/validate_v071.py" in build_workflow, "build workflow does not run v0.7.1 validator")
    require("tools/validate_v071.py" in ci_workflow, "CI does not run v0.7.1 validator")

    require("tools/validate_v071.py" in package_source, "source package script does not run v0.7.1 validator")
    require("tools/validate_v070.py" not in package_source and "tools/validate_v060.py" not in package_source,
            "source package script still runs version-locked historical validators")
    require("VERSION=match.group(1)" in package_source, "source package version is not derived from CMake")

    # Publish must be manual-only. Reject common automatic triggers rather than matching words in comments/scripts.
    require(re.search(r"(?m)^on:\s*\n\s+workflow_dispatch:", publish_workflow) is not None,
            "Publish Release must use workflow_dispatch")
    for forbidden in ("push:", "pull_request:", "schedule:", "workflow_run:", "release:"):
        header = publish_workflow.split("permissions:", 1)[0]
        require(forbidden not in header, f"Publish Release contains forbidden trigger: {forbidden}")
    require("environment: release" in publish_workflow, "publish job must use protected release environment")
    require("contents: write" in publish_workflow, "publish job must receive contents:write")
    require("contents: read" in publish_workflow and "actions: read" in publish_workflow,
            "publish workflow preflight permissions are not least-privilege")
    require("authorized_publishers.json" in publish_workflow, "authorized publisher gate missing")
    require("headSha" in publish_workflow and "GITHUB_SHA" in publish_workflow,
            "exact-SHA workflow gate missing")
    require("--draft" in publish_workflow and "gh release upload" in publish_workflow,
            "draft-first release transaction missing")
    require("id: draft" in publish_workflow and "steps.draft.outputs.release_id" in publish_workflow,
            "publish workflow must carry the draft release id across steps")
    require("Recovering existing draft release" in publish_workflow
            and "Deleting stale draft release" in publish_workflow
            and "releases/assets/$asset_id" in publish_workflow,
            "publish workflow must recover same-SHA drafts, replace stale drafts, and clean partial assets before retry")
    require('releases/$RELEASE_ID/assets?per_page=100' in publish_workflow,
            "draft asset verification must use release id instead of the published-tag endpoint")
    require('releases/tags/$TAG" --jq' not in publish_workflow,
            "publish workflow must not resolve a draft release through /releases/tags/{tag}")
    require("update-repositories" in publish_workflow and "HEAD:refs/heads/updates" in publish_workflow,
            "GitHub-hosted QtIFW repository publication missing")

    forbidden_public = set(assets.get("forbidden_public_patterns", []))
    for expected in {"*.sha256", "*-Legal.zip", "*.aab", "*-unsigned.zip"}:
        require(expected in forbidden_public, f"missing forbidden release asset policy: {expected}")

    return finish()


def finish() -> int:
    if ERRORS:
        print("v0.7.1 validation FAILED", file=sys.stderr)
        for error in ERRORS:
            print(f" - {error}", file=sys.stderr)
        return 1
    print("v0.7.1 validation PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
