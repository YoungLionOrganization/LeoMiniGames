// SPDX-License-Identifier: GPL-3.0-or-later
// Compatibility facade. New code should use Constants directly.
pragma Singleton
import QtQuick
QtObject {
    readonly property color espresso990: Constants.background
    readonly property color espresso950: Constants.background
    readonly property color espresso900: Constants.background
    readonly property color espresso850: Constants.surfaceRaised
    readonly property color espresso800: Constants.surfaceRaised
    readonly property color espresso750: Constants.surfaceInteractive
    readonly property color espresso700: Constants.surfaceInteractive
    readonly property color bronzeDark: Constants.borderStrong
    readonly property color bronze: Constants.primary
    readonly property color bronzeLight: Constants.primaryHover
    readonly property color gold: Constants.accent
    readonly property color goldSoft: Constants.accent
    readonly property color cream: Constants.text
    readonly property color muted: Constants.textMuted
    readonly property color mutedDark: Constants.textSubtle
    readonly property color danger: Constants.danger
    readonly property color success: Constants.success
    readonly property color info: Constants.info
    readonly property color purple: Constants.special
    readonly property real radius: Constants.radiusLg
    readonly property real smallRadius: Constants.radiusMd
    readonly property real gap: Constants.spaceMd
    readonly property real fast: Constants.durationFast
    readonly property real normal: Constants.durationNormal
    readonly property real slow: Constants.durationSlow
}
