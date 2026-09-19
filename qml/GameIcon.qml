// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

Item {
    id: root
    required property string gameId
    property string iconUrl: ""

    readonly property string iconName: gameId === "xox" ? "xox"
        : gameId === "blackjack" ? "cards"
        : gameId === "minesweeper" ? "mine"
        : gameId === "2048" ? "2048"
        : gameId === "memory_match" ? "memory"
        : gameId === "reaction_tap" ? "reaction"
        : "play"

    ThemeSurface {
        anchors.fill: parent
        radius: Math.max(Constants.n("alias.gameIcon.minimumSize"), Math.min(width, height) * Constants.n("alias.gameIcon.radiusRatio"))
        color: Constants.surfaceInteractive
        borderColor: Constants.borderStrong
        borderWidth: Constants.u1

        Loader {
            id: externalIcon
            anchors.fill: parent
            anchors.margins: Constants.u7
            active: root.iconUrl.length > Constants.spaceNone
            sourceComponent: Component {
                Image {
                    source: root.iconUrl
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    cache: true
                }
            }
        }

        AppIcon {
            anchors.centerIn: parent
            width: Math.max(Constants.n("alias.gameIcon.minimumSize"), parent.width * Constants.n("alias.gameIcon.contentRatio"))
            height: Math.max(Constants.n("alias.gameIcon.minimumSize"), parent.height * Constants.n("alias.gameIcon.contentRatio"))
            name: root.iconName
            visible: !externalIcon.item || externalIcon.item.status !== Image.Ready
        }
    }
}
