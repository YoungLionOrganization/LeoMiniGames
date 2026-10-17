// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "FakeNetwork.h"
#include "core/SettingsManager.h"
#include "core/UpdateService.h"
#include <QGuiApplication>
#include <QDesktopServices>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

class UrlRecorder final : public QObject {
    Q_OBJECT
public:
    QUrl last;
public slots:
    void record(const QUrl &url) {last=url;}
};

int main(int argc,char **argv) {
    QGuiApplication app(argc,argv);app.setApplicationVersion("0.7.3");
    QTemporaryDir temporary;SettingsManager settings(temporary.filePath("settings.ini"));
    settings.setValue("updates/channel","stable");FakeNetwork network;UpdateService updates(&settings,nullptr,&network);
    int checks=0,failures=0;const auto check=[&](bool ok,const char *name){++checks;if(!ok){++failures;qCritical()<<"FAIL:"<<name;}};
    UrlRecorder recorder;QDesktopServices::setUrlHandler("https",&recorder,"record");
    const QUrl page("https://leominigames.younglion.xyz/updates/releases/v0.7.4");
    const QUrl download("https://leominigames.younglion.xyz/api/v1/updates/v1/releases/v0.7.4/assets/recommended-package.AppImage");
    const auto complete=[&](const QString &version) {
        updates.checkForUpdates();
        network.last->complete(QJsonDocument(QJsonObject{{"ok",true},{"data",QJsonObject{
            {"schema",1},{"provider_contract","leominigames-update-v1"},
            {"release",QJsonObject{{"version",version},{"page_url",page.toString()}}},
            {"download",QJsonObject{{"url",download.toString()}}}
        }}}).toJson());
    };
    check(!updates.maintenanceAvailable(),"navigation sandbox has no maintenance tool");
    complete("0.7.4");check(updates.updateAvailable(),"newer release fixture accepted");
    check(updates.openUpdate(),"available update opens approved destination");
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    check(recorder.last==page,"Linux update opens package selection instead of generic distro binary");
#else
    check(recorder.last==download,"other platforms retain recommended package navigation");
#endif
    complete("0.7.2");recorder.last=QUrl{};
    check(!updates.updateAvailable()&&!updates.openUpdate()&&recorder.last.isEmpty(),"older release cannot initiate update navigation");
    complete("0.7.3");recorder.last=QUrl{};
    check(!updates.updateAvailable()&&!updates.openUpdate()&&recorder.last.isEmpty(),"current release cannot initiate update navigation");
    check(updates.openReleasePage()&&recorder.last==page,"release information remains available without an update");
    QDesktopServices::unsetUrlHandler("https");
    qInfo()<<checks<<"update navigation assertions,"<<failures<<"failures";return failures?1:0;
}
#include "test_update_navigation.moc"
