// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#pragma once
#include <QObject>
#include <QHash>
#include <QPointer>

class GameInput final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
public:
    explicit GameInput(QObject *parent = nullptr);
    bool ready() const { return true; }
    QString version() const { return QStringLiteral("0.7"); }
    Q_INVOKABLE void press(const QString &action);
    Q_INVOKABLE void press(const QString &action, qreal value);
    Q_INVOKABLE void release(const QString &action);
    Q_INVOKABLE void setValue(const QString &action, qreal value);
    Q_INVOKABLE bool isPressed(const QString &action) const;
    Q_INVOKABLE qreal value(const QString &action) const;
    Q_INVOKABLE bool handleKey(int key, quint32 nativeScanCode, const QString &text, bool pressed, bool autoRepeat = false);
    Q_INVOKABLE QString actionForKey(int key, quint32 nativeScanCode, const QString &text) const;
    Q_INVOKABLE QStringList capabilities() const;
    Q_INVOKABLE void reset();
    Q_INVOKABLE void setFocusRoot(QObject *root);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
signals:
    void actionPressed(const QString &action, qreal value);
    void actionReleased(const QString &action);
    void actionValueChanged(const QString &action, qreal value);
private:
    QHash<QString, qreal> m_values;
    QHash<quint64, QString> m_physicalKeys;
    QPointer<QObject> m_focusRoot;
};
