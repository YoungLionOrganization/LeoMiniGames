// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

ThemeSurface {
    id: root

    property string themeId: ""
    property string themeName: themeId
    property string description: ""
    property string publisher: ""
    property string publisherStatus: ""
    property bool publisherVerified: false
    property string version: ""
    property string installedVersion: ""
    property string category: ""
    property var tags: []
    property string iconUrl: ""
    property var previewUrls: []
    property double sizeBytes: Constants.spaceNone
    property double downloads: Constants.spaceNone
    property string state: "idle"
    property real progress: Constants.ratio0
    property string operationError: ""
    property bool installed: false
    property bool updateAvailable: false
    property bool active: false
    property bool builtIn: false
    property bool inCatalog: true
    property bool installedView: false

    signal installRequested(string themeId)
    signal applyRequested(string themeId)
    signal removeRequested(string themeId)

    function formattedSize() {
        if (root.sizeBytes <= Constants.spaceNone)
            return ""
        if (root.sizeBytes >= Constants.u1024 * Constants.u1024)
            return qsTr("%1 MB").arg((root.sizeBytes / (Constants.u1024 * Constants.u1024)).toFixed(Constants.u1))
        if (root.sizeBytes >= Constants.u1024)
            return qsTr("%1 KB").arg(Math.round(root.sizeBytes / Constants.u1024))
        return qsTr("%1 B").arg(Math.round(root.sizeBytes))
    }

    surfaceToken: "surface.card"
    radius: Constants.cardRadius
    borderWidth: root.active ? Constants.n("alias.themeMarket.active.borderWidth") : Constants.cardBorderWidth
    borderColor: root.active ? Constants.success : Constants.cardBorderNormal
    implicitHeight: Math.max(Constants.n("alias.themeMarket.card.minHeight"), contentRow.implicitHeight + Constants.n("alias.themeMarket.card.padding") * Constants.u2)

    Row {
        id: contentRow
        anchors.fill: parent
        anchors.margins: Constants.n("alias.themeMarket.card.padding")
        spacing: Constants.n("alias.themeMarket.card.gap")

        ThemeSurface {
            id: iconShell
            width: Constants.n("alias.themeMarket.card.iconSize")
            height: Constants.n("alias.themeMarket.card.iconSize")
            surfaceToken: "surface.control"
            radius: Constants.n("alias.thumbnail.radius")
            visible: root.iconUrl.toString().length > Constants.spaceNone

            Loader {
                anchors.fill: parent
                anchors.margins: Constants.n("alias.themeMarket.card.iconInset")
                active: root.iconUrl.toString().length > Constants.spaceNone
                sourceComponent: Component {
                    Image {
                        source: root.iconUrl
                        fillMode: Image.PreserveAspectFit
                        asynchronous: true
                        cache: true
                        smooth: true
                    }
                }
            }
        }

        Column {
            width: Math.max(Constants.spaceNone, parent.width - (iconShell.visible ? iconShell.width + parent.spacing : Constants.spaceNone))
            spacing: Constants.n("alias.themeMarket.card.gap")

            Row {
                width: parent.width
                spacing: Constants.n("alias.themeMarket.badge.gap")

                TokenText {
                    width: Math.max(Constants.spaceNone, parent.width - statusBadge.width - parent.spacing)
                    text: root.themeName
                    role: "heading"
                    elide: Text.ElideRight
                }

                ThemeBadge {
                    id: statusBadge
                    text: root.active ? qsTr("ACTIVE")
                                      : root.updateAvailable ? qsTr("UPDATE")
                                                             : root.installed ? qsTr("INSTALLED") : root.category
                    tone: root.active ? "success" : root.updateAvailable ? "info" : "accent"
                }
            }

            Row {
                width: parent.width
                spacing: Constants.u5
                TokenText { width: Math.max(Constants.u1, parent.width - themePublisherBadge.width - parent.spacing); text: root.publisher + (root.publisherStatus.length > Constants.spaceNone ? " · " + root.publisherStatus : ""); role: "labelSmall"; colorToken: root.publisherVerified ? "color.success" : "color.textMuted"; elide: Text.ElideRight }
                PublisherBadge { id: themePublisherBadge; width: Constants.u16; height: Constants.u16; anchors.verticalCenter: parent.verticalCenter; status: root.publisherStatus; verified: root.publisherVerified }
            }

            TokenText {
                width: parent.width
                visible: root.description.length > Constants.spaceNone
                text: root.description
                role: "bodySmall"
                colorToken: "color.textMuted"
                wrapMode: Text.WordWrap
                maximumLineCount: Math.round(Constants.n("alias.themeMarket.card.descriptionMaxLines"))
                elide: Text.ElideRight
            }

            Flow {
                width: parent.width
                spacing: Constants.n("alias.themeMarket.badge.gap")

                ThemeBadge { text: qsTr("v%1").arg(root.version); tone: "accent" }
                ThemeBadge {
                    visible: root.installedVersion.length > Constants.spaceNone && root.updateAvailable
                    text: qsTr("Installed %1").arg(root.installedVersion)
                    tone: "info"
                }
                ThemeBadge {
                    visible: root.category.length > Constants.spaceNone
                    text: root.category
                    tone: "accent"
                }
                ThemeBadge {
                    visible: root.formattedSize().length > Constants.spaceNone
                    text: root.formattedSize()
                    tone: "accent"
                }
                ThemeBadge {
                    visible: root.downloads > Constants.spaceNone
                    text: qsTr("%1 downloads").arg(Math.round(root.downloads))
                    tone: "accent"
                }
                ThemeBadge {
                    visible: root.builtIn
                    text: qsTr("BUILT-IN")
                    tone: "accent"
                }
                ThemeBadge {
                    visible: !root.builtIn && root.installedView && !root.inCatalog
                    text: qsTr("LOCAL")
                    tone: "accent"
                }
            }

            ThemeProgressBar {
                width: parent.width
                visible: root.state === "resolving" || root.state === "downloading" || root.state === "installing"
                indeterminate: root.state === "resolving" || root.state === "installing"
                value: root.progress
            }

            TokenText {
                width: parent.width
                visible: root.operationError.length > Constants.spaceNone
                text: root.operationError
                role: "caption"
                colorToken: "color.danger"
                wrapMode: Text.WordWrap
            }

            Flow {
                width: parent.width
                spacing: Constants.n("alias.themeMarket.action.gap")

                ThemeButton {
                    visible: !root.installedView || (root.installedView && root.inCatalog)
                    text: root.state === "downloading" ? qsTr("DOWNLOADING")
                         : root.state === "installing" ? qsTr("INSTALLING")
                         : root.updateAvailable ? qsTr("UPDATE")
                         : root.installed ? qsTr("REINSTALL")
                         : qsTr("INSTALL")
                    enabled: root.state === "idle" || root.state === "error" || root.state === "installed" || root.state === "update_available"
                    primary: !root.installed || root.updateAvailable
                    onActivated: root.installRequested(root.themeId)
                }

                ThemeButton {
                    visible: root.installed
                    text: root.active ? qsTr("ACTIVE") : qsTr("APPLY")
                    enabled: !root.active
                    primary: !root.active
                    onActivated: root.applyRequested(root.themeId)
                }

                ThemeButton {
                    visible: root.installedView && !root.builtIn && !root.active
                    text: qsTr("REMOVE")
                    iconName: "remove"
                    onActivated: root.removeRequested(root.themeId)
                }
            }
        }
    }
}
