// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls

Item {
    id: root
    signal backRequested()
    property int activeTab: Constants.u0

    function refreshCatalog() {
        Audio.play("click")
        ThemeCatalog.refresh()
    }

    function installTheme(id) {
        Audio.play("download")
        ThemeCatalog.install(id)
    }

    function applyTheme(id) {
        if (ThemeRuntime.applyTheme(id))
            Audio.play("click")
    }

    function removeTheme(id) {
        Audio.play("click")
        ThemeCatalog.remove(id)
    }

    Component.onCompleted: {
        ThemesInstalled.sortMode = "active"
        if (ThemeCatalog.count === Constants.spaceNone && !ThemeCatalog.loading)
            ThemeCatalog.refresh()
    }

    AppBackground { anchors.fill: parent }

    Column {
        anchors.fill: parent
        anchors.margins: Constants.n("alias.themeMarket.page.padding")
        spacing: Constants.n("alias.themeMarket.page.gap")

        Row {
            width: parent.width
            spacing: Constants.n("alias.themeMarket.header.gap")

            ThemeIconButton {
                width: Constants.n("alias.themeMarket.header.backSize")
                height: Constants.n("alias.themeMarket.header.backSize")
                iconName: "back"
                onClicked: root.backRequested()
            }

            Column {
                width: Math.max(Constants.spaceNone, parent.width
                                - Constants.n("alias.themeMarket.header.reservedWidth"))
                anchors.verticalCenter: parent.verticalCenter
                spacing: Constants.n("alias.header.gap")

                TokenText {
                    text: qsTr("Themes")
                    role: "display"
                }
                TokenText {
                    width: parent.width
                    text: qsTr("Discover, install and activate design systems for LeoMiniGames and theme-aware games.")
                    role: "caption"
                    colorToken: "color.textMuted"
                    elide: Text.ElideRight
                }
            }

            ThemeIconButton {
                width: Constants.n("alias.themeMarket.header.actionSize")
                height: Constants.n("alias.themeMarket.header.actionSize")
                iconName: "refresh"
                enabled: !ThemeCatalog.loading
                onClicked: root.refreshCatalog()
            }
        }

        Row {
            width: parent.width
            spacing: Constants.n("alias.themeMarket.tab.gap")

            ThemeButton {
                text: qsTr("EXPLORE")
                width: (parent.width - parent.spacing) / Constants.u2
                height: Constants.n("alias.themeMarket.tab.height")
                enabled: root.activeTab !== Constants.u0
                primary: root.activeTab === Constants.u0
                onActivated: root.activeTab = Constants.u0
            }
            ThemeButton {
                text: qsTr("INSTALLED (%1)").arg(ThemesInstalled.count)
                width: (parent.width - parent.spacing) / Constants.u2
                height: Constants.n("alias.themeMarket.tab.height")
                enabled: root.activeTab !== Constants.u1
                primary: root.activeTab === Constants.u1
                onActivated: root.activeTab = Constants.u1
            }
        }

        ThemeTextField {
            id: search
            width: parent.width
            height: Constants.n("alias.themeMarket.search.height")
            placeholderText: root.activeTab === Constants.u0
                             ? qsTr("Search themes, publishers, categories or tags…")
                             : qsTr("Search installed themes…")
            onTextChanged: {
                ThemesExplore.searchQuery = text
                ThemesInstalled.searchQuery = text
            }
        }

        Row {
            visible: root.activeTab === Constants.u0
            width: parent.width
            spacing: Constants.n("alias.themeMarket.filter.gap")

            ComboBox {
                id: category
                width: (parent.width - parent.spacing) * Constants.n("alias.themeMarket.filter.widthRatio")
                model: ThemesExplore.categories
                onCurrentTextChanged: ThemesExplore.category = currentText.length > Constants.spaceNone ? currentText : "All"
            }
            ComboBox {
                id: sortMode
                width: (parent.width - parent.spacing) * Constants.n("alias.themeMarket.filter.widthRatio")
                model: [qsTr("Updated"), qsTr("Name"), qsTr("Downloads")]
                onCurrentIndexChanged: ThemesExplore.sortMode = currentIndex === Constants.u1
                                       ? "name" : (currentIndex === Constants.u2 ? "downloads" : "updated")
            }
        }

        ThemeSurface {
            visible: ThemeCatalog.error.length > Constants.spaceNone || ThemeCatalog.loading
            width: parent.width
            height: Constants.n("alias.themeMarket.status.height")
            radius: Constants.n("alias.status.radius")
            color: Constants.c("alias.status.background.normal")
            borderColor: ThemeCatalog.error.length > Constants.spaceNone ? Constants.danger : Constants.border

            TokenText {
                anchors.fill: parent
                anchors.margins: Constants.n("alias.themeMarket.status.padding")
                text: ThemeCatalog.loading ? qsTr("Refreshing theme catalog…") : ThemeCatalog.error
                colorToken: ThemeCatalog.error.length > Constants.spaceNone ? "color.danger" : "color.textMuted"
                role: "bodySmall"
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
            }
        }

        GridView {
            id: exploreGrid
            visible: root.activeTab === Constants.u0
            width: parent.width
            height: Math.max(Constants.spaceNone, parent.height - y)
            model: ThemesExplore
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            cellWidth: width < Constants.n("alias.themeMarket.grid.breakpoint") ? width : width / Constants.u2
            cellHeight: Constants.n("alias.themeMarket.grid.cellHeight")
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            delegate: exploreDelegate

            ThemeEmptyState {
                anchors.centerIn: parent
                visible: ThemesExplore.count === Constants.spaceNone && !ThemeCatalog.loading
                title: ThemeCatalog.error.length > Constants.spaceNone ? qsTr("Catalog unavailable") : qsTr("No themes found")
                description: ThemeCatalog.error.length > Constants.spaceNone
                             ? qsTr("Check the network or Theme Market backend and refresh.")
                             : qsTr("No published theme matches the current search and category filter.")
            }
        }

        GridView {
            id: installedGrid
            visible: root.activeTab === Constants.u1
            width: parent.width
            height: Math.max(Constants.spaceNone, parent.height - y)
            model: ThemesInstalled
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            cellWidth: width < Constants.n("alias.themeMarket.grid.breakpoint") ? width : width / Constants.u2
            cellHeight: Constants.n("alias.themeMarket.grid.cellHeight")
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            delegate: installedDelegate

            ThemeEmptyState {
                anchors.centerIn: parent
                visible: ThemesInstalled.count === Constants.spaceNone
                title: qsTr("No installed themes")
                description: qsTr("The built-in fallback theme should always remain available. Reload the application if this state persists.")
            }
        }
    }

    Component {
        id: exploreDelegate
        Item {
            required property string themeId
            required property string themeName
            required property string themeDescription
            required property string themePublisher
            required property string themePublisherStatus
            required property bool themePublisherVerified
            required property string themeVersion
            required property string themeInstalledVersion
            required property string themeCategory
            required property var themeTags
            required property string themeIconUrl
            required property var themePreviewUrls
            required property real themeSizeBytes
            required property real themeDownloads
            required property string themeState
            required property real themeProgress
            required property string themeOperationError
            required property bool themeInstalled
            required property bool themeUpdateAvailable
            required property bool themeActive

            width: GridView.view.cellWidth
            height: GridView.view.cellHeight

            ThemeMarketCard {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: Constants.n("alias.themeMarket.grid.cardRightMargin")
                height: Constants.n("alias.themeMarket.grid.cardHeight")
                themeId: parent.themeId
                themeName: parent.themeName
                description: parent.themeDescription
                publisher: parent.themePublisher
                publisherStatus: parent.themePublisherStatus
                publisherVerified: parent.themePublisherVerified
                version: parent.themeVersion
                installedVersion: parent.themeInstalledVersion
                category: parent.themeCategory
                tags: parent.themeTags
                iconUrl: parent.themeIconUrl
                previewUrls: parent.themePreviewUrls
                sizeBytes: parent.themeSizeBytes
                downloads: parent.themeDownloads
                state: parent.themeState
                progress: parent.themeProgress
                operationError: parent.themeOperationError
                installed: parent.themeInstalled
                updateAvailable: parent.themeUpdateAvailable
                active: parent.themeActive
                inCatalog: true
                installedView: false
                onInstallRequested: function(id) { root.installTheme(id) }
                onApplyRequested: function(id) { root.applyTheme(id) }
            }
        }
    }

    Component {
        id: installedDelegate
        Item {
            required property string themeId
            required property string themeName
            required property string themeDescription
            required property string themePublisher
            required property string themePublisherStatus
            required property bool themePublisherVerified
            required property string themeVersion
            required property string themeInstalledVersion
            required property string themeCategory
            required property var themeTags
            required property string themeIconUrl
            required property var themePreviewUrls
            required property real themeSizeBytes
            required property real themeDownloads
            required property string themeState
            required property real themeProgress
            required property string themeOperationError
            required property bool themeInstalled
            required property bool themeUpdateAvailable
            required property bool themeActive
            required property bool themeBuiltIn
            required property bool themeInCatalog

            width: GridView.view.cellWidth
            height: GridView.view.cellHeight

            ThemeMarketCard {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: Constants.n("alias.themeMarket.grid.cardRightMargin")
                height: Constants.n("alias.themeMarket.grid.cardHeight")
                themeId: parent.themeId
                themeName: parent.themeName
                description: parent.themeDescription
                publisher: parent.themePublisher
                publisherStatus: parent.themePublisherStatus
                publisherVerified: parent.themePublisherVerified
                version: parent.themeVersion
                installedVersion: parent.themeInstalledVersion
                category: parent.themeCategory
                tags: parent.themeTags
                iconUrl: parent.themeIconUrl
                previewUrls: parent.themePreviewUrls
                sizeBytes: parent.themeSizeBytes
                downloads: parent.themeDownloads
                state: parent.themeState
                progress: parent.themeProgress
                operationError: parent.themeOperationError
                installed: parent.themeInstalled
                updateAvailable: parent.themeUpdateAvailable
                active: parent.themeActive
                builtIn: parent.themeBuiltIn
                inCatalog: parent.themeInCatalog
                installedView: true
                onInstallRequested: function(id) { root.installTheme(id) }
                onApplyRequested: function(id) { root.applyTheme(id) }
                onRemoveRequested: function(id) { root.removeTheme(id) }
            }
        }
    }
}
