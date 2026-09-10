// SPDX-License-Identifier: GPL-3.0-or-later
#include "AppController.h"
#include "GameRegistry.h"
AppController::AppController(GameRegistry *registry,QObject *parent):QObject(parent),m_registry(registry){}
QObject *AppController::currentGame() const{return m_currentGame.data();}
QUrl AppController::currentGameUrl() const{return m_currentGameUrl;}
QString AppController::currentGameId() const{return m_currentGameId;}
QString AppController::currentGameVersion() const{return m_currentGameVersion;}
QString AppController::currentGameSource() const{return m_currentGameSource;}
bool AppController::openGame(const QString &id)
{
    if (!m_registry) return false;
    closeGame();
    const QUrl url=m_registry->entryUrl(id);
    const QString source=m_registry->sourceFor(id);
    QObject *game=m_registry->createGame(id,this);
    const bool needsNativeObject=source!=QStringLiteral("ExternalRcc");
    if(!url.isValid()||url.isEmpty()||(needsNativeObject&&!game)){
        if(game)game->deleteLater();emit openFailed(QStringLiteral("Could not open game: %1").arg(id));return false;
    }
    m_currentGame=game;m_currentGameUrl=url;m_currentGameId=id;m_currentGameVersion=m_registry->versionFor(id);m_currentGameSource=source;emit currentGameChanged();emit gameOpened();return true;
}
bool AppController::openExternalSession(const QString &id,const QUrl &url,const QString &version)
{
    if(id.isEmpty()||!url.isValid()||url.scheme()!=QStringLiteral("qrc"))return false;
    closeGame();m_currentGame.clear();m_currentGameUrl=url;m_currentGameId=id;m_currentGameVersion=version.isEmpty()?QStringLiteral("dev"):version;m_currentGameSource=QStringLiteral("DeveloperRcc");emit currentGameChanged();emit gameOpened();return true;
}
void AppController::closeGame(){if (!m_currentGame && m_currentGameId.isEmpty()) return;if(m_currentGame)m_currentGame->deleteLater();m_currentGame.clear();m_currentGameUrl=QUrl{};m_currentGameId.clear();m_currentGameVersion.clear();m_currentGameSource.clear();emit currentGameChanged();emit gameClosed();}
