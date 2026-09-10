// SPDX-License-Identifier: GPL-3.0-or-later
#include "GameLifecycleFacade.h"
#include "GameLifecycle.h"
GameLifecycleFacade::GameLifecycleFacade(GameLifecycle *lifecycle, QObject *parent)
    : QObject(parent), m_lifecycle(lifecycle)
{
    if (!m_lifecycle)
        return;
    connect(m_lifecycle, &GameLifecycle::gameChanged, this, &GameLifecycleFacade::gameChanged);
    connect(m_lifecycle, &GameLifecycle::loaded, this, &GameLifecycleFacade::loaded);
    connect(m_lifecycle, &GameLifecycle::started, this, &GameLifecycleFacade::started);
    connect(m_lifecycle, &GameLifecycle::paused, this, &GameLifecycleFacade::paused);
    connect(m_lifecycle, &GameLifecycle::resumed, this, &GameLifecycleFacade::resumed);
    connect(m_lifecycle, &GameLifecycle::backgrounded, this, &GameLifecycleFacade::backgrounded);
    connect(m_lifecycle, &GameLifecycle::foregrounded, this, &GameLifecycleFacade::foregrounded);
    connect(m_lifecycle, &GameLifecycle::saveRequested, this, &GameLifecycleFacade::saveRequested);
    connect(m_lifecycle, &GameLifecycle::closed, this, &GameLifecycleFacade::closed);
    connect(m_lifecycle, &GameLifecycle::unloaded, this, &GameLifecycleFacade::unloaded);
}
QString GameLifecycleFacade::gameId() const { return m_lifecycle ? m_lifecycle->gameId() : QString{}; }
void GameLifecycleFacade::load() { if (m_lifecycle) m_lifecycle->load(); }
void GameLifecycleFacade::start() { if (m_lifecycle) m_lifecycle->start(); }
void GameLifecycleFacade::pause() { if (m_lifecycle) m_lifecycle->pause(); }
void GameLifecycleFacade::resume() { if (m_lifecycle) m_lifecycle->resume(); }
void GameLifecycleFacade::background() { if (m_lifecycle) m_lifecycle->background(); }
void GameLifecycleFacade::foreground() { if (m_lifecycle) m_lifecycle->foreground(); }
void GameLifecycleFacade::save() { if (m_lifecycle) m_lifecycle->save(); }
void GameLifecycleFacade::close() { if (m_lifecycle) m_lifecycle->close(); }
void GameLifecycleFacade::unload() { if (m_lifecycle) m_lifecycle->unload(); }
