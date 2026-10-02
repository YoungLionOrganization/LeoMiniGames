#include "FakeNetwork.h"
#include "core/AppPaths.h"
#include "core/SettingsManager.h"
#include "core/ModManager.h"
#include "core/ThemeCatalogManager.h"
#include "core/UpdateService.h"
#include "core/PackagePaths.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QCryptographicHash>

int main(int argc,char **argv) {
 QTemporaryDir temp; qputenv("XDG_DATA_HOME",temp.path().toUtf8());qputenv("XDG_CONFIG_HOME",temp.path().toUtf8());
 QCoreApplication app(argc,argv);app.setOrganizationName("LMGTests");app.setApplicationName("CatalogOperations");app.setApplicationVersion("0.7.3");
 int count=0;auto check=[&](bool ok,const char *name){++count;if(!ok)qFatal("FAIL: %s",name);};
 check(argc==2,"fixtures supplied"); AppPaths paths;SettingsManager settings; FakeNetwork network;
 const QString id="modern_v07_fixture";const QString folder=paths.mods()+"/"+id+"/old";QDir().mkpath(folder);
 const QString package=folder+"/"+id+".rcc";check(QFile::copy(QString::fromLocal8Bit(argv[1])+"/v0_7_modern.rcc",package),"seed installed package");
 QFile index(paths.mods()+"/installed.json");check(index.open(QIODevice::WriteOnly),"seed index");
 index.write(QJsonDocument(QJsonArray{QJsonObject{{"id",id},{"installed_version","0.7.0"},{"catalog_version","0.7.0"},{"file",package},{"entry","Main.qml"},{"save_version",1},{"api_version","0.7"}}}).toJson());index.close();
 ModManager mods(&paths,&settings,nullptr,&network);check(mods.isInstalled(id),"existing fixture loads");
 mods.refresh();const int requests=network.requests;mods.install(id);check(network.requests==requests,"install blocked during refresh");
 network.last->complete(QJsonDocument(QJsonObject{{"data",QJsonArray{QJsonObject{{"id",id},{"version","0.8.0"},{"save_version",2},{"api_version","0.8"},{"capabilities",QJsonArray{"network"}}}}}}).toJson());
 check(mods.saveVersionFor(id)==1,"catalog cannot promote installed save schema");check(!mods.networkAllowedFor(id),"catalog cannot grant installed network permission");
 mods.refresh();network.last->complete("{\"data\":{}}");check(mods.isInstalled(id)&&mods.count()==1&&!mods.error().isEmpty(),"malformed catalog preserves installed model");
 mods.refresh();network.last->complete(QJsonDocument(QJsonObject{{"data",QJsonArray{QJsonObject{{"id",id}},QJsonObject{{"id",id}}}}}).toJson());check(mods.count()==1&&!mods.error().isEmpty(),"duplicate catalog rejected atomically");
 QFile incompatible(QString::fromLocal8Bit(argv[1])+"/unsupported_api.rcc");check(incompatible.open(QIODevice::ReadOnly),"incompatible package fixture");const QByteArray candidate=incompatible.readAll();
 mods.refresh();network.last->complete(QJsonDocument(QJsonObject{{"data",QJsonArray{QJsonObject{{"id",id},{"version","0.8.0"},{"sha256",QString::fromLatin1(QCryptographicHash::hash(candidate,QCryptographicHash::Sha256).toHex())},{"size_bytes",candidate.size()},{"download_url","https://example.invalid/package.rcc"}}}}}).toJson());
 mods.install(id);network.last->complete(candidate);check(mods.installedVersion(id)=="0.7.0"&&QFile::exists(package),"incompatible first install keeps previous package");
 mods.setActiveGameId(id);mods.uninstall(id);check(mods.isInstalled(id)&&QFile::exists(package),"running package cannot uninstall");mods.setActiveGameId(QString{});
 QFile::remove(paths.mods()+"/installed.json");QDir().mkdir(paths.mods()+"/installed.json");mods.uninstall(id);check(mods.isInstalled(id)&&QFile::exists(package),"failed index removal restores package");
 QTemporaryDir outside;const QString sentinel=outside.path()+"/keep";QFile keep(sentinel);keep.open(QIODevice::WriteOnly);keep.write("protected");keep.close();
 check(QFile::link(outside.path(),paths.mods()+"/escape"),"symlink fixture");check(PackagePaths::directory(paths.mods(),"escape","key").isEmpty(),"symlink directory rejected");check(QFile::exists(sentinel),"outside sentinel preserved");
 FakeNetwork themesNetwork;ThemeCatalogManager themes(&paths,&settings,nullptr,nullptr,&themesNetwork);themes.refresh();themesNetwork.last->complete("{\"data\":[{\"id\":\"test_theme\"}]}");check(themes.count()==1,"theme catalog seed");
 themes.refresh();themesNetwork.last->complete("{\"data\":null}");check(themes.count()==1&&!themes.error().isEmpty(),"bad theme data retains model");
 FakeNetwork updateNetwork;UpdateService updates(&settings,nullptr,&updateNetwork);updates.checkForUpdates();auto *old=updateNetwork.last;
 updates.setChannel("preview");updates.checkForUpdates();auto *newReply=updateNetwork.last;old->lateComplete("{}");check(updates.checking()&&newReply!=old,"stale update cannot consume new request");
 newReply->complete(R"({"ok":true,"data":{"schema":1,"provider_contract":"leominigames-update-v1","release":{"version":"0.7.4-rc.1","page_url":"https://leominigames.younglion.xyz/updates/release"}}})");check(updates.updateAvailable(),"preview accepts valid prerelease");
 updates.setChannel("stable");updates.checkForUpdates();updateNetwork.last->complete(R"({"ok":true,"data":{"schema":1,"provider_contract":"leominigames-update-v1","release":{"version":"0.7.4-rc.1","page_url":"https://leominigames.younglion.xyz/updates/release"}}})");check(!updates.updateAvailable()&&!updates.errorString().isEmpty(),"stable rejects prerelease");check(updates.downloadUrl().isEmpty()&&updates.releaseUrl().isEmpty(),"failed request clears old links");
 qInfo()<<"PASS:"<<count<<"catalog/update assertions";
}
