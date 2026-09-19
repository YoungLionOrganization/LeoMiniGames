// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick
import QtQuick.Shapes

Item {
    id: root
    property string shapeType: "cutCorners"
    property color fillColor: Constants.surfaceRaised
    property color strokeColor: Constants.border
    property real strokeWidth: Constants.borderHairline
    property real cutSize: Constants.spaceSm
    readonly property real cut: Math.min(Math.max(Constants.spaceNone, cutSize), Math.min(width, height) * Constants.ratio50)

    Loader {
        anchors.fill: parent
        sourceComponent: root.shapeType === "diamond" ? diamondShape
                       : root.shapeType === "hexagon" ? hexagonShape
                       : cutCornerShape
    }

    Component {
        id: cutCornerShape
        Shape {
            ShapePath {
                strokeColor: root.strokeColor
                strokeWidth: root.strokeWidth
                fillColor: root.fillColor
                joinStyle: ShapePath.RoundJoin
                startX: root.cut; startY: Constants.spaceNone
                PathLine { x: root.width - root.cut; y: Constants.spaceNone }
                PathLine { x: root.width; y: root.cut }
                PathLine { x: root.width; y: root.height - root.cut }
                PathLine { x: root.width - root.cut; y: root.height }
                PathLine { x: root.cut; y: root.height }
                PathLine { x: Constants.spaceNone; y: root.height - root.cut }
                PathLine { x: Constants.spaceNone; y: root.cut }
                PathLine { x: root.cut; y: Constants.spaceNone }
            }
        }
    }

    Component {
        id: diamondShape
        Shape {
            ShapePath {
                strokeColor: root.strokeColor
                strokeWidth: root.strokeWidth
                fillColor: root.fillColor
                joinStyle: ShapePath.RoundJoin
                startX: root.width * Constants.ratio50; startY: Constants.spaceNone
                PathLine { x: root.width; y: root.height * Constants.ratio50 }
                PathLine { x: root.width * Constants.ratio50; y: root.height }
                PathLine { x: Constants.spaceNone; y: root.height * Constants.ratio50 }
                PathLine { x: root.width * Constants.ratio50; y: Constants.spaceNone }
            }
        }
    }

    Component {
        id: hexagonShape
        Shape {
            ShapePath {
                strokeColor: root.strokeColor
                strokeWidth: root.strokeWidth
                fillColor: root.fillColor
                joinStyle: ShapePath.RoundJoin
                startX: root.width * Constants.ratio25; startY: Constants.spaceNone
                PathLine { x: root.width * Constants.ratio75; y: Constants.spaceNone }
                PathLine { x: root.width; y: root.height * Constants.ratio50 }
                PathLine { x: root.width * Constants.ratio75; y: root.height }
                PathLine { x: root.width * Constants.ratio25; y: root.height }
                PathLine { x: Constants.spaceNone; y: root.height * Constants.ratio50 }
                PathLine { x: root.width * Constants.ratio25; y: Constants.spaceNone }
            }
        }
    }
}
