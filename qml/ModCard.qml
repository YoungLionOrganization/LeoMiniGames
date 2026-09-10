// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls

ThemeSurface {
    id: root

    required property string modId
    required property string modName
    required property string modDescription
    required property string modAuthor
    required property string modVersion
    required property string modCategory
    required property string modIcon
    required property string modIconUrl
    required property real modSizeBytes
    required property real modDownloads
    required property bool modInstalled
    required property string modInstalledVersion
    required property string modState
    required property real modProgress
    required property string modError
    required property string modMinAppVersion
    required property string modPublisherStatus
    required property bool modPublisherVerified
    required property string modLicense
    required property string modSourceUrl
    required property bool modSourceAvailable
    required property int modPluginLevel
    required property bool modReviewed
    required property bool modNative
    property bool installedView: false

    signal installRequested(string id)
    signal uninstallRequested(string id)
    signal launchRequested(string id)

    readonly property bool busy: modState === "resolving" || modState === "downloading" || modState === "installing"
    readonly property bool updateAvailable: modInstalled && modVersion.length > 0 && modInstalledVersion !== modVersion
    readonly property real safeProgress: Number.isFinite(Number(modProgress)) ? Math.max(Constants.ratio0, Math.min(Constants.ratio100, Number(modProgress))) : Constants.ratio0

    function prettyBytes(value) {
        if (value < 1024) return value + " B"
        if (value < 1024 * 1024) return (value / 1024).toFixed(1) + " KiB"
        return (value / (1024 * 1024)).toFixed(1) + " MiB"
    }

    radius: Constants.n("alias.modCard.radius")
    color: Constants.surfaceRaised
    borderColor: pointer.containsMouse ? Constants.primaryHover : Constants.surfaceInteractive
    borderWidth: pointer.containsMouse ? Constants.n("alias.modCard.borderHover") : Constants.n("alias.modCard.borderNormal")
    clip: true

    Behavior on borderColor { ColorAnimation { duration: Settings.animationsEnabled ? Constants.durationFast : 0 } }

    MouseArea { id: pointer; anchors.fill: parent; hoverEnabled: true; acceptedButtons: Qt.NoButton }

    Column {
        anchors.fill: parent
        anchors.margins: Constants.n("alias.modCard.contentPadding")
        spacing: Constants.n("alias.modCard.contentGap")

        Row {
            width: parent.width
            spacing: Constants.n("alias.modCard.headerGap")

            ThemeSurface {
                width: Constants.n("alias.modCard.iconFrameSize"); height: Constants.n("alias.modCard.iconFrameSize"); radius: Constants.n("alias.modCard.iconFrameRadius"); color: Constants.surfaceInteractive; borderColor: Constants.borderStrong
                Loader {
                    id: catalogIcon
                    anchors.fill: parent
                    anchors.margins: Constants.n("alias.modCard.iconInset")
                    active: root.modIconUrl.length > 0
                    sourceComponent: Component {
                        Image {
                            source: root.modIconUrl
                            fillMode: Image.PreserveAspectFit
                            asynchronous: true
                            cache: true
                        }
                    }
                }
                AppIcon { anchors.centerIn: parent; width: Constants.n("alias.modCard.fallbackIconSize"); height: Constants.n("alias.modCard.fallbackIconSize"); name: "package"; visible: !catalogIcon.item || catalogIcon.item.status !== Image.Ready }
            }

            Column {
                width: parent.width - Constants.n("alias.modCard.iconReservedWidth")
                spacing: Constants.n("alias.modCard.headerTextGap")
                Text { width: parent.width; text: root.modName; color: Constants.text; font.pixelSize: Constants.n("alias.modCard.titleFontSize"); font.bold: true; elide: Text.ElideRight }
                Row {
                    width: parent.width
                    spacing: Constants.u5
                    Text { width: Math.max(Constants.u1, parent.width - modPublisherBadge.width - versionText.implicitWidth - parent.spacing * 2); text: root.modAuthor; color: root.modPublisherVerified ? Constants.accent : Constants.primaryHover; font.pixelSize: Constants.n("alias.modCard.metaFontSize"); font.bold: root.modPublisherVerified; elide: Text.ElideRight }
                    PublisherBadge { id: modPublisherBadge; width: Constants.u16; height: Constants.u16; anchors.verticalCenter: parent.verticalCenter; status: root.modPublisherStatus; verified: root.modPublisherVerified }
                    Text { id: versionText; text: "v" + root.modVersion; color: Constants.textMuted; font.pixelSize: Constants.n("alias.modCard.metaFontSize") }
                }
                Text { width: parent.width; text: root.modCategory + "  •  " + root.prettyBytes(root.modSizeBytes); color: Constants.textMuted; font.pixelSize: Constants.n("alias.modCard.detailFontSize"); elide: Text.ElideRight }
            }
        }

        Text { width: parent.width; text: root.modDescription; color: Constants.textMuted; wrapMode: Text.WordWrap; maximumLineCount: Constants.n("alias.modCard.descriptionLines"); elide: Text.ElideRight; font.pixelSize: Constants.n("alias.modCard.descriptionFontSize") }

        Row {
            width: parent.width
            spacing: Constants.n("alias.modCard.badgeGap")
            ThemeSurface {
                height: Constants.n("alias.modCard.badgeHeight"); width: statusText.implicitWidth + Constants.n("alias.modCard.badgeHorizontalExtra"); radius: Constants.n("alias.modCard.badgeRadius"); color: Constants.surfaceInteractive
                Text { id: statusText; anchors.centerIn: parent; text: root.modPublisherStatus.length > 0 ? root.modPublisherStatus : qsTr("Unverified Publisher"); color: root.modPublisherVerified ? Constants.success : Constants.textMuted; font.pixelSize: Constants.n("alias.modCard.badgeFontSize") }
            }
            ThemeSurface {
                visible: root.modLicense.length > 0
                height: Constants.n("alias.modCard.badgeHeight"); width: licenseText.implicitWidth + Constants.n("alias.modCard.badgeHorizontalExtra"); radius: Constants.n("alias.modCard.badgeRadius"); color: Constants.surfaceInteractive
                Text { id: licenseText; anchors.centerIn: parent; text: root.modLicense; color: Constants.primaryHover; font.pixelSize: Constants.n("alias.modCard.badgeFontSize") }
            }
            ThemeSurface {
                visible: root.modPluginLevel >= 3
                height: Constants.n("alias.modCard.badgeHeight"); width: nativeText.implicitWidth + Constants.n("alias.modCard.badgeHorizontalExtra"); radius: Constants.n("alias.modCard.badgeRadius"); color: Constants.surfaceInteractive
                Text { id: nativeText; anchors.centerIn: parent; text: qsTr("Trusted native"); color: Constants.accent; font.pixelSize: Constants.n("alias.modCard.badgeFontSize") }
            }
        }

        ThemeSurface {
            visible: root.busy
            width: parent.width; height: Constants.n("alias.modCard.progressHeight"); radius: Constants.n("alias.modCard.progressRadius"); color: Constants.surfaceInteractive
            ThemeSurface { width: parent.width * Math.max(Constants.n("alias.modCard.progressMinimum"), root.safeProgress); height: parent.height; radius: Constants.n("alias.modCard.progressRadius"); color: Constants.accent; Behavior on width { NumberAnimation { duration: Settings.animationsEnabled ? Constants.durationFast : 0 } } }
        }

        Text { visible: root.modError.length > 0; width: parent.width; text: root.modError; color: Constants.danger; font.pixelSize: Constants.n("alias.modCard.detailFontSize"); elide: Text.ElideRight }
        Item { width: Constants.n("alias.modCard.spacerSize"); height: Constants.n("alias.modCard.spacerSize") }

        Row {
            width: parent.width
            spacing: Constants.n("alias.modCard.actionGap")

            BronzeButton {
                visible: root.installedView && root.modInstalled
                text: qsTr("OPEN")
                width: Constants.n("alias.modCard.openWidth"); height: Constants.n("alias.modCard.actionHeight"); enabled: !root.busy
                onClicked: root.launchRequested(root.modId)
            }

            BronzeButton {
                visible: !root.modInstalled || root.installedView || root.updateAvailable
                text: root.modNative ? qsTr("TRUSTED NATIVE")
                      : root.busy
                      ? (root.modState === "installing" ? qsTr("Installing…") : qsTr("Downloading…"))
                      : root.updateAvailable ? qsTr("UPDATE")
                      : root.modInstalled ? qsTr("REINSTALL")
                      : qsTr("INSTALL")
                width: root.installedView ? Constants.n("alias.modCard.actionWidthInstalled") : Constants.n("alias.modCard.actionWidthCatalog")
                height: Constants.n("alias.modCard.actionHeight")
                enabled: !root.busy && !root.modNative
                onClicked: root.installRequested(root.modId)
            }

            ThemeSurface {
                visible: !root.installedView && root.modInstalled && !root.updateAvailable
                height: Constants.n("alias.modCard.actionHeight"); width: Constants.n("alias.modCard.installedBadgeWidth"); radius: Constants.radiusMd; color: Constants.surfaceInteractive
                Text { anchors.centerIn: parent; text: qsTr("INSTALLED"); color: Constants.success; font.pixelSize: Constants.n("alias.modCard.detailFontSize"); font.bold: true }
            }

            BronzeButton {
                visible: root.installedView && root.modInstalled
                text: qsTr("REMOVE")
                width: Constants.n("alias.modCard.removeWidth"); height: Constants.n("alias.modCard.actionHeight"); enabled: !root.busy
                onClicked: root.uninstallRequested(root.modId)
            }
        }
    }
}
