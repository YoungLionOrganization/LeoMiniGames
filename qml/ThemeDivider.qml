// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

Rectangle {
    id: root
    property bool vertical: false
    implicitWidth: vertical ? Constants.separatorBorderWidth : Constants.control2xl
    implicitHeight: vertical ? Constants.control2xl : Constants.separatorBorderWidth
    color: Constants.separatorBorderNormal
    opacity: Constants.separatorOpacity
}
