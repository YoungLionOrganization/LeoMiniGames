pragma Singleton
import QtQml
QtObject {
    readonly property bool ready: typeof ThemeRuntime !== "undefined" && typeof GameRuntime !== "undefined" && GameRuntime.supports("resources")
    readonly property bool hostObjectsHidden: typeof Developer === "undefined" && typeof Paths === "undefined" && typeof Mods === "undefined"
}
