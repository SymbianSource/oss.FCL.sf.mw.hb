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
#ifndef HBABSTRACTVKBHOST_P_H
#define HBABSTRACTVKBHOST_P_H

#include <QGraphicsObject>
#include <QWidget>
#include <QTimeLine>
#include <QPointer>
#include <QPointF>
#include <QRectF>
#include <QSizeF>

#include "hbinputvkbhost.h"

#include <hbview.h>

class QGraphicsWidget;
class HbMainWindow;
class HbVirtualKeyboard;
class HbAbstractVkbHost;
class HbInputFocusObject;
class HbVkbHostContainerWidget;

class HbPendingVkbHostCall
{
public:
    HbPendingVkbHostCall() : vkb(0),
                             animationAllowed(false),
                             popupOpening(false)
    {}

public:
    HbVirtualKeyboard *vkb;
    bool animationAllowed;
    bool popupOpening;
};

class HB_CORE_PRIVATE_EXPORT HbAbstractVkbHostPrivate
{
    Q_DECLARE_PUBLIC(HbAbstractVkbHost)
public:
    HbAbstractVkbHostPrivate(HbAbstractVkbHost *myVkbHost, QObject *containerWidget);
    virtual ~HbAbstractVkbHostPrivate();

    virtual void openKeypad();
    virtual void closeKeypad();
    virtual void openKeypadWithoutAnimation();
    virtual void closeKeypadWithoutAnimation();
    virtual void cancelAnimationAndHideVkbWidget();
    virtual bool prepareContainerAnimation(HbVkbHost::HbVkbStatus status);
    virtual bool prepareKeypadAnimation(HbVkbHost::HbVkbStatus status);

    void prepareAnimationsCommon();
    bool prepareAnimations(HbVkbHost::HbVkbStatus status);

    virtual void connectSignals();
    virtual void disconnectSignals();

    HbMainWindow *mainWindow() const;
    QSizeF screenSize() const;
    bool disableCursorShift();
    bool getViewAndFocusObjects(HbView*& currentView, HbInputFocusObject*& focusObject);
    void _q_containerAboutToClose();
    void ensureVisibilityInsideScrollArea() const;
    void ensureVisibilityInsideVisibleArea() const;
    QRectF findEditorRect(HbInputFocusObject* fo) const;
    QPointF selectLongestVerticalVector(const QPointF &v1, const QPointF &v2) const;
    bool popupAnimationInProgress() const;

public:
    HbAbstractVkbHost *q_ptr;
    HbVirtualKeyboard *mCallback;
    QPointer<QGraphicsWidget> mKeypad;
    HbVkbHostContainerWidget *mContainerWidget;
    QSizeF mScreenSize;
    QTimeLine mTimeLine;
    HbVkbHost::HbVkbStatus mKeypadStatus;
    bool mKeypadOperationOngoing;
    QPointF mOriginalContainerPosition;
    QPointF mContainerMovementStartingPoint;
    QPointF mContainerMovementVector;
    QPointF mKeypadMovementStartingPoint;
    QPointF mKeypadMovementVector;
    QPointer<HbInputMethod> mInputMethod;
    HbVkbHost::HbVkbStatus mKeypadStatusBeforeOrientationChange;
    HbPendingVkbHostCall mPendingCall;
    bool mTitleBarHiddenByVkbHost;
    bool mStatusBarHiddenByVkbHost;
    mutable QRectF mScrollAreaRect;
    qreal mMarginInPixels;

private: // For unit test.
    static HbAbstractVkbHostPrivate *d_ptr(HbAbstractVkbHost *host) {
        Q_ASSERT(host);
        return host->d_func();
    }
    friend class TestHbAbstractVkbHostPrivate;
};

#endif // HBABSTRACTVKBHOST_P_H

// End of file

