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
#include "hbvkbhostcontainerwidget_p.h"
#include "hbinputfocusobject.h"
#include "hbvkbconstants_p.h"
#include "hbdeviceprofile.h"
#include "hbmainwindow_p.h"
#include "hbstatusbar_p.h"
#include "hbtitlebar_p.h"
#include "hbmainwindow.h"
#include "hbpopup.h"
#include "hbview.h"

#include <QWidget>

/*!
\internal
\class HbVkbHostContainerWidget
Encapsulates editor container widget and knows how the perform certain operations
with it, regardless of its base class (QWidget or QGraphicsObject).
In case of HbView, takes care of hiding title and status bars by returning suitable
fixed movement vector.

This class is for internal use only.
*/

HbVkbHostContainerWidget::HbVkbHostContainerWidget(QObject *containerWidget) 
 : mTopLevelItem(0)
{
    mGraphicsObject = qobject_cast<QGraphicsObject*>(containerWidget);
    if (!mGraphicsObject) {
        mWidget = qobject_cast<QWidget*>(containerWidget);
    }
}

/*!
\internal
Sets container widgets position to new position.
*/
void HbVkbHostContainerWidget::setPos(QPointF newPosition)
{
    if (mGraphicsObject) {
        if (!mTopLevelItem) {
            mTopLevelItem = mGraphicsObject->topLevelItem();
        }
        mTopLevelItem->setPos(newPosition);
        return;
    }

    if (mWidget) {
#ifdef Q_WS_WIN
        QPoint finalPosition = newPosition.toPoint();
        finalPosition -= mWidget->geometry().topLeft() - mWidget->frameGeometry().topLeft();
        mWidget->move(finalPosition);
#else
        mWidget->move(newPosition.toPoint());
#endif
        return;
    }
}

/*!
\internal
Returns the global position, if container widget is a QGraphicsObject, it returns
scene position. In case the widget is QWidget it returns global co-ordinates
*/
QPointF HbVkbHostContainerWidget::pos() const
{
    if (mGraphicsObject) {
        if (!mTopLevelItem) {
            mTopLevelItem = mGraphicsObject->topLevelItem();
        }
        return mTopLevelItem->pos();
    }

    if (mWidget) {
        return mWidget->mapToGlobal(QPoint(0, 0));
    }

    return QPointF(0, 0);
}

/*!
\internal
Returns the bounding rect in global co-ordinate, if container widget is a QGraphicsObject
it returns in scene co-ordinate, incase widget is QWidget it returns in global co-ordinate
*/
QRectF HbVkbHostContainerWidget::sceneBoundingRect() const
{
    if (mGraphicsObject) {
        return mGraphicsObject->sceneBoundingRect();
    }

    if (mWidget) {
        return QRectF(mWidget->mapToGlobal(QPoint(0, 0)), mWidget->size());
    }

    return QRectF(0, 0, 0, 0);
}

/*!
\internal
Connects container specific signals.
*/
void HbVkbHostContainerWidget::connectSignals(QObject *receiver)
{
    if (mGraphicsObject) {
        QObject::connect(mGraphicsObject, SIGNAL(yChanged()),
                         receiver, SLOT(ensureCursorVisibility()));
    }

    HbPopup *popup = qobject_cast<HbPopup *>(mGraphicsObject);
    if (popup) {
        QObject::connect(popup, SIGNAL(aboutToHide()), receiver, SLOT(_q_containerAboutToClose()));
    }

    HbView *view = qobject_cast<HbView *>(mGraphicsObject);
    if (view) {
        QObject::connect(view->mainWindow(), SIGNAL(currentViewChanged(HbView *)),
                         receiver, SLOT(_q_containerAboutToClose()));
    }
}

/*!
\internal
Disconnects container specific signals.
*/
void HbVkbHostContainerWidget::disconnectSignals(QObject *receiver)
{
    if (mGraphicsObject) {
        QObject::disconnect(mGraphicsObject, SIGNAL(yChanged()),
                            receiver, SLOT(ensureCursorVisibility()));
    }

    HbPopup *popup = qobject_cast<HbPopup *>(mGraphicsObject);
    if (popup) {
        QObject::disconnect(popup, SIGNAL(aboutToHide()), receiver, SLOT(_q_containerAboutToClose()));
    }

    HbPopup *view = qobject_cast<HbPopup *>(mGraphicsObject);
    if (view) {
        QObject::disconnect(view->mainWindow(), SIGNAL(currentViewChanged(HbView *)),
                            receiver, SLOT(_q_containerAboutToClose()));
    }
}

/*!
\internal
Returns fixed container movement vector. This vector is the minimum amount
the container always needs to be moved when the keypad opens.
In case of HbView, it is used for hiding status and title bars.
*/
QPointF HbVkbHostContainerWidget::fixedContainerMovement() const
{
    // Find out if the editor is inside a view and if it is, eliminate possible
    // status and title bars with suitable fixed movement vector.
    HbView *view = qobject_cast<HbView*>(mGraphicsObject);
    qreal yComponent = 0.0;
    if (view) {
        HbMainWindow *mainWindow = view->mainWindow();
        if (mainWindow) {
            if (view->isItemVisible(Hb::TitleBarItem)) {
                 yComponent += HbMainWindowPrivate::d_ptr(mainWindow)->mTitleBar->sceneBoundingRect().height();
            }
            if (view->isItemVisible(Hb::StatusBarItem)) {
                 yComponent += HbMainWindowPrivate::d_ptr(mainWindow)->mStatusBar->sceneBoundingRect().height();
            }
            return QPointF(0, -yComponent);
        }    
    }

    return QPointF(0, 0);
}

// End of file
