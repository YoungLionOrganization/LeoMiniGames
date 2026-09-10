import QtQuick

Item {
    id: root
    property int taps: 0
    property string status: ""
    readonly property int achievementTapTarget: 10 // gameplay rule, intentionally not a theme metric

    function persist() {
        GameSave.set("taps", taps)
        if (GameSave.autosave())
            status = GameI18n.text("saved", "Progress saved")
    }

    function loadProgress() {
        if (GameSave.load("autosave")) {
            taps = GameSave.get("taps", 0)
            status = GameI18n.text("loaded", "Progress loaded")
        }
    }

    Rectangle {
        anchors.fill: parent
        color: GameTheme.color("alias.page.background.normal")
    }

    Connections {
        target: GameInput
        function onActionPressed(action, value) {
            if (action === "fire") {
                root.taps += 1
                GameAudio.playEffect(Qt.resolvedUrl("sfx/click.wav"))
            }
        }
    }

    Connections {
        target: Lifecycle
        function onSaveRequested() { root.persist() }
    }

    SdkPanel {
        id: panel
        anchors.centerIn: parent
        width: Math.min(Viewport.safeWidth - GameTheme.number("alias.page.paddingX") * 2,
                        GameTheme.number("alias.panel.maxWidth"))
        height: content.implicitHeight + contentPadding * 2

        Column {
            id: content
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: panel.contentPadding
            anchors.rightMargin: panel.contentPadding
            spacing: GameTheme.number("alias.panel.gap")

            SdkText {
                width: parent.width
                text: GameI18n.text("title", "Modern API Example")
                sizeToken: "metric.font.2xl"
                horizontalAlignment: Text.AlignHCenter
                font.bold: true
            }

            SdkText {
                width: parent.width
                text: root.taps
                sizeToken: "metric.font.3xl"
                colorToken: "color.accent"
                horizontalAlignment: Text.AlignHCenter
                font.bold: true
            }

            SdkButton {
                width: parent.width
                text: GameI18n.text("tap", "Tap me")
                primary: true
                onActivated: {
                    root.taps += 1
                    Haptics.light()
                    GameAudio.playEffect(Qt.resolvedUrl("sfx/click.wav"))
                    GameStats.increment("taps")
                    if (root.taps >= root.achievementTapTarget)
                        Achievements.unlock("ten_taps")
                }
            }

            Row {
                width: parent.width
                spacing: GameTheme.number("alias.button.gap")
                SdkButton {
                    width: (parent.width - parent.spacing) / 2
                    text: GameI18n.text("save", "Save")
                    onActivated: root.persist()
                }
                SdkButton {
                    width: (parent.width - parent.spacing) / 2
                    text: GameI18n.text("load", "Load")
                    onActivated: root.loadProgress()
                }
            }

            SdkText {
                width: parent.width
                text: root.status
                sizeToken: "metric.font.sm"
                colorToken: "color.textMuted"
                horizontalAlignment: Text.AlignHCenter
            }
            SdkText {
                width: parent.width
                text: GameI18n.text("difficulty", "Difficulty") + ": " + GameSettings.value("difficulty", "normal")
                sizeToken: "metric.font.xs"
                colorToken: "color.textMuted"
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    Component.onCompleted: {
        GameAudio.preload(Qt.resolvedUrl("sfx/click.wav"))
        GameSave.registerMigration(1, 2, function(oldSave) {
            if (oldSave.score !== undefined && oldSave.taps === undefined)
                oldSave.taps = oldSave.score
            delete oldSave.score
            return oldSave
        })
        loadProgress()
    }
}
