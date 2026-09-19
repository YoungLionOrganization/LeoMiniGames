// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "GameLoggerFacade.h"
#include "PluginDiagnostics.h"

GameLoggerFacade::GameLoggerFacade(PluginDiagnostics *diagnostics, QObject *parent)
    : QObject(parent), m_diagnostics(diagnostics)
{
    if (m_diagnostics)
        connect(m_diagnostics, &PluginDiagnostics::lastErrorChanged, this, &GameLoggerFacade::lastErrorChanged);
}

QString GameLoggerFacade::lastError() const
{
    return m_diagnostics ? m_diagnostics->lastError() : QString{};
}

void GameLoggerFacade::log(const QString &level, const QString &message)
{
    if (m_diagnostics)
        m_diagnostics->log(level, message);
}

void GameLoggerFacade::log(const QString &level, const QString &message, const QString &source)
{
    if (m_diagnostics)
        m_diagnostics->log(level, message, source);
}

void GameLoggerFacade::log(const QString &level, const QString &message, const QString &source, int line)
{
    if (m_diagnostics)
        m_diagnostics->log(level, message, source, line);
}
