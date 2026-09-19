// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

PressableSurface {
    role: "gameTile"
    surfaceToken: "surface.gameTile"
    implicitWidth: Constants.gameInventorySlot
    implicitHeight: Constants.gameInventorySlot
    radius: Constants.gameTileRadius
}
