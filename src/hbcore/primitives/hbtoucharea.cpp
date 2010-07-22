
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

#include "hbtoucharea.h"
#include "hbtoucharea_p.h"

#include <QStyleOptionGraphicsItem>
#include <QGraphicsLayout>
#include <QPainter>
#include <QGraphicsScene>
#include <hbmainwindow.h>
#include <hbinstance.h>

bool HbTouchAreaPrivate::outlinesEnabled = false;

/*!
 \class HbTouchArea
 \brief HbTouchArea handles invisible touch area for widgets.
 @beta
 */

HbTouchAreaPrivate::HbTouchAreaPrivate() : q_ptr(0)
{
}

HbTouchAreaPrivate::~HbTouchAreaPrivate()
{
}

bool HbTouchAreaPrivate::outlineDrawingEnabled()
{
    return HbTouchAreaPrivate::outlinesEnabled;
}

void HbTouchAreaPrivate::setOutlineDrawing(bool enabled)
{
    HbTouchAreaPrivate::outlinesEnabled = enabled;

    // Because this method is static we need to toggle QGraphicsItem::ItemHasNoContents flag 
    // value hard way. This is RnD feature anyway.
    foreach (HbMainWindow *window, hbInstance->allMainWindows()) {
        QGraphicsScene *scene = static_cast<QGraphicsScene*>(window->scene()); //krazy:exclude=qclasses
        foreach( QGraphicsItem *item, scene->items() ) {
            if (HbTouchArea *widget = qgraphicsitem_cast<HbTouchArea *>(item)) {
                widget->setFlag(QGraphicsItem::ItemHasNoContents, !enabled);
            }
        }
    }
}

/*!
 Constructs a new HbTouchArea.
 */
HbTouchArea::HbTouchArea(QGraphicsItem *parent) :
    HbWidgetBase(*new HbTouchAreaPrivate(), parent)
{
    Q_D(HbTouchArea);
    d->q_ptr = this;
    setFlag(QGraphicsItem::ItemHasNoContents, !HbTouchAreaPrivate::outlinesEnabled);
    setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
}

/*!
 \internal
 */
HbTouchArea::HbTouchArea(HbTouchAreaPrivate &dd, QGraphicsItem *parent) :
    HbWidgetBase(dd, parent)
{
    setFlag(QGraphicsItem::ItemHasNoContents, !HbTouchAreaPrivate::outlinesEnabled);
}

/*!
 Destructs the graphics item.
 */
HbTouchArea::~HbTouchArea()
{
}

/*!
 Sets the size for the touch area. Without calling this method,
 the touch area uses its default size.
 This function needs to be called only when item
 is not added in layout. If its added to layout then layout
 will decide the position and size which will then be set
 in setGeometry() function.

 This is a convenience function for setGeometry(QRectF(pos(), size))
 */
void HbTouchArea::setSize(const QSizeF &size)
{
    setGeometry(QRectF(pos(), size));
}

/*!
 Paints the graphics item. When used in a release environment this will do
 nothing; when used internally AND the draw boxes flag is set (by calling 
 HbTouchArea::setDrawBoxes(true), a translucent box will be drawn indicating
 the active touch area.
 \note Currently option and widget parameters are unused
 \reimp
 */
void HbTouchArea::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)
    
    HbWidgetBase::paint(painter, option, widget);

    // The paint method is called only if HbTouchAreaPrivate::outlinesEnabled is true (RnD feature)
    // because flag QGraphicsItem::ItemHasNoContents is set otherwise.
    if (HbTouchAreaPrivate::outlinesEnabled) {
        painter->setPen(Qt::red); //krazy:exclude=qenums
        painter->drawLine(contentsRect().topLeft(), contentsRect().bottomRight());
        painter->drawLine(contentsRect().topRight(), contentsRect().bottomLeft()); 
        painter->setBrush(QBrush(QColor(255, 0, 0, 50)));
        painter->drawRect(contentsRect());
    }
}

/*!
    Virtual function from QGraphicsLayoutItem.
    Sets the new position and size of HbTouchArea.
 */
void HbTouchArea::setGeometry(const QRectF &rect)
{
    HbWidgetBase::setGeometry(rect);
}

/*!
 Virtual function from QGraphicsLayoutItem.
 */
void HbTouchArea::updateGeometry()
{
    // call the base class updateGeometry
    HbWidgetBase::updateGeometry();
}

/*!
 Virtual function from QGraphicsLayoutItem.
 Returns the size based upon the size hint(minimum, maximum and preferred) passed.
 The minimum size is QSizeF(0, 0);
 The maximum size is QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
 The preferred size is defaultSize() of the image.
 */
QSizeF HbTouchArea::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(constraint);
    if (which == Qt::MinimumSize) {
        return QSizeF(0, 0);
    } else if (which == Qt::MaximumSize) {
        return QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    } else {
        // Touch area should not enforce a size
		return QSizeF(0, 0);
    }
}

/*!
  Touch areas accept gestures by default
  */
void HbTouchArea::gestureEvent(QGestureEvent *event)
{
    Q_UNUSED(event);
}

// end of file
