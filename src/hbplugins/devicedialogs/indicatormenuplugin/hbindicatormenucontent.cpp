/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbPlugins module of the UI Extensions for Mobile.
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

#include <hbstyleoptionindicatormenu.h>
#include <hbindicatorinterface.h>
#include "hbindicatormenucontent_p.h"
#include "hbindicatormenuclock_p.h"
#include "hbindicatormenudate_p.h"

static const int ListWidgetItemIndicatorTypeRole = Hb::UserRole;
static const int DefaultItemsVisible = 6;

HbIndicatorListItem::HbIndicatorListItem(QGraphicsItem *parent) :
        HbListViewItem(parent)
{
    setObjectName("indicatorListItem");
}

HbAbstractViewItem *HbIndicatorListItem::createItem()
{
    HbIndicatorListItem* item = new HbIndicatorListItem(*this);
    item->setObjectName("indicatorListItem");
    item->setContentsMargins(0,0,0,0);
    return item;
}

void HbIndicatorListItem::updateChildItems()
{
    QModelIndex index(modelIndex());
    bool itemContainsLink = false;
    HbIndicatorInterface *indicator =
            HbIndicatorMenuContent::indicatorFromIndex(index);
    if (indicator && indicator->interactionTypes().testFlag(
            HbIndicatorInterface::InteractionActivated)){
        itemContainsLink = true;
    }
    setProperty("link", itemContainsLink);
    HbListViewItem::updateChildItems();
}

IndicatorList::IndicatorList(HbIndicatorMenuContent *content) :
        HbListView(content), mContent(content), mUpdateListSize(false)
{
    setItemRecycling(true);
    setUniformItemSizes(true);
}

void IndicatorList::rowsInserted(
        const QModelIndex &parent, int start, int end)
{
    HbListView::rowsInserted(parent, start, end);
    if (static_cast<QGraphicsItem*>(this)->isVisible()) {
        updateGeometry();
        mUpdateListSize = true;
    }
}

void IndicatorList::rowsRemoved(
        const QModelIndex &parent,int start,int end)
{
    HbListView::rowsRemoved(parent, start, end);
    if (static_cast<QGraphicsItem*>(this)->isVisible()) {
        updateGeometry();
        mUpdateListSize = true;
    }
}

void IndicatorList::dataChanged(const QModelIndex &topLeft,
                                const QModelIndex &bottomRight)
{
    HbListView::dataChanged(topLeft, bottomRight);
    if (static_cast<QGraphicsItem*>(this)->isVisible()) {
        updateGeometry();
        mUpdateListSize = true;
    }
}

QSizeF IndicatorList::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    switch(which) {
    case Qt::PreferredSize:
        if (mUpdateListSize) {
            mUpdateListSize = false;
            HbAbstractViewItem *item = 0;

            const int indicators = model()->rowCount();
            //get the height of one of the listitems.
            for (int i = 0; i < indicators && !item; ++i) {
                item = viewItem(i);
            }
            if (item) {
                QSizeF itemSize(item->effectiveSizeHint(Qt::PreferredSize));
                qreal height = qMin(DefaultItemsVisible, indicators)
                               * itemSize.height();
                mSize = QSizeF(itemSize.width(), height);
            } else {
                mSize.setWidth(0);
                mSize.setHeight(0);
            }
        }
        return mSize;
    default:
        break;
    }

    return HbListView::sizeHint(which, constraint);
}

void IndicatorList::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    updateGeometry();
    mUpdateListSize = true;
}

/*!
    Constructs a new HbItemContainer with \a parent.
*/
HbIndicatorMenuContent::HbIndicatorMenuContent(QGraphicsItem *parent) :
    HbWidget(parent)
{
    for (int i = 0; i < IndicatorTypes; ++i) {
        mGroupTypeIndeces[i] = 0;
    }

    mHeaderBackground = style()->createPrimitive(HbStyle::P_Popup_heading_frame, this);

    mClock = new HbIndicatorMenuClock(this);
    mDate = new HbIndicatorMenuDate(this);
    connect(mClock, SIGNAL(dateChanged()), mDate, SLOT(updateDate()));

    mIndicatorList = new IndicatorList(this);
    mIndicatorList->setItemPrototype(new HbIndicatorListItem(this));
    mIndicatorList->setModel(&indicatorModel);
    connect(mIndicatorList, SIGNAL(activated(QModelIndex)),
            this, SLOT(itemActivated(QModelIndex)));
    connect(mIndicatorList, SIGNAL(scrollingStarted()),
            this, SLOT(indicatorlist_scrollingStarted()));
    connect(mIndicatorList, SIGNAL(scrollingEnded()),
            this, SLOT(indicatorlist_scrollingEnded()));

    HbStyle::setItemName(mHeaderBackground, "background");
    HbStyle::setItemName(mClock, "clock");
    HbStyle::setItemName(mDate, "date");
    HbStyle::setItemName(mIndicatorList, "list");

    updatePrimitives();
}

HbIndicatorMenuContent::~HbIndicatorMenuContent()
{
}


int HbIndicatorMenuContent::indicatorCount() const
{
     return indicatorModel.rowCount();
}

void HbIndicatorMenuContent::updatePrimitives()
{
    HbStyleOptionPopup option;
    if (mHeaderBackground->hasFocus()) {
        option.headingMode = QIcon::Selected;
    } else if (mHeaderBackground->isSelected()) {
        option.headingMode = QIcon::Active;
    } else {
        option.headingMode = QIcon::Normal;
    }
    style()->updatePrimitive(mHeaderBackground, HbStyle::P_Popup_heading_frame, &option);
}

void HbIndicatorMenuContent::itemActivated(const QModelIndex &modelIndex)
{
    HbIndicatorInterface *indicator = indicatorFromIndex(modelIndex);
    if (indicator && indicator->interactionTypes().testFlag(
            HbIndicatorInterface::InteractionActivated)) {
        indicator->handleInteraction(HbIndicatorInterface::InteractionActivated);
        emit aboutToClose();
    }
}

void HbIndicatorMenuContent::indicatorsActivated(
        QList<HbIndicatorInterface*> activatedIndicators)
{
    foreach(HbIndicatorInterface *indicator, activatedIndicators) {
        indicatorActivated(indicator);
    }
}

void HbIndicatorMenuContent::indicatorActivated(
        HbIndicatorInterface *activatedIndicator)
{
    QStandardItem *item = new QStandardItem();
    HbIndicatorInterface::Category category = activatedIndicator->category();

    int index = (category < IndicatorTypes) ? mGroupTypeIndeces[category]
                                         : indicatorModel.rowCount();

    indicatorModel.insertRow(index, item);
    setData(activatedIndicator, item->index());

    //update indices.
    for(int i = category+1; i < IndicatorTypes;++i){
        mGroupTypeIndeces[i]++;
    }

    //store the indicator pointer inside indicator model.
    QObject *ind_ptr = activatedIndicator;
    item->setData(QVariant::fromValue(ind_ptr),
                  ListWidgetItemIndicatorTypeRole);

    //connect indicator's update signal
    QObject::connect(activatedIndicator, SIGNAL(dataChanged()),
                     this, SLOT(indicatorUpdated()));
    repolish();
}

void HbIndicatorMenuContent::setData(
        HbIndicatorInterface *source,
        const QModelIndex &modelIndex)
{
    QString primaryText = source->indicatorData(
        HbIndicatorInterface::PrimaryTextRole).toString();
    QString secondaryText = source->indicatorData(
        HbIndicatorInterface::SecondaryTextRole).toString();
    QString iconPath = source->indicatorData(
        HbIndicatorInterface::DecorationNameRole).toString();

    QStringList texts;
    if (secondaryText.isEmpty()) {
        secondaryText = " "; //always reserve space for secondary text. Item size should be uniform.
    }
    texts << primaryText << secondaryText;

    indicatorModel.setData(modelIndex, texts, Qt::DisplayRole);
    if (!iconPath.isEmpty()) {
        HbIcon icon(iconPath);
        indicatorModel.setData(modelIndex, icon, Qt::DecorationRole);
    }
}

void HbIndicatorMenuContent::indicatorRemoved(
        HbIndicatorInterface *indicatorRemoved)
{
    int index = listIndexFromIndicator(indicatorRemoved);
    if (index >= 0) {
        indicatorModel.removeRow(index);
    }
    //update indices.
    for(int i = indicatorRemoved->category()+1; i < IndicatorTypes;++i){
        mGroupTypeIndeces[i]--;
    }
    repolish();
}

void HbIndicatorMenuContent::initStyleOption(
        HbStyleOptionIndicatorMenu *option) const
{
    HbWidget::initStyleOption(option);
}

//data changed inside indicator.
void HbIndicatorMenuContent::indicatorUpdated()
{
    HbIndicatorInterface *senderIndicator =
            qobject_cast<HbIndicatorInterface*>(sender());
    if (senderIndicator) {
        int index = listIndexFromIndicator(senderIndicator);
        if (index >= 0) {
            setData(senderIndicator, indicatorModel.item(index)->index());
        }
    }
}

void HbIndicatorMenuContent::indicatorlist_scrollingStarted()
{
    emit userActivityStarted();
}

void HbIndicatorMenuContent::indicatorlist_scrollingEnded()
{
    emit userActivityEnded();
}

HbIndicatorInterface *HbIndicatorMenuContent::indicatorFromIndex(
        const QModelIndex &modelIndex)
{
    QObject *ind_ptr =
        modelIndex.data(ListWidgetItemIndicatorTypeRole).value<QObject*>();
    return qobject_cast<HbIndicatorInterface*>(ind_ptr);
}

int HbIndicatorMenuContent::listIndexFromIndicator(
        HbIndicatorInterface *indicator) {

    int index = -1;
    int rowCount = mIndicatorList->model()->rowCount();
    for(int i = 0; i < rowCount; ++i) {
        HbIndicatorInterface *itemIndicator =
            indicatorFromIndex(indicatorModel.item(i)->index());

        if (itemIndicator && itemIndicator == indicator) {
            index = i;
            break;
        }
    }
    return index;
}

