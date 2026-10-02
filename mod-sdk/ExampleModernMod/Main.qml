import QtQuick

Item {
    id: root
    property int taps: 0
    property string statusKey: ""
    property string statusFallback: ""
    property string statusDetail: ""
    readonly property string status: statusKey ? tr(statusKey, statusFallback) + (statusDetail ? ": " + statusDetail : "") : ""
    function setStatus(key, fallback, detail) { statusKey = key; statusFallback = fallback; statusDetail = detail || "" }
    readonly property int achievementTapTarget: 10 // gameplay rule, intentionally not a theme metric

    property bool sessionPaused: false
    property bool writable: true
    property int settingsRevision: 0
    function setting(key, fallback) { let revision = settingsRevision; return GameSettings.value(key, fallback) }

    function tr(key, fallback) {
        // An invokable text() call alone is not a live language binding.
        let language = GameI18n.language
        return GameI18n.text(key, fallback)
    }
    function color(token) { let revision = GameTheme.revision; return GameTheme.color(token) }
    function number(token) { let revision = GameTheme.revision; return GameTheme.number(token) }
    function persist() {
        if (!writable) return false
        GameSave.set("taps", taps)
        const ok = GameSave.autosave()
        if (ok) setStatus("saved", "Progress saved")
        else setStatus("save_failed", "Save failed", GameSave.lastError)
        return ok
    }
    function loadProgress() {
        const existing = GameSave.hasStoredSlot("autosave")
        if (!GameSave.load("autosave")) {
            writable = !existing
            if (existing) setStatus("load_failed", "Load failed", GameSave.lastError)
            else setStatus("new", "New session")
            return
        }
        const value = GameSave.get("taps", 0)
        if (typeof value !== "number" || !isFinite(value) || value < 0 || Math.floor(value) !== value || value > 1000000) {
            writable = false
            setStatus("invalid", "Saved progress is invalid; original data retained")
            return
        }
        writable = true
        taps = value
        setStatus("loaded", "Progress loaded")
    }
    function registerTap() {
        if (sessionPaused || !writable) return
        taps += 1
        GameAudio.playEffect(GameResources.url("sfx/click.wav"))
        if (Haptics.available) Haptics.light()
        GameStats.increment("taps")
        if (taps >= achievementTapTarget) Achievements.unlock("ten_taps")
        persist()
    }
    // The host attaches this root, then calls load/start. Do not reattach Lifecycle.
    function load() { loadProgress() }
    function start() { sessionPaused = false }
    function pause() { sessionPaused = true }
    function resume() { sessionPaused = false }
    function save() { persist() }

    Rectangle {
        anchors.fill: parent
        color: root.color("alias.page.background.normal")
    }

    Connections {
        target: GameInput
        function onActionPressed(action, value) { if (action === "fire" && !tapButton.activeFocus && !saveButton.activeFocus && !loadButton.activeFocus) root.registerTap() }
    }

    Connections {
        target: GameSettings
        function onValueChanged(key, value) { root.settingsRevision += 1 }
    }

    Flickable {
        anchors.fill: parent
        contentWidth: width
        contentHeight: Math.max(height, panel.height + 32)
        clip: true
    SdkPanel {
        id: panel
        objectName: "sdkPanel"
        x: (parent.width - width) / 2
        y: Math.max(16, (parent.height - height) / 2)
        width: Math.max(1, Math.min(parent.width - root.number("alias.page.paddingX") * 2,
                        root.number("alias.panel.maxWidth")))
        height: content.implicitHeight + contentPadding * 2

        Column {
            id: content
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: panel.contentPadding
            anchors.rightMargin: panel.contentPadding
            spacing: root.number("alias.panel.gap")

            SdkText {
                width: parent.width
                objectName: "sdkTitle"
                text: root.tr("title", "Modern API Example")
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
                id: tapButton
                objectName: "sdkTapButton"
                text: root.tr("tap", "Tap me")
                primary: true
                enabled: !root.sessionPaused && root.writable
                onActivated: root.registerTap()
            }

            Row {
                width: parent.width
                spacing: root.number("alias.button.gap")
                SdkButton {
                    width: (parent.width - parent.spacing) / 2
                    id: saveButton
                    text: root.tr("save", "Save")
                    enabled: root.writable && !root.sessionPaused
                    onActivated: root.persist()
                }
                SdkButton {
                    width: (parent.width - parent.spacing) / 2
                    id: loadButton
                    text: root.tr("load", "Load")
                    enabled: !root.sessionPaused
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
                objectName: "sdkDifficulty"
                text: root.tr("difficulty", "Difficulty") + ": " + root.setting("difficulty", "normal")
                sizeToken: "metric.font.xs"
                colorToken: "color.textMuted"
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    } // Flickable

    Component.onCompleted: {
        GameAudio.preload(Qt.resolvedUrl("sfx/click.wav"))
        GameSave.registerMigration(1, 2, function(oldSave) {
            if (oldSave.score !== undefined && oldSave.taps === undefined)
                oldSave.taps = oldSave.score
            delete oldSave.score
            return oldSave
        })
    }
}
