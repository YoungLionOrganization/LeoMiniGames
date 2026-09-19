// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick
import QtQuick.Controls

Item {
    id: root
    required property var sourceUrl
    property string modTitle: qsTr("Mod")
    property string modDescription: ""
    signal backRequested()

    AppBackground { anchors.fill: parent }

    Column {
        anchors.fill: parent
        spacing: Constants.u0

        ThemeSurface {
            width: parent.width
            height: Constants.u66
            color: Constants.background
            borderColor: Constants.surfaceRaised

            Row {
                anchors.fill: parent
                anchors.margins: Constants.u9
                spacing: Constants.u10

                BronzeButton {
                    text: ""
                    iconName: "back"
                    iconSize: Constants.u20
                    width: Constants.u46
                    height: Constants.u46
                    onClicked: root.backRequested()
                }

                Column {
                    width: parent.width - Constants.n("alias.modHost.headerReservedWidth")
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: Constants.u1
                    Text {
                        width: parent.width
                        text: root.modTitle
                        color: Constants.text
                        font.pixelSize: Constants.u20
                        font.bold: true
                        elide: Text.ElideRight
                    }
                    Text {
                        width: parent.width
                        text: root.modDescription
                        color: Constants.textMuted
                        font.pixelSize: Constants.u10
                        elide: Text.ElideRight
                    }
                }
            }
        }

        Loader {
            id: modLoader
            width: parent.width
            height: parent.height - Constants.n("alias.modHost.bodyReservedHeight")
            source: root.sourceUrl

            onStatusChanged: {
                if (status === Loader.Error)
                    Audio.play("error")
            }
        }
    }
}
