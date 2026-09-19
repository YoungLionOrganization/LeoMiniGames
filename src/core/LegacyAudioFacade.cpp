// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "LegacyAudioFacade.h"
#include "AudioManager.h"
#include <QDir>
#include <QFileInfo>

LegacyAudioFacade::LegacyAudioFacade(AudioManager *audio, QObject *parent)
    : QObject(parent), m_audio(audio) {}

bool LegacyAudioFacade::available() const { return m_audio && m_audio->available(); }
void LegacyAudioFacade::activate(const QString &gameId)
{
    if (m_audio && !m_gameId.isEmpty() && m_gameId != gameId)
        m_audio->releasePrefix(QStringLiteral("qrc:/mods/%1/").arg(m_gameId));
    m_gameId = gameId.trimmed();
}
QStringList LegacyAudioFacade::capabilities() const { return m_audio ? m_audio->capabilities() : QStringList{}; }
void LegacyAudioFacade::play(const QString &name) { if (m_audio) m_audio->play(name); }

QUrl LegacyAudioFacade::normalized(const QUrl &url) const
{
    if (!url.isValid() || url.isEmpty() || m_gameId.isEmpty())
        return QUrl{};
    if (url.scheme().isEmpty()) {
        QString relative = QDir::cleanPath(url.toString());
        if (relative.isEmpty() || relative == QStringLiteral(".") || relative.startsWith(QStringLiteral("../")) || relative.contains(QLatin1Char('\\')))
            return QUrl{};
        const QString resource = QStringLiteral(":/mods/%1/%2").arg(m_gameId, relative);
        if (QFileInfo::exists(resource))
            return QUrl(QStringLiteral("qrc:/mods/%1/%2").arg(m_gameId, relative));
        return QUrl{};
    }
    if (url.scheme().compare(QStringLiteral("qrc"), Qt::CaseInsensitive) != 0)
        return QUrl{};
    const QString path = QDir::cleanPath(url.path());
    const QString prefix = QStringLiteral("/mods/%1/").arg(m_gameId);
    if (!path.startsWith(prefix) || path.contains(QStringLiteral("/../")))
        return QUrl{};
    return QUrl(QStringLiteral("qrc:%1").arg(path));
}

void LegacyAudioFacade::playUrl(const QUrl &url) { playUrl(url, 1.0); }
void LegacyAudioFacade::playUrl(const QUrl &url, qreal gain)
{
    if (!m_audio)
        return;
    if (url.scheme().isEmpty() && !url.toString().contains(QLatin1Char('/')) && !url.toString().contains(QLatin1Char('.'))) {
        m_audio->play(url.toString());
        return;
    }
    const QUrl safe = normalized(url);
    if (!safe.isEmpty())
        m_audio->playUrl(safe, gain);
}
void LegacyAudioFacade::preload(const QUrl &url) { preload(url, 1.0); }
void LegacyAudioFacade::preload(const QUrl &url, qreal gain)
{
    Q_UNUSED(gain)
    if (!m_audio)
        return;
    const QUrl safe = normalized(url);
    if (!safe.isEmpty())
        m_audio->preload(safe);
}
void LegacyAudioFacade::stopAll() { if (m_audio) m_audio->stopAll(); }
