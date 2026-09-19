// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QColor>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
class ThemeManager;
class GameThemeFacade final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString activeThemeId READ activeThemeId NOTIFY activeThemeChanged)
    Q_PROPERTY(QString activeThemeName READ activeThemeName NOTIFY activeThemeChanged)
    Q_PROPERTY(int revision READ revision NOTIFY revisionChanged)
    Q_PROPERTY(QVariantList themes READ themes NOTIFY themesChanged)
    Q_PROPERTY(int themeCount READ themeCount NOTIFY themesChanged)
public:
    explicit GameThemeFacade(ThemeManager *theme,QObject *parent=nullptr);
    bool ready() const { return true; }
    QString version() const { return QStringLiteral("0.7"); }
    QString activeThemeId() const;
    QString activeThemeName() const;
    int revision() const;
    QVariantList themes() const;
    int themeCount() const;
    Q_INVOKABLE QVariant value(const QString &key) const;
    Q_INVOKABLE QColor color(const QString &key) const;
    Q_INVOKABLE qreal number(const QString &key) const;
    Q_INVOKABLE QString stringValue(const QString &key) const;
    Q_INVOKABLE bool hasValue(const QString &key) const;
    Q_INVOKABLE QStringList keys(const QString &prefix=QString()) const;
    Q_INVOKABLE QVariantMap values(const QString &prefix=QString()) const;
    Q_INVOKABLE QVariantMap surface(const QString &key) const;
    Q_INVOKABLE bool isInstalled(const QString &id) const;
    Q_INVOKABLE QString installedVersion(const QString &id) const;
    // Legacy compatibility: selecting an already-installed theme was part of the old exposed ThemeManager surface.
    Q_INVOKABLE bool applyTheme(const QString &id);
    Q_INVOKABLE QString lastError() const;
    Q_INVOKABLE QStringList capabilities() const;
signals:
    void activeThemeChanged();
    void revisionChanged();
    void themesChanged();
private:
    ThemeManager *m_theme=nullptr;
};
