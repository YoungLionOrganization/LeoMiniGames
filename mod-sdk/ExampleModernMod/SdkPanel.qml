import QtQuick

Rectangle {
    id: root
    property string backgroundToken: "alias.panel.background.normal"
    property string borderToken: "alias.panel.border.normal"
    property real contentPadding: GameTheme.number("alias.panel.padding")
    radius: GameTheme.number("alias.panel.radius")
    color: GameTheme.color(backgroundToken)
    border.color: GameTheme.color(borderToken)
    border.width: GameTheme.number("alias.panel.borderWidth")
}
