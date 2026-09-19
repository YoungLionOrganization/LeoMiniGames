// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

Item {
    id: root
    property string horizontalAction: "move_x"
    property string verticalAction: "move_y"
    property bool inputEnabled: true
    readonly property real axisX: knob.axisX
    readonly property real axisY: knob.axisY
    signal moved(real x, real y)

    implicitWidth: Constants.n("alias.virtualJoystick.minWidth")
    implicitHeight: Constants.n("alias.virtualJoystick.height")

    ThemeSurface {
        anchors.fill: parent
        surfaceToken: "surface.control"
        radius: Math.min(width, height) * Constants.ratio50
        color: Constants.c("alias.virtualJoystick.background.normal")
        borderColor: Constants.c("alias.virtualJoystick.border.normal")
        borderWidth: Constants.n("alias.virtualJoystick.borderWidth")
    }

    ThemeSurface {
        id: knob
        property real axisX: Constants.ratio0
        property real axisY: Constants.ratio0
        width: Math.min(root.width, root.height) * Constants.n("alias.virtualJoystick.knobRatio")
        height: width
        x: (root.width - width) * Constants.ratio50 + axisX * root.maximumTravel
        y: (root.height - height) * Constants.ratio50 + axisY * root.maximumTravel
        radius: width * Constants.ratio50
        color: Constants.c("alias.virtualJoystick.accent.normal")
        borderColor: Constants.c("alias.virtualJoystick.border.normal")
    }

    readonly property real maximumTravel: Math.max(Constants.spaceNone, (Math.min(width, height) - knob.width) * Constants.ratio50)

    function updateAxes(px, py) {
        const cx = root.width * Constants.ratio50
        const cy = root.height * Constants.ratio50
        const dx = px - cx
        const dy = py - cy
        const distance = Math.sqrt(dx * dx + dy * dy)
        const radius = Math.max(Constants.ratio100, Math.min(root.width, root.height) * Constants.ratio50)
        const scale = Math.min(Constants.ratio100, distance / radius)
        const nx = distance > Constants.spaceNone ? (dx / distance) * scale : Constants.ratio0
        const ny = distance > Constants.spaceNone ? (dy / distance) * scale : Constants.ratio0
        const dead = Constants.n("alias.virtualJoystick.deadZone")
        knob.axisX = Math.abs(nx) < dead ? Constants.ratio0 : nx
        knob.axisY = Math.abs(ny) < dead ? Constants.ratio0 : ny
        if (root.inputEnabled) {
            GameInput.setValue(root.horizontalAction, knob.axisX)
            GameInput.setValue(root.verticalAction, knob.axisY)
        }
        root.moved(knob.axisX, knob.axisY)
    }

    function resetAxes() {
        knob.axisX = Constants.ratio0
        knob.axisY = Constants.ratio0
        if (root.inputEnabled) {
            GameInput.release(root.horizontalAction)
            GameInput.release(root.verticalAction)
        }
        root.moved(Constants.ratio0, Constants.ratio0)
    }

    MouseArea {
        anchors.fill: parent
        preventStealing: true
        onPressed: mouse => root.updateAxes(mouse.x, mouse.y)
        onPositionChanged: mouse => { if (pressed) root.updateAxes(mouse.x, mouse.y) }
        onReleased: root.resetAxes()
        onCanceled: root.resetAxes()
    }
}
