import QtQuick
Rectangle {
    id: root
    property string backgroundToken: "alias.panel.background.normal"
    property string borderToken: "alias.panel.border.normal"
    function n(token) { let revision = GameTheme.revision; return GameTheme.number(token) }
    function c(token) { let revision = GameTheme.revision; return GameTheme.color(token) }
    property real contentPadding: n("alias.panel.padding")
    radius: n("alias.panel.radius")
    color: c(backgroundToken)
    border.color: c(borderToken)
    border.width: n("alias.panel.borderWidth")
}
