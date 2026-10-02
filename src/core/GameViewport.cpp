// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "GameViewport.h"
#include <QtGlobal>
#include <QQuickItem>
#include <QQuickWindow>
GameViewport::GameViewport(QObject *parent) : QObject(parent) {}
qreal GameViewport::safeWidth() const { return qMax<qreal>(1.0, m_width); }
qreal GameViewport::safeHeight() const { return qMax<qreal>(1.0, m_height - m_top - m_bottom); }
qreal GameViewport::safeTop() const { return qMax<qreal>(0.0, m_top); }
qreal GameViewport::safeBottom() const { return qMax<qreal>(0.0, m_bottom); }
qreal GameViewport::density() const { return qMax<qreal>(0.1, m_density); }
bool GameViewport::portrait() const { return safeHeight() >= safeWidth(); }
bool GameViewport::landscape() const { return !portrait(); }
bool GameViewport::valid() const { return m_width >= 1.0 && m_height >= 1.0; }
void GameViewport::update(qreal width, qreal height, qreal top, qreal bottom, qreal densityValue) { m_width = qMax<qreal>(1.0, width); m_height = qMax<qreal>(1.0, height); m_top = qBound<qreal>(0.0, top, m_height); m_bottom = qBound<qreal>(0.0, bottom, m_height - m_top); m_density = qMax<qreal>(0.1, densityValue); emit changed(); }

QVariantMap GameViewport::windowInsets(QObject *object) {
    qreal top = 0, bottom = 0;
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    auto *item = qobject_cast<QQuickItem *>(object);
    if (item && item->window()) {
        if (m_window != item->window()) {
            if (m_window) disconnect(m_window, nullptr, this, nullptr);
            m_window = item->window();
            connect(m_window, &QWindow::safeAreaMarginsChanged, this, &GameViewport::nativeInsetsChanged);
        }
        const QMargins margins = m_window->safeAreaMargins();
        const qreal sceneTop = item->mapToScene(QPointF(0, 0)).y();
        const qreal sceneBottom = item->mapToScene(QPointF(0, item->height())).y();
        top = qMax<qreal>(0, margins.top() - sceneTop);
        bottom = qMax<qreal>(0, margins.bottom() - (m_window->height() - sceneBottom));
    }
#else
    Q_UNUSED(object)
#endif
    return {{QStringLiteral("top"), top}, {QStringLiteral("bottom"), bottom}};
}
