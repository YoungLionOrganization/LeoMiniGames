# Audio API (v0.7.3)

The launcher owns audio playback and platform backends. A game uses the same call on Android, Windows, Linux, macOS and Apple mobile; it must not select `QSoundEffect`, `MediaPlayer`, a backend or an OS-specific resource path itself:

```qml
GameAudio.playEffect(Qt.resolvedUrl("sfx/click.wav"))
GameAudio.playEffect(Qt.resolvedUrl("sfx/hit.ogg"), 0.8, "sfx")
GameAudio.playMusic(Qt.resolvedUrl("music/theme.ogg"), true, "music")
```

`playEffect(url[, gain[, group]])`, `preload(url[, legacyGain])`, `playMusic(url[, loop[, group]])`, `stopMusic()`, `pauseAll()`, `resumeAll()`, `volume(group)` and `setVolume(group,value)` remain API 0.7. Groups are `sfx`, `music`, `ui` and `ambient`, case insensitive. Gain is clamped to 0–2; group/master volumes to 0–1. Master, group and effect gain are composed and retained when settings change. Preload prepares an effect without playing; its historical gain argument is retained for call compatibility, with playback gain supplied to `playEffect`.

## Host playback and formats

The launcher copies RCC streams to bounded, temporary local files, keeping them alive until their decoder is released. Native/FFmpeg backends can therefore read the same packaged resources without requiring game-specific Android or desktop fixes. Effects use the low latency `QSoundEffect` path for WAV. If that decoder rejects the file, the same pending request falls back to `QMediaPlayer`. Other formats and music use the media backend directly. A short **16-bit PCM WAV, mono/stereo, 44.1 or 48 kHz** remains the recommended portable effect format. Ogg/MP3/FLAC/etc. require a codec supported by the shipped Qt backend; arbitrary formats are not guaranteed, and this layer does not transcode unsupported codecs.

Normal builds require Qt Multimedia. Explicit testing/headless builds can opt out with `-DLEOMINIGAMES_ENABLE_AUDIO=OFF`; they report `available=false`, empty playback capabilities and an error instead of silently promising audio. `available` describes compiled support, not proof that a physical output device is connected or audible. `capabilities()` additionally identifies music/groups/pause/errors/media fallback when supported. Desktop packaging and Android ABI validation reject a payload that omits the media backend plugin.

Errors from effects and music are emitted through `GameAudio.audioError(message, source)` and recorded in host diagnostics. A decoder failure does not crash gameplay:

```qml
Connections {
    target: GameAudio
    function onAudioError(message, source) { console.warn(message + " " + source) }
}
```

The effect cache contains at most 32 decoders/files, including named effects. Replaying the same effect restarts that clip, matching legacy behavior. RCC effect resources are limited to 32 MiB each, music to 128 MiB. Package close/unload releases its effects/music/files; audio is stopped before resources disappear. Background/pause stops effects, rejects background effect calls and retains music position/request for foreground resume. Effects are not replayed on resume. Muting cancels pending effects.

## Legacy compatibility

`Audio.play(name)`, `Audio.playUrl(url[,gain])`, `Audio.preload(url[,gain])`, and `Audio.stopAll()` remain callable and use the same host effect path. Named sounds such as `click`, `move` and `win` are supported by both APIs; a legacy named `playUrl` gain is preserved. Existing packages can keep their calls. New code should prefer `GameAudio` to access music, groups and error signals.

External URL playback remains restricted to the game's own `qrc:/mods/<id>/...` resources. Relative package files are resolved by the host; arbitrary `file:` and network playback are not granted to external games. `Qt.resolvedUrl` is recommended to make the QML source's relative directory explicit. Named host effects and paths are separate concepts; an unknown named key such as `game/set/move` is not a WAV URL.

## Platform acceptance and future work

The automated host test exercises modern/legacy calls, gain/group/master changes, mute/background, loading/resume, unload/cache limits and actual PCM decoding after a forced effect-decoder fallback. This is not a physical speaker test. Release QA must check PCM effects, compressed effects and looping music on each platform, including pause/resume, mute, Bluetooth/headphone routing and OS audio interruptions. Android needs fresh APK install/upgrade evidence; Linux needs both X11/Wayland and deployed backend dependencies.

Not yet provided: simultaneous voices of the same effect (a bounded voice pool), user-selectable output device, explicit mobile audio focus/ducking policy, or universal transcoding of unsupported formats. Keep these as future host features, rather than adding platform branches to games.
