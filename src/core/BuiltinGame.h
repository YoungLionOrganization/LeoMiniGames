// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QVariantMap>
class GameSave;

// Internal native-game adapter; the public native plugin ABI remains unchanged.
class BuiltinGame : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool suspended READ suspended NOTIFY suspensionChanged)
public:
    explicit BuiltinGame(QObject *parent = nullptr);
    void setSaveService(GameSave *save);
    bool suspended() const { return m_suspended; }
    Q_INVOKABLE virtual QVariantMap snapshot() const = 0;
    Q_INVOKABLE virtual bool restoreSnapshot(const QVariantMap &state) = 0;
    Q_INVOKABLE void load();
    Q_INVOKABLE virtual void start() {}
    Q_INVOKABLE void save();
    Q_INVOKABLE virtual void pause();
    Q_INVOKABLE virtual void resume();
    Q_INVOKABLE void close();
    Q_INVOKABLE void unload();
signals:
    void suspensionChanged();
private slots:
    void scheduleCheckpoint();
private:
    QPointer<GameSave> m_save;
    QTimer m_checkpoint;
    bool m_loaded = false;
    bool m_restoring = false;
    bool m_suspended = false;
};
