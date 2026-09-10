// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

Rectangle {
    id: root
    property bool active: false
    property real targetRadius: Constants.radiusMd
    color: "transparent"
    radius: targetRadius + Constants.n("alias.focus.ringOffset")
    border.width: active ? Constants.n("alias.focus.ringWidth") : Constants.spaceNone
    border.color: Constants.c("alias.focusRing.color")
    opacity: active ? Constants.opacityFull : Constants.spaceNone
    Behavior on opacity {
        NumberAnimation { duration: Settings.animationsEnabled ? Constants.motionFastDuration : Constants.durationInstant }
    }
}
