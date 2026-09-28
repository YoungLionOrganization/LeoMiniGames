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
    require(release.get("version") == "0.7.2", "canonical release version must be 0.7.2")
    require(release.get("tag") == "v0.7.2", "canonical release tag must be v0.7.2")
    require(release.get("target_branch") == "main", "release target branch must be main")
    require(release.get("update_track") in {"stable", "preview"}, "invalid update track")
    require(bool(publishers.get("publishers")), "authorized publisher allowlist must not be empty")
    require(assets.get("version") == release.get("version"), "release/assets.json version mismatch")
    notes_path = Path(str(release.get("release_notes_path", "")))
    safe_notes = not notes_path.is_absolute() and ".." not in notes_path.parts
    notes_file = ROOT / notes_path if safe_notes else None
    require(bool(notes_file and notes_file.is_file() and notes_file.read_text(encoding="utf-8").strip()),
            "release notes missing, empty or unsafe")

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
    developer_cpp = text("src/core/DeveloperManager.cpp")
    android_retry = text("tools/ci/android_build_with_retry.sh")
    package_android = text("tools/package/package_android.sh")
    gradle_alignment = text("tools/ci/preserve_android_native_alignment.gradle")

    package_source = text("tools/package/package_source.py")
    require("GRADLE_USER_HOME" in android_retry and "preserve_android_native_alignment.gradle" in android_retry
            and "keepDebugSymbols.add('**/*.so')" in gradle_alignment,
            "Android packaging must preserve aligned Qt native libraries through Gradle stripping")
    require(ci_workflow.count('python3 tools/validate_android_package.py "$apk" --abis') >= 2,
            "CI must validate actual packaged per-ABI and universal APK ELF alignment")

    require("project(LeoMiniGames VERSION 0.7.2" in cmake, "CMake version is not 0.7.2")
    require("LEOMINIGAMES_ANDROID_VERSION_CODE 702" in cmake, "Android versionCode is not 702")
    require('setApplicationVersion(QStringLiteral("0.7.2"))' in main_cpp, "runtime app version is not 0.7.2")
    game_runtime_h = text("src/core/GameRuntime.h")
    game_runtime_cpp = text("src/core/GameRuntime.cpp")
    require('QStringLiteral("0.7.0")' not in game_runtime_h, "GameRuntime still hardcodes the previous app version")
    require("QCoreApplication::applicationVersion()" in game_runtime_cpp, "GameRuntime version is not bound to the canonical application version")
    require("<Version>0.7.2</Version>" in package_xml, "QtIFW package version is not 0.7.2")
    require("<Version>0.7.2</Version>" in config_xml, "QtIFW config version is not 0.7.2")

    require("<RemoteRepositories>" in config_xml, "QtIFW RemoteRepositories missing")
    require("@LMG_UPDATE_REPOSITORY_URL@" in config_xml, "QtIFW repository placeholder missing")
    require("--hybrid" in package_windows, "Windows installer is not built as QtIFW hybrid")
    require("repogen" in package_windows.lower(), "Windows package script does not generate an update repository")
    require("LeoMiniGamesMaintenance" in package_windows or "LeoMiniGamesMaintenance" in config_xml,
            "Maintenance Tool configuration missing")

    control_qs = text("installer/config/control.qs")
    installer_script = text("installer/packages/xyz.younglion.leominigames/meta/installscript.qs")
    require("<ControlScript>control.qs</ControlScript>" in config_xml and "<WizardStyle>Classic</WizardStyle>" in config_xml,
            "QtIFW installer control script / stable Classic layout missing")
    require("<Logo>" not in config_xml and "<PageListPixmap>" not in config_xml,
            "QtIFW installer still stacks redundant logo/page-list artwork")
    require(all(name in package_xml for name in ("existinginstallation.ui", "installoptions.ui", "installationsummary.ui")),
            "QtIFW existing-install/options/summary pages missing")
    require("<Checkable>false</Checkable>" not in package_xml and "<ForcedInstallation>true</ForcedInstallation>" in package_xml
            and "<Default>true</Default>" in package_xml,
            "QtIFW application component must be preselected and required")
    require("LMGExistingInstallDir" in control_qs and "LeoMiniGamesMaintenance.exe" in control_qs
            and "--start-updater" in control_qs and "--start-package-manager" in control_qs and "--start-uninstaller" in control_qs,
            "existing-install Upgrade/Modify/Uninstall detection flow missing")
    require("ReadyForInstallationPageCallback" in control_qs and "InstallMsgLabel" in control_qs
            and "DynamicInstallationSummary" in control_qs and 'if (!page) return "upgrade"' in control_qs,
            "Ready page does not explicitly populate the You are installing summary")
    require("DesktopShortcutCheckBox" in installer_script and "StartMenuShortcutCheckBox" in installer_script
            and "MaintenanceShortcutCheckBox" in installer_script,
            "Windows integration choices missing from installer")
    require('setValidatorForCustomPage(component, "ExistingInstallationPage", "validateExistingInstallationPage")' in installer_script,
            "Existing installation must be handed to Maintenance Tool before TargetDirectory validation")
    require('selected("DesktopShortcutCheckBox")' in installer_script and 'return !checkbox || checkbox.checked' in installer_script,
            "QtIFW dynamic checkbox access is not guarded")
    require("LEOMINIGAMES_WINDOWS_CPU_PROFILE" in cmake and "/arch:AVX2" in cmake
            and "LMG_WINDOWS_X64_AVX2=1" in cmake and "CMAKE_SIZEOF_VOID_P EQUAL 4" in cmake,
            "Windows baseline/AVX2/64-bit-only build contract missing")
    require("windows-x64-avx2:" in build_workflow and "LMG_PLATFORM_SUFFIX: x86_64-AVX2" in build_workflow,
            "Windows x64 AVX2 release lane missing")
    require("-A Win32" not in build_workflow and "win32_msvc" not in build_workflow.lower(),
            "unsupported Windows x86 32-bit release lane must not be advertised")
    require("x86_64-avx2" in update_cpp,
            "AVX2 build does not preserve its architecture identity in update checks")
    require("windows/x86_64-AVX2/Updates.xml" in publish_workflow,
            "Publish preflight does not require the AVX2 QtIFW repository")
    asset_ids = {entry.get("id") for entry in assets.get("assets", [])}
    require({"windows-x64-avx2-portable","windows-x64-avx2-setup"}.issubset(asset_ids),
            "public release asset policy does not include AVX2 Windows ZIP/Setup")

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
    require("tools/validate_v072.py" in build_workflow, "build workflow does not run v0.7.2 validator")
    require("tools/validate_v072.py" in ci_workflow, "CI does not run v0.7.2 validator")
    require("tools/ci/android_build_with_retry.sh build-android apk" in ci_workflow,
            "CI Android lanes do not use the transient Gradle/network retry guard")
    require("gradle-distributions" in android_retry and "HTTP response code: 5" in android_retry
            and "non-retryable error" in android_retry,
            "Android retry helper must retry only recognized transient Gradle/network failures")
    require("android_build_with_retry.sh" in package_android,
            "Android release packaging does not use the transient Gradle/network retry guard")
    require('[[ -f "$ANDROID_BUILD_RETRY" ]]' in package_android
            and '[[ -x "$ANDROID_BUILD_RETRY" ]]' not in package_android,
            "Android packaging must not require executable permission on the retry helper")
    require('bash "$ANDROID_BUILD_RETRY" "$BUILD_DIR" apk' in package_android
            and 'bash "$ANDROID_BUILD_RETRY" "$BUILD_DIR" aab' in package_android,
            "Android packaging must invoke the retry helper through bash")
    require("/api/v1/developer/auth/verify" in developer_cpp,
            "Developer Lab does not use the dedicated API-key verification endpoint")
    require("/api/v1/developer/contents" not in developer_cpp,
            "Developer Lab still abuses the content-listing endpoint as an auth probe")
    require('value(QStringLiteral("authenticated")).toBool(false)' in developer_cpp
            and 'value(QStringLiteral("key")).isObject()' in developer_cpp,
            "Developer Lab does not validate the v0.7.2 auth response contract")

    require("tools/validate_v072.py" in package_source, "source package script does not run v0.7.2 validator")
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
    require("headSha" in publish_workflow and "TARGET_SHA" in publish_workflow and "build_run_id" in publish_workflow,
            "immutable build-candidate workflow gate missing")
    require("is_safe_publisher_delta" in publish_workflow
            and ".github/workflows/publish-release.yml|tools/validate_v072.py" in publish_workflow,
            "publisher-only delta reuse guard missing")
    require('--method POST "repos/$GITHUB_REPOSITORY/releases"' in publish_workflow
            and "'draft': True" in publish_workflow
            and "['upload_url']" in publish_workflow
            and "https://uploads.github.com/" in publish_workflow,
            "draft creation must use the REST response id/upload URL without a post-create lookup race")
    require("gh release upload" not in publish_workflow,
            "draft asset upload must not depend on tag-based gh release upload")
    require("id: draft" in publish_workflow and "steps.draft.outputs.release_id" in publish_workflow,
            "publish workflow must carry the draft release id across steps")
    require("Recovering existing draft release" in publish_workflow
            and "Deleting stale draft release" in publish_workflow
            and "releases/assets/$asset_id" in publish_workflow,
            "publish workflow must recover same-target drafts, replace stale drafts, and clean partial assets before retry")
    require('releases/$RELEASE_ID/assets?per_page=100' in publish_workflow,
            "draft asset verification must use release id instead of the published-tag endpoint")
    require('Created draft could not be resolved by release id' not in publish_workflow
            and 'find_release_id()' not in publish_workflow,
            "publish workflow still contains the draft post-create list-resolution race")
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
        print("v0.7.2 validation FAILED", file=sys.stderr)
        for error in ERRORS:
            print(f" - {error}", file=sys.stderr)
        return 1
    print("v0.7.2 validation PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
