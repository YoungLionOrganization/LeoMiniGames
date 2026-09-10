// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

Rectangle {
    id: root
    property bool hovered: false
    property bool pressed: false
    property bool focused: false
    property string role: "control"
    color: pressed ? Constants.c("alias." + role + ".background.pressed")
         : hovered ? Constants.c("alias." + role + ".background.hover")
         : focused ? Constants.c("alias." + role + ".background.focus")
         : "transparent"
    radius: Constants.n("alias." + role + ".radius")
    opacity: pressed || hovered || focused ? Constants.opacityFull : Constants.spaceNone
    Behavior on opacity {
        NumberAnimation { duration: Settings.animationsEnabled ? Constants.n("alias." + root.role + ".animationFast") : Constants.durationInstant }
    }
}
