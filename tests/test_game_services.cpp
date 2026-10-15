// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "core/AppPaths.h"
#include "core/Achievements.h"
#include "core/GameStats.h"
#include "core/GameClock.h"
#include "core/GameInput.h"
#include <QGuiApplication>
#include <QTemporaryDir>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QCborValue>
#include <limits>
#include <cmath>

int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    QTemporaryDir directory;
    AppPaths paths(directory.path()+"/data",directory.path()+"/cache");
    int checks=0, failures=0;
    const auto check=[&](bool ok,const char *name){++checks;if(!ok){++failures;qCritical()<<"FAIL:"<<name;}};
    const double nan=std::numeric_limits<double>::quiet_NaN(), inf=std::numeric_limits<double>::infinity();
    GameClock clock;clock.setTimeScale(2.0);clock.setTimeScale(nan);
    check(clock.timeScale()==2.0,"NaN cannot change the clock speed");
    clock.setTimeScale(inf);check(clock.timeScale()==2.0,"infinity cannot change the clock speed");
    GameInput input;input.press("axis",nan);
    check(!input.isPressed("axis")&&std::isfinite(input.value("axis")),"NaN cannot create an input action");
    input.press("axis");input.setValue("axis",inf);
    check(!input.isPressed("axis")&&input.value("axis")==0.0,"invalid axis input releases stale movement");
    Achievements achievements(&paths);achievements.activate("service_fixture");
    achievements.setProgress("progress",0.25);achievements.setProgress("progress",nan);
    check(!achievements.isUnlocked("progress")&&achievements.progress("progress")==0.25,"NaN cannot unlock an achievement");
    achievements.setProgress("progress",inf);
    check(!achievements.isUnlocked("progress")&&achievements.progress("progress")==0.25,"infinity cannot unlock an achievement");
    GameStats stats(&paths);stats.activate("service_fixture");
    constexpr qint64 maximum=std::numeric_limits<qint64>::max(), minimum=std::numeric_limits<qint64>::min();
    stats.setCounter("score",maximum);check(stats.increment("score")==maximum,"counter overflow saturates at maximum");
    stats.setCounter("score",minimum);check(stats.increment("score",-1)==minimum,"counter underflow saturates at minimum");
    stats.setCounter("score",10);check(stats.increment("score",-15)==-5,"signed counter increments remain supported");
    stats.startSession();QEventLoop loop;QTimer::singleShot(25,&loop,&QEventLoop::quit);loop.exec();
    stats.startSession();stats.endSession();
    check(stats.gamesPlayed()==1,"repeated session start counts once");
    check(stats.totalTimeMs()>=20,"repeated session start preserves elapsed time");
    stats.endSession();check(stats.gamesPlayed()==1,"repeated session end is idempotent");
    QVariantMap fullMetrics;
    for(int i=0;i<4096;++i)fullMetrics.insert(QStringLiteral("counter/metric%1").arg(i),i);
    const QString fullDirectory=paths.saves()+"/full_stats";check(QDir().mkpath(fullDirectory),"full stats fixture directory");
    QFile file(fullDirectory+"/stats.cbor");check(file.open(QIODevice::WriteOnly),"full stats fixture file");
    const QByteArray payload=QCborValue::fromVariant(fullMetrics).toCbor();check(file.write(payload)==payload.size(),"full stats fixture saved");file.close();
    GameStats full(&paths);full.activate("full_stats");
    check(full.increment("new_counter")==0&&full.counter("new_counter")==0,"rejected metric insertion returns actual stored counter");
    check(full.increment("metric0")==1,"existing metrics remain writable at capacity");
    full.startSession();full.endSession();
    GameStats fullReopened(&paths);fullReopened.activate("full_stats");
    check(fullReopened.counter("metric0")==1&&fullReopened.counter("metric4095")==4095,"session accounting cannot invalidate a full stats store");
    qInfo()<<checks<<"game service assertions,"<<failures<<"failures";
    return failures?1:0;
}
