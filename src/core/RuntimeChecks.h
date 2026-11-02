// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QString>

class QQmlEngine;
namespace RuntimeChecks {
bool graphics(QQmlEngine *engine, QString *error);
bool stageMod(const QString &source, const QString &modsRoot, QString *id, QString *error);
int audio();
}
