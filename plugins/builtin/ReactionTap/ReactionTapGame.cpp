// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "ReactionTapGame.h"

#include <QRandomGenerator>
#include "sdk/GameLocalStats.h"
#include "sdk/BuiltinState.h"

ReactionTapGame::ReactionTapGame(QObject *parent) : BuiltinGame(parent)
{
    GameLocalStats::migrateLegacy(QStringLiteral("reaction_tap"), {
        {QStringLiteral("games/reaction/bestMs"), QStringLiteral("custom/bestMs")}
    });
    m_bestMs = GameLocalStats::value(QStringLiteral("reaction_tap"), QStringLiteral("custom/bestMs"), 0).toInt();
    m_waitTimer.setSingleShot(true);
    connect(&m_waitTimer, &QTimer::timeout, this, [this] {
        m_state = QStringLiteral("ready");
        m_elapsed.restart();
        emit stateChanged();
    });
}

QString ReactionTapGame::state() const { return m_state; }
int ReactionTapGame::lastMs() const { return m_lastMs; }
int ReactionTapGame::bestMs() const { return m_bestMs; }
int ReactionTapGame::rounds() const { return m_rounds; }
int ReactionTapGame::averageMs() const
{
    return m_rounds > 0 ? static_cast<int>(m_totalMs / m_rounds) : 0;
}

void ReactionTapGame::startRound()
{
    if (m_paused) return;
    m_elapsedBeforePause = 0;
    m_waitTimer.stop();
    m_state = QStringLiteral("waiting");
    const int delay = 1200 + QRandomGenerator::global()->bounded(2301);
    m_waitTimer.start(delay);
    emit stateChanged();
}

void ReactionTapGame::tap()
{
    if (m_paused) return;
    if (m_state == QStringLiteral("waiting")) {
        m_waitTimer.stop();
        m_state = QStringLiteral("false_start");
        emit stateChanged();
        return;
    }

    if (m_state != QStringLiteral("ready"))
        return;

    m_lastMs = static_cast<int>(m_elapsedBeforePause + m_elapsed.elapsed());
    ++m_rounds;
    m_totalMs += m_lastMs;
    if (m_bestMs == 0 || m_lastMs < m_bestMs) {
        m_bestMs = m_lastMs;
        GameLocalStats::setValues(QStringLiteral("reaction_tap"), {
            {QStringLiteral("custom/bestMs"), m_bestMs}
        });
    }
    m_state = QStringLiteral("result");
    emit statsChanged();
    emit stateChanged();
}

void ReactionTapGame::pause() {
    BuiltinGame::pause();
    if (m_paused) return;
    m_paused = true;
    m_waitTimer.stop();
    if (m_state == QStringLiteral("waiting") || m_state == QStringLiteral("ready")) {
        m_state = QStringLiteral("idle"); m_elapsed.invalidate(); m_elapsedBeforePause = 0;
        emit stateChanged();
    }
}
void ReactionTapGame::resume() {
    BuiltinGame::resume(); m_paused = false; }

QVariantMap ReactionTapGame::snapshot() const {
    // Wall-clock reaction rounds cannot resume fairly after process suspension.
    return {{"state",m_state=="waiting" || m_state=="ready" ? QStringLiteral("idle") : m_state}, {"lastMs",m_lastMs}, {"rounds",m_rounds}, {"totalMs",m_totalMs}};
}
bool ReactionTapGame::restoreSnapshot(const QVariantMap &state) {
    int last,rounds; bool ok=false;
    const QVariant totalValue=state.value("totalMs");
    if (totalValue.metaType().id()!=QMetaType::LongLong && totalValue.metaType().id()!=QMetaType::Int) return false;
    const qint64 total=totalValue.toLongLong(&ok); const QString status=state.value("state").toString();
    if (!BuiltinState::integer(state,"lastMs",0,3600000,last) || !BuiltinState::integer(state,"rounds",0,BuiltinState::maxCounter(),rounds) || !ok || total<last || total>qint64(rounds)*3600000 || (rounds==0 && (last!=0 || total!=0)) || (status!="idle" && status!="result" && status!="false_start")) return false;
    if (status=="result" && rounds==0) return false;
    m_waitTimer.stop(); m_elapsed.invalidate(); m_elapsedBeforePause=0; m_state=status; m_lastMs=last; m_rounds=rounds; m_totalMs=total;
    emit stateChanged(); emit statsChanged(); return true;
}
