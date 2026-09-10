// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QColor>
#include <QHash>
#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

class AppPaths;

class ThemeManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString activeThemeId READ activeThemeId NOTIFY activeThemeChanged)
    Q_PROPERTY(QString activeThemeName READ activeThemeName NOTIFY activeThemeChanged)
    Q_PROPERTY(int revision READ revision NOTIFY revisionChanged)
    Q_PROPERTY(QVariantList themes READ themes NOTIFY themesChanged)
    Q_PROPERTY(int themeCount READ themeCount NOTIFY themesChanged)

public:
    explicit ThemeManager(AppPaths *paths, QObject *parent = nullptr);
    ~ThemeManager() override;

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
    Q_INVOKABLE QStringList keys(const QString &prefix = QString{}) const;
    Q_INVOKABLE QVariantMap values(const QString &prefix = QString{}) const;
    Q_INVOKABLE QVariantMap surface(const QString &key) const;

    Q_INVOKABLE bool isInstalled(const QString &id) const;
    Q_INVOKABLE QString installedVersion(const QString &id) const;

    Q_INVOKABLE bool applyTheme(const QString &id);
    // Install is intentionally upsert-like for external themes. A package with an
    // already installed external id replaces the old version transactionally.
    Q_INVOKABLE bool installThemeRcc(const QString &sourcePath, const QString &expectedSha256 = QString{});
    Q_INVOKABLE bool updateThemeRcc(const QString &sourcePath, const QString &expectedSha256 = QString{});
    Q_INVOKABLE bool removeTheme(const QString &id);
    Q_INVOKABLE void reloadInstalled();
    Q_INVOKABLE QString lastError() const;

signals:
    void activeThemeChanged();
    void revisionChanged();
    void themesChanged();
    void themeInstalled(const QString &id);
    void themeUpdated(const QString &id);
    void themeRemoved(const QString &id);
    void errorOccurred(const QString &message);

private:
    struct ThemeRecord {
        QString id;
        QString name;
        QString version;
        QString publisher;
        QString filePath;
        QString resourceRoot;
        QVariantMap document;
        bool builtIn = false;
        bool registered = false;
    };

    bool loadBuiltIn();
    bool registerExternal(const QString &filePath);
    bool loadExternalRecord(const QString &filePath, ThemeRecord *record);
    bool validateThemeResources(const QString &resourceRoot);
    bool parseDocument(const QByteArray &json, ThemeRecord *record, const QString &resourceRoot, const QString &filePath, bool builtIn);
    bool recordContainsKey(const ThemeRecord &record, const QString &key) const;
    QVariant resolveKey(const QString &key, int depth = 0) const;
    QVariant resolveNested(const QVariant &input, const QString &assetRoot, int depth = 0) const;
    QString sha256File(const QString &path) const;
    void setError(const QString &message);
    void bumpRevision();
    void restoreSelection();
    void persistSelection() const;

    AppPaths *m_paths = nullptr;
    QHash<QString, ThemeRecord> m_records;
    QString m_activeId;
    QString m_defaultId = QStringLiteral("younglion.bronze-espresso");
    int m_revision = 0;
    QString m_lastError;
};
