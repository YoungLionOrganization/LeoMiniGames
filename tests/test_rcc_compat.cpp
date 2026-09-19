// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "core/RccPackageInspector.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <iostream>
static bool positive(const QString &path,const QString &id,bool legacy){
    const auto i=RccPackageInspector::inspect(path);
    if(!i.valid||i.packageId!=id||i.legacyLayout!=legacy){std::cerr<<i.error.toStdString()<<"\n";return false;}
    if(!RccPackageInspector::mount(path,i)) return false;
    const bool entry=QFileInfo::exists(QStringLiteral(":/mods/%1/%2").arg(id,i.entryPath));
    RccPackageInspector::unmount(path,i); return entry;
}
int main(int argc,char **argv){
    QCoreApplication app(argc,argv); if(argc!=2) return 10; const QString d=QString::fromLocal8Bit(argv[1]);
    if(!positive(d+"/v0_5_canonical.rcc","legacy_v05_canonical",false)) return 1;
    if(!positive(d+"/v0_5_legacy_prefix.rcc","legacy_v05_prefix",true)) return 2;
    if(!positive(d+"/v0_6_services.rcc","legacy_v06_services",false)) return 3;
    if(!positive(d+"/v0_7_modern.rcc","modern_v07_fixture",false)) return 4;
    const auto bad=RccPackageInspector::inspect(d+"/malicious_legacy_namespace.rcc");
    if(bad.valid||!bad.error.contains("outside its own")) return 5;
    std::cout << "RCC backward compatibility PASS\n"; return 0;
}
