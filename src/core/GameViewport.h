// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>

class GameViewport final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal safeWidth READ safeWidth NOTIFY changed)
    Q_PROPERTY(qreal safeHeight READ safeHeight NOTIFY changed)
    Q_PROPERTY(qreal safeTop READ safeTop NOTIFY changed)
    Q_PROPERTY(qreal safeBottom READ safeBottom NOTIFY changed)
    Q_PROPERTY(qreal density READ density NOTIFY changed)
    Q_PROPERTY(bool portrait READ portrait NOTIFY changed)
    Q_PROPERTY(bool landscape READ landscape NOTIFY changed)
    Q_PROPERTY(bool valid READ valid NOTIFY changed)
public:
    explicit GameViewport(QObject *parent = nullptr);
    qreal safeWidth() const; qreal safeHeight() const; qreal safeTop() const; qreal safeBottom() const; qreal density() const;
    bool portrait() const; bool landscape() const; bool valid() const;
    Q_INVOKABLE void update(qreal width, qreal height, qreal top = 0.0, qreal bottom = 0.0, qreal density = 1.0);
signals: void changed();
private:
    qreal m_width = 1.0, m_height = 1.0, m_top = 0.0, m_bottom = 0.0, m_density = 1.0;
};
