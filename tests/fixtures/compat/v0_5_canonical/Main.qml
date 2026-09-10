import QtQuick
Item {
    Component.onCompleted: {
        var previous = Settings.value("difficulty", "normal")
        Settings.setValue("difficulty", previous)
        Audio.play("tap")
        if (Lang.availableLanguages.length > 0) Lang.text("Legacy", Lang.language)
        if (GameTheme.hasValue("color.background")) GameTheme.color("color.background")
        GameSave.get("slot", {})
    }
}
