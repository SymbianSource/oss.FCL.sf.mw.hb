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
#include "hbabstractvkbhost.h"
#include "hbabstractvkbhost_p.h"
#include "hbinputvirtualkeyboard.h"
#include "hbinputsettingproxy.h"
#include "hbinputvkbhostbridge.h"
#include "hbinputmethod.h"
#include "hbdeviceprofile.h"
#include "hbmainwindow.h"
#include "hbpopup.h"
#include "hbview.h"
#include "hbwidgetfeedback.h"
#include "hbinstance.h"

#include <QTextEdit>

const int HbAnimationTime = 200;
const qreal HbEditorExtraMargin = 5.0;
const qreal HbCursorLineMargin = 15.0;
const qreal HbContainerBorderMargin = 20.0;
const qreal HbHeightVerticalFactor = 0.5;
const qreal HbHeightHorizFactor = 0.7;
const QString KHandWritingName("Handwriting");

HbVkbHostContainerWidget::HbVkbHostContainerWidget(QObject *containterWidget)
:mContainerWidget(containterWidget)
{
}

// sets container widgets position to new position.
void HbVkbHostContainerWidget::setPos(QPointF newPosition)
{
    if (mContainerWidget) {
        QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject *>(mContainerWidget);
        if (graphicsObject) {
            graphicsObject->setPos(newPosition);
            return;
        }

        QWidget *qWidget = qobject_cast<QWidget *>(mContainerWidget);
        if (qWidget) {
#ifdef Q_WS_WIN
             QPoint finalPosition = newPosition.toPoint();
             finalPosition -= qWidget->geometry().topLeft() - qWidget->frameGeometry().topLeft();
             qWidget->move(finalPosition);
#else             
             qWidget->move(newPosition.toPoint());
#endif
            return;
        }
    }
}

// returns the global position, if container widget is a QGraphicsObject, it returns
// scene position. In case the widget is QWidget it returns global co-ordinates
QPointF HbVkbHostContainerWidget::pos()
{
    if (mContainerWidget) {
        QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject *>(mContainerWidget);
        if (graphicsObject) {
            return graphicsObject->pos();;
        } 

        QWidget *qWidget = qobject_cast<QWidget *>(mContainerWidget);
        if (qWidget) {
            return qWidget->mapToGlobal(QPoint(0,0));
        }
    }

    return QPointF(0, 0);
}

// returns the bounding rect in global co-ordinate, if container widget is a QGraphicsObject
// it returns in scene co-ordinate, incase widget is QWidget it returns in global co-ordinate
QRectF HbVkbHostContainerWidget::sceneBoundingRect()
{
    if (mContainerWidget) {
        QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject *>(mContainerWidget);
        if (graphicsObject) {
            return graphicsObject->sceneBoundingRect();;
        } 

        QWidget *qWidget = qobject_cast<QWidget *>(mContainerWidget);
        if (qWidget) {
            return QRectF(qWidget->mapToGlobal(QPoint(0,0)), qWidget->size());
        }
    }

    return QRectF(0, 0, 0, 0);
}

// connect container specific signals here.
void HbVkbHostContainerWidget::connectSignals(QObject *reciever)
{
    if (qobject_cast<QGraphicsObject*> (mContainerWidget)) {
        QObject::connect(mContainerWidget, SIGNAL(yChanged())
            , reciever, SLOT(ensureCursorVisibility()));
    }

    HbPopup *popup = qobject_cast<HbPopup*>(mContainerWidget);
    if (popup) {
        QObject::connect(popup, SIGNAL(aboutToHide()), reciever, SLOT(containerAboutToClose()));
    }

    HbView *view = qobject_cast<HbView*>(mContainerWidget);
    if (view) {
        QObject::connect(view->mainWindow(), SIGNAL(currentViewChanged(HbView *))
            , reciever, SLOT(currentViewChanged(HbView *)));
    }
}

// disconnect container specific signals here.
void HbVkbHostContainerWidget::disconnectSignals(QObject *reciever)
{
    if (qobject_cast<QGraphicsObject*> (mContainerWidget)) {
        QObject::disconnect(mContainerWidget, SIGNAL(yChanged())
            , reciever, SLOT(ensureCursorVisibility()));
    }

    HbPopup *popup = qobject_cast<HbPopup*>(mContainerWidget);
    if (popup) {
        QObject::disconnect(popup, SIGNAL(aboutToHide()), reciever, SLOT(containerAboutToClose()));
    }
    
    HbPopup *view = qobject_cast<HbPopup*>(mContainerWidget);
    if (view) {
        QObject::disconnect(view->mainWindow(), SIGNAL(currentViewChanged(HbView *))
            , reciever, SLOT(currentViewChanged(HbView *)));
    }
}

/*!
\proto
\class HbAbstractVkbHost
\brief Base class for HbCore's virtual keyboard hosts.

This class contains common code for HbCore's virtual keyboard hosts.
*/

/// @cond

HbAbstractVkbHostPrivate::HbAbstractVkbHostPrivate(HbAbstractVkbHost *myHost, QObject *containerWidget)
: q_ptr(myHost),
mCallback(0),
mKeypad(0),
mContainerWidget(new HbVkbHostContainerWidget(containerWidget)),
mTimeLine(HbAnimationTime),
mKeypadStatus(HbVkbHost::HbVkbStatusClosed),
mKeypadOperationOngoing(false),
mOriginalContainerPosition(QPointF(0,0)),
mContainerMovementStartingPoint(QPointF(0, 0)),
mContainerMovementVector(QPointF(0, 0)),
mKeypadMovementStartingPoint(QPointF(0, 0)),
mKeypadMovementVector(QPointF(0, 0)),
mInputMethod(0),
mKeypadStatusBeforeOrientationChange(HbVkbHost::HbVkbStatusClosed)
{
    mTimeLine.setUpdateInterval(16);
}

HbAbstractVkbHostPrivate::~HbAbstractVkbHostPrivate()
{
    delete mContainerWidget;
}

void HbAbstractVkbHostPrivate::prepareAnimationsCommon()
{
    if (mContainerWidget->widgetObject() && mKeypad) {
        // If the keyboard is not already open, remember the original position.
        // That is where the container will eventually be returned to.
        if (mKeypadStatus == HbVkbHost::HbVkbStatusClosed) {
            mOriginalContainerPosition = mContainerWidget->pos();
        }

        // Initialize movement variables to starting values. These will
        // be fine tuned later.
        mKeypadMovementVector = QPointF(0, 0);
        mContainerMovementVector = QPointF(0, 0);
        mContainerMovementStartingPoint = mContainerWidget->pos();
        mKeypadMovementStartingPoint = mKeypad->pos();
    }

    mScreenSize = screenSize();
}

bool HbAbstractVkbHostPrivate::prepareContainerAnimation(HbVkbHost::HbVkbStatus status)
{
    if (!mKeypad || !mContainerWidget->widgetObject() || !mInputMethod || !mInputMethod->focusObject()) {
        return false;
    }

    if (status == HbVkbHost::HbVkbStatusOpened) {
        // Calculate the area that remains visible when the keypad is open.
        QRectF visibleArea = QRectF(0.0, 0.0, mScreenSize.width(), mScreenSize.height() - mKeypad->size().height());

        // Find out the container area.
        QRectF containerArea = mContainerWidget->sceneBoundingRect();
        containerArea.adjust(0.0, -HbContainerBorderMargin, 0.0, HbContainerBorderMargin);

        if (visibleArea.contains(containerArea)) {
            // The whole container is already inside the visible area, nothing to do.
            return false;
        }

        if (visibleArea.height() >= containerArea.height()) {
            // It isn't in the visible area yet but fits there. Let's move it there.
            mContainerMovementVector = QPointF(0.0, visibleArea.bottom() - containerArea.bottom());
            return true;
        }

        // Find out the editor bounding box and add a small margin to height.
        QRectF editorGeometry = mInputMethod->focusObject()->editorGeometry();
        editorGeometry.adjust(0.0, -HbCursorLineMargin, 0.0, HbCursorLineMargin);

        // Then see if the editor is already inside the visible area.
        // If it isn't, see if it would fit there.
        if (!visibleArea.contains(editorGeometry)) {
            if (editorGeometry.width() <= visibleArea.width() &&
                editorGeometry.height() <= visibleArea.height()) {
                // Yes, it fits into visible area, let's move it there so that
                // the whole editor area is in use right away.
                // First check if we want to move it to upper or lower
                // part of the visible area.
                if (editorGeometry.top() <= visibleArea.top()) {
                    // It goes to the upper part of the visible area.
                    mContainerMovementVector = QPointF(0.0, -editorGeometry.top());
                } else {
                    mContainerMovementVector = QPointF(0.0, visibleArea.bottom() - editorGeometry.bottom());
                }

                return true;
            }
        }

        // The editor is either already inside the visible area or doesn't fit there.
        // Let's see if the cursor is visble.
        // First find out micro focus rectangle and increase the height by a small margin.
        QRectF microFocus = mInputMethod->focusObject()->microFocus();
        microFocus.setTop(microFocus.top() - HbEditorExtraMargin);
        microFocus.setBottom(microFocus.bottom() + HbEditorExtraMargin);

        // Check whether the cursor rectangle is inside visible area.
        if (!visibleArea.contains(microFocus)) {
            QRectF realEditorGeometry = editorGeometry;
            realEditorGeometry.adjust(0.0, HbCursorLineMargin, 0.0, -HbCursorLineMargin);
            if (!realEditorGeometry.contains(microFocus)) {
                // A sanity check. If the microFocus rectangle is outside the editor
                // bounding rect, don't do anything. The situation in editor widget is not
                // up to date.
                return false;
            }
            // The cursor is outside the visible area. Figure out how much and
            // to which direction the container has to be moved.
            if (microFocus.bottom() <= visibleArea.top()) {
                // First see what would happen if we returned the container to original position.
                // Is the cursor visible then?
                // This is always preferred, use it if possible.
                QPointF toOriginalPos = mOriginalContainerPosition - mContainerWidget->pos();
                QRectF translatedMicroFocus = microFocus.translated(toOriginalPos);
                if (visibleArea.contains(translatedMicroFocus)) {
                    mContainerMovementVector = toOriginalPos;
                } else {
                    // It goes to the upper part of the visible area.
                    mContainerMovementVector = QPointF(0.0, HbCursorLineMargin - microFocus.top());
                }
            } else {
                mContainerMovementVector = QPointF(0.0, visibleArea.bottom() - HbCursorLineMargin - microFocus.bottom());
            }

            return true;
        }
    } else {
        // It is going to be closed or minimized.
        mContainerMovementVector = mOriginalContainerPosition - mContainerMovementStartingPoint;
        return true;
    }

    return false;
}

bool HbAbstractVkbHostPrivate::prepareKeypadAnimation(HbVkbHost::HbVkbStatus status)
{
    if (status == HbVkbHost::HbVkbStatusOpened) {
        if (mKeypadStatus == HbVkbHost::HbVkbStatusClosed) {
             // Set up keyboard open animation.
             mKeypadMovementStartingPoint.setY(mScreenSize.height());
             mKeypadMovementVector.setY(-mKeypad->size().height());
             if (!disableCursorShift()) {
                 // Initialize keypad position
                 mKeypad->setPos(mKeypadMovementStartingPoint);
             }
             return true;
         } else if (mKeypadStatus == HbVkbHost::HbVkbStatusMinimized && mCallback) {
             mKeypadMovementVector.setY(-(mKeypad->size().height() - mCallback->minimizedKeyboardSize().height()));
             return true;
         }
    } else if (status == HbVkbHost::HbVkbStatusMinimized && mCallback) {
        mKeypadMovementVector = QPointF(0, mKeypad->size().height() - mCallback->minimizedKeyboardSize().height());
        return true;
    } else {
        // It is going to be closed.
        mKeypadMovementVector = QPointF(0, mKeypad->size().height());
        return true;
    }

    return false;
}

bool HbAbstractVkbHostPrivate::prepareAnimations(HbVkbHost::HbVkbStatus status)
{
    prepareAnimationsCommon();

    bool containerResult = prepareContainerAnimation(status);
    if (containerResult) {
        // A sanity check. Container should never be moved below it's original
        // position. Limit the movement in case editor's micro focus returned faulty value
        // or something else bad happened.
        if ((mContainerMovementStartingPoint + mContainerMovementVector).y() > mOriginalContainerPosition.y()) {
            mContainerMovementVector.setY(mOriginalContainerPosition.y() - mContainerMovementStartingPoint.y());
            qWarning("Abstract VKB host: Invalid container position.");
        }
    }

    return (containerResult | prepareKeypadAnimation(status));
}

void HbAbstractVkbHostPrivate::connectSignals()
{
    mContainerWidget->connectSignals(q_ptr);

    // global signal not specific to any containter widget, can be connected now.
    HbMainWindow* mainWindow = this->mainWindow();
    if (mainWindow) {
        q_ptr->connect(mainWindow, SIGNAL(aboutToChangeOrientation()), q_ptr, SLOT(orientationAboutToChange()));
        q_ptr->connect(mainWindow, SIGNAL(orientationChanged(Qt::Orientation)), q_ptr, SLOT(orientationChanged(Qt::Orientation)));
        q_ptr->connect(mainWindow, SIGNAL(currentViewChanged(HbView*)), q_ptr, SLOT(currentViewChanged(HbView*)));
    }
}

void HbAbstractVkbHostPrivate::disconnectSignals()
{
    mContainerWidget->disconnectSignals(q_ptr);

    // global signal not specific to any containter widget, can be connected now.
    HbMainWindow* mainWindow = this->mainWindow();
    if (mainWindow) {
        q_ptr->disconnect(mainWindow, SIGNAL(aboutToChangeOrientation()), q_ptr, SLOT(orientationAboutToChange()));
        q_ptr->disconnect(mainWindow, SIGNAL(orientationChanged(Qt::Orientation)), q_ptr, SLOT(orientationChanged(Qt::Orientation)));
        q_ptr->disconnect(mainWindow, SIGNAL(currentViewChanged(HbView*)), q_ptr, SLOT(currentViewChanged(HbView*)));
    }
}

void HbAbstractVkbHostPrivate::openKeypad()
{    
    if (mContainerWidget->widgetObject()) {
        HbMainWindow* mainWin = mainWindow();
        if (mainWin && mKeypad) {
            if (mKeypad->scene() != mainWin->scene()) {
                // Add keypad to scene if it is not already in there.
                mainWin->scene()->addItem(mKeypad);
            }

            if (mKeypadStatus != HbVkbHost::HbVkbStatusOpened) {
                mCallback->aboutToOpen(q_ptr);
                q_ptr->resizeKeyboard(); // Make sure that the keyboard doesn't exceed given boundaries.
            }

            if (prepareAnimations(HbVkbHost::HbVkbStatusOpened)) {
                // Run the animation
                mKeypadStatus = HbVkbHost::HbVkbStatusOpened;
                mTimeLine.start();
            }
        }
    }
}

void HbAbstractVkbHostPrivate::closeKeypad()
{
    // Since the keypad is already in a minimized state we should not play animation.
    if (mKeypadStatus == HbVkbHost::HbVkbStatusMinimized) {
        closeKeypadWithoutAnimation();
    }

    if (mKeypadStatus != HbVkbHost::HbVkbStatusClosed) {
        mCallback->aboutToClose(q_ptr);

        if (prepareAnimations(HbVkbHost::HbVkbStatusClosed)) {
            mKeypadStatus = HbVkbHost::HbVkbStatusClosed;
            mTimeLine.start();
        }
    }
}

void HbAbstractVkbHostPrivate::minimizeKeypad()
{
    if (mCallback && mKeypadStatus != HbVkbHost::HbVkbStatusMinimized) {
        mCallback->aboutToClose(q_ptr);
        if (mContainerWidget->widgetObject()) {
            if (prepareAnimations(HbVkbHost::HbVkbStatusMinimized)) {
                mKeypadStatus = HbVkbHost::HbVkbStatusMinimized;
                mTimeLine.start();
            }
        }
    }
}

void HbAbstractVkbHostPrivate::openKeypadWithoutAnimation()
{
    HbMainWindow *mainWin = mainWindow();
    if (mKeypadStatus!= HbVkbHost::HbVkbStatusOpened && mKeypad && mContainerWidget->widgetObject() && mainWin) {
        if (mKeypad->scene() != mainWin->scene()) {
            // Add item to scene if it is not already in there.
            mainWin->scene()->addItem(mKeypad);
        }

        if (mKeypadStatus != HbVkbHost::HbVkbStatusOpened) {
            mCallback->aboutToOpen(q_ptr);
            q_ptr->resizeKeyboard(); // Make sure that the keyboard doesn't exceed given boundaries.
        }
        if (prepareAnimations(HbVkbHost::HbVkbStatusOpened)) {
            if (!disableCursorShift()) {
                // Move the container widget to keep the focused line visible.
                mContainerWidget->setPos(mContainerWidget->pos() + mContainerMovementVector);
                
                // Move the keypad
                mKeypad->setPos(mKeypadMovementStartingPoint + mKeypadMovementVector);
            }

            mKeypadStatus = HbVkbHost::HbVkbStatusOpened;
            mCallback->keyboardOpened(q_ptr);
        }
    }
}

void HbAbstractVkbHostPrivate::openMinimizedKeypad()
{
    // No need of any animation as this minimized keypad is very small to be a candidate for an
    // animation.
    HbMainWindow *mainWin = mainWindow();
    if (mainWin && mKeypad && mContainerWidget->widgetObject()) {
        if (mKeypad->scene() != mainWin->scene()) {
            // Add item to scene if it is not already in there.
            mainWin->scene()->addItem(mKeypad);
        }

        if (mKeypadStatus != HbVkbHost::HbVkbStatusMinimized) {
            mCallback->aboutToOpen(q_ptr);           
            q_ptr->resizeKeyboard(); // Make sure that the keyboard doesn't exceed given boundaries.
        }

        if (prepareAnimations(HbVkbHost::HbVkbStatusMinimized)) {             
            if (!disableCursorShift()) {
                mKeypad->setPos(0.0, mScreenSize.height() - mCallback->minimizedKeyboardSize().height());
            }
            mKeypadStatus = HbVkbHost::HbVkbStatusMinimized;
            mCallback->keyboardMinimized(q_ptr);
        }
    }
}

void HbAbstractVkbHostPrivate::closeKeypadWithoutAnimation()
{
    if (mKeypadStatus != HbVkbHost::HbVkbStatusClosed && mKeypad) {
        mCallback->aboutToClose(q_ptr);
        // Set original content widget position
        mKeypadStatus = HbVkbHost::HbVkbStatusClosed;

        if (!disableCursorShift()) {
            // Return the container widget to original position.
            mContainerWidget->setPos(mOriginalContainerPosition);
        }

        // Hide the keypad
        mKeypad->hide();
        mCallback->keyboardClosed(q_ptr);
        mCallback = 0;
    }
}

void HbAbstractVkbHostPrivate::minimizeKeypadWithoutAnimation()
{
    HbMainWindow *mainWin = mainWindow();
    if (mKeypadStatus != HbVkbHost::HbVkbStatusMinimized && mKeypad && mainWin) {
        mCallback->aboutToClose(q_ptr);                
        if (mKeypad->scene() != mainWin->scene()) {
            // Add item to scene if it is not already in there.
            mainWin->scene()->addItem(mKeypad);
        }

        mKeypadStatus = HbVkbHost::HbVkbStatusMinimized;
        if (!disableCursorShift()) {
            // Return the container widget to original position.
            mContainerWidget->setPos(mOriginalContainerPosition);

            // Set the keypad to minimized position.
            mKeypad->setPos(QPointF(0.0, mScreenSize.height() - mCallback->minimizedKeyboardSize().height()));
        }
    }
}

void HbAbstractVkbHostPrivate::cancelAnimationAndHideVkbWidget()
{
    if (mTimeLine.state() == QTimeLine::Running) {
        mTimeLine.stop();

        if (!disableCursorShift()) {
            mContainerWidget->setPos(mOriginalContainerPosition);
        }

        if (mKeypad) {
            mKeypad->hide();
        }

        // Clear possible pending call.
        mPendingCall.vkb = 0;

        emit q_ptr->keypadClosed();
        HbVkbHostBridge::instance()->connectHost(0);
        mKeypadStatus = HbVkbHost::HbVkbStatusClosed;
    }
}

HbMainWindow *HbAbstractVkbHostPrivate::mainWindow() const
{
    HbWidget *hbWidget = qobject_cast<HbWidget*>(mContainerWidget->widgetObject());
    if (hbWidget) {
        return hbWidget->mainWindow();
    }

    // below is the case when we have a pure vanilla application.
    // there should be one hbmainwindow to show all the widgets.
    if (hbInstance->allMainWindows().size())
        return hbInstance->allMainWindows().at(0);

    // no mainwindow.
    return 0;
}

QSizeF HbAbstractVkbHostPrivate::screenSize() const
{
    HbMainWindow *mainWin = mainWindow();
    QSizeF result = static_cast<QSizeF>(HbDeviceProfile::profile(mainWin).logicalSize());

    // do some sanity checking for the size got from device profile
    if( result.isNull() || result.width() < 200 || result.height() < 200 ) {
        qWarning("VkbHost error: size from device profile is faulty, using fallback!");
        if (mainWin) {
            if (mainWin->orientation() == Qt::Horizontal) {
                result.setWidth(640);
                result.setHeight(360);
            } else {
                result.setWidth(360);
                result.setHeight(640);
            }
        }
    }

    return result;
}

bool HbAbstractVkbHostPrivate::disableCursorShift() {

    if (!mInputMethod
        || mainWindow()) {
        return false;
    }

    QByteArray baModes = HbInputSettingProxy::instance()->preferredInputMethodCustomData(Qt::Horizontal);
    QString imName(baModes);

    if (mainWindow() && mainWindow()->orientation() == Qt::Horizontal && imName == KHandWritingName) {
        return true;
    }
    return false;
}


/// @endcond


HbAbstractVkbHost::HbAbstractVkbHost(HbWidget *containerWidget) : d_ptr(new HbAbstractVkbHostPrivate(this, containerWidget))
{
    Q_D(HbAbstractVkbHost);

    setParent(containerWidget);
    HbVkbHost::attachHost(this, containerWidget);

    connect(&d->mTimeLine, SIGNAL(finished()), this, SLOT(animationFinished()));
    connect(&d->mTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(animValueChanged(qreal)));
}

HbAbstractVkbHost::HbAbstractVkbHost(QWidget *containerWidget) : d_ptr(new HbAbstractVkbHostPrivate(this, containerWidget))
{
    Q_D(HbAbstractVkbHost);

    setParent(containerWidget);
    HbVkbHost::attachHost(this, containerWidget);

    connect(&d->mTimeLine, SIGNAL(finished()), this, SLOT(animationFinished()));
    connect(&d->mTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(animValueChanged(qreal)));
}

HbAbstractVkbHost::HbAbstractVkbHost(QGraphicsWidget *containerWidget) : d_ptr(new HbAbstractVkbHostPrivate(this, containerWidget))
{
    Q_D(HbAbstractVkbHost);

    setParent(containerWidget);
    HbVkbHost::attachHost(this, containerWidget);

    connect(&d->mTimeLine, SIGNAL(finished()), this, SLOT(animationFinished()));
    connect(&d->mTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(animValueChanged(qreal)));
}

HbAbstractVkbHost::HbAbstractVkbHost(QGraphicsObject *containerWidget) : d_ptr(new HbAbstractVkbHostPrivate(this, containerWidget))
{
    Q_D(HbAbstractVkbHost);

    setParent(containerWidget);
    HbVkbHost::attachHost(this, containerWidget);

    connect(&d->mTimeLine, SIGNAL(finished()), this, SLOT(animationFinished()));
    connect(&d->mTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(animValueChanged(qreal)));
}

HbAbstractVkbHost::HbAbstractVkbHost(HbAbstractVkbHostPrivate *dd) : d_ptr(dd)
{
    Q_D(HbAbstractVkbHost);

    setParent(d->mContainerWidget->widgetObject());
    HbVkbHost::attachHost(this, d->mContainerWidget->widgetObject());

    connect(&d->mTimeLine, SIGNAL(finished()), this, SLOT(animationFinished()));
    connect(&d->mTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(animValueChanged(qreal)));
}


HbAbstractVkbHost::~HbAbstractVkbHost()
{
    if (d_ptr->mKeypad) {
        d_ptr->mKeypad->hide();

        if (d_ptr->mCallback) {
            d_ptr->mCallback->keyboardClosed(this);
            d_ptr->mCallback = 0;
        }
    }
    emit keypadClosed();
    delete d_ptr;
}

/*!
\reimp
*/
HbVkbHost::HbVkbStatus HbAbstractVkbHost::keypadStatus() const
{
    Q_D(const HbAbstractVkbHost);
    return d->mKeypadStatus;
}

/*!
\reimp
*/

void HbAbstractVkbHost::openKeypad(HbVirtualKeyboard *vkb, HbInputMethod* owner, bool animationAllowed)
{
    Q_D(HbAbstractVkbHost);

    if (owner) {
        d->mInputMethod = owner;
    }

    if ((d->mKeypadStatus != HbVkbStatusMinimized) && (!vkb || !owner)) {
        // The caller is opening the keypad for the first time but didn't supply
        // all the needed parameters. Null parameters are ok only if minimized
        // keypad is reopened.
        return;
    }

    if (!HbVkbHostBridge::instance()->connectHost(this)) {
        // Do not set open call pending if orientation change is ongoing
        if (HbInputSettingProxy::instance()->orientationChangeCompleted()) {
            connect(HbVkbHostBridge::instance(), SIGNAL(stateTransitionCompleted()), this, SLOT(stateTransitionCompleted()));
            // The previous keyboard is still closing. Set the call pending and return.
            d->mPendingCall.vkb = vkb;
            d->mPendingCall.animationAllowed = animationAllowed;
        }
        return;
    }

    if (!d->mKeypadOperationOngoing) {
        d->mKeypadOperationOngoing = true;

        if (vkb && (d->mCallback != vkb || !d->mKeypad)) {
            // This keypad is opened for the first time or it was opened before but some other keypad
            // was opened in between.
            d->mCallback = vkb;
            d->mKeypad = vkb->asGraphicsWidget();
        }

        if (!d->mKeypad) {
            // Keyboard widget creation failed for some reason, can't go on.
            d->mCallback = 0;
            return;
        }

        emit aboutToOpen();

        if (animationAllowed) {
            d->openKeypad();
        } else {
            d->openKeypadWithoutAnimation();
            emit keypadOpened();
        }
        HbWidgetFeedback::triggered(qobject_cast<const HbWidget*>(d->mKeypad), Hb::InstantPopupOpened);

        d->connectSignals();
        d->mKeypadOperationOngoing = false;
    }
}

/*!
\reimp
*/
void HbAbstractVkbHost::closeKeypad(bool animationAllowed)
{
    Q_D(HbAbstractVkbHost);

    if (d->mKeypadStatus != HbVkbStatusClosed && !d->mKeypadOperationOngoing) {
        d->mKeypadOperationOngoing = true;

        emit aboutToClose();

        if (animationAllowed) {
            d->closeKeypad();
        } else {
            d->closeKeypadWithoutAnimation();
            emit keypadClosed();
            HbVkbHostBridge::instance()->connectHost(0);
        }

        d->disconnectSignals();
        d->mKeypadOperationOngoing = false;
    }
}

/*!
This slot is called every time an animation frame is drawn.
*/
void HbAbstractVkbHost::animValueChanged(qreal value)
{
    Q_D(HbAbstractVkbHost);

    if (!d->disableCursorShift()) {
        // Move the container.
    if (d->mContainerWidget->widgetObject()) {
            d->mContainerWidget->setPos(d->mContainerMovementStartingPoint + (d->mContainerMovementVector * value));
        }

        // Move keypad
        if (d->mKeypad) {
            d->mKeypad->setPos(d->mKeypadMovementStartingPoint + (d->mKeypadMovementVector * value));
        }
    }

    if (d->mCallback && d->mKeypad) {
        d->mCallback->keyboardAnimationFrame(HbVirtualKeyboard::HbVkbAnimOpen, value);
    }
}

/*!
This slot is called when an animation sequence is completed.
*/
void HbAbstractVkbHost::animationFinished()
{
    Q_D(HbAbstractVkbHost);

    if (d->mContainerWidget->widgetObject() && d->mKeypad && d->mCallback && d->mInputMethod) {
        if (!d->disableCursorShift()) {
            // Make sure the container reached target position.
            d->mContainerWidget->setPos(d->mContainerMovementStartingPoint + d->mContainerMovementVector);
            // Make sure the keypad reached target position.
            d->mKeypad->setPos(d->mKeypadMovementStartingPoint + d->mKeypadMovementVector);
        }

        // Notify
        if (d->mKeypadStatus == HbVkbHost::HbVkbStatusOpened) {
            d->mCallback->keyboardOpened(this);

            if (d->mInputMethod->focusObject()) {
                // This is hopefully temporary...
                QTextEdit *textEdit = qobject_cast<QTextEdit*>(d->mInputMethod->focusObject()->object());
                if (textEdit) {
                    textEdit->ensureCursorVisible();
                }
            }

            // Make sure the keypad never steals focus.
            d->mKeypad->setFlag(QGraphicsItem::ItemIsPanel, true);
            d->mKeypad->setActive(false);
            emit keypadOpened();
        } else if (d->mKeypadStatus == HbVkbHost::HbVkbStatusMinimized) {
            d->mCallback->keyboardMinimized(this);
            emit keypadClosed();
        } else {
            // It was closed. Hide the keyboard.
            d->mKeypad->hide();
            // Return the container where it was.
            d->mContainerWidget->setPos(d->mOriginalContainerPosition);
            d->mCallback->keyboardClosed(this);
            emit keypadClosed();
            HbVkbHostBridge::instance()->connectHost(0);
        }
    }
}

/*!
\reimp
*/
QSizeF HbAbstractVkbHost::keyboardArea() const
{
    Q_D(const HbAbstractVkbHost);

    HbMainWindow *mainWindow = d->mainWindow();
    if (d->mContainerWidget->widgetObject() && mainWindow) {
        QSizeF screenSize = d->screenSize();

        if (mainWindow->orientation() == Qt::Horizontal) {
            return QSizeF(screenSize.width(), screenSize.height() * HbHeightHorizFactor);
        } else {
            return QSizeF(screenSize.width(), screenSize.height() * HbHeightVerticalFactor);
        }
    }

    return QSizeF(0.0, 0.0);
}

/*!
This slot is connected to orientation change warning signal from the framework
and notifies setting proxy. Notification will then be delivered through setting proxy to all the
interested parties.
*/
void HbAbstractVkbHost::orientationAboutToChange()
{
    Q_D(HbAbstractVkbHost);
    d->mKeypadStatusBeforeOrientationChange = d->mKeypadStatus;
    HbInputSettingProxy::instance()->notifyScreenOrientationChange();
}

/*!
This slot is connected to orientation change signal from the framework and notifies
the setting proxy. Notification will then be froearded to other interested parties
by the setting proxy.
*/
void HbAbstractVkbHost::orientationChanged(Qt::Orientation orientation)
{
    HbInputSettingProxy::instance()->setScreenOrientation(orientation);
}

/*!
\reimp
*/
HbVirtualKeyboard* HbAbstractVkbHost::activeKeypad() const
{
    Q_D(const HbAbstractVkbHost);
    return d->mCallback;
}

/*!
\reimp
*/
void HbAbstractVkbHost::ensureCursorVisibility()
{
    Q_D(HbAbstractVkbHost);

    if ((d->mTimeLine.state() == QTimeLine::Running) ||
        (d->mKeypadStatus == HbVkbStatusClosed) ||
        (d->mKeypadStatus == HbVkbStatusMinimized) ||
        !d->mContainerWidget->widgetObject()) {
            return;
    }

    // This will refresh the situation if needed.
    d->openKeypad();
}

/*!
Returns the area inside active main window view that will remain visible when the
virtual keyboard is open.
*/
QRectF HbAbstractVkbHost::activeViewRect() const
{
    Q_D(const HbAbstractVkbHost);

    HbMainWindow *mainWindow = d->mainWindow();
    if (d->mContainerWidget->widgetObject() && mainWindow && d->mCallback) {
        QSizeF vpSize = d->screenSize();
        QRectF viewport = QRectF(QPointF(0.0, 0.0), QPointF(vpSize.width(), vpSize.height()));

        if (d->mKeypadStatus == HbVkbStatusMinimized) {
            viewport.setHeight(viewport.height() - d->mCallback->minimizedKeyboardSize().height());
        } else {
            viewport.setHeight(viewport.height() - confirmedKeyboardSize().height());
        }
        return viewport;
    }

    return QRectF();
}

/*!
Returns confirmed keyboard size. The method first queries preferred keyboard
size and then clips it against maximum allowed keyboard size. Resulting size is returned.
*/
QSizeF HbAbstractVkbHost::confirmedKeyboardSize()const
{
    Q_D(const HbAbstractVkbHost);

    if (d->mCallback && d->mKeypad) {
        QSizeF kbArea = keyboardArea();        
        QSizeF confirmed = d->mCallback->preferredKeyboardSize();

        if (confirmed.width() > kbArea.width()) {
            confirmed.setWidth(kbArea.width());
        }
        if (confirmed.height() > kbArea.height()) {
            confirmed.setHeight(kbArea.height());
        }

        return QSizeF(confirmed);
    }

    return QSizeF();
}

/*!
Resizes keyboard widget to its preferred size and makes sure that
the size does not exceed the size that host is willing to give to it.
*/
void HbAbstractVkbHost::resizeKeyboard()
{
    Q_D(HbAbstractVkbHost);

    if (d->mKeypad) {
        QSizeF currentSize = d->mKeypad->size();
        QSizeF newSize = confirmedKeyboardSize();
        if (currentSize != newSize) {
            d->mKeypad->resize(newSize);
        }
    }
}

/*!
\reimp
*/
QRectF HbAbstractVkbHost::applicationArea() const
{
    Q_D(const HbAbstractVkbHost);

    if (d->mKeypadStatus == HbVkbStatusOpened ||
        d->mKeypadStatus == HbVkbStatusMinimized) {
        return activeViewRect();
    }

    return QRectF();
}

/*!
\reimp
*/
void HbAbstractVkbHost::minimizeKeypad(bool animationAllowed)
{
    Q_D(HbAbstractVkbHost);
    if (d->mKeypadStatus != HbVkbStatusClosed && !d->mKeypadOperationOngoing) {
        d->mKeypadOperationOngoing = true;

        if (animationAllowed) {
            d->minimizeKeypad();
        } else {
            d->minimizeKeypadWithoutAnimation();
        }

        d->mKeypadOperationOngoing = false;
    }
}

/*!
\reimp
*/
void HbAbstractVkbHost::openMinimizedKeypad(HbVirtualKeyboard *vkb, HbInputMethod* owner)
{
    Q_D(HbAbstractVkbHost);
    d->mInputMethod = owner;

    if (!d->mKeypadOperationOngoing) {
        d->mKeypadOperationOngoing = true;

        if (d->mCallback != vkb || !d->mKeypad) {
            // This keypad is opened for the first time or it was opened before but some other keypad
            // was opened in between.
            d->mCallback = vkb;
            d->mKeypad = vkb->asGraphicsWidget();
        }

        if (!d->mKeypad) {
            // Keyboard widget creation failed for some reason, can't go on.
            d->mCallback = 0;
            return;
        }

        d->openMinimizedKeypad();
        d->connectSignals();
        d->mKeypadOperationOngoing = false;
    }
}

/*!
\reimp
*/
HbVkbHost::HbVkbStatus HbAbstractVkbHost::keypadStatusBeforeOrientationChange() const
{
    Q_D(const HbAbstractVkbHost);
    return d->mKeypadStatusBeforeOrientationChange;
}

/*!
This slot is called when active HbView changes.
*/
void HbAbstractVkbHost::currentViewChanged(HbView *view)
{
    Q_D(HbAbstractVkbHost);

    if (view != d->mContainerWidget->widgetObject()) {
        if (d->mTimeLine.state() == QTimeLine::Running) {               
            d->cancelAnimationAndHideVkbWidget();
            if (d->mCallback) {
                d->mCallback->keyboardClosed(this);
            }          
        } else if (d->mKeypadStatus != HbVkbStatusClosed) {
            d->closeKeypadWithoutAnimation();
            emit keypadClosed();
        }
    }
}

/*!
\reimp
*/
void HbAbstractVkbHost::refresh()
{
     Q_D(HbAbstractVkbHost);

     if (d->mKeypadStatus == HbVkbHost::HbVkbStatusOpened &&
         d->mTimeLine.state() != QTimeLine::Running) {
         d->prepareAnimationsCommon();
         if (d->prepareContainerAnimation(HbVkbHost::HbVkbStatusOpened)) {
             // Container status needs to be updated. Run the animation.
             d->mTimeLine.start();
         }
     }
}

/*!
\reimp
*/
bool HbAbstractVkbHost::stateTransitionOngoing() const
{
     Q_D(const HbAbstractVkbHost);
     return (d->mTimeLine.state() == QTimeLine::Running);
}

/*!
Receives signal from HbVkbHostBridge when previous host completes its state
transition and sens pending call if any.
*/
void HbAbstractVkbHost::stateTransitionCompleted()
{
    Q_D(HbAbstractVkbHost);

    disconnect(HbVkbHostBridge::instance(), SIGNAL(stateTransitionCompleted()), this, SLOT(stateTransitionCompleted()));

    if (d->mPendingCall.vkb) {
        // There was an open call pending. Do it now.
        HbVirtualKeyboard *vkb = d->mPendingCall.vkb;
        d->mPendingCall.vkb = 0;
        openKeypad(vkb, d->mInputMethod, d->mPendingCall.animationAllowed);
    }
}

// End of file
