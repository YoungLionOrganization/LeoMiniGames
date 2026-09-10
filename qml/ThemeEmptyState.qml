// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

Item {
    id: root
    property string iconName: "info"
    property string title: ""
    property string description: ""
    property string actionText: ""
    signal actionTriggered()
    implicitWidth: Constants.contentEmptyMaxWidth
    implicitHeight: column.implicitHeight

    Column {
        id: column
        width: Math.min(parent.width, Constants.contentEmptyMaxWidth)
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: Constants.emptyStateGap
        AppIcon { width: Constants.emptyStateIconSize; height: width; name: root.iconName; anchors.horizontalCenter: parent.horizontalCenter; color: Constants.emptyStateIconNormal }
        TokenText { width: parent.width; text: root.title; role: "heading"; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap }
        TokenText { width: parent.width; text: root.description; role: "bodySmall"; colorToken: "color.textMuted"; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap }
        ThemeButton { visible: root.actionText.length > 0; text: root.actionText; anchors.horizontalCenter: parent.horizontalCenter; onActivated: root.actionTriggered() }
    }
}
