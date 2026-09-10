// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

Rectangle {
    id: root
    property bool vertical: false
    implicitWidth: vertical ? Constants.separatorBorderWidth : Constants.control2xl
    implicitHeight: vertical ? Constants.control2xl : Constants.separatorBorderWidth
    color: Constants.separatorBorderNormal
    opacity: Constants.separatorOpacity
}
