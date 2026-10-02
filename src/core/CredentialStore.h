// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QString>
#include <functional>

struct CredentialResult { bool ok = false; bool notFound = false; QString value; };
class CredentialStore : public QObject {
public:
    using Callback = std::function<void(CredentialResult)>;
    using QObject::QObject;
    virtual void read(Callback callback) = 0;
    virtual void write(const QString &value, Callback callback) = 0;
    virtual void erase(Callback callback) = 0;
};
