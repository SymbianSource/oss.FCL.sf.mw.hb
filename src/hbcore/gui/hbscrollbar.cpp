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
#include <hbwidgetfeedback.h>
#include <hbframeitem.h>
#include <hbstyleframeprimitivedata.h>
#include <hbstyletouchareaprimitivedata.h>

#include <QGraphicsSceneMouseEvent>
#include <QGesture>
#include <QDebug>

#ifdef HB_EFFECTS
#include "hbeffect.h"
#include "hbeffectinternal_p.h"
bool HbScrollBarPrivate::effectsLoaded = false;
#endif

static const int REPEATION_TIME = 500;
static const qreal THRESHOLD_VALUE = 4.0;
/*!
    @stable
    @hbcore
    \class HbScrollBar
    \brief The HbScrollBar class provides a vertical or horizontal scroll bar.

    Scroll bars indicate that more content is available than can fit within a container and
    that the contents can be scrolled. A scroll bar consists of a groove and a handle (which
    is sometimes called a thumb). The position of the handle on the groove indicates the
    position of the visible content within the available contents. The size of the handle
    indicates the amount of content that is visible relative to the total. For example, a
    small handle indicates that there is a lot more content that is out of view.

    Scroll bars float above the content and do not need reserved space. There are two types of
    scroll bar:

    - <b>Indicative</b>. These scroll bars simply give an indication that the content can be
      scrolled and they do not provide features that enable scrolling (the ability to scroll
      is provided by the widget to which the scroll bar is attached). Indicative scroll bars are
      suitable for shorter lists and content that you expect the user to browse rather than to
      search for specific items. This is the default type of scroll bar.

    - <b>Interactive</b>. With this type of scroll bar, the user can drag the handle or press the
      groove to change the scroll position. When used in an item model view (classes derived
      from HbAbstractItemView), these scroll bars can provide index feedback (HbIndexFeedback
      class) when the user drags the scroll bar or taps on the groove. The feedback is a popup
      that shows, for example, the initial letter in an alphabetical list. Interactive scroll bars
      are particularly suitable for long lists.

    Call \link HbScrollBar::setInteractive() setInteractive(true)\endlink to make a scroll bar
    interactive. Call isInteractive() to discover whether a scroll bar is interactive.

    \image html hbscrollbar.png A list view with a vertical interactive scroll bar

    %HbScrollBar provides other properties that control the following aspects of a scroll bar:

    - <b>Value</b>. This is a real value in the range 0.0 to 1.0 that indicates how far the handle
      is from the start of the groove. For example, 0.5 means the handle is half way along the
      groove. Call setValue() to set this property.

    - <b>Page size</b>. This is a real value in the range 0.0 to 1.0 that indicates how much of the
      total contents are currently visible. This property also controls the size of the handle.
      For example, a page size of 0.1 indicates that one out of ten pages are visible. Call
      setPageSize() to set this property.

    - <b>Orientation</b>. This controls whether the scroll bar is horizontal or vertical. Call
      setOrientation() to set this property.

    \section _usecases_hbscrollbar Using the HbScrollBar class

    Although it is possible to create an HbScrollBar object directly, scroll bars are created
    automatically when you create an HbScrollArea object or one of the item view classes that
    are derived from it. This example demonstrates changing the default scroll bars
    created for an HbScrollArea object from indicative to interactive:

    \code
    scrollArea->verticalScrollBar()->setInteractive(true);
    scrollArea->horizontalScrollBar()->setInteractive(true);
    \endcode

    You can replace the existing scroll bars by calling HbScrollArea::setHorizontalScrollBar()
    and HbScrollArea::setVerticalScrollBar().

    \sa HbScrollArea, HbAbstractItemView, HbIndexFeedback
 */

/*!
    \fn int HbScrollBar::type() const
 */

/*!
    \fn void HbScrollBar::valueChanged( qreal value, Qt::Orientation orientation );

    This signal is emitted when the user changes the position of the handle in an interactive
    scroll bar.
 */

/*!
    \fn void HbScrollBar::valueChangeRequested( qreal value, Qt::Orientation orientation );

    This signal is emitted when the user presses an interactive scroll bar's groove. The widget
    using the scroll bar must handle this signal (for example, by providing an animation that
    scrolls to the target position).

    \param value The value to which the user wants to scroll.
    \param orientation The scroll bar's orientation.

    \sa HbScrollBar::setInteractive()
 */

/*!
    \primitives
    \primitive{groove} HbFrameItem representing the groove of a scroll bar.
    \primitive{handle} HbFrameItem representing the handle of a scroll bar.
    \primitive{toucharea} HbTouchArea representing the scroll bar toucharea.
 */

static const int TOUCHAREA_ZVALUE = 1000;

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
        mLimitingFactor(0.0),
        mTopLeft(0.0),
        hasEffects(false),
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
    q->setFlag(QGraphicsItem::ItemHasNoContents, true);
}

void HbScrollBarPrivate::createPrimitives()
{
    Q_Q(HbScrollBar);

    if ( !grooveItem ) {
        grooveItem = q->style()->createPrimitive(HbStyle::PT_FrameItem, "groove", q);
        grooveItem->setZValue(2);
    }
    if ( !handleItem ) {
        handleItem = q->style()->createPrimitive(HbStyle::PT_FrameItem, "handle", q);
        handleItem->setZValue(3);
    }
    if( !mTouchArea ) {
        mTouchArea = q->style()->createPrimitive(HbStyle::PT_TouchArea, "toucharea", q);
        mTouchArea->setFlags(QGraphicsItem::ItemIsFocusable);
    }
    q->updatePrimitives();
}

void HbScrollBarPrivate::updatePosition()
{
    if (handleItem){
        if (mOrientation == Qt::Vertical) {
            handleItem->setPos(mTopLeft, mCurrentPosition * mLimitingFactor);
        } else {
            handleItem->setPos(mCurrentPosition * mLimitingFactor, mTopLeft);
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
        QRectF bRect = q->boundingRect();
        if(item){
            if (mOrientation == Qt::Vertical) {
                qreal height(mPageSize * bRect.height());
                if(!qFuzzyCompare(item->preferredHeight(),height)){
                    item->setPreferredHeight(height);
                    item->resize(item->size().width(), height);
                }
                mLimitingFactor =  bRect.height() - item->geometry().height();
                mTopLeft = item->geometry().topLeft().x();
            } else {
                qreal width(mPageSize * bRect.width());
                if(!qFuzzyCompare(item->preferredWidth(),width)){
                    item->setPreferredWidth(width);
                    item->resize(width, item->size().height());
                }
                mLimitingFactor =  bRect.width() - item->geometry().width();
                mTopLeft = item->geometry().topLeft().y();
            }
            updatePosition();
        }
    }
}

void HbScrollBarPrivate::groovePrimitiveData(HbStyleFramePrimitiveData *data)
{
    if (mInteractive) {
        if (mGroovePressed) {
            if (mOrientation == Qt::Vertical) {
                data->frameType = HbFrameDrawer::ThreePiecesVertical;
                data->frameGraphicsName = QLatin1String("qtg_fr_scroll_v_active_frame_pressed");
            } else {
                data->frameType = HbFrameDrawer::ThreePiecesHorizontal;
                data->frameGraphicsName = QLatin1String("qtg_fr_scroll_h_active_frame_pressed");
            }
        } else {
            if (mOrientation == Qt::Vertical) {
                data->frameType = HbFrameDrawer::ThreePiecesVertical;
                data->frameGraphicsName = QLatin1String("qtg_fr_scroll_v_active_frame_normal");
            } else {
                data->frameType = HbFrameDrawer::ThreePiecesHorizontal;
                data->frameGraphicsName = QLatin1String("qtg_fr_scroll_h_active_frame_normal");
            }
        }
    } else {
        if (mOrientation == Qt::Vertical) {
            data->frameType = HbFrameDrawer::ThreePiecesVertical;
            data->frameGraphicsName = QLatin1String("qtg_fr_scroll_v_frame");
        } else {
            data->frameType = HbFrameDrawer::ThreePiecesHorizontal;
            data->frameGraphicsName = QLatin1String("qtg_fr_scroll_h_frame");
        }
    }
    data->fillWholeRect = true;
}

void HbScrollBarPrivate::handlePrimitiveData(HbStyleFramePrimitiveData *data)
{
    if (mInteractive) {
        if (mThumbPressed) {
            if (mOrientation == Qt::Vertical) {
                data->frameType = HbFrameDrawer::ThreePiecesVertical;
                data->frameGraphicsName = QLatin1String("qtg_fr_scroll_v_active_handle_pressed");
            } else {
                data->frameType = HbFrameDrawer::ThreePiecesHorizontal;
                data->frameGraphicsName = QLatin1String("qtg_fr_scroll_h_active_handle_pressed");
            }
        } else {
            if (mOrientation == Qt::Vertical) {
                data->frameType = HbFrameDrawer::ThreePiecesVertical;
                data->frameGraphicsName = QLatin1String("qtg_fr_scroll_v_active_handle_normal");
            } else {
                data->frameType = HbFrameDrawer::ThreePiecesHorizontal;
                data->frameGraphicsName = QLatin1String("qtg_fr_scroll_h_active_handle_normal");
            }
        }
    } else {
        if (mOrientation == Qt::Vertical) {
            data->frameType = HbFrameDrawer::ThreePiecesVertical;
            data->frameGraphicsName = QLatin1String("qtg_fr_scroll_v_handle");
        } else {
            data->frameType = HbFrameDrawer::ThreePiecesHorizontal;
            data->frameGraphicsName = QLatin1String("qtg_fr_scroll_h_handle");
        }
    }
    data->fillWholeRect = true;
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
    Constructs a scroll bar with the given \a parent.
 */
HbScrollBar::HbScrollBar( QGraphicsItem *parent ) :
        HbWidget(*new HbScrollBarPrivate, parent)
{
    Q_D(HbScrollBar);
    d->q_ptr = this;
    d->init();
}

/*!
    Constructs a scroll bar with the given \a orientation and \a parent.
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
    Destructor.
 */
HbScrollBar::~HbScrollBar()
{
}

/*!
    Returns the scroll bar's \c value property. This indicates how far the handle is
    from the start of the groove and can be in range of 0.0 to 1.0.

    \sa HbScrollBar::setValue()
 */
qreal HbScrollBar::value() const
{
    Q_D( const HbScrollBar );
    return d->mCurrentPosition;
}

/*!
    Returns scroll bar's \c pageSize property. This is a real value in the range of 0.0 to
    1.0, which indicates the size of the visible content relative to the whole content.
    For example, a page size of 0.2 indicates that the overall size of the content is five
    times larger than what is currently visible.

    \sa HbScrollBar::setPageSize()
 */
qreal HbScrollBar::pageSize() const
{
    Q_D( const HbScrollBar );
    return d->mPageSize;
}

/*!
    Returns the orientation of scroll bar.

    \sa HbScrollBar::setOrientation()
 */
Qt::Orientation HbScrollBar::orientation() const
{
    Q_D( const HbScrollBar );
    return d->mOrientation;
}

/*!
    Returns true if the scroll bar is interactive and false if it is indicative.

    \sa HbScrollBar::setInteractive()
*/
bool HbScrollBar::isInteractive() const
{
    Q_D(const HbScrollBar);
    return d->mInteractive;
}

/*!
    Sets the value of the scroll bar's \c interactive property, which controls whether the
    scroll bar is interactive or indicative (the default).

    When the scroll bar is interactive, the user can drag the handle or press the groove to
    change the scroll position. The following table lists the signals that an interactive
    scroll bar emits when the user drags the handle or presses the groove.

    <table border="1" style="border-collapse: collapse; border: solid;">
    <tr><th>Signal</th><th>Description</th></tr>
    <tr><td>valueChanged()</td><td>This signal is emitted when the user drags the handle.</td></tr>
    <tr><td>valueChangeRequested()</td><td>This signal is emitted when the user presses the scroll
    bar's groove. The widget using the scroll bar must handle this signal (for example, by
    providing an animation that moves to the target position).</td></tr>
    </table>

    \param enabled A Boolean value; true for an interactive scroll bar, false for an indicative
           scroll bar.

    \sa HbScrollBar::isInteractive()
*/
void HbScrollBar::setInteractive( bool enabled )
{
    Q_D( HbScrollBar );
    if( d->mInteractive != enabled){
        d->mInteractive = enabled;
        if ( enabled ) {
            grabGesture(Qt::PanGesture);
            grabGesture(Qt::TapGesture);
        } else {
            ungrabGesture(Qt::PanGesture);
            ungrabGesture(Qt::TapGesture);
        }
        if(d->handleItem) {
            repolish();
            updatePrimitives();
        }
    }
}

/*!
    Sets the scroll bar's \c value property, which controls how far the handle is from
    the start of the groove.

    \param value A real value in the range 0.0 to 1.0. A value of 0.0 indicates that the
    handle is at the start of the groove and a value of 1.0 indicates that it is at the
    end.

    \sa HbScrollBar::value()
 */
void HbScrollBar::setValue( qreal value )
{
    Q_D(HbScrollBar);        

    value = qBound(qreal(0.0), value, qreal(1.0));
    if( !qFuzzyCompare(d->mCurrentPosition,value )) {
        d->mCurrentPosition = value;
        d->updatePosition();
    }
}

/*!
    Sets the scroll bar's \c pageSize property, which indicates the size of the visible content
    relative to the whole content. For example, a page size of 0.2 indicates that the
    overall size of the content is five times larger than what is currently visible.

    \param size A real value in the range of 0.0 to 1.0.
    \sa HbScrollBar::pageSize()
 */
void HbScrollBar::setPageSize( qreal size )
{
    Q_D(HbScrollBar);
    size = qBound(qreal(0.0), size, qreal(1.0));

    if(!qFuzzyCompare(d->mPageSize,size)) {
        d->mPageSize = size;
        d->sizeHelper();
    }
}

/*!
    Sets the scroll bar's \c orientation property.

    \param orientation Set this to \c Qt::Horizontal for a horizontal scroll bar and
            \c Qt::Vertical for a vertical scroll bar.

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

    if (d->grooveItem) {
        HbStyleFramePrimitiveData data;
        initPrimitiveData(&data, d->grooveItem);
        style()->updatePrimitive(d->grooveItem, &data, this);
    }
    if (d->handleItem) {
        HbStyleFramePrimitiveData data;
        initPrimitiveData(&data, d->handleItem);
        style()->updatePrimitive(d->handleItem, &data, this);
    }
    if (d->mTouchArea) {
        HbStyleTouchAreaPrimitiveData data;
        initPrimitiveData(&data, d->mTouchArea);
        style()->updatePrimitive(d->mTouchArea, &data, this);
        d->mTouchArea->setZValue(TOUCHAREA_ZVALUE);
    }
}

void HbScrollBar::initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive)
{
    Q_D(HbScrollBar);
    HbWidget::initPrimitiveData(primitiveData, primitive);
    QString itemName = HbStyle::itemName(primitive);
    if (itemName == QLatin1String("groove")) {
        d->groovePrimitiveData(hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData));
    } else if (itemName == QLatin1String("handle")) {
        d->handlePrimitiveData(hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData));
    }
}

/*!
    Reimplemented from QGraphicsItem.
 */
void HbScrollBar::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    Q_D(HbScrollBar);
    QGraphicsWidget::mousePressEvent(event);

    if ( !d->mInteractive ) {
        return;
    }

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
            HbStyleFramePrimitiveData data;
            initPrimitiveData(&data, d->handleItem);
            style()->updatePrimitive(d->handleItem, &data, this);
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
            HbStyleFramePrimitiveData data;
            initPrimitiveData(&data, d->handleItem);
            style()->updatePrimitive(d->handleItem, &data, this);
            emit d->core.handlePressed();
        }

        break;
    }
    event->accept();
}

/*!
    Reimplemented from QGraphicsItem.
 */
void HbScrollBar::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    Q_D(HbScrollBar);
    QGraphicsWidget::mouseReleaseEvent(event);

    if ( !d->mInteractive ) {
        return;
    }
    if (d->mThumbPressed) {
        d->mThumbPressed = false;
        HbWidgetFeedback::triggered(this, Hb::InstantReleased, Hb::ModifierSliderHandle);
        emit valueChanged(value(), orientation());
        emit d->core.handleReleased();
        HbStyleFramePrimitiveData data;
        initPrimitiveData(&data, d->handleItem);
        style()->updatePrimitive(d->handleItem, &data, this);
    } else if (d->mGroovePressed){
        HbWidgetFeedback::triggered(this, Hb::InstantReleased);
        d->repeatActionTimer.stop();
        d->mGroovePressed = false;
        HbStyleFramePrimitiveData data;
        initPrimitiveData(&data, d->grooveItem);
        style()->updatePrimitive(d->grooveItem, &data, this);
    }
    d->emittedPos = false;
    event->accept();            
}

/*!
    Reimplemented from QGraphicsItem.
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
    Reimplemented from QGraphicsWidget.
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
    Reimplemented from QObject.
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
    if (event->type() == QEvent::GraphicsSceneResize) {
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
        QRectF geo = item->geometry();
        if (d->mOrientation == Qt::Vertical) {
            d->mTopLeft = geo.topLeft().x();
            d->mLimitingFactor =  boundingRect().height() - geo.height();
        } else {
            d->mTopLeft = geo.topLeft().y();
            d->mLimitingFactor =  boundingRect().width() - geo.width();
        }
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
