// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "FakeNetwork.h"
#include "core/SettingsManager.h"
#include "core/UpdateService.h"
#include <QCoreApplication>
#include <QDebug>
#include <QPointer>
#include <QTemporaryDir>
#include <QUrlQuery>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setApplicationVersion("0.7.3");
    QTemporaryDir directory;
    SettingsManager settings(directory.filePath("settings.ini"));
    int assertions = 0;
    const auto check = [&](bool condition, const char *message) {
        ++assertions;
        if (!condition) qFatal("FAIL: %s", message);
    };
    {
        FakeNetwork network;
        UpdateService updates(&settings, nullptr, &network);
        bool replaced = false;
        QObject::connect(&updates, &UpdateService::updateStateChanged, &updates, [&] {
            if (!replaced && updates.status() == "idle") {
                replaced = true;
                updates.checkForUpdates();
            }
        });
        updates.checkForUpdates();
        check(network.requests == 1 && updates.checking(), "nested idle request owns one network reply");
    }
    {
        FakeNetwork network;
        UpdateService updates(&settings, nullptr, &network);
        QObject::connect(&updates, &UpdateService::updateStateChanged, &updates, [&] {
            if (updates.checking()) updates.clearResult();
        });
        updates.checkForUpdates();
        check(network.requests == 0 && !updates.checking() && updates.status() == "idle",
              "clearing the checking notification prevents a late request");
    }
    {
        FakeNetwork network;
        settings.setValue("updates/channel", "stable");
        UpdateService updates(&settings, nullptr, &network);
        bool replaced = false;
        QObject::connect(&updates, &UpdateService::updateStateChanged, &updates, [&] {
            if (!replaced && updates.checking()) {
                replaced = true;
                updates.setChannel("preview");
                updates.checkForUpdates();
            }
        });
        updates.checkForUpdates();
        check(network.requests == 1 && updates.checking(), "replacement channel request retains ownership");
        check(QUrlQuery(network.last->request().url()).queryItemValue("channel") == "preview",
              "replacement request uses its selected channel");
        network.last->complete(R"({"ok":true,"data":{"schema":1,"provider_contract":"leominigames-update-v1","release":{"version":"0.7.4-rc.1","page_url":"https://leominigames.younglion.xyz/updates/release"}}})");
        check(updates.updateAvailable() && !updates.checking(), "replacement preview response is retained");
    }
    for (const int phase : {0, 1, 2}) {
        FakeNetwork network;
        settings.setValue("updates/channel", "stable");
        QPointer<UpdateService> updates = new UpdateService(&settings, nullptr, &network);
        QObject::connect(updates, &UpdateService::updateStateChanged, updates, [&] {
            if (phase == 1 && !updates->checking()) return;
            delete updates.data();
        });
        if (phase == 2) updates->setChannel("preview");
        else updates->checkForUpdates();
        check(!updates && network.requests == 0, "state observer may retire the service before request creation");
    }
    qInfo() << "PASS:" << assertions << "update reentrancy assertions";
}
