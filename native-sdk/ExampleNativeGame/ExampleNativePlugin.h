#pragma once
#include "IGamePlugin.h"

class ExampleNativeState final : public QObject {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    using QObject::QObject;
    int count() const { return m_count; }
    Q_INVOKABLE void increment() { if (m_count < 1000000) { ++m_count; emit countChanged(); } }
    Q_INVOKABLE void restore(int value) { if (value >= 0 && value <= 1000000) { m_count=value;emit countChanged(); } }
signals:
    void countChanged();
private:
    int m_count=0;
};
class ExampleNativePlugin final : public QObject, public IGamePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LeoMiniGamesPlugin_iid FILE "metadata.json")
    Q_INTERFACES(IGamePlugin)
public:
    GamePluginInfo info() const override;
    QObject *createGame(QObject *parent) override;
};
