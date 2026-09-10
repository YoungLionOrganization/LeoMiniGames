// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

Item {
    id: root
    property string name: "package"
    property color color: Constants.accent
    property color secondaryColor: Constants.primaryHover
    property real strokeScale: 1.0

    onNameChanged: canvas.requestPaint()
    onColorChanged: canvas.requestPaint()
    onSecondaryColorChanged: canvas.requestPaint()
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    Canvas {
        id: canvas
        anchors.fill: parent
        renderTarget: Canvas.Image
        antialiasing: true

        function circle(ctx, x, y, r, fill) {
            ctx.beginPath(); ctx.arc(x, y, r, 0, Math.PI * 2); ctx.closePath()
            if (fill) ctx.fill(); else ctx.stroke()
        }

        function line(ctx, x1, y1, x2, y2) {
            ctx.beginPath(); ctx.moveTo(x1, y1); ctx.lineTo(x2, y2); ctx.stroke()
        }

        onPaint: {
            const ctx = getContext("2d")
            const w = width
            const h = height
            const s = Math.min(w, h)
            const x0 = (w - s) / 2
            const y0 = (h - s) / 2
            function X(v) { return x0 + v * s }
            function Y(v) { return y0 + v * s }

            ctx.clearRect(0, 0, w, h)
            ctx.strokeStyle = root.color
            ctx.fillStyle = root.color
            ctx.lineWidth = Math.max(1.6, s * 0.075 * root.strokeScale)
            ctx.lineCap = "round"
            ctx.lineJoin = "round"

            switch (root.name) {
            case "back":
                line(ctx, X(.72), Y(.18), X(.30), Y(.50))
                line(ctx, X(.30), Y(.50), X(.72), Y(.82))
                break
            case "play":
            case "open":
                ctx.beginPath(); ctx.moveTo(X(.32), Y(.20)); ctx.lineTo(X(.78), Y(.50)); ctx.lineTo(X(.32), Y(.80)); ctx.closePath(); ctx.fill()
                break
            case "refresh":
                ctx.beginPath(); ctx.arc(X(.50), Y(.52), s*.27, Math.PI*.20, Math.PI*1.72); ctx.stroke()
                ctx.beginPath(); ctx.moveTo(X(.65),Y(.18)); ctx.lineTo(X(.82),Y(.22)); ctx.lineTo(X(.75),Y(.38)); ctx.closePath(); ctx.fill()
                break
            case "download":
                line(ctx, X(.50), Y(.14), X(.50), Y(.62))
                line(ctx, X(.29), Y(.45), X(.50), Y(.66))
                line(ctx, X(.71), Y(.45), X(.50), Y(.66))
                line(ctx, X(.20), Y(.83), X(.80), Y(.83))
                break
            case "remove":
                line(ctx, X(.20), Y(.25), X(.80), Y(.25))
                ctx.strokeRect(X(.29), Y(.32), s*.42, s*.48)
                line(ctx, X(.40), Y(.18), X(.60), Y(.18))
                break
            case "flag":
                line(ctx, X(.31), Y(.16), X(.31), Y(.84))
                ctx.beginPath(); ctx.moveTo(X(.34),Y(.20)); ctx.lineTo(X(.76),Y(.31)); ctx.lineTo(X(.34),Y(.45)); ctx.closePath(); ctx.fill()
                line(ctx, X(.20), Y(.84), X(.50), Y(.84))
                break
            case "mine":
                circle(ctx, X(.50), Y(.50), s*.21, true)
                for (let i=0;i<8;i++) {
                    const a=i*Math.PI/4
                    line(ctx, X(.50)+Math.cos(a)*s*.28, Y(.50)+Math.sin(a)*s*.28,
                              X(.50)+Math.cos(a)*s*.40, Y(.50)+Math.sin(a)*s*.40)
                }
                ctx.fillStyle = root.secondaryColor
                circle(ctx, X(.43), Y(.42), s*.045, true)
                break
            case "settings":
                circle(ctx, X(.50), Y(.50), s*.17, false)
                for (let i=0;i<8;i++) {
                    const a=i*Math.PI/4
                    line(ctx, X(.50)+Math.cos(a)*s*.28, Y(.50)+Math.sin(a)*s*.28,
                              X(.50)+Math.cos(a)*s*.40, Y(.50)+Math.sin(a)*s*.40)
                }
                circle(ctx, X(.50), Y(.50), s*.055, true)
                break
            case "mods":
                ctx.strokeRect(X(.16), Y(.18), s*.28, s*.28)
                ctx.strokeRect(X(.56), Y(.18), s*.28, s*.28)
                ctx.strokeRect(X(.16), Y(.58), s*.28, s*.24)
                ctx.strokeRect(X(.56), Y(.58), s*.28, s*.24)
                line(ctx, X(.44),Y(.32),X(.56),Y(.32)); line(ctx,X(.30),Y(.46),X(.30),Y(.58)); line(ctx,X(.70),Y(.46),X(.70),Y(.58))
                break
            case "theme":
                circle(ctx, X(.50), Y(.50), s*.31, false)
                ctx.fillStyle = root.secondaryColor
                circle(ctx, X(.37), Y(.37), s*.055, true)
                circle(ctx, X(.59), Y(.32), s*.050, true)
                circle(ctx, X(.69), Y(.51), s*.047, true)
                circle(ctx, X(.50), Y(.70), s*.050, true)
                ctx.fillStyle = root.color
                circle(ctx, X(.30), Y(.58), s*.047, true)
                break
            case "package":
                ctx.beginPath(); ctx.moveTo(X(.50),Y(.13)); ctx.lineTo(X(.82),Y(.30)); ctx.lineTo(X(.50),Y(.48)); ctx.lineTo(X(.18),Y(.30)); ctx.closePath(); ctx.stroke()
                line(ctx,X(.18),Y(.30),X(.18),Y(.67)); line(ctx,X(.82),Y(.30),X(.82),Y(.67))
                line(ctx,X(.18),Y(.67),X(.50),Y(.86)); line(ctx,X(.82),Y(.67),X(.50),Y(.86)); line(ctx,X(.50),Y(.48),X(.50),Y(.86))
                break
            case "cards":
                ctx.save(); ctx.translate(X(.43),Y(.49)); ctx.rotate(-.16); ctx.strokeRect(-s*.22,-s*.30,s*.44,s*.60); ctx.restore()
                ctx.save(); ctx.translate(X(.59),Y(.52)); ctx.rotate(.14); ctx.strokeRect(-s*.22,-s*.30,s*.44,s*.60); ctx.restore()
                break
            case "memory":
                ctx.strokeRect(X(.17),Y(.18),s*.27,s*.27); ctx.strokeRect(X(.56),Y(.18),s*.27,s*.27)
                ctx.strokeRect(X(.17),Y(.57),s*.27,s*.27); ctx.strokeRect(X(.56),Y(.57),s*.27,s*.27)
                ctx.fillStyle=root.secondaryColor; circle(ctx,X(.305),Y(.315),s*.055,true); circle(ctx,X(.695),Y(.705),s*.055,true)
                break
            case "reaction":
                ctx.beginPath(); ctx.moveTo(X(.58),Y(.10)); ctx.lineTo(X(.26),Y(.55)); ctx.lineTo(X(.48),Y(.55)); ctx.lineTo(X(.40),Y(.90)); ctx.lineTo(X(.76),Y(.43)); ctx.lineTo(X(.53),Y(.43)); ctx.closePath(); ctx.fill()
                break
            case "xox":
                line(ctx,X(.18),Y(.22),X(.43),Y(.47)); line(ctx,X(.43),Y(.22),X(.18),Y(.47)); circle(ctx,X(.69),Y(.34),s*.14,false)
                line(ctx,X(.18),Y(.66),X(.43),Y(.91)); line(ctx,X(.43),Y(.66),X(.18),Y(.91)); circle(ctx,X(.69),Y(.78),s*.14,false)
                break
            case "2048":
                ctx.font = "bold " + Math.max(9, Math.round(s*.25)) + "px sans-serif"
                ctx.textAlign = "center"; ctx.textBaseline = "middle"; ctx.fillText("2048", X(.50), Y(.52))
                break
            case "verified":
                circle(ctx, X(.50), Y(.50), s*.36, false)
                line(ctx, X(.30), Y(.51), X(.44), Y(.66))
                line(ctx, X(.44), Y(.66), X(.72), Y(.34))
                break
            default:
                circle(ctx,X(.50),Y(.50),s*.30,false); circle(ctx,X(.50),Y(.50),s*.08,true)
                break
            }
        }
    }
}
