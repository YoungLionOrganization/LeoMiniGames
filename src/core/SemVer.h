// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QString>
#include <QStringList>
#include <QRegularExpression>
namespace LmgSemVer {
struct SemVer {
    int major = 0;
    int minor = 0;
    int patch = 0;
    QStringList prerelease;
    bool valid = false;
};

inline SemVer parseSemVer(QString value)
{
    static const QRegularExpression re(QStringLiteral(
        R"(^[vV]?(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-([0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*))?(?:\+[0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*)?$)"));
    value = value.trimmed();
    const QRegularExpressionMatch match = re.match(value);
    if (!match.hasMatch())
        return SemVer{};

    SemVer out;
    bool majorOk, minorOk, patchOk;
    out.major = match.captured(1).toInt(&majorOk);
    out.minor = match.captured(2).toInt(&minorOk);
    out.patch = match.captured(3).toInt(&patchOk);
    if (!majorOk || !minorOk || !patchOk) return SemVer{};
    if (!match.captured(4).isEmpty())
        out.prerelease = match.captured(4).split(QLatin1Char('.'));
    static const QRegularExpression digits(QStringLiteral("^[0-9]+$"));
    for (const QString &part : out.prerelease)
        if (digits.match(part).hasMatch() && part.size() > 1 && part.startsWith(QLatin1Char('0'))) return SemVer{};
    out.valid = true;
    return out;
}

inline int compareIdentifier(const QString &left, const QString &right)
{
    static const QRegularExpression digits(QStringLiteral("^[0-9]+$"));
    const bool leftNumeric = digits.match(left).hasMatch();
    const bool rightNumeric = digits.match(right).hasMatch();
    if (leftNumeric && rightNumeric) {
        if (left.size() != right.size()) return left.size() < right.size() ? -1 : 1;
        const int comparison = QString::compare(left, right, Qt::CaseSensitive);
        return comparison < 0 ? -1 : comparison > 0 ? 1 : 0;
    }
    if (leftNumeric != rightNumeric) return leftNumeric ? -1 : 1;
    const int result = QString::compare(left, right, Qt::CaseSensitive);
    return result < 0 ? -1 : (result > 0 ? 1 : 0);
}

inline int compareSemVer(const SemVer &left, const SemVer &right)
{
    if (left.major != right.major)
        return left.major < right.major ? -1 : 1;
    if (left.minor != right.minor)
        return left.minor < right.minor ? -1 : 1;
    if (left.patch != right.patch)
        return left.patch < right.patch ? -1 : 1;

    if (left.prerelease.isEmpty() && right.prerelease.isEmpty())
        return 0;
    if (left.prerelease.isEmpty())
        return 1;
    if (right.prerelease.isEmpty())
        return -1;

    const qsizetype common = qMin(left.prerelease.size(), right.prerelease.size());
    for (qsizetype i = 0; i < common; ++i) {
        const int result = compareIdentifier(left.prerelease.at(i), right.prerelease.at(i));
        if (result != 0)
            return result;
    }
    if (left.prerelease.size() == right.prerelease.size())
        return 0;
    return left.prerelease.size() < right.prerelease.size() ? -1 : 1;
}

}
