import QtQuick
Item {
    Component.onCompleted: {
        Settings.soundEnabled = Settings.soundEnabled
        Settings.soundVolume = Settings.soundVolume
        Settings.animationsEnabled = Settings.animationsEnabled
        Settings.setValue("difficulty", Settings.value("difficulty", "normal"))
        Settings.contains("difficulty")
        Lang.setLanguage(Lang.language)
        Lang.text("Legacy v0.6", Lang.language)
        Audio.play("tap")
        Audio.preload(Qt.resolvedUrl("click.wav"))
        Audio.preload(Qt.resolvedUrl("click.wav"), 1.0)
        Audio.playUrl(Qt.resolvedUrl("click.wav"))
        GameAudio.preload(Qt.resolvedUrl("click.wav"), 1.0)
        GameTheme.value("color.background")
        GameTheme.activeThemeId
        App.currentGameId
        Lifecycle.load(); Lifecycle.start(); Lifecycle.save()
    }
}
