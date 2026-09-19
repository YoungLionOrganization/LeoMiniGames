// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "GameClock.h"
#include <QtGlobal>
GameClock::GameClock(QObject *parent) : QObject(parent) { m_timer.start(); }
bool GameClock::paused() const { return m_paused; }
double GameClock::timeScale() const { return m_timeScale; }
void GameClock::setTimeScale(double scale) { scale = qBound(0.0, scale, 8.0); if (qFuzzyCompare(m_timeScale, scale)) return; if (!m_paused) { m_accumulatedMs += static_cast<qint64>(m_timer.elapsed() * m_timeScale); m_timer.restart(); } m_timeScale = scale; emit timeScaleChanged(); }
void GameClock::reset() { m_accumulatedMs = 0; m_timer.restart(); }
void GameClock::pause() { if (m_paused) return; m_accumulatedMs += static_cast<qint64>(m_timer.elapsed() * m_timeScale); m_paused = true; emit pausedChanged(); }
void GameClock::resume() { if (!m_paused) return; m_timer.restart(); m_paused = false; emit pausedChanged(); }
qint64 GameClock::elapsedMs() const { return m_accumulatedMs + (m_paused ? 0 : static_cast<qint64>(m_timer.elapsed() * m_timeScale)); }
double GameClock::elapsedSeconds() const { return elapsedMs() / 1000.0; }
