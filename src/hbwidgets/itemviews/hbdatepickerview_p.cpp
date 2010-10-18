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
#include "hbdatepickerview_p.h"
#include "hbabstractitemview_p.h"
#include "hbabstractitemcontainer_p.h"
#include "hbabstractitemcontainer_p_p.h"
#include "hblistlayout_p.h"
#include "hbstyleoption_p.h"
#include "hbframeitem.h"
#include "hbstyleprimitivedata.h"

#include <hbmodeliterator.h>
#include <hbdatepickerviewitem_p.h>
#include <hbtapgesture.h>
#include <hbstyleframeprimitivedata.h>

#include <qmath.h>
#include <QTimeLine>
#include <QGraphicsItemAnimation>
#include <QGestureEvent>
#include <QGesture>
#include <QStringListModel>
#include <QCoreApplication>

#define HB_DATEPICKERVIEW_PREFERRED_ITEMS 3

class HbDatePickerViewPrivate : public HbAbstractItemViewPrivate
{
public:
    HbDatePickerViewPrivate();
    ~HbDatePickerViewPrivate();

    void _q_scrollingStarted();
    void _q_scrollingEnded();
    void _q_itemSelected(QPointF point);

    void createPrimitives();
    void updateScrollMetrics(){ mAbleToScrollY = true; }

    qreal topBoundary();
    qreal bottomBoundary();

    HbAbstractViewItem* getCenterItem();
    void slectItemAt(const QPointF& point,QGraphicsItem* itemToMap);
    void setItemState(HbAbstractViewItem* item = 0);

    QTimeLine *animationtimer;
    QGraphicsItemAnimation *animation;
    Q_DECLARE_PUBLIC(HbDatePickerView)
    QGraphicsObject   *mBackground;
    QGraphicsObject   *mFrame;//overlay
    QGraphicsObject   *mHighlight;
    QGraphicsObject   *mDivider;
   // bool tapActive;
    QPointer<HbAbstractViewItem> mPreviousItem;

    enum ViewState
    {
        NoState = 0,
        TapStarted = 1,
        ScrollStarted = 2
    };
    Q_DECLARE_FLAGS(ViewStates, ViewState)
    ViewStates viewStates;

};
class HbDatePickerContainerPrivate;


class HbDatePickerContainer:public HbAbstractItemContainer
{
public:
    HbDatePickerContainer(QGraphicsItem* parent = 0);

    void setModelIndexes(const QModelIndex &startIndex = QModelIndex());
    void resizeContainer();
    void itemAdded(int index, HbAbstractViewItem *item, bool animate);
    void itemRemoved(HbAbstractViewItem *item, bool animate);

    bool isLoopingEnabled(){return true;}
    bool isLoopingNeeded();
    int maxItemPossible()const;

protected:
    QPointF recycleItems(const QPointF &delta);
    int maxItemCount() const;
    HbAbstractViewItem *createDefaultPrototype() const;
    void viewResized(const QSizeF &size);

private:
    Q_DECLARE_PRIVATE(HbDatePickerContainer)
    friend class HbDatePickerView;
    friend class HbDatePickerViewPrivate;
};

class HbDatePickerContainerPrivate:public HbAbstractItemContainerPrivate
{
public:
    HbDatePickerContainerPrivate();
    HbAbstractViewItem *shiftDownItem(QPointF& delta);
    HbAbstractViewItem *shiftUpItem(QPointF& delta);
    qreal getSmallestItemHeight() const;
    void init();
    HbAbstractViewItem *item(const QModelIndex &index) const;

    HbListLayout *mLayout;
    Q_DECLARE_PUBLIC(HbDatePickerContainer)
};

HbDatePickerContainerPrivate::HbDatePickerContainerPrivate()
    :mLayout(0)
{

}


void HbDatePickerContainerPrivate::init()
{
    Q_Q(HbDatePickerContainer);
    mLayout = new HbListLayout();
    mLayout->setContentsMargins(0, 0, 0, 0);
    //mLayout->setPreferredWidth(q->size().width());

    q->setLayout(mLayout);
}

bool HbDatePickerContainer::isLoopingNeeded()
{
    Q_D(HbDatePickerContainer);
    return (isLoopingEnabled() && !(d->mItems.count() < maxItemPossible()));
}

void HbDatePickerContainer::setModelIndexes(const QModelIndex &startIndex)
{
    Q_D(HbDatePickerContainer);

    if (!d->mItemView || !d->mItemView->model()) {
        return;
    }

    QModelIndex index = startIndex;
    if (!index.isValid()) {
        if (!d->mItems.isEmpty()) {
            index = d->mItems.first()->modelIndex();
        }

        if (!index.isValid()) {
            index = d->mItemView->modelIterator()->nextIndex(index);
        }
    }

    QModelIndexList indexList;

    int itemCount(d->mItems.count());

    if (itemCount != 0 && index.isValid()) {
        indexList.append(index);
    }

    for (int i = indexList.count(); i < itemCount; ++i) {
        index = d->mItemView->modelIterator()->nextIndex(indexList.last());

        if (index.isValid()) {
            indexList.append(index);
        } else {
            break;
        }
    }
    if(isLoopingEnabled() && indexList.count()<itemCount){
        QModelIndex firstModelIndex =  d->mItemView->modelIterator()->index(0,QModelIndex());
        indexList.append(firstModelIndex);
        for (int i = indexList.count(); i < itemCount; ++i) {
            index = d->mItemView->modelIterator()->nextIndex(indexList.last());

            if (index.isValid()) {
                indexList.append(index);
            } else {
                break;
            }
        }
    }

    int indexCount(indexList.count());
    for (int i = 0; i < itemCount; ++i) {
        HbAbstractViewItem *item = d->mItems.at(i);
        HbAbstractViewItem *prototype = 0;
        // setItemModelIndex() is considered recycling. It is called only, if recycling is permitted
        if (i < indexCount) {
            prototype = d->itemPrototype(indexList.at(i));
        }
        if (prototype) {
            if (    prototype == item->prototype()
                &&  d->mItemRecycling) {
                setItemModelIndex(item, indexList.at(i));
            } else if (     d->mItemRecycling
                       ||   (   !d->mItemRecycling
                            &&  item->modelIndex() != indexList.at(i))) {
                d->deleteItem(item);
                insertItem(i, indexList.at(i));
            } // else: !d->mItemRecycling && item->modelIndex().isValid() == indexList.at(i))
        } else {
            d->deleteItem(item);
            itemCount--;
            i--;
        }
    }
}

void HbDatePickerContainer::resizeContainer()
{
    Q_D(HbAbstractItemContainer);

    if (d->mItemView) {
        resize(d->mItemView->size().width(), layout()->preferredHeight());
    } else {
        resize(0, layout()->preferredHeight());
    }
}

void HbDatePickerContainer::itemAdded(int index, HbAbstractViewItem *item, bool animate)
{
    Q_D(HbDatePickerContainer);
    d->mLayout->insertItem(index,item, animate);
}

void HbDatePickerContainer::itemRemoved(HbAbstractViewItem *item, bool animate)
{
    Q_D(HbDatePickerContainer);
    d->mLayout->removeItem(item, animate);
}

void HbDatePickerContainer::viewResized(const QSizeF &viewSize)
{
    QSizeF newSize = this->size();
    newSize.setWidth( viewSize.width() );
    resize( newSize );
}

QPointF HbDatePickerContainer::recycleItems(const QPointF &delta)
{
    Q_D(HbDatePickerContainer);

    if( itemView()->size().height() < size().height()){
          const qreal diff = d->getDiffWithoutScrollareaCompensation(delta);
          if(diff !=0.0){
                QPointF newDelta(0.0, 0.0);
                qreal result = 0.0;
            HbAbstractViewItem *item = 0;
            if (diff < 0.0) {
                while (-newDelta.y() > diff) {
                    item = d->shiftUpItem(newDelta);
                    if (!item) {
                        break;
                    }
                }
            }
            else {
                while (-newDelta.y() < diff) {
                    item = d->shiftDownItem(newDelta);
                    if (!item) {
                        break;
                    }
                }
            }
                result = newDelta.y();
                return QPointF(delta.x(),delta.y()+result);

          }
      }
      return delta;

}

HbAbstractViewItem *HbDatePickerContainerPrivate::item(const QModelIndex &index) const
{
    int itemCount = mItems.count();
    for(int i=0;i<itemCount;++i) {
        if(mItems.at(i)->modelIndex() == index) {
            return mItems.at(i);
        }
    }
    return 0;
}
HbAbstractViewItem *HbDatePickerContainerPrivate::shiftDownItem(QPointF& delta)
{
    Q_Q(HbDatePickerContainer);

    HbAbstractViewItem *item = 0;
    HbAbstractViewItem *lastItem = mItems.last();

    QModelIndex nextIndex = mItemView->modelIterator()->nextIndex(lastItem->modelIndex());
    if(q->isLoopingEnabled() && (!nextIndex.isValid())){
       nextIndex = mItemView->model()->index(0,0);
    }
    if (nextIndex.isValid()) {
        item = mItems.takeFirst();

        q->itemRemoved(item,false);

        delta.setY(delta.y() - item->size().height());

        mItems.append(item);

        q->setItemModelIndex(item, nextIndex);

        q->itemAdded(mItems.count() - 1, item,false);
    }
    mBufferSize=4;
    return item;
}

HbAbstractViewItem *HbDatePickerContainerPrivate::shiftUpItem(QPointF& delta)
{
    Q_Q(HbDatePickerContainer);

    HbAbstractViewItem *item = 0;
    HbAbstractViewItem *firstItem = mItems.first();

    QModelIndex previousIndex = mItemView->modelIterator()->previousIndex(firstItem->modelIndex());
    if(q->isLoopingEnabled() && !previousIndex.isValid()){
        previousIndex = mItemView->model()->index(mItemView->model()->rowCount()-1,0);
    }
    if (previousIndex.isValid()) {
        item = mItems.takeLast();

        q->itemRemoved(item,false);

        mItems.insert(0, item);

        q->setItemModelIndex(item, previousIndex);

        qreal itemHeight=0;
        if (q->uniformItemSizes()) {
            itemHeight = mItems.last()->preferredHeight();
        } else {
            //This is time consuming and causes backwards srolling to be slower than forwards.
            //The sizehint of the item is dirty.
            itemHeight = item->preferredHeight();
        }

        delta.setY(delta.y() + itemHeight);

        q->itemAdded(0, item,false);
    }
    return item;
}
qreal HbDatePickerContainerPrivate::getSmallestItemHeight() const
{
    Q_Q(const HbDatePickerContainer);
    qreal minHeight = 0.0;
    if (mItems.count() > 0) {
        minHeight = mLayout->smallestItemHeight();
    }
    if (minHeight == 0.0) {
        QModelIndex index;
        while (mItems.isEmpty()) {
            // in practise following conditions must apply: itemview is empty and scrollTo() has been called.
            // Starts populating items from given mFirstItemIndex
            if ( mFirstItemIndex.isValid()) {
                index = mFirstItemIndex;
                const_cast<QPersistentModelIndex&>(mFirstItemIndex) = QModelIndex();
            } else {
                index = mItemView->modelIterator()->nextIndex(index);
            }
            if (!index.isValid()) {
                break;
            }
            const_cast<HbDatePickerContainer *>(q)->insertItem(0, index);
        }

        if (!mItems.isEmpty()) {
            minHeight = mItems.first()->preferredHeight();
        }
    }
    return minHeight;
}
int HbDatePickerContainer::maxItemPossible()const
{
    Q_D(const HbDatePickerContainer);
    qreal minHeight = d->getSmallestItemHeight();
    if (minHeight > 0) {
        int countEstimate = qCeil(d->mItemView->boundingRect().height() / minHeight)
            + d->mBufferSize;
        return countEstimate;
    }
    return 0;
}
int HbDatePickerContainer::maxItemCount() const
{
    Q_D(const HbDatePickerContainer);
    int targetCount = HbAbstractItemContainer::maxItemCount();
    if (targetCount > 0
        && d->mItemRecycling
        && d->mPrototypes.count() <= 1) {

        qreal minHeight = d->getSmallestItemHeight();

        if (minHeight > 0) {
            int countEstimate = qCeil(d->mItemView->boundingRect().height() / minHeight)
                + d->mBufferSize;

            // This limits the targetCount not to be larger
            // than row count inside model.
            targetCount = qMin(targetCount, countEstimate);
        } else {
            targetCount = 0;
        }
    }
    return targetCount;
}
HbAbstractViewItem *HbDatePickerContainer::createDefaultPrototype()const
{
    return new HbDatePickerViewItem();
}

HbDatePickerContainer::HbDatePickerContainer(QGraphicsItem* parent)
    :HbAbstractItemContainer(*(new HbDatePickerContainerPrivate),parent )
{
    Q_D(HbDatePickerContainer);
    d->init();
}

HbDatePickerViewPrivate::HbDatePickerViewPrivate()
    :animationtimer(0),animation(0),
    mBackground(0),mFrame(0),mHighlight(0),mDivider(0),
    viewStates(NoState)
{

}
HbDatePickerViewPrivate::~HbDatePickerViewPrivate()
{
    if(animationtimer){
        if(animationtimer->state() != QTimeLine::NotRunning) {
            animationtimer->stop();
        }
        delete animationtimer;
        delete animation;
    }
}


HbDatePickerView::HbDatePickerView(QGraphicsItem *parent)
    :HbAbstractItemView(*(new HbDatePickerViewPrivate),new HbDatePickerContainer,new HbModelIterator(),parent)
{
    Q_D(HbDatePickerView);
    d->mAbleToScrollY = true;
    setSelectionMode(HbAbstractItemView::SingleSelection);
    setLongPressEnabled(false);
    setUniformItemSizes(true);
    setEnabledAnimations(HbAbstractItemView::TouchDown);
    setItemRecycling(true);
    //scroll area settings
    setClampingStyle(HbScrollArea::BounceBackClamping);
    setVerticalScrollBarPolicy(HbScrollArea::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(HbScrollArea::ScrollBarAlwaysOff);
    setFrictionEnabled(true);

    //dont want this to occupy entire screen. preferred is few items.
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);


    bool b = connect(this,SIGNAL(scrollingStarted()),this,SLOT(_q_scrollingStarted()));
    Q_ASSERT(b);
    b = connect(this,SIGNAL(scrollingEnded()),this,SLOT(_q_scrollingEnded()));
    Q_ASSERT(b);
    d->createPrimitives();
    updatePrimitives();
    setModel(new QStringListModel());

}

HbDatePickerView::~HbDatePickerView()
{

}

void HbDatePickerView::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_D(HbDatePickerView);

    HbAbstractItemView::rowsRemoved(parent,start,end);
    HbDatePickerContainer* container = static_cast<HbDatePickerContainer*>(d->mContainer);

    container->d_func()->updateItemBuffer();
    //d->mContainer->layout()->activate();

    d->mPostponedScrollIndex = currentIndex();
    d->mPostponedScrollHint = PositionAtCenter;
    QCoreApplication::postEvent(this, new QEvent(QEvent::LayoutRequest));
}
void HbDatePickerView::rowsAboutToBeInserted(const QModelIndex &index, int start, int end)
{
    Q_D(HbDatePickerView);
    QPointF containerPos =  d->mContainer->pos();
    if(containerPos.y()>0){
        containerPos.setY(0);
        d->mContainer->setPos(containerPos);
    }
    HbAbstractItemView::rowsAboutToBeInserted(index,start,end);
}

void HbDatePickerView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_D(HbDatePickerView);
    HbAbstractItemView::rowsInserted(parent,start,end);
    //d->mContainer->layout()->activate();
   // scrollTo(currentIndex(),PositionAtCenter);
    d->mPostponedScrollIndex = currentIndex();
    d->mPostponedScrollHint = PositionAtCenter;
}

void HbDatePickerView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    Q_D(HbDatePickerView);
    hint = PositionAtCenter;
    if (!d->mModelIterator->model()
        ||  index.model() != d->mModelIterator->model()) {
        return;
    }
    //If item is in the buffer, just reveal it.
    //This is always the case if recycling is off
    //and sometimes the case when recycling is on
    if (itemRecycling()) {
        if (    !d->mContainer->itemByIndex(index)
            ||  hint != EnsureVisible) {
            //Now the item is not in the buffer.
            //We must first set the item to be in the buffer
            //If the item is above let's put it first and if it is below put it last
            int newIndex = -1;
            switch (hint) {
            case PositionAtCenter: {
                    int containerCount = d->mContainer->items().count();
                    newIndex = index.row() - containerCount / 2 ;
                    if(newIndex < 0){
                        if(((HbDatePickerContainer*)(d->mContainer))->isLoopingNeeded()){
                            newIndex = d->mModelIterator->indexCount()+newIndex;
                        }
                        else{
                            newIndex = 0;
                        }
                    }
                    else if(newIndex > 0) {
                            //the item is in buffer and the looping is not needed
                        if(!((HbDatePickerContainer*)(d->mContainer))->isLoopingNeeded()){
                            newIndex = 0;
                            }
                        }
                    
                      //no change newIndex = 0
                      break;
                }
                    
            case EnsureVisible:
            case PositionAtTop:
            case PositionAtBottom:
            default: {
                     }
            }
            d->mContainer->setModelIndexes(d->mModelIterator->index(newIndex));
        }
    }



    //HbAbstractItemView::scrollTo(index, hint);
    if (    index.isValid()
        &&  d->mModelIterator->model() == index.model()) {
        d->mPostponedScrollIndex = QPersistentModelIndex();
        d->scrollTo(index, hint);
        d->mPostponedScrollIndex = index;
        d->mPostponedScrollHint = hint;
        if (    d->mContainer->itemRecycling()
            &&  !d->mContainer->items().count()) {
            // settings index from which loading viewitems start when itemview is
            // empty or reset by setModel()
            ((HbDatePickerContainer*)d->mContainer)->d_func()->mFirstItemIndex = index;
        }
       // QCoreApplication::postEvent(this, new QEvent(QEvent::LayoutRequest));
    }
}

void HbDatePickerViewPrivate::_q_scrollingStarted()
{
    viewStates |= ScrollStarted;
}

void HbDatePickerViewPrivate::slectItemAt(const QPointF& point,QGraphicsItem* itemTomap)
{
    Q_Q(HbDatePickerView);
    QPointF mappedPoint = itemTomap->mapToScene(point);
    HbDatePickerContainer* container = static_cast<HbDatePickerContainer*>(mContainer);
    HbDatePickerContainerPrivate* containerPriv = container->d_func();



    HbAbstractViewItem* item = qobject_cast<HbAbstractViewItem*>(itemAt(mappedPoint)); //(q->sender()); 

    if(item){
        QPointF centerPt = q->mapToScene(q->boundingRect().center());
        qreal itemHeight = containerPriv->getSmallestItemHeight();
        centerPt.setY(centerPt.y()- itemHeight/2);
        QPointF itemPos = q->mapToScene(containerPriv->itemBoundingRect(item).topLeft());
        QPointF delta = itemPos-centerPt;

        if(qCeil(delta.y())!=0){
            QPointF contPos = container->pos();
            QPointF newPos =  contPos-QPointF(0,qCeil(delta.y()));
            //need to remove already posted request to avoid multiple scrolling
            QCoreApplication::removePostedEvents(q,QEvent::LayoutRequest);
            q->scrollContentsTo(-newPos,200);
        }
        else {
            q->setCurrentIndex(item->modelIndex());
            emit q->itemSelected(item->modelIndex().row());
            setItemState(item);
        }


    }

}

void HbDatePickerViewPrivate::setItemState(HbAbstractViewItem* item)
{
    if(!item){
        Q_Q(HbDatePickerView);
        QPointF mappedPoint = q->mapToScene(q->boundingRect().center());
        item = qobject_cast<HbAbstractViewItem*>(itemAt(mappedPoint));
    }
    if(item){
        //change the items states for the current selection
        if(mPreviousItem && mPreviousItem->property("state").toString().compare("normal")){
            mPreviousItem->setProperty("state", "normal");
        }
        if(item && item->property("state").toString().compare("selected")){
            item->setProperty("state", "selected");
        }
        mPreviousItem = item;
    }
}

void HbDatePickerViewPrivate::_q_scrollingEnded()
{
    if(mOptions & PanningActive){
        return;
    }
    if(viewStates.testFlag(TapStarted)){
        viewStates &= ~TapStarted;
        return;
    }
    Q_Q(HbDatePickerView);
    mContainer->layout()->activate();//call this to get proper item;

    slectItemAt(q->boundingRect().center(),q);
}

void HbDatePickerViewPrivate::_q_itemSelected(QPointF point)
{
    Q_Q(HbDatePickerView);
    if(!mIsScrolling && !mIsAnimating){
        slectItemAt(point,qobject_cast<HbAbstractViewItem*>(q->sender()));
    }

}

HbAbstractViewItem* HbDatePickerViewPrivate::getCenterItem()
{
    Q_Q(HbDatePickerView);

    if(!q->scene()) {
        return 0;
    }
    QPointF centerPt = q->mapToScene(q->boundingRect().center());
    return itemAt(centerPt);
}
qreal HbDatePickerViewPrivate::topBoundary()
{
    //top boundary and bottom boundary is different for tumble view
    //it is half item less than the middle of the view
    Q_Q( HbDatePickerView );
    qreal itemheight = (static_cast<HbDatePickerContainer*>(mContainer))->d_func()->getSmallestItemHeight();
    return (-(q->boundingRect().height()-itemheight)/2);
}

qreal HbDatePickerViewPrivate::bottomBoundary()
{
    Q_Q ( HbDatePickerView);
    qreal itemheight = (static_cast<HbDatePickerContainer*>(mContainer))->d_func()->getSmallestItemHeight();   
    return mContents->boundingRect().height()-((q->boundingRect().height()+itemheight)/2);   
}

QSizeF HbDatePickerView::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(constraint);
    Q_D(const HbDatePickerView);
    QSizeF sh=HbAbstractItemView::sizeHint(which,constraint);
    qreal mheight =(static_cast<HbDatePickerContainer*>(d->mContainer))->d_func()->getSmallestItemHeight();
    switch(which) {
        case Qt::MinimumSize:
            sh = QSizeF(sh.width(),HB_DATEPICKERVIEW_PREFERRED_ITEMS*mheight);
            break;
        case Qt::PreferredSize:
            sh = QSizeF(sh.width(),HB_DATEPICKERVIEW_PREFERRED_ITEMS*mheight);
            sh.setWidth(HbWidget::sizeHint(which, constraint).width());
            break;
        case Qt::MaximumSize:
            break;
        default:
            qWarning("HbTumbleView::sizeHint(): unknown 'which'");
            break;
    }
    return sh;
}
void HbDatePickerView::setSelected(int row)
{
    Q_D(HbDatePickerView);
    if(model()){
        setCurrentIndex(model()->index(row,0));
        d->viewStates &= ~HbDatePickerViewPrivate::ScrollStarted;
        scrollTo(model()->index(row,0),PositionAtCenter);
        //scrolling will not be started all the time so we need to emit signal
        //here in those cases.
        if(!d->viewStates.testFlag(HbDatePickerViewPrivate::ScrollStarted)){
            emit itemSelected(row);
            if(scene()){
                d->setItemState();
            }
        }
    }
}

int HbDatePickerView::selected() const
{
    return currentIndex().row();
}

void HbDatePickerView::setLoopingEnabled(bool looping)
{
    Q_UNUSED(looping)
}
bool HbDatePickerView::isLoopinEnabled()const
{
    return true;
}
void HbDatePickerViewPrivate::createPrimitives()
{
    Q_Q(HbDatePickerView);

    //this is the highlight which is placed at center
    if(!mHighlight) {
         mHighlight = q->style()->createPrimitive(HbStyle::PT_FrameItem,"highlight",q);
    }

    if(!mDivider) {
        mDivider = q->style()->createPrimitive(HbStyle::PT_FrameItem,"separator",q);
    }

    if(!mBackground){
        mBackground = q->style()->createPrimitive(HbStyle::PT_FrameItem, "background", q);
        mBackground->hide();
    }

    if(!mFrame){
        mFrame = q->style()->createPrimitive(HbStyle::PT_FrameItem, "frame", q);
        mFrame->hide();
    }
}
void HbDatePickerView::updatePrimitives()
{
    Q_D(HbDatePickerView);
    HbAbstractItemView::updatePrimitives();

    if(d->mHighlight) {
        HbStyleFramePrimitiveData data;
        initPrimitiveData(&data,d->mHighlight);
        style()->updatePrimitive(d->mHighlight,&data,this);
    }

    if(d->mDivider) {
        HbStyleFramePrimitiveData data;
        initPrimitiveData(&data,d->mDivider);
        style()->updatePrimitive(d->mDivider,&data,this);
    }
    
    if(d->mBackground) {
        HbStyleFramePrimitiveData data;
        initPrimitiveData(&data, d->mBackground);
        style()->updatePrimitive(d->mBackground, &data, this);
    } 
    
    if(d->mFrame) {
        HbStyleFramePrimitiveData data;
        initPrimitiveData(&data, d->mFrame);
        style()->updatePrimitive(d->mFrame, &data, this);
    }
}
void HbDatePickerView::initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive)
{
    HbWidget::initPrimitiveData(primitiveData, primitive);
    QString itemName = HbStyle::itemName(primitive);

    if(itemName == QLatin1String("highlight")) {
        HbStyleFramePrimitiveData *frameItem  = hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData);
        frameItem->frameGraphicsName= QLatin1String("qtg_fr_tumbler_highlight_pri");
        frameItem->frameType = HbFrameDrawer::ThreePiecesHorizontal;
        (const_cast<QGraphicsObject *> (primitive))->setZValue(-1);
    }

    if(itemName == QLatin1String("separator")) {
        HbStyleFramePrimitiveData *frameItem  = hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData);
        frameItem->frameGraphicsName= QLatin1String("qtg_graf_tumbler_divider");
        frameItem->frameType = HbFrameDrawer::OnePiece;
        (const_cast<QGraphicsObject *> (primitive))->setZValue(2);
    }
    
    if(itemName == QLatin1String("background")) {
        HbStyleFramePrimitiveData *frameItem  = hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData);
        frameItem->frameGraphicsName= QLatin1String("qtg_fr_tumbler_bg");
        frameItem->frameType = HbFrameDrawer::NinePieces;
        (const_cast<QGraphicsObject *> (primitive))->setZValue(-5);
    }
    
    if(itemName == QLatin1String("frame")) {
        HbStyleFramePrimitiveData *frameItem  = hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData);
        frameItem->frameGraphicsName= QLatin1String("qtg_fr_tumbler_overlay");
        frameItem->frameType = HbFrameDrawer::NinePieces;
        (const_cast<QGraphicsObject *> (primitive))->setZValue(1);
    }

}

void HbDatePickerView::gestureEvent(QGestureEvent *event)
{
    Q_D(HbDatePickerView);
    if(QTapGesture *gesture = static_cast<QTapGesture*>(event->gesture(Qt::TapGesture))){
        switch(gesture->state()){
            case Qt::GestureStarted:
            d->viewStates |= HbDatePickerViewPrivate::TapStarted;
                break;
            case Qt::GestureCanceled:

                break;
            case Qt::GestureFinished:
                d->slectItemAt(boundingRect().center(),this);
                break;
            default:
                break;

        }
    }
    HbAbstractItemView::gestureEvent(event);
}

void HbDatePickerView::currentIndexChanged(const QModelIndex &current, const QModelIndex &previous)
{
    HbAbstractItemView::currentIndexChanged(current,previous);
}

void HbDatePickerView::orientationAboutToBeChanged()
{
    Q_D(HbDatePickerView);
    d->mVisibleIndex = currentIndex();
}


#include "moc_hbdatepickerview_p.cpp"
