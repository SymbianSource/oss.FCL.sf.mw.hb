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

#include <hbscrollbar.h>
#include <hbscrollbar_p.h>
#include <hbwidget_p.h>
#include <hbstyleoptionscrollbar.h>
#include <hbwidgetfeedback.h>
#include <hbframeitem.h>

#include <QGraphicsSceneMouseEvent>
#include <QGesture>
#include <QDebug>

#ifdef HB_EFFECTS
#include "hbeffect.h"
#include "hbeffectinternal_p.h"
bool HbScrollBarPrivate::effectsLoaded = false;
#endif

static const int REPEATION_TIME = 500;
static const qreal THRESHOLD_VALUE = 2.0;
/*!
    @stable
    @hbcore
    \class HbScrollBar
    \brief HbScrollBar represents a scrollbar that can be used in different kinds of lists, options
    menus and editors.

    A vertical scrollbar is created in this example to listview-parent and also shown:

    \code
    //HbListView *listView is defined already
    HbScrollBar *scrollbar = new HbScrollBar(Qt::Vertical, listView);
    scrollbar->show();
    \endcode

    By default scrollbar is hidden.
*/

/*!
    \reimp
    \fn int HbScrollBar::type() const
 */

/*!
    \fn void HbScrollBar::valueChanged( qreal value, Qt::Orientation orientation )

    This signal is emitted when thumb position is changed from user interaction.
*/

HbScrollBarPrivate::HbScrollBarPrivate():
        mOrientation(Qt::Vertical),
        mCurrentPosition(0.5),
        mPageSize(1.0),
        mActivated(false),
        mThumbPressed(false),
        mGroovePressed(false),
        mInteractive(false),
        mPressedTargetValue(0.0),
        grooveItem(0),
        handleItem(0),
        mTouchArea(0),
        mBoundingRect(QRectF(0,0,0,0)),
        lastEmittedPos(QPointF()),
        emittedPos(false)
{
}

HbScrollBarPrivate::~HbScrollBarPrivate()
{
}

void HbScrollBarPrivate::init()
{
    Q_Q(HbScrollBar);
    q->grabGesture(Qt::PanGesture);
    q->grabGesture(Qt::TapGesture);
}

void HbScrollBarPrivate::createPrimitives()
{
    Q_Q(HbScrollBar);

    if ( !grooveItem ) {
        grooveItem = q->style()->createPrimitive( HbStyle::P_ScrollBar_groove, q );
        grooveItem->setZValue(2);
        HbStyle::setItemName( grooveItem, "groove" );
    }

    if ( !handleItem ) {
        handleItem = q->style()->createPrimitive( HbStyle::P_ScrollBar_handle, q );
        handleItem->setZValue(3);
        HbStyle::setItemName( handleItem, "handle" );
    }
    if( !mTouchArea ) {
        mTouchArea = q->style()->createPrimitive(HbStyle::P_ScrollBar_toucharea, q);
    }
    q->updatePrimitives();
}

void HbScrollBarPrivate::updatePosition()
{
    if(!mHandleGeometry.isValid() || !handleItem)
        return;
    if (handleItem){
        if (mOrientation == Qt::Vertical) {
            handleItem->setPos(mHandleGeometry.topLeft().x(), mCurrentPosition * (mBoundingRect.height() - mHandleGeometry.height()));
        } else {
            handleItem->setPos(mCurrentPosition * (mBoundingRect.width() - mHandleGeometry.width()), mHandleGeometry.topLeft().y());
        }
    }
}

void HbScrollBarPrivate::sizeHelper()
{
    if(!polished)
        return;
    Q_Q(HbScrollBar);
    if(handleItem){
        HbFrameItem *item = qgraphicsitem_cast<HbFrameItem*>(handleItem);
        mBoundingRect = q->boundingRect();
        if(item){
            if (mOrientation == Qt::Vertical) {
                qreal height(mPageSize * mBoundingRect.height());
                if(!qFuzzyCompare(item->preferredHeight(),height)){
                    item->setPreferredHeight(height);
                    item->resize(item->size().width(), height);
                }
            } else {
                qreal width(mPageSize * mBoundingRect.width());
                if(!qFuzzyCompare(item->preferredWidth(),width)){
                    item->setPreferredWidth(width);
                    item->resize(width, item->size().height());
                }
            }
            mHandleGeometry = item->geometry();
            updatePosition();
        }
    }
}

void HbScrollBarPrivate::loadEffects()
{
#if defined(HB_EFFECTS)
    if (effectsLoaded)
        return;
    hasEffects = HbEffectInternal::add( "HB_scrollbar",
                                        "scrollbar_activate",
                                        "activate" );
    if ( hasEffects ) {
        hasEffects = HbEffectInternal::add( "HB_scrollbar",
                                            "scrollbar_deactivate",
                                            "deactivate" );

    }
    effectsLoaded = true;
#endif
}

void HbScrollBarPrivate::startShowEffect() 
{
#if defined(HB_EFFECTS)
    if (!hasEffects)
        loadEffects();
    Q_Q(HbScrollBar);
    if (hasEffects) {
        HbEffect::start(q, "HB_scrollbar", "activate");
    }
#endif
}

void HbScrollBarPrivate::startHideEffect() 
{
#if defined(HB_EFFECTS)
    Q_Q(HbScrollBar);
    if (hasEffects) {
        HbEffect::start(q, "HB_scrollbar", "deactivate");
        //        q->resetTransform();

    }
#endif
}

/*!
    Constructs a scrollbar with \a parent.
*/
HbScrollBar::HbScrollBar( QGraphicsItem *parent ) :
        HbWidget(*new HbScrollBarPrivate, parent)
{
    Q_D(HbScrollBar);
    d->q_ptr = this;
    d->init();
}

/*!
    Constructs a scrollbar with \a orientation and \a parent.
*/
HbScrollBar::HbScrollBar( Qt::Orientation orientation, QGraphicsItem *parent ) :
        HbWidget( *new HbScrollBarPrivate, parent )
{
    Q_D(HbScrollBar);
    d->q_ptr = this;
    d->mOrientation = orientation;
    d->init();
}

/*!
    Destructor
 */
HbScrollBar::~HbScrollBar()
{
}

/*!
    Returns the value of the scrollbar. The value is in range of 0.0 to 1.0.
    The value corresponds to the position of the thumb.

    \sa HbScrollBar::setValue()
*/
qreal HbScrollBar::value() const
{
    Q_D( const HbScrollBar );
    return d->mCurrentPosition;
}

/*!
   Returns relative size of the visible area the scrollbar is attached to.
   For example if the pageSize is 0.2 it means that the overall size of the content
   is five times larger than what is shown currently.

   The size is in range of 0.0 to 1.0.
   \sa HbScrollBar::setPageSize()
*/
qreal HbScrollBar::pageSize() const
{
    Q_D( const HbScrollBar );
    return d->mPageSize;
}

/*!
    Returns the orientation of scrollbar.

    \sa HbScrollBar::setOrientation()
*/
Qt::Orientation HbScrollBar::orientation() const
{
    Q_D( const HbScrollBar );
    return d->mOrientation;
}

/*!
    Returns whether scrollbar is in interactive mode.

    \sa HbScrollBar::setInteractive()
*/
bool HbScrollBar::isInteractive() const
{
    const Q_D(HbScrollBar);
    return d->mInteractive;
}

/*!
    Returns whether scrollbar is in interactive mode.

    \deprecated HbScrollBar::interactive() const
        is deprecated. Please use HbScrollBar::isInteractive() const instead.

    \sa HbScrollBar::isInteractive()
*/
bool HbScrollBar::interactive() const
{
    qWarning() << "HbScrollBar::interactive() is DEPRECATED! Use HbScrollBar::isInteractive";
    return isInteractive();
}

/*!
    Sets the value of interactive property. If this value is set
    to true scrollbar is interactive, the user can change the scroll position by
    dragging the thumb or pressing the groove. Dragging the thumb emits valueChanged
    signal and pressing the groove emits valueChangeRequested which means
    that the widget using scrollbars should handle the value change (for example
    by animating to the given target value).

    The default behavior is non interactive,
    which means that the scrollbar is just indicative.

    \sa HbScrollBar::isInteractive()
*/
void HbScrollBar::setInteractive( bool enabled )
{
    Q_D( HbScrollBar );
    if( d->mInteractive != enabled){
        d->mInteractive = enabled;
        if(d->handleItem) {
            repolish();
            updatePrimitives();
        }
    }
}

/*!
    Sets the value of the scrollbar. The given \avalue should be from 0.0 to 1.0.
    The value is used to position the thumb.

    \sa HbScrollBar::value()
*/
void HbScrollBar::setValue( qreal value )
{
    Q_D(HbScrollBar);        

    value = qBound(static_cast<qreal>(0.0), value, static_cast<qreal>(1.0));
    if( !qFuzzyCompare(d->mCurrentPosition,value )) {
        d->mCurrentPosition = value;
        d->updatePosition();
    }
}

/*!
    Sets the page size for the scrollbar. The page size is relative size of the visible area
    the scrollbar is attached to. For example if the pageSize is 0.2 it means that the
    overall size of the content is five times larger than what is shown currently.

    \asize should be in the range of 0.0 to 1.0.
    \sa HbScrollBar::pageSize()
*/
void HbScrollBar::setPageSize( qreal size )
{
    Q_D(HbScrollBar);
    size = qBound(static_cast<qreal>(0.0), size, static_cast<qreal>(1.0));

    if(!qFuzzyCompare(d->mPageSize,size)) {
        d->mPageSize = size;
        d->sizeHelper();
    }
}

/*!
    Sets the orientation of scrollbar.

    \sa HbScrollBar::orientation()
*/
void HbScrollBar::setOrientation( Qt::Orientation orientation )
{
    Q_D(HbScrollBar);
    if(d->mOrientation != orientation) {
        d->mOrientation = orientation;
        repolish();
    }
}

/*!
    \reimp
 */
void HbScrollBar::updatePrimitives()
{
    Q_D(HbScrollBar);

    HbStyleOptionScrollBar option;
    initStyleOption(&option);
    if (d->grooveItem) {
        style()->updatePrimitive(d->grooveItem, HbStyle::P_ScrollBar_groove, &option);
    }
    if (d->handleItem) {
        style()->updatePrimitive(d->handleItem, HbStyle::P_ScrollBar_handle, &option);
    }
    if( d->mTouchArea ) {
        style()->updatePrimitive(d->mTouchArea, HbStyle::P_ScrollBar_toucharea, &option);
    }
}

/*!
    \reimp
 */
void HbScrollBar::initStyleOption( HbStyleOptionScrollBar *option ) const
{
    Q_D(const HbScrollBar);
    HbWidget::initStyleOption(option);

    Q_ASSERT(option);
    option->orientation = d->mOrientation;
    option->thumbPressed = d->mThumbPressed;
    option->groovePressed = d->mGroovePressed;
    option->interactive = d->mInteractive;
}

/*!
    \reimp
 */
void HbScrollBar::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    Q_D(HbScrollBar);
    QGraphicsWidget::mousePressEvent(event);

    if ( !d->mInteractive ) {
        return;
    }

    HbStyleOptionScrollBar opt;

    QRectF handleBounds = d->handleItem->boundingRect();

    d->mPressPosition = mapToItem(d->handleItem, event->pos());

    switch (orientation()) {
    case Qt::Horizontal:
        d->mThumbPressed = (event->pos().x() >= d->handleItem->pos().x() &&
                            event->pos().x() <= d->handleItem->pos().x() + handleBounds.width());
        if (!d->mThumbPressed) {
            d->mGroovePressed = true;
            HbWidgetFeedback::triggered(this, Hb::InstantPressed);

            if (d->handleItem->pos().x() < event->pos().x()) {
                emit valueChangeRequested(qMin(value() + pageSize(), qreal(1.0)), orientation());
            } else {
                emit valueChangeRequested(qMax(value() - pageSize(), qreal(0.0)), orientation());
            }
            d->mPressedTargetValue = qBound(qreal(0.0),
                                            qreal((event->pos().x() - (handleBounds.width() / 2.0)) / (boundingRect().width() - handleBounds.width())),
                                            qreal(1.0));

            d->repeatActionTimer.start(REPEATION_TIME, this);
        } else {
            HbWidgetFeedback::triggered(this, Hb::InstantPressed, Hb::ModifierSliderHandle);
            initStyleOption(&opt);
            style()->updatePrimitive(d->handleItem, HbStyle::P_ScrollBar_handle, &opt );
            emit d->core.handlePressed();
        }

        break;
    case Qt::Vertical:
        d->mThumbPressed = (event->pos().y() >= d->handleItem->pos().y() &&
                            event->pos().y() <= d->handleItem->pos().y() + handleBounds.height());
        if (!d->mThumbPressed) {
            d->mGroovePressed = true;
            HbWidgetFeedback::triggered(this, Hb::InstantPressed);

            if (d->handleItem->pos().y() < event->pos().y()) {
                emit valueChangeRequested(qMin(value() + pageSize(), qreal(1.0)), orientation());
            } else {
                emit valueChangeRequested(qMax(value() - pageSize(), qreal(0.0)), orientation());
            }

            d->mPressedTargetValue = qBound(qreal(0.0),
                                            qreal((event->pos().y() - (handleBounds.height() / 2.0)) / (boundingRect().height() - handleBounds.height())),
                                            qreal(1.0));

            d->repeatActionTimer.start(REPEATION_TIME, this);
        } else {
            HbWidgetFeedback::triggered(this, Hb::InstantPressed, Hb::ModifierSliderHandle);
            initStyleOption(&opt);
            style()->updatePrimitive(d->handleItem, HbStyle::P_ScrollBar_handle, &opt );
            emit d->core.handlePressed();
        }

        break;
    }
    event->accept();
}

/*!
    \reimp
 */
void HbScrollBar::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    Q_D(HbScrollBar);
    QGraphicsWidget::mouseReleaseEvent(event);

    if ( !d->mInteractive ) {
        return;
    }
    HbStyleOptionScrollBar opt;
    if (d->mThumbPressed) {
        d->mThumbPressed = false;
        HbWidgetFeedback::triggered(this, Hb::InstantReleased, Hb::ModifierSliderHandle);
        emit valueChanged(value(), orientation());
        emit d->core.handleReleased();
        initStyleOption(&opt);
        style()->updatePrimitive(d->handleItem, HbStyle::P_ScrollBar_handle, &opt );
    } else if (d->mGroovePressed){
        HbWidgetFeedback::triggered(this, Hb::InstantReleased);
        d->repeatActionTimer.stop();
        d->mGroovePressed = false;
        initStyleOption(&opt);
        style()->updatePrimitive(d->grooveItem, HbStyle::P_ScrollBar_groove, &opt );
    }
    d->emittedPos = false;
    event->accept();            
}

/*!
    \reimp
 */
void HbScrollBar::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
    Q_D(HbScrollBar);
    QGraphicsWidget::mouseMoveEvent(event);

    if ( !d->mInteractive ) {
        return;
    }

    if (d->mThumbPressed) {
        qreal newPosition(0);
        QPointF movePosition(0, 0);
        switch (orientation()) {
        case Qt::Horizontal:
            newPosition = (event->pos().x() - d->mPressPosition.x()) /
                          (boundingRect().width() - d->handleItem->boundingRect().width());
            movePosition.setX(event->pos().x());
            break;
        case Qt::Vertical:
            newPosition = (event->pos().y() - d->mPressPosition.y()) / 
                          (boundingRect().height() - d->handleItem->boundingRect().height());
            movePosition.setY(event->pos().y());
            break;
        }
        HbWidgetFeedback::continuousTriggered(this, Hb::ContinuousDragged);
        qreal newValue = qBound(qreal(0.0), newPosition,  qreal(1.0));
        if (!d->emittedPos ||
            (qAbs(d->lastEmittedPos.x() - movePosition.x()) >= qreal(THRESHOLD_VALUE)) ||
            (qAbs(d->lastEmittedPos.y() - movePosition.y()) >= qreal(THRESHOLD_VALUE))) {
            setValue(newValue);
            d->lastEmittedPos = movePosition;
            d->emittedPos = true;
            emit valueChanged(newValue, orientation());
        }        
    }
}

/*!
    \reimp
 */
QRectF HbScrollBar::boundingRect() const
{
    Q_D(const HbScrollBar);
    QRectF newBoundingRect = HbWidget::boundingRect();    
    /* Workaround for touch area and event filter issue */
    if (d->mInteractive) {
        if (d->mOrientation == Qt::Vertical && d->mTouchArea) {
            QRectF toucharea = d->mTouchArea->boundingRect();
            qreal newWidth = (toucharea.width() - newBoundingRect.width());
            newBoundingRect.setRight(toucharea.width());
            newBoundingRect.setLeft(-newWidth);
        } else if (d->mTouchArea) {
            QRectF toucharea = d->mTouchArea->boundingRect();
            qreal newHeight = (toucharea.height() - newBoundingRect.height());
            newBoundingRect.setBottom(toucharea.height());
            newBoundingRect.setTop(-newHeight);
        }
    }
    /* Workaround ends */
    return newBoundingRect;
}

/*!
    \reimp
 */
void HbScrollBar::timerEvent( QTimerEvent *event )
{
    Q_D(HbScrollBar);
    if (event->timerId() == d->repeatActionTimer.timerId()) {
        if (value() > d->mPressedTargetValue) {
            HbWidgetFeedback::triggered(this, Hb::InstantKeyRepeated, Hb::ModifierSliderHandle);
            emit valueChangeRequested(value() - qMin(pageSize(), value() - d->mPressedTargetValue), orientation());
        } else if (value() < d->mPressedTargetValue){
            HbWidgetFeedback::triggered(this, Hb::InstantKeyRepeated, Hb::ModifierSliderHandle);
            emit valueChangeRequested(value() + qMin(pageSize(), d->mPressedTargetValue - value()), orientation());
        }
        if (value() != d->mPressedTargetValue) {
            d->repeatActionTimer.start(REPEATION_TIME, this);
        } else {
            d->repeatActionTimer.stop();
        }
    }
}

/*!
    \reimp
*/
bool HbScrollBar::event(QEvent *event)
{
    if (event && event->type() == QEvent::GraphicsSceneResize) {
        Q_D(HbScrollBar);
        d->sizeHelper();
    }
    return HbWidget::event(event);
}

void HbScrollBar::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    HbWidget::resizeEvent(event);
    Q_D(HbScrollBar);
    if (d->handleItem) {
        HbFrameItem* item = (qgraphicsitem_cast<HbFrameItem*>(d->handleItem));
        d->mHandleGeometry = item->geometry();
    }
}

/*!
    \reimp
*/
void HbScrollBar::updateGeometry()
{
    Q_D(HbScrollBar);
    d->sizeHelper();
    HbWidget::updateGeometry();
}

/*!
    \reimp
 */
QVariant HbScrollBar::itemChange ( GraphicsItemChange change, const QVariant & value )
{
    if (change == QGraphicsItem::ItemVisibleChange) {
        Q_D(HbScrollBar);
        if (value.toBool()) {
            if (!d->mActivated) {
                d->startShowEffect();
                d->mActivated = true;
            }
        } else {
            if (d->mActivated) {
                d->startHideEffect();
                d->mActivated = false;
            }
        }
    }
    return HbWidget::itemChange(change, value);
}

/*!
    \reimp
 */
void HbScrollBar::polish( HbStyleParameters& params )
{
    Q_D(HbScrollBar);
    if (!d->handleItem && isVisible()){
        d->createPrimitives();
    }
    HbWidget::polish(params);
    d->sizeHelper();
}

/*!
  \reimp
*/
void HbScrollBar::gestureEvent(QGestureEvent* event) 
{
    Q_D(HbScrollBar);
    if ( !d->mInteractive ) {
        return;
    }
    // as gestures don't provide low enough details, just eat them to
    // prevent propagation to other components.
    event->accept(Qt::TapGesture);
    event->accept(Qt::PanGesture);
    event->accept();
}
#include "moc_hbscrollbar.cpp"
