import QtQuick
import QtQuick.Controls.Basic
Button {
    id: root
    property bool primary: false
    signal activated()
    function n(token) { let revision = GameTheme.revision; return GameTheme.number(token) }
    function c(token) { let revision = GameTheme.revision; return GameTheme.color(token) }
    implicitHeight: n("alias.button.height")
    focusPolicy: Qt.StrongFocus
    onClicked: activated()
    background: Rectangle {
        radius: root.n("alias.button.radius")
        color: root.down ? root.c("alias.button.background.pressed")
                        : root.hovered ? root.c("alias.button.background.hover")
                        : root.primary ? root.c("alias.button.accent.normal") : root.c("alias.button.background.normal")
        border.color: root.activeFocus ? root.c("alias.button.border.focus") : root.c("alias.button.border.normal")
        border.width: root.activeFocus ? root.n("alias.button.checkedBorderWidth") : root.n("alias.button.borderWidth")
        opacity: root.enabled ? 1 : 0.5
    }
    contentItem: Text {
        text: root.text
        color: root.primary ? root.c("color.espresso.950") : root.c("alias.button.text.normal")
        font.pixelSize: root.n("alias.button.fontSize")
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
