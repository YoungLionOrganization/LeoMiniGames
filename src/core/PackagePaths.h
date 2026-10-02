// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QDir>
#include <QFileInfo>
#include <QString>

namespace PackagePaths {
inline bool inside(const QString &root, const QString &path) {
    const QString canonicalRoot = QFileInfo(root).canonicalFilePath();
    const QFileInfo candidate(path);
    const QString canonicalPath = candidate.canonicalFilePath();
    return !canonicalRoot.isEmpty() && !canonicalPath.isEmpty() && !candidate.isSymLink() &&
        canonicalPath.startsWith(canonicalRoot + QLatin1Char('/'));
}
inline QString directory(const QString &root, const QString &id, const QString &key) {
    const QString game = QDir(root).filePath(id);
    const QString version = QDir(game).filePath(key);
    if (QFileInfo(game).isSymLink() || QFileInfo(version).isSymLink()) return QString{};
    if (!QDir().mkpath(game) || !inside(root, game) || !QDir().mkpath(version) || !inside(game, version)) return QString{};
    return QFileInfo(version).canonicalFilePath();
}
}
