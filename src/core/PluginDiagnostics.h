// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QString>
#include <QVariantList>
class QQmlEngine;
class QQmlApplicationEngine;
class PluginDiagnostics final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QVariantList records READ records NOTIFY recordsChanged)
    Q_PROPERTY(int count READ count NOTIFY recordsChanged)
public:
    explicit PluginDiagnostics(QObject *parent=nullptr);
    QString lastError() const; QVariantList records() const; int count() const;
    void attach(QQmlApplicationEngine *engine); void attachEngine(QQmlEngine *engine);
    void activate(const QString &gameId,const QString &version,const QString &source);
    Q_INVOKABLE void log(const QString &level,const QString &message);
    Q_INVOKABLE void log(const QString &level,const QString &message,const QString &source);
    Q_INVOKABLE void log(const QString &level,const QString &message,const QString &source,int line);
    Q_INVOKABLE bool exportText(const QString &path) const; Q_INVOKABLE bool exportJson(const QString &path) const;
    Q_INVOKABLE void clear();
signals:void lastErrorChanged();void recordsChanged();void diagnostic(const QString &line);
private:
    QString logPath() const; QString suggestionFor(const QString &message) const; void rotateIfNeeded() const;
    QString m_gameId,m_version,m_source,m_lastError; QVariantList m_records;
};
