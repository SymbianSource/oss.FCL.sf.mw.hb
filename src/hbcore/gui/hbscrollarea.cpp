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

#include "hbscrollarea.h"
#include "hbscrollarea_p.h"
#include "hbscrollbar.h"
#include "hbdeviceprofile.h"
#include "hbinstance.h"
#include <hbwidgetfeedback.h>
#include <hbevent.h>
#include "hbglobal_p.h"

#include <QGesture>

#include <QDebug>

/*!
 @beta
 @hbcore
 \class HbScrollArea
 \brief HbScrollArea provides a finger-touch enabled scrollable container class.  
 
 HbScrollArea handles the events need to scroll the contents placed inside it.  It also
 handles the display of scrollbar while scrolling is occurring.
 
 HbScrollArea is used by constructing a QGraphicsWidget that contains the content to be
 displayed, then calling the setContentWidget() method.  The content widget must have its size
 set appropriately either by associating a layout with the widget or by explicitly setting
 the size (e.g. by calling QGraphicsWidget::setGeometry()).
 
 The class can be used by itself to provide default scrolling behavior or can be
 subclassed to add touch feedback, selection feedback, etc.
 
 By default, the class provides dragging, flicking with animated follow-on, a
 simple inertia algorithm for slowing the animated follow-on scrolling and
 a bounce-back algorithm for animating the content back to its bounding
 limits at the end of a drag or flick action.
 */

/*!
 Here are the main properties of the class:

 \li HbScrollArea::scrollDirections     : 
 \li HbScrollArea::clampingStyle        : 
 \li HbScrollArea::scrollingStyle       : 
 \li HbScrollArea::showScrollBars       : 
 \li HbScrollArea::scrollBarWidth       :  
 \li HbScrollArea::scrollBarMargin      : 
 \li HbScrollArea::frictionEnabled      :
 \li HbScrollArea::handleLongPress      :  
 */

/*!
    \property HbScrollArea::showScrollBars
    \brief
*/

/*!
    \property HbScrollArea::scrollBarWidth
    \brief
*/

/*!
    \property HbScrollArea::scrollBarMargin
    \brief
*/

/*!
    \property HbScrollArea::scrollDirections
    \brief 
*/

/*!
    \property HbScrollArea::clampingStyle
    \brief 
*/

/*!
    \property HbScrollArea::scrollingStyle
    \brief 
*/

/*!
    \property HbScrollArea::frictionEnabled
    \brief
*/

/*!
    \property HbScrollArea::handleLongPress
    \brief 
*/

/*!
    \property HbScrollArea::verticalScrollBarPolicy
    \brief
*/

/*!
    \property HbScrollArea::horizontalScrollBarPolicy
    \brief
*/

/*!
    \fn void HbScrollArea::scrollDirectionsChanged(Qt::Orientations newValue)

    This signal is emitted when scrolling direction is changed.
*/

/*!
    \fn void HbScrollArea::scrollingStarted()

    This signal is emitted whenever a scrolling action begins.
*/

/*!
    \fn void HbScrollArea::scrollingEnded()

    This signal is emitted whenever a scrolling action ends.
*/

/*!
    \fn void HbScrollArea::scrollPositionChanged(QPointF newposition)
    This signal is emitted when scroll position is changed.
*/

/*!
    \enum HbScrollArea::ClampingStyle

    Clamping styles supported by HbScrollArea.

    This enum describes how scrolling is behaving when reaching boundaries of the content item.
*/
/*!
    \var HbScrollArea::StrictClamping

    Scrolling is limited to the bounding rectangle of the content item.
*/
/*!
    \var HbScrollArea::BounceBackClamping

    Scrolling can go beyond the bounding rectangle of the content item, but bounces back to the
    limits of the bounding rectangle when released or when inertia scrolling stops.
*/
/*!
    \var HbScrollArea::NoClamping

    Scrolling is completely unclamped (this is usually used when the subclass implements its own).
*/
/*!
    \enum HbScrollArea::ScrollingStyle

    Different scrolling styles supported by HbScrollArea.

    This enum describes how scroll area can be scrolled.
*/
/*!
    \var HbScrollArea::Pan

    Dragging motion pans the view with no follow-on scrolling animation.
*/
/*!
    \var HbScrollArea::PanOrFlick

    Dragging motion pans the view with no follow-on scrolling animation,
    quick flicking motion triggers scrolling animation.
*/
/*!
    \var HbScrollArea::PanWithFollowOn

    Dragging motion pans the view, velocity at end of drag motion triggers follow-on animated scrolling.
*/

/*!
    \enum HbScrollArea::ScrollBarPolicy

    This enum type describes the various visibility modes of HbScrollArea's scroll bars.
*/

/*!
    \var HbScrollArea::ScrollBarAsNeeded

    HbScrollArea shows a scroll bar when the content is too large to fit and not otherwise.
*/
/*!
    \var HbScrollArea::ScrollBarAlwaysOff

    HbScrollArea never shows a scroll bar.
*/
/*!
    \var HbScrollArea::ScrollBarAlwaysOn

    HbScrollArea always shows a scroll bar.
*/
/*!
    \var HbScrollArea::ScrollBarAutoHide

    HbScrollArea shows scroll bar for short period of time when the content is displayed or scrolled. Scroll bar is not shown if not needed.

    This is the default behavior.
*/

/*!
  Constructor
 
  \sa HbScrollArea::HbScrollArea
 */
HbScrollArea::HbScrollArea(QGraphicsItem* parent) : 
        HbWidget( *new HbScrollAreaPrivate, parent )
{
    Q_D( HbScrollArea );
    d->q_ptr = this;
    d->init();
}

HbScrollArea::HbScrollArea(HbScrollAreaPrivate &dd, QGraphicsItem *parent):
        HbWidget( dd, parent  )
{
    Q_D( HbScrollArea );
    d->q_ptr = this;
    d->init();
}
   
/*!
 Destructor
 
 \sa HbScrollArea::~HbScrollArea
 */
HbScrollArea::~HbScrollArea()
{
    Q_D( HbScrollArea );
    if (d && d->mContents) {
        d->mContents->setParentLayoutItem(0);
    }
}

/*!
 Returns a pointer to the QGraphicsWidget,which is the content of scrollable area.
 
 \sa HbScrollArea::setContentWidget()
 */
QGraphicsWidget* HbScrollArea::contentWidget() const
{
    Q_D( const HbScrollArea );

    return d->mContents;
}

/*!
 Assigns the QGraphicsWidget that is to be scrolled.  The HbScrollArea widget takes ownership of
 the QGraphicsWidget.
 
 \sa HbScrollArea::contentWidget()
 */
void HbScrollArea::setContentWidget(QGraphicsWidget* contents)
{
    Q_D( HbScrollArea );

    if ( contents == d->mContents ) {
        return;
    }

    d->stopAnimating();

    if (0 != d->mContents) {
        d->mContents->setParentLayoutItem(0);
        delete d->mContents;
    }
    d->mContents = contents;
    updateGeometry();

    if (0 != contents) {
        contents->setParentLayoutItem(this);
        contents->setParentItem(this);
        contents->installEventFilter(this);
        d->mResetAlignment = true;
        d->adjustContent();
        setContinuationIndicators(d->mContinuationIndicators);
    } else {
        d->hideChildComponents();
    }
}

/*!
Removes the content widget, which is set to the scrollarea and returns it.
The ownership of the \a QGraphicsWidget is transferred to the caller.

\note This function is particularly useful if one wants to switch between
different contents without deleting previous content.

    \sa setContentWidget()
*/
QGraphicsWidget *HbScrollArea::takeContentWidget()
{
    Q_D(HbScrollArea);
    QGraphicsWidget* content = d->mContents;
    d->mContents = 0;

    // Reset the ownership
    if (content) {
        content->setParentLayoutItem(0);
        content->setParentItem(0);
        content->removeEventFilter(this);
    }

    d->hideChildComponents();
    return content;
}

/*!
 Returns the value of the clampingStyle property
 
 \sa HbScrollArea::setClampingStyle()
 */
HbScrollArea::ClampingStyle HbScrollArea::clampingStyle() const
{
    Q_D( const HbScrollArea );

    return d->mClampingStyle;
}

/*!
 Sets the clampingStyle property that controls how the scrolling is constrained
 relative to the contents of the scrolling area.
 
 Possible values for the clamping style include:
 
 	StrictClamping - scrolling is limited to the bounding rectangle of the content item
 	BounceBackClamping - scrolling can go beyond the bounding rectangle of the content item, but bounces back to the
 					limits of the bounding rectangle when released or when inertia scrolling stops
 	NoClamping - scrolling is completely unclamped (this is usually used when the subclass implements its own
 					custom clamping behavior)
 
 The default value is BounceBackClamping.
 
 \sa HbScrollArea::clampingStyle()
 */
void HbScrollArea::setClampingStyle(ClampingStyle value)
{
    Q_D( HbScrollArea );

    d->mClampingStyle = value;
}

/*!
  Returns the value of the scrollingStyle property
 
  \sa HbScrollArea::setScrollingStyle()
 */
HbScrollArea::ScrollingStyle HbScrollArea::scrollingStyle() const
{
    Q_D( const HbScrollArea );

    return d->mScrollingStyle;
}

/*!
  Sets the scrollingStyle property that controls how the style of scrolling interaction
  provided by the widget
 
  Possible values for the clamping style include:
 
 		Pan - dragging motion pans the view with no follow-on scrolling animation
               \deprecated PanOrFlick
                    is deprecated.
 		PanWithFollowOn - dragging motion pans the view, velocity at end of drag motion triggers follow-on animated scrolling
 
  The default value is PanWithFollowOn.
 
  \sa HbScrollArea::scrollingStyle()
 */
void HbScrollArea::setScrollingStyle(ScrollingStyle value)
{
    Q_D( HbScrollArea );

    if (value == HbScrollArea::PanOrFlick) {
        d->mScrollingStyle = HbScrollArea::PanWithFollowOn;
        HB_DEPRECATED("HbScrollArea::PanOrFlick is deprecated");
    } else {
        d->mScrollingStyle = value;
    }
}

/*!
  Returns the value of the scrollDirections property.
 
  \sa HbScrollArea::setScrollDirections()
 */
Qt::Orientations HbScrollArea::scrollDirections() const
{
    Q_D( const HbScrollArea );

    return d->mScrollDirections;
}

/*!
  Sets the value of the scrollDirections property.  This value is of
  type Qt::Orientations and can set to either Qt::Horizontal to enable horizontal scrolling,
  Qt::Vertical to enable vertical scrolling or (Qt::Horizontal | Qt::Vertical) to enable
  scrolling in both directions.
 
  The default value is Qt::Vertical.
 
  \sa HbScrollArea::scrollDirections()
 */
void HbScrollArea::setScrollDirections(Qt::Orientations value)
{
    Q_D( HbScrollArea );

    bool isChanged = (d->mScrollDirections != value);

    d->mScrollDirections = value;

    if (isChanged) {
        emit scrollDirectionsChanged( value );
    }
}

/*!
  Returns true if the inertia scrolling effect is enabled, false otherwise.
 
  \sa HbScrollArea::setFrictionEnabled()
 */
bool HbScrollArea::frictionEnabled() const
{
    Q_D( const HbScrollArea );

    return d->mFrictionEnabled;
}

/*!
  Enables/disables the inertia scrolling effect.
  By default, the inertia effect is enabled.
 
  \sa HbScrollArea::frictionEnabled()
 */
void HbScrollArea::setFrictionEnabled(bool value)
{
    Q_D( HbScrollArea );

    d->mFrictionEnabled = value;
}

/*!
  Returns true if the scroll area handles
  long press gestures, false otherwise
 
  \deprecated HbScrollArea::longPressEnabled()
      is deprecated.
 
  \sa HbScrollArea::setHandleLongPress()
 */
bool HbScrollArea::longPressEnabled() const
{
    HB_DEPRECATED("HbScrollArea::longPressEnabled() is deprecated");    
    return false;
}

/*!
  Sets the value of the handleLongPress property.  This value is set
  to true if the widget is to respond to long press gestures, false otherwise.
 
  The default value is false.
 
  \deprecated HbScrollArea::setLongPressEnabled(bool)
        is deprecated.
 
  \sa HbScrollArea::handleLongPress()
 */
void HbScrollArea::setLongPressEnabled (bool value)
{
    HB_DEPRECATED("HbScrollArea::setLongPressEnabled(bool) is deprecated");
    Q_UNUSED(value);
}

/*
  \reimp
 */
QVariant HbScrollArea::itemChange(GraphicsItemChange change, const QVariant &value)
{
    Q_D( HbScrollArea );

    if (change == QGraphicsItem::ItemVisibleHasChanged && d->mContents) {
        if (value.toBool() == true)
            d->adjustContent();
        else
            d->_q_hideScrollBars();
    }

    return HbWidget::itemChange(change, value);
}

/*! @beta
  upGesture() is a virtual slot function that is called whenever an
  up flick gesture is detected, if the scrollDirection is set to
  enable vertical scrolling.
 
  Derived classes can override this method to add custom handling of
  the gesture.  In most cases, derived classes should call up to the
  HbScrollArea parent method.
 
  \deprecated HbScrollArea::upGesture(int)
        is deprecated.
 */
void HbScrollArea::upGesture(int speedPixelsPerSecond)
{
    HB_DEPRECATED("HbScrollArea::upGesture(int) is deprecated. Use gesture FW.");

    Q_UNUSED(speedPixelsPerSecond);
}

/*! @beta
  downGesture() is a virtual slot function that is called whenever an
  down flick gesture is detected, if the scrollDirection is set to
  enable vertical scrolling. 
 
  Derived classes can override this method to add custom handling of
  the gesture.  In most cases, derived classes should call up to the
  HbScrollArea parent method.
 
  \deprecated HbScrollArea::downGesture(int)
        is deprecated.
 */
void HbScrollArea::downGesture(int speedPixelsPerSecond)
{
    HB_DEPRECATED("HbScrollArea::downGesture(int) is deprecated. Use gesture FW.");
    Q_UNUSED(speedPixelsPerSecond);
}

/*! @beta
  leftGesture() is a virtual slot function that is called whenever an
  left flick gesture is detected, if the scrollDirection is set to
  enable horizontal scrolling.
 
  Derived classes can override this method to add custom handling of
  the gesture.  In most cases, derived classes should call up to the
  HbScrollArea parent method.
 
  \deprecated HbScrollArea::leftGesture(int)
            is deprecated.
 */
void HbScrollArea::leftGesture(int speedPixelsPerSecond)
{
    HB_DEPRECATED("HbScrollArea::leftGesture(int) is deprecated. Use gesture FW.");
    Q_UNUSED(speedPixelsPerSecond);
}

/*! @beta
  rightGesture() is a virtual slot function that is called whenever an
  right flick gesture is detected, if the scrollDirection is set to
  enable horizontal scrolling.
 
  Derived classes can override this method to add custom handling of
  the gesture.  In most cases, derived classes should call up to the
  HbScrollArea parent method.
 
  \deprecated HbScrollArea::rightGesture(int)
        is deprecated.
 */
void HbScrollArea::rightGesture(int speedPixelsPerSecond)
{
    HB_DEPRECATED("HbScrollArea::rightGesture(int) is deprecated. Use gesture FW.");
    Q_UNUSED(speedPixelsPerSecond);
}


/*!
  panGesture() is a virtual slot function that is called whenever an
  pan gesture is detected.
 
  Derived classes can override this method to add custom handling of
  the gesture.  In most cases, derived classes should call up to the
  HbScrollArea parent method.
 
  \deprecated HbScrollArea::panGesture(const QPointF&)
   is deprecated.
 */
void HbScrollArea::panGesture(const QPointF &delta)
{
    HB_DEPRECATED("HbScrollArea::panGesture(const QPointF &) is deprecated. Use gesture FW.");
    Q_UNUSED(delta);
}

/*!  @beta
  longPressGesture() is a virtual slot function that is called whenever an
  long press gesture is detected, if the handleLongPress property is set to true.
 
  Derived classes can override this method to add custom handling of
  the gesture.  By default, HbScrollArea does not respond to a long press.
 
  \deprecated HbScrollArea::longPressGesture(const QPointF&)
   is deprecated.
 
  \sa setHandleLongPress(), handleLongPress()
 */
void HbScrollArea::longPressGesture(const QPointF &)
{
    HB_DEPRECATED("HbScrollArea::longPressGesture(const QPointF &) is deprecated. Use gesture FW.");
}

/*
    \reimp
*/
void HbScrollArea::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED (event);
}

/*!
  Returns true if a scrolling action is in progress, false otherwise.
 */
bool HbScrollArea::isScrolling() const
{
    Q_D( const HbScrollArea );

    return d->mIsScrolling;
}

/*!
  Returns true if the scrolling is due to dragging as opposed to follow-on scrolling
 */
bool HbScrollArea::isDragging() const
{
    Q_D( const HbScrollArea );

    return (d->mIsScrolling && !d->mIsAnimating);
}

/*!
  Scrolls the view by the amount indicated by "delta".
 
  The function returns TRUE if the view was able to scroll, FALSE otherwise.
 
  The function is virtual so subclasses can override it to customize the behavior by, for example, 
  clamping the position so that at least one item in the view remains visible.
 */
bool HbScrollArea::scrollByAmount(const QPointF& delta)
{
    Q_D( HbScrollArea );

    return d->scrollByAmount(delta);
}
 
/*!
   \reimp
 */
bool HbScrollArea::event(QEvent *event)
{
    Q_D(HbScrollArea);
    bool value(false);
    if(event) {
        value = HbWidget::event(event);
        if(event->type() == QEvent::ApplicationLayoutDirectionChange
                   || event->type() == QEvent::LayoutDirectionChange) {
             d->changeLayoutDirection(layoutDirection());
        } else if (event->type() == HbEvent::ChildFocusOut) {
            //qDebug() << "focusout";
            if ( !d->positionOutOfBounds() ) {
                d->stopAnimating();
            }
        } else if( event->type() == QEvent::GestureOverride ) {
            if(static_cast<QGestureEvent *>(event)->gesture(Qt::TapGesture) &&
                    d->mIsAnimating && !d->positionOutOfBounds() && !d->mMultiFlickEnabled) {
                event->accept();
                return true;
            }
        } else if (event->type() == QEvent::LayoutRequest) {
            if (d->mContents) {
                if (preferredSize() != d->mContents->preferredSize()) {
                    updateGeometry();
                }

                QSizeF newSize = d->mContents->size();
                QSizePolicy contentPolicy = d->mContents->sizePolicy();

                if (d->mScrollDirections & Qt::Vertical) {
                    if ((contentPolicy.verticalPolicy() & QSizePolicy::ExpandFlag) &&
                        (d->mContents->preferredHeight() < size().height())) {
                        newSize.setHeight(size().height());
                    } else if (contentPolicy.verticalPolicy() != QSizePolicy::Ignored) {
                        newSize.setHeight(d->mContents->preferredHeight());
                    }
                } else {
                    newSize.setHeight(size().height());
                }

                if (d->mScrollDirections & Qt::Horizontal) {
                    if ((contentPolicy.horizontalPolicy() & QSizePolicy::ExpandFlag) &&
                        (d->mContents->preferredWidth() < size().width())) {
                        newSize.setWidth(size().width());
                    } else if (contentPolicy.horizontalPolicy() != QSizePolicy::Ignored) {
                        newSize.setWidth(d->mContents->preferredWidth());
                    }
                } else {
                    newSize.setWidth(size().width());
                }

                d->mContents->resize(newSize);
            }
        } else if (event->type() == QEvent::GraphicsSceneResize) {
            if (d->mContents) {
                if ( d->mIsAnimating ) {
                    d->stopAnimating();
                }
                QSizeF newSize = d->mContents->size();
                bool sizeChanged = false;

                if (!(d->mScrollDirections & Qt::Vertical)) {
                    newSize.setHeight(size().height());
                    sizeChanged = true;
                }

                if (!(d->mScrollDirections & Qt::Horizontal)) {
                    newSize.setWidth(size().width());
                    sizeChanged = true;
                }
                if (sizeChanged) {
                    d->mContents->resize(newSize);
                } else {
                    d->adjustContent();
                }
            }
        }
    }
  return value;
}

/*!
   \reimp
 */
bool HbScrollArea::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    Q_D(HbScrollArea);
    if (event && event->type() == QEvent::GraphicsSceneResize) {
        if (isVisible()) {            
            d->adjustContent();
        }
    }  // no else

    return false;
}

/*!
    \reimp
*/
QSizeF HbScrollArea::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_D ( const HbScrollArea );

    QSizeF sh(0, 0);

    switch (which) {
        case Qt::MinimumSize:
            break;
        case Qt::PreferredSize:
            if (d->mContents) {
                sh = d->mContents->effectiveSizeHint( which, constraint );
            } else {
                sh = HbWidget::sizeHint( which, constraint );
            }
            break;
        case Qt::MaximumSize:
            sh = QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            break;
        default:
            qWarning("QGraphicsWidget::sizeHint(): Don't know how to handle the value of 'which'");
            break;
    }
    
    return sh;
}

/*!
 \reimp
 */
void HbScrollArea::focusOutEvent( QFocusEvent *event )
{
    Q_D ( HbScrollArea );
    Q_UNUSED ( event );

    if ( !d->positionOutOfBounds() ) {
        d->stopAnimating();
    }

}

#ifdef HB_GESTURE_FW
void HbScrollArea::gestureEvent(QGestureEvent *event)
{    
    Q_D ( HbScrollArea );
    if(QTapGesture *gesture = static_cast<QTapGesture *>(event->gesture(Qt::TapGesture))) {        
        // Stop scrolling on tap
        if (gesture->state() == Qt::GestureStarted) {
            if (d->mIsAnimating && !d->positionOutOfBounds() && !d->mMultiFlickEnabled) {
                d->stopAnimating();
                HbWidgetFeedback::triggered(this, Hb::InstantPressed, Hb::ModifierScrolling);
                event->accept(gesture);
            } else {
                event->ignore(gesture);
            }
        }
    }
    if (QPanGesture *panGesture = qobject_cast<QPanGesture*>(event->gesture(Qt::PanGesture))) {
        if (!d->pan(panGesture)) {
            event->ignore(panGesture);
        } else {
            event->accept(panGesture);
            setFocus(Qt::MouseFocusReason);
        }
    }
}

#endif

/*!
    Returns the scrollbar policy for vertical scrollbar

    \sa horizontalScrollBarPolicy(), setVerticalScrollBarPolicy()
*/

HbScrollArea::ScrollBarPolicy HbScrollArea::verticalScrollBarPolicy() const
{
    Q_D(const HbScrollArea);
    return d->mVerticalScrollBarPolicy;
}

/*!
    Sets the policy for vertical scrollbar

    The default policy is HbScrollArea::ScrollBarAutoHide.

    \sa setHorizontalScrollBarPolicy(), verticalScrollBarPolicy()
*/
void HbScrollArea::setVerticalScrollBarPolicy(ScrollBarPolicy policy)
{
    Q_D(HbScrollArea);
    d->setScrollBarPolicy(Qt::Vertical, policy);
}

/*!
  Returns the vertical scroll bar.

  \sa verticalScrollBarPolicy(), horizontalScrollBar()
 */
HbScrollBar *HbScrollArea::verticalScrollBar() const
{
    Q_D(const HbScrollArea);
    return d->mVerticalScrollBar;
}

/*!
   Replaces the existing vertical scroll bar with \a scrollBar. The former
   scroll bar is deleted.

   HbScrollArea already provides vertical and horizontal scroll bars by
   default. You can call this function to replace the default vertical
   scroll bar with your own custom scroll bar.

   \sa verticalScrollBar(), setHorizontalScrollBar()
*/
void HbScrollArea::setVerticalScrollBar(HbScrollBar *scrollBar)
{
    Q_D(HbScrollArea);
    if (!scrollBar) {
        qWarning("HbScrollArea::setVerticalScrollBar: Cannot set a null scroll bar");
        return;
    }

    d->replaceScrollBar(Qt::Vertical, scrollBar);
}

/*!
    \brief Returns the policy for horizontal scrollbar

    \sa verticalScrollBarPolicy(), setHorizontalScrollBarPolicy()
*/

HbScrollArea::ScrollBarPolicy HbScrollArea::horizontalScrollBarPolicy() const
{
    Q_D(const HbScrollArea);
    return d->mHorizontalScrollBarPolicy;
}

/*!
    \brief Sets the policy for horizontal scrollbar

    The default policy is HbScrollArea::ScrollBarAutoHide.

    \sa setVerticalScrollBarPolicy(), horizontalScrollBarPolicy()
*/
void HbScrollArea::setHorizontalScrollBarPolicy(ScrollBarPolicy policy)
{
    Q_D(HbScrollArea);    
    d->setScrollBarPolicy(Qt::Horizontal, policy);
}

/*!
  Returns the horizontal scroll bar.

  \sa horizontalScrollBarPolicy(), verticalScrollBar()
 */
HbScrollBar *HbScrollArea::horizontalScrollBar() const
{
    Q_D(const HbScrollArea);
    return d->mHorizontalScrollBar;
}

/*!
   Replaces the existing horizontal scroll bar with \a scrollBar. The former
   scroll bar is deleted.

   HbScrollArea already provides vertical and horizontal scroll bars by
   default. You can call this function to replace the default horizontal
   scroll bar with your own custom scroll bar.

   \sa horizontalScrollBar(), setVerticalScrollBar()
*/
void HbScrollArea::setHorizontalScrollBar(HbScrollBar *scrollBar)
{
    Q_D(HbScrollArea);
    if (!scrollBar) {
        qWarning("HbScrollArea::setHorizontalScrollBar: Cannot set a null scroll bar");
        return;
    }

    d->replaceScrollBar(Qt::Horizontal, scrollBar);
}

/*!
    \brief the alignment of the scroll area's widget    

    By default, the widget stays rooted to the top-left corner of the
    scroll area.
    \sa alignment
*/

void HbScrollArea::setAlignment(Qt::Alignment alignment)
{
    Q_D(HbScrollArea);
    d->mAlignment = alignment;
    d->mResetAlignment = true;
    if (d->mContents)
        d->adjustContent();
}
/*!
    \brief the alignment of the scroll area's widget

    By default, the widget stays rooted to the top-left corner of the
    scroll area.
    \sa setAlignment
*/
Qt::Alignment HbScrollArea::alignment() const
{
    Q_D(const HbScrollArea);
    return d->mAlignment;
}


/*!
    \brief Contination indicators for scroll area

    By default continuation graphics are disabled. When continuation
    graphics are enabled, scroll area shows indications where it is
    possible to scroll.

    \sa continuationIndicators
*/

void HbScrollArea::setContinuationIndicators(bool indication)
{
    Q_D(HbScrollArea);
    if (d->mContinuationIndicators == indication)
        return;
    d->mContinuationIndicators = indication;
    d->createPrimitives();
    d->updatePrimitives();
    if (d->mContents && indication) {
        d->updateIndicators(-d->mContents->pos());
    }
    repolish();
}

/*!
    \brief Contination indicators for scroll area

    By default continuation graphics are disabled. When continuation
    graphics are enabled, scroll area shows indications where it is
    possible to scroll.

    \sa setContinuationIndicators
*/
bool HbScrollArea::continuationIndicators() const
{
    Q_D(const HbScrollArea);
    return d->mContinuationIndicators;
}


/*!
    Scrolls the contents of the scroll area so that the point \a position is visible
    inside the region of the scrollArea with margins specified in pixels by \a xMargin and
    \a yMargin. If the specified point cannot be reached, the contents are scrolled to
    the nearest valid position. The default and minimum valid value for both margins is 0 pixels.
    Valid range for \a xMargin is between 0 and width of scrollArea, valid range for \a yMargin is
    between 0 and height of scrollArea.\a xMargin, \a yMargin, x and y values of \a position will be
    ignored depending on scrollingDirection.

    \sa setScrollDirections()
*/
void HbScrollArea::ensureVisible(const QPointF& position, qreal xMargin, qreal yMargin)
{
    Q_D(HbScrollArea);
    d->ensureVisible(position, xMargin, yMargin);
}

/*!
    Scrolls the contents of the scroll area to the \a newPosition in a given \a time.
    If the time is 0, contents is scrolled to the position instantly.
*/
void HbScrollArea::scrollContentsTo (const QPointF& newPosition, int time) {
    Q_D(HbScrollArea);
    if (time > 0){
        d->startTargetAnimation (newPosition, qMax (0, time));
    } else {
        scrollByAmount(newPosition - (-d->mContents->pos()));
        d->stopScrolling();

    }
}

/*
  \reimp
 */
void HbScrollArea::polish(HbStyleParameters& params)
{
    Q_D(HbScrollArea);

    d->doLazyInit();

    // fetch scrolling parameters from css
    const QString SpeedFactor = "speed-factor";
    const QString IntertiaSpeedFactor = "inertia-speed-factor";
    const QString MaxScrollSpeed = "max-scroll-speed";
    const QString SpringStrength = "spring-strength";
    const QString SpringDampingFactor = "spring-damping-factor";
    const QString FrictionPerMilliSecond = "friction";

    params.addParameter(SpeedFactor);
    params.addParameter(IntertiaSpeedFactor);
    params.addParameter(MaxScrollSpeed);
    params.addParameter(SpringStrength);
    params.addParameter(SpringDampingFactor);
    params.addParameter(FrictionPerMilliSecond);
    HbWidget::polish(params);

    if (!params.value(SpeedFactor).isNull()) {
        d->mSpeedFactor = params.value(SpeedFactor).toDouble();
    }
    if (!params.value(IntertiaSpeedFactor).isNull()) {
        d->mInertiaSpeedFactor = params.value(IntertiaSpeedFactor).toDouble();
    }
    if (!params.value(MaxScrollSpeed).isNull()) {
        d->mMaxScrollSpeed = params.value(MaxScrollSpeed).toDouble();
    }
    if (!params.value(SpringStrength).isNull()) {
        d->mSpringStrength = params.value(SpringStrength).toDouble();
    }
    if (!params.value(SpringDampingFactor).isNull()) {
        d->mSpringDampingFactor = params.value(SpringDampingFactor).toDouble();
    }
    if (!params.value(FrictionPerMilliSecond).isNull()) {
        d->mFrictionPerMilliSecond = params.value(FrictionPerMilliSecond).toDouble();
    }
    if (d->mContinuationIndicators) {
        d->updateIndicators(-d->mContents->pos());
    }
}

#include "moc_hbscrollarea.cpp"
