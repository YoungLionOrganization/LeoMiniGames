// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "GameStats.h"
#include "AppPaths.h"
#include <QCborValue>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QtGlobal>

namespace {
constexpr qint64 kMaxStatsBytes = 2LL * 1024LL * 1024LL;
constexpr qsizetype kMaxStatsEntries = 4096;
constexpr qsizetype kMaxMetricName = 256;

bool safeMetricName(const QString &name)
{
    return !name.isEmpty() && name.size() <= kMaxMetricName && !name.contains(QChar(u'\0'));
}

bool canInsertMetric(const QVariantMap &data, const QString &key)
{
    return data.contains(key) || data.size() < kMaxStatsEntries;
}
}

GameStats::GameStats(AppPaths *paths, QObject *parent) : QObject(parent), m_paths(paths) {}
QString GameStats::gameId() const { return m_gameId; }
QString GameStats::path() const { if (!m_paths || m_gameId.isEmpty()) return QString{}; const QString dir = m_paths->saves()+QLatin1Char('/')+m_gameId; QDir().mkpath(dir); return dir+QStringLiteral("/stats.cbor"); }
void GameStats::load(){ m_data.clear(); const QString p=path(); if(p.isEmpty())return; QFile f(p); if (!f.open(QIODevice::ReadOnly) || f.size() <= 0 || f.size() > kMaxStatsBytes) return; QCborParserError e; const QCborValue v=QCborValue::fromCbor(f.read(kMaxStatsBytes+1),&e); if(e.error==QCborError::NoError&&v.isMap()){const QVariantMap loaded=v.toVariant().toMap();if(loaded.size()<=kMaxStatsEntries)m_data=loaded;} }
void GameStats::persist() const { const QString p=path(); if(p.isEmpty())return; const QByteArray payload=QCborValue::fromVariant(m_data).toCbor(); if(payload.isEmpty()||payload.size()>kMaxStatsBytes)return; QSaveFile f(p); if(!f.open(QIODevice::WriteOnly))return; if(f.write(payload)!=payload.size()){f.cancelWriting();return;} f.commit(); }
void GameStats::activate(const QString &gameId){ if(m_gameId==gameId)return; endSession(); m_gameId=gameId; load(); emit changed(); }
qint64 GameStats::highScore() const{return highScore(QStringLiteral("default"));}
qint64 GameStats::highScore(const QString &name) const { if(!safeMetricName(name))return 0; return m_data.value(QStringLiteral("high/%1").arg(name),0).toLongLong(); }
bool GameStats::submitHighScore(qint64 score){return submitHighScore(score,QStringLiteral("default"));}
bool GameStats::submitHighScore(qint64 score,const QString &name){ if(!safeMetricName(name)||score<=highScore(name))return false; const QString key=QStringLiteral("high/%1").arg(name); if(!canInsertMetric(m_data,key))return false; m_data.insert(key,score);persist();emit changed();return true; }
qint64 GameStats::counter(const QString &name) const{if(!safeMetricName(name))return 0;return m_data.value(QStringLiteral("counter/%1").arg(name),0).toLongLong();}
void GameStats::setCounter(const QString &name,qint64 value){if(!safeMetricName(name))return;const QString key=QStringLiteral("counter/%1").arg(name);if(!canInsertMetric(m_data,key))return;m_data.insert(key,value);persist();emit changed();}
qint64 GameStats::increment(const QString &name){return increment(name,1);}
qint64 GameStats::increment(const QString &name,qint64 amount){if(!safeMetricName(name))return 0;const qint64 v=counter(name)+amount;setCounter(name,v);return v;}
qint64 GameStats::gamesPlayed() const{return m_data.value(QStringLiteral("gamesPlayed"),0).toLongLong();}
qint64 GameStats::totalTimeMs() const{return m_data.value(QStringLiteral("totalTimeMs"),0).toLongLong();}
void GameStats::startSession(){ if(m_gameId.isEmpty())return; m_data.insert(QStringLiteral("gamesPlayed"),gamesPlayed()+1); m_session.restart(); persist(); emit changed(); }
void GameStats::endSession(){ if(!m_session.isValid()||m_gameId.isEmpty())return; m_data.insert(QStringLiteral("totalTimeMs"),totalTimeMs()+m_session.elapsed());m_session.invalidate();persist();emit changed(); }
