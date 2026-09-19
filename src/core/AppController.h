// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QPointer>
#include <QUrl>

class GameRegistry;

class AppController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *currentGame READ currentGame NOTIFY currentGameChanged)
    Q_PROPERTY(QUrl currentGameUrl READ currentGameUrl NOTIFY currentGameChanged)
    Q_PROPERTY(QString currentGameId READ currentGameId NOTIFY currentGameChanged)
    Q_PROPERTY(QString currentGameVersion READ currentGameVersion NOTIFY currentGameChanged)
    Q_PROPERTY(QString currentGameSource READ currentGameSource NOTIFY currentGameChanged)
public:
    explicit AppController(GameRegistry *registry, QObject *parent = nullptr);
    QObject *currentGame() const; QUrl currentGameUrl() const; QString currentGameId() const; QString currentGameVersion() const; QString currentGameSource() const;
    Q_INVOKABLE bool openGame(const QString &id); Q_INVOKABLE void closeGame();
    bool openExternalSession(const QString &id, const QUrl &url, const QString &version);
signals:
    void currentGameChanged(); void gameOpened(); void gameClosed(); void openFailed(const QString &message);
private:
    GameRegistry *m_registry = nullptr; QPointer<QObject> m_currentGame; QUrl m_currentGameUrl; QString m_currentGameId, m_currentGameVersion, m_currentGameSource;
};
