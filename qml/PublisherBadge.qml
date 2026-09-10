// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls

Item {
    id: root
    property string status: ""
    property bool verified: false
    readonly property string normalized: status.toLowerCase()
    readonly property string level: normalized.indexOf("official") >= 0 ? "official" : (normalized.indexOf("native") >= 0 || normalized.indexOf("l3") >= 0 ? "verified_native" : (verified ? "verified" : "unverified"))
    readonly property color badgeColor: level === "official" ? Constants.accent : level === "verified_native" ? Constants.success : Constants.info
    readonly property string tooltipText: level === "official" ? qsTr("Official Publisher") : level === "verified_native" ? qsTr("Verified publisher with Native/L3 permission") : level === "verified" ? qsTr("Verified Publisher") : qsTr("Unverified Publisher")
    visible: level !== "unverified"
    implicitWidth: Constants.u18
    implicitHeight: Constants.u18

    AppIcon { anchors.fill: parent; name: "verified"; color: root.badgeColor; secondaryColor: root.badgeColor }
    HoverHandler { id: hover }
    ToolTip.visible: hover.hovered
    ToolTip.text: root.tooltipText
}
