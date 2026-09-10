# Audio API

Qt Multimedia is capability-probed rather than mandatory. Audio failure must not crash gameplay.

Legacy `Audio.play`, `Audio.playUrl`, `Audio.preload`, and `Audio.stopAll` remain callable. v0.7 also keeps a compatibility `preload(url, gain)` overload because older packages used this call shape. External URL playback is restricted to the game’s own `qrc:/mods/<id>/...` resources; arbitrary `file:` and network audio is not granted to external games.

Modern code should use `GameAudio.playEffect`, `preload`, `playMusic`, group volume, and capability discovery. Audio objects are lazy/cache-bounded and a game’s resource cache is released when switching/unloading packages.

Named host effects and file URLs are distinct concepts; do not pass a named key such as `game/set/move` to a decoder as though it were a WAV URL.
