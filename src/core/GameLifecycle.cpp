// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "GameLifecycle.h"
#include <QMetaObject>
#include <QMetaMethod>
#include <QScopeGuard>
GameLifecycle::GameLifecycle(QObject *parent) : QObject(parent) {}
QString GameLifecycle::gameId() const { return m_gameId; }
bool GameLifecycle::active() const { return m_object && !m_closed && !m_invoking.contains("unload"); }
void GameLifecycle::attach(QObject *gameObject, const QString &gameId) {
    ++m_generation; m_invoking.clear();
    m_object=gameObject; m_gameId=gameId; m_paused=false; m_started=false; m_closed=false;
    emit gameChanged();
}
bool GameLifecycle::invoke(const char *method) {
    const QByteArray name(method);
    if (m_invoking.contains(name)) return false;
    const quint64 generation=m_generation;
    m_invoking.insert(name);
    const auto guard=qScopeGuard([this,name,generation]{if(m_generation==generation)m_invoking.remove(name);});
    if (!m_object) return true;
    const QMetaObject *meta=m_object->metaObject();
    for(int i=0;i<meta->methodCount();++i) {
        const QMetaMethod candidate=meta->method(i);
        if(candidate.name()==name && candidate.parameterCount()==0) {
            QMetaObject::invokeMethod(m_object,method,Qt::DirectConnection);break;
        }
    }
    return true;
}
void GameLifecycle::load() {
    if(!active())return; const quint64 generation=m_generation;
    if(invoke("load") && generation==m_generation && active())emit loaded();
}
void GameLifecycle::start() {
    if(!active()||m_started)return; m_started=true; const quint64 generation=m_generation;
    if(invoke("start") && generation==m_generation && active())emit started();
}
void GameLifecycle::pause() {
    if(!active()||m_paused)return; m_paused=true; const quint64 generation=m_generation;
    if(invoke("pause") && generation==m_generation && active() && m_paused)emit paused();
}
void GameLifecycle::resume() {
    if(!active()||!m_paused)return; m_paused=false; const quint64 generation=m_generation;
    if(invoke("resume") && generation==m_generation && active() && !m_paused)emit resumed();
}
void GameLifecycle::background() {
    if(!active())return; const quint64 generation=m_generation;
    if(invoke("background") && generation==m_generation && active())emit backgrounded();
}
void GameLifecycle::foreground() {
    if(!active())return; const quint64 generation=m_generation;
    if(invoke("foreground") && generation==m_generation && active())emit foregrounded();
}
void GameLifecycle::save() {
    if(!active())return; const quint64 generation=m_generation;
    if(invoke("save") && generation==m_generation && active())emit saveRequested();
}
void GameLifecycle::close() {
    if(!m_object||m_closed)return; m_closed=true; const quint64 generation=m_generation;
    if(invoke("close") && generation==m_generation)emit closed();
}
void GameLifecycle::unload() {
    if(!m_object&&m_gameId.isEmpty())return; const quint64 generation=m_generation;
    if(!invoke("unload")||generation!=m_generation)return;
    m_object.clear();m_gameId.clear();m_paused=false;m_started=false;m_closed=true;
    m_invoking.clear();const quint64 retired=++m_generation;
    emit unloaded();
    if(retired==m_generation)emit gameChanged();
}
