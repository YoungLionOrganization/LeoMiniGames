// SPDX-License-Identifier: GPL-3.0-or-later
#include "LegacyAppFacade.h"
#include "AppController.h"
LegacyAppFacade::LegacyAppFacade(AppController *controller,QObject *parent):QObject(parent),m_controller(controller)
{
    if(m_controller) connect(m_controller,&AppController::currentGameChanged,this,&LegacyAppFacade::currentGameChanged);
}
QUrl LegacyAppFacade::currentGameUrl() const { return m_controller ? m_controller->currentGameUrl() : QUrl(); }
QString LegacyAppFacade::currentGameId() const { return m_controller ? m_controller->currentGameId() : QString(); }
QString LegacyAppFacade::currentGameVersion() const { return m_controller ? m_controller->currentGameVersion() : QString(); }
QString LegacyAppFacade::currentGameSource() const { return m_controller ? m_controller->currentGameSource() : QString(); }
void LegacyAppFacade::closeGame(){ if(m_controller) m_controller->closeGame(); }
