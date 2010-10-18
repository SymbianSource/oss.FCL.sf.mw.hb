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
#include "hbinputscreenshotwidget.h"

#include <QPixmap>
#include <QPainter>

#include <hbeffect.h>
#include <hbwidget_p.h>
#include <hbinputregioncollector_p.h>
#include "hbframedrawerpool_p.h"

#include "hbinputvkbwidget.h"

/*!
@stable
@hbinput
\class HbInputScreenshotWidget
\brief The HbInputScreenshotWidget class provides screenshots of virtual keyboards for animation purposes.

This class is used, for example, in HbInputVkbWidget when it shows an animation during a virtual keyboard change.
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
    qreal mCloseHandleHeight;
    qreal mCloseHandleWidth;

};

HbInputScreenshotWidgetPrivate::HbInputScreenshotWidgetPrivate()
    : mIconDrawer(0), mCloseHandleHeight(0), mCloseHandleWidth(0)
{
}

HbInputScreenshotWidgetPrivate::~HbInputScreenshotWidgetPrivate()
{
    HbFrameDrawerPool::release(mIconDrawer);
}


/// @endcond

/*!
Constructor.
*/
HbInputScreenshotWidget::HbInputScreenshotWidget(QGraphicsItem *parent)
    : HbWidget(*new HbInputScreenshotWidgetPrivate, parent)
{
    Q_D(HbInputScreenshotWidget);
    d->q_ptr = this;
    HbInputRegionCollector::instance()->attach(this);

    setPos(QPointF(0, 0));

#ifdef HB_EFFECTS
    HbEffect::disable(this);
#endif // HB_EFFECTS

    // Make sure the keypad never steals focus.
    setFlag(QGraphicsItem::ItemIsPanel, true);
    setActive(false);
}

/*!
Destructor.
*/
HbInputScreenshotWidget::~HbInputScreenshotWidget()
{
}

/*!
Sets \a pixmap as the current pixmap of the widget.
*/
void HbInputScreenshotWidget::setScreenshot(QPixmap &pixmap)
{
    Q_D(HbInputScreenshotWidget);
    d->mPixmap = pixmap;

    if (!d->mPixmap.isNull()) {
        HbFrameDrawerPool::release(d->mIconDrawer);

        qreal unitValue = HbDeviceProfile::profile(mainWindow()).unitValue();
        d->mCloseHandleHeight = HbCloseHandleHeightInUnits * unitValue;
        d->mCloseHandleWidth = HbCloseHandleWidthInUnits * unitValue;
        QSizeF handleSize = QSizeF(d->mCloseHandleWidth, d->mCloseHandleHeight);
        d->mIconDrawer = HbFrameDrawerPool::get(HbInputVkbHandleIcon, HbFrameDrawer::OnePiece, handleSize);
    }
}

/*!
Handles a mouse press event.
*/
void HbInputScreenshotWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
}

/*!
Handles a mouse release event.
*/
void HbInputScreenshotWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
}

/*!
Draws the widget. Parameters \a option and \a widget are not used.
*/
void HbInputScreenshotWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    Q_D(HbInputScreenshotWidget);
    if (!d->mPixmap.isNull()) {
        QRectF rect = boundingRect();

        painter->save();
        painter->translate(rect.width() / 2 - d->mCloseHandleWidth / 2, 0);
        rect.setWidth(d->mCloseHandleWidth);
        rect.setHeight(d->mCloseHandleHeight);
        d->mIconDrawer->paint(painter, rect);
        painter->restore();

        painter->save();
        painter->translate(0, d->mCloseHandleHeight);
        painter->drawPixmap(0, 0, d->mPixmap);
        painter->restore();
    }
}


/*!
Returns the bounding area of the widget as a graphical shape,
which can be used for collision detection and hit test.
*/
QPainterPath HbInputScreenshotWidget::shape() const
{
    QRectF rect = boundingRect();
    QPainterPath path;
    path.addRect(rect);
    return path;
}


// End of file
