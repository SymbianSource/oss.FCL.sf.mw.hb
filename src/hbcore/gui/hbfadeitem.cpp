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

#include "hbfadeitem_p.h"
#include "hbwidget_p.h"
#include "hbgraphicsscene.h"
#include <QEvent>
#include <QApplication>

/*!
  \class HbFadeItem

  \brief Item that blocks all scene events.

  \internal
*/

class HbFadeItemPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbFadeItem)

public:
    HbFadeItemPrivate();
    virtual ~HbFadeItemPrivate();
    void init();

    mutable QRectF mRect;

private:
    static HbFadeItemPrivate *d_ptr(HbFadeItem *fadeItem) {
        Q_ASSERT(fadeItem);
        return fadeItem->d_func();
    }
};

HbFadeItemPrivate::HbFadeItemPrivate()
{
}

HbFadeItemPrivate::~HbFadeItemPrivate()
{
}

void HbFadeItemPrivate::init()
{
    Q_Q(HbFadeItem);

    q->setBackgroundItem(HbStyle::P_Fade_background);

    q->setAcceptHoverEvents(true);
    // This is needed to be able to block moving the focus to items behind background item by
    // clicking on them.
    q->setFlag(QGraphicsItem::ItemIsFocusable);
}

HbFadeItem::HbFadeItem(QGraphicsItem *parent) : HbWidget(*new HbFadeItemPrivate, parent)
{
    Q_D(HbFadeItem);
    d->q_ptr = this;
    d->init();
}

HbFadeItem::HbFadeItem(HbFadeItemPrivate &dd, QGraphicsItem *parent) : HbWidget(dd, parent)
{
    Q_D(HbFadeItem);
    d->q_ptr = this;
    d->init();
}

QRectF HbFadeItem::boundingRect() const
{
    Q_D(const HbFadeItem);
    if (!d->mRect.isValid()) {
        // set size so that it is big enough
        // to cover the screen both landscape and portrait mode
        const QSizeF screenSize = HbDeviceProfile::profile(this).logicalSize();
        qreal dim = qMax(screenSize.width(), screenSize.height());
        d->mRect.adjust(0, 0, dim, dim);
    }
    return d->mRect;
}

bool HbFadeItem::sceneEvent(QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        // Prevents last focus item losing its focus
        // This event is received only when popup is modal
        QFocusEvent restoreLastFocus(QEvent::FocusIn, Qt::OtherFocusReason);
        QApplication::sendEvent(window()->scene(), &restoreLastFocus);
    }
    event->accept();
    return true;
}

bool HbFadeItem::event(QEvent *event)
{
    if (event->type() == QEvent::Polish) {
        // No need for any real polishing.
        Q_D(HbFadeItem);
        d->polished = true;
        return true;
    }
    return HbWidget::event(event);
}
