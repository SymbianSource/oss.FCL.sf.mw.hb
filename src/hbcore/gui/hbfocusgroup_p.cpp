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

#include <hbfocusgroup_p.h>
#include <QDebug>

HbFocusGroup::HbFocusGroup(QGraphicsWidget *parent):QGraphicsItem(parent),
    mCurrentFocusChild(0),
    mFocusFirst(0),
    mFocusLast(0),
    mFocusDelegation(HbWidget::FocusDelegationFirstChild),
    mFocusLooping(true),
    mFocusMode(HbWidget::FocusModeNormal),
    mFocusParameters(NeverOn)
{
}

HbFocusGroup::~HbFocusGroup()
{
#if QT_VERSION < 0x040501
    if (parentItem()) {
        unregisterChildren(parentItem());
    }
#endif
}

void HbFocusGroup::registerItem(QGraphicsItem *item)
{
    if (item->scene() && item != this && item->flags().testFlag(QGraphicsItem::ItemIsFocusable)) {

        // add item and update focus order
        childList[item] = item;
        setFocusOrder(mFocusLast,item);

        // Make sure the focus chain is closed
        // dumpFocusChain();
    }
}

bool HbFocusGroup::isItemRegistered(QGraphicsItem *item) const
{
    ChildInfo itemInfo = childList.value(item);
    return itemInfo.focusNext && itemInfo.focusPrev;
}

void HbFocusGroup::unregisterItem(QGraphicsItem *item)
{
    if(item != this) {
        removeItem(item);
    }
}

void HbFocusGroup::registerChildren(QGraphicsItem *item)
{
    if (item->scene()) {
        foreach (QGraphicsItem *child, item->children()) {
            registerItem(child);
        }
    }
}

void HbFocusGroup::unregisterChildren(QGraphicsItem *item)
{
    if (item->scene()) {
        foreach (QGraphicsItem *child, item->children()) {
            unregisterItem(child);
        }
    }
}

bool HbFocusGroup::isFocusNextPrevKey(const QKeyEvent &event, bool next) const
{
    QKeySequence key = next?mKeyFocusNextChild:mKeyFocusPreviousChild;

    if (!key.isEmpty()) {
        //The keypad modifier should not make a difference
        int searchkey = (event.modifiers() | event.key()) & ~(Qt::KeypadModifier);
        for (int i = 0; i < static_cast<int>(key.count()); i++) {
            if (key[i] == searchkey) {
                return true;
            }
        }
    }
    return false;
}

bool HbFocusGroup::isFocusModeChangeKey(const QKeyEvent &event) const
{
    if (!mKeyFocusMode.isEmpty()) {
        int searchkey = (event.modifiers() | event.key()) & ~(Qt::KeypadModifier);
        for (int i = 0; i < static_cast<int>(mKeyFocusMode.count()); i++) {
            if (mKeyFocusMode[i] == searchkey) {
                return true;
            }
        }
    }
    return false;
}

void HbFocusGroup::setFocusOrder(QGraphicsItem *first, QGraphicsItem *second)
{
    if (!first && !second) {
        qWarning("HbWidget::setFocusOrder(0, 0) is undefined");
        return;
    }

    // Update focusFirst, focusLast
    if(first == mFocusLast) {
        mFocusLast = second;
    }

    if(second == mFocusFirst) {
        mFocusFirst = first;
    }

    // If first 0, the group's focusFirst is set to second
    // If second 0, the group's focusLast is set to first
    if (!first) {
        mFocusFirst = second;
        return;
    }
    if (!second) {
        mFocusLast = first;
        return;
    }

    // Both first and second are != 0.

    ChildInfo secondInfo = childList.value(second);
    QGraphicsItem *secondFocusPrev = secondInfo.focusPrev;
    QGraphicsItem *secondFocusNext = secondInfo.focusNext;
    QGraphicsItem *firstFocusNext = childList.value(first).focusNext;
    if (firstFocusNext == second) {
        // Nothing to do.
        return;
    }

    // Update the focus chain.
    // Step 1 update secondFocusPrev item
    ChildInfo secondFocusPrevInfo = childList.value(secondFocusPrev);
    secondFocusPrevInfo.focusNext = secondFocusNext;
    childList[secondFocusPrev] = secondFocusPrevInfo;

    // Step 2 update secondFocusNext item
    ChildInfo secondFocusNextInfo = childList.value(secondFocusNext);
    secondFocusPrevInfo.focusPrev = secondFocusPrev;
    childList[secondFocusNext] = secondFocusPrevInfo;

    // Step 3 update firstFocusNext item
    ChildInfo firstFocusNextInfo = childList.value(firstFocusNext);
    firstFocusNextInfo.focusPrev = second;
    childList[firstFocusNext] = firstFocusNextInfo;

    // Step 4 update first item
    ChildInfo firstInfo = childList.value(first);
    firstInfo.focusNext = second;
    childList[first] = firstInfo;

    // Step 5 update second item
    secondInfo.focusNext = firstFocusNext;
    secondInfo.focusPrev = first;
    childList[second] = secondInfo;

    Q_ASSERT(childList.value(childList.value(first).focusNext).focusPrev == first);
    Q_ASSERT(childList.value(childList.value(first).focusPrev).focusNext == first);
    Q_ASSERT(childList.value(childList.value(second).focusNext).focusPrev == second);
    Q_ASSERT(childList.value(childList.value(second).focusPrev).focusNext == second);
}

bool HbFocusGroup::focusNextPrevItem(QGraphicsItem *item, bool next) const
{    
    if (item) {
        ChildInfo itemInfo = findNextPrevItem(item, next);

        if (next) {
            if(mFocusLooping || item != mFocusLast) {
                if (itemInfo.focusNext) {
                    itemInfo.focusNext->setFocus();                    
                    return true;
                }
            }
        } else {
            if(mFocusLooping || item != mFocusFirst) {
                if (itemInfo.focusPrev) {
                    itemInfo.focusPrev->setFocus(Qt::BacktabFocusReason);                    
                    return true;
                }
            }
        }
    }
    return false;
}

HbFocusGroup::ChildInfo HbFocusGroup::findNextPrevItem(QGraphicsItem *item, bool next) const
{
    ChildInfo itemInfo = childList.value(item);

    if (next) {
        while (itemInfo.focusNext && (!itemInfo.focusNext->isVisible() || !itemInfo.focusNext->isEnabled())) {
            itemInfo = childList.value(itemInfo.focusNext);

            if (item == itemInfo.focusNext) {
                break;
            }
        }

        if (item == itemInfo.focusNext) {
            // full circle gone through.
            bool result = item->isVisible() && item->isEnabled();
            if (!mFocusLooping || !result) {
                return ChildInfo(0);
            }
        }
    } else {
        while (itemInfo.focusPrev && (!itemInfo.focusPrev->isVisible() || !itemInfo.focusPrev->isEnabled())) {
            itemInfo = childList.value(itemInfo.focusPrev);

            if (item == itemInfo.focusPrev) {
                break;
            }
        }

        if (item == itemInfo.focusPrev) {
            // full circle gone through.
            bool result = item->isVisible() && item->isEnabled();
            if (!mFocusLooping || !result) {
                return ChildInfo(0);
            }
        }
    }
    return itemInfo;
}

bool HbFocusGroup::delegateFocus(QFocusEvent *event)
{
    QGraphicsItem *delegatee = 0;

    if (mFocusDelegation != HbWidget::FocusDelegationNone) {
        switch (mFocusDelegation) {
            case HbWidget::FocusDelegationFirstChild:
                delegatee = mFocusFirst;
                break;
            case HbWidget::FocusDelegationLastChild:
                delegatee = mFocusLast;
                break;
            case HbWidget::FocusDelegationBasedOnDirection:
                if (event->reason() == Qt::BacktabFocusReason) {
                    delegatee = mFocusLast;
                } else {
                    delegatee = mFocusFirst;
                }
                break;
            case HbWidget::FocusDelegationFocusedChild:
                delegatee = mCurrentFocusChild;
                break;
            default:;
        }

        if (!delegatee) {
            delegatee = mFocusFirst;
        }

        if (delegatee) {
            if (delegatee->isVisible() && delegatee->isEnabled()) {
                delegatee->setFocus(event->reason());
            } else {
                return focusNextPrevItem(delegatee, event->reason() != Qt::BacktabFocusReason);
            }
        }
    }
    return delegatee;
}

void HbFocusGroup::updateCurrentFocusChild(QGraphicsItem *item)
{
    if(item && item != mCurrentFocusChild && isItemRegistered(item)) {
        qDebug() << "Update current focuschild: " << QString::number(quintptr(this), 16)
            << " has " << QString::number(quintptr(item), 16);
        mCurrentFocusChild = item;
    }
}

/*
void HbFocusGroup::dumpFocusChain()
{
    qDebug() << "=========== Begin of dumping focus chain ==============";
    qDebug() << "focusFirst: " << QString::number(uint(mFocusFirst), 16) <<
            (mFocusFirst&&mFocusFirst->isWidget()?static_cast<QGraphicsWidget*>(mFocusFirst)->metaObject()->className():"");
    qDebug() << "focusLast: " << QString::number(uint(mFocusLast), 16) <<
            (mFocusLast&&mFocusLast->isWidget()?static_cast<QGraphicsWidget*>(mFocusLast)->metaObject()->className():"");
    qDebug() << "lastFocusChild: " << QString::number(uint(mCurrentFocusChild), 16) <<
            (mCurrentFocusChild&&mCurrentFocusChild->isWidget()?static_cast<QGraphicsWidget*>(mCurrentFocusChild)->metaObject()->className():"");


    qDebug() << "Number of items in chain: " << childList.size();

    if (childList.size()) {

        QGraphicsItem *next = mFocusFirst;
        QSet<QGraphicsItem*> visited;
        do {
            if (!next) {
                qWarning("Found a focus chain that is not circular, (next == 0)");
                break;
            }
            QGraphicsItem *nextFocusNext = childList.value(next).focusNext;
            QGraphicsItem *nextFocusPrev = childList.value(next).focusPrev;

            qDebug() << "next: " << QString::number(uint(next), 16) <<
                            (next&&next->isWidget()?static_cast<QGraphicsWidget*>(next)->metaObject()->className():"") <<
                        " next->focusNext: " << QString::number(uint(nextFocusNext), 16) <<
                            (nextFocusNext&&nextFocusNext->isWidget()?static_cast<QGraphicsWidget*>(nextFocusNext)->metaObject()->className():"") <<
                        " next->focusPrev: " << QString::number(uint(nextFocusPrev), 16) <<
                            (nextFocusPrev&&nextFocusPrev->isWidget()?static_cast<QGraphicsWidget*>(nextFocusPrev)->metaObject()->className():"");

            if (visited.contains(next)) {
                qWarning("Already visited this node. However, I expected to dump until I found myself.");
                break;
            }
            visited << next;
            next = childList.value(next).focusNext;
        } while (next != mFocusFirst);

    } else {
        if (mFocusFirst) {
            qWarning("focusFirst is not 0 for empty focus group");
        }
        if (mFocusLast) {
            qWarning("focusLast is not 0 for empty focus group");
        }
        if (mCurrentFocusChild) {
            qWarning("lastFocusChild is not 0 for empty focus group");
        }
    }
    qDebug() << "=========== End of dumping focus chain ==============";
}
*/

void HbFocusGroup::removeItem(QGraphicsItem *item)
{
    ChildInfo itemInfo = childList.value(item);

    if (itemInfo.focusNext && itemInfo.focusPrev) {

        // Update the focus chain.
        QGraphicsItem *itemFocusPrev = itemInfo.focusPrev;
        QGraphicsItem *itemFocusNext = itemInfo.focusNext;

        // Step 1 update itemFocusPrev item
        ChildInfo itemFocusPrevInfo = childList.value(itemFocusPrev);
        itemFocusPrevInfo.focusNext = itemFocusNext;
        childList[itemFocusPrev] = itemFocusPrevInfo;

        // Step 2 update itemFocusNext item
        ChildInfo itemFocusNextInfo = childList.value(itemFocusNext);
        itemFocusNextInfo.focusPrev = itemFocusPrev;
        childList[itemFocusNext] = itemFocusNextInfo;

        Q_ASSERT(childList.value(childList.value(itemFocusPrev).focusNext).focusPrev == itemFocusPrev);
        Q_ASSERT(childList.value(childList.value(itemFocusPrev).focusPrev).focusNext == itemFocusPrev);
        Q_ASSERT(childList.value(childList.value(itemFocusNext).focusNext).focusPrev == itemFocusNext);
        Q_ASSERT(childList.value(childList.value(itemFocusNext).focusPrev).focusNext == itemFocusNext);

        // remove item from hash table
        childList.remove(item);

        // Update lastFocusChild, focusFirst and focusLast if needed
        if (mCurrentFocusChild == item) {
            mCurrentFocusChild = itemFocusPrev;
        }
        if (!childList.size()) {
            mFocusFirst = 0;
            mFocusLast = 0;
            mCurrentFocusChild = 0;
        } else if (mFocusFirst == item) {
            mFocusFirst = itemFocusNext;
        } else if (mFocusLast == item) {
            mFocusLast = itemFocusPrev;
        }
    }
}

void HbFocusGroup::changeFocusMode()
{
    if (mFocusMode == HbWidget::FocusModeNormal) {
        mFocusMode = HbWidget::FocusModeEdit;
    }
    else {
        mFocusMode = HbWidget::FocusModeNormal;
    }
}

void HbFocusGroup::startHighlightTimer(HbWidget *widget)
{
    if (mFocusParameters.testFlag(Transient) && !mFocusParameters.testFlag(HbFocusGroup::AlwaysOn)) {
        mHighlightTimer.setInterval(3000);
        QObject::connect( &mHighlightTimer, SIGNAL(timeout()), widget, SLOT(hideFocusHighlight()) );
        mHighlightTimer.start();
    }
}

void HbFocusGroup::stopHighlightTimer(HbWidget *widget, bool disconnect)
{
    if (disconnect) {
        QObject::disconnect( &mHighlightTimer, SIGNAL(timeout()), widget, SLOT(hideFocusHighlight()) );
    }

    if (mHighlightTimer.isActive()) {
        mHighlightTimer.stop();
    }
}

void HbFocusGroup::setKeyForFocusNextChild(const QKeySequence &key)
{
    mKeyFocusNextChild = key;
}

void HbFocusGroup::setKeyForFocusPreviousChild(const QKeySequence &key)
{
    mKeyFocusPreviousChild = key;
}

void HbFocusGroup::setFocusMode(HbWidget::FocusMode focusMode)
{
    mFocusMode = focusMode;
}

void HbFocusGroup::setKeyForFocusMode(const QKeySequence &key)
{
    mKeyFocusMode = key;
}

void HbFocusGroup::setFocusDelegation(HbWidget::FocusDelegation focusDelegation)
{
    mFocusDelegation = focusDelegation;
}

void HbFocusGroup::setFocusLooping(bool enable)
{
    mFocusLooping = enable;
}
