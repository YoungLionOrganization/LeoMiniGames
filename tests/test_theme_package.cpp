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
 AppPaths paths(data.path()+"/data",data.path()+"/cache");ThemeManager themes(&paths);const QString source=QString::fromLocal8Bit(argv[1])+"/valid.rcc";
 check(themes.number("metric.duration.fast")==110,"duration is milliseconds rather than a substring ratio");
 check(themes.number("alias.motion.standard.duration")==180,"aliased animation duration retains milliseconds");
 check(themes.number("alias.game.reaction.pulseDuration")==300,"game animation duration retains milliseconds");
 check(themes.number("alias.modCard.progressHeight")==5,"progress geometry is not a normalized progress value");
 check(themes.number("alias.progress.maxHeight")==1024,"progress maximum height retains layout units");
 check(themes.number("metric.ratio.50")==0.5,"actual ratios remain normalized");
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
 for(const QString &name:{QStringLiteral("legacy_outside"),QStringLiteral("legacy_bad_id"),QStringLiteral("legacy_bad_data"),QStringLiteral("qml_payload")})
   check(!themes.installThemeRcc(QString::fromLocal8Bit(argv[1])+"/"+name+".rcc"),"legacy boundaries, identity, literal data and canonical no-code policy remain enforced");
 check(themes.installThemeRcc(QString::fromLocal8Bit(argv[1])+"/legacy_data.rcc",{},"legacy_data","1.0.1",1),"legacy JSON is adapted without loading companion QML");
 check(themes.applyTheme("legacy_data"),"legacy theme can be selected");
 check(themes.color("color.primary")==QColor("#2468ac")&&themes.color("color.background")==QColor("#102030"),"legacy semantic colors drive current host aliases");
 check(themes.number("metric.radius.md")==17&&themes.number("alias.motion.standard.duration")==180,"legacy geometry overrides preserve complete default timing");
 check(themes.surface("surface.app").value("fillType")=="solid","legacy colors do not inherit an unrelated default gradient");
 check(themes.removeTheme("legacy_data"),"legacy theme unmounts using the same servicing path");
 qInfo()<<"PASS:"<<count<<"theme package assertions";
}
