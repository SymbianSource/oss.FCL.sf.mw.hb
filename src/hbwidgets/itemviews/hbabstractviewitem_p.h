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
#include <hboogmwatcher_p.h>
#include <hbabstractitemview.h>
#include <hbabstractitemview_p.h>
#include <hbstyle.h>

#include <QObject>
#include <QGraphicsItem>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QExplicitlySharedDataPointer>
#include <QSharedData>
#include <QPainter>

class QGraphicsObject;
class QTimer;
class QGestureEvent;

#define HB_SD(Class) Class##Shared * sd = (Class##Shared *)(d->mSharedData.data())
#define HB_SDD(Class) Q_D(Class); Class##Shared * sd = (Class##Shared *)(d->mSharedData.data())

class HbViewItemPixmapPainter : public QGraphicsItem
{
public:

    HbViewItemPixmapPainter(int zValue, QGraphicsItem *parent) :
        QGraphicsItem(parent),
        mPixmap(0),
        mBoundingRect(parent->boundingRect())
    {
        setZValue(zValue);

        QGraphicsItem::GraphicsItemFlags itemFlags = flags();
        itemFlags &= ~QGraphicsItem::ItemUsesExtendedStyleOption;
        itemFlags |= QGraphicsItem::ItemHasNoContents;
        setFlags(itemFlags);
    }

    ~HbViewItemPixmapPainter()
    {
        delete mPixmap;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        if (mPixmap) {
            painter->drawPixmap(QPoint(0,0), *mPixmap);
        }
    }

    QRectF boundingRect() const
    {
        return mBoundingRect;
    }
    
    void setPixmap(QPixmap *pixmap) 
    {
        if (pixmap != mPixmap) {
            if (pixmap) {
                if (!mPixmap) {
                    setFlag(QGraphicsItem::ItemHasNoContents, false);
                }
            } else {
                if (mPixmap) {
                    setFlag(QGraphicsItem::ItemHasNoContents, true);
                }
            }

            delete mPixmap;
            mPixmap = pixmap;
        }
    }

    inline QPixmap *pixmap() const
    {
        return mPixmap;
    }

    void setSize(const QSizeF &size) {
        prepareGeometryChange();
        mBoundingRect = QRectF(QPointF(), size);
    }

private:

    QPixmap *mPixmap;
    QRectF mBoundingRect;
};

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
          mPressedItem(0),
          mLowGraphicsMemory(false)
        {
            HbOogmWatcher *watcher = HbOogmWatcher::instance();
            
            connect(watcher, SIGNAL(graphicsMemoryLow()), this, SLOT(disablePixmapCaches()));
            connect(watcher, SIGNAL(graphicsMemoryGood()), this, SLOT(enablePixmapCaches()));
        }
        
        virtual ~HbAbstractViewItemShared()
        {
        }

        void updateIconItemsAsyncMode();

        void setItemView(HbAbstractItemView *view);

    public slots:

        void pressStateChangeTimerTriggered();
        void disablePixmapCaches();
        void enablePixmapCaches();

        void scrollingStarted();

    public:

        HbAbstractViewItem *mPrototype;
        HbAbstractItemView *mItemView;

        QList<HbAbstractViewItem *> mCloneItems;
        HbFrameBackground mDefaultFrame;

        QString mItemType;
        static const int ViewItemDeferredDeleteEvent;

        QTimer *mPressStateChangeTimer;
        QPointer <HbAbstractViewItem> mPressedItem;
        bool mAnimatePress;

        bool mLowGraphicsMemory;
};

class HbAbstractViewItemPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC( HbAbstractViewItem )

    public:

        explicit HbAbstractViewItemPrivate(HbAbstractViewItem *prototype, HbAbstractViewItemShared *shared = 0) :
          HbWidgetPrivate(),
          mBackgroundItem(0),
          mFrame(0),
          mCheckState(Qt::Unchecked),
          mSelectionItem(0),
          mModelItemType(Hb::StandardItem),
          mContentChangedSupported(false),
          mItemsChanged(false),
          mPressed(false),
          mFocusItem(0),
          mMultiSelectionTouchArea(0),                    
          mSharedData(shared),
          mBackPixmapPainter(0),
          mFrontPixmapPainter(0),
          mInPaintItems(false),
          mNonCachableItem(0),
          mResetPixmapCache(true)
        {
            if (!mSharedData) {
                mSharedData = new HbAbstractViewItemShared;
            }
            mSharedData->mPrototype = prototype;
        }
        
        HbAbstractViewItemPrivate(const HbAbstractViewItemPrivate &source) :
            HbWidgetPrivate(),
            mIndex(source.mIndex),
            mBackgroundItem(0),
            mFrame(0),
            mCheckState(source.mCheckState),
            mSelectionItem(0),
            mModelItemType(source.mModelItemType),
            mContentChangedSupported(source.mContentChangedSupported),
            mItemsChanged(false),
            mPressed(false),
            mFocusItem(0),
            mMultiSelectionTouchArea(0),
            mSharedData(source.mSharedData),
            mBackPixmapPainter(0),
            mFrontPixmapPainter(0),
            mInPaintItems(false),
            mNonCachableItem(source.mNonCachableItem),
            mResetPixmapCache(true)
        {
        }
        
        HbAbstractViewItemPrivate &operator=(const HbAbstractViewItemPrivate &source)
        {
            mIndex = source.mIndex;
            mBackgroundItem = 0;
            mBackground = QVariant();
            mFrame = 0;
            mCheckState = Qt::Unchecked;
            mModelItemType = source.mModelItemType;
            mSelectionItem = 0;
            mContentChangedSupported = source.mContentChangedSupported;
            mItemsChanged = false;
            mPressed = false;
            mFocusItem = 0;
            mSharedData = source.mSharedData;
            mMultiSelectionTouchArea = 0;
            mBackPixmapPainter = 0;
            mFrontPixmapPainter = 0;
            mInPaintItems = false;
            mNonCachableItem = source.mNonCachableItem;
            mResetPixmapCache = true;

            return *this;
        }

        void init();
        
        static HbAbstractViewItemPrivate *d_ptr(HbAbstractViewItem *item) {
            Q_ASSERT(item);
            return item->d_func();
        }

        inline bool isPrototype() const
        {
            Q_Q(const HbAbstractViewItem);
            return q == mSharedData->mPrototype;
        }


        virtual int modelItemType() const;

        void _q_animationFinished(const HbEffect::EffectStatus &status);
        void _q_childrenChanged();

        void repolishCloneItems();
        void updateCloneItems(bool updateChildItems);

        virtual void setInsidePopup(bool insidePopup);

        virtual void tapTriggered(QGestureEvent *event);

        void revealItem();

        void setPressed(bool pressed, bool animate);

        void paintItems(QPainter *painter, QStyleOptionGraphicsItem *option, QGraphicsItem *startItem, QGraphicsItem *endItem);
        void paintChildItemsRecursively(QGraphicsItem *child, QPainter *painter,QStyleOptionGraphicsItem *option, const QPointF &translatePosition);
        void drawSubPixmap(QPixmap *pixmap,
                           QPainter *painter,
                           const QStyleOptionGraphicsItem *option);
        void setChildFlags(QGraphicsItem *child, bool pixmapCacheEnabled);
        void setChildFlagRecursively(bool pixmapCacheEnabled);

        inline bool usePixmapCache() const;

        void updatePixmap(QPixmap *pixmap, 
                        QPainter *painter, 
                        const QStyleOptionGraphicsItem *option, 
                        QGraphicsItem *startItem, 
                        QGraphicsItem *endItem);

        void releasePixmaps();

        static bool iconLoadedCallback(HbIconItem *target, void *param);

        bool iconLoaded(HbIconItem *target);

        void updateIconItemsAsyncMode(QGraphicsItem *item);

        inline void repolishItem();
        inline bool viewAnimating();

public:
        QPersistentModelIndex mIndex;
        
        QGraphicsObject *mBackgroundItem;
        QVariant mBackground;

        QGraphicsObject *mFrame;

        Qt::CheckState mCheckState;
        QGraphicsObject *mSelectionItem;

        int mModelItemType;

        // whether mContentChanged flag is supported
        bool mContentChangedSupported;
        // Status of child item existence changed.
        bool mItemsChanged;
        bool mPressed;

        QGraphicsObject *mFocusItem;

        QGraphicsObject *mMultiSelectionTouchArea;

        QExplicitlySharedDataPointer<HbAbstractViewItemShared> mSharedData;

        HbViewItemPixmapPainter *mBackPixmapPainter;
        HbViewItemPixmapPainter *mFrontPixmapPainter;

        QVector<QGraphicsItem*> mUpdateItems;

        bool mInPaintItems;

        QGraphicsItem *mNonCachableItem;
        bool mResetPixmapCache;
        
        QList<QGraphicsItem *> mChildren;

        friend class HbAbstractViewItemShared;
};

bool HbAbstractViewItemPrivate::usePixmapCache() const
{
     if (mSharedData->mItemView 
         && mSharedData->mItemView->itemPixmapCacheEnabled() 
         && !mSharedData->mLowGraphicsMemory) {
        return true;
    } else {
        return false;
    }
}

void HbAbstractViewItemPrivate::repolishItem()
{
    Q_Q(HbAbstractViewItem);
    q->repolish();
}

bool HbAbstractViewItemPrivate::viewAnimating()
{
    return mSharedData->mItemView && mSharedData->mItemView->d_func()->mIsAnimating;
}

#endif /*HBABSTRACTVIEWITEM_P_H*/
