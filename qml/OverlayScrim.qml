// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

Rectangle {
    id: root
    property bool dismissOnTap: false
    signal dismissed()
    color: Constants.c("color.overlay.scrim")
    opacity: Constants.n("alias.overlay.opacity")
    TapHandler { enabled: root.dismissOnTap; onTapped: root.dismissed() }
}
