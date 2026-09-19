// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "Achievements.h"
#include "AppPaths.h"
#include <QCborValue>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QtGlobal>

namespace {
constexpr qint64 kMaxAchievementBytes = 2LL * 1024LL * 1024LL;
constexpr qsizetype kMaxAchievementEntries = 4096;
constexpr qsizetype kMaxAchievementId = 256;

bool safeAchievementId(const QString &id)
{
    return !id.isEmpty() && id.size() <= kMaxAchievementId && !id.contains(QChar(u'\0'));
}

bool canInsertAchievement(const QVariantMap &data, const QString &id)
{
    const QString unlockedKey=QStringLiteral("u/%1").arg(id);
    const QString progressKey=QStringLiteral("p/%1").arg(id);
    qsizetype needed=0;
    if(!data.contains(unlockedKey))++needed;
    if(!data.contains(progressKey))++needed;
    return data.size()+needed<=kMaxAchievementEntries;
}
}

Achievements::Achievements(AppPaths *paths,QObject *parent):QObject(parent),m_paths(paths){}
QString Achievements::gameId() const{return m_gameId;}
QString Achievements::path() const{if(!m_paths||m_gameId.isEmpty())return QString{};const QString d=m_paths->saves()+QLatin1Char('/')+m_gameId;QDir().mkpath(d);return d+QStringLiteral("/achievements.cbor");}
void Achievements::load(){m_data.clear();const QString p=path();if(p.isEmpty())return;QFile f(p);if(!f.open(QIODevice::ReadOnly)||f.size()<=0||f.size()>kMaxAchievementBytes)return;QCborParserError e;auto v=QCborValue::fromCbor(f.read(kMaxAchievementBytes+1),&e);if(e.error==QCborError::NoError&&v.isMap()){const QVariantMap loaded=v.toVariant().toMap();if(loaded.size()<=kMaxAchievementEntries)m_data=loaded;}}
void Achievements::persist() const{const QString p=path();if(p.isEmpty())return;const QByteArray payload=QCborValue::fromVariant(m_data).toCbor();if(payload.isEmpty()||payload.size()>kMaxAchievementBytes)return;QSaveFile f(p);if(!f.open(QIODevice::WriteOnly))return;if(f.write(payload)!=payload.size()){f.cancelWriting();return;}f.commit();}
void Achievements::activate(const QString &gameId){if(m_gameId==gameId)return;m_gameId=gameId;load();emit changed();}
bool Achievements::isUnlocked(const QString &id) const{if(!safeAchievementId(id))return false;return m_data.value(QStringLiteral("u/%1").arg(id),false).toBool();}
bool Achievements::unlock(const QString &id){if(!safeAchievementId(id)||isUnlocked(id)||!canInsertAchievement(m_data,id))return false;m_data.insert(QStringLiteral("u/%1").arg(id),true);m_data.insert(QStringLiteral("p/%1").arg(id),1.0);persist();emit changed();emit unlocked(id);return true;}
qreal Achievements::progress(const QString &id) const{if(!safeAchievementId(id))return 0.0;return qBound<qreal>(0.0,m_data.value(QStringLiteral("p/%1").arg(id),0.0).toReal(),1.0);}
void Achievements::setProgress(const QString &id,qreal p){if(!safeAchievementId(id)||!canInsertAchievement(m_data,id))return;p=qBound<qreal>(0.0,p,1.0);m_data.insert(QStringLiteral("p/%1").arg(id),p);if(p>=1.0&&!isUnlocked(id)){unlock(id);return;}persist();emit changed();}
