/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at developer.feedback@nokia.com.
**
****************************************************************************/

#ifndef HBPOPUP_P_H
#define HBPOPUP_P_H
#include <QPointer>

#include "hbpopup.h"
#include "hbwidget_p.h"
#include "hbnamespace_p.h"
#include "hbvgmaskeffect_p.h"
#ifdef HB_EFFECTS
#include "hbeffect.h"
#endif // HB_EFFECTS

class HbPopupBackGround;

QT_FORWARD_DECLARE_CLASS(QEventLoop)
QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QGraphicsWidget)
QT_FORWARD_DECLARE_CLASS(QGraphicsSceneMouseEvent)

class HB_CORE_PRIVATE_EXPORT HbPopupPrivate :public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbPopup)

public:
    HbPopupPrivate();
    virtual ~HbPopupPrivate();
    void init();

    /* Priority of popups within single app. Only VirtualKeyboard
     * currently needs this. If need arises, please add your priority
     * here instead of using magic numbers.
     */
    enum Priority {
        Minimum = 0,
        Default = Minimum,
        VirtualKeyboard = 127,
        AlwaysOnTop = 255
    };

// Private features
public:
    quint8 priority() const { return priorityValue; }
    void setPriority( quint8 priority );

public:
    QEventLoop *eventLoop;
    bool hasEffects;
    bool closed;
    bool hidingInProgress;
    bool delayedHide;
    bool deleteOnClose;
    bool modal;
    bool fadeBackground;
    bool inDestruction;

    //TODO
    // This is a temporay solution for a Qt bug that causes show event sent twice for
    // graphics items that hidden by default. This flag makes sure that aboutToShowSignal
    // is emitted only once during the popup becomes visible.
    // This can be removed after Qt fixes this bug.
    bool aboutToShowSignalGuard;
    bool duplicateShowEvent;

    bool timedOut;
    int timeout;

    quint8 priorityValue;

    HbPopup::DismissPolicy dismissPolicy;
    HbPopupBackGround *backgroundItem;

    enum MouseEventLocationType {
        None,
        Background,
        Popup
    };

    MouseEventLocationType mousePressLocation;
    HbPopup::FrameType frameType;
    QPointF preferredPos;
    bool preferredPosSet;
    HbPopup::Placement placement;
    bool mStartEffect;
    QPointer<QObject> receiverToDisconnectOnClose;
    QByteArray memberToDisconnectOnClose;
    qreal mScreenMargin;
    bool mAutoLayouting;

public:
#ifdef HB_EFFECTS
    void _q_delayedHide(HbEffect::EffectStatus status);
    void _q_orientationAboutToChange(Qt::Orientation orient, bool animate);
#endif // HB_EFFECTS
    void _q_timeoutFinished();
    void _q_orientationChanged();

    bool addPopupToScene();
    void handleBackgroundMousePressEvent();
    void handleBackgroundMouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void calculateShape();

    static int timeoutValue(HbPopup::DefaultTimeout timeout);
    void setTimeout(int msec);

    void forceHide();
    virtual void addPopupEffects();

    void stopTimeout();
    void startTimeout();
    QTimer *timeoutTimer();
    void doSetModal( bool modal );
    QString effectType;
    HbVgMaskEffect *mVgMaskEffect;
    bool mOrientationEffectHide;
private:
    static bool popupEffectsLoaded;
    static HbPopupPrivate *d_ptr(HbPopup *popup) {
        Q_ASSERT(popup);
        return popup->d_func();
    }
    friend class HbPopupBackGround;
    friend class HbPopupManagerPrivate;
    friend class HbPopupLayoutProxy;
    friend class HbDeviceDialogManagerPrivate;

    // To be able to unit test private features
    friend class TestHbPopup;

    QTimer *timeoutTimerInstance;
};

class HbPopupBackGround : public QGraphicsItem
{
public:
    explicit HbPopupBackGround(HbPopup *popup, QGraphicsItem *parent = 0);
    ~HbPopupBackGround();
    void setRect(QRectF rect);
    bool isModal() {return popup->isModal();}
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    enum { Type = HbPrivate::ItemType_PopupBackGround };
    int type() const { return Type; }

protected:
    bool sceneEvent(QEvent *event);

private:
    mutable QRectF mRect;
    HbPopup *popup;
    friend class HbPopup;
};

#endif // HBPOPUP_P_H
