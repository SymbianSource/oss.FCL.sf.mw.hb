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
#include "private/hbvkbgeometrylogic_p.h"
#include "hbvkbhostcontainerwidget_p.h"
#include "hbinputvirtualkeyboard.h"
#include "hbinputsettingproxy.h"
#include "hbinputvkbhostbridge.h"
#include "hbvkbconstants_p.h"
#include "hbwidgetfeedback.h"
#include "hbinputmethod.h"
#include "hbdeviceprofile.h"
#include "hbscrollarea.h"
#include "hbmainwindow.h"
#include "hbpopup_p.h"
#include "hbpopup.h"
#include "hbview.h"
#include "hbinstance.h"

#include <QTextEdit>

const QString KHandWritingName("Handwriting");
// see hbpopup.cpp for this
extern const char* KPositionManagedByVKB;

/*!
@stable
@hbcore
\class HbAbstractVkbHost
\brief The default virtual keyboard host

This class implements the default virtual keyboard host. A virtual keyboard
host is responsible for the interaction between application background
and the virtual keyboard widget when a virtual keyboard becomes visible.

Its main task is to guarantee that editor's cursor line remains visible
and that the virtual keyboard widget doesn't cover it.

The abstract vkb host does this by moving the editor container around in suitable way.

See \ref vkbHandling "virtual keyboard handling guide" for more information

\sa HbVkbHost
\sa HbShrinkingVkbHost
\sa HbStaticVkbHost
*/

HbAbstractVkbHostPrivate::HbAbstractVkbHostPrivate(HbAbstractVkbHost *myHost, QObject *containerWidget)
    : q_ptr(myHost),
      mCallback(0),
      mKeypad(0),
      mContainerWidget(new HbVkbHostContainerWidget(containerWidget)),
      mTimeLine(HbVkbAnimationTime),
      mKeypadStatus(HbVkbHost::HbVkbStatusClosed),
      mKeypadOperationOngoing(false),
      mOriginalContainerPosition(QPointF(0, 0)),
      mContainerMovementStartingPoint(QPointF(0, 0)),
      mContainerMovementVector(QPointF(0, 0)),
      mKeypadMovementStartingPoint(QPointF(0, 0)),
      mKeypadMovementVector(QPointF(0, 0)),
      mInputMethod(0),
      mKeypadStatusBeforeOrientationChange(HbVkbHost::HbVkbStatusClosed),
      mTitleBarHiddenByVkbHost(false),
      mStatusBarHiddenByVkbHost(false),
      mMarginInPixels(0)
{
    mTimeLine.setUpdateInterval(16);
    mMarginInPixels = HbDeviceProfile::current().unitValue() * HbVkbHostMargin;
}

HbAbstractVkbHostPrivate::~HbAbstractVkbHostPrivate()
{
    delete mContainerWidget;
}

/*!
\internal
Initializes starting values to parameters needed for running the keypad and
container widget animation effects.
*/
void HbAbstractVkbHostPrivate::prepareAnimationsCommon()
{
    // Initialize movement variables to starting values. These will
    // be fine tuned later.
    mKeypadMovementVector = QPointF(0, 0);
    mContainerMovementVector = QPointF(0, 0);
    mContainerMovementStartingPoint = QPointF();
    mKeypadMovementStartingPoint = QPointF();
    mScrollAreaRect = QRectF();

    if (mContainerWidget && mContainerWidget->widgetObject() && mKeypad) {
        // If the keyboard is not already open, remember the original position.
        // That is where the container will eventually be returned to.
        if (mKeypadStatus == HbVkbHost::HbVkbStatusClosed) {
            mOriginalContainerPosition = mContainerWidget->pos();
        }

        mContainerMovementStartingPoint = mContainerWidget->pos();
        mKeypadMovementStartingPoint = mKeypad->pos();
    }

    mScreenSize = screenSize();

    // Make sure that the editor is completely visible inside a scroll area.
    ensureVisibilityInsideScrollArea();
}

/*!
\internal
Sets up view and focus object variables.
*/
bool HbAbstractVkbHostPrivate::getViewAndFocusObjects(HbView*& currentView, HbInputFocusObject*& focusObject)
{
    if (!mKeypad || !mInputMethod || !mContainerWidget ) {
        return false;
    }

    HbMainWindow* window = mainWindow();
    if (!window) {
        return false;
    }

    // Update margin pixel value while you're at it.
    mMarginInPixels = HbDeviceProfile::profile(window).unitValue() * HbVkbHostMargin;

    currentView = window->currentView();
    if (!currentView) {
        return false;
    }

    focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        return false;
    }

    return true;
}

/*!
\internal
Sets up the container widget animation effect.
*/
bool HbAbstractVkbHostPrivate::prepareContainerAnimation(HbVkbHost::HbVkbStatus status)
{
    // Init and check main objects
    HbView* currentView = 0;
    HbInputFocusObject* focusObject = 0;
    if (!getViewAndFocusObjects(currentView, focusObject)) {
        return false;
    }

    bool result = false;

    if (status == HbVkbHost::HbVkbStatusOpened) {
        // Init parameters before calling.
        QSizeF keypadSize = mKeypad->size();
        QRectF viewRect = currentView->sceneBoundingRect();
        bool isPopupType = qobject_cast<HbPopup *>(mContainerWidget->widgetObject()) != 0;
        bool vkbOpen = mKeypadStatus == HbVkbHost::HbVkbStatusOpened;
        QRectF containerRect = mContainerWidget->sceneBoundingRect();
        QRectF editorRect = findEditorRect(focusObject);
        QRectF cursorRect = focusObject->microFocus();
        QPointF fixedMovement = mContainerWidget->fixedContainerMovement();

        // We know that the container will be moved at least the amount of
        // fixed movement vector (which typically, if non-zero, is the
        // height of the title bar). So we reduce the keypad area
        // to make the calculations match (fixed movement points
        // to negative direction, that's why addition).
        if (!vkbOpen) {
            keypadSize.setHeight(keypadSize.height() + fixedMovement.y());
        }

        // Initialize geometry calculation unit to handle all geometry calculations.
        HbVkbGeometryLogicPrivate unit = HbVkbGeometryLogicPrivate(
                mScreenSize,
                keypadSize,
                viewRect,
                isPopupType,
                vkbOpen,
                containerRect,
                editorRect,
                cursorRect,
                mMarginInPixels);

        result = unit.calculateContainerMovement(mContainerMovementVector);
        if (!vkbOpen) {
            // Apply fixed movement only if the keyboard is not already open.
            // Fixed movement is used for hiding the title bar.
            mContainerMovementVector = selectLongestVerticalVector(fixedMovement, mContainerMovementVector);
        }
    } else if (status == HbVkbHost::HbVkbStatusClosed) {
        if (mContainerMovementStartingPoint != mOriginalContainerPosition) {
           mContainerMovementVector = mOriginalContainerPosition - mContainerMovementStartingPoint;
           result = true;
        }
    }

    return result;
}

/*!
\internal
Sets up the keypad widget animation effect.
*/
bool HbAbstractVkbHostPrivate::prepareKeypadAnimation(HbVkbHost::HbVkbStatus status)
{
    if (mKeypad) {
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
            }
        } else {
            // It is going to be closed.
            mKeypadMovementVector = QPointF(0, mKeypad->size().height());
            return true;
        }
    }

    return false;
}

/*!
\internal
Sets up all the animation effects.
*/
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

/*!
\internal
Connects orientation change, view switching and possible other related signals.
*/
void HbAbstractVkbHostPrivate::connectSignals()
{
    mContainerWidget->connectSignals(q_ptr);

    // global signal not specific to any containter widget, can be connected now.
    HbMainWindow *mainWindow = this->mainWindow();
    if (mainWindow) {
        q_ptr->connect(mainWindow, SIGNAL(aboutToChangeOrientation()), q_ptr, SLOT(orientationAboutToChange()));
        q_ptr->connect(mainWindow, SIGNAL(orientationChanged(Qt::Orientation)), q_ptr, SLOT(orientationChanged(Qt::Orientation)));
        q_ptr->connect(mainWindow, SIGNAL(aboutToChangeView(HbView *, HbView *)), q_ptr, SLOT(aboutToChangeView(HbView *, HbView *)));
    }
}

/*!
\internal
Disconnects orientation change, view switching and possible other related signals.
*/
void HbAbstractVkbHostPrivate::disconnectSignals()
{
    mContainerWidget->disconnectSignals(q_ptr);

    // global signal not specific to any containter widget, can be connected now.
    HbMainWindow *mainWindow = this->mainWindow();
    if (mainWindow) {
        q_ptr->disconnect(mainWindow, SIGNAL(aboutToChangeOrientation()), q_ptr, SLOT(orientationAboutToChange()));
        q_ptr->disconnect(mainWindow, SIGNAL(orientationChanged(Qt::Orientation)), q_ptr, SLOT(orientationChanged(Qt::Orientation)));
        q_ptr->disconnect(mainWindow, SIGNAL(aboutToChangeView(HbView *, HbView *)), q_ptr, SLOT(aboutToChangeView(HbView *, HbView *)));
    }
}

/*!
\internal
Opens the keypad widget with animation effect.
*/
void HbAbstractVkbHostPrivate::openKeypad()
{
    if (mContainerWidget->widgetObject()) {
        HbMainWindow *mainWin = mainWindow();
        if (mainWin && mKeypad) {
            if (mKeypad->scene() != mainWin->scene()) {
                // Add keypad to scene if it is not already in there.
                mainWin->scene()->addItem(mKeypad);
            }

            if (mKeypadStatus != HbVkbHost::HbVkbStatusOpened) {
                if (mCallback) {
                    mCallback->aboutToOpen(q_ptr);
                }
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

/*!
\internal
Closes the keypad with animation effect.
*/
void HbAbstractVkbHostPrivate::closeKeypad()
{  
    if (mKeypadStatus != HbVkbHost::HbVkbStatusClosed) {
        if (mCallback) {
            mCallback->aboutToClose(q_ptr);
        }

        if (prepareAnimations(HbVkbHost::HbVkbStatusClosed)) {
            mKeypadStatus = HbVkbHost::HbVkbStatusClosed;
            mTimeLine.start();
        }
    }
}

/*!
\internal
Opens the keypad widget without animation effect.
*/
void HbAbstractVkbHostPrivate::openKeypadWithoutAnimation()
{
    HbMainWindow *mainWin = mainWindow();
    if (mKeypadStatus != HbVkbHost::HbVkbStatusOpened && mKeypad && mContainerWidget->widgetObject() && mainWin) {
        if (mKeypad->scene() != mainWin->scene()) {
            // Add item to scene if it is not already in there.
            mainWin->scene()->addItem(mKeypad);
        }

        if (mKeypadStatus != HbVkbHost::HbVkbStatusOpened) {
            if (mCallback) {
                mCallback->aboutToOpen(q_ptr);
            }
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
            if (mCallback) {
                mCallback->keyboardOpened(q_ptr);
            }
        }
    }

    ensureVisibilityInsideScrollArea();
    ensureVisibilityInsideVisibleArea();
}

/*!
\internal
Closes the keypad widget without animating it.
*/
void HbAbstractVkbHostPrivate::closeKeypadWithoutAnimation()
{
    if (mKeypadStatus != HbVkbHost::HbVkbStatusClosed && mKeypad && mCallback) {
        mCallback->aboutToClose(q_ptr);

        // Set original content widget position
        mKeypadStatus = HbVkbHost::HbVkbStatusClosed;

        if (!disableCursorShift()) {
            // Return the container widget to original position.
            mContainerWidget->setPos(mOriginalContainerPosition);
            mContainerWidget->widgetObject()->setProperty(KPositionManagedByVKB, false);
        }

        // Hide the keypad
        mKeypad->hide();
        mCallback->keyboardClosed(q_ptr);
        mCallback = 0;
    }
}

/*!
\internal
Cancels the ongoing keypad animation and resets the timeline timer.
*/
void HbAbstractVkbHostPrivate::cancelAnimationAndHideVkbWidget()
{
    if (mTimeLine.state() == QTimeLine::Running) {
        mTimeLine.stop();

        if (!disableCursorShift() && mContainerWidget && mContainerWidget->widgetObject()) {
            mContainerWidget->setPos(mOriginalContainerPosition);
            mContainerWidget->widgetObject()->setProperty(KPositionManagedByVKB, false);
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

/*!
\internal
Returns pointer to container's main window (if one exists).
*/
HbMainWindow *HbAbstractVkbHostPrivate::mainWindow() const
{
    HbWidget *hbWidget = qobject_cast<HbWidget *>(mContainerWidget->widgetObject());
    if (hbWidget) {
        return hbWidget->mainWindow();
    }

    // below is the case when we have a pure vanilla application.
    // there should be one hbmainwindow to show all the widgets.
    if (hbInstance->allMainWindows().size()) {
        return hbInstance->allMainWindows().at(0);
    }

    // no mainwindow.
    return 0;
}

/*!
\internal
Returns screen size.
*/
QSizeF HbAbstractVkbHostPrivate::screenSize() const
{
    HbMainWindow *mainWin = mainWindow();
    QSizeF result = static_cast<QSizeF>(HbDeviceProfile::profile(mainWin).logicalSize());

    // do some sanity checking for the size got from device profile
    if (result.isNull() || result.width() < 200 || result.height() < 200) {
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

bool HbAbstractVkbHostPrivate::disableCursorShift()
{
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

/*!
\internal
Closes the keypad. This slot is connected to various signals from 
different container classes.
*/
void HbAbstractVkbHostPrivate::_q_containerAboutToClose()
{
    Q_Q(HbAbstractVkbHost);
    q->closeKeypad();
}

/*!
\internal
Finds out if given editor is inside a scroll area and makes sure the cursor position is visible inside the
scroll area.
*/
void HbAbstractVkbHostPrivate::ensureVisibilityInsideScrollArea() const
{
    if (mInputMethod && mInputMethod->focusObject()) {
        HbInputFocusObject *fo = mInputMethod->focusObject();
        HbScrollArea *scrollArea = 0;

        QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject *>(fo->object());
        if (graphicsObject) {
            for (QGraphicsWidget *parent = graphicsObject->parentWidget(); parent; parent = parent->parentWidget()) {
                scrollArea = qobject_cast<HbScrollArea*>(parent);
                if (scrollArea) {
                    break;
                }
            }
        }

        if (scrollArea && scrollArea->contentWidget()) {
            QRectF scrollRect = scrollArea->sceneBoundingRect();
            mScrollAreaRect = scrollRect;
            QRectF editorRect = fo->editorGeometry();

            if (!scrollRect.contains(editorRect)) {
                // The editor is not visible inside a scroll area.
                // Calculate how much the area needs to be scrolled
                // to make the cursor line visible inside the scroll
                // area. The call scroll area's ensure visibility and
                // return the calculated value (it needs to be factored in
                // to container animation calculations).                            
                if (editorRect.height() < scrollRect.height()) {
                    // Whole editor rect fits into scroll area. Move it there.
                    if (editorRect.bottom() > scrollRect.bottom()) {
                        // Scroll upwards.                    
                        scrollArea->ensureVisible(scrollArea->contentWidget()->mapFromScene(editorRect.bottomLeft()), 0.0, mMarginInPixels);
                    } else {
                        // Scroll downwards.                        
                        scrollArea->ensureVisible(scrollArea->contentWidget()->mapFromScene(editorRect.topLeft()), 0.0, mMarginInPixels);
                    }
                } else {
                    // Whole editor doesn't fit into scroll area. Used micro focus position instead.
                    QRectF microFocus = fo->microFocus();
                    if (microFocus.bottom() > scrollRect.bottom()) {
                        // Scroll upwards.
                        scrollArea->ensureVisible(scrollArea->contentWidget()->mapFromScene(microFocus.bottomLeft()), 0.0, mMarginInPixels);
                    } else {
                        // Scroll downwards.
                        scrollArea->ensureVisible(scrollArea->contentWidget()->mapFromScene(microFocus.topLeft()), 0.0, mMarginInPixels);
                    }
                }
            }
        }
    }
}

/*!
\internal
Finds out if given editor is inside a scroll area and makes sure the cursor position is visible inside the
visible area.
*/
void HbAbstractVkbHostPrivate::ensureVisibilityInsideVisibleArea() const
{
    Q_Q(const HbAbstractVkbHost);
    if (mInputMethod && mInputMethod->focusObject()) {
        HbInputFocusObject *fo = mInputMethod->focusObject();
        HbScrollArea *scrollArea = 0;

        QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject *>(fo->object());
        if (graphicsObject) {
            for (QGraphicsWidget *parent = graphicsObject->parentWidget(); parent; parent = parent->parentWidget()) {
                scrollArea = qobject_cast<HbScrollArea*>(parent);
                if (scrollArea) {
                    break;
                }
            }
        }

        if (scrollArea && scrollArea->contentWidget()) {
            QRectF editorRect = fo->editorGeometry();
            QRectF visibleArea = q->applicationArea();
            if (!visibleArea.isValid() && mContainerWidget) {
                visibleArea = mContainerWidget->sceneBoundingRect();
            }

            if (!visibleArea.contains(editorRect)) {
                // Editor is inside scroll area but not on visible area so scroll it to be visible
                if (editorRect.bottom() > visibleArea.bottom()) {
                    // Scroll upwards.
                    QPointF point = QPointF(0.0, -scrollArea->contentWidget()->pos().y() + (editorRect.bottom() - visibleArea.bottom()) + mMarginInPixels);
                    scrollArea->scrollContentsTo(point);
                } else {
                    // Scroll downwards.
                    QPointF point = QPointF(0.0, -scrollArea->contentWidget()->pos().y() + editorRect.y() - mMarginInPixels);
                    scrollArea->scrollContentsTo(point);
                }
            }
        }
    }
}

/*!
\internal
Returns the editor bounding rect. If the editor is inside a scroll area, then we actually
need to use the intersection of scroll area bounding rect and the editor rect in
our calculations.
*/
QRectF HbAbstractVkbHostPrivate::findEditorRect(HbInputFocusObject* fo) const
{
    if (fo) {      
        QRectF editorRect = fo->editorGeometry();
        if (!mScrollAreaRect.isEmpty()) {
            QRectF adjustedSrcollArea = mScrollAreaRect;
            // Cancel out the to-be-added margin
            adjustedSrcollArea.adjust(0, mMarginInPixels, 0, -mMarginInPixels);
            return editorRect.intersected(adjustedSrcollArea);
        }
        return editorRect;
    }

    return QRectF();
}

/*!
\internal
Returns true if the container is a popup and it is still running its opening animation
when the open keypad call comes in.
*/
bool HbAbstractVkbHostPrivate::popupAnimationInProgress() const
{
    HbPopup *popup = qobject_cast<HbPopup *>(mContainerWidget->widgetObject());
    if (popup) {
        HbPopupPrivate *popupPrivate = HbPopupPrivate::d_ptr(popup);
        return popupPrivate->showingInProgress;
    }

    return false;
}

/*!
\internal
From two given vector, returns the longest along the y-axis that points to the direction defined
by v1.
*/
QPointF HbAbstractVkbHostPrivate::selectLongestVerticalVector(const QPointF &v1, const QPointF &v2) const
{
    if (v1.y() * v2.y() < 0) {
        // They point to opposite directions.
        return v1;
    }

    if (qAbs(v2.y()) > qAbs(v1.y())) {
        // v2 is longer.
        return v2;
    }

    return v1;
}

HbAbstractVkbHost::HbAbstractVkbHost(HbWidget *containerWidget) : d_ptr(new HbAbstractVkbHostPrivate(this, containerWidget))
{
    Q_D(HbAbstractVkbHost);

    setParent(containerWidget);
    HbVkbHost::attachHost(this, containerWidget);
    if (containerWidget) {
        containerWidget->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    }

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
    if (containerWidget) {
        containerWidget->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    }

    connect(&d->mTimeLine, SIGNAL(finished()), this, SLOT(animationFinished()));
    connect(&d->mTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(animValueChanged(qreal)));
}

HbAbstractVkbHost::HbAbstractVkbHost(QGraphicsObject *containerWidget) : d_ptr(new HbAbstractVkbHostPrivate(this, containerWidget))
{
    Q_D(HbAbstractVkbHost);

    setParent(containerWidget);
    HbVkbHost::attachHost(this, containerWidget);
    if (containerWidget) {
        containerWidget->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    }

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
void HbAbstractVkbHost::openKeypad(HbVirtualKeyboard *vkb, HbInputMethod *owner, bool animationAllowed)
{
    Q_D(HbAbstractVkbHost);

    if (owner) {
        d->mInputMethod = owner;
    }

    if (!vkb || !owner) {
        // The caller is opening the keypad for the first time but didn't supply
        // all the needed parameters.
        return;
    }

    if (!HbVkbHostBridge::instance()->connectHost(this)) {
        // Do not set open call pending if orientation change is ongoing
        connect(HbVkbHostBridge::instance(), SIGNAL(stateTransitionCompleted()), this, SLOT(stateTransitionCompleted()));
        // The previous keyboard is still closing. Set the call pending and return.
        d->mPendingCall.vkb = vkb;
        d->mPendingCall.animationAllowed = animationAllowed;
        return;
    }

    if (d->popupAnimationInProgress()) {
        // The container is a popup and it is still running its opening animation.
        // delay.
        connect(d->mContainerWidget->widgetObject(), SIGNAL(popupReady()), this, SLOT(stateTransitionCompleted()));
        d->mPendingCall.vkb = vkb;
        d->mPendingCall.animationAllowed = animationAllowed;
        d->mPendingCall.popupOpening = true;
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

        if (d->mContainerWidget && d->mContainerWidget->widgetObject()) {
            d->mContainerWidget->widgetObject()->setProperty(KPositionManagedByVKB, true);
        }

        if (animationAllowed) {
            d->openKeypad();
        } else {
            d->openKeypadWithoutAnimation();
            emit keypadOpened();
        }
        HbWidgetFeedback::triggered(qobject_cast<const HbWidget *>(d->mKeypad), Hb::InstantPopupOpened);

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

            // Move keypad
            if (d->mKeypad) {
                QPointF keypadPos = d->mKeypadMovementStartingPoint + (d->mKeypadMovementVector * value);
                d->mKeypad->setPos(keypadPos);
            }
        }
    }

    if (d->mCallback) {
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
                QTextEdit *textEdit = qobject_cast<QTextEdit *>(d->mInputMethod->focusObject()->object());
                if (textEdit) {
                    textEdit->ensureCursorVisible();
                }
            }
            d->ensureVisibilityInsideScrollArea();
            d->ensureVisibilityInsideVisibleArea();

            // Make sure the keypad never steals focus.
            d->mKeypad->setFlag(QGraphicsItem::ItemIsPanel, true);
            if (d->mKeypad->isActive()) {
                d->mKeypad->setActive(false);
            }
            emit keypadOpened();
        } else {
            // It was closed. Hide the keyboard.
            d->mKeypad->hide();
            // Return the container where it was.
            d->mContainerWidget->setPos(d->mOriginalContainerPosition);
            d->mContainerWidget->widgetObject()->setProperty(KPositionManagedByVKB, false);
            d->mCallback->keyboardClosed(this);
            emit keypadClosed();

            // Keyboard might be opened again due to pending open call
            if (d->mKeypadStatus == HbVkbHost::HbVkbStatusClosed) {
                HbVkbHostBridge::instance()->connectHost(0);
            }
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
}

/*!
This slot is connected to orientation change signal from the framework and notifies
the setting proxy. Notification will then be froearded to other interested parties
by the setting proxy.
*/
void HbAbstractVkbHost::orientationChanged(Qt::Orientation orientation)
{
    Q_UNUSED(orientation);
}

/*!
\reimp
*/
HbVirtualKeyboard *HbAbstractVkbHost::activeKeypad() const
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
    if (d->mContainerWidget && d->mContainerWidget->widgetObject() && mainWindow && d->mCallback) {
        QSizeF vpSize = d->screenSize();
        QRectF viewport = QRectF(QPointF(0.0, 0.0), QPointF(vpSize.width(), vpSize.height()));

        viewport.setHeight(viewport.height() - confirmedKeyboardSize().height());
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

    if (d->mCallback) {
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

    if (d->mKeypadStatus == HbVkbStatusOpened) {
        return activeViewRect();
    }

    return QRectF();
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
\deprecated HbAbstractVkbHost::currentViewChanged(HbView *view)
    is deprecated.
*/
void HbAbstractVkbHost::currentViewChanged(HbView *view)
{
    Q_UNUSED(view);
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

    if (!d->mPendingCall.popupOpening) {
        disconnect(HbVkbHostBridge::instance(), SIGNAL(stateTransitionCompleted()), this, SLOT(stateTransitionCompleted()));
    } else {
        disconnect(d->mContainerWidget->widgetObject(), SIGNAL(popupReady()), this, SLOT(stateTransitionCompleted()));
    }

    if (d->mPendingCall.vkb) {
        // There was an open call pending. Do it now.
        HbVirtualKeyboard *vkb = d->mPendingCall.vkb;
        d->mPendingCall.vkb = 0;
        openKeypad(vkb, d->mInputMethod, d->mPendingCall.animationAllowed);
    }
}

/*!
This slot is called when change in active HbView starts.
*/
void HbAbstractVkbHost::aboutToChangeView(HbView *oldView, HbView *newView)
{
    Q_D(HbAbstractVkbHost);

    if (oldView != newView) {
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

#include "moc_hbabstractvkbhost.cpp"

// End of file
