// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QString>

class QQmlEngine;
namespace RuntimeChecks {
bool graphics(QQmlEngine *engine, QString *error);
int audio();
}
