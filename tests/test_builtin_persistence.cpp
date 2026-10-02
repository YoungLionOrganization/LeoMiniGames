// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "Xox/XoxGame.h"
#include "Blackjack/BlackjackGame.h"
#include "Minesweeper/MinesweeperGame.h"
#include "TwentyFortyEight/TwentyFortyEightGame.h"
#include "MemoryMatch/MemoryMatchGame.h"
#include "ReactionTap/ReactionTapGame.h"
#include "core/GameSave.h"
#include "core/AppPaths.h"
#include "core/GameLifecycle.h"
#include <QGuiApplication>
#include <QTemporaryDir>
#include <QCborValue>
#include <QEventLoop>
#include <QDebug>
#include <QFile>

static int checks=0;
static void check(bool condition,const char *name) { ++checks; if (!condition) qFatal("FAIL: %s",name); }
static QVariantMap cbor(const QVariantMap &state) { return QCborValue::fromCbor(QCborValue::fromVariant(state).toCbor()).toVariant().toMap(); }
static void wait(int ms) { QEventLoop loop; QTimer::singleShot(ms,&loop,&QEventLoop::quit); loop.exec(); }
static QByteArray bytes(const QString &path) { QFile file(path); if (!file.open(QIODevice::ReadOnly)) return QByteArray{}; return file.readAll(); }

template<class Game> static void session(AppPaths &paths, const QString &id) {
    GameSave store(&paths); store.activate(id,"1.1.0",1);
    QVariantMap expected;
    {
        Game game; game.setSaveService(&store); GameLifecycle lifecycle;
        lifecycle.attach(&game,id); lifecycle.load(); lifecycle.start();
        lifecycle.pause(); expected=cbor(game.snapshot());
        check(game.suspended(),"native lifecycle pauses game");
        lifecycle.save(); check(store.forceSave(),"native lifecycle persists LMGSAVE");
        lifecycle.close(); lifecycle.unload();
    }
    GameSave reopened(&paths); reopened.activate(id,"1.1.0",1);
    Game game; game.setSaveService(&reopened); GameLifecycle lifecycle;
    lifecycle.attach(&game,id); lifecycle.load(); lifecycle.start(); lifecycle.pause();
    check(reopened.lastError().isEmpty(),"native persisted state loads without error");
    auto restored=cbor(game.snapshot());
    if (expected.contains("elapsedMs")) {
        check(restored["elapsedMs"].toLongLong()>=expected["elapsedMs"].toLongLong() && restored["elapsedMs"].toLongLong()-expected["elapsedMs"].toLongLong()<1000,"restored time excludes time outside session");
        restored.remove("elapsedMs"); expected.remove("elapsedMs");
    }
    check(restored==expected,"native session reopens same state");
    lifecycle.save(); check(reopened.forceSave(),"reopened state saves safely");
}
int main(int argc,char **argv) {
    QTemporaryDir directory; qputenv("XDG_DATA_HOME",directory.path().toUtf8()); qputenv("XDG_CONFIG_HOME",directory.path().toUtf8());
    QGuiApplication app(argc,argv); app.setOrganizationName("LMGTests"); app.setApplicationName("BuiltinPersistence");
    AppPaths paths;
    session<XoxGame>(paths,"xox"); session<BlackjackGame>(paths,"blackjack"); session<MinesweeperGame>(paths,"minesweeper");
    session<TwentyFortyEightGame>(paths,"2048"); session<MemoryMatchGame>(paths,"memory_match"); session<ReactionTapGame>(paths,"reaction_tap");
    XoxGame xox; xox.play(0); xox.play(4); XoxGame xoxCopy;
    check(xoxCopy.restoreSnapshot(cbor(xox.snapshot())) && xoxCopy.board()==xox.board() && xoxCopy.currentPlayer()=="X","XOX resumes turn and board");
    auto invalid=xox.snapshot(); invalid["board"]=QStringList{"O","O","","","","","","",""};
    check(!xoxCopy.restoreSnapshot(cbor(invalid)),"XOX invalid turns rejected");
    check(xoxCopy.restoreSnapshot({{"board",QStringList{"X","O","X","X","O","O","O","X","X"}}}) && xoxCopy.status()=="draw" && xoxCopy.currentPlayer()=="X", "XOX completed draw preserves last player");
    xox.pause(); check(!xox.play(1),"XOX rejects paused move");
    TwentyFortyEightGame tiles;
    QVariantList board; for (int i=0;i<16;++i) board.append(i<4?2:0);
    check(tiles.restoreSnapshot({{"tiles",board},{"score",0},{"moves",0}}),"2048 deterministic board accepted");
    check(tiles.move("left") && tiles.score()==8 && tiles.tiles()[0].toInt()==4 && tiles.tiles()[1].toInt()==4,"2048 merges each pair once");
    auto tileState=tiles.snapshot(); tiles.pause(); check(!tiles.move("down") && tiles.snapshot()==tileState,"2048 paused input cannot mutate state");
    invalid=tileState; auto badTiles=board; badTiles[0]=3; invalid["tiles"]=badTiles;
    check(!tiles.restoreSnapshot(invalid),"2048 non-power tile rejected atomically");
    MemoryMatchGame memory; memory.flip(0); MemoryMatchGame memoryCopy;
    check(memoryCopy.restoreSnapshot(cbor(memory.snapshot())) && memoryCopy.cards()==memory.cards(),"Memory restores selected card and layout");
    const auto cards=memory.cards(); int second=1; while(cards[second].toMap()["symbol"]==cards[0].toMap()["symbol"]) ++second;
    memory.flip(second); check(memory.locked(),"Memory mismatch fixture pending");
    check(memoryCopy.restoreSnapshot(cbor(memory.snapshot())) && !memoryCopy.locked(),"Memory pending mismatch settles on restore");
    for (const auto &card:memoryCopy.cards()) check(!card.toMap()["faceUp"].toBool(),"Memory pending mismatch cards hidden");
    MinesweeperGame mines; check(mines.openCell(0),"Mines first click safe"); mines.toggleFlag(80); mines.pause(); MinesweeperGame minesCopy; minesCopy.pause();
    check(minesCopy.restoreSnapshot(cbor(mines.snapshot())) && minesCopy.cells()==mines.cells(),"Mines restores hidden mine map and flags");
    check(minesCopy.snapshot()==mines.snapshot(),"Mines internal mine positions preserved");
    BlackjackGame blackjack; blackjack.start(); BlackjackGame handCopy;
    check(handCopy.restoreSnapshot(cbor(blackjack.snapshot())) && handCopy.playerCards()==blackjack.playerCards() && handCopy.dealerCards()==blackjack.dealerCards(),"Blackjack deck and hidden hole card resume");
    QVariantList deck; for (int token=0;token<52;++token) if (token!=0 && token!=13 && token!=8 && token!=21) deck.append(token);
    check(handCopy.restoreSnapshot({{"deck",deck},{"player",QVariantList{0,13}},{"dealer",QVariantList{8,21}},{"roundOver",false},{"message","your_turn"}}) && handCopy.playerValue()==12,"Blackjack ace values restore correctly");
    auto natural=blackjack.snapshot(); natural["player"]=QVariantList{0,9}; natural["dealer"]=QVariantList{8,21}; natural["deck"]=QVariantList{}; natural["roundOver"]=true; natural["message"]="blackjack_win";
    check(handCopy.restoreSnapshot(natural) && handCopy.playerValue()==21,"Blackjack natural terminal state restores without awarding twice");
    invalid=blackjack.snapshot(); auto player=invalid["player"].toList(); player[1]=player[0]; invalid["player"]=player;
    check(!handCopy.restoreSnapshot(invalid),"Blackjack duplicate card rejected");
    ReactionTapGame reaction; reaction.startRound(); ReactionTapGame reactionCopy;
    check(reactionCopy.restoreSnapshot(cbor(reaction.snapshot())) && reactionCopy.state()=="idle","Reaction active timing round invalidated on reopen");
    GameSave store(&paths); store.activate("guard","1.1.0",1); store.set("builtin_state",QVariantMap{{"version",1},{"state",QVariantMap{{"board",QStringList{"invalid"}}}}});
    check(store.save(),"invalid builtin-state fixture written"); const QString path=paths.saves()+"/guard/autosave.lmgsave"; const auto original=bytes(path);
    XoxGame bad; bad.setSaveService(&store); bad.load(); bad.play(0); bad.save();
    check(!store.forceSave() && bytes(path)==original,"invalid builtin save remains protected from overwrite");
    GameSave legacy(&paths); legacy.activate("legacy_builtin","1.0.0",1); check(legacy.save(),"legacy empty autosave fixture");
    XoxGame converted; converted.setSaveService(&legacy); converted.load(); converted.play(0); wait(250);
    check(legacy.load() && legacy.contains("builtin_state"),"legacy empty autosave upgraded and action checkpoint written");
    qInfo()<<"PASS:"<<checks<<"builtin persistence assertions";
}
