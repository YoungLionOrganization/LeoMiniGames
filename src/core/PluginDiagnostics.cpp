// SPDX-License-Identifier: GPL-3.0-or-later
#include "PluginDiagnostics.h"
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QQmlError>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>
namespace { constexpr qint64 kLogLimit=2LL*1024LL*1024LL; constexpr int kMaxRecords=500; }
PluginDiagnostics::PluginDiagnostics(QObject *parent):QObject(parent){}
QString PluginDiagnostics::lastError() const{return m_lastError;} QVariantList PluginDiagnostics::records() const{return m_records;} int PluginDiagnostics::count() const{return m_records.size();}
QString PluginDiagnostics::logPath() const{const QString d=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+QStringLiteral("/logs");QDir().mkpath(d);return d+QStringLiteral("/plugins.log");}
void PluginDiagnostics::rotateIfNeeded() const{const QString p=logPath();QFileInfo i(p);if(i.exists()&&i.size()>kLogLimit){QFile::remove(p+QStringLiteral(".2"));QFile::rename(p+QStringLiteral(".1"),p+QStringLiteral(".2"));QFile::rename(p,p+QStringLiteral(".1"));}}
QString PluginDiagnostics::suggestionFor(const QString&m)const{const QString v=m.toLower();if(v.contains(QStringLiteral("binding loop")))return QStringLiteral("Break the circular QML binding and store one side as explicit state.");if(v.contains(QStringLiteral("undefined")))return QStringLiteral("Guard the value and provide a typed default before assigning it.");if(v.contains(QStringLiteral("no such file"))||v.contains(QStringLiteral("not found")))return QStringLiteral("Check the RCC prefix, alias and case-sensitive resource path.");if(v.contains(QStringLiteral("audio"))||v.contains(QStringLiteral("decode")))return QStringLiteral("Use a named effect or a valid local qrc:/ URL and capability-probe audio.");return QString{};}
void PluginDiagnostics::attach(QQmlApplicationEngine *engine){attachEngine(engine);}
void PluginDiagnostics::attachEngine(QQmlEngine *engine){if(!engine)return;connect(engine,&QQmlEngine::warnings,this,[this](const QList<QQmlError>&errors){for(const QQmlError&e:errors)log(QStringLiteral("qml"),e.description(),e.url().toString(),e.line());});}
void PluginDiagnostics::activate(const QString &gameId,const QString &version,const QString &source){m_gameId=gameId;m_version=version;m_source=source;m_lastError.clear();m_records.clear();emit lastErrorChanged();emit recordsChanged();}
void PluginDiagnostics::log(const QString &level,const QString &message){log(level,message,QString{},0);} void PluginDiagnostics::log(const QString &level,const QString &message,const QString &source){log(level,message,source,0);}
void PluginDiagnostics::log(const QString &level,const QString &message,const QString &source,int line){if(message.trimmed().isEmpty())return;const QString timestamp=QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);const QString normalizedLevel=level.trimmed().toLower();QVariantMap record{{QStringLiteral("timestamp"),timestamp},{QStringLiteral("severity"),normalizedLevel},{QStringLiteral("gameId"),m_gameId},{QStringLiteral("version"),m_version},{QStringLiteral("runtimeSource"),m_source},{QStringLiteral("file"),source},{QStringLiteral("line"),line},{QStringLiteral("message"),message},{QStringLiteral("suggestion"),suggestionFor(message)}};m_records.append(record);while(m_records.size()>kMaxRecords)m_records.removeFirst();emit recordsChanged();const QString text=QStringLiteral("%1 [%2] game=%3 version=%4 type=%5 source=%6:%7 %8").arg(timestamp,normalizedLevel,m_gameId,m_version,m_source,source).arg(line).arg(message);rotateIfNeeded();QFile f(logPath());if(f.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text)){QTextStream out(&f);out<<text<<'\n';}if(normalizedLevel==QStringLiteral("error")||normalizedLevel==QStringLiteral("qml")){if(m_lastError!=message){m_lastError=message;emit lastErrorChanged();}}emit diagnostic(text);}
bool PluginDiagnostics::exportText(const QString &path) const{const QString local=QUrl(path).isLocalFile()?QUrl(path).toLocalFile():path;if(local.isEmpty())return false;QFile f(local);if(!f.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text))return false;QTextStream out(&f);for(const QVariant&r:m_records){const QVariantMap m=r.toMap();out<<m.value(QStringLiteral("timestamp")).toString()<<" ["<<m.value(QStringLiteral("severity")).toString()<<"] "<<m.value(QStringLiteral("file")).toString()<<':'<<m.value(QStringLiteral("line")).toInt()<<' '<<m.value(QStringLiteral("message")).toString()<<'\n';}return true;}
bool PluginDiagnostics::exportJson(const QString &path) const{const QString local=QUrl(path).isLocalFile()?QUrl(path).toLocalFile():path;if(local.isEmpty())return false;QFile f(local);if(!f.open(QIODevice::WriteOnly|QIODevice::Truncate))return false;return f.write(QJsonDocument::fromVariant(m_records).toJson(QJsonDocument::Indented))>=0;}
void PluginDiagnostics::clear(){const bool had=!m_lastError.isEmpty()||!m_records.isEmpty();m_lastError.clear();m_records.clear();if(had){emit lastErrorChanged();emit recordsChanged();}}
