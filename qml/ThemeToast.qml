// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

TokenPanel {
    id: root
    role: "toast"
    surfaceToken: "surface.panel"
    property string message: ""
    property int timeout: Math.round(Constants.motionSlowDuration * Constants.u10)
    property bool open: false
    signal dismissed()
    visible: opacity > Constants.ratio0
    opacity: open ? Constants.toastOpacity : Constants.ratio0
    implicitWidth: Math.min(Constants.dialogMaxWidth, textItem.implicitWidth + Constants.toastPaddingX * Constants.u2)

    TokenText { id: textItem; text: root.message; role: "bodySmall"; colorToken: "alias.toast.text.normal" }
    Timer { interval: root.timeout; running: root.open && root.timeout > Constants.durationInstant; onTriggered: { root.open = false; root.dismissed() } }
    Behavior on opacity { NumberAnimation { duration: Settings.animationsEnabled ? Constants.toastAnimationNormal : Constants.durationInstant; easing.type: Constants.easing("alias.motion.standard.easing") } }
    function show(text, duration) {
        message = text
        if (duration !== undefined)
            timeout = duration
        open = true
    }
}
