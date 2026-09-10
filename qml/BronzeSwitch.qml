// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls

Switch {
    id: control
    indicator: ThemeSurface {
        implicitWidth: Constants.n("alias.switch.width")
        implicitHeight: Constants.n("alias.switch.height")
        x: control.leftPadding
        y: parent.height * Constants.ratio50 - height * Constants.ratio50
        radius: height * Constants.ratio50
        color: control.checked ? Constants.switchAccentNormal : Constants.switchBackgroundNormal
        borderColor: control.checked ? Constants.switchBorderFocus : Constants.switchBorderNormal

        ThemeSurface {
            width: Constants.n("alias.switch.thumbSize")
            height: Constants.n("alias.switch.thumbSize")
            radius: Constants.n("alias.switch.thumbRadius")
            y: Constants.n("alias.switch.thumbInset")
            x: control.checked
               ? parent.width - width - Constants.n("alias.switch.thumbInset")
               : Constants.n("alias.switch.thumbInset")
            color: Constants.switchForegroundNormal
            Behavior on x {
                NumberAnimation {
                    duration: Settings.animationsEnabled ? Constants.switchAnimationFast : Constants.durationInstant
                    easing.type: Constants.easing("alias.motion.press.easing")
                }
            }
        }
    }
}
