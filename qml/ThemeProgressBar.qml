// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

Item {
    id: root
    property real value: Constants.ratio0
    property real from: Constants.ratio0
    property real to: Constants.ratio100
    property bool indeterminate: false
    readonly property real normalized: Math.max(Constants.ratio0, Math.min(Constants.ratio100, (value - from) / Math.max(Constants.ratio1, to - from)))
    implicitHeight: Constants.progressHeight

    ThemeSurface { anchors.fill: parent; surfaceToken: "surface.control"; radius: Constants.progressRadius; color: Constants.progressBackgroundNormal }
    ThemeSurface {
        id: fill
        height: parent.height
        width: root.indeterminate ? parent.width * Constants.ratio35 : parent.width * root.normalized
        surfaceToken: "surface.control"
        radius: Constants.progressRadius
        color: Constants.progressAccentNormal
        x: root.indeterminate ? (parent.width - width) * pulse : Constants.spaceNone
        property real pulse: Constants.ratio0
        SequentialAnimation on pulse {
            running: root.indeterminate && Settings.animationsEnabled
            loops: Animation.Infinite
            NumberAnimation { to: Constants.ratio100; duration: Constants.motionSlowDuration; easing.type: Constants.easing("alias.motion.standard.easing") }
            NumberAnimation { to: Constants.ratio0; duration: Constants.motionSlowDuration; easing.type: Constants.easing("alias.motion.standard.easing") }
        }
    }
}
