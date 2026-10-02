import QtQuick
Text {
    id: root
    property string colorToken: "alias.panel.text.normal"
    property string sizeToken: "alias.panel.fontSize"
    function n(token) { let revision = GameTheme.revision; return GameTheme.number(token) }
    function c(token) { let revision = GameTheme.revision; return GameTheme.color(token) }
    color: c(colorToken)
    font.pixelSize: n(sizeToken)
    wrapMode: Text.WordWrap
}
