// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

Rectangle {
    id: root
    property bool dismissOnTap: false
    signal dismissed()
    color: Constants.c("color.overlay.scrim")
    opacity: Constants.n("alias.overlay.opacity")
    TapHandler { enabled: root.dismissOnTap; onTapped: root.dismissed() }
}
