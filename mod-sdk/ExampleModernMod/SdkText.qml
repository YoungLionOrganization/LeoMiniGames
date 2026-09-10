import QtQuick

Text {
    id: root
    property string colorToken: "alias.panel.text.normal"
    property string sizeToken: "alias.panel.fontSize"
    color: GameTheme.color(colorToken)
    font.pixelSize: GameTheme.number(sizeToken)
    wrapMode: Text.WordWrap
}
