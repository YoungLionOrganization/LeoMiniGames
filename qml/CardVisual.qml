// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

ThemeSurface {
    id: root
    property string label: ""
    width: Constants.n("alias.playingCard.width")
    height: Constants.n("alias.playingCard.height")
    radius: Constants.n("alias.playingCard.radius")
    color: Constants.playingCardBackground
    borderColor: Constants.primary
    borderWidth: Constants.borderHairline

    readonly property string suitCode: label.indexOf("♥") >= 0 ? "heart"
                                       : label.indexOf("♦") >= 0 ? "diamond"
                                       : label.indexOf("♣") >= 0 ? "club"
                                       : label.indexOf("♠") >= 0 ? "spade" : ""
    readonly property bool redSuit: suitCode === "heart" || suitCode === "diamond"
    readonly property string rankText: suitCode.length === 0 ? label
                                      : label.replace("♥", "").replace("♦", "").replace("♣", "").replace("♠", "")
    readonly property color suitColor: redSuit ? Constants.playingCardRedSuit : Constants.playingCardBlackSuit

    scale: Constants.n("alias.playingCard.enterScale")
    opacity: Constants.n("alias.playingCard.hiddenOpacity")

    Component.onCompleted: {
        if (Settings.animationsEnabled)
            entrance.start()
        else {
            scale = Constants.scaleNormal
            opacity = Constants.opacityFull
        }
    }

    ParallelAnimation {
        id: entrance
        NumberAnimation { target: root; property: "scale"; to: Constants.scaleNormal; duration: Constants.motionEnterDuration; easing.type: Constants.easing("alias.motion.enter.easing") }
        NumberAnimation { target: root; property: "opacity"; to: Constants.opacityFull; duration: Constants.motionFastDuration }
    }

    Text {
        id: rank
        anchors.centerIn: parent
        anchors.verticalCenterOffset: root.suitCode.length > 0 ? -root.height * 0.12 : 0
        text: root.rankText
        color: root.label === "??" ? Constants.surfaceInteractive : root.suitColor
        font.pixelSize: root.label === "??" ? Constants.n("alias.playingCard.backFontSize") : Constants.n("alias.playingCard.frontFontSize") * 0.76
        font.weight: Constants.typographyGameTileWeight
    }

    Canvas {
        id: suitCanvas
        visible: root.suitCode.length > 0 && root.label !== "??"
        width: Math.max(14, root.width * 0.25)
        height: width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: rank.bottom
        anchors.topMargin: Math.max(1, root.height * 0.03)
        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            ctx.fillStyle = root.suitColor
            const w = width, h = height, cx = w / 2, cy = h / 2
            if (root.suitCode === "diamond") {
                ctx.beginPath(); ctx.moveTo(cx, 0); ctx.lineTo(w, cy); ctx.lineTo(cx, h); ctx.lineTo(0, cy); ctx.closePath(); ctx.fill()
            } else if (root.suitCode === "heart") {
                ctx.beginPath(); ctx.moveTo(cx, h * 0.92)
                ctx.bezierCurveTo(w * 0.10, h * 0.62, 0, h * 0.36, w * 0.12, h * 0.18)
                ctx.bezierCurveTo(w * 0.28, -h * 0.03, cx, h * 0.08, cx, h * 0.27)
                ctx.bezierCurveTo(cx, h * 0.08, w * 0.72, -h * 0.03, w * 0.88, h * 0.18)
                ctx.bezierCurveTo(w, h * 0.36, w * 0.90, h * 0.62, cx, h * 0.92); ctx.closePath(); ctx.fill()
            } else if (root.suitCode === "club") {
                const r = w * 0.22
                ctx.beginPath(); ctx.arc(cx, h * 0.25, r, 0, Math.PI * 2); ctx.arc(w * 0.31, h * 0.51, r, 0, Math.PI * 2); ctx.arc(w * 0.69, h * 0.51, r, 0, Math.PI * 2); ctx.fill()
                ctx.fillRect(w * 0.44, h * 0.50, w * 0.12, h * 0.37)
                ctx.beginPath(); ctx.moveTo(w * 0.31, h * 0.90); ctx.lineTo(w * 0.69, h * 0.90); ctx.lineTo(w * 0.56, h * 0.72); ctx.lineTo(w * 0.44, h * 0.72); ctx.closePath(); ctx.fill()
            } else if (root.suitCode === "spade") {
                ctx.beginPath(); ctx.moveTo(cx, h * 0.05)
                ctx.bezierCurveTo(w * 0.42, h * 0.22, w * 0.10, h * 0.43, w * 0.14, h * 0.62)
                ctx.bezierCurveTo(w * 0.18, h * 0.83, w * 0.43, h * 0.77, cx, h * 0.62)
                ctx.bezierCurveTo(w * 0.57, h * 0.77, w * 0.82, h * 0.83, w * 0.86, h * 0.62)
                ctx.bezierCurveTo(w * 0.90, h * 0.43, w * 0.58, h * 0.22, cx, h * 0.05); ctx.closePath(); ctx.fill()
                ctx.fillRect(w * 0.45, h * 0.59, w * 0.10, h * 0.27)
                ctx.beginPath(); ctx.moveTo(w * 0.31, h * 0.91); ctx.lineTo(w * 0.69, h * 0.91); ctx.lineTo(w * 0.55, h * 0.76); ctx.lineTo(w * 0.45, h * 0.76); ctx.closePath(); ctx.fill()
            }
        }
        Connections { target: root; function onSuitColorChanged() { suitCanvas.requestPaint() } function onSuitCodeChanged() { suitCanvas.requestPaint() } }
    }
}
