// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "GameResources.h"
#include <QDir>
#include <QFileInfo>

GameResources::GameResources(QObject *parent)
    : QObject(parent)
{
}

void GameResources::activate(const QString &gameId){if(m_gameId==gameId)return;m_gameId=gameId;emit changed();}
bool GameResources::safePath(const QString &path){return !path.isEmpty()&&!path.startsWith(QLatin1Char('/'))&&!path.contains(QStringLiteral(".."))&&!path.contains(QLatin1Char('\\'))&&QDir::cleanPath(path)==path;}
QUrl GameResources::url(const QString &relativePath) const { if(m_gameId.isEmpty()||!safePath(relativePath))return QUrl(); return QUrl(QStringLiteral("qrc:/mods/%1/%2").arg(m_gameId,relativePath)); }
bool GameResources::exists(const QString &relativePath) const { const QUrl u=url(relativePath); return u.isValid() && QFileInfo::exists(QStringLiteral(":/mods/%1/%2").arg(m_gameId,relativePath)); }
