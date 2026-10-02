#include "core/ThemeManager.h"
#include "core/AppPaths.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QCryptographicHash>
#include <QDebug>
#include <QUrl>
int main(int argc,char **argv) {
 QTemporaryDir data;qputenv("XDG_DATA_HOME",data.path().toUtf8());QCoreApplication app(argc,argv);app.setOrganizationName("LMGTests");app.setApplicationName("ThemePackage");
 int count=0;auto check=[&](bool ok,const char *name){++count;if(!ok)qFatal("FAIL: %s",name);};check(argc==2,"fixtures supplied");
 AppPaths paths;ThemeManager themes(&paths);const QString source=QString::fromLocal8Bit(argv[1])+"/valid.rcc";
 QFile file(source);check(file.open(QIODevice::ReadOnly),"fixture bytes");const QString hash=QString::fromLatin1(QCryptographicHash::hash(file.readAll(),QCryptographicHash::Sha256).toHex());
 const QString mounted=":/__lmgtheme/"+hash.left(24)+"/theme/theme.json";
 check(!themes.installThemeRcc(source,hash,"other_theme","1.0.0",1),"catalog ID mismatch rejected");check(!QFile::exists(mounted),"rejected identity unmounts exact resource root");
 check(!themes.installThemeRcc(source,hash,"test_theme","2.0.0",1),"catalog version mismatch rejected");
 check(!themes.installThemeRcc(source,hash,"test_theme","1.0.0",2),"catalog API mismatch rejected");
 check(themes.installThemeRcc(source,hash,"test_theme","1.0.0",1),"matching candidate installed");check(QFile::exists(mounted),"installed resources mounted");
 check(themes.installedVersion("test_theme")=="1.0.0","installed version truthful");
 check(!themes.installThemeRcc(source,hash,"other_theme","1.0.0",1),"cached hash still validates identity");
 check(!themes.installThemeRcc(QString::fromLocal8Bit(argv[1])+"/outside.rcc"),"resources outside theme namespace rejected");
 for(const QString &name:{QStringLiteral("cycle"),QStringLiteral("bad_api"),QStringLiteral("bad_version")})
   check(!themes.installThemeRcc(QString::fromLocal8Bit(argv[1])+"/"+name+".rcc"),"bad aliases, API types and versions rejected");
 check(themes.applyTheme("test_theme"),"apply installed theme");
 const QString asset=themes.stringValue("alias.asset.test");
 check(asset.startsWith("qrc:/__lmgtheme/")&&QFile::exists(QString(":")+QUrl(asset).path()),"aliased theme asset resolves relative to owning RCC");
 const QVariantMap surface=themes.surface("surface.card");
 check(surface.contains("radius")&&surface.contains("borderWidth"),"partial surface inherits default geometry");
 check(surface.value("gradient").toMap().value("start")=="#112233"&&surface.value("gradient").toMap().contains("end"),"partial nested gradient inherits default stops");
 check(surface.value("texture")==asset,"surface token asset uses resolved RCC URL");
 check(themes.color("color.background")==QColor("#112233"),"active token applied");
 check(themes.removeTheme("test_theme"),"remove theme");
 check(themes.activeThemeId()=="younglion.bronze-espresso"&&themes.number("alias.card.radius")>0,"active removal returns to complete built-in theme");check(!QFile::exists(mounted)&&!themes.isInstalled("test_theme"),"remove actually unmounts and updates model");
 qInfo()<<"PASS:"<<count<<"theme package assertions";
}
