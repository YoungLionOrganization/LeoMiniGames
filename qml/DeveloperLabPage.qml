// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Item {
    id: root
    required property var developer
    required property var gameLogger
    signal backRequested()

    AppBackground { anchors.fill: parent }

    FileDialog {
        id: importDialog
        title: qsTr("Import local RCC")
        nameFilters: [qsTr("Qt resource packages (*.rcc)"), qsTr("All files (*)")]
        onAccepted: root.developer.importRcc(selectedFile)
    }
    FileDialog {
        id: textExportDialog
        title: qsTr("Export diagnostics as text")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "txt"
        nameFilters: [qsTr("Text log (*.txt)")]
        onAccepted: root.gameLogger.exportText(selectedFile.toString())
    }
    FileDialog {
        id: jsonExportDialog
        title: qsTr("Export diagnostics as JSON")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "json"
        nameFilters: [qsTr("JSON log (*.json)")]
        onAccepted: root.gameLogger.exportJson(selectedFile.toString())
    }

    Flickable {
        anchors.fill: parent
        contentWidth: width
        contentHeight: page.implicitHeight + Constants.u24
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

        Column {
            id: page
            width: parent.width
            anchors.top: parent.top
            anchors.topMargin: Constants.u16
            spacing: Constants.u14

            Row {
                width: parent.width - Constants.u32
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Constants.u12
                BronzeButton { text: ""; iconName: "back"; width: Constants.u48; height: Constants.u48; onClicked: root.backRequested() }
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    Text { text: qsTr("Developer Lab"); color: Constants.text; font.pixelSize: Constants.u24; font.bold: true }
                    Text { text: qsTr("Validate and run local RCC packages without publishing them."); color: Constants.textMuted; font.pixelSize: Constants.u11 }
                }
            }

            SectionCard {
                width: parent.width - Constants.u32
                anchors.horizontalCenter: parent.horizontalCenter
                title: qsTr("Developer account")
                subtitle: qsTr("The API key is verified for this process only and is never written to QSettings or disk.")
                Column {
                    width: parent.width
                    spacing: Constants.u8
                    Text { width: parent.width; text: root.developer.status; color: root.developer.authenticated ? Constants.success : Constants.textMuted; wrapMode: Text.WordWrap }
                    ThemeTextField {
                        id: keyField
                        width: parent.width
                        visible: !root.developer.authenticated
                        placeholderText: "lmg_…"
                        echoMode: TextInput.Password
                    }
                    Row {
                        spacing: Constants.u8
                        BronzeButton { visible: !root.developer.authenticated; text: qsTr("Open Developer Portal"); onClicked: root.developer.openDeveloperPortal() }
                        BronzeButton { visible: !root.developer.authenticated; enabled: !root.developer.verifying && keyField.text.length > 0; text: root.developer.verifying ? qsTr("Verifying…") : qsTr("Verify key"); onClicked: { root.developer.verifyApiKey(keyField.text); keyField.text = "" } }
                        BronzeButton { visible: root.developer.authenticated; text: qsTr("Log out"); onClicked: root.developer.logout() }
                    }
                    Text { visible: root.developer.lastError.length > 0; width: parent.width; text: root.developer.lastError; color: Constants.danger; wrapMode: Text.WordWrap }
                }
            }

            SectionCard {
                visible: root.developer.authenticated
                width: parent.width - Constants.u32
                anchors.horizontalCenter: parent.horizontalCenter
                title: qsTr("Local RCC Test Lab")
                subtitle: qsTr("Packages are copied to a verified session cache. Native/L3 permission is never granted to local manifests.")
                Column {
                    width: parent.width
                    spacing: Constants.u8
                    Row {
                        spacing: Constants.u8
                        BronzeButton { text: qsTr("Import .rcc"); onClicked: importDialog.open() }
                        BronzeButton { enabled: Object.keys(root.developer.packageInfo).length > 0; text: qsTr("Run package"); onClicked: root.developer.launchImported() }
                        BronzeButton { enabled: Object.keys(root.developer.packageInfo).length > 0; text: qsTr("Clear"); onClicked: root.developer.clearImported() }
                    }
                    Grid {
                        visible: Object.keys(root.developer.packageInfo).length > 0
                        width: parent.width
                        columns: width > Constants.u600 ? 2 : 1
                        spacing: Constants.u6
                        Repeater {
                            model: [
                                qsTr("ID") + ": " + (root.developer.packageInfo.id || ""),
                                qsTr("Version") + ": " + (root.developer.packageInfo.version || ""),
                                qsTr("Entry") + ": " + (root.developer.packageInfo.entry || ""),
                                qsTr("API") + ": " + (root.developer.packageInfo.apiVersion || "legacy"),
                                qsTr("RCC mount") + ": " + (root.developer.packageInfo.mountMode || ""),
                                qsTr("Resources") + ": " + (root.developer.packageInfo.resourceCount || 0),
                                qsTr("Publisher trust") + ": " + (root.developer.packageInfo.publisherTrust || ""),
                                qsTr("Native/L3") + ": " + ((root.developer.packageInfo.nativeRequested || false) ? qsTr("requested, denied") : qsTr("not requested"))
                            ]
                            delegate: Text { required property string modelData; width: root.width > Constants.u600 ? (root.width - Constants.u48) / 2 : root.width - Constants.u48; text: modelData; color: Constants.textMuted; wrapMode: Text.WrapAnywhere }
                        }
                    }
                }
            }

            SectionCard {
                visible: root.developer.authenticated
                width: parent.width - Constants.u32
                anchors.horizontalCenter: parent.horizontalCenter
                title: qsTr("Device profile")
                subtitle: qsTr("Developer RCC games can be rendered in a logical viewport and safe-area simulation.")
                ComboBox {
                    width: parent.width
                    model: root.developer.deviceProfiles
                    textRole: "name"
                    currentIndex: root.developer.selectedProfile
                    onActivated: function(index) { root.developer.selectDeviceProfile(index) }
                }
                Text {
                    width: parent.width
                    text: root.developer.profileWidth > 0 ? qsTr("%1 × %2 · safe top %3 · safe bottom %4").arg(root.developer.profileWidth).arg(root.developer.profileHeight).arg(root.developer.profileSafeTop).arg(root.developer.profileSafeBottom) : qsTr("Uses the current application window.")
                    color: Constants.textMuted
                    wrapMode: Text.WordWrap
                }
            }

            SectionCard {
                visible: root.developer.authenticated
                width: parent.width - Constants.u32
                anchors.horizontalCenter: parent.horizontalCenter
                title: qsTr("Runtime diagnostics")
                subtitle: qsTr("QML errors and warnings are retained in a bounded in-memory history and rotating log.")
                Column {
                    width: parent.width
                    spacing: Constants.u8
                    Text { text: qsTr("%1 diagnostic records").arg(root.gameLogger.count); color: Constants.text }
                    Text { visible: root.gameLogger.lastError.length > 0; width: parent.width; text: root.gameLogger.lastError; color: Constants.danger; wrapMode: Text.WordWrap }
                    Flow {
                        width: parent.width
                        spacing: Constants.u8

                        BronzeButton {
                            text: qsTr("Export TXT")
                            onClicked: textExportDialog.open()
                        }

                        BronzeButton {
                            text: qsTr("Export JSON")
                            onClicked: jsonExportDialog.open()
                        }

                        BronzeButton {
                            text: qsTr("Clear log")
                            onClicked: root.gameLogger.clear()
                        }
                    }
                }
            }
        }
    }
}
