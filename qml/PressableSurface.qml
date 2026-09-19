// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

ThemeSurface {
    id: root
    signal clicked()
    signal pressAndHold()
    property string role: "button"
    property bool hoverEnabled: true
    property bool hapticEnabled: false
    property string hapticStrength: "light"
    readonly property bool pressed: tap.pressed
    readonly property bool hovered: hover.hovered

    opacity: enabled ? Constants.n("alias." + role + ".opacity") : Constants.n("alias." + role + ".disabledOpacity")
    scale: tap.pressed ? Constants.n("alias." + role + ".pressedScale")
                       : hover.hovered ? Constants.n("alias." + role + ".hoverScale")
                                       : Constants.scaleNormal
    radius: Constants.n("alias." + role + ".radius")
    borderWidth: Constants.n("alias." + role + ".borderWidth")

    HoverHandler { id: hover; enabled: root.enabled && root.hoverEnabled }
    TapHandler {
        id: tap
        enabled: root.enabled
        onTapped: {
            if (root.hapticEnabled)
                Haptics.trigger(root.hapticStrength)
            root.clicked()
        }
        onLongPressed: root.pressAndHold()
    }
    Behavior on scale {
        NumberAnimation {
            duration: Settings.animationsEnabled ? Constants.n("alias." + root.role + ".animationFast") : Constants.durationInstant
            easing.type: Constants.easing("alias.motion.press.easing")
        }
    }
    Behavior on opacity {
        NumberAnimation { duration: Settings.animationsEnabled ? Constants.motionFastDuration : Constants.durationInstant }
    }
}
