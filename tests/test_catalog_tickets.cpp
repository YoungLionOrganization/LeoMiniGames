// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "FakeNetwork.h"
#include "core/AppPaths.h"
#include "core/ModManager.h"
#include "core/SettingsManager.h"
#include "core/ThemeCatalogManager.h"
#include "core/ThemeManager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <cstdlib>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("LMGTests"); app.setApplicationName("CatalogTickets");
    app.setApplicationVersion("0.7.3");
    int assertions = 0;
    const auto check = [&](bool value, const char *message) {
        ++assertions;
        if (!value) { qCritical("FAIL: %s", message); std::exit(1); }
    };
    const QString digest(64, 'a');
    const QJsonObject entry{{"id", "ticket_fixture"}, {"version", "1.0.0"},
                            {"sha256", digest}, {"size_bytes", 16}};
    const auto catalog = [&](const QJsonObject &row) {
        return QJsonDocument(QJsonObject{{"data", QJsonArray{row}}}).toJson();
    };
    const auto ticket = [&](QString sha, int size) {
        return QJsonObject{{"url", "https://assets.example.invalid/package.rcc"},
                           {"sha256", sha}, {"size_bytes", size}};
    };
    for (const int scenario : {0, 1, 2, 3, 4}) {
        QTemporaryDir temp;
        AppPaths paths(temp.path()+"/data", temp.path()+"/cache");
        SettingsManager settings(temp.filePath("settings.ini"));
        FakeNetwork network;
        ModManager mods(&paths, &settings, nullptr, &network);
        mods.refresh(); network.last->complete(catalog(entry));
        mods.install("ticket_fixture");
        check(network.requests == 2, "mod requests a download ticket");
        QJsonObject response = ticket(scenario == 0 ? QString(64, 'b') : digest,
                                      scenario == 1 ? 17 : 16);
        if (scenario == 2) { response.remove("sha256"); response.remove("size_bytes"); }
        if (scenario == 4) settings.setValue("mods/apiBaseUrl", "https://other.example.invalid/api/v1");
        network.last->complete(QJsonDocument(QJsonObject{{"data", response}}).toJson());
        if (scenario == 2 || scenario == 3) {
            check(network.requests == 3, "matching or legacy omitted ticket metadata uses catalog identity");
            network.last->abort();
        } else {
            check(network.requests == 2, "changed ticket digest, size or origin cannot start package download");
            check(!mods.data(mods.index(0,0), ModManager::ErrorRole).toString().isEmpty(),
                  "ticket rejection reports a row error");
        }
    }
    {
        QTemporaryDir temp;
        AppPaths paths(temp.path()+"/data", temp.path()+"/cache");
        SettingsManager settings(temp.filePath("settings.ini"));
        ThemeManager installed(&paths);
        FakeNetwork network;
        ThemeCatalogManager themes(&paths, &settings, &installed, nullptr, &network);
        themes.refresh(); network.last->complete(catalog(entry));
        themes.install("ticket_fixture");
        check(network.requests == 2, "theme requests a download ticket");
        settings.setValue("themes/apiBaseUrl", "https://other.example.invalid/api/v1");
        network.last->complete(QJsonDocument(QJsonObject{{"data", ticket(digest, 16)}}).toJson());
        check(network.requests == 2, "changed theme origin cannot start package download");
        check(!themes.data(themes.index(0,0), ThemeCatalogManager::OperationErrorRole).toString().isEmpty(),
              "theme ticket rejection reports a row error");
    }
    {
        QTemporaryDir temp;
        AppPaths paths(temp.path()+"/data", temp.path()+"/cache");
        SettingsManager settings(temp.filePath("settings.ini"));
        settings.setValue("themes/apiBaseUrl", "https://example-user:unused@catalog.example.invalid/api/v1");
        FakeNetwork network;
        ThemeCatalogManager themes(&paths, &settings, nullptr, nullptr, &network);
        themes.refresh();
        check(network.requests == 0 && !themes.error().isEmpty(), "theme catalog rejects embedded URL credentials");
    }
    {
        QTemporaryDir temp;
        AppPaths paths(temp.path()+"/data", temp.path()+"/cache");
        SettingsManager settings(temp.filePath("settings.ini"));
        FakeNetwork network;
        ModManager mods(&paths, &settings, nullptr, &network);
        QJsonObject row = entry;
        row.insert("download_endpoint", "https://example-user:unused@leominigames.younglion.xyz/api/v1/mods/ticket_fixture/download");
        mods.refresh(); network.last->complete(catalog(row));
        mods.install("ticket_fixture");
        check(network.requests == 1, "mod ticket endpoint rejects embedded URL credentials");
    }
    for (const bool direct : {false, true}) {
        QTemporaryDir temp;
        AppPaths paths(temp.path()+"/data", temp.path()+"/cache");
        SettingsManager settings(temp.filePath("settings.ini"));
        ThemeManager installed(&paths);
        FakeNetwork network;
        ThemeCatalogManager themes(&paths, &settings, &installed, nullptr, &network);
        QJsonObject row = entry;
        const QString url = "https://example-user:unused@assets.example.invalid/theme.rcc";
        if (direct) row.insert("download_url", url);
        themes.refresh(); network.last->complete(catalog(row));
        themes.install("ticket_fixture");
        if (!direct) {
            QJsonObject response = ticket(digest, 16);
            response.insert("url", url);
            network.last->complete(QJsonDocument(QJsonObject{{"data", response}}).toJson());
        }
        check(network.requests == (direct ? 1 : 2), "theme downloads reject embedded URL credentials");
        check(!themes.data(themes.index(0,0), ThemeCatalogManager::OperationErrorRole).toString().isEmpty(),
              "rejected theme download clears resolving state with a row error");
    }
    qInfo() << "PASS:" << assertions << "catalog ticket assertions";
}
