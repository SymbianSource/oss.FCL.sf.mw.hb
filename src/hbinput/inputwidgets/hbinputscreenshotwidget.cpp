/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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
#include <QPixmap>
#include <QPainter>

#include <hbframedrawer.h>
#include <hbeffect.h>
#include <hbwidget_p.h>

#include "hbinputscreenshotwidget.h"
#include "hbinputvkbwidget.h"


/*!
@proto
@hbinput
\class HbInputVkbWidget
\brief A base class for touch keypads.

This class implements default mechanisms for opening and closing touch keypads.
It know how to operate in landscape and in portait modes and it know how
implement split view -mechasnism for S60 QT UI's Hb library. It also implements
closing mechansim, where used is able to close the touch keypad by sliding it downwards
with a finger. This class also implements background drawing for touch keypads.
*/

/// @cond

class HbInputScreenshotWidgetPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbInputScreenshotWidget)

public:
    HbInputScreenshotWidgetPrivate();
    virtual ~HbInputScreenshotWidgetPrivate();

    QPixmap mPixmap;
    HbFrameDrawer *mIconDrawer;

};

HbInputScreenshotWidgetPrivate::HbInputScreenshotWidgetPrivate()
{
    mIconDrawer = new HbFrameDrawer();
}

HbInputScreenshotWidgetPrivate::~HbInputScreenshotWidgetPrivate()
{
    delete mIconDrawer;
}


/// @endcond

/*!
Costructs the object.
*/
HbInputScreenshotWidget::HbInputScreenshotWidget(QGraphicsItem* parent)
    : HbWidget(*new HbInputScreenshotWidgetPrivate, parent)
{
    Q_D(HbInputScreenshotWidget);
    d->q_ptr = this;

    setPos(QPointF(0,0));

#ifdef HB_EFFECTS
    HbEffect::disable(this);
#endif // HB_EFFECTS

#if QT_VERSION >= 0x040600
    // Make sure the keypad never steals focus.
    setFlag(QGraphicsItem::ItemIsPanel, true);
    setActive(false);
#endif
}

/*!
Destructs the object.
*/
HbInputScreenshotWidget::~HbInputScreenshotWidget()
{ 
}

void HbInputScreenshotWidget::setScreenshot(QPixmap &pixmap)
{
    Q_D(HbInputScreenshotWidget);
    d->mPixmap = pixmap;
}

/*!
handles mouse press event.
*/
void HbInputScreenshotWidget::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);
}

/*!
Handles mouse release event.
*/
void HbInputScreenshotWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);
}

/*!
The paint method. Draws the widget.
*/
void HbInputScreenshotWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    Q_D(HbInputScreenshotWidget);
    if (!d->mPixmap.isNull()){
        d->mIconDrawer->setFrameType(HbFrameDrawer::OnePiece);
        QRectF rect = boundingRect();
        rect.setLeft(rect.width()/2 - HbCloseHandleHeight*3);
        rect.setWidth(HbCloseHandleHeight*6);
        rect.setHeight(HbCloseHandleHeight);
		d->mIconDrawer->setFrameGraphicsName(HbInputVkbHandleIcon);
        d->mIconDrawer->paint(painter, rect);
    	painter->drawPixmap(0, (int)HbCloseHandleHeight, d->mPixmap);
    }
}


/*!
shape function actually refines the bounding rect. This function is used for collision detection
and hit test.
*/
QPainterPath HbInputScreenshotWidget::shape() const
{
    QRectF rect = boundingRect();
    QPainterPath path;
    path.addRect(rect);
    return path;
}


// End of file
