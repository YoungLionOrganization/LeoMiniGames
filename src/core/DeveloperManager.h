// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QVariantMap>
#include <QUrl>
#include "RccPackageInspector.h"
class AppPaths; class GameRegistry; class PluginDiagnostics; class QNetworkAccessManager;
class DeveloperManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool authenticated READ authenticated NOTIFY changed)
    Q_PROPERTY(bool verifying READ verifying NOTIFY changed)
    Q_PROPERTY(QString status READ status NOTIFY changed)
    Q_PROPERTY(QString lastError READ lastError NOTIFY changed)
    Q_PROPERTY(QVariantMap packageInfo READ packageInfo NOTIFY packageChanged)
    Q_PROPERTY(QVariantList deviceProfiles READ deviceProfiles CONSTANT)
    Q_PROPERTY(int selectedProfile READ selectedProfile NOTIFY profileChanged)
    Q_PROPERTY(QString profileName READ profileName NOTIFY profileChanged)
    Q_PROPERTY(qreal profileWidth READ profileWidth NOTIFY profileChanged)
    Q_PROPERTY(qreal profileHeight READ profileHeight NOTIFY profileChanged)
    Q_PROPERTY(qreal profileSafeTop READ profileSafeTop NOTIFY profileChanged)
    Q_PROPERTY(qreal profileSafeBottom READ profileSafeBottom NOTIFY profileChanged)
public:
    explicit DeveloperManager(AppPaths *paths,GameRegistry *games,PluginDiagnostics *diagnostics,QObject *parent=nullptr);
    ~DeveloperManager() override;
    bool authenticated()const{return m_authenticated;} bool verifying()const{return m_verifying;} QString status()const{return m_status;} QString lastError()const{return m_error;} QVariantMap packageInfo()const{return m_packageInfo;}
    QVariantList deviceProfiles() const; int selectedProfile() const{return m_selectedProfile;} QString profileName() const; qreal profileWidth() const; qreal profileHeight() const; qreal profileSafeTop() const; qreal profileSafeBottom() const;
    Q_INVOKABLE void openDeveloperPortal();
    Q_INVOKABLE void selectDeviceProfile(int index);
    Q_INVOKABLE void verifyApiKey(const QString &apiKey);
    Q_INVOKABLE bool importRcc(const QUrl &sourceFile);
    Q_INVOKABLE bool launchImported();
    Q_INVOKABLE void clearImported();
    Q_INVOKABLE void logout();
    Q_INVOKABLE QVariantMap settingsSchemaFor(const QString &id) const; Q_INVOKABLE QStringList localesFor(const QString &id) const; Q_INVOKABLE QString defaultLocaleFor(const QString &id) const; Q_INVOKABLE int saveVersionFor(const QString &id) const;
signals: void changed(); void packageChanged(); void profileChanged(); void launchRequested(const QString &id,const QUrl &url,const QString &version);
private:
    void setError(const QString &message); void setStatus(const QString &message); static QString sha256File(const QString &path); static bool safeKeyFormat(const QString &key);
    AppPaths *m_paths=nullptr; GameRegistry *m_games=nullptr; PluginDiagnostics *m_diagnostics=nullptr; QNetworkAccessManager *m_network=nullptr;
    bool m_authenticated=false,m_verifying=false; int m_selectedProfile=0; QString m_status,m_error,m_rccFile; QVariantMap m_packageInfo; RccPackageInspection m_inspection;
};
