// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

Item {
    id: root
    default property alias contentData: contentLayer.data

    property string surfaceToken: "surface.card"
    property var spec: Constants.surface(surfaceToken)
    property color color: spec.color !== undefined ? spec.color : Constants.surfaceRaised
    property real radius: safeNumber(spec.radius, Constants.radiusLg, Constants.spaceNone, Constants.n("alias.surface.maxRadius"))
    property color borderColor: spec.border !== undefined ? spec.border : Constants.border
    property real borderWidth: safeNumber(spec.borderWidth, Constants.borderHairline, Constants.spaceNone, Constants.n("alias.surface.maxBorderWidth"))
    property bool themedFill: true
    property bool textureEnabled: true
    readonly property string shapeType: spec.shape !== undefined ? String(spec.shape) : "roundedRect"
    readonly property real cutSize: safeNumber(spec.cutSize, Constants.spaceSm, Constants.spaceNone, Constants.n("alias.surface.maxCutSize"))
    readonly property bool polygonShape: shapeType === "cutCorners" || shapeType === "diamond" || shapeType === "hexagon"
    readonly property string textureSource: spec.texture !== undefined ? String(spec.texture) : ""
    readonly property bool safeTextureSource: textureSource.indexOf("qrc:/") === Constants.u0
    readonly property real safeTextureOpacity: safeNumber(spec.textureOpacity, Constants.ratio0,
                                                          Constants.n("alias.surface.minTextureOpacity"),
                                                          Constants.n("alias.surface.maxTextureOpacity"))

    function safeNumber(value, fallback, minimum, maximum) {
        var candidate = Number(value)
        if (!isFinite(candidate))
            candidate = fallback
        return Math.min(maximum, Math.max(minimum, candidate))
    }

    Rectangle {
        id: base
        anchors.fill: parent
        visible: !root.polygonShape
        color: root.color
        radius: root.shapeType === "ellipse" ? Math.min(width, height) * Constants.ratio50 : root.radius
        border.color: root.borderColor
        border.width: root.borderWidth
        gradient: root.themedFill && root.spec.fillType === "gradient" ? themedGradient : null

        Gradient {
            id: themedGradient
            orientation: root.spec.gradient && root.spec.gradient.orientation === "horizontal" ? Gradient.Horizontal : Gradient.Vertical
            GradientStop { position: Constants.ratio0; color: root.spec.gradient && root.spec.gradient.start !== undefined ? root.spec.gradient.start : root.color }
            GradientStop { position: Constants.ratio50; color: root.spec.gradient && root.spec.gradient.middle !== undefined ? root.spec.gradient.middle : root.color }
            GradientStop { position: Constants.ratio100; color: root.spec.gradient && root.spec.gradient.end !== undefined ? root.spec.gradient.end : root.color }
        }
    }

    ThemePolygonSurface {
        anchors.fill: parent
        visible: root.polygonShape
        shapeType: root.shapeType
        fillColor: root.color
        strokeColor: root.borderColor
        strokeWidth: root.borderWidth
        cutSize: root.cutSize
    }

    Image {
        anchors.fill: parent
        visible: root.themedFill && root.textureEnabled && root.safeTextureSource && root.textureSource.length > Constants.spaceNone
        source: visible ? root.textureSource : ""
        opacity: root.safeTextureOpacity
        fillMode: Image.Tile
        asynchronous: true
        cache: true
    }

    Item {
        id: contentLayer
        anchors.fill: parent
    }
}
