// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick
import QtQuick.Controls

Item {
    id: root
    signal backRequested()
    signal launchRequested(string id)
    property int activeTab: 0

    function refreshCatalog() { Audio.play("click"); Mods.refresh() }
    function installMod(id) { Audio.play("download"); Mods.install(id) }
    function uninstallMod(id) { Audio.play("click"); Mods.uninstall(id) }

    Connections {
        target: Mods
        function onModInstalled(id) { Audio.play("install") }
        function onModUninstalled(id) { Audio.play("uninstall") }
    }

    Component.onCompleted: Mods.refresh()
    AppBackground { anchors.fill: parent }

    Column {
        anchors.fill: parent
        anchors.margins: Constants.n("alias.mods.pagePadding")
        spacing: Constants.n("alias.mods.pageGap")

        Row {
            width: parent.width
            spacing: Constants.n("alias.mods.pageGap")
            BronzeButton { text: ""; iconName: "back"; iconSize: Constants.n("alias.mods.backIconSize"); width: Constants.n("alias.mods.headerButtonHeight"); height: Constants.n("alias.mods.headerButtonHeight"); onClicked: root.backRequested() }
            Column {
                width: parent.width - Constants.n("alias.mods.headerReservedWidth")
                anchors.verticalCenter: parent.verticalCenter
                spacing: Constants.n("alias.mods.headerTextGap")
                Text { text: qsTr("Mods"); color: Constants.text; font.pixelSize: Constants.n("alias.mods.titleFontSize"); font.bold: true }
                Text { width: parent.width; text: qsTr("Discover and manage external games."); color: Constants.textMuted; font.pixelSize: Constants.n("alias.mods.subtitleFontSize"); elide: Text.ElideRight }
            }
            BronzeButton { text: ""; iconName: "refresh"; iconSize: Constants.n("alias.mods.refreshIconSize"); width: Constants.n("alias.mods.refreshButtonWidth"); height: Constants.n("alias.mods.headerButtonHeight"); enabled: !Mods.loading; onClicked: root.refreshCatalog() }
        }

        Row {
            width: parent.width
            spacing: Constants.n("alias.mods.filterGap")
            BronzeButton { text: qsTr("EXPLORE"); width: (parent.width - parent.spacing) / Constants.n("alias.mods.tabColumns"); height: Constants.n("alias.mods.tabHeight"); enabled: root.activeTab !== 0; onClicked: root.activeTab = 0 }
            BronzeButton { text: qsTr("INSTALLED (%1)").arg(ModsInstalled.count); width: (parent.width - parent.spacing) / Constants.n("alias.mods.tabColumns"); height: Constants.n("alias.mods.tabHeight"); enabled: root.activeTab !== 1; onClicked: root.activeTab = 1 }
        }

        TextField {
            id: search
            width: parent.width
            height: Constants.n("alias.mods.searchHeight")
            placeholderText: root.activeTab === 0 ? qsTr("Search catalog…") : qsTr("Search installed mods…")
            color: Constants.text
            placeholderTextColor: Constants.textSubtle
            leftPadding: Constants.n("alias.mods.searchPaddingX"); rightPadding: Constants.n("alias.mods.searchPaddingX")
            background: ThemeSurface { radius: Constants.radiusMd; color: Constants.surfaceRaised; borderColor: search.activeFocus ? Constants.primary : Constants.surfaceInteractive }
            onTextChanged: {
                ModsExplore.searchQuery = text
                ModsInstalled.searchQuery = text
            }
        }

        Row {
            visible: root.activeTab === 0
            width: parent.width
            spacing: Constants.n("alias.mods.filterGap")
            ComboBox {
                id: category
                width: (parent.width - parent.spacing) / Constants.n("alias.mods.tabColumns")
                model: [qsTr("All"), "Action", "Arcade", "Board", "Card", "Puzzle", "Strategy", "Example"]
                onCurrentTextChanged: ModsExplore.category = currentIndex === 0 ? "All" : currentText
            }
            ComboBox {
                id: sortMode
                width: (parent.width - parent.spacing) / Constants.n("alias.mods.tabColumns")
                model: [qsTr("Updated"), qsTr("Name"), qsTr("Downloads")]
                onCurrentIndexChanged: ModsExplore.sortMode = currentIndex === 1 ? "name" : (currentIndex === 2 ? "downloads" : "updated")
            }
        }

        ThemeSurface {
            visible: Mods.error.length > 0 || Mods.loading
            width: parent.width
            height: Constants.n("alias.mods.statusHeight")
            radius: Constants.radiusMd
            color: Constants.surfaceRaised
            borderColor: Mods.error.length > 0 ? Constants.danger : Constants.surfaceInteractive
            Text {
                anchors.fill: parent; anchors.margins: Constants.n("alias.mods.statusPadding")
                text: Mods.loading ? qsTr("Refreshing catalog…") : Mods.error
                color: Mods.error.length > 0 ? Constants.danger : Constants.textMuted
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
                font.pixelSize: Constants.n("alias.mods.subtitleFontSize")
            }
        }

        GridView {
            id: exploreGrid
            visible: root.activeTab === 0
            width: parent.width
            height: parent.height - y
            model: ModsExplore
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            cellWidth: width < Constants.n("alias.mods.grid.breakpoint") ? width : width / Constants.n("alias.mods.grid.columnsWide")
            cellHeight: Constants.n("alias.mods.card.cellHeight")
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            delegate: modDelegate

            Column {
                anchors.centerIn: parent
                visible: ModsExplore.count === 0 && !Mods.loading
                spacing: Constants.n("alias.mods.emptyGap")
                AppIcon { width: Constants.n("alias.mods.emptyIconSize"); height: Constants.n("alias.mods.emptyIconSize"); name: "package"; anchors.horizontalCenter: parent.horizontalCenter; opacity: Constants.ratio80 }
                Text { text: Mods.error.length > 0 ? qsTr("Catalog is unavailable.") : qsTr("No catalog mods match this filter."); color: Constants.textMuted; horizontalAlignment: Text.AlignHCenter }
            }
        }

        GridView {
            id: installedGrid
            visible: root.activeTab === 1
            width: parent.width
            height: parent.height - y
            model: ModsInstalled
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            cellWidth: width < Constants.n("alias.mods.grid.breakpoint") ? width : width / Constants.n("alias.mods.grid.columnsWide")
            cellHeight: Constants.n("alias.mods.card.cellHeight")
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            delegate: installedDelegate

            Column {
                anchors.centerIn: parent
                visible: ModsInstalled.count === 0
                spacing: Constants.n("alias.mods.emptyGap")
                AppIcon { width: Constants.n("alias.mods.emptyIconSize"); height: Constants.n("alias.mods.emptyIconSize"); name: "package"; anchors.horizontalCenter: parent.horizontalCenter; opacity: Constants.ratio80 }
                Text { text: qsTr("No downloaded mods are installed."); color: Constants.textMuted }
            }
        }
    }

    Component {
        id: modDelegate
        Item {
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
            width: GridView.view.cellWidth; height: GridView.view.cellHeight
            ModCard {
                anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top; anchors.rightMargin: Constants.n("alias.mods.card.rightMargin"); height: Constants.n("alias.mods.card.height")
                modId: parent.modId; modName: parent.modName; modDescription: parent.modDescription; modAuthor: parent.modAuthor
                modVersion: parent.modVersion; modCategory: parent.modCategory; modIcon: parent.modIcon; modIconUrl: parent.modIconUrl
                modSizeBytes: parent.modSizeBytes; modDownloads: parent.modDownloads; modInstalled: parent.modInstalled; modInstalledVersion: parent.modInstalledVersion
                modState: parent.modState; modProgress: parent.modProgress; modError: parent.modError; modMinAppVersion: parent.modMinAppVersion
                modPublisherStatus: parent.modPublisherStatus; modPublisherVerified: parent.modPublisherVerified; modLicense: parent.modLicense
                modSourceUrl: parent.modSourceUrl; modSourceAvailable: parent.modSourceAvailable; modPluginLevel: parent.modPluginLevel; modReviewed: parent.modReviewed; modNative: parent.modNative
                installedView: false
                onInstallRequested: function(id) { root.installMod(id) }
                onUninstallRequested: function(id) { root.uninstallMod(id) }
                onLaunchRequested: function(id) { root.launchRequested(id) }
            }
        }
    }

    Component {
        id: installedDelegate
        Item {
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
            width: GridView.view.cellWidth; height: GridView.view.cellHeight
            ModCard {
                anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top; anchors.rightMargin: Constants.n("alias.mods.card.rightMargin"); height: Constants.n("alias.mods.card.height")
                modId: parent.modId; modName: parent.modName; modDescription: parent.modDescription; modAuthor: parent.modAuthor
                modVersion: parent.modVersion; modCategory: parent.modCategory; modIcon: parent.modIcon; modIconUrl: parent.modIconUrl
                modSizeBytes: parent.modSizeBytes; modDownloads: parent.modDownloads; modInstalled: parent.modInstalled; modInstalledVersion: parent.modInstalledVersion
                modState: parent.modState; modProgress: parent.modProgress; modError: parent.modError; modMinAppVersion: parent.modMinAppVersion
                modPublisherStatus: parent.modPublisherStatus; modPublisherVerified: parent.modPublisherVerified; modLicense: parent.modLicense
                modSourceUrl: parent.modSourceUrl; modSourceAvailable: parent.modSourceAvailable; modPluginLevel: parent.modPluginLevel; modReviewed: parent.modReviewed; modNative: parent.modNative
                installedView: true
                onInstallRequested: function(id) { root.installMod(id) }
                onUninstallRequested: function(id) { root.uninstallMod(id) }
                onLaunchRequested: function(id) { root.launchRequested(id) }
            }
        }
    }
}
