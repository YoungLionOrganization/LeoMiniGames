# Host service reference — API 0.7 / application 0.7.3

Generated from `src/core` declarations with `python3 tools/sdk/generate_api_reference.py`. Do not edit the signature lists manually. Check drift with `--check`.

Qt context objects are available directly in QML; do not import the private application module. `QString`/`QUrl` become strings/URLs, `QStringList`/`QVariantList` become lists, `QVariantMap` becomes a JavaScript object, and `qreal`/`qint64` become numbers (JavaScript integer precision is limited to 2^53−1). Overloads listed below preserve legacy call shapes.

**Properties are accessed without parentheses.** In particular use `GameRuntime.capabilities`, while other services with an invokable `capabilities()` use parentheses. `ready` means the service exists; `GameAudio.available`/`Haptics.available` probe their support. Required host capabilities do not guarantee an attached audio device, hardware input or network permission.

`ThemeRuntime` in an external engine is an alias for the same scoped facade as `GameTheme`, not the host installer. `App.closeGame()` requests session navigation/teardown. Lifecycle attachment, `Viewport.update/windowInsets` and `GameInput.setFocusRoot/handleKey` are host integration concerns; games should use lifecycle callbacks, read viewport geometry and consume actions.


## `GameRuntime`

Source: [src/core/GameRuntime.h](../../src/core/GameRuntime.h).

Properties:

- `bool ready READ ready CONSTANT`
- `QString version READ version CONSTANT`
- `QString apiVersion READ apiVersion CONSTANT`
- `QStringList capabilities READ capabilities CONSTANT`

Methods:

- `bool supports(const QString &capability) const`
- `QVariantMap checkCompatibility(const QString &apiVersion, const QString &minApiVersion, const QStringList &requiredCapabilities = {}) const`

## `GameSave`

Source: [src/core/GameSave.h](../../src/core/GameSave.h).

Properties:

- `QString gameId READ gameId NOTIFY gameChanged`
- `QString currentSlot READ currentSlot NOTIFY slotChanged`
- `int schemaVersion READ schemaVersion NOTIFY gameChanged`
- `int loadedSchemaVersion READ loadedSchemaVersion NOTIFY loadedChanged`
- `QString lastError READ lastError NOTIFY lastErrorChanged`

Methods:

- `QVariant get(const QString &key) const`
- `QVariant get(const QString &key, const QVariant &fallback) const`
- `void set(const QString &key, const QVariant &value)`
- `void remove(const QString &key)`
- `bool contains(const QString &key) const`
- `bool save()`
- `bool save(const QString &slot)`
- `bool load()`
- `bool load(const QString &slot)`
- `bool createSlot(const QString &slot)`
- `bool deleteSlot(const QString &slot)`
- `QStringList listSlots() const`
- `bool autosave()`
- `bool forceSave()`
- `bool forceSave(const QString &slot)`
- `bool restoreBackup(const QString &slot)`
- `bool registerMigration(int fromVersion, int toVersion, const QJSValue &callback)`
- `void clearMemory()`
- `bool hasStoredSlot(const QString &slot) const`

Signals:

- `void gameChanged()`
- `void slotChanged()`
- `void loadedChanged()`
- `void lastErrorChanged()`
- `void saved(const QString &slot)`
- `void loaded(const QString &slot)`
- `void backupRestored(const QString &slot)`

## `GameSettings`

Source: [src/core/GameSettings.h](../../src/core/GameSettings.h).

Properties:

- `QString gameId READ gameId NOTIFY gameChanged`
- `QVariantMap schema READ schema NOTIFY gameChanged`

Methods:

- `QVariant value(const QString &key) const`
- `QVariant value(const QString &key, const QVariant &fallback) const`
- `void setValue(const QString &key, const QVariant &value)`
- `bool contains(const QString &key) const`
- `void remove(const QString &key)`
- `void clear()`

Signals:

- `void gameChanged()`
- `void valueChanged(const QString &key, const QVariant &value)`

## `GameAudio`

Source: [src/core/GameAudio.h](../../src/core/GameAudio.h).

Properties:

- `bool ready READ ready CONSTANT`
- `bool available READ available CONSTANT`
- `QString version READ version CONSTANT`

Methods:

- `QStringList capabilities() const`
- `void playEffect(const QUrl &url)`
- `void playEffect(const QUrl &url,qreal gain)`
- `void playEffect(const QUrl &url,qreal gain,const QString &group)`
- `void preload(const QUrl &url)`
- `void preload(const QUrl &url,qreal legacyGain)`
- `void playMusic(const QUrl &url)`
- `void playMusic(const QUrl &url,bool loop)`
- `void playMusic(const QUrl &url,bool loop,const QString &group)`
- `void stopMusic()`
- `void pauseAll()`
- `void resumeAll()`
- `qreal volume(const QString &group) const`
- `void setVolume(const QString &group,qreal value)`

Signals:

- `void audioError(const QString &message, const QString &source)`

## `GameInput`

Source: [src/core/GameInput.h](../../src/core/GameInput.h).

Properties:

- `bool ready READ ready CONSTANT`
- `QString version READ version CONSTANT`

Methods:

- `void press(const QString &action)`
- `void press(const QString &action, qreal value)`
- `void release(const QString &action)`
- `void setValue(const QString &action, qreal value)`
- `bool isPressed(const QString &action) const`
- `qreal value(const QString &action) const`
- `bool handleKey(int key, quint32 nativeScanCode, const QString &text, bool pressed, bool autoRepeat = false)`
- `QString actionForKey(int key, quint32 nativeScanCode, const QString &text) const`
- `QStringList capabilities() const`
- `void reset()`
- `void setFocusRoot(QObject *root)`

Signals:

- `void actionPressed(const QString &action, qreal value)`
- `void actionReleased(const QString &action)`
- `void actionValueChanged(const QString &action, qreal value)`

## `Viewport`

Source: [src/core/GameViewport.h](../../src/core/GameViewport.h).

Properties:

- `qreal safeWidth READ safeWidth NOTIFY changed`
- `qreal safeHeight READ safeHeight NOTIFY changed`
- `qreal safeTop READ safeTop NOTIFY changed`
- `qreal safeBottom READ safeBottom NOTIFY changed`
- `qreal density READ density NOTIFY changed`
- `bool portrait READ portrait NOTIFY changed`
- `bool landscape READ landscape NOTIFY changed`
- `bool valid READ valid NOTIFY changed`

Methods:

- `QVariantMap windowInsets(QObject *item)`
- `void update(qreal width, qreal height, qreal top = 0.0, qreal bottom = 0.0, qreal density = 1.0)`

Signals:

- `void changed()`
- `void nativeInsetsChanged()`

## `GameTheme`

Source: [src/core/GameThemeFacade.h](../../src/core/GameThemeFacade.h).

Properties:

- `bool ready READ ready CONSTANT`
- `QString version READ version CONSTANT`
- `QString activeThemeId READ activeThemeId NOTIFY activeThemeChanged`
- `QString activeThemeName READ activeThemeName NOTIFY activeThemeChanged`
- `int revision READ revision NOTIFY revisionChanged`
- `QVariantList themes READ themes NOTIFY themesChanged`
- `int themeCount READ themeCount NOTIFY themesChanged`

Methods:

- `QVariant value(const QString &key) const`
- `QColor color(const QString &key) const`
- `qreal number(const QString &key) const`
- `QString stringValue(const QString &key) const`
- `bool hasValue(const QString &key) const`
- `QStringList keys(const QString &prefix=QString()) const`
- `QVariantMap values(const QString &prefix=QString()) const`
- `QVariantMap surface(const QString &key) const`
- `bool isInstalled(const QString &id) const`
- `QString installedVersion(const QString &id) const`
- `bool applyTheme(const QString &id)`
- `QString lastError() const`
- `QStringList capabilities() const`

Signals:

- `void activeThemeChanged()`
- `void revisionChanged()`
- `void themesChanged()`

## `GameI18n`

Source: [src/core/GameI18n.h](../../src/core/GameI18n.h).

Properties:

- `QString gameId READ gameId NOTIFY changed`
- `QString language READ language WRITE setLanguage NOTIFY languageChanged`
- `QString defaultLocale READ defaultLocale NOTIFY changed`
- `bool ready READ ready CONSTANT`
- `QString version READ version CONSTANT`
- `QStringList locales READ locales NOTIFY changed`

Methods:

- `QStringList capabilities() const`
- `bool hasKey(const QString &key) const`
- `QString text(const QString &key)`
- `QString text(const QString &key, const QString &fallback)`
- `QString format(const QString &key, const QVariantMap &arguments)`
- `QString format(const QString &key, const QVariantMap &arguments, const QString &fallback)`
- `QString plural(const QString &key, qint64 count)`
- `QString plural(const QString &key, qint64 count, const QString &fallback)`
- `void setLanguage(const QString &language)`

Signals:

- `void changed()`
- `void languageChanged()`
- `void missingKey(const QString &key, const QString &locale)`

## `GameResources`

Source: [src/core/GameResources.h](../../src/core/GameResources.h).

Properties:

- `QString gameId READ gameId NOTIFY changed`
- `bool ready READ ready CONSTANT`
- `QString version READ version CONSTANT`

Methods:

- `QUrl url(const QString &relativePath) const`
- `bool exists(const QString &relativePath) const`
- `QStringList capabilities() const`

Signals:

- `void changed()`

## `Lifecycle`

Source: [src/core/GameLifecycleFacade.h](../../src/core/GameLifecycleFacade.h).

Properties:

- `QString gameId READ gameId NOTIFY gameChanged`

Methods:

- `void load()`
- `void start()`
- `void pause()`
- `void resume()`
- `void background()`
- `void foreground()`
- `void save()`
- `void close()`
- `void unload()`

Signals:

- `void gameChanged()`
- `void loaded()`
- `void started()`
- `void paused()`
- `void resumed()`
- `void backgrounded()`
- `void foregrounded()`
- `void saveRequested()`
- `void closed()`
- `void unloaded()`

## `GameClock`

Source: [src/core/GameClock.h](../../src/core/GameClock.h).

Properties:

- `bool paused READ paused NOTIFY pausedChanged`
- `double timeScale READ timeScale WRITE setTimeScale NOTIFY timeScaleChanged`

Methods:

- `void reset()`
- `void pause()`
- `void resume()`
- `qint64 elapsedMs() const`
- `double elapsedSeconds() const`

Signals:

- `void pausedChanged()`
- `void timeScaleChanged()`

## `GameRandom`

Source: [src/core/GameRandom.h](../../src/core/GameRandom.h).

Properties:

- `QString state READ state WRITE setState NOTIFY stateChanged`

Methods:

- `void seed(const QString &seedText)`
- `int nextInt(int minimumInclusive, int maximumInclusive)`
- `double nextReal()`
- `bool chance(double probability)`
- `int pickIndex(int count)`
- `QVariantList shuffled(const QVariantList &values)`

Signals:

- `void stateChanged()`

## `GameEvents`

Source: [src/core/GameEvents.h](../../src/core/GameEvents.h).

Methods:

- `void emitEvent(const QString &name, const QVariant &payload = QVariant{})`

Signals:

- `void eventEmitted(const QString &name, const QVariant &payload)`
- `}`

## `GameStats`

Source: [src/core/GameStats.h](../../src/core/GameStats.h).

Properties:

- `QString gameId READ gameId NOTIFY changed`

Methods:

- `qint64 highScore() const`
- `qint64 highScore(const QString &name) const`
- `bool submitHighScore(qint64 score)`
- `bool submitHighScore(qint64 score, const QString &name)`
- `qint64 counter(const QString &name) const`
- `void setCounter(const QString &name, qint64 value)`
- `qint64 increment(const QString &name)`
- `qint64 increment(const QString &name, qint64 amount)`
- `qint64 gamesPlayed() const`
- `qint64 totalTimeMs() const`

Signals:

- `void changed()`

## `Achievements`

Source: [src/core/Achievements.h](../../src/core/Achievements.h).

Properties:

- `QString gameId READ gameId NOTIFY changed`

Methods:

- `bool unlock(const QString &id)`
- `bool isUnlocked(const QString &id) const`
- `qreal progress(const QString &id) const`
- `void setProgress(const QString &id, qreal progress)`

Signals:

- `void changed()`
- `void unlocked(const QString &id)`

## `Haptics`

Source: [src/core/Haptics.h](../../src/core/Haptics.h).

Properties:

- `bool available READ available CONSTANT`

Methods:

- `void light()`
- `void medium()`
- `void heavy()`
- `void pulse(const QString &level)`

## `GameLogger`

Source: [src/core/GameLoggerFacade.h](../../src/core/GameLoggerFacade.h).

Properties:

- `QString lastError READ lastError NOTIFY lastErrorChanged`

Methods:

- `void log(const QString &level, const QString &message)`
- `void log(const QString &level, const QString &message, const QString &source)`
- `void log(const QString &level, const QString &message, const QString &source, int line)`

Signals:

- `void lastErrorChanged()`

## `App`

Source: [src/core/LegacyAppFacade.h](../../src/core/LegacyAppFacade.h).

Properties:

- `QUrl currentGameUrl READ currentGameUrl NOTIFY currentGameChanged`
- `QString currentGameId READ currentGameId NOTIFY currentGameChanged`
- `QString currentGameVersion READ currentGameVersion NOTIFY currentGameChanged`
- `QString currentGameSource READ currentGameSource NOTIFY currentGameChanged`

Methods:

- `void closeGame()`

Signals:

- `void currentGameChanged()`

## `Settings`

Source: [src/core/LegacySettingsFacade.h](../../src/core/LegacySettingsFacade.h).

Properties:

- `bool soundEnabled READ soundEnabled WRITE setSoundEnabled NOTIFY soundEnabledChanged`
- `qreal soundVolume READ soundVolume WRITE setSoundVolume NOTIFY soundVolumeChanged`
- `bool animationsEnabled READ animationsEnabled WRITE setAnimationsEnabled NOTIFY animationsEnabledChanged`

Methods:

- `QVariant value(const QString &key) const`
- `QVariant value(const QString &key, const QVariant &fallback) const`
- `void setValue(const QString &key, const QVariant &value)`
- `bool contains(const QString &key) const`
- `void remove(const QString &key)`
- `void setSoundEnabled(bool enabled)`
- `void setSoundVolume(qreal volume)`
- `void setAnimationsEnabled(bool enabled)`

Signals:

- `void soundEnabledChanged()`
- `void soundVolumeChanged()`
- `void animationsEnabledChanged()`

## `Audio`

Source: [src/core/LegacyAudioFacade.h](../../src/core/LegacyAudioFacade.h).

Properties:

- `bool available READ available CONSTANT`
- `bool ready READ ready CONSTANT`
- `QString version READ version CONSTANT`

Methods:

- `QStringList capabilities() const`
- `void play(const QString &name)`
- `void playUrl(const QUrl &url)`
- `void playUrl(const QUrl &url, qreal gain)`
- `void preload(const QUrl &url)`
- `void preload(const QUrl &url, qreal gain)`
- `void stopAll()`

## `Lang`

Source: [src/core/LegacyLanguageFacade.h](../../src/core/LegacyLanguageFacade.h).

Properties:

- `QString language READ language WRITE setLanguage NOTIFY languageChanged`
- `QVariantList availableLanguages READ availableLanguages CONSTANT`

Methods:

- `QString text(const QString &source, const QString &languageDependency = QString{}) const`
- `void setLanguage(const QString &code)`

Signals:

- `void languageChanged()`

Native C++ plugins use a separate ABI: [Native SDK](NATIVE_API.md). Workflow examples and behavior details are indexed in [Overview](OVERVIEW.md).
