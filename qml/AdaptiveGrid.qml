// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
import QtQuick

Grid {
    id: root
    property real minimumCellWidth: Constants.cardMd
    property int maximumColumns: Math.round(Constants.n("alias.grid.maxColumns"))
    property real horizontalGap: Constants.gridGap
    property real verticalGap: Constants.gridGap
    readonly property real availableWidth: Math.max(Constants.ratio100, width)
    columns: Math.max(Math.round(Constants.ratio100), Math.min(maximumColumns, Math.floor((availableWidth + horizontalGap) / Math.max(Constants.ratio100, minimumCellWidth + horizontalGap))))
    columnSpacing: horizontalGap
    rowSpacing: verticalGap
    readonly property real cellWidth: Math.max(Constants.ratio100, (availableWidth - Math.max(Constants.spaceNone, columns - Constants.ratio100) * horizontalGap) / columns)
}
