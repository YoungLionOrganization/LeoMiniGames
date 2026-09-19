// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

Item {
    id: root
    signal tapped()
    signal longPressed()
    property real minimumSize: Constants.touchTarget
    implicitWidth: minimumSize
    implicitHeight: minimumSize
    TapHandler {
        enabled: root.enabled
        onTapped: root.tapped()
        onLongPressed: root.longPressed()
    }
}
