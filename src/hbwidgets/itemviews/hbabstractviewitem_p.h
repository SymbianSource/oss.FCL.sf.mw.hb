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
#ifndef HBABSTRACTVIEWITEM_P_H
#define HBABSTRACTVIEWITEM_P_H

#include "hbabstractviewitem.h"

#include <hbwidget_p.h>
#include <hbeffect.h>
#include <hbframebackground.h>
#include <hbnamespace.h>

#include <QObject>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QExplicitlySharedDataPointer>
#include <QSharedData>

class HbAbstractItemView;
class QGraphicsItem;
class QTimer;
class QGestureEvent;

#define HB_SD(Class) Class##Shared * sd = (Class##Shared *)(d->mSharedData.data())
#define HB_SDD(Class) Q_D(Class); Class##Shared * sd = (Class##Shared *)(d->mSharedData.data())

class HbAbstractViewItemShared : public QObject, public QSharedData
{
    Q_OBJECT

    public:

        HbAbstractViewItemShared() :
          mPrototype(0),
          mItemView(0),
          mDefaultFrame(),
          mItemType("viewitem"),
          mPressStateChangeTimer(0),
          mPressedItem(0)
        {
        }

    public slots:

        void pressStateChangeTimerTriggered();

    public:

        HbAbstractViewItem *mPrototype;
        HbAbstractItemView *mItemView;

        QList<HbAbstractViewItem *> mCloneItems;
        HbFrameBackground mDefaultFrame;

        QString mItemType;
        static const int ViewItemDeferredDeleteEvent;

        QTimer *mPressStateChangeTimer;
        HbAbstractViewItem *mPressedItem;
        bool mAnimatePress;
};

class HbAbstractViewItemPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC( HbAbstractViewItem )

    public:

        explicit HbAbstractViewItemPrivate(HbAbstractViewItem *prototype, HbAbstractViewItemShared *shared = 0) :
          HbWidgetPrivate(),
          mFocused(false),
          mBackgroundItem(0),
          mFrame(0),
          mCheckState(Qt::Unchecked),
          mSelectionItem(0),
          mModelItemType(Hb::StandardItem),
          mRepolishRequested(false),
          mContentChangedSupported(false),
          mItemsChanged(false),
          mPressed(false),
          mFocusItem(0),
          mMultiSelectionTouchArea(0),                    
          mSharedData(shared)
        {
            if (!mSharedData) {
                mSharedData = new HbAbstractViewItemShared;
            }
            mSharedData->mPrototype = prototype;
        }
        
        HbAbstractViewItemPrivate(const HbAbstractViewItemPrivate &source) :
            HbWidgetPrivate(),
            mIndex(source.mIndex),
            mFocused(source.mFocused),
            mBackgroundItem(0),
            mFrame(0),
            mCheckState(source.mCheckState),
            mSelectionItem(0),
            mModelItemType(source.mModelItemType),
            mRepolishRequested(false),
            mContentChangedSupported(source.mContentChangedSupported),
            mItemsChanged(false),
            mPressed(false),
            mFocusItem(0),
            mMultiSelectionTouchArea(0),
            mSharedData(source.mSharedData)
        {
        }
        
        HbAbstractViewItemPrivate &operator=(const HbAbstractViewItemPrivate &source)
        {
            mIndex = source.mIndex;
            mFocused = source.mFocused;
            mBackgroundItem = 0;
            mBackground = QVariant();
            mFrame = 0;
            mCheckState = Qt::Unchecked;
            mModelItemType = source.mModelItemType;
            mSelectionItem = 0;
            mRepolishRequested = false;
            mContentChangedSupported = source.mContentChangedSupported;
            mItemsChanged = false;
            mPressed = false;
            mFocusItem = 0;
            mSharedData = source.mSharedData;
            mMultiSelectionTouchArea = 0;

            return *this;
        }

        void init();
        
        inline bool isPrototype() const
        {
            Q_Q(const HbAbstractViewItem);
            return q == mSharedData->mPrototype;
        }


        virtual int modelItemType() const;

        void _q_animationFinished(const HbEffect::EffectStatus &status);

        void repolishCloneItems();
        void updateCloneItems(bool updateChildItems);

        virtual void setInsidePopup(bool insidePopup);

        virtual void tapTriggered(QGestureEvent *event);

        void revealItem();

        void setPressed(bool pressed, bool animate);

public:
        QPersistentModelIndex mIndex;
        bool mFocused;
        
        QGraphicsItem *mBackgroundItem;
        QVariant mBackground;

        QGraphicsItem *mFrame;

        Qt::CheckState mCheckState;
        QGraphicsItem *mSelectionItem;

        int mModelItemType;
        bool mRepolishRequested;

        // whether mContentChanged flag is supported
        bool mContentChangedSupported;
        // Status of child item existence changed.
        bool mItemsChanged;
        bool mPressed;

        QGraphicsItem *mFocusItem;

        QGraphicsItem *mMultiSelectionTouchArea;

        QExplicitlySharedDataPointer<HbAbstractViewItemShared> mSharedData;
};

#endif /*HBABSTRACTVIEWITEM_P_H*/
