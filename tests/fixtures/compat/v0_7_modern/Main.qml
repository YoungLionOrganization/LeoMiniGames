import QtQuick
import "." as Scoped
Item {
    Component.onCompleted: {
        if (!Scoped.Services.ready) throw new Error("Scoped services are missing from the per-engine singleton context")
        if (!Scoped.Services.hostObjectsHidden) throw new Error("Raw host objects escaped into the game engine")
        if (GameRuntime.supports("resources")) GameResources.exists("manifest.json")
    }
}
