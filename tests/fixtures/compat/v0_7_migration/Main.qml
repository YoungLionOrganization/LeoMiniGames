import QtQuick
Item {
    property bool migrationSucceeded: false
    property bool parentAttached: false
    property int coins: 0
    property int loads: 0
    property int saves: 0
    property int closes: 0
    function load() { loads += 1 }
    function save() { saves += 1; GameSave.set("coins", coins) }
    function close() { closes += 1; App.closeGame() }
    function requestClose() { App.closeGame() }
    Component.onCompleted: {
        parentAttached = parent !== null
        GameSave.registerMigration(1, 2, function(data) {
            data.coins = data.coins + 1
            data.nested = { inventory: ["gem", "coin"] }
            return data
        })
        migrationSucceeded = GameSave.load()
        coins = GameSave.get("coins", 0)
    }
}
