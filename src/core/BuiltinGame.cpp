// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "BuiltinGame.h"
#include "GameSave.h"
#include "sdk/BuiltinState.h"
#include <QMetaMethod>

BuiltinGame::BuiltinGame(QObject *parent) : QObject(parent) {
    m_checkpoint.setSingleShot(true);
    m_checkpoint.setInterval(150);
    connect(&m_checkpoint, &QTimer::timeout, this, [this] { save(); if (m_save) m_save->autosave(); });
}
void BuiltinGame::setSaveService(GameSave *save) {
    m_save = save;
    const QMetaMethod slot = metaObject()->method(metaObject()->indexOfSlot("scheduleCheckpoint()"));
    for (const char *signal : {"boardChanged()", "cardsChanged()", "cellsChanged()", "stateChanged()", "statsChanged()", "scoreChanged()", "boardReset()"}) {
        const int index = metaObject()->indexOfSignal(signal);
        if (index >= 0) connect(this, metaObject()->method(index), this, slot, Qt::UniqueConnection);
    }
}
void BuiltinGame::load() {
    if (!m_save) return;
    m_restoring = true;
    m_loaded = false;
    if (!m_save->hasStoredSlot(QStringLiteral("autosave"))) m_loaded = true;
    else if (m_save->load()) {
        // v0.7.2 used empty autosaves; statistics remain in local_stats.cbor.
        if (!m_save->contains(QStringLiteral("builtin_state"))) m_loaded = true;
        else {
            const QVariantMap envelope = m_save->get(QStringLiteral("builtin_state")).toMap();
            int version = 0;
            if (BuiltinState::integer(envelope, QStringLiteral("version"), 1, 1, version) &&
                envelope.value(QStringLiteral("state")).metaType() == QMetaType::fromType<QVariantMap>())
                m_loaded = restoreSnapshot(envelope.value(QStringLiteral("state")).toMap());
            if (!m_loaded) m_save->protectSlot(QStringLiteral("autosave"), tr("Built-in game state is invalid; preserving the existing save."));
        }
    }
    m_restoring = false;
}
void BuiltinGame::save() {
    m_checkpoint.stop();
    if (!m_save || !m_loaded) return;
    m_save->set(QStringLiteral("builtin_state"), QVariantMap{{QStringLiteral("version"), 1}, {QStringLiteral("state"), snapshot()}});
}
void BuiltinGame::scheduleCheckpoint() { if (m_loaded && !m_restoring && !m_checkpoint.isActive()) m_checkpoint.start(); }
void BuiltinGame::pause() { if (!m_suspended) { m_suspended = true; emit suspensionChanged(); } }
void BuiltinGame::resume() { if (m_suspended) { m_suspended = false; emit suspensionChanged(); } }
void BuiltinGame::close() { m_checkpoint.stop(); pause(); }
void BuiltinGame::unload() { m_checkpoint.stop(); m_save.clear(); m_loaded = false; }
