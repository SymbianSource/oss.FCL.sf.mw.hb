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

#include "hbbackgrounditem_p.h"
#include "hbwidget_p.h"
#include "hbinstance.h"
#include "hbdeviceprofile.h"
#include "hbevent.h"
#include "hbmainwindow_p.h"
#include <QGraphicsSceneResizeEvent>
#include <QPainter>
#include <QObject>
#include <QDebug>

/*
    \class HbBackgroundItem

    \brief Draws the background.

    \internal
*/

HbBackgroundItem::HbBackgroundItem(HbMainWindow *mainWindow, QGraphicsWidget *parent) :
    HbWidget(parent),
    mMainWindow(mainWindow),
    mImageMode(Hb::ScaleBackgroundToFit)
{
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setFlag(QGraphicsItem::ItemHasNoContents, false);
    mPrtImageName = defaultImageName(Qt::Vertical);
    mLscImageName = defaultImageName(Qt::Horizontal);
    updateBackgroundImage();
}

HbBackgroundItem::~HbBackgroundItem()
{
}

void HbBackgroundItem::setImageName(Qt::Orientation orientation, const QString &name)
{
    bool changed = false;
    if (orientation == Qt::Vertical) {
        if (name != mPrtImageName) {
            mPrtImageName = name;
            changed = true;
        }
    } else {
        if (name != mLscImageName) {
            mLscImageName = name;
            changed = true;
        }
    }
    if (changed) {
        updateBackgroundImage();
    }
}

QString HbBackgroundItem::imageName(Qt::Orientation orientation) const
{
    return orientation == Qt::Vertical ? mPrtImageName : mLscImageName;
}

QString HbBackgroundItem::defaultImageName(Qt::Orientation orientation) const
{
    return orientation == Qt::Vertical
           ? QLatin1String("qtg_graf_screen_bg_prt")
           : QLatin1String("qtg_graf_screen_bg_lsc");
}

void HbBackgroundItem::setImageMode(Hb::BackgroundImageMode mode)
{
    if (mode != mImageMode) {
        mImageMode = mode;
        updateBackgroundImage();
    }
}

Hb::BackgroundImageMode HbBackgroundItem::imageMode() const
{
    return mImageMode;
}

void HbBackgroundItem::updateBackgroundImage()
{
    prepareGeometryChange();
    if (mMainWindow) {
        QSizeF size(HbDeviceProfile::profile(mMainWindow).logicalSize());
        mBoundingRect.setWidth(size.width());
        mBoundingRect.setHeight(size.height());
        if (mMainWindow->orientation() == Qt::Vertical) {
            mBackground.setIconName(mPrtImageName);
        } else {
            mBackground.setIconName(mLscImageName);
        }
        if (mImageMode == Hb::KeepOriginalBackgroundSize
                || mImageMode == Hb::KeepOriginalBackgroundSizeIfSmaller) {
            QSizeF imageSize = mBackground.defaultSize();
            if (mImageMode == Hb::KeepOriginalBackgroundSize
                    || (imageSize.width() <= size.width() && imageSize.height() <= size.height())) {
                size = imageSize;
            }
        }
        mBackground.setSize(size);
    }
}

void HbBackgroundItem::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    HbWidget::resizeEvent(event);

    // RnD feature for resizing the window by dragging
    if (HbMainWindowPrivate::dragToResizeEnabled) {
        prepareGeometryChange();
        if (mMainWindow) {
            QSizeF size(event->newSize());
            mBoundingRect.setWidth(size.width());
            mBoundingRect.setHeight(size.height());
            mBackground.setSize(size);
        }
    }
}

bool HbBackgroundItem::event(QEvent *e)
{
    if (e->type() == QEvent::Polish) {
        // No need for any real polishing.
        static_cast<HbWidgetPrivate *>(d_ptr)->polished = true;
        return true;
    } else if (e->type() == HbEvent::DeviceProfileChanged) {
        updateBackgroundImage();
    }
    return HbWidget::event(e);
}

QRectF HbBackgroundItem::boundingRect() const
{
    return mBoundingRect;
}

void HbBackgroundItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    if (mImageMode == Hb::DoNotDrawBackground) {
        return;
    }

    // Note: No optimizations to disable alpha blending etc. The background
    // image may be anything, it can have transparent parts too.

    Qt::AspectRatioMode aspRatMode;
    switch (mImageMode) {
    case Hb::ScaleBackgroundToFitWithoutExpanding:
        aspRatMode = Qt::KeepAspectRatio;
        break;
    case Hb::StretchBackgroundToFit:
        aspRatMode = Qt::IgnoreAspectRatio;
        break;
    default:
        aspRatMode = Qt::KeepAspectRatioByExpanding;
        break;
    }

    mBackground.paint(painter, mBoundingRect, aspRatMode, Qt::AlignCenter);
}
