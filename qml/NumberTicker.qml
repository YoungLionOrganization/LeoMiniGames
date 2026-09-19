// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

TokenText {
    id: root
    property real value: Constants.spaceNone
    property int decimals: Math.round(Constants.spaceNone)
    property string prefix: ""
    property string suffix: ""
    property real displayedValue: value
    text: prefix + Number(displayedValue).toFixed(decimals) + suffix

    Behavior on displayedValue {
        NumberAnimation {
            duration: Settings.animationsEnabled ? Constants.n("alias.numberTicker.duration") : Constants.durationInstant
            easing.type: Constants.easing("alias.motion.standard.easing")
        }
    }
    onValueChanged: displayedValue = value
}
