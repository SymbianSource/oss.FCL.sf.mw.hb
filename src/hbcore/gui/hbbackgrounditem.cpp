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

#include <QGraphicsSceneResizeEvent>
#include <QPainter>
#include <QObject>
#include <QDebug>

#include "hbbackgrounditem_p.h"
#include "hbwidget_p.h"
#include "hbinstance.h"
#include "hbdeviceprofile.h"
#include "hbevent.h"
#include "hbmainwindow_p.h"

#ifndef HB_NVG_CS_ICON
#define ENABLE_FAST_PAINT_
#endif

/*
    \class HbBackgroundItem

    \brief HbBackgroundItem draws background

    \internal
*/

HbBackgroundItem::HbBackgroundItem(HbMainWindow *mainWindow, QGraphicsWidget *parent) :
        HbWidget(parent),
        mMainWindow(mainWindow)
{
#ifdef ENABLE_FAST_PAINT_
    setAttribute(Qt::WA_NoSystemBackground); // Disable clearing of background
#endif
    setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );

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

void HbBackgroundItem::updateBackgroundImage()
{
    prepareGeometryChange();
    if (mMainWindow) {
        QSizeF size(HbDeviceProfile::profile(mMainWindow).logicalSize());
        mBoundingRect.setWidth(size.width());
        mBoundingRect.setHeight(size.height());
        mBackground.setSize(size);
        if (mMainWindow->orientation() == Qt::Vertical) {
            mBackground.setIconName(mPrtImageName);
        } else {
            mBackground.setIconName(mLscImageName);
        }
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
    Q_UNUSED(widget)
    Q_UNUSED(option)

#ifdef ENABLE_FAST_PAINT_
    QPainter::CompositionMode compositionMode = painter->compositionMode();
    painter->setCompositionMode( QPainter::CompositionMode_Source );  // Do not use alpha blending..
#endif

    mBackground.paint(painter, mBoundingRect, Qt::KeepAspectRatioByExpanding);

#ifdef ENABLE_FAST_PAINT_
    painter->setCompositionMode( compositionMode );  // restore old composition mode
#endif
}
