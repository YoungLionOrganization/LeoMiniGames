// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "GameLifecycle.h"
#include <QMetaObject>
#include <QMetaMethod>
GameLifecycle::GameLifecycle(QObject *parent) : QObject(parent) {}
QString GameLifecycle::gameId() const { return m_gameId; }
void GameLifecycle::attach(QObject *gameObject, const QString &gameId) { m_object = gameObject; m_gameId = gameId; emit gameChanged(); }
void GameLifecycle::invoke(const char *method) { if (!m_object) return; const QMetaObject *mo = m_object->metaObject(); for (int i = 0; i < mo->methodCount(); ++i) { const QMetaMethod mm = mo->method(i); if (mm.name() == method && mm.parameterCount() == 0) { QMetaObject::invokeMethod(m_object, method, Qt::DirectConnection); return; } } }
void GameLifecycle::load(){invoke("load");emit loaded();} void GameLifecycle::start(){invoke("start");emit started();}
void GameLifecycle::pause(){invoke("pause");emit paused();} void GameLifecycle::resume(){invoke("resume");emit resumed();}
void GameLifecycle::background(){invoke("background");emit backgrounded();} void GameLifecycle::foreground(){invoke("foreground");emit foregrounded();}
void GameLifecycle::save(){invoke("save");emit saveRequested();} void GameLifecycle::close(){invoke("close");emit closed();}
void GameLifecycle::unload(){invoke("unload");emit unloaded(); m_object.clear(); m_gameId.clear(); emit gameChanged();}
