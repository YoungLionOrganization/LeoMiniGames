// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls

Button {
    id: control
    property bool soundOnPress: true
    property string iconName: ""
    property url iconSource: ""   // backward-compatible fallback for external callers
    property int iconSize: Math.round(Constants.n("alias.button.iconSize"))
    implicitHeight: Constants.n("alias.button.height")
    padding: Constants.n("alias.button.padding")

    scale: down ? Constants.n("alias.button.pressedScale")
                : hovered ? Constants.n("alias.button.hoverScale")
                          : Constants.scaleNormal
    Behavior on scale {
        NumberAnimation {
            duration: Settings.animationsEnabled ? Constants.buttonAnimationFast : Constants.durationInstant
            easing.type: Constants.easing("alias.motion.press.easing")
        }
    }

    onPressed: if (soundOnPress) Audio.play("click")

    contentItem: Item {
        implicitWidth: contentRow.implicitWidth
        implicitHeight: Math.max(contentRow.implicitHeight, Constants.n("alias.button.contentMinHeight"))

        Row {
            id: contentRow
            anchors.centerIn: parent
            spacing: (control.iconName.length > 0 || control.iconSource.toString().length > 0) && control.text.length > 0
                     ? Constants.n("alias.button.gap") : Constants.spaceNone

            AppIcon {
                visible: control.iconName.length > 0
                width: visible ? control.iconSize : Constants.spaceNone
                height: visible ? control.iconSize : Constants.spaceNone
                name: control.iconName
                color: control.checked ? Constants.buttonAccentPressed : Constants.buttonAccentNormal
            }

            Image {
                visible: control.iconName.length === 0 && control.iconSource.toString().length > 0
                width: visible ? control.iconSize : Constants.spaceNone
                height: visible ? control.iconSize : Constants.spaceNone
                source: visible ? control.iconSource : ""
                sourceSize.width: control.iconSize * Constants.n("alias.image.sourceScale")
                sourceSize.height: control.iconSize * Constants.n("alias.image.sourceScale")
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Text {
                text: control.text
                font: control.font
                color: control.enabled
                       ? (control.checked ? Constants.buttonAccentPressed : Constants.buttonTextNormal)
                       : Constants.buttonTextDisabled
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    background: ThemeSurface {
        surfaceToken: "surface.control"
        radius: Constants.buttonRadius
        color: control.down ? Constants.buttonBackgroundPressed
                            : (control.hovered ? Constants.buttonBackgroundHover : Constants.buttonBackgroundNormal)
        borderWidth: control.checked ? Constants.n("alias.button.checkedBorderWidth") : Constants.buttonBorderWidth
        borderColor: control.checked ? Constants.buttonAccentNormal : Constants.buttonBorderNormal
        opacity: control.enabled ? Constants.buttonOpacity : Constants.buttonDisabledOpacity
        Behavior on color {
            ColorAnimation { duration: Settings.animationsEnabled ? Constants.buttonAnimationFast : Constants.durationInstant }
        }
    }
}
