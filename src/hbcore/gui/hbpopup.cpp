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

#include "hbpopup.h"
#include "hbpopup_p.h"
#include "hbinstance.h"
#include "hbpopupmanager_p.h"
#include "hbdeviceprofile.h"
#include "hbevent.h"
#include "hbgraphicsscene.h"
#include "hbgraphicsscene_p.h"
#include "hbtooltip.h"
#include "hbglobal_p.h"
#include "hbvgmaskeffect_p.h"
#include <QTimer>
#include <QGraphicsSceneMouseEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QEventLoop>
#include <QPointer>
#include <QDebug>
#include <QBitmap>
#include <QApplication> // krazy:exclude=qclasses

#include <hbwidgetfeedback.h>

#ifdef HB_EFFECTS
#include "hbeffectinternal_p.h"
bool HbPopupPrivate::popupEffectsLoaded = false;
#endif
/*!
    @beta
    @hbcore
    \class HbPopup
    \brief HbPopup is a base class for different popups in Hb library.

    Popup is a widget that is displayed above other widgets in the view.

    Lastly shown popup is always positioned in Z order on the the top
    of already visible popups.  

    A popup can be permanent or automatically dismissed after a
    time-out.  Modal popups interrupt any other user interaction
    outside of the popup while they are visible, whereas non-modal
    popups do not.

    \sa HbDialog
*/

/*!
    \reimp
    \fn int HbPopup::type() const
 */

/*!
    \enum HbPopup::DefaultTimeout

    This enum defines available default timeout values to be used in method
    setTimeout(HbPopup::DefaultTimeout).
 */

/*!
    \var HbPopup::NoTimeout

    No timeout is defined for automatically closing the popup. i.e. the popup is permanent.
 */

/*!
    \var HbPopup::ConfirmationNoteTimeout

    Timeout value intended to be used by confirmation notes.
 */

/*!
    \var HbPopup::StandardTimeout

    The default timeout value intended to be used by most non-permanent popups e.g. by notes.
 */

/*!
    \var HbPopup::ContextMenuTimeout

    The default timeout value intended to be used by context menus.
 */

/*!
    \enum HbPopup::DismissPolicy

    This enum defines available dismiss policy values.

    The dismiss policy defines what user actions will cause the popup to be dismissed i.e. closed.
 */

/*!
    \var HbPopup::NoDismiss

    The popup cannot be dismissed automatically by user interaction.
 */

/*!
    \var HbPopup::TapInside

    The popup is dismissed when user taps within the bounding rectangle of the popup.
 */

/*!
    \var HbPopup::TapOutside

    The popup is dismissed when user taps outside of the bounding rectangle of the popup.
 */

/*!
    \var HbPopup::TapAnywhere

    The popup is dismissed when user taps either within or outside
    of the bounding rectangle of the popup.
 */

/*!
    \enum HbPopup::FrameType

    This enum defines available frame type values.

    The frame types defines what frame item backgrounds will be used
    by the popup. Actual appearance is dependent on theme.
 */

/*!
    \var HbPopup::Strong

    The popup is using strong frame.
 */

/*!
    \var HbPopup::Weak

    The popup is using weak frame.
 */

/*!
    \fn void HbPopup::aboutToShow();

    This signal is emitted when the popup is about to be shown i.e. when method show() is called.
 */

/*!
    \fn void HbPopup::aboutToHide();

    This signal is emitted when the popup is about to be hidden i.e. when method hide() is called.
 */


/*!
    \fn void HbPopup::aboutToClose();

    This signal is emitted when the popup is about to be closed i.e. when method close() is called
    or the popup is
    dismissed by the user or timeout.
 */

static const struct { HbPopup::DefaultTimeout timeout; int value; } timeoutValues[] =
{
    {HbPopup::NoTimeout,0},
    {HbPopup::ConfirmationNoteTimeout,1500},
    {HbPopup::StandardTimeout,3000},
    {HbPopup::ContextMenuTimeout,6000},
};

HbPopupBackGround::HbPopupBackGround(HbPopup * popup, QGraphicsItem *parent) :
        QGraphicsItem(parent),
        popup(popup)
{
    // This is needed to be able to block moving the focus to items behind background item by
    // clicking on them.
    setFlag(QGraphicsItem::ItemIsFocusable);

#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemHasNoContents, true);
#endif
}

HbPopupBackGround::~HbPopupBackGround()
{
    // Set backgroundItem to 0 to avoid double deletion
    // e.g. when backgroundItem is deleted by scene before its popup
    if (popup) {
        HbPopupPrivate::d_ptr(popup)->backgroundItem = 0;
    }
}

void HbPopupBackGround::setRect(QRectF rect)
{
    mRect = rect;
}

QRectF HbPopupBackGround::boundingRect() const
{
    if(!mRect.isValid()){
        // set backgroundItem's size so that it is big enough
        // to cover the screen both landscape and portrait mode
        const QSizeF screenSize = HbDeviceProfile::profile(this).logicalSize();
        qreal dim = qMax(screenSize.width(), screenSize.height());
        mRect.adjust(0,0,dim,dim);
    }
    return mRect;
}

void HbPopupBackGround::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Q_UNUSED(option)
    Q_UNUSED(widget);
    Q_UNUSED(painter);
}

bool HbPopupBackGround::sceneEvent(QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneMousePress) {
        HbPopupPrivate::d_ptr(popup)->handleBackgroundMousePressEvent();
    }
    if (event->type() == QEvent::GraphicsSceneMouseRelease) {
        HbPopupPrivate::d_ptr(popup)->handleBackgroundMouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
    }
    if (event->type() == QEvent::FocusIn && scene()) {
        // Prevents last focus item losing its focus
        // This event is received only when popup is modal
        QFocusEvent restoreLastFocus(QEvent::FocusIn,Qt::OtherFocusReason);
        QApplication::sendEvent(scene(),&restoreLastFocus); // krazy:exclude=qclasses
    }
    // accept events only for modal popups
    if(popup->isModal()){
        event->accept();
        return true;
    } else {
        event->ignore();
    }
    return QGraphicsItem::sceneEvent(event);
}

HbPopupPrivate::HbPopupPrivate( ) :
    eventLoop(0),
    hasEffects(false),
    closed(false),
    hidingInProgress(true),
    delayedHide(false),
    deleteOnClose(false),
    modal(true), // must be in sync QGraphicsItem::ItemIsFocusable of backgroundItem
    fadeBackground(true),
    inDestruction(false),
    aboutToShowSignalGuard(false),
    duplicateShowEvent(false),
    timedOut(false),
    timeout(HbPopupPrivate::timeoutValue(HbPopup::StandardTimeout)),
    //    priorityValue(HbPopup::Default),
    priorityValue(0),
    dismissPolicy(HbPopup::TapOutside),
    backgroundItem(0),
    mousePressLocation(None),
    frameType(HbPopup::Strong),
    preferredPosSet(false),
    mStartEffect(false),
    mScreenMargin(0.0),
    mVgMaskEffect(0),
    timeoutTimerInstance(0)
{
}

HbPopupPrivate::~HbPopupPrivate()
{
}

void HbPopupPrivate::init()
{
    Q_Q(HbPopup);

    q->setAttribute(Hb::InsidePopup);

    // By default popups are focusable
    q->setFocusPolicy(Qt::StrongFocus);    
    q->setBackgroundItem(HbStyle::P_Popup_background);

    // Only for popup without parent
    if (!q->parentItem()) {
        backgroundItem = new HbPopupBackGround(q);
        backgroundItem->setVisible(false);

        // Popup is invisible by default (explicit show or open call is required)
        q->setVisible(false);
    }
    hidingInProgress = false;   

    q->setFlag(QGraphicsItem::ItemClipsToShape);
    q->setFlag(QGraphicsItem::ItemClipsChildrenToShape);

#if QT_VERSION > 0x040602
    q->grabGesture(Qt::TapGesture);
    q->grabGesture(Qt::TapAndHoldGesture);
    q->grabGesture(Qt::PanGesture);
    q->grabGesture(Qt::SwipeGesture);
    q->grabGesture(Qt::PinchGesture);
#endif

}

/*
 Sets the priority for a popup.
 A popup with higher priority is always shown on top of a popup with lower priority.
 In case of popups with same priority the lastly shown will be on top.
 Default priority is HbPopup::Default
 \sa priority()
*/
void HbPopupPrivate::setPriority(quint8 priority)
{
    //TODO: consider implementing dynamic change of prorities
    // i.e. if the priority changes while the popup is registered to popupManager
    // then re-register it to get its Z value updated
    priorityValue=priority;
}

#ifdef HB_EFFECTS
void HbPopupPrivate::_q_delayedHide(HbEffect::EffectStatus status)
{
    Q_UNUSED(status);

    Q_Q(HbPopup);

    // Apply forceHide only if the effect started successfully
    if (status.reason != Hb::EffectNotStarted) {
        forceHide();
    } else {
        delayedHide = false;
    }

    if (deleteOnClose) {
        q->deleteLater();
    }
    hidingInProgress = false;
}

void HbPopupPrivate::_q_orientationChange(Qt::Orientation orient, bool animate)
{
    Q_UNUSED(orient);
    if (animate) {
    Q_Q(HbPopup);
    HbEffect::start(q, "HB_POPUP", "orientationswitch");
    }

}
#endif // HB_EFFECTS

void HbPopupPrivate::_q_timeoutFinished()
{
    Q_Q(HbPopup);
    timedOut = true;
    q->close();
}

void HbPopupPrivate::stopTimeout()
{
    if (timeoutTimerInstance)
        timeoutTimerInstance->stop();
}

void HbPopupPrivate::startTimeout()
{
    if (timeout > 0) {
        timeoutTimer()->start();
        timedOut = false;
    }
}

void HbPopupPrivate::setTimeout(int msec)
{
    Q_Q(HbPopup);

    int prevTimeout = timeout;
    timeout = msec;
    if (msec > 0) {
        timeoutTimer()->setInterval(msec);
        // If timeout was 0 and it is now set to something > 0 then start the timer.
        if (q->isVisible() && prevTimeout <= 0)
            startTimeout();
    } else {
        stopTimeout();
    }
}

QTimer *HbPopupPrivate::timeoutTimer()
{
    Q_Q(HbPopup);
    if (!timeoutTimerInstance) {
        timeoutTimerInstance = new QTimer(q);
        timeoutTimerInstance->setSingleShot(true);
        q->connect(timeoutTimerInstance, SIGNAL(timeout()), q, SLOT(_q_timeoutFinished()));
    }

    return timeoutTimerInstance;
}

//returns true if popup has been added to scene here.
bool HbPopupPrivate::addPopupToScene()
{
    Q_Q(HbPopup);
    bool popupAdded(false);
    if (!q->parentItem()) {
        if (!q->scene() && !HbInstance::instance()->allMainWindows().isEmpty()) {            
            HbInstance::instance()->allMainWindows().at(0)->scene()->addItem(q);           
            popupAdded = true;
            if (backgroundItem) {
                q->scene()->addItem(backgroundItem);
            }
        } else if (q->scene() && backgroundItem && backgroundItem->scene() != q->scene()) {
            q->scene()->addItem(backgroundItem);
        }
    }
    return popupAdded;
}

void HbPopupPrivate::handleBackgroundMousePressEvent()
{
    Q_Q(HbPopup);
    mousePressLocation = Background;
    if (dismissPolicy & HbPopup::TapOutside) {
        q->close();
    }
}

void HbPopupPrivate::handleBackgroundMouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_Q(HbPopup);

    // Handle cases only for Background or Popup originated mouse presses and when
    // any dismiss policy defined
    if (mousePressLocation != None && dismissPolicy != HbPopup::NoDismiss) {

        MouseEventLocationType mouseReleaseLocation = Background;

        if (q->contains (q->mapFromScene(event->scenePos()))) {
            mouseReleaseLocation = Popup;
        }

        // Mouse is released within popup
        if (mouseReleaseLocation == Popup) {
            // Handle cases only when TapInside is set
            if (dismissPolicy & HbPopup::TapInside) {
                // Close popup if mouse press is initiated within popup or TapOutside is set
                if (mousePressLocation == Popup || dismissPolicy & HbPopup::TapOutside) {
                    q->close();
                }
            }
        }
    }

    // reset mousePressLocation
    mousePressLocation = None;
}


int HbPopupPrivate::timeoutValue(HbPopup::DefaultTimeout timeout)
{
    int count = sizeof(timeoutValues) / sizeof(timeoutValues[0]);
    if (timeout < 0 || timeout >= count) {
        return timeoutValues[HbPopup::NoTimeout].value;
    }
    return timeoutValues[timeout].value;
}

void HbPopupPrivate::forceHide()
{
    Q_Q(HbPopup);

    delayedHide = false;
    q->hide();
    delayedHide = hasEffects;
}

void HbPopupPrivate::addPopupEffects()
{

#ifdef HB_EFFECTS
    effectType = "HB_POPUP";
    hasEffects = popupEffectsLoaded;
    if (popupEffectsLoaded)
        return;
    popupEffectsLoaded = true;
    hasEffects = HbEffectInternal::add("HB_POPUP",
                                       "popup_appear",
                                       "appear");
    if (hasEffects) {
        hasEffects = HbEffectInternal::add("HB_POPUP",
                                           "popup_disappear",
                                           "disappear");
    }
    if (hasEffects ) {
        hasEffects = HbEffectInternal::add("HB_POPUP",
                                           "dialog_rotate",
                                           "orientationswitch");
    }
#endif
}


void HbPopupPrivate::doSetModal( bool modal ) {
    if(backgroundItem) {
        // When the popup is modal background item must receive focus
        // events to be able to prevent last focus item losing its
        // focus
        backgroundItem->setFlag(QGraphicsItem::ItemIsFocusable, modal);
        backgroundItem->setFlag(QGraphicsItem::ItemIsPanel, modal);
        if(modal) {
            backgroundItem->setPanelModality(QGraphicsItem::PanelModal);
        } else {
            backgroundItem->setPanelModality(QGraphicsItem::NonModal);
        }
    }
}

void HbPopupPrivate::calculateShape()
{
#if 0
    Q_Q(HbPopup);
    if (!mVgMaskEffect) {
        mVgMaskEffect = new HbVgMaskEffect();
        mVgMaskEffect->install(q);
    }

    QPixmap image(QSize(static_cast<int>(q->backgroundItem()->boundingRect().width()),
                            static_cast<int>(q->backgroundItem()->boundingRect().height())));
    image.fill(Qt::transparent);

    QPainter imagePainter(&image);

    q->backgroundItem()->paint(&imagePainter, 0, 0);

    imagePainter.end();

    mVgMaskEffect->setMask(image);

#endif
}

/*!
 Constructs a popup with given  \a parent graphics item.\n
 Note: popups with \a parent set as 0 are behaving as real popups.
 This is actually the intended use.

 However in some situation could be useful to embedd a popup into a QGraphicsItem.
 In this case a non zero \a parent value must be passed.
 Popups with parent items behaving just like any other QGraphicsWidget.
 The following features are not supported (i.e. ignored) for popup with parents:

       - modality
       - timeout
       - unfadedItems
       - dismissPolicy
       - signal aboutToClose
*/
HbPopup::HbPopup(QGraphicsItem *parent) :
    HbWidget(*new HbPopupPrivate,parent)
{
    Q_D(HbPopup);
    d->q_ptr = this;
    d->init();    
}


/*!
    \internal
 */
HbPopup::HbPopup(HbPopupPrivate &dd, QGraphicsItem *parent) :
    HbWidget(dd, parent)
{
    Q_D(HbPopup);
    d->q_ptr = this;
    d->init();    
}
/*!
 Destroys the popup.
*/
HbPopup::~HbPopup()
{
    Q_D(HbPopup);
    d->inDestruction = true;

    // Deregister popup from HbPopupManager in case hideEvent() was not called
    // before destruction
    HbGraphicsScene *hbScene = qobject_cast<HbGraphicsScene *>(scene());
    if (hbScene) {
        hbScene->d_ptr->hidePopup(this);
    }

    if (d->eventLoop) {
        d->eventLoop->exit();
    }
    if (d->backgroundItem) {
        // Set backgroundItem->popup to 0 to avoid double deletion
        // e.g. when popup is deleted by scene before its backgroundItem
        d->backgroundItem->popup = 0;

        // Delete the background item only and only if it's not going
        // to be cleaned up by the destructing graphics scene
        QGraphicsScene *scene = d->backgroundItem->scene();   // krazy:exclude=qclasses
        if (!scene || !scene->property("destructed").isValid()) {
            delete d->backgroundItem;
        }
    }
}


/*!
 Returns the popup timeout property in milliseconds.
 If this property is not set the deafult is HbPopup::StandardTimeout.
 \sa setTimeout()
*/
int HbPopup::timeout() const
{
    Q_D(const HbPopup);
    return d->timeout;
}

/*!
 Sets the popup timeout property in milliseconds.
 If timeout <= 0 then the popup is permanent and not closed automatically.
 \sa timeout() setTimeout(HbPopup::DefaultTimeout) QGraphicsWidget::close()
*/
void HbPopup::setTimeout(int timeout)
{
    Q_D(HbPopup);
    d->setTimeout(timeout);
    //d->timeout = timeout;
}

/*!
 It is a convenience overload of \a timeout() for setting HbPopup::DefaultTimeout values
 to achieve common look & feel.
 \sa enum DefaultTimeout
 \sa timeout() setTimeout(int) QGraphicsWidget::close()
*/
void HbPopup::setTimeout(HbPopup::DefaultTimeout timeout)
{
    setTimeout(HbPopupPrivate::timeoutValue(timeout));
}

/*!
 Returns the popup modality property.
 A modal popup blocks any user initiated events outside of the popup
 until it is closed.
 \sa setModal()
*/
bool HbPopup::isModal() const
{
    Q_D(const HbPopup);
    return d->modal;
}

/*!
 Sets the popup modality property.
 \sa isModal()
*/
void HbPopup::setModal(bool enabled)
{
    Q_D(HbPopup);
    d->modal = enabled;
    d->doSetModal( d->modal );
}

/*!
 Sets the background of popup faded if \a fadeBackground is true otherwise
 the background will not be faded.
 \sa isBackgroundFaded()
*/
void HbPopup::setBackgroundFaded(bool fadeBackground)
{
    Q_D(HbPopup);
    d->fadeBackground = fadeBackground;
}

/*!
 Returns if the background of the popup is faded or not.
 Default: true
 \sa isBackgroundFaded()
*/
bool HbPopup::isBackgroundFaded() const
{
    Q_D(const HbPopup);
    return d->backgroundItem && d->fadeBackground;
}

/*!
 Returns the dismiss policy of the popup.
 Default is HbPopup::TapOutside.
 \sa setDismissPolicy()
*/
HbPopup::DismissPolicy HbPopup::dismissPolicy() const
{
    Q_D(const HbPopup);
    return d->dismissPolicy;
}

/*!
 Sets the dismiss policy property for the the popup.

 \sa dismissPolicy()
*/
void HbPopup::setDismissPolicy(HbPopup::DismissPolicy dismissPolicy)
{
    Q_D(HbPopup);
    d->dismissPolicy = dismissPolicy;
}

/*!
 Returns the frame type of the popup.
 Default is HbPopup::Strong
 \sa setFrameType()
*/
HbPopup::FrameType HbPopup::frameType() const
{
    Q_D(const HbPopup);
    return d->frameType;
}

/*!
 Sets the frame typeproperty for the the popup.

 \sa frameType()
*/
void HbPopup::setFrameType(HbPopup::FrameType frameType)
{
    Q_D(HbPopup);
    if ( d->frameType != frameType ) {
        switch( frameType ) {
        case HbPopup::Weak:
            setBackgroundItem(HbStyle::P_Popup_background_weak);
            break;
        case HbPopup::Strong:
        default:
            setBackgroundItem(HbStyle::P_Popup_background);
            break;
        }
        d->frameType = frameType;
        updatePrimitives();
    }
}


/*!
 Shows the popup as modal popup returning immediately.  

 Connects aboutToClose() signal to the slot specified by \a receiver and
 \a member. The signal will be disconnected from the slot when the
 popup is closed.

 For non modal popups, use show().  
*/
void HbPopup::open( QObject *receiver, const char *member )
{
    Q_D(HbPopup);
    if (receiver) {
        connect(this, SIGNAL(aboutToClose()), receiver, member);
    }
    d->receiverToDisconnectOnClose = receiver;
    d->memberToDisconnectOnClose = member;

    show();
}

/*!
    \reimp
 */
QVariant HbPopup::itemChange ( GraphicsItemChange change, const QVariant & value )
{
    Q_D(HbPopup);

    /*if (change == QGraphicsItem::ItemVisibleHasChanged) {
        if (value.toBool()) {
            if(d->hasEffects && boundingRect().isValid()) {

#ifdef HB_EFFECTS
                QRectF extRect(0.0,
                               -boundingRect().height(),
                               boundingRect().width(),
                               0);
                HbEffect::start(this, d->effectType, "appear", 0, 0, QVariant(), extRect);
#endif//HB_EFFECTS
                d->mStartEffect = false;
            } else {
                d->mStartEffect = true;
            }
        }
    }*/

    if (change == QGraphicsItem::ItemVisibleChange) {
        if (value.toBool()) {
            if(!d->hasEffects){
                d->addPopupEffects();
            }
            if (!d->aboutToShowSignalGuard)
            {
                d->aboutToShowSignalGuard = true;
                emit aboutToShow();
            }
            // Note: when visibility changes to "visible" base class implementation needs
            //       to be called otherwise showEvent() is not called.
        } else {
            d->aboutToShowSignalGuard = false;
            if (!d->hidingInProgress) {
                emit aboutToHide();
            }

            if (d->delayedHide &&  // about to hide and we wanna delay hiding
                d->hasEffects && !parentItem()) { // only for popup without parent
                if (!d->hidingInProgress) { // Prevent reentrance
                    d->hidingInProgress = true;
#ifdef HB_EFFECTS
                    QRectF extRect(0.0,
                                   -boundingRect().height(),
                                   boundingRect().width(),
                                   0);
                    if (!HbEffect::start(this, d->effectType, "disappear",
                                         this, "_q_delayedHide",
                                         QVariant(), extRect)) {
                        d->delayedHide = false;
                    }
#endif
                }
                if (d->delayedHide) {
                    return true;
                } else {
                    d->delayedHide = d->hasEffects;
                    d->hidingInProgress = false;
                    // fallback to base class imp
                }
            }
        }
    } else if (change == QGraphicsItem::ItemSceneHasChanged) {
        HbMainWindow* w(mainWindow());
        if ( w ){
            disconnect(this, SLOT(handlePopupPos()));
            connect( w, SIGNAL(orientationChanged(Qt::Orientation)),
                     this, SLOT(handlePopupPos()) );
        }
    }
    return HbWidget::itemChange(change, value);
}

/*!
 Handles the popup position when Orientation changes
*/
void HbPopup::handlePopupPos()
{
    QEvent userEvent(QEvent::ContextMenu);
    QCoreApplication::sendEvent(this, &userEvent);
}

/*!
    \reimp
 */
void HbPopup::mousePressEvent(QGraphicsSceneMouseEvent *event )
{
    Q_D(HbPopup);

    Q_UNUSED(event);

    d->mousePressLocation = HbPopupPrivate::Popup;

    // We need to reimplement this function otherwise this events will be
    // ignored by default and we wont get further mouse events
    // not even mouseReleaseEvent. See doc of QGraphicsItem::mousePressEvent()
    // for more info.
    QGraphicsItem::mousePressEvent(event);

    // Event has to be accepted cause QGraphicsItem::mousePressEvent can mark it
    // to ignored
    event->accept();
}

/*!
    \reimp
 */
void HbPopup::mouseReleaseEvent(QGraphicsSceneMouseEvent *event )
{
    QGraphicsItem::mouseReleaseEvent(event);        
    event->accept();
    // Note: Mouse release event is always handled in handleBackgroundMouseReleaseEvent
}

/*!
    \reimp
 */
//
// Shows the popup with an animation and starts the timer to dismiss the popup,
// unless it is a permanent popup.
//
void HbPopup::showEvent(QShowEvent *event)
{
    Q_D(HbPopup);

    HbWidget::showEvent(event);

    // Only for popup without parent
    // Note:
    //  popups without parent are treated as popups i.e.:
    //  - can have background item
    //  - timeout
    //  - and registered to HbPopupManager
    //  otherwise popup is treated as normal widget
    if(d->duplicateShowEvent){
        d->duplicateShowEvent = false;
        return;
    }
    if (!parentItem()) {
        //check if popup needs to be added to scene.This can result in duplciate show event,
        // if popup is added to scene here.
        if(d->addPopupToScene()) {
              d->duplicateShowEvent = true;
        }
        // Popup clears closed state
        d->closed = false;
        if (d->backgroundItem) {
            d->backgroundItem->setVisible(true);
            d->backgroundItem->setAcceptHoverEvents(isModal());
            if (isModal()) {
                d->backgroundItem->setFlag(QGraphicsItem::ItemIsPanel);
             }
        }
        if (qobject_cast<HbGraphicsScene *>(scene())) {
            qobject_cast<HbGraphicsScene *>(scene())->d_ptr->showPopup(this);
            HbWidgetFeedback::triggered(this, Hb::InstantPopupOpened);
        }

        //workaround
        resetTransform();
        setOpacity(1);
        //workaround ends

        // delay hiding if effects are enabled
        d->delayedHide = d->hasEffects;

        // If it is not permanent launch a timer for closing the popup
        if (0 < d->timeout) {
            d->timeoutTimer()->setInterval(d->timeout);
            d->startTimeout();
        }
    }
}

/*!
    \reimp
 */
void HbPopup::hideEvent(QHideEvent *event)
{
    Q_D(HbPopup);

    HbWidget::hideEvent(event);

    // Only for popup without parent
    if (!parentItem()) {
        if (d->backgroundItem) {
            d->backgroundItem->setVisible(false);
        }
        qobject_cast<HbGraphicsScene *>(scene())->d_ptr->hidePopup(this);
    }

    HbWidgetFeedback::triggered(this, Hb::InstantPopupClosed);
    if (d->eventLoop) {
        d->eventLoop->exit();
    }

    d->doSetModal( d->modal );

}

/*!
    \reimp
 */
void HbPopup::resizeEvent( QGraphicsSceneResizeEvent * event )
{    
    HbWidget::resizeEvent(event);
    updatePrimitives();
#if 1
    Q_D(HbPopup);
    if (d->polished) {
        d->calculateShape();
    }
#endif
}

/*!
    \reimp
 */
void HbPopup::closeEvent ( QCloseEvent * event )
{
    Q_D(HbPopup);
    d->stopTimeout();
    // Only for popup without parent
    if (!d->closed && !parentItem()) {
        // Popup goes to closed state
        d->closed = true;

        emit aboutToClose();

        // prevent delete on close before effects are finished
        if (d->hasEffects && isVisible()) {
            d->deleteOnClose = testAttribute(Qt::WA_DeleteOnClose);
            setAttribute(Qt::WA_DeleteOnClose,false);
        }
        HbToolTip::hideText(qobject_cast<HbGraphicsScene *>(scene()));
    }
    if (d->receiverToDisconnectOnClose) {
        disconnect(this, SIGNAL(aboutToClose()),
                   d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose);
        d->receiverToDisconnectOnClose = 0;
    }
    d->memberToDisconnectOnClose.clear();
    HbWidget::closeEvent(event);
}

/*!
    \reimp
 */
bool HbPopup::event(QEvent *event)
{
/*    Q_D(HbPopup);
    if (event->type() == QEvent::LayoutRequest) {
        //Workaround when showing first time                           
#ifdef HB_EFFECTS
        if(d->mStartEffect && boundingRect().isValid()) {
            d->mStartEffect = false;
            QCoreApplication::sendPostedEvents(this, QEvent::LayoutRequest);
            QRectF extRect(0.0,
                           -boundingRect().height(),
                           boundingRect().width(),
                           0);
            HbEffect::start(this, d->effectType, "appear", 0, 0, QVariant(), extRect);            
            qDebug() << "effect start";
        }
#endif//HB_EFFECTS
        //workaround ends
    }
    qDebug() << "event: " << event;*/
    /*Q_D(HbPopup);
    if (event->type() == QEvent::LayoutDirectionChange) {
        d->calculateShape();
    }*/
    return HbWidget::event(event);
}

/*!
  Sets preferred position\a position for popup with \a placement
  as origin.

  \param position is the position at which the popup is shown.
  \param placement is the corner or edge which \a position refers to

  Example usage:
  \code
  HbDialog popup;
  ...
  popup.setPreferredPosition( QPointF(x,y), HbPopupBase::BottomEdgeCenter );
  popup.show();
  \endcode

 */

void HbPopup::setPreferredPos( const QPointF& preferredPos,
                               HbPopup::Placement placement )
{
    Q_D(HbPopup);
    bool layoutFlag = false;
    if (d->preferredPos != preferredPos ) {
        d->preferredPos = preferredPos;
        layoutFlag = true;
    }
    if (d->placement != placement) {
        d->placement = placement;
        layoutFlag = true;
    }
    d->preferredPosSet = true;
    //If position updated, informing layoutproxy with layoutrequest
    if (layoutFlag) {
        QApplication::sendEvent(this, new QEvent(QEvent::LayoutRequest));
    }
}

QPainterPath HbPopup::shape() const
{    
#if 0
   /*Q_D(const HbPopup);
    if (backgroundItem() && d->mPath) {
        return *d->mPath;
    } else {
        return HbWidget::shape();
    }*/
#else
    return HbWidget::shape();
#endif
}

#include "moc_hbpopup.cpp"
