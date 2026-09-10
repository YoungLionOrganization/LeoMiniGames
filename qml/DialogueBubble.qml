// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

ThemeSurface {
    id: root
    property string speaker: ""
    property string text: ""
    property url avatarSource: ""
    surfaceToken: "surface.panel"
    radius: Constants.n("alias.dialogueBubble.radius")
    borderWidth: Constants.n("alias.dialogueBubble.borderWidth")
    implicitWidth: Constants.n("alias.dialogueBubble.maxWidth")
    implicitHeight: row.implicitHeight + Constants.n("alias.dialogueBubble.paddingY") * Constants.u2

    Row {
        id: row
        anchors.fill: parent
        anchors.margins: Constants.n("alias.dialogueBubble.padding")
        spacing: Constants.n("alias.dialogueBubble.gap")
        ThemeSurface {
            id: avatar
            visible: root.avatarSource.toString().length > Constants.spaceNone
            width: Constants.n("alias.dialogueBubble.avatarSize")
            height: width
            radius: width * Constants.ratio50
            Image { anchors.fill: parent; source: avatar.visible ? root.avatarSource : ""; fillMode: Image.PreserveAspectCrop; asynchronous: true }
        }
        Column {
            width: parent.width - (avatar.visible ? avatar.width + parent.spacing : Constants.spaceNone)
            spacing: Constants.n("alias.dialogueBubble.gap")
            TokenText { visible: root.speaker.length > Constants.spaceNone; text: root.speaker; role: "label"; colorToken: "color.accent" }
            TokenText { width: parent.width; text: root.text; role: "body"; wrapMode: Text.WordWrap; maximumLineCount: Math.round(Constants.n("alias.dialogueBubble.maxLines")); elide: Text.ElideRight }
        }
    }
}
