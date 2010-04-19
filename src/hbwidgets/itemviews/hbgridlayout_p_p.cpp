/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
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
#include <hbgridlayout_p_p.h>
#include <hbgridlayout_p.h>
#include <hbabstractviewitem.h>
#include <hblayoututils_p.h>

#include <QGraphicsItem>
#include <QWidget> // for QWIDGETSIZE_MAX
#include <QTimer>
#include <QDebug>

namespace {
static const int ANIMATION_TIMEOUT = 20;
static const qreal ANIMATION_DELAY_RESOLUTION = 0.1;
}

HbGridLayoutPrivate::HbGridLayoutPrivate():
        mMaxVisRows(1),
        mMaxVisColumns(1),
        mScrollDirection(Qt::Vertical),
        mItemSize(QSize(0,0)),
        mPreferedSizeHint(QSize(0,0)),
        mMaxSizeHint(QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)),
        mSpacing(0),
        mLeftMargin(0),
        mRightMargin(0),
        mTopMargin(0),
        mBottomMargin(0),
        mClearCacheHint(false),
        mClearGeometry(false),
        mRecycling(false),
        mGeometry(QRectF()),
        q_ptr(0),
        mTimer(0),
        mEffectLoaded(false)

{
}

HbGridLayoutPrivate::~HbGridLayoutPrivate()
{

}

void HbGridLayoutPrivate::init()
{
    Q_Q(HbGridLayout);

    mTimer = new QTimer(q);
    q->connect(mTimer, SIGNAL(timeout()), q, SLOT(_q_timerUpdate()));

    mEffectLoaded = HbEffect::add("gridviewitem", "gridviewitem_reposition", "reposition");
}

void HbGridLayoutPrivate::setSize(const QSizeF &size,int &targetCount)
{
    qreal newHeight = size.height() - (mMaxVisRows * mSpacing) - mTopMargin - mBottomMargin;
    qreal newWidth = size.width() - (mMaxVisColumns * mSpacing) - mLeftMargin - mRightMargin;
    if(newHeight <0)
        newHeight = 0;
    if(newWidth <0)
        newWidth = 0;
    mItemSize.setHeight(newHeight/ mMaxVisRows);
    mItemSize.setWidth(newWidth/ mMaxVisColumns);
    targetCount = mMaxVisColumns * mMaxVisRows;
    reset();
}

void HbGridLayoutPrivate::setMargins(qreal topMargin, qreal bottomMargin, qreal leftMargin, qreal rightMargin)
{
    mLeftMargin = leftMargin;
    mRightMargin = rightMargin;
    mTopMargin = topMargin;
    mBottomMargin = bottomMargin;
}

void HbGridLayoutPrivate::setGeometry(const QRectF &effectiveRect)
{
        QRectF adjustRect(effectiveRect);
        adjustRect.adjust(+mLeftMargin, +mTopMargin, -mRightMargin, -mBottomMargin);
        if(Qt::Vertical == mScrollDirection)
            setVerticalGeometry(adjustRect);
        else
            setHorizontalGeometry(adjustRect);
        mClearGeometry = false;
}

/*!
    From QGraphicsLayout.

    The minimum size hint is the minimum size of the first child layout item.

    The preferred size hint is the sum of the preferred size hints of the
    child layout items.

    The maximum size hint "unlimited", i.e. the child layout items do not
    affect the maximum size hint.
 */
QSizeF HbGridLayoutPrivate::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    QSizeF sizeHint( 0, 0 );
    if(which == Qt::MaximumSize) {
        // Sub items do not affect the maximum size hint.
        sizeHint = mMaxSizeHint;
    } else if(which == Qt::PreferredSize) {
        if(!mClearCacheHint){
            sizeHint = mPreferedSizeHint;
        }
        else {
            constraint.boundedTo(QSizeF(0,0));
            sizeHint = Qt::Vertical ==mScrollDirection?verSizeHint()
                                                              :horSizeHint();
            const_cast<HbGridLayoutPrivate*>( this )->mPreferedSizeHint = sizeHint;
            const_cast<HbGridLayoutPrivate*>( this )->mClearCacheHint = false;
        }
        
    }
    else {
        if(mItemSize.isValid()) {
         sizeHint = mItemSize;
        }
    }
    return sizeHint;
}

/*!
    From QGraphicsLayout.
 */
void HbGridLayoutPrivate::setVerticalGeometry( const QRectF &effectiveRect ){

    Q_Q( HbGridLayout );
    qreal y (effectiveRect.y());
    qreal x (effectiveRect.x());
    int dinamicCount(mMaxVisColumns);
    int itemCount(q->count());
    int r (0);
    qreal tempheight(0);
    for(int i=0;i<itemCount;i++) {
        if (r%dinamicCount == 0){
            r=0;
            x =effectiveRect.x();
        }
        QGraphicsLayoutItem *item = q->itemAt(i);
        if(item){
            item->setPreferredSize(mItemSize.width(),mItemSize.height());
            item->setMinimumSize(mItemSize.width(),mItemSize.height());
            item->setMaximumSize(mItemSize.width(),mItemSize.height());
            item->setGeometry( QRectF(x,
                                      y,
                                      mItemSize.width(),
                                      mItemSize.height() ));
            tempheight = qMax(tempheight,mItemSize.height());
            x += mItemSize.width()+mSpacing;
            if (r == dinamicCount-1) {
                y += tempheight+mSpacing;
            }
            ++r;
        }
    }
}

/*!
    From QGraphicsLayout.
 */
void HbGridLayoutPrivate::setHorizontalGeometry(const QRectF &effectiveRect){

    Q_Q(HbGridLayout);
    qreal y (effectiveRect.y());
    qreal x (effectiveRect.x());
    int dinamicCount(mMaxVisRows);
    int itemCount(q->count());
    int r(0);
    qreal tempWidth(0);
    for (int i = 0; i < itemCount; i++) {
        if (r%dinamicCount == 0){
            if(r!=0)
                r=0;
            y =effectiveRect.y();
        }
        QGraphicsLayoutItem *item = q->itemAt(i);
        if(item){
            item->setPreferredSize(mItemSize.width(),mItemSize.height());
            item->setMinimumSize(mItemSize.width(),mItemSize.height());
            item->setMaximumSize(mItemSize.width(),mItemSize.height());
            item->setGeometry(QRectF(x,
                                     y,
                                     mItemSize.width(),
                                     mItemSize.height()));
            tempWidth = qMax(tempWidth,mItemSize.width());
            y += mItemSize.height()+mSpacing;
            if (r == dinamicCount-1) {
                x += tempWidth+mSpacing;
            }
            ++r;
        }
    }
}

QSizeF HbGridLayoutPrivate::verSizeHint() const{

    Q_Q( const HbGridLayout );
    int dinamicCount(mMaxVisColumns);
    int itemCount(q->count());
    QSizeF tempSize(0,0);
    int r(0);
    QSizeF sizeHint(0,0);
    for (int i=0;i<itemCount;i++) {
        if ((r%dinamicCount == 0)||(i == itemCount -1 )){
            r=0;
            sizeHint.rwidth() = qMax( tempSize.width(), sizeHint.width() );
            tempSize.setWidth(0);
        }
        QGraphicsLayoutItem *item = q->itemAt(i);
        if(item){
            if(mItemSize.isValid()) {
                tempSize.rwidth() += (mItemSize.width()+mSpacing);
                tempSize.rheight() = qMax(tempSize.height(),(mItemSize.height()+mSpacing));
            }
        }
        if((r == dinamicCount-1) ||(i == itemCount -1)) {
            sizeHint.rheight() = sizeHint.height() + tempSize.height();
        }
        ++r;
    }
    sizeHint.rheight() += mTopMargin + mBottomMargin;
    sizeHint.rwidth() += mLeftMargin + mRightMargin;
    return sizeHint;
}

QSizeF HbGridLayoutPrivate::horSizeHint() const{

    Q_Q(const HbGridLayout);
    int dinamicCount(mMaxVisRows);
    int itemCount(q->count());
    QSizeF tempSize( 0, 0 );
    QSizeF sizeHint( 0, 0 );
    int r(0);
    for (int i=0;i<itemCount;i++) {
        if ((r%dinamicCount == 0)||(i == itemCount -1)){
            r=0;
            sizeHint.rheight() = qMax(tempSize.height(),sizeHint.height());
            tempSize.setHeight(0);
        }
        QGraphicsLayoutItem *item = q->itemAt(i);
        if(item){
            if(mItemSize.isValid()) {
                tempSize.rheight() += (mItemSize.height()+mSpacing);
                tempSize.rwidth() = qMax(tempSize.width(),(mItemSize.width()+mSpacing));
            }
        }
        if((r == dinamicCount-1) ||(i == itemCount -1)) {
            sizeHint.rwidth() = sizeHint.width() + tempSize.width();
        }
        ++r;
    }
    sizeHint.rheight() +=mLeftMargin + mRightMargin;
    sizeHint.rwidth() += mTopMargin + mBottomMargin;
    return sizeHint;
}


void HbGridLayoutPrivate::reset()
{
    mClearCacheHint = true;
    mClearGeometry = true;
}

void HbGridLayoutPrivate::insertItem(QGraphicsLayoutItem *item, int index, bool animate, bool reset)
{
    index = qMin(index, mItems.count());
    if (index < 0) {
        index = mItems.count();
    }
    HbLayoutUtils::addChildItem(q_ptr, item);

    if (animate) {
        updateAnimationList(item, index - 1, HbGridLayout::InsertAnimation);
    }

    mItems.insert(index, item);

    if (reset) {
        this->reset();
    }

    if (animate && !mAnimatedItems.isEmpty()) {
        if (!mTimer->isActive()) {
            mTimer->start(ANIMATION_TIMEOUT);
        }

        q_ptr->invalidate();
        q_ptr->activate();

        _q_timerUpdate(false);
    } else if (mAnimatedItems.isEmpty()) {
        mTimer->stop();
    }
}

void HbGridLayoutPrivate::removeItem(QGraphicsLayoutItem *item, int index, bool animate)
{
    for (int i = mAnimatedItems.count() - 1; i >= 0; --i) {
        AnimationData *itemData = mAnimatedItems.at(i);
        if (itemData->item == item->graphicsItem()) {
            stopAnimation(itemData);
        } 
        
        if (itemData->startItem == item) {
            itemData->startItem = 0;
        }
    }

    if (animate) {
        updateAnimationList(item, index, HbGridLayout::RemoveAnimation);
    }

    mItems.removeAt(index);

    item->setParentLayoutItem(0);
    reset();

    if (animate && !mAnimatedItems.isEmpty()) {
        if (!mTimer->isActive()) {
            mTimer->start(ANIMATION_TIMEOUT);
        }

        q_ptr->invalidate();
        q_ptr->activate();

        _q_timerUpdate(false);
    } else if (mAnimatedItems.isEmpty()) {
        mTimer->stop();
    }
}

/*!
    First stops the current item animation if it was animated. Then creates a new animation data for next index
    items if not already present.
*/
void HbGridLayoutPrivate::updateAnimationList(QGraphicsLayoutItem *item, int index, HbGridLayout::AnimationType type)
{
    if (!mEffectLoaded) {
        emit q_ptr->animationFinished(item, type);
        return;
    }


    q_ptr->activate();

    int itemCount = mItems.count();
    if ((index + 1) >= itemCount) {
        emit q_ptr->animationFinished(item, type);
    } else {
        for (int i = index + 1; i < itemCount; ++i) {
            HbAbstractViewItem *viewItem = qobject_cast<HbAbstractViewItem *>(static_cast<QGraphicsWidget *>(mItems.at(i)->graphicsItem()));
            if (!viewItem->modelIndex().isValid()) {
                if (i == (index + 1)) {
                    emit q_ptr->animationFinished(item, type);
                }
                break;
            }

            AnimationData *data = animationData(viewItem);            
            if (!data) {
                data = new AnimationData;
                mAnimatedItems.append(data);

                data->item = viewItem;
                QTransform itemTransform = data->item->transform();
                data->originalM31 = itemTransform.m31();
                data->originalM32 = itemTransform.m32();
                data->originalZValue = data->item->zValue();
                data->item->setZValue(data->originalZValue + 1);
               
                data->startDelay = (i - index - 1) * ANIMATION_DELAY_RESOLUTION;
                data->startPos = data->item->pos();
                data->startScenePos = data->item->scenePos();

                if (i == (index + 1)) {
                    if (type == HbGridLayout::RemoveAnimation) {
                        data->startItem = mItems.at(i);
                    } else {
                        data->startItem = item;
                    }
                    data->type = type;
                } else {
                    data->startItem = 0;
                }
            } else {
                if (i == (index + 1)) {
                    if (data->startItem) {
                        emit q_ptr->animationFinished(data->startItem, data->type);
                    }

                    if (type == HbGridLayout::RemoveAnimation) {
                        data->startItem = mItems.at(i);
                    } else {
                        data->startItem = item;
                    }

                data->type = type;
                }
                break;
            }
        }
    }
}

void HbGridLayoutPrivate::_q_timerUpdate(bool updateProgress)
{
    bool stopTimer = true;

    int itemCount = mAnimatedItems.count();
    for (int i = itemCount - 1; i >= 0; --i) {
        AnimationData *data = mAnimatedItems.at(i);
        
        if (!HbEffect::effectRunning(data->item, "reposition")) {
            if (updateProgress) {
                if (data->startDelay <= 0) {
                    QTransform transform;
                    transform.setMatrix(transform.m11(), transform.m12(), transform.m13(), 
                        transform.m21(), transform.m22(), transform.m23(),
                        data->originalM31, data->originalM32, transform.m33());
                    data->item->setTransform(transform);

                    QRectF extRect(data->startScenePos, data->item->boundingRect().size());
                    HbEffect::start(data->item, "gridviewitem", "reposition", q_ptr, "effectFinished", QVariant(), extRect);
                } else {
	                data->startDelay -= ANIMATION_DELAY_RESOLUTION;
                    stopTimer = false;
			    }
            } else {
                QPointF delta = data->startPos - data->item->pos();
                data->item->setTransform(QTransform::fromTranslate(delta.x(), delta.y()), false);
                stopTimer = false;
            }
        } 
    }

    if (stopTimer) {
        mTimer->stop();
    }
}

void HbGridLayoutPrivate::stopAnimation(AnimationData *data)
{
    if (data) {
        QTransform transform;
        transform.setMatrix(transform.m11(), transform.m12(), transform.m13(), 
            transform.m21(), transform.m22(), transform.m23(),
            data->originalM31, data->originalM32, transform.m33());
        data->item->setTransform(transform);

        if (HbEffect::effectRunning(data->item, "reposition")) {
            HbEffect::cancel(data->item, "reposition", false, false, true);
        }

        data->item->setZValue(data->originalZValue);

        if (data->startItem) {
            emit q_ptr->animationFinished(data->startItem, data->type);
        }

        mAnimatedItems.removeOne(data);
        delete data;
    }
}

HbGridLayoutPrivate::AnimationData *HbGridLayoutPrivate::animationData(QGraphicsLayoutItem *item) const
{   
    if (item) {
        QGraphicsItem *graphicsItem = item->graphicsItem();

        int animateItemsCount = mAnimatedItems.count();
        for (int j = animateItemsCount - 1; j >= 0; --j) {
            AnimationData *currentData = mAnimatedItems.at(j);
            
            if (currentData->item == graphicsItem) {
                return currentData;
            }
        }
    }

    return 0;
}

