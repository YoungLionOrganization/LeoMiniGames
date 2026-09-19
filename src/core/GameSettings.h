// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QVariant>

class SettingsManager;

class GameSettings final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString gameId READ gameId NOTIFY gameChanged)
    Q_PROPERTY(QVariantMap schema READ schema NOTIFY gameChanged)
public:
    explicit GameSettings(SettingsManager *settings, QObject *parent = nullptr);
    QString gameId() const;
    QVariantMap schema() const;
    Q_INVOKABLE QVariant value(const QString &key) const;
    Q_INVOKABLE QVariant value(const QString &key, const QVariant &fallback) const;
    Q_INVOKABLE void setValue(const QString &key, const QVariant &value);
    Q_INVOKABLE bool contains(const QString &key) const;
    Q_INVOKABLE void remove(const QString &key);
    Q_INVOKABLE void clear();
    void activate(const QString &gameId, const QVariantMap &schema = QVariantMap{});
signals:
    void gameChanged();
    void valueChanged(const QString &key, const QVariant &value);
private:
    QString namespaced(const QString &key) const;
    static bool safeKey(const QString &key);
    SettingsManager *m_settings = nullptr;
    QString m_gameId;
    QVariantMap m_schema;
};
