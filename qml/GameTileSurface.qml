// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

PressableSurface {
    role: "gameTile"
    surfaceToken: "surface.gameTile"
    implicitWidth: Constants.gameInventorySlot
    implicitHeight: Constants.gameInventorySlot
    radius: Constants.gameTileRadius
}
