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
#include <hbtapgesture.h>
#include <hbnamespace_p.h>
#include <hbstyleframeprimitivedata.h>
#include <QGraphicsSceneResizeEvent>

#include <QGesture>

#include <QDebug>

/*!
    @beta
    @hbcore
    \class HbScrollArea
    \brief The HbScrollArea class provides a touch-enabled scrolling view onto another
    widget.

    A scroll area displays the contents of another widget within a frame. If the size of
    this widget exceeds the size of the frame, the user can scroll so that the entire
    contents can be viewed. The user scrolls through the contents by using a dragging
    gesture. If scroll bars are enabled and the scroll bar is interactive, the user can
    also scroll through the contents using the scroll bar (HbScrollBar class).

    %HbScrollArea is a concrete class that you can use directly in your applications to
    provide default scrolling behavior. %HbScrollArea is also a base class for
    HbAbstractItemView and the item view classes that derive from it. If necessary, you
    can subclass %HbScrollArea to add additional features, such as touch and selection
    feedback.

    \image html hbscrollarea.png A scroll area containing a large image and showing scrollbars

    %HbScrollArea provides properties that you can use to customize the following aspects
    of the scroll area:

    - <b>Scrolling style</b>. The user scrolls through the contents by using a dragging
         movement. This property controls what happens when the user releases the pressure.
         The default is that a follow-on animation continues the scrolling if the dragging
         was fast. Alternatively, the scrolling can stop as soon as the user releases the
         pressure, regardless of the dragging speed. Call setScrollingStyle() to set this
         property.

    - <b>Friction effect</b>. This property controls the speed of the follow-on scrolling
         animation. When the friction effect is enabled, it slows the animation and causes
         it to stop more quickly than when the effect is not enabled. Call setFrictionEnabled()
         to set this property.

    - <b>Clamping style</b>. This property controls how scrolling is constrained relative
         to the contents of the scrolling area. Scrolling can be limited to the bounding
         rectangle of the contents or it can go beyond the bounding rectangle and bounce
         back to its limits (this is the default). Call setClampingStyle() to set this
         property.

    - <b>Scroll direction</b>. This property controls the scrolling direction. The default is
         vertical scrolling, but this can be changed to horizontal or bi-directional scrolling.
         Call setScrollDirections() to set this property.

    - <b>Scrollbar policy</b>. There is a separate scrollbar policy property for both scrollbars.
         You can set these so that the scrollbar is always shown, never shown, only shown when
         needed or for short periods (autohide), which is the default behavior. Call
         setVerticalScrollBarPolicy() and setHorizontalScrollBarPolicy() to set these properties.

    - <b>Continuation indicators</b>. This property controls whether visual feedback is provided
         to indicate when scrolling is possible. The default value is false. Call
         setContinuationIndicators() to set this property.

    The contents widget must be an object of a class derived from QGraphicsWidget. After
    constructing this widget, you set it into the scroll area by calling setContentWidget().
    The scroll area uses QGraphicsWidget::sizeHint() to resize the content widget to fit,
    taking into account the scroll direction. For example, when the scrolling direction is
    vertical, the scroll area resizes the contents widget to fit the width of the scroll area.

    \section _usecases_hbscrollarea Using the HbScrollArea class

    This example creates an HbScrollArea widget and sets a large photo as its contents.

    \code
int main(int argc, char *argv[])
{
    HbApplication app(argc, argv);
    app.setApplicationName( "Scroll Area" );

    HbMainWindow window;
    HbView* view = new HbView();

    // Create the scroll area object.
    HbScrollArea* scrollArea = new HbScrollArea();

    // Create the content widget.
    QGraphicsWidget* content = new QGraphicsWidget();

    // Create a pixmap as a child of the content widget.
    QString string(":/gfx/beach.jpg");
    QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(string, content);

    // Set the preferred size of the content widget to match the full size of the photo.
    content->setPreferredSize(pixmapItem->boundingRect().size());

    // Load the content widget into the scrolling area.
    scrollArea->setContentWidget(content);

    // Set the scroll area to scroll in both directions.
    scrollArea->setScrollDirections(Qt::Vertical | Qt::Horizontal);

    view->setWidget(scrollArea);
    window.addView(view);
    window.show();
    return app.exec();
}
    \endcode

    \sa HbScrollBar
 */

/*!
    \fn void HbScrollArea::scrollDirectionsChanged(Qt::Orientations newValue)

    This signal is emitted when the scrolling direction changes.
*/

/*!
    \fn void HbScrollArea::scrollingStarted()

    This signal is emitted when a scrolling action begins.
*/

/*!
    \fn void HbScrollArea::scrollingEnded()

    This signal is emitted when a scrolling action ends.
*/

/*!
    \fn void HbScrollArea::scrollPositionChanged(const QPointF &newPosition)

    This signal is emitted when the scroll position is changed, but only if this signal
    is connected to a slot.
*/

/*!
    \enum HbScrollArea::ClampingStyle

    Defines the clamping styles supported by HbScrollArea. The clamping style
    controls the behavior when scrolling to the edge of the contents of the scrolling
    area.

    \sa setClampingStyle(), clampingStyle()
*/

/*!
    \var HbScrollArea::StrictClamping
    Scrolling is limited to the bounding rectangle of the contents.
*/

/*!
    \var HbScrollArea::BounceBackClamping
    Scrolling can go beyond the bounding rectangle of the contents, but bounces
    back to the limits of the bounding rectangle when released or when the follow-on
    scrolling animation stops. This is the default clamping style.
*/

/*!
    \var HbScrollArea::NoClamping
    Scrolling is unclamped. Typically you use this only in a subclass that implements
    its own custom clamping behavior.
*/

/*!
    \enum HbScrollArea::ScrollingStyle

    Defines the scrolling styles supported by HbScrollArea. The scrolling style
    controls which gestures the user can use to scroll the contents and how the scrolling
    responds to those gestures.

    \sa setScrollingStyle(), scrollingStyle()
*/

/*!
    \var HbScrollArea::Pan

    The user can scroll through the contents by using a dragging motion. The scrolling
    stops as soon as the user stops the dragging motion.
*/

/*!
    \var HbScrollArea::PanOrFlick

    \deprecated
    The user can scroll through the contents by using a dragging motion and a quick
    flicking motion triggers a follow-on scrolling animation.
*/

/*!
    \var HbScrollArea::PanWithFollowOn

    The user can scroll through the contents by using a dragging motion. In addition
    a fast dragging motion triggers a follow-on scrolling animation when the user
    stops dragging and releases the pressure. This is the default scrolling style.
*/

/*!
    \enum HbScrollArea::ScrollBarPolicy

    Defines the scroll bar visibility modes supported by  HbScrollArea.

    \sa setVerticalScrollBarPolicy(), verticalScrollBarPolicy(),
    setHorizontalScrollBarPolicy(), horizontalScrollBarPolicy()
*/

/*!
    \var HbScrollArea::ScrollBarAsNeeded

    Show the scroll bar only when the contents are too large to fit.
*/
/*!
    \var HbScrollArea::ScrollBarAlwaysOff

    Never show the scroll bar.
*/
/*!
    \var HbScrollArea::ScrollBarAlwaysOn

    Always show the scroll bar.
*/
/*!
    \var HbScrollArea::ScrollBarAutoHide

    Show the scroll bar for a short period when the contents are first displayed or
    when the user scrolls the contents. This is the default behavior.
*/

/*!
    \primitives
    \primitives{continuation-indicator-bottom} HbFrameItem representing the scrollarea continuation indicator on the bottom of the scrollarea.
    \primitives{continuation-indicator-top} HbFrameItem representing the scrollarea continuation indicator on the top of the scrollarea.
    \primitives{continuation-indicator-left} HbFrameItem representing the scrollarea continuation indicator on the left side of the scrollarea.
    \primitives{continuation-indicator-right} HbFrameItem representing the scrollarea continuation indicator on the right side of the scrollarea.
  */

/*!
    Constructs an HbScrollArea object with the given \a parent.
 */
HbScrollArea::HbScrollArea(QGraphicsItem* parent) :
        HbWidget( *new HbScrollAreaPrivate, parent )
{
    Q_D( HbScrollArea );
    d->q_ptr = this;
    d->init();
}

/*!
    Protected constructor.
  */
HbScrollArea::HbScrollArea(HbScrollAreaPrivate &dd, QGraphicsItem *parent):
        HbWidget( dd, parent  )
{
    Q_D( HbScrollArea );
    d->q_ptr = this;
    d->init();
}
   
/*!
    Destructor
 */
HbScrollArea::~HbScrollArea()
{
    Q_D( HbScrollArea );
    if (d && d->mContents) {
        d->mContents->setParentLayoutItem(0);
    }
}

/*!
    Returns a pointer to the widget that is contained within the scroll area
    and that defines the contents to be scrolled.

    \sa setContentWidget()
 */
QGraphicsWidget* HbScrollArea::contentWidget() const
{
    Q_D( const HbScrollArea );

    return d->mContents;
}

/*!
   Assigns a widget (QGraphicsWidget object) to the scroll area. This widget defines
   the contents of the scroll area. The HbScrollArea object takes ownership of the
   contents widget.

   \sa contentWidget()
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
    Removes the scroll area's content widget from the scroll area, returns it, and
    transfers ownership of the content widget to the caller. This is useful when you
    need to switch the contents without deleting previous contents.

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
    Returns the value of the \c clampingStyle property.

    \sa setClampingStyle(), HbScrollArea::ClampingStyle
 */
HbScrollArea::ClampingStyle HbScrollArea::clampingStyle() const
{
    Q_D( const HbScrollArea );

    return d->mClampingStyle;
}

/*!
    Sets the \c clampingStyle property, which controls how scrolling is constrained
    relative to the contents of the scrolling area. The possible values are defined
    by the HbScrollArea::ClampingStyle enum. The default is
    HbScrollArea::BounceBackClamping.

    \sa clampingStyle()
 */
void HbScrollArea::setClampingStyle(ClampingStyle value)
{
    Q_D( HbScrollArea );

    d->mClampingStyle = value;
}

/*!
    Returns the value of the \c scrollingStyle property.

    \sa setScrollingStyle()
 */
HbScrollArea::ScrollingStyle HbScrollArea::scrollingStyle() const
{
    Q_D( const HbScrollArea );

    return d->mScrollingStyle;
}

/*!
    Sets the \c scrollingStyle property, which controls which gestures the user can
    use to scroll the contents and how the scrolling responds to those gestures. The
    possible values are defined by the HbScrollArea::ScrollingStyle enum. The default
    value is HbScrollArea::PanWithFollowOn.

    \sa scrollingStyle()
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
    Returns the value of the \c scrollDirections property.
 
    \sa setScrollDirections()
 */
Qt::Orientations HbScrollArea::scrollDirections() const
{
    Q_D( const HbScrollArea );

    return d->mScrollDirections;
}

/*!
    Sets the \c scrollDirections property, which controls the scrolling direction. The
    possible values are defined by the Qt::Orientations enum, as shown in the following
    table. The default value is Qt::Vertical.

    <table border="1" style="border-collapse: collapse; border: solid;">
    <tr><th>To enable:</th><th>Set to:</th></tr>
    <tr><td>Horizontal scrolling</td><td>Qt::Horizontal</td></tr>
    <tr><td>Vertical scrolling</td><td>Qt::Vertical</td></tr>
    <tr><td>Scrolling in both directions</td><td>Qt::Horizontal | Qt::Vertical</td></tr>
    </table>

    \sa scrollDirections()
 */
void HbScrollArea::setScrollDirections(Qt::Orientations value)
{
    Q_D( HbScrollArea );

    bool isChanged = (d->mScrollDirections != value);

    d->mScrollDirections = value;        
    if (d->mContents && isChanged) {
        QPointF pos = d->mContents->pos();
        QEvent layoutRequest(QEvent::LayoutRequest);
        QCoreApplication::sendEvent(this, &layoutRequest);
        d->mContents->setPos(pos);
    }

    if (isChanged) {
        emit scrollDirectionsChanged( value );
    }
}

/*!
    Returns true if the friction effect is enabled, false otherwise.
 
    \sa setFrictionEnabled()
 */
bool HbScrollArea::frictionEnabled() const
{
    Q_D( const HbScrollArea );

    return d->mFrictionEnabled;
}

/*!
    Sets the \c frictionEnabled property. The default value is true, which
    indicates that the friction effect is enabled.

    When the HbScrollArea::PanWithFollowOn scrolling style is in use, the
    \c frictionEnabled property controls the speed of the follow-on scrolling
    animation. When the friction effect is enabled (the default), it slows the
    animation and causes it to stop earlier than when the friction effect is not
    in use.

    \sa frictionEnabled()
 */
void HbScrollArea::setFrictionEnabled(bool value)
{
    Q_D( HbScrollArea );

    d->mFrictionEnabled = value;
}

/*!
    Returns true if the scroll area handles long press gestures, false otherwise.

    \deprecated This function is deprecated.
 */
bool HbScrollArea::longPressEnabled() const
{
    HB_DEPRECATED("HbScrollArea::longPressEnabled() is deprecated");    
    return false;
}

/*!
    Sets the value of the \c handleLongPress property. Set \a value to true if
    the widget responds to long press gestures. Otherwise set it to false.
    The default value is false.

    \deprecated This function is deprecated.
 */
void HbScrollArea::setLongPressEnabled (bool value)
{
    HB_DEPRECATED("HbScrollArea::setLongPressEnabled(bool) is deprecated");
    Q_UNUSED(value);
}

/*!
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
        //need to invalidate the whole region as we apply clipping
        prepareGeometryChange();
    }

    return HbWidget::itemChange(change, value);
}

/*!
    A virtual slot function that is called when an upwards flick gesture is detected
    while vertical scrolling is enabled.

    \deprecated This function is deprecated.
 */
void HbScrollArea::upGesture(int speedPixelsPerSecond)
{
    HB_DEPRECATED("HbScrollArea::upGesture(int) is deprecated. Use gesture FW.");

    Q_UNUSED(speedPixelsPerSecond);
}

/*!
    A virtual slot function that is called when a downwards flick gesture is
    detected while vertical scrolling is enabled.

    \deprecated This function is deprecated.
 */
void HbScrollArea::downGesture(int speedPixelsPerSecond)
{
    HB_DEPRECATED("HbScrollArea::downGesture(int) is deprecated. Use gesture FW.");
    Q_UNUSED(speedPixelsPerSecond);
}

/*!
    A virtual slot function that is called when a left flick gesture
    is detected while horizontal scrolling is enabled.

    \deprecated This function is deprecated.
 */
void HbScrollArea::leftGesture(int speedPixelsPerSecond)
{
    HB_DEPRECATED("HbScrollArea::leftGesture(int) is deprecated. Use gesture FW.");
    Q_UNUSED(speedPixelsPerSecond);
}

/*!
    A virtual slot function that is called when a right flick gesture is detected
    while horizontal scrolling is enabled.

    \deprecated This function is deprecated.
 */
void HbScrollArea::rightGesture(int speedPixelsPerSecond)
{
    HB_DEPRECATED("HbScrollArea::rightGesture(int) is deprecated. Use gesture FW.");
    Q_UNUSED(speedPixelsPerSecond);
}


/*!
    A virtual slot function that is called when a pan gesture is detected.

    \deprecated This function is deprecated.
 */
void HbScrollArea::panGesture(const QPointF &delta)
{
    HB_DEPRECATED("HbScrollArea::panGesture(const QPointF &) is deprecated. Use gesture FW.");
    Q_UNUSED(delta);
}

/*!
    A virtual slot function that is called when a long press gesture is
    detected while the \c handleLongPress property is set to true.

    \deprecated This function is deprecated.
 */
void HbScrollArea::longPressGesture(const QPointF &)
{
    HB_DEPRECATED("HbScrollArea::longPressGesture(const QPointF &) is deprecated. Use gesture FW.");
}

/*!
    Reimplemented from QObject.
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
    Returns true if the scrolling is in a response to the user dragging, false if it
    is the follow-on scrolling animation.
 */
bool HbScrollArea::isDragging() const
{
    Q_D( const HbScrollArea );

    return (d->mIsScrolling && !d->mIsAnimating);
}

/*!
    Scrolls the view by the amount indicated by \a delta and returns true if
    the scroll was successful and false otherwise.

    This is a virtual function, which subclasses can override to customize the
    behavior; for example, to clamp the position so that at least one item in
    the view remains visible.
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
            if(HbTapGesture *tap = qobject_cast<HbTapGesture*>(static_cast<QGestureEvent *>(event)->gesture(Qt::TapGesture))) {
                if (d->mIsAnimating && !d->positionOutOfBounds() && !d->mMultiFlickEnabled) {
                    event->accept();
                    return true;
                } else if (tap->state() == Qt::GestureStarted){
                    if (d->mAbleToScrollY) {
                        tap->setProperty(HbPrivate::VerticallyRestricted.latin1(), true);
                    }
                    if (d->mAbleToScrollX){
                        tap->setProperty(HbPrivate::HorizontallyRestricted.latin1(), true);
                    }
                }
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

                if (!(d->mScrollDirections & Qt::Vertical) ||
                    ((d->mContents->sizePolicy().verticalPolicy() & QSizePolicy::ExpandFlag) &&
                    (newSize.height() < size().height()))) {
                    newSize.setHeight(size().height());
                    sizeChanged = true;
                }

                if (!(d->mScrollDirections & Qt::Horizontal) ||
                    ((d->mContents->sizePolicy().horizontalPolicy() & QSizePolicy::ExpandFlag) &&
                     (newSize.width() < size().width()))) {
                    newSize.setWidth(size().width());
                    sizeChanged = true;
                }
                if (sizeChanged) {
                    d->mContents->resize(newSize);
                }
                d->adjustContent();
            }
        }
    }
  return value;
}

/*!
    Reimplemented from QObject.
 */
bool HbScrollArea::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    Q_D(HbScrollArea);
    if (event && event->type() == QEvent::GraphicsSceneResize) {
        //If layoutdirection is from right to left, we expand the contentwidget to the left
        //instead of the right
        if (layoutDirection() == Qt::RightToLeft) {
            QGraphicsSceneResizeEvent *resizeEvent = static_cast<QGraphicsSceneResizeEvent*>(event);
            if (resizeEvent) {
                qreal xChange = resizeEvent->newSize().width() - resizeEvent->oldSize().width();
                qreal newXPos = d->mContents->pos().x() - xChange;
                d->mContents->setPos(newXPos, d->mContents->pos().y());
            }
        }
        if (isVisible()) {
            d->adjustContent();
        }

        if (d->mAbleToScrollX && d->mHorizontalScrollBar && d->mHorizontalScrollBar->isVisible()) {
            d->updateScrollBar(Qt::Horizontal);
        }

        if (d->mAbleToScrollY && d->mVerticalScrollBar && d->mVerticalScrollBar->isVisible()) {
            d->updateScrollBar(Qt::Vertical);
        }
    }  // no else

    return false;
}

/*!
    Reimplemented from QGraphicsWidget.
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
                sh = d->mContents->effectiveSizeHint( which );
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
    Returns the display policy for the vertical scrollbar.

    \sa horizontalScrollBarPolicy(), setVerticalScrollBarPolicy()
 */
HbScrollArea::ScrollBarPolicy HbScrollArea::verticalScrollBarPolicy() const
{
    Q_D(const HbScrollArea);
    return d->mVerticalScrollBarPolicy;
}

/*!
    Sets the display policy for the vertical scrollbar. The possible values are
    defined by the HbScrollArea::ScrollBarPolicy enum. The default value is
    HbScrollArea::ScrollBarAutoHide.

    \sa setHorizontalScrollBarPolicy(), verticalScrollBarPolicy(), setVerticalScrollBar()
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
    if (!d->mVerticalScrollBar) {
        const_cast<HbScrollAreaPrivate *>(d)->createScrollBars(Qt::Vertical);
    }
    return d->mVerticalScrollBar;
}

/*!
    Replaces the existing vertical scroll bar with \a scrollBar. The former
    scroll bar is deleted.

    %HbScrollArea provides provides vertical and horizontal scroll bars by default.
    Call this function to replace the default vertical scroll bar with your own
    custom scroll bar, if required.

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
    Returns the display policy for the horizontal scrollbar.

    \sa verticalScrollBarPolicy(), setHorizontalScrollBarPolicy()
 */
HbScrollArea::ScrollBarPolicy HbScrollArea::horizontalScrollBarPolicy() const
{
    Q_D(const HbScrollArea);
    return d->mHorizontalScrollBarPolicy;
}

/*!
    Sets the display policy for the horizontal scrollbar. The possible values are
    defined by the HbScrollArea::ScrollBarPolicy enum. The default value is
    HbScrollArea::ScrollBarAutoHide.

    \sa setVerticalScrollBarPolicy(), horizontalScrollBarPolicy(), setHorizontalScrollBar()
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
    if (!d->mHorizontalScrollBar) {
        const_cast<HbScrollAreaPrivate *>(d)->createScrollBars(Qt::Horizontal);
    }
    return d->mHorizontalScrollBar;
}

/*!
   Replaces the existing horizontal scroll bar with \a scrollBar. The former
   scroll bar is deleted.

   %HbScrollArea provides vertical and horizontal scroll bars by default.
   Call this function to replace the default horizontal scroll bar with your
   own custom scroll bar, if required.

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
    Sets the alignment of the scroll area's content widget. The possible values are
    defined by the Qt::Alignment enum.

    The default value is \c Qt::AlignLeft | \c Qt::AlignTop, which roots the content
    widget in the top-left corner of the scroll area.

    \sa alignment()
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
    Returns the alignment of the scroll area's content widget.

    \sa setAlignment()
 */
Qt::Alignment HbScrollArea::alignment() const
{
    Q_D(const HbScrollArea);
    return d->mAlignment;
}


/*!
    Sets the \c contuationIndicators property for the scroll area. Set
    \a indication to true if you want the scroll area to provide visual
    feedback when scrolling is possible. The default value is false.

    \sa continuationIndicators()
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
    Returns the value of the \c continuationIndicators property for the
    scroll area.

    \sa setContinuationIndicators()
*/
bool HbScrollArea::continuationIndicators() const
{
    Q_D(const HbScrollArea);
    return d->mContinuationIndicators;
}



/*!
    Scrolls the contents of the scroll area so that \a position is visible within
    the scroll area with the given margins. If the specified point cannot be shown,
    the contents are scrolled to the nearest valid position.

    \param position Defines the position within the content widget that is to be shown within
               the scroll area.
    \param xMargin The width of the vertical margins in pixels. This can be between 0 and the
               width of the scroll area. The default value is 0.

    \param yMargin The height of the horizontal margins in pixels. This can be between 0
               and the height of the scroll area. The default value is 0.

    \sa setScrollDirections()
 */
void HbScrollArea::ensureVisible(const QPointF& position, qreal xMargin, qreal yMargin)
{
    Q_D(HbScrollArea);
    d->ensureVisible(position, xMargin, yMargin);
}

/*!
    Scrolls the contents of the scroll area to \a newPosition in the given \a time.
    If \a time is 0, the contents are scrolled to the position immediately.
 */
void HbScrollArea::scrollContentsTo (const QPointF& newPosition, int time) {
    Q_D(HbScrollArea);

    if (!contentWidget())
        return;

    d->stopAnimating();
    d->stopScrolling();

    if (time > 0){
        d->startTargetAnimation (newPosition, qMax (0, time));
    } else {
        scrollByAmount(newPosition - (-d->mContents->pos()));
        d->stopScrolling();
    }
}

/*!
    \reimp
 */
void HbScrollArea::polish(HbStyleParameters& params)
{
    if (isVisible()) {
        Q_D(HbScrollArea);
        d->doLazyInit();

        // fetch scrolling parameters from css
        const QLatin1String SpeedFactor("speed-factor");
        const QLatin1String IntertiaSpeedFactor("inertia-speed-factor");
        const QLatin1String MaxScrollSpeed("max-scroll-speed");
        const QLatin1String SpringStrength("spring-strength");
        const QLatin1String SpringDampingFactor("spring-damping-factor");
        const QLatin1String FrictionPerMilliSecond("friction");

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
    } else {
        HbWidget::polish(params);
    }
}

/*!
    Reimplemented from QObject.
 */
void HbScrollArea::timerEvent(QTimerEvent *event)
{
    Q_D(HbScrollArea);
    if (event->timerId() == d->mScrollTimerId) {
        d->_q_animateScrollTimeout();
    } else if (event->timerId() == d->mScrollBarHideTimerId) {
        d->_q_hideScrollBars();
    }
}

/*!
    Reimplemented from QGraphicsWidget.
 */
QPainterPath HbScrollArea::shape() const
{
    Q_D(const HbScrollArea);
    if ( d->mClearCachedRect){
        d->reCalculateCachedValue();
    }
    return d->mShape;
}

/*!
    Reimplemented from QGraphicsWidget.
 */
QRectF HbScrollArea::boundingRect() const

{
    Q_D(const HbScrollArea);
    if (d->mClearCachedRect) {
        d->reCalculateCachedValue();
    }
    return d->mBoundingRect;
}

/*!
    Reimplemented from QGraphicsWidget.
 */
void HbScrollArea::setGeometry(const QRectF& rect)
{
    Q_D(HbScrollArea);
    d->mClearCachedRect = true;
    HbWidget::setGeometry(rect);
}

/*!
    Reimplemented from QObject.
 */
void HbScrollArea::disconnectNotify (const char *signal)
{
    Q_D(HbScrollArea);
    if (d->mEmitPositionChangedSignal &&
        QLatin1String(signal) == SIGNAL(scrollPositionChanged(QPointF))) {
        if (receivers(SIGNAL(scrollPositionChanged(QPointF))) == 0) {
            d->mEmitPositionChangedSignal = false;
        }
    }
    HbWidget::disconnectNotify(signal);
}

/*!
    Reimplemented from QObject.
 */
void HbScrollArea::connectNotify(const char * signal)
{
    Q_D(HbScrollArea);
    if (!d->mEmitPositionChangedSignal &&
        QLatin1String(signal) == SIGNAL(scrollPositionChanged(QPointF))) {
        d->mEmitPositionChangedSignal = true;
    }
    HbWidget::connectNotify(signal);
}

/*!
  Reimplemented from HbWidgetBase.
  */
void HbScrollArea::initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive)
{
    HbWidgetBase::initPrimitiveData(primitiveData, primitive);
    QString itemName = HbStyle::itemName(primitive);

    HbStyleFramePrimitiveData *data = hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData);
    data->frameType = HbFrameDrawer::OnePiece;

    if (itemName == QLatin1String("continuation-indicator-top"))
    {
        data->frameGraphicsName = QLatin1String("qtg_graf_list_mask_t");
    }
    else if (itemName == QLatin1String("continuation-indicator-bottom"))
    {
        data->frameGraphicsName = QLatin1String("qtg_graf_list_mask_b");
    }
    else if (itemName == QLatin1String("continuation-indicator-left"))
    {
        data->frameGraphicsName = QLatin1String("qtg_graf_list_mask_l");
    }
    else if (itemName == QLatin1String("continuation-indicator-right"))
    {
        data->frameGraphicsName = QLatin1String("qtg_graf_list_mask_r");
    }
}

#include "moc_hbscrollarea.cpp"
