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

#include "hblistview_p.h"
#include "hblistitemcontainer_p.h"
#include "hblistitemcontainer_p_p.h"
#include "hbmodeliterator.h"

#include <hbtumbleview.h>
#include <hbtumbleviewitem.h>
#include <hbevent.h>
#include <hbstyleoption.h>

#include <QGraphicsSceneMouseEvent>
#include <QStringListModel>

#define HB_TUMBLE_ITEM_ANIMATION_TIME 500
#define HB_TUMBLE_PREFERRED_ITEMS 3
#define DELAYED_SELECT_INTERVAL 100

#define HBTUMBLE_DEBUG
#ifdef HBTUMBLE_DEBUG
#include <QDebug>
#endif

class HbTumbleViewItemContainerPrivate;

class HbTumbleViewItemContainer:public HbListItemContainer
{
    Q_DECLARE_PRIVATE(HbTumbleViewItemContainer)
public:
    HbTumbleViewItemContainer(QGraphicsItem* parent = 0);
    QPointF recycleItems(const QPointF &delta);

    void setLoopingEnabled(bool looping) ;
    bool isLoopingEnabled() const ;
};

class HbTumbleViewItemContainerPrivate:public HbListItemContainerPrivate
{
    Q_DECLARE_PUBLIC(HbTumbleViewItemContainer)
public:
    HbTumbleViewItemContainerPrivate();
    QPointF recycleItems(const QPointF &delta);
    HbAbstractViewItem *shiftDownItem(QPointF& delta);
    HbAbstractViewItem *shiftUpItem(QPointF& delta);

    bool mIsLooped;
};

class HbTumbleViewPrivate : public HbListViewPrivate
{
    Q_DECLARE_PUBLIC(HbTumbleView)

public:
    HbTumbleViewPrivate();

    qreal topBoundary();//virtual functions
    qreal bottomBoundary();
    virtual void updateScrollMetrics();
    virtual void scrollTo(const QModelIndex &index, HbAbstractItemView::ScrollHint hint);

    void init(QAbstractItemModel *model);
    void calculateItemHeight();

    void selectMiddleItem();

    void createPrimitives();

    void delayedSelectCurrent(const QModelIndex& index);

    void _q_scrollingStarted();//private slot
    void _q_scrollingEnded();//private slot
    void _q_delayedSelectCurrent();//private slot

    void setPressedState(HbAbstractViewItem *item);

private:
    qreal mHeight;
    QPointer<HbAbstractViewItem> mPrevSelectedItem;
    bool mInternalScroll;
    bool mStartup;//needed for layout request

    //geometry prob, some how loop setGeometry call is happening
    QRectF mPrevSetGeometryRect;

    QModelIndex mDelayedSelectIndex;
    QTimer mDelayedSelectTimer;

    //primitives
    QGraphicsItem   *mBackground;
    QGraphicsItem   *mFrame;//overlay
    QGraphicsItem   *mHighlight;
    int             mSelected;
    bool mNeedScrolling;
    QGraphicsItem   *mDivider;
};


HbTumbleViewItemContainer::HbTumbleViewItemContainer(QGraphicsItem* parent )
    :HbListItemContainer(*(new HbTumbleViewItemContainerPrivate),parent)
{

}

QPointF HbTumbleViewItemContainer::recycleItems(const QPointF &delta)
{
    Q_D(HbTumbleViewItemContainer);

    if (d->mPrototypes.count() != 1) {
        return delta;
    }

    QRectF viewRect(d->itemBoundingRect(d->mItemView));
    viewRect.moveTopLeft(viewRect.topLeft() + delta);

    int firstVisibleBufferIndex = -1;
    int lastVisibleBufferIndex = -1;
    d->firstAndLastVisibleBufferIndex(firstVisibleBufferIndex, lastVisibleBufferIndex, viewRect, false);

    int hiddenAbove = firstVisibleBufferIndex;
    int hiddenBelow = d->mItems.count() - lastVisibleBufferIndex - 1;

    if (d->mItems.count()
        && (firstVisibleBufferIndex == -1 || lastVisibleBufferIndex == -1)) {
        // All items out of sight.
        if (d->itemBoundingRect(d->mItems.first()).top() < 0) {
            // All items above view.
            hiddenAbove = d->mItems.count();
            hiddenBelow = 0;
        } else {
            // All items below view.
            hiddenAbove = 0;
            hiddenBelow = d->mItems.count();
        }
    }

    QPointF newDelta(delta);

    while (hiddenAbove > hiddenBelow + 1) {
        HbAbstractViewItem *item = d->shiftDownItem(newDelta);
        if (!item){
            break;
        }

        if (!d->visible(item, viewRect)) {
            hiddenBelow++;
        }
        hiddenAbove--;
    }

    while (hiddenBelow > hiddenAbove + 1) {
        HbAbstractViewItem *item = d->shiftUpItem(newDelta);
        if (!item) {
            break;
        }

        if (!d->visible( item, viewRect)) {
            hiddenAbove++;
        }
        hiddenBelow--;
    }

    return newDelta;
}
void HbTumbleViewItemContainer::setLoopingEnabled(bool looped) {
    Q_D(HbTumbleViewItemContainer);
    d->mIsLooped = looped;
}
bool HbTumbleViewItemContainer::isLoopingEnabled() const {
    Q_D(const HbTumbleViewItemContainer);
    return d->mIsLooped;
}

HbTumbleViewItemContainerPrivate::HbTumbleViewItemContainerPrivate()
    : mIsLooped(false) 
{ 
}


HbAbstractViewItem *HbTumbleViewItemContainerPrivate::shiftDownItem(QPointF& delta)
{
    Q_Q(HbTumbleViewItemContainer);

    HbAbstractViewItem *item = 0;
    HbAbstractViewItem *lastItem = mItems.last();

    QModelIndex nextIndex = mItemView->modelIterator()->nextIndex(lastItem->modelIndex());
    if(mIsLooped && (!nextIndex.isValid())){
       nextIndex = mItemView->model()->index(0,0);
    }
    if (nextIndex.isValid()) {
        item = mItems.takeFirst();

        q->itemRemoved(item);

        delta.setY(delta.y() - item->size().height());

        mItems.append(item);

        q->setItemModelIndex(item, nextIndex);

        q->itemAdded(mItems.count() - 1, item);
    }

    return item;
}

HbAbstractViewItem *HbTumbleViewItemContainerPrivate::shiftUpItem(QPointF& delta)
{
    Q_Q(HbTumbleViewItemContainer);

    HbAbstractViewItem *item = 0;
    HbAbstractViewItem *firstItem = mItems.first();

    QModelIndex previousIndex = mItemView->modelIterator()->previousIndex(firstItem->modelIndex());
    if(mIsLooped && !previousIndex.isValid()){
        previousIndex = mItemView->model()->index(mItemView->model()->rowCount()-1,0);
    }
    if (previousIndex.isValid()) {
        item = mItems.takeLast();

        q->itemRemoved(item);

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

        q->itemAdded(0, item);
    }
    return item;
}

HbTumbleViewPrivate::HbTumbleViewPrivate()
    :HbListViewPrivate()
    ,mHeight(10.0)
    ,mPrevSelectedItem(0)
    ,mInternalScroll(false)
    ,mStartup(true)
    ,mDelayedSelectIndex()
    ,mDelayedSelectTimer(0)
    ,mBackground(0)
    ,mFrame(0)
    ,mHighlight(0)
    ,mSelected(-1)
    ,mNeedScrolling(true)
    ,mDivider(0)
{
}

qreal HbTumbleViewPrivate::topBoundary()
{
    //top boundary and bottom boundary is different for tumble view
    //it is half item less than the middle of the view
    Q_Q( HbTumbleView );
    return (-(q->boundingRect().height()-mHeight)/2);
}

qreal HbTumbleViewPrivate::bottomBoundary()
{
    Q_Q ( HbTumbleView);    
    return mContents->boundingRect().height()-((q->boundingRect().height()+mHeight)/2);
} 


void HbTumbleViewPrivate::init(QAbstractItemModel *model)
{
    Q_Q(HbTumbleView);
    mNeedScrolling = true;
    //list settings
    HbTumbleViewItem *proto = new HbTumbleViewItem();
    proto->setFlag(QGraphicsItem::ItemIsFocusable,false);        
    q->setModel(model,proto);
    q->setSelectionMode(q->HbAbstractItemView::SingleSelection);
    q->setLongPressEnabled(false);
    q->setUniformItemSizes(true);
    q->setEnabledAnimations(HbAbstractItemView::TouchDown);

    //scroll area settings
    q->setClampingStyle(HbScrollArea::BounceBackClamping);
    q->setVerticalScrollBarPolicy(HbScrollArea::ScrollBarAlwaysOff);
    q->setHorizontalScrollBarPolicy(HbScrollArea::ScrollBarAlwaysOff);
    q->setFrictionEnabled(true);
    mDelayedSelectTimer.setSingleShot(true);
    bool b = q->connect(q,SIGNAL(scrollingStarted()),q,SLOT(_q_scrollingStarted()));
    Q_ASSERT(b);
    b = q->connect(q,SIGNAL(scrollingEnded()),q,SLOT(_q_scrollingEnded()));
    Q_ASSERT(b);
    b = q->connect(&mDelayedSelectTimer,SIGNAL(timeout()),q,SLOT(_q_delayedSelectCurrent()));
    Q_UNUSED(b);
    createPrimitives();
}

void HbTumbleViewPrivate::selectMiddleItem()
{
    Q_Q(HbTumbleView);
    //scroll little amount so item settle's in
    if(!q->scene()) {
        return;
    }
    QPointF centerPt = q->mapToScene(q->boundingRect().center());
    HbAbstractViewItem *item = itemAt(centerPt);

    if(item) {
#ifdef HBTUMBLE_DEBUG  
    qDebug() << "HbTumbleViewPrivate::selectMiddleItem - " << item->modelIndex().row() ;
#endif
            delayedSelectCurrent(item->modelIndex());
            mSelected = item->modelIndex().row();
        }
    }

void HbTumbleViewPrivate::scrollTo(const QModelIndex &index, HbAbstractItemView::ScrollHint hint)
{
    Q_Q(HbTumbleView);
#ifdef HBTUMBLE_DEBUG  
    qDebug() << "HbTumbleViewPrivate::scrollTo(" << index.row() << "," << hint << " )";
#endif
    if(!q->scene()) {
        return;
    }
    
    HbListViewPrivate::scrollTo(index, hint);

    HbAbstractViewItem *item = mContainer->itemByIndex(index);
    if(item) {
        setPressedState(item); 
    } 
#ifdef HBTUMBLE_DEBUG  
    else {
        qDebug() << "HbTumbleViewPrivate::scrollTo(" << index.row() << ",failed to get itembyindex";
    }
#endif
}

void HbTumbleView::scrollTo(const QModelIndex &index, ScrollHint)
{
#ifdef HBTUMBLE_DEBUG  
    qDebug() << "HbTumbleView::scrollTo(" << index.row() << ", )";
#endif
    HbListView::scrollTo(index, PositionAtCenter);
}

void HbTumbleViewPrivate::createPrimitives()
{
    Q_Q(HbTumbleView);

    //this is the highlight which is placed at center
    if(!mHighlight) {
        mHighlight = q->style()->createPrimitive(HbStyle::P_TumbleView_highlight,q);
        q->style()->setItemName(mHighlight,"highlight");
    }
    if(!mDivider){
        mDivider = q->style()->createPrimitive(HbStyle::P_DateTimePicker_separator,q);
        q->style()->setItemName(mDivider,"separator");
        mDivider->hide();
    }

}


void HbTumbleViewPrivate::calculateItemHeight()
{
    Q_Q(HbTumbleView);
    if (!q->items().isEmpty()) {
        //Let's create a temporary item and take the height for the size hint
        HbAbstractViewItem *tempItem = q->itemPrototypes().first()->createItem();
        tempItem->setModelIndex(mModelIterator->nextIndex(QModelIndex()));

        qreal oldHeight = mHeight;
        mHeight = tempItem->effectiveSizeHint(Qt::PreferredSize).height();

        delete tempItem;

        if (mHeight != oldHeight) {
            q->updateGeometry();
        }
    }
}

void HbTumbleViewPrivate::updateScrollMetrics()
{
    mAbleToScrollY = true;
}

/*!
    @proto
    \class HbTumbleView 
    \this is a tumbler widget which lets the user select alphanumeric values from a predefined list of 
    values via vertical flicking and dragging. Typically widgets such as date picker and time picker use the 
    Tumbler. The Tumbler could also be used to change other values such as number code sequence, 
    landmark coordinates, country selection, and currency.

    Only strings can be accepted as HbTumbleView's items.

    \this can be used like this:
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,52}
*/

/*!
    \fn void itemSelected(int index)

    This signal is emitted when an item is selected in date time picker.
    \param index  selected item.

*/

/*!
    HbTumbleView's default constructor.

    \param parent item to set as parent.
*/
HbTumbleView::HbTumbleView(QGraphicsItem *parent)
    :HbListView(*new HbTumbleViewPrivate,
        new HbTumbleViewItemContainer,
        parent)
{
    Q_D(HbTumbleView);
    d->q_ptr = this;
    d->init(new QStringListModel(this));

}

/*!
    HbTumbleView's constructor.

    \param list to be set as data to QStringListModel.
    \parent item to set as parent.
*/
HbTumbleView::HbTumbleView(const QStringList &list,QGraphicsItem *parent)
    :HbListView(*new HbTumbleViewPrivate,
        new HbTumbleViewItemContainer,
        parent)
{
    Q_D(HbTumbleView);
    d->q_ptr = this;
    d->init(new QStringListModel(list,this));

    d->calculateItemHeight();
}

/*!
    Protected constructor.
*/
HbTumbleView::HbTumbleView(HbTumbleViewPrivate &dd, QGraphicsItem *parent):
    HbListView(dd,
        new HbTumbleViewItemContainer,
        parent)
{
    Q_D(HbTumbleView);
    d->q_ptr = this;
    d->init(new QStringListModel(this));

    d->calculateItemHeight();
}

/*!
   Destructor
*/
HbTumbleView::~HbTumbleView()
{
}

/*!
    Sets the HbTumbleView's items to the given string \a list.

    \param list Items to be set as tumble view's model.

*/
void HbTumbleView::setItems(const QStringList &list)
{
    Q_D(HbTumbleView);
    QStringListModel *stringListModel = qobject_cast<QStringListModel *>(model());
    if (stringListModel) {
        stringListModel->setStringList(list);
        d->calculateItemHeight();
    }
    updatePrimitives();
}

/*!
    Returns items in QStringList format.

    \return list of items in tumbleview's model in QStringList format.
*/
QStringList HbTumbleView::items() const
{
    QStringListModel *stringListModel = qobject_cast<QStringListModel *>(model());
    if(stringListModel) {
        return stringListModel->stringList();
    }
    return QStringList();
}

/*!
    Sets the selection to the item at \a index.

    \param index to be selected in the tumble view.

*/
void HbTumbleView::setSelected(int index)
{
#ifdef HBTUMBLE_DEBUG
    qDebug() << "HbTumbleView::setSelected(" << index << ");" ;
#endif
    Q_D(HbTumbleView);
    d->mSelected = index;

    QModelIndex modelIndex = d->mModelIterator->index(index, rootIndex());
    if(modelIndex.isValid()) {
        d->delayedSelectCurrent(modelIndex);
        emitActivated(modelIndex);
    } 
}
 
/*!
    Returns the index of the current selected item in integer format.

    \return current index selected in tumble view.
*/
int HbTumbleView::selected() const
{
    return currentIndex().row();
}

/*!

    \deprecated HbTumbleView::primitive(HbStyle::Primitive)
        is deprecated.

    \reimp
*/
QGraphicsItem *HbTumbleView::primitive(HbStyle::Primitive id) const
{
    Q_D(const HbTumbleView);

    switch(id) {
        case HbStyle::P_TumbleView_background:
            return d->mBackground;
        case HbStyle::P_TumbleView_frame:
            return d->mFrame;
        case HbStyle::P_TumbleView_highlight:
            return d->mHighlight;
        default:
            return HbListView::primitive(id);
    }
}

/*!
    \reimp
*/
QGraphicsItem *HbTumbleView::primitive(const QString &itemName) const
{
    return HbListView::primitive(itemName);
}

/*!
    \reimp
*/
void HbTumbleView::currentIndexChanged(const QModelIndex &current, const QModelIndex &previous)
{

    Q_D(HbTumbleView);
    HbListView::currentIndexChanged(current,previous);
    if(d->mNeedScrolling && current.isValid()){
        //scrolling
        d->mInternalScroll = true;
        scrollTo(current,PositionAtCenter);
        emit itemSelected(current.row());
        d->mInternalScroll = false;

        //below code should be after scrolling. setModelIndexes should have finished.
        HbAbstractViewItem *item=d->mContainer->itemByIndex(current);
        if(item) {
            d->setPressedState(item);
        } 
    }
}

/*!
    \reimp
*/
void HbTumbleView::updatePrimitives()
{                   
    Q_D(HbTumbleView);

    HbStyleOption opt;
    initStyleOption(&opt);

    if(d->mBackground) {
        style()->updatePrimitive(d->mBackground,HbStyle::P_TumbleView_background,&opt);
    }
    if(d->mFrame) {
        style()->updatePrimitive(d->mFrame,HbStyle::P_TumbleView_frame,&opt);
    } 
    if(d->mHighlight) {
        style()->updatePrimitive(d->mHighlight,HbStyle::P_TumbleView_highlight,&opt);
    }
    if(d->mDivider){
        style()->updatePrimitive(d->mDivider, HbStyle::P_DateTimePicker_separator, &opt);
    }
    HbListView::updatePrimitives();

}

/*!
   \reimp
*/
QVariant HbTumbleView::itemChange(GraphicsItemChange change,const QVariant &value)
{
    if((change==QGraphicsItem::ItemVisibleHasChanged) && (value.toBool()==true)) {
        QModelIndex selected=currentIndex();
        scrollTo(selected,PositionAtCenter);
    }
    return HbListView::itemChange(change,value);
}

/*!
    \reimp
*/
void HbTumbleView::rowsInserted(const QModelIndex &parent,int start,int end)
{
    HbListView::rowsInserted(parent,start,end);
    scrollTo(currentIndex(),PositionAtCenter);
}

void HbTumbleViewPrivate::_q_delayedSelectCurrent()
{
    Q_Q(HbTumbleView);
    if(!mIsAnimating && !mIsScrolling) {
         if(mDelayedSelectIndex == q->currentIndex()){
             HbAbstractViewItem *item =q->itemByIndex(mDelayedSelectIndex);
             QPointF delta = pixelsToScroll(item,HbAbstractItemView::PositionAtCenter );
             QPointF newPos = -mContainer->pos() + delta;
             checkBoundaries(newPos);
             scrollByAmount(newPos - (-mContents->pos()));
         }
         else{
            q->setCurrentIndex(mDelayedSelectIndex);
        }
    }
}

/*!
    \reimp
*/
void HbTumbleView::rowsAboutToBeInserted(const QModelIndex &index, int start, int end)
{
    HbListView::rowsAboutToBeInserted(index,start,end);
}

/*!
    \reimp
*/
bool HbTumbleView::event(QEvent *e)
{
    Q_D(HbTumbleView);
    if(e->type() == HbEvent::ChildFocusOut) {
        return true;
    }
    else if(d->mStartup && (e->type() == QEvent::LayoutRequest) ) {
        if(d->mSelected < 0) {
            d->selectMiddleItem();
        }
        d->mStartup = false;
    }
    return HbListView::event(e);
}

/*!
    \reimp
*/
void HbTumbleView::setGeometry(const QRectF &rect)
{
    Q_D(HbTumbleView);
    if(d->mPrevSetGeometryRect == rect) {
#ifdef HBTUMBLE_DEBUG
    qDebug() << "HbTumbleView::setGeometry equal rect";
#endif
        return;
    }
#ifdef HBTUMBLE_DEBUG
    qDebug() << "HbTumbleView::setGeometry not equal rect";
#endif
    d->mPrevSetGeometryRect = rect;
    HbListView::setGeometry(rect);

    d->calculateItemHeight();
    QModelIndex selected = currentIndex();
    if(selected.isValid() && !d->mIsAnimating) {
        scrollTo(selected,PositionAtCenter);
    }
    updatePrimitives();
}

/*!
    Sets the looping enabled flag to eith true or false, which makes the tumbleview to scroll in a circular way.

    \param looped flag to enable curcular view.

    \sa isLoopingEnabled
*/
void HbTumbleView::setLoopingEnabled(bool looped) 
{
    Q_D(HbTumbleView);
    HbTumbleViewItemContainer *container=static_cast<HbTumbleViewItemContainer*>(d->mContainer);
    if(container) {
        container->setLoopingEnabled(looped);
    }
}

/*!
    Returns looping enabled flag.

    \return looping enabled flag.

    \sa setLoopingEnabled

*/
bool HbTumbleView::isLoopingEnabled() const
{
    Q_D(const HbTumbleView);
    HbTumbleViewItemContainer *container=static_cast<HbTumbleViewItemContainer*>(d->mContainer);
    if(container) {
        return container->isLoopingEnabled();
    }
    return false;
}

/*!
    \reimp
*/
void HbTumbleView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
#ifdef HBTUMBLE_DEBUG
    qDebug() << "HbTumbleView::mousePressEvent";
#endif
    HbListView::mousePressEvent(event);
}

/*!
    \reimp
*/
void HbTumbleView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
#ifdef HBTUMBLE_DEBUG
    qDebug() << "HbTumbleView::mouseReleaseEvent";
#endif
    HbListView::mouseReleaseEvent(event);
}

/*!
    \reimp
*/
QSizeF HbTumbleView::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(constraint);
    Q_D(const HbTumbleView); 
    QSizeF sh=HbListView::sizeHint(which,constraint);
    switch(which) {
        case Qt::MinimumSize:
			sh = QSizeF(sh.width(),HB_TUMBLE_PREFERRED_ITEMS*d->mHeight);
            break;
        case Qt::PreferredSize:
            sh = QSizeF(sh.width(),HB_TUMBLE_PREFERRED_ITEMS*d->mHeight);
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
 
void HbTumbleViewPrivate::_q_scrollingStarted()
{
#ifdef HBTUMBLE_DEBUG
    qDebug() << "HbTumbleViewPrivate::_q_scorllingStarted() :" << mInternalScroll;
#endif
    if(mInternalScroll) {
        return;
    }

    setPressedState(0);//disable current selected item
}

void HbTumbleViewPrivate::_q_scrollingEnded()
{
#ifdef HBTUMBLE_DEBUG
    qDebug() << "HbTumbleViewPrivate::_q_scorllingEnded() :" << mInternalScroll;
#endif
    Q_Q(HbTumbleView);

    if(!q->scene()) {
        return;
    }
     if(mInternalScroll || mStartup) {
        return;
    }

    QPointF pt = q->mapToScene(q->boundingRect().center());
    HbAbstractViewItem *centerItem=itemAt(pt);
    if(centerItem) {
        setPressedState(centerItem);

        if(centerItem->modelIndex().isValid()) {
            delayedSelectCurrent(centerItem->modelIndex());
            q->emitActivated(centerItem->modelIndex());
        } 
    }
}
void HbTumbleViewPrivate::setPressedState(HbAbstractViewItem *item)
{
    //set state
    if(mPrevSelectedItem) {
        mPrevSelectedItem->setProperty("state","normal");
    }

    mPrevSelectedItem = item;

    if(mPrevSelectedItem) {
        mPrevSelectedItem->setProperty("state","selected");
    }
}

/*!
    \reimp
*/
void HbTumbleView::rowsAboutToBeRemoved(const QModelIndex &index, int start, int end)
{
    Q_D(HbTumbleView);
    d->mNeedScrolling = false;
    HbListView::rowsAboutToBeInserted(index,start,end);
}

/*!
    \reimp
*/
void HbTumbleView::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_D(HbTumbleView);
    d->mNeedScrolling = true;
    HbListView::rowsRemoved(parent,start,end);
    scrollTo(currentIndex(),PositionAtCenter);
}
void HbTumbleViewPrivate::delayedSelectCurrent(const QModelIndex& index)
{
    mDelayedSelectIndex = index;
    mDelayedSelectTimer.start(DELAYED_SELECT_INTERVAL);
}

#include "moc_hbtumbleview.cpp"
