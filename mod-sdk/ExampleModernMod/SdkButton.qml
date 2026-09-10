import QtQuick

Rectangle {
    id: root
    property alias text: label.text
    property bool primary: false
    signal activated()

    implicitHeight: GameTheme.number("alias.button.height")
    radius: GameTheme.number("alias.button.radius")
    color: pointer.pressed
           ? GameTheme.color("alias.button.background.pressed")
           : pointer.containsMouse
             ? GameTheme.color("alias.button.background.hover")
             : (primary ? GameTheme.color("alias.button.accent.normal")
                        : GameTheme.color("alias.button.background.normal"))
    border.color: pointer.activeFocus
                  ? GameTheme.color("alias.button.border.focus")
                  : GameTheme.color("alias.button.border.normal")
    border.width: pointer.activeFocus
                  ? GameTheme.number("alias.button.checkedBorderWidth")
                  : GameTheme.number("alias.button.borderWidth")

    Text {
        id: label
        anchors.centerIn: parent
        color: root.primary
               ? GameTheme.color("color.espresso.950")
               : GameTheme.color("alias.button.text.normal")
        font.pixelSize: GameTheme.number("alias.button.fontSize")
        font.weight: Font.DemiBold
    }

    MouseArea {
        id: pointer
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.activated()
    }
}
