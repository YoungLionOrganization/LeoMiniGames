// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QVariantMap>
#include <limits>
namespace BuiltinState {
inline bool integer(const QVariantMap &state, const QString &key, int minimum, int maximum, int &output) {
    const QVariant value = state.value(key);
    const int type = value.metaType().id();
    if (type != QMetaType::Int && type != QMetaType::LongLong && type != QMetaType::UInt && type != QMetaType::ULongLong && type != QMetaType::Double) return false;
    bool ok = false; const double number = value.toDouble(&ok);
    if (!ok || !(number >= minimum && number <= maximum)) return false;
    output = static_cast<int>(number); return number == output;
}
inline bool boolean(const QVariantMap &state, const QString &key, bool &output) {
    const QVariant value = state.value(key);
    if (value.metaType().id() != QMetaType::Bool) return false;
    output = value.toBool(); return true;
}
inline int maxCounter() { return std::numeric_limits<int>::max() / 2; }
}
