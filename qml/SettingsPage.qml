// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick
import QtQuick.Controls

Item {
    id: root
    signal backRequested()
    signal themesRequested()
    signal developerLabRequested()

    AppBackground { anchors.fill: parent }

    Flickable {
        anchors.fill: parent
        contentWidth: width
        contentHeight: page.implicitHeight + Constants.n("alias.settings.contentBottomPadding")
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

                BronzeButton {
                    text: ""
                    iconName: "back"
                    iconSize: Constants.u20
                    width: Constants.u48
                    height: Constants.u48
                    onClicked: root.backRequested()
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: Constants.u2
                    Text { text: qsTr("Settings"); color: Constants.text; font.pixelSize: Constants.u24; font.bold: true }
                    Text { text: qsTr("Tune LeoMiniGames for your device."); color: Constants.textMuted; font.pixelSize: Constants.u11 }
                }
            }

            SectionCard {
                width: parent.width - Constants.u32
                anchors.horizontalCenter: parent.horizontalCenter
                title: qsTr("Language")
                subtitle: qsTr("Changes apply instantly across the interface.")

                ComboBox {
                    id: languageBox
                    width: parent.width
                    model: Lang.availableLanguages
                    textRole: "name"

                    Component.onCompleted: {
                        for (let i = 0; i < model.length; ++i) {
                            if (model[i].code === Lang.language) {
                                currentIndex = i
                                break
                            }
                        }
                    }
                    onActivated: function(index) {
                        Audio.play("click")
                        Lang.language = model[index].code
                    }
                }
            }

            SectionCard {
                width: parent.width - Constants.u32
                anchors.horizontalCenter: parent.horizontalCenter
                title: qsTr("Audio")
                subtitle: Audio.available
                          ? qsTr("Low-latency feedback sounds are available.")
                          : qsTr("Qt Multimedia is not installed; the game still works without sound.")

                Row {
                    width: parent.width
                    spacing: Constants.u10

                    BronzeSwitch {
                        checked: Settings.soundEnabled
                        onToggled: {
                            Settings.soundEnabled = checked
                            if (checked) Audio.play("success")
                        }
                    }

                    Column {
                        width: parent.width - Constants.u60
                        anchors.verticalCenter: parent.verticalCenter
                        Text { text: qsTr("Sound effects"); color: Constants.text; font.bold: true }
                        Text {
                            width: parent.width
                            text: qsTr("UI, cards, flags, matches, reactions, wins and errors.")
                            color: Constants.textMuted
                            font.pixelSize: Constants.u10
                            wrapMode: Text.WordWrap
                        }
                    }
                }

                Column {
                    width: parent.width
                    spacing: Constants.u6
                    Row {
                        width: parent.width
                        Text { text: qsTr("Volume"); color: Constants.text }
                        Item { width: parent.width - Constants.u130; height: Constants.u1 }
                        Text { text: Math.round(Settings.soundVolume * Constants.u100) + "%"; color: Constants.accent; font.bold: true }
                    }
                    Slider {
                        width: parent.width
                        from: Constants.u0
                        to: Constants.u1
                        value: Settings.soundVolume
                        enabled: Settings.soundEnabled && Audio.available
                        onMoved: Settings.soundVolume = value
                    }
                }
            }

            SectionCard {
                width: parent.width - Constants.u32
                anchors.horizontalCenter: parent.horizontalCenter
                title: qsTr("Theme")
                subtitle: qsTr("Themes use shared design tokens across the app and compatible games.")

                Row {
                    width: parent.width
                    spacing: Constants.spaceSm
                    Column {
                        width: parent.width - themeButton.width - Constants.spaceSm
                        Text { text: ThemeRuntime.activeThemeName; color: Constants.text; font.bold: true }
                        Text { text: qsTr("%1 themes installed").arg(ThemeRuntime.themeCount); color: Constants.textMuted; font.pixelSize: Constants.fontXs }
                    }
                    BronzeButton { id: themeButton; text: qsTr("Theme Market"); onClicked: root.themesRequested() }
                }
            }

            SectionCard {
                width: parent.width - Constants.u32
                anchors.horizontalCenter: parent.horizontalCenter
                title: qsTr("Visuals")
                subtitle: qsTr("Disable motion if you prefer maximum responsiveness on older phones.")

                Row {
                    width: parent.width
                    spacing: Constants.u10

                    BronzeSwitch {
                        checked: Settings.animationsEnabled
                        onToggled: Settings.animationsEnabled = checked
                    }

                    Column {
                        width: parent.width - Constants.u60
                        anchors.verticalCenter: parent.verticalCenter
                        Text { text: qsTr("Animations"); color: Constants.text; font.bold: true }
                        Text { text: qsTr("Card entrances, button feedback and game transitions."); color: Constants.textMuted; font.pixelSize: Constants.u10 }
                    }
                }
            }

            SectionCard {
                width: parent.width - Constants.u32
                anchors.horizontalCenter: parent.horizontalCenter
                title: qsTr("Mods & network")
                subtitle: qsTr("Community packages are downloaded from the official catalog and verified before use.")

                Column {
                    width: parent.width
                    spacing: Constants.u7

                    Text { text: qsTr("Catalog endpoint"); color: Constants.text; font.bold: true }
                    Text {
                        width: parent.width
                        text: Mods.apiBaseUrl
                        color: Constants.primaryHover
                        font.pixelSize: Constants.u10
                        wrapMode: Text.WrapAnywhere
                    }
                    Text {
                        width: parent.width
                        text: qsTr("%1 installed/catalog entries currently known").arg(Mods.count)
                        color: Constants.textMuted
                        font.pixelSize: Constants.u10
                    }
                    BronzeButton {
                        text: qsTr("Refresh mod catalog")
                        onClicked: {
                            Audio.play("click")
                            Mods.refresh()
                        }
                    }
                }
            }

            SectionCard {
                width: parent.width - Constants.u32
                anchors.horizontalCenter: parent.horizontalCenter
                title: qsTr("Developer Mode")
                subtitle: qsTr("Local RCC validation, compatibility diagnostics and device-profile testing.")
                Row {
                    width: parent.width
                    spacing: Constants.u8
                    Text { width: parent.width - devButton.width - Constants.u8; text: Developer.authenticated ? qsTr("Active for this session") : qsTr("Requires a verified developer credential"); color: Developer.authenticated ? Constants.success : Constants.textMuted; wrapMode: Text.WordWrap }
                    BronzeButton { id: devButton; text: qsTr("Developer Lab"); onClicked: root.developerLabRequested() }
                }
            }


            SectionCard {
                width: parent.width - Constants.u32
                anchors.horizontalCenter: parent.horizontalCenter
                title: qsTr("Updates")
                subtitle: qsTr("Check GitHub for LeoMiniGames application updates.")

                Column {
                    width: parent.width
                    spacing: Constants.u8

                    Column {
                        width: parent.width
                        spacing: Constants.u6
                        Text {
                            width: parent.width
                            text: qsTr("Update channel")
                            color: Constants.text
                            font.bold: true
                            wrapMode: Text.WordWrap
                        }
                        ComboBox {
                            id: channelBox
                            width: Math.min(parent.width, Constants.u180)
                            model: [qsTr("Stable"), qsTr("Preview")]
                            currentIndex: Updates.channel === "preview" ? 1 : 0
                            onActivated: function(index) {
                                Updates.channel = index === 1 ? "preview" : "stable"
                            }
                        }
                    }

                    Text {
                        width: parent.width
                        visible: Updates.channel === "preview"
                        text: qsTr("Preview includes alpha, beta and release-candidate builds.")
                        color: Constants.accent
                        font.pixelSize: Constants.u10
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        width: parent.width
                        text: {
                            if (Updates.checking)
                                return qsTr("Checking for updates…")
                            if (Updates.status === "available")
                                return qsTr("Update available: %1").arg(Updates.latestVersion)
                            if (Updates.status === "up-to-date")
                                return qsTr("LeoMiniGames is up to date.")
                            if (Updates.status === "error")
                                return qsTr("Update check failed: %1").arg(Updates.errorString)
                            return qsTr("Current version: %1").arg(Updates.currentVersion)
                        }
                        color: Updates.status === "available" ? Constants.success : Constants.textMuted
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        width: parent.width
                        text: Updates.maintenanceAvailable
                              ? qsTr("Installed build · updates are applied by LeoMiniGames Maintenance.")
                              : qsTr("Portable build · updates open the matching GitHub release package.")
                        color: Constants.textMuted
                        font.pixelSize: Constants.u10
                        wrapMode: Text.WordWrap
                    }

                    Flow {
                        width: parent.width
                        spacing: Constants.u8
                        BronzeButton {
                            text: Updates.checking ? qsTr("Checking…") : qsTr("Check for updates")
                            enabled: !Updates.checking
                            onClicked: Updates.checkForUpdates()
                        }
                        BronzeButton {
                            visible: Updates.updateAvailable
                            text: Updates.maintenanceAvailable ? qsTr("Open updater") : qsTr("Download update")
                            onClicked: Updates.openUpdate()
                        }
                        BronzeButton {
                            visible: Updates.releaseUrl.toString().length > 0
                            text: qsTr("Release notes")
                            onClicked: Updates.openReleasePage()
                        }
                    }
                }
            }

            SectionCard {
                width: parent.width - Constants.u32
                anchors.horizontalCenter: parent.horizontalCenter
                title: qsTr("About")
                subtitle: qsTr("A lightweight plugin-based mini game collection.")

                Row {
                    spacing: Constants.u12
                    BrandLogo { width: Constants.u72; height: Constants.u72 }
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: Constants.u2
                        Text { text: "LeoMiniGames"; color: Constants.text; font.pixelSize: Constants.u17; font.bold: true }
                        Text { text: "v" + Qt.application.version + " · YoungLion"; color: Constants.primaryHover; font.pixelSize: Constants.u11 }
                        Text { text: qsTr("%1 games available").arg(Games.totalCount); color: Constants.textMuted; font.pixelSize: Constants.u10 }
                        Text { text: "LicenseRef-LMG-SAPEL-1.0"; color: Constants.textMuted; font.pixelSize: Constants.u10 }
                        Text { text: qsTr("Plugin API 0.7 · legacy 0.5/0.6 · RCC v1 compatible"); color: Constants.textMuted; font.pixelSize: Constants.u10 }
                    }
                }
            }
        }
    }
}
