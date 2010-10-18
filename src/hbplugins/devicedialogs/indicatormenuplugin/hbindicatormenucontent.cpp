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

#include <hbindicatorinterface.h>
#include <hbmainwindow.h>
#include "hbindicatormenucontent_p.h"

static const int ListWidgetItemIndicatorTypeRole = Hb::UserRole;

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

IndicatorList::IndicatorList(HbIndicatorMenuContent *content) :
    HbListView(content), mUpdateListSize(false), mPreferredHeight(-1), mContent(content)
{
    setItemRecycling(true);
    setUniformItemSizes(true);

    qreal text1(0);
    style()->parameter(QLatin1String("hb-param-text-height-primary"), text1);
    qreal text2(0);
    style()->parameter(QLatin1String("hb-param-text-height-secondary"), text2);
    qreal margin(0);
    style()->parameter(QLatin1String("hb-param-margin-gene-middle-horizontal"), margin);
    mPreferredHeight = text1 + text2 + 3 * margin;
}

void IndicatorList::rowsInserted(
        const QModelIndex &parent, int start, int end)
{
    HbListView::rowsInserted(parent, start, end);
    if (mUpdateListSize == false) {
        mUpdateListSize = true;
        updateGeometry();
    }
}

void IndicatorList::rowsRemoved(
        const QModelIndex &parent,int start,int end)
{
    HbListView::rowsRemoved(parent, start, end);
    if (mUpdateListSize == false) {
        mUpdateListSize = true;
        updateGeometry();
    }
}

void IndicatorList::dataChanged(const QModelIndex &topLeft,
                                const QModelIndex &bottomRight)
{
    HbListView::dataChanged(topLeft, bottomRight);
    if (mUpdateListSize == false) {
        mUpdateListSize = true;
        updateGeometry();
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
                QSizeF itemSize(item->effectiveSizeHint(Qt::PreferredSize, QSizeF(-1, mPreferredHeight)));
                int defaultItemsVisible = 6;
                if (mContent->orientation() == Qt::Horizontal) {
                    defaultItemsVisible = 4;
                } 
                qreal height = qMin(defaultItemsVisible, indicators)
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
    if (mUpdateListSize == false) {
        mUpdateListSize = true;
        updateGeometry();
    }
}

/*!
    Constructs a new HbItemContainer with \a parent.
*/
HbIndicatorMenuContent::HbIndicatorMenuContent(QGraphicsItem *parent) :
    HbWidget(parent), mOrientation(Qt::Vertical)
{
    for (int i = 0; i < IndicatorTypes; ++i) {
        mGroupTypeIndeces[i] = 0;
    }

    mIndicatorList = new IndicatorList(this);
    mIndicatorList->setItemPrototype(new HbIndicatorListItem(this));
    mIndicatorList->setModel(&indicatorModel);
    connect(mIndicatorList, SIGNAL(activated(QModelIndex)),
            this, SLOT(itemActivated(QModelIndex)));
    connect(mIndicatorList, SIGNAL(scrollingStarted()),
            this, SLOT(indicatorlist_scrollingStarted()));
    connect(mIndicatorList, SIGNAL(scrollingEnded()),
            this, SLOT(indicatorlist_scrollingEnded()));

    HbStyle::setItemName(mIndicatorList, "list");
}

HbIndicatorMenuContent::~HbIndicatorMenuContent()
{
}

int HbIndicatorMenuContent::indicatorCount() const
{
     return indicatorModel.rowCount();
}

void HbIndicatorMenuContent::handleAboutToShow(HbMainWindow *mainWindow)
{
    if (mainWindow) {
        mOrientation = mainWindow->orientation();
    }

    for (int i = 0; i < mIndicatorList->model()->rowCount(); ++i) {
        HbIndicatorInterface *indicator =
            indicatorFromIndex(indicatorModel.item(i)->index());
        if (indicator) {
            bool refreshed = true;
            try {
                refreshed = indicator->refreshData();
            } catch (const std::bad_alloc &) {
                refreshed = false;
            }
            if (refreshed) {
                setData(indicator, indicatorModel.item(i)->index());
            }
        }
    }
}

Qt::Orientation HbIndicatorMenuContent::orientation() const
{
    return mOrientation;
}

void HbIndicatorMenuContent::updatePrimitives()
{
    repolish();
}

void HbIndicatorMenuContent::itemActivated(const QModelIndex &modelIndex)
{
    HbIndicatorInterface *indicator = indicatorFromIndex(modelIndex);
    if (indicator && indicator->interactionTypes().testFlag(
            HbIndicatorInterface::InteractionActivated)) {
        try {
            indicator->handleInteraction(HbIndicatorInterface::InteractionActivated);
        }  catch (const std::bad_alloc &) {
        }
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

void HbIndicatorMenuContent::indicatorActivated(HbIndicatorInterface *activatedIndicator)
{
    if (!hasMenuData(*activatedIndicator)) {
        return;
    }

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

void HbIndicatorMenuContent::setData(HbIndicatorInterface *source, const QModelIndex &modelIndex)
{
    QString primaryText = source->indicatorData(
        HbIndicatorInterface::PrimaryTextRole).toString();
    QString secondaryText = source->indicatorData(
        HbIndicatorInterface::SecondaryTextRole).toString();
    QString iconPath = source->indicatorData(
        HbIndicatorInterface::DecorationNameRole).toString();

    QStringList texts;

    // List doesn't allow empty text
    static const char empty[] = " ";
    if (primaryText.isEmpty()) {
        primaryText.append(empty);
    }
    // Always reserve space for secondary text. Item size should be uniform.
    if (secondaryText.isEmpty()) {
        secondaryText.append(empty);
    }
    texts << primaryText << secondaryText;

    indicatorModel.setData(modelIndex, texts, Qt::DisplayRole);
    if (!iconPath.isEmpty()) {
        HbIcon icon(iconPath);
        indicatorModel.setData(modelIndex, icon, Qt::DecorationRole);
    }
}

void HbIndicatorMenuContent::indicatorRemoved(HbIndicatorInterface *indicatorRemoved)
{
    int index = listIndexFromIndicator(indicatorRemoved);
    if (index >= 0) {
        indicatorModel.removeRow(index);
        //update indices.
        for(int i = indicatorRemoved->category()+1; i < IndicatorTypes;++i){
            mGroupTypeIndeces[i]--;
        }
        repolish();
    }
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
    repolish();
}

void HbIndicatorMenuContent::indicatorlist_scrollingStarted()
{
    emit userActivityStarted();
}

void HbIndicatorMenuContent::indicatorlist_scrollingEnded()
{
    emit userActivityEnded();
}

HbIndicatorInterface *HbIndicatorMenuContent::indicatorFromIndex(const QModelIndex &modelIndex)
{
    QObject *ind_ptr =
        modelIndex.data(ListWidgetItemIndicatorTypeRole).value<QObject*>();
    return qobject_cast<HbIndicatorInterface*>(ind_ptr);
}

int HbIndicatorMenuContent::listIndexFromIndicator(HbIndicatorInterface *indicator) 
{
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

bool HbIndicatorMenuContent::hasMenuData(const HbIndicatorInterface &indicator) const
{
    if (!indicator.indicatorData(HbIndicatorInterface::PrimaryTextRole).toString().isEmpty() 
        || !indicator.indicatorData(HbIndicatorInterface::SecondaryTextRole).toString().isEmpty() 
        || !indicator.indicatorData(HbIndicatorInterface::DecorationNameRole).toString().isEmpty()) {
        return true;
    }

    return false;
}

