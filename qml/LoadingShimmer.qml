// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

Item {
    id: root
    property bool running: true
    property string role: "skeleton"
    clip: true
    implicitHeight: Constants.n("alias.skeleton.height")

    ThemeSurface {
        anchors.fill: parent
        surfaceToken: "surface.control"
        color: Constants.c("alias.skeleton.background.normal")
        radius: Constants.n("alias.skeleton.radius")
    }
    Rectangle {
        id: band
        height: parent.height
        width: parent.width * Constants.n("alias.loadingShimmer.bandWidth")
        x: -width + parent.width * phase
        color: Constants.c("alias.skeleton.accent.normal")
        opacity: Constants.n("alias.loadingShimmer.maxOpacity")
        property real phase: Constants.ratio0
        NumberAnimation on phase {
            running: root.running && Settings.animationsEnabled
            loops: Animation.Infinite
            from: Constants.ratio0
            to: Constants.ratio100
            duration: Constants.n("alias.skeleton.animationSlow")
            easing.type: Constants.easing("alias.motion.standard.easing")
        }
    }
}
