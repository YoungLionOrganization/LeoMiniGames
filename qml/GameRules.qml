// SPDX-License-Identifier: GPL-3.0-or-later
pragma Singleton
import QtQuick

// Immutable gameplay constants. Themes never resolve or override these values.
// Presentation belongs in Constants/ThemeRuntime; rules belong here or in the
// game's C++ state model.
QtObject {
    readonly property int xoxColumns: 3
    readonly property int xoxCellCount: 9

    readonly property int memoryColumns: 4
    readonly property int memoryPairCount: 8

    readonly property int twentyFortyEightColumns: 4
    readonly property int twentyFortyEightEmpty: 0
    readonly property int twentyFortyEightBronzeLowMax: 4
    readonly property int twentyFortyEightBronzeHighMax: 16
    readonly property int twentyFortyEightGoldLowMax: 64
    readonly property int twentyFortyEightGoldHighMax: 256
    readonly property int twentyFortyEightDarkTextFrom: 128
    readonly property int twentyFortyEightSmallFontFrom: 1024

    readonly property int minesweeperEasy: 0
    readonly property int minesweeperMedium: 1
    readonly property int minesweeperHard: 2
    readonly property int minesweeperNeighborLow: 1
    readonly property int minesweeperNeighborMedium: 2
    readonly property int minesweeperNeighborDanger: 3

    readonly property int trustedNativePluginLevel: 3
}
