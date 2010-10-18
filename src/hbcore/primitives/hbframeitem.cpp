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

#include "hbframeitem.h"
#include "hbevent.h"
#include "hbframedrawerpool_p.h"
#include "hbframedrawer_p.h"
#include "hbinstance_p.h"
#include <QPainter>
#include <QGraphicsWidget>

/*!
    @stable
    @hbcore
    \class HbFrameItem
    \brief A graphics item that draws a frame using the given HbFrameDrawer instance.

    This class is not intended to be derived from.

    Example of how to create a graphics frame item and use it.

    \dontinclude ultimatecodesnippet/ultimatecodesnippet.cpp
    \skip for the frame item
    \until }

    \sa class HbFrameDrawer
*/

/*!
    \reimp
    \fn int HbFrameItem::type() const
 */

class HbFrameItemPrivate
{
public:
    HbFrameItemPrivate(HbFrameItem *q);
    HbFrameItemPrivate(HbFrameItem *q, HbFrameDrawer *drawer);
    HbFrameItemPrivate(HbFrameItem *q,
                       const QString &frameGraphicsName,
                       HbFrameDrawer::FrameType frameGraphicsType);
    ~HbFrameItemPrivate();

    void init();
    void destroyFrameDrawer();

    HbFrameItem *item;
    HbFrameDrawer *frameDrawer;

private:
    Q_DISABLE_COPY(HbFrameItemPrivate)
};

HbFrameItemPrivate::HbFrameItemPrivate(HbFrameItem *q)
    : item(q),
      frameDrawer(new HbFrameDrawer())
{
    init();
}

HbFrameItemPrivate::HbFrameItemPrivate(HbFrameItem *q, HbFrameDrawer *drawer)
    : item(q),
      frameDrawer(drawer)
{
    init();
}

HbFrameItemPrivate::HbFrameItemPrivate(HbFrameItem *q,
                                       const QString &frameGraphicsName,
                                       HbFrameDrawer::FrameType frameGraphicsType)
    : item(q)
{
    frameDrawer = HbFrameDrawerPool::get(frameGraphicsName, frameGraphicsType);
    init();
}

HbFrameItemPrivate::~HbFrameItemPrivate()
{
    destroyFrameDrawer();
}

void HbFrameItemPrivate::init()
{
    item->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    if (QGraphicsWidget *parent = item->parentWidget()) {
        frameDrawer->setLayoutDirection(parent->layoutDirection());
    }

    frameDrawer->setGraphicsItem(item);
}

void HbFrameItemPrivate::destroyFrameDrawer()
{
    HbFrameDrawerPool::release(frameDrawer); // works also when frameDrawer is not from the pool
}

/*! Constructs a new graphics frame item with the given \a parent graphics item.
*/
HbFrameItem::HbFrameItem(QGraphicsItem *parent) :
    HbWidgetBase(parent)
{
    d = new HbFrameItemPrivate(this);
}

/*! Constructs a new graphics frame item with the given frame \a drawer and \a parent graphics item.
* The ownership of the given frame drawer object is transferred. It can be accessed with method
* HbFrameItem::frameDrawer().
*/
HbFrameItem::HbFrameItem(HbFrameDrawer *drawer, QGraphicsItem *parent)
    : HbWidgetBase(parent)
{
    d = new HbFrameItemPrivate(this, drawer);
}

/*!
  Constructs a new graphics frame item by automatically creating (or reusing) a framedrawer internally.

  Sharing is done by using the global HbFrameDrawerPool instance.

  This is the most efficient way of creating frame items that show the same graphics because they can
  share the same HbFrameDrawer instance internally between each other.

  \sa HbFrameDrawerPool
*/
HbFrameItem::HbFrameItem(const QString &frameGraphicsName,
                         HbFrameDrawer::FrameType frameGraphicsType,
                         QGraphicsItem *parent)
    : HbWidgetBase(parent)
{
    d = new HbFrameItemPrivate(this, frameGraphicsName, frameGraphicsType);
}

/*!
* Destroys the graphics frame item.
*/
HbFrameItem::~HbFrameItem()
{
    delete d;
}

/*!
* Returns a reference to the frame drawer used for painting the frame.
* The bounding rectangle for the frame can be changed by calling
* setRect() to the returned frame drawer.
*
* Note that if the HbFrameItem(const QString &, HbFrameDrawer::FrameType, QGraphicsItem *)
* constructor was used then the frame drawer returned by this function may be an instance
* that is shared between several frame items.
*
* \sa HbFrameItem::setFrameDrawer()
*/
HbFrameDrawer &HbFrameItem::frameDrawer() const
{
    return *(d->frameDrawer);
}

/*!
* Sets the frame \a drawer used for painting the frame.
* The ownership of the given frame drawer object is transferred.
* It can be accessed with method HbFrameItem::frameDrawer().
*
* \sa HbFrameItem::frameDrawer()
*/
void HbFrameItem::setFrameDrawer(HbFrameDrawer *drawer)
{
    if (drawer && drawer != d->frameDrawer) {
        d->destroyFrameDrawer();
        d->frameDrawer = drawer;
        d->init();
    }
}

/*!
* Paints the frame.
*/
void HbFrameItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    d->frameDrawer->paint(painter, boundingRect());
}

/*!
   \reimp
*/
void HbFrameItem::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LayoutDirectionChange) {
        if (QGraphicsWidget *parent = parentWidget()) {
            d->frameDrawer->setLayoutDirection(parent->layoutDirection());
        }
    } else if (event->type() == HbEvent::ThemeChanged) {
        d->frameDrawer->themeChanged();
    }

    switch (event->type()) {
        // suppress events (optimize)
    case QEvent::FontChange:
    case QEvent::PaletteChange:
    case QEvent::ParentChange:
        break;
    case QEvent::StyleChange: // flow through
    default:
        HbWidgetBase::changeEvent(event);
        break;
    }
}


QSizeF HbFrameItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (which == Qt::MinimumSize) {
        return QSizeF(0, 0);
    } else if (which == Qt::PreferredSize) {
        return QSizeF(0, 0);
    } else {
        return HbWidgetBase::sizeHint(which, constraint);
    }
}

/*!
   \reimp
*/
QVariant HbFrameItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemVisibleHasChanged) {
        if (!value.toBool()
            && d->frameDrawer
            && HbInstancePrivate::d_ptr()->mDropHiddenIconData) {
            HbFrameDrawerPrivate::d_ptr(d->frameDrawer)->reset();
        }
    }
    return HbWidgetBase::itemChange(change, value);
}


// End of File
