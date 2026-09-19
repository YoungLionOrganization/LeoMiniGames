// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick
import QtQuick.Controls

Item {
    id: root
    signal gameRequested(string id)
    signal settingsRequested()
    signal modsRequested()
    signal themesRequested()

    AppBackground { anchors.fill: parent }

    Column {
        anchors.fill: parent
        anchors.margins: Constants.n("alias.library.pagePadding")
        spacing: Constants.n("alias.library.pageGap")

        Row {
            width: parent.width
            spacing: Constants.n("alias.library.headerGap")

            BrandLogo { width: Constants.n("alias.library.logoSize"); height: Constants.n("alias.library.logoSize") }

            Column {
                width: parent.width - Constants.n("alias.library.headerReservedWidth")
                anchors.verticalCenter: parent.verticalCenter
                spacing: Constants.n("alias.library.titleGap")
                Text { text: "LeoMiniGames"; color: Constants.text; font.pixelSize: Constants.n("alias.library.titleFontSize"); font.bold: true }
                Text {
                    width: parent.width
                    text: qsTr("%1 games available").arg(Games.totalCount)
                    color: Constants.primaryHover
                    font.pixelSize: Constants.n("alias.library.subtitleFontSize")
                    elide: Text.ElideRight
                }
            }

            BronzeButton { text: ""; iconName: "mods"; iconSize: Constants.n("alias.library.headerButtonIconSize"); width: Constants.n("alias.library.headerButtonSize"); height: Constants.n("alias.library.headerButtonSize"); Accessible.name: qsTr("Mods"); anchors.verticalCenter: parent.verticalCenter; onClicked: root.modsRequested() }
            BronzeButton { text: ""; iconName: "theme"; iconSize: Constants.n("alias.library.headerButtonIconSize"); width: Constants.n("alias.library.headerButtonSize"); height: Constants.n("alias.library.headerButtonSize"); Accessible.name: qsTr("Themes"); anchors.verticalCenter: parent.verticalCenter; onClicked: root.themesRequested() }
            BronzeButton { text: ""; iconName: "settings"; iconSize: Constants.n("alias.library.headerButtonIconSize"); width: Constants.n("alias.library.headerButtonSize"); height: Constants.n("alias.library.headerButtonSize"); Accessible.name: qsTr("Settings"); anchors.verticalCenter: parent.verticalCenter; onClicked: root.settingsRequested() }
        }

        TextField {
            id: search
            width: parent.width
            height: Constants.n("alias.library.searchHeight")
            placeholderText: qsTr("Search games, categories, tags or publishers…")
            color: Constants.text
            placeholderTextColor: Constants.textSubtle
            selectByMouse: true
            leftPadding: Constants.n("alias.library.searchPaddingX")
            rightPadding: Constants.n("alias.library.searchPaddingX")
            background: ThemeSurface { radius: Constants.radiusMd; color: Constants.surfaceRaised; borderColor: search.activeFocus ? Constants.primary : Constants.surfaceInteractive }
            onTextChanged: Games.query = text
        }

        ThemeSurface {
            visible: Games.count === Constants.spaceNone
            width: parent.width
            height: Constants.n("alias.library.emptyHeight")
            radius: Constants.radiusMd
            color: Constants.surfaceRaised
            borderColor: Constants.surfaceInteractive
            Text { anchors.centerIn: parent; text: qsTr("No games match this search."); color: Constants.textMuted }
        }

        GridView {
            id: grid
            width: parent.width
            height: parent.height - y
            clip: true
            model: Games
            cellWidth: width < Constants.n("alias.library.grid.mediumBreakpoint") ? width : (width < Constants.n("alias.library.grid.largeBreakpoint") ? width / Constants.u2 : width / Constants.u3)
            cellHeight: Constants.n("alias.library.grid.cellHeight")
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

            delegate: Item {
                required property string gameId
                required property string gameName
                required property string gameDescription
                required property string gameCategory
                required property string gameVersion
                required property string gamePublisher
                required property string gameIcon
                required property string gameIconUrl
                required property bool builtIn
                required property string gameSource
                required property string publisherStatus
                required property bool publisherVerified
                required property int pluginLevel
                width: grid.cellWidth
                height: grid.cellHeight

                GameCard {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.rightMargin: Constants.n("alias.library.grid.cardRightMargin")
                    height: Constants.n("alias.library.grid.cardHeight")
                    gameId: parent.gameId
                    gameName: parent.gameName
                    gameDescription: parent.gameDescription
                    gameCategory: parent.gameCategory
                    gameVersion: parent.gameVersion
                    gamePublisher: parent.gamePublisher
                    gameIcon: parent.gameIcon
                    gameIconUrl: parent.gameIconUrl
                    builtIn: parent.builtIn
                    gameSource: parent.gameSource
                    publisherStatus: parent.publisherStatus
                    publisherVerified: parent.publisherVerified
                    pluginLevel: parent.pluginLevel
                    onPlayRequested: function(id) { root.gameRequested(id) }
                }
            }
        }
    }
}
