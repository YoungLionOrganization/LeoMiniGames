import QtQuick
import QtQuick.Controls
Item {
    id: root
    property bool sessionPaused: false
    property bool writable: true
    function themeColor(token) { let revision = GameTheme.revision; return GameTheme.color(token) }
    function load() {
        if (!GameSave.load()) { writable = !GameSave.hasStoredSlot("autosave"); return }
        const count = GameSave.get("count",0)
        writable = typeof count === "number" && count >= 0 && count <= 1000000 && Math.floor(count) === count
        if (writable) App.currentGame.restore(count)
    }
    function save() {
        if (!writable) return
        GameSave.set("count",App.currentGame.count)
        if (!GameSave.autosave()) GameLogger.log("error",GameSave.lastError,"Main.qml")
    }
    function pause() { sessionPaused=true }
    function resume() { sessionPaused=false }
    Column {
        anchors.centerIn: parent
        Text { color: root.themeColor("color.text"); text: App.currentGame ? App.currentGame.count : 0 }
        Button { text: "Increment"; enabled: !root.sessionPaused && root.writable; onClicked: App.currentGame.increment() }
    }
}
