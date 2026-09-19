// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QUrl>
class AppController;
class LegacyAppFacade final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl currentGameUrl READ currentGameUrl NOTIFY currentGameChanged)
    Q_PROPERTY(QString currentGameId READ currentGameId NOTIFY currentGameChanged)
    Q_PROPERTY(QString currentGameVersion READ currentGameVersion NOTIFY currentGameChanged)
    Q_PROPERTY(QString currentGameSource READ currentGameSource NOTIFY currentGameChanged)
public:
    explicit LegacyAppFacade(AppController *controller, QObject *parent=nullptr);
    QUrl currentGameUrl() const;
    QString currentGameId() const;
    QString currentGameVersion() const;
    QString currentGameSource() const;
    Q_INVOKABLE void closeGame();
signals:
    void currentGameChanged();
private:
    AppController *m_controller=nullptr;
};
