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
#include "hbvgchainedeffect_p.h"
#include <QTimer>
#include <QGraphicsSceneMouseEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QPointer>
#include <QDebug>
#include <QBitmap>
#include <hbinstance_p.h>
#include <QGraphicsScene>
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
    \brief The HbPopup class defines a set of properties that control the behavior of 
    the many dialog and popup classes that derive from it. 
    
    Objects of classes derived from %HbPopup are called \b popups. Typically they
    are displayed above other objects within the view, at the highest z-order. The %HbPopup
    class simply defines a frame with a number of properties. If you want to create an
    application dialog with a heading, content and a toolbar, use the HbDialog class (which
    is derived from %HbPopup) or one of its convenience subclasses.
    
    %HbPopup provides properties that you can use to customize the following aspects of a popup:
    
    - \b Modality. You can define whether the popup is modal or non-modal. A modal popup stops
      the user interacting with anything outside of the popup until it closes. A non-modal 
      popup does not block the user's interaction with things outside of the popup. You 
      set and get the modality by calling setModal() and isModal(), respectively.  
    
    - <b>Dismiss policy</b>. You can define which user actions, if any, cause the popup to 
      close. Possible values are defined by HbPopup::DismissPolicy. You set and get the dismiss 
      policy by calling setDismissPolicy() and dismissPolicy(), respectively.
    
    - <b>Background fade policy</b>. You can set a policy to fade everything behind the 
      popup. This is useful for modal dialogs, because the user cannot interact with 
      anything behind the popup. You control this policy by calling setBackgroundFaded() and 
      get the current policy by calling isBackgroundFaded().
    
    - \b Timeout. You can define a timeout that causes the popup to be dismissed automatically
      after a set a period. You set and get this property by calling setTimeout() and 
      timeout(), respectively. Call the setTimeout(HbPopup::DefaultTimeout) overload to use the 
      default value for the popup type, which provides a common look and feel.
      
    - <b>Frame background type</b>. You can define the popup's background style. However, 
      the actual appearance of the popup depends on the theme. You set and get this property by
      calling setFrameType() and frameType(), respectively.
      
    By default, popups are displayed in the center of the screen. Typically you should not override
    the default position of modal dialogs. However, some popups, such as context menus, need to be 
    in a specific position. For these, you can use the setPreferredPos() method to set the 
    preferred position.
    
    Popups that contain an editor may be repositioned by the virtual keyboard when it opens. 
    When the virtual keyboard closes, it attempts to reposition the popup back to its 
    previous position. However, this is not always possible, particularly when there has also
    been an orientation switch.

    \section _usecases_hbpopup Using the HbPopup class
    
    Although it is possible to create an instance of the HbPopup class, it is designed as a
    base class to provide common features to the many popup and dialog classes that are 
    derived from it (such as HbMenu, HbDialog, HbToolBarExtension, HbZoomSliderPopup). 
    
    How you open a popup depends on whether it is modal. It is important to use the 
    appropriate method, because this ensures the correct touch event handling.
    
    <table border="1" style="border-collapse: collapse; border: solid;">
    <tr><th>To open</TH><TH>Call</th></tr>
    <tr><td>Modal popups</td><td>open()</td></tr>
    <tr><td>Non-modal popups</td><td>\link QGraphicsItem::show() show()\endlink</td></tr>
    </table>

    \sa HbDialog, HbMenu
*/

/*!
    \fn int HbPopup::type() const
 */

/*!
    \enum HbPopup::DefaultTimeout

    Identifies default timeout values for various popup types. The timeout defines a time
    period after which the popup is automatically closed. Using these default values 
    provides a consistent look and feel.
    \sa setTimeout(HbPopup::DefaultTimeout)
 */

/*!
    \var HbPopup::NoTimeout
    Defines a permanent popup, for which no timeout is defined.
 */

/*!
    \var HbPopup::ConfirmationNoteTimeout
    The default timeout value for confirmation notes.
 */

/*!
    \var HbPopup::StandardTimeout
    The default timeout value for standard non-permanent popups, such as notes.
 */

/*!
    \var HbPopup::ContextMenuTimeout
    The default timeout value for context menus.
 */

/*!
    \enum HbPopup::DismissPolicy

    Defines the available dismiss policy values. The dismiss policy defines which
    user actions cause the popup to close.
    
    \sa setDismissPolicy(), dismissPolicy()
 */

/*!
    \var HbPopup::NoDismiss
    The popup cannot be closed by the user.
 */

/*!
    \var HbPopup::TapInside
    Closes the popup when the user taps within the popup's bounding rectangle.
 */

/*!
    \var HbPopup::TapOutside

    Closes the popup when the user taps outside of the popup's bounding rectangle.
 */

/*!
    \var HbPopup::TapAnywhere
    Closes the popup when the user taps either inside or outside the popup's bounding 
    rectangle.
 */

/*!
    \enum HbPopup::FrameType

    Identifies the background graphical styles of the popup's frame.

    \sa setFrameType(), frameType()
 */

/*!
    \var HbPopup::Strong
    Use the "strong" frame background style.
 */

/*!
    \var HbPopup::Weak
    Use the "weak" frame background style.
 */

/*!
    \fn void HbPopup::aboutToShow();

    This signal is emitted when the popup is about to be shown; that is, when 
    \link QGraphicsItem::show() show()\endlink is called.
 */

/*!
    \fn void HbPopup::aboutToHide();

    This signal is emitted when the popup is about to be hidden; that is, when 
    \link QGraphicsItem::hide() hide()\endlink is called.
 */


/*!
    \fn void HbPopup::aboutToClose();

    This signal is emitted when the popup is about to close; that is, when 
    \link QGraphicsWidget::close() close()\endlink is called or the popup is dismissed 
    by the user or timeout.
 */

/*!
    \enum HbPopup::Placement
    Identifies the corners and edges of the popup for use when setting its position
    using setPreferredPos(). 
  */
  
/*!
    \var HbPopup::TopLeftCorner
    The popup's top left corner.
 */
  
/*!
    \var HbPopup::TopRightCorner
    The popup's top right corner.
 */
   
/*!
    \var HbPopup::BottomLeftCorner
    The popup's bottom left corner.
 */
   
/*!
    \var HbPopup::BottomRightCorner
    The popup's bottom right corner.
 */
   
/*!
    \var HbPopup::TopEdgeCenter
    The center of the popup's top edge.
 */
   
/*!
    \var HbPopup::RightEdgeCenter
    The center of the popup's rightmost edge.
 */
    
/*!
    \var HbPopup::BottomEdgeCenter
    The center of the popup's bottom edge.
 */
    
/*!
    \var HbPopup::LeftEdgeCenter
    The center of the popup's leftmost edge.
 */
 
 /*!
    \var HbPopup::Center
    The center of the popup.
 */

namespace {
    //initial values if not found from hbpopup.css
    static const int NO_TIMEOUT = 0;
    static const int CONFIRMATION_NOTE_TIMEOUT = 1500;
    static const int STANDARD_TIMEOUT = 3000;
    static const int CONTEXT_MENU_TIMEOUT = 6000;

    static const QLatin1String NO_TIMEOUT_STRING("no-timeout");
    static const QLatin1String CONFIRMATION_NOTE_TIMEOUT_STRING("confirmation-note-timeout");
    static const QLatin1String STANDARD_TIMEOUT_STRING("standard-timeout");
    static const QLatin1String CONTEXT_MENU_TIMEOUT_STRING("context-menu-timeout");
}

/*
   Currently, virtual keyboard must be able to position a popup
   containing an editor to an arbitrary place. VKB does its best to
   reposition the popup back to original position when needed. At least in
   orientation switch the old position naturally is wrong, hence popup
   must be relayouted.

   It would be unreasonable to make special checks for popup in vkb
   side. It also would be unreasonable to do special checks for vkb in
   popup side. Hence this semi-hidden dynamic property for
   communicating this special case.

   WARNING: Do not count on this special behaviour, we might remove it
   without prior notice. If you do require such a behavior, please
   raise a feature request and we might make this a proper API.
 */
const char* KPositionManagedByVKB("PositionManagedByVKB");

/*
    \primitives
    \primitive{background} HbFrameItem representing the popup background. The background can be 
    weak or strong (different graphical styles) depending on popup type.
    \primitive{P_Popup_heading_frame} HbFrameItem representing the popup heading text background
  */
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
    hasEffects(false),
    closed(false),
    hidingInProgress(true),
    showingInProgress(false),
    delayedHide(false),
    deleteOnClose(false),
    modal(true), // must be in sync QGraphicsItem::ItemIsFocusable of backgroundItem
    fadeBackground(true),
    inDestruction(false),
    aboutToShowSignalGuard(false),
    duplicateShowEvent(false),
    timedOut(false),
    timeout(-1),
    timeoutPending(true),
    pendingTimeout(HbPopup::StandardTimeout),
    //    priorityValue(HbPopup::Default),
    priorityValue(0),
    dismissPolicy(HbPopup::TapOutside),
    backgroundItem(0),
    mousePressLocation(None),
    frameType(HbPopup::Strong),
    preferredPosSet(false),
    placement(HbPopup::TopLeftCorner),
    mStartEffect(true),
    mScreenMargin(0.0),
    mAutoLayouting(true),
    mOriginalAutoLayouting(mAutoLayouting),
    mActivePopup(true),
    mMaskedItem(0),
    mPopupMask(0),
    mVgMaskEffect(0),
    mOrientationEffectHide(false),
    mGestureOverride(false),
    mFullScreen(false),
    timeoutTimerInstance(0)
{
}

HbPopupPrivate::~HbPopupPrivate()
{
    stopTimeout();
    delete timeoutTimerInstance;
    delete mPopupMask;
}

void HbPopupPrivate::init()
{
    Q_Q(HbPopup);

    q->setAttribute(Hb::InsidePopup);

    // By default popups are focusable
    q->setFocusPolicy(Qt::StrongFocus);
    q->setBackgroundItem(new HbFrameItem(QLatin1String("qtg_fr_popup"), HbFrameDrawer::NinePieces), -1);
    q->updatePrimitives();


    // Only for popup without parent
    if (!q->parentItem()) {
        backgroundItem = new HbPopupBackGround(q);
        backgroundItem->setVisible(false);
        // Popup is invisible by default (explicit show or open call is required)
        q->setVisible(false);
    }
    hidingInProgress = false; 
    QGraphicsItem::GraphicsItemFlags itemFlags = q->flags();
    itemFlags |= QGraphicsItem::ItemClipsToShape;
    itemFlags |= QGraphicsItem::ItemClipsChildrenToShape;
    itemFlags |= QGraphicsItem::ItemSendsGeometryChanges;
    itemFlags |= QGraphicsItem::ItemIsPanel;
    q->setFlags(itemFlags);
    q->setActive(false);

    q->setAcceptTouchEvents(true);

    defaultTimeouts[HbPopup::NoTimeout] = NO_TIMEOUT;
    defaultTimeouts[HbPopup::ConfirmationNoteTimeout] = CONFIRMATION_NOTE_TIMEOUT;
    defaultTimeouts[HbPopup::StandardTimeout] = STANDARD_TIMEOUT;
    defaultTimeouts[HbPopup::ContextMenuTimeout] = CONTEXT_MENU_TIMEOUT;
}

void HbPopupPrivate::_q_appearEffectEnded(HbEffect::EffectStatus status)
{
    Q_UNUSED(status);
    Q_Q(HbPopup);

    showingInProgress = false;
    q->emit popupReady();
}

void HbPopupPrivate::_q_maskEffectDestroyed()
{
    mVgMaskEffect = 0;
}

CSystemToneService* HbPopupPrivate::systemToneService()
{
    return HbInstancePrivate::d_ptr()->systemTone();
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
    mGestureOverride = false;
}

void HbPopupPrivate::_q_orientationAboutToChange(Qt::Orientation orient, bool animate)
{
    Q_UNUSED(orient);    
    Q_Q(HbPopup);
    if (animate && q->isVisible()) {
        HbEffect::start(q, "HB_POPUP", "orient_disappear");
        mOrientationEffectHide = true;
    }
}

#endif // HB_EFFECTS

void HbPopupPrivate::_q_orientationChanged()
{
    Q_Q(HbPopup);
    if (q->isVisible()) {
        QEvent userEvent(QEvent::LayoutRequest);
        QCoreApplication::sendEvent(q, &userEvent);
    }

    q->repolish();
#ifdef HB_EFFECTS
    if (mOrientationEffectHide) {
        QRectF extRect(0.0,
                       -q->boundingRect().height(),
                       q->boundingRect().width(),
                       0);
        HbEffect::cancel(q);
        HbEffect::start(q, "HB_POPUP", "orient_appear", q, "_q_appearEffectEnded",
                        QVariant(), extRect);
        mOrientationEffectHide = false;
        showingInProgress = true;
    }
#endif
}

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

    timeoutPending = false;
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
        timeoutTimerInstance = new QTimer();
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
    if (dismissPolicy & HbPopup::TapOutside && !modal) {
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
        } else if ( mousePressLocation == Background &&
                    dismissPolicy & HbPopup::TapOutside && modal) {
            q->close();
        }
    }

    // reset mousePressLocation
    mousePressLocation = None;
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
                                           "popup_orient_disappear",
                                           "orient_disappear");
    }
    if (hasEffects ) {
        hasEffects = HbEffectInternal::add("HB_POPUP",
                                           "popup_orient_appear",
                                           "orient_appear");
    }
    hasEffects = true; //Workaround until orient appear effects are in place
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

void HbPopupPrivate::setFullScreen(bool enable)
{
    if (mFullScreen != enable) {
        Q_Q(HbPopup);
        mFullScreen = enable;
        if (enable) {
            q->setBackgroundItem(new HbFrameItem(QLatin1String("qtg_graf_screen_bg_prt"), HbFrameDrawer::OnePiece), -1);
        } else {
            if (frameType == HbPopup::Weak) {
                q->setBackgroundItem(new HbFrameItem(QLatin1String("qtg_fr_popup_secondary"), HbFrameDrawer::NinePieces), -1);
            } else {
                q->setBackgroundItem(new HbFrameItem(QLatin1String("qtg_fr_popup"), HbFrameDrawer::NinePieces), -1);
            }
        }
        q->repolish();
    }
}

void HbPopupPrivate::calculateShape()
{
    Q_Q(HbPopup);
    // Only used for HbMenu currently, this is not fully correct as some dialogs may need masking too.
    if (q->type() != Hb::ItemType_Menu && q->type() != Hb::ItemType_ColorDialog) {
        return;
    }
    // Cannot set up masking if the background item is not available.
    if (!q->backgroundItem() || !q->backgroundItem()->boundingRect().isValid()) {
        return;
    }
    // Contrary to the name, HbVgMaskEffect has a software
    // implementation too, and we will actually force the usage of
    // that here, ignoring the pure OpenVG version.
    if (!mVgMaskEffect) {
        if (!mMaskedItem) {
            mMaskedItem = q;
        }
        mVgMaskEffect = new HbVgMaskEffect;
        // Masking does not work reliably on HW.
        mVgMaskEffect->setForceSwMode(true);
        // There may be children (like the scroll area in menus) that
        // would mess up the masking so exclude those.
        mVgMaskEffect->setIncludeSourceItemOnly(true);
        if (!mMaskedItem->graphicsEffect()) {
            // Attach the effect. Ownership is transferred to the chain. The
            // ownership of the chain is transferred to q.  The chain is needed
            // because we must co-exist with FXML-based filter effects.
            HbVgChainedEffect *c = new HbVgChainedEffect;
            c->add(mVgMaskEffect);            
            c->install(mMaskedItem);
        } else {
            // Avoid replacing already set effects. Do not mask if this is not
            // possible, otherwise we would unexpectedly delete the previously
            // set graphics effect. However by being able to add the effect to a
            // chain makes it possible to co-exist with FXML-based filter
            // effects. Similar solution is also present in HbEffectGroup.
            HbVgChainedEffect *c = qobject_cast<HbVgChainedEffect *>(mMaskedItem->graphicsEffect());
            if (c) {
                c->add(mVgMaskEffect);
            } else {
                delete mVgMaskEffect;
                mVgMaskEffect = 0;
            }
        }
        if (mVgMaskEffect) {
            // Make sure that mask effect is reset when destroyed
            QObject::connect(mVgMaskEffect, SIGNAL(destroyed()), q, SLOT(_q_maskEffectDestroyed()));
        }
    }
    /*if (!mPopupMask) {
        mPopupMask= new HbFrameItem();
        mPopupMask->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
        mPopupMask->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_mask"));
    }
    mPopupMask->setGeometry(q->boundingRect());*/

    QPixmap image(q->backgroundItem()->boundingRect().size().toSize());
    image.fill(Qt::transparent);
    QPainter imagePainter(&image);
    //mPopupMask->paint(&imagePainter, 0, 0);
    q->backgroundItem()->paint(&imagePainter, 0, 0);
    imagePainter.end();
    mVgMaskEffect->setMask(image);
}



/*!
    Constructs a popup with the given \a parent graphics item. For true popup 
    behavior (which means that it opens above other objects, at the highest z-order) 
    set \a parent to 0. This is the primary intended use of this class.

    However, it may sometimes be useful to embed a popup into a QGraphicsItem. To do 
    this, pass a non-zero \a parent value. HbPopup objects that have a parent are 
    not real popups: they behave like any other QGraphicsWidget object and following
    properties are ignored. In addition the aboutToClose() signal is not emitted.
    
    - \link isModal() modal\endlink property
    - \link timeout() timeout\endlink  property
    - \link isBackgroundFaded() Backgroundfaded\endlink property
    - \link  dismissPolicy() dismissPolicy\endlink property
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
    Destructor.
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
    Returns the popup's timeout property in milliseconds. The timeout causes the popup to
    be dismissed automatically after the specified time period has elapsed. A value of
    zero or less than zero, means that the popup is permanent and the user must take 
    some explicit action in order to close the popup. 
    
    The default value for this property is HbPopup::StandardTimeout.
      
    \sa setTimeout(), setTimeout(HbPopup::DefaultTimeout)
*/
int HbPopup::timeout() const
{
    Q_D(const HbPopup);
    if (d->timeoutPending) {
        HbStyleParameters params;
        QLatin1String timeoutString("");
        switch (d->pendingTimeout) {
            case HbPopup::NoTimeout:
                timeoutString = NO_TIMEOUT_STRING;
                break;
            case HbPopup::ContextMenuTimeout:
                timeoutString = CONTEXT_MENU_TIMEOUT_STRING;
                break;
            case HbPopup::StandardTimeout:
                timeoutString = STANDARD_TIMEOUT_STRING;
                break;
            case HbPopup::ConfirmationNoteTimeout:
                timeoutString = CONFIRMATION_NOTE_TIMEOUT_STRING;
                break;
        }
        params.addParameter(timeoutString);

        style()->widgetParameters(params, const_cast<HbPopup *>(this));

        if (!params.value(timeoutString).isNull()) {
           return params.value(timeoutString).toInt();
       } else {
           return d->timeout;
       }
    } else {
    return d->timeout;
}
}

/*!
    Sets the popup's timeout property in milliseconds. A value of zero or less than zero
    means that the popup is permanent and is not closed automatically.
    
    \overload
    \sa timeout() setTimeout(HbPopup::DefaultTimeout), QGraphicsWidget::close()
*/
void HbPopup::setTimeout(int timeout)
{
    Q_D(HbPopup);    
    d->setTimeout(timeout);
}

/*!
    Sets the popup's timeout property to the default value for a standard popup type.
    This has the advantage of creating a common look and feel.

    \overload
    \sa timeout(), setTimeout(int), QGraphicsWidget::close()
*/
void HbPopup::setTimeout(HbPopup::DefaultTimeout timeout)
{
    Q_D(HbPopup);
    if (d->polished) {
        int newTimeout = d->defaultTimeouts.value(timeout);        
        setTimeout(newTimeout);
    } else {
        d->timeoutPending = true;
        d->pendingTimeout = timeout;
    }
}

/*!
    Returns the popup's modality property. A modal popup blocks any user-initiated
    events outside of the popup until it is closed.
    \sa setModal()
*/
bool HbPopup::isModal() const
{
    Q_D(const HbPopup);
    return d->modal;
}

/*!
    Sets the popup's modality property.
    
    To open a modal popup, call open(). To open a non-modal popup, call 
    \link QGraphicsItem::show() show()\endlink.
    \sa isModal()
*/
void HbPopup::setModal(bool modal)
{
    Q_D(HbPopup);
    d->modal = modal;
    d->doSetModal( d->modal );
}

/*!
    Sets the popup's background fade policy property. When this is set to true, it causes
    everything behind the popup to be faded. 
    \sa isBackgroundFaded()
*/
void HbPopup::setBackgroundFaded(bool fadeBackground)
{
    Q_D(HbPopup);
    d->fadeBackground = fadeBackground;
}

/*!
    Returns the popup's background fade policy property. This controls whether 
    everything behind the popup is faded while the popup is on the screen. Typically
    this property is set to true for modal dialogs only. The default is true.
    \sa setBackgroundFaded()
*/
bool HbPopup::isBackgroundFaded() const
{
    Q_D(const HbPopup);
    return d->backgroundItem && d->fadeBackground;
}

/*!
    Returns the popup's dismiss policy. This defines which user actions (if any) cause 
    the popup to close. The default is HbPopup::TapOutside.
    \sa setDismissPolicy()
*/
HbPopup::DismissPolicy HbPopup::dismissPolicy() const
{
    Q_D(const HbPopup);
    return d->dismissPolicy;
}

/*!
    Sets the popup's dismiss policy property.

    \sa dismissPolicy()
*/
void HbPopup::setDismissPolicy(HbPopup::DismissPolicy dismissPolicy)
{
    Q_D(HbPopup);
    d->dismissPolicy = dismissPolicy;
}

/*!
    Returns the popup's frame type. This controls the popup's background style. However, 
    the actual appearance depends on the theme. The default value is HbPopup::Strong.
    
    \sa setFrameType()
*/
HbPopup::FrameType HbPopup::frameType() const
{
    Q_D(const HbPopup);
    return d->frameType;
}

void HbPopupPrivate::doSetFrameType(HbPopup::FrameType newFrameType)
{
    Q_Q(HbPopup);
    switch( newFrameType ) {
    case HbPopup::Weak:
        q->setBackgroundItem(new HbFrameItem(QLatin1String("qtg_fr_popup_secondary"), HbFrameDrawer::NinePieces), -1);
        break;
    case HbPopup::Strong:
    default:
        q->setBackgroundItem(new HbFrameItem(QLatin1String("qtg_fr_popup"), HbFrameDrawer::NinePieces), -1);
        break;
    }
}

/*!
    Sets the popup's frame type, which controls the popup's background style.

    \sa frameType()
*/
void HbPopup::setFrameType(HbPopup::FrameType frameType)
{
    Q_D(HbPopup);
    if ( d->frameType != frameType ) {
        d->frameType = frameType;
        d->doSetFrameType(frameType);
        updatePrimitives();
    }
}

/*!
    Returns true if the popup is set to occupy the whole screen, false if not.

    \sa setFullScreen()
*/
bool HbPopup::isFullScreen() const
{
    Q_D(const HbPopup);
    return d->mFullScreen;
}

/*!
    Sets the popup to occupy the whole screen. When the popup is in full screen different
    background graphics are used and there are no margins between popup and the main window.

    Inherited classes may have different color and graphics for full screen popups.

    \param enable A Boolean value; true for full screen popup, false for regular popup.

    \sa isFullScreen()
*/
void HbPopup::setFullScreen(bool fullScreen)
{
    Q_D(HbPopup);
    d->setFullScreen(fullScreen);
}

/*!
    Displays the popup on the screen and returns immediately. This function also 
    connects the popup's aboutToClose() signal to a specified slot. The signal 
    is disconnected from the slot when the popup closes.
    
    Use this function to open modal popups. To open non-modal popups, call 
    \link QGraphicsItem::show() show()\endlink.  

    \param receiver  The object that is to receive the signal.  
    \param member  The slot on the receiver to which the signal is to connect. 
    
    \sa isModal() 
*/
void HbPopup::open( QObject *receiver, const char *member )
{
    Q_D(HbPopup);
    if (receiver) {
        connect(this, SIGNAL(aboutToClose()), receiver, member);
    }
    d->receiverToDisconnectOnClose = receiver;
    d->memberToDisconnectOnClose = member;

    d->showingInProgress = true;
    show();
}

/*!
    
 */
QVariant HbPopup::itemChange ( GraphicsItemChange change, const QVariant & value )
{
    Q_D(HbPopup);

    if (change == QGraphicsItem::ItemPositionChange) {        
        // Skipping X-coordinate changes made by VKB since popupmanager handles popup to be in the
        // center of the screen
        if (property(KPositionManagedByVKB).isValid() && property(KPositionManagedByVKB).toBool() &&
            !d->mAutoLayouting && value.isValid() && !value.toPointF().isNull()) {
            QPointF newPointF(geometry().x(), value.toPointF().y());
            d->mAutoLayouting = false;
            return newPointF;
        }
        d->mAutoLayouting = false;
    }
    if (change == QGraphicsItem::ItemVisibleHasChanged) {
        if (value.toBool()) {
            if (d->hasEffects && boundingRect().isValid() && d->polished) {

#ifdef HB_EFFECTS
                QRectF extRect(0.0,
                               -boundingRect().height(),
                               boundingRect().width(),
                               0);
                HbEffect::cancel(this);
                d->mStartEffect = false;
                HbEffectInternal::start(this, this, HbEffectInternal::UpdateAtEachStep,
                                        d->effectType, "appear", this, "_q_appearEffectEnded",
                                        QVariant(), extRect);
                d->showingInProgress = true;
#endif //HB_EFFECTS
            }
        }
    }

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
            if(isPanel() && isActive() && d->mActivePopup) {
                setActive(false);
            }
            d->aboutToShowSignalGuard = false;
            if (!d->hidingInProgress) {
                emit aboutToHide();
            }
            if (d->delayedHide &&  // about to hide and we wanna delay hiding
                d->hasEffects && !parentItem()) { // only for popup without parent
                bool hideDelayed = d->delayedHide;
                if (!d->hidingInProgress) { // Prevent reentrance
                    d->hidingInProgress = true;
#ifdef HB_EFFECTS
                    QRectF extRect(0.0,
                                   -boundingRect().height(),
                                   boundingRect().width(),
                                   0);
                    HbEffect::cancel(this);
                    if (!HbEffectInternal::start(this, this, HbEffectInternal::UpdateAtEachStep,
                                                 d->effectType, "disappear",
                                                 this, "_q_delayedHide",
                                                 QVariant(), extRect)) {
                        d->delayedHide = false;
                        return HbWidget::itemChange(change, value);
                    }
                    d->mGestureOverride = true;
#endif
                }
                if (hideDelayed) {
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
            disconnect(this, SLOT(_q_orientationAboutToChange(Qt::Orientation, bool)));
            connect( w, SIGNAL(aboutToChangeOrientation(Qt::Orientation, bool)),
                     this, SLOT(_q_orientationAboutToChange(Qt::Orientation, bool)) );
            disconnect(this, SLOT(_q_orientationChanged()));
            connect( w, SIGNAL(orientationChanged(Qt::Orientation)),
                     this, SLOT(_q_orientationChanged()) );
        }

    }
    return HbWidget::itemChange(change, value);
}

/*!
    Reimplemented from QGraphicsItem.
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
    Reimplemented from QGraphicsItem.
 */
void HbPopup::mouseReleaseEvent(QGraphicsSceneMouseEvent *event )
{
    Q_D(HbPopup);
    d->handleBackgroundMouseReleaseEvent(event);

    QGraphicsItem::mouseReleaseEvent(event);
    event->accept();
}

/*!
    Reimplemented from QGraphicsWidget. 
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
        //setting popup as active panel failed in previous showEvent
        if (d->mActivePopup) {
            setActive(true);
        }
        return;
    }
    if (!parentItem()) {
        //check if popup needs to be added to scene.This can result in duplciate show event,
        // if popup is added to scene here.
        if(d->addPopupToScene()) {
              d->duplicateShowEvent = true;
        }
        if (d->mActivePopup) {
            setActive(true);
        }
        // Popup clears closed state
        d->closed = false;
        if (d->backgroundItem) {
            d->backgroundItem->setVisible(true);
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
    Reimplemented from QGraphicsWidget. 
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

    d->doSetModal( d->modal );
}

/*!
    Reimplemented from QGraphicsWidget. 
 */
void HbPopup::resizeEvent( QGraphicsSceneResizeEvent * event )
{    
    HbWidget::resizeEvent(event);
    updatePrimitives();
    Q_D(HbPopup);
    if (d->polished) {
        d->calculateShape();
    }
}

/*!
    Reimplemented from QGraphicsWidget. 
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
    Reimplemented from QGraphicsWidget. 
 */
bool HbPopup::event(QEvent *event)
{
    Q_D(HbPopup);
    if ( event->type() == QEvent::DynamicPropertyChange ) {
        QVariant v(property(KPositionManagedByVKB));
        if (v.isValid() ){
            if (v.toBool()) {
                // position is now managed by vkb
                d->mOriginalAutoLayouting = d->mAutoLayouting;
                d->mAutoLayouting = false;
            } else {
                d->mAutoLayouting = d->mOriginalAutoLayouting;

                // vkb has finished, and we might be on totally wrong
                // place.
                QEvent layoutRequest = QEvent::LayoutRequest;
                QApplication::sendEvent(this, &layoutRequest);
            }
        }
    } else if (event->type() == QEvent::LayoutRequest) {
        //Workaround when showing first time                           
#ifdef HB_EFFECTS
        if (d->mStartEffect && boundingRect().isValid()) {
            d->mStartEffect = false;
            QCoreApplication::sendPostedEvents(this, QEvent::LayoutRequest);
            QRectF extRect(0.0,
                           -boundingRect().height(),
                           boundingRect().width(),
                           0);
            HbEffect::cancel(this);
            HbEffectInternal::start(this, this, HbEffectInternal::UpdateAtEachStep,
                                    d->effectType, "appear", this, "_q_appearEffectEnded",
                                    QVariant(), extRect);
            d->showingInProgress = true;
        } else if (d->mStartEffect) {
        }
#endif //HB_EFFECTS
    } else if (event->type() == QEvent::TouchBegin) {
        // Accept all touch begin events to get the full Begin..End sequence
        event->accept();
        return true;
    } else if (d->mGestureOverride && event->type() == QEvent::GestureOverride) {
        event->accept();
        return true;
    }

    return HbWidget::event(event);
}

/*!
    Sets the preferred position of the popup. By default, the popup is placed in the middle
    of the screen. If you use this function to set the preferred position, it is your 
    responsibility to ensure that this position works correctly on devices with different 
    screen sizes.
  
    Typically you should set the preferred position only for context aware popups, such as 
    context menus and slider popups. The position is not relevant for most dialogs and
    popups and you should use the default position for these.

    \param preferredPos Defines the coordinates of preferred position on the screen where 
           the popup is to open.
    \param placement The corner or edge of the dialog that is to be placed at \a preferredPos.

    \b Example:
    \code
    HbPopup *popup = new HbPopup();
  
    popup->setPreferredPos( QPointF(x,y), HbPopup::BottomEdgeCenter );
    popup->show();
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
        QEvent layoutRequest = QEvent::LayoutRequest;
        QApplication::sendEvent(this, &layoutRequest);
    }
}

/*!
     Returns the shape of this item as a QPainterPath.
 */
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

/*!
    Reimplemented from HbWidget.
 */
void HbPopup::polish(HbStyleParameters &params)
{
    if (isVisible()) {
        Q_D(HbPopup);

        // fetch scrolling parameters from css

        params.addParameter(NO_TIMEOUT_STRING);
        params.addParameter(CONFIRMATION_NOTE_TIMEOUT_STRING);
        params.addParameter(STANDARD_TIMEOUT_STRING);
        params.addParameter(CONTEXT_MENU_TIMEOUT_STRING);

        HbWidget::polish(params);

        if (!params.value(NO_TIMEOUT_STRING).isNull()) {
            d->defaultTimeouts[HbPopup::NoTimeout] = params.value(NO_TIMEOUT_STRING).toInt();
        }
        if (!params.value(CONFIRMATION_NOTE_TIMEOUT_STRING).isNull()) {
            d->defaultTimeouts[HbPopup::ConfirmationNoteTimeout] = params.value(CONFIRMATION_NOTE_TIMEOUT_STRING).toInt();
        }
        if (!params.value(STANDARD_TIMEOUT_STRING).isNull()) {
            d->defaultTimeouts[HbPopup::StandardTimeout] = params.value(STANDARD_TIMEOUT_STRING).toInt();
        }
        if (!params.value(CONTEXT_MENU_TIMEOUT_STRING).isNull()) {
            d->defaultTimeouts[HbPopup::ContextMenuTimeout] = params.value(CONTEXT_MENU_TIMEOUT_STRING).toInt();
        }
        if (d->timeoutPending) {
            d->timeout = d->defaultTimeouts[d->pendingTimeout];
            if (d->timeout > 0) {
                d->timeoutTimer()->setInterval(d->timeout);
                d->startTimeout();
            }
            d->timeoutPending = false;
        }
    } else {
        HbWidget::polish(params);
    }
}

#include "moc_hbpopup.cpp"
