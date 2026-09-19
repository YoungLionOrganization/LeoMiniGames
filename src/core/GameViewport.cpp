// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "GameViewport.h"
#include <QtGlobal>
GameViewport::GameViewport(QObject *parent) : QObject(parent) {}
qreal GameViewport::safeWidth() const { return qMax<qreal>(1.0, m_width); }
qreal GameViewport::safeHeight() const { return qMax<qreal>(1.0, m_height - m_top - m_bottom); }
qreal GameViewport::safeTop() const { return qMax<qreal>(0.0, m_top); }
qreal GameViewport::safeBottom() const { return qMax<qreal>(0.0, m_bottom); }
qreal GameViewport::density() const { return qMax<qreal>(0.1, m_density); }
bool GameViewport::portrait() const { return safeHeight() >= safeWidth(); }
bool GameViewport::landscape() const { return !portrait(); }
bool GameViewport::valid() const { return m_width >= 1.0 && m_height >= 1.0; }
void GameViewport::update(qreal width, qreal height, qreal top, qreal bottom, qreal densityValue) { m_width = qMax<qreal>(1.0, width); m_height = qMax<qreal>(1.0, height); m_top = qBound<qreal>(0.0, top, m_height); m_bottom = qBound<qreal>(0.0, bottom, m_height - m_top); m_density = qMax<qreal>(0.1, densityValue); emit changed(); }
