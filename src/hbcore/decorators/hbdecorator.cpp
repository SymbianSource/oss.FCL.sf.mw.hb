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

#include <QGraphicsSceneMouseEvent>

#include "hbdecorator_p.h"
#include "hbdecorator_p_p.h"
#include "hbstyleoptiondecorator.h"
#include "hbmainwindow_p.h"
#include "hbview.h"

/*
    \class HbDecorator
    \brief HbDecorator is the base class for common decorators. Decorators are
    components that are always available even if the view contents change.

    HbDecorator class is a base class for all decorators. This class defines the available decorator
    types:

    enum DecoratorType
    {
        SoftKey,
        Indicator,
        TitlePane,
        NaviPane

    HbDecorator offers a simple interface for showing, hiding and updating a decorator.
    Even though this is not an abstract API it is meant to be derived and does not have much
    use as is.

    For more information please see the documentation of the implemented decorators (from the
    inheritance diagram).

    Typically decorators are created by using HbDecoratorGroup.

    \sa HbDecoratorGroup

 */

HbDecoratorPrivate::HbDecoratorPrivate()
    : mMode(QIcon::Normal)
{
//#ifdef HB_EFFECTS
//    HbEffect::add("decorator", "decorator_pressed", "pressed");
//    HbEffect::add("decorator", "decorator_released", "released");
//    HbEffect::add("decorator", "decorator_latched", "latched");
//#endif
}

void HbDecoratorPrivate::init(HbDecorator::DecoratorType type)
{
    mType = type;
}

/*
    Constructor, the decorator \a type is required as a parameter.
    The \a parent is an optional parameter.
*/
HbDecorator::HbDecorator(DecoratorType type, QGraphicsItem *parent)
    : HbWidget(*new HbDecoratorPrivate, parent)
{
    Q_D(HbDecorator);
    d->init(type);
    setFocusPolicy(Qt::ClickFocus);
}

HbDecorator::HbDecorator(HbDecoratorPrivate &dd, DecoratorType type, QGraphicsItem *parent)
    : HbWidget(dd, parent)
{
    Q_D(HbDecorator);
    d->init(type);
    setFocusPolicy(Qt::ClickFocus);
}

/*
    Destructor.
 */
HbDecorator::~HbDecorator()
{
}


/*
    \a property HbDecorator::decoratorType
    \brief This property holds the decorator type
 */
HbDecorator::DecoratorType HbDecorator::decoratorType() const
{
    Q_D(const HbDecorator);
    return d->mType;
}

/*
    Resets icon mode
*/
void HbDecorator::resetMode()
{
    Q_D(HbDecorator);
    d->mMode = QIcon::Normal;
#ifdef HB_EFFECTS
    HbEffect::start(this, "decorator", "released");
#endif
}

/*
    \reimp
*/
QVariant HbDecorator::itemChange(GraphicsItemChange change, const QVariant & value)
{
    switch( change ) {
        case ItemVisibleHasChanged: 
            {
            Q_D(const HbDecorator);
            if ( d->polished )  {
                // Cannot emit signals directly from "visibilityChanged", since it might
                // lead to e.g. item deletion that's not allowed in "itemChange".
                // Using QMetaObject to emit the signal asynchronously.
                QMetaObject::invokeMethod(this, "visibilityChanged", Qt::QueuedConnection);
            }
            break;
            }      
        default:
            break;
    }
    return HbWidget::itemChange( change, value );
}

void HbDecorator::initStyleOption(HbStyleOptionDecorator *option) const
{
    Q_D(const HbDecorator);
    HbWidget::initStyleOption(option);

    option->mode = d->mMode;
    if (mainWindow() && mainWindow()->currentView()) {
        if (mainWindow()->currentView()->titleBarFlags() & HbView::TitleBarTransparent) {
            option->transparent = true;
        }
    }
}

/*
    \reimp
 */
void HbDecorator::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbDecorator);
    Q_UNUSED( event );
    d->mMode = QIcon::Active;
    updatePrimitives();
#ifdef HB_EFFECTS
    if (boundingRect().contains(event->pos())) {
        HbEffect::start(this, "decorator", "pressed");
    }
#endif
}

/*
    \reimp
 */
void HbDecorator::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbDecorator);

    if (boundingRect().contains(event->pos())) {
        d->mMode = QIcon::Selected;
        updatePrimitives();
#ifdef HB_EFFECTS
        if (boundingRect().contains(event->pos())) {
            HbEffect::start(this, "decorator", "latched");
        }
#endif
    } else {
        d->mMode = QIcon::Normal;
        updatePrimitives();
#ifdef HB_EFFECTS
        if (boundingRect().contains(event->pos())) {
            HbEffect::start(this, "decorator", "released");
        }
#endif
    }
}

/*
    \reimp
 */
void HbDecorator::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    Q_D(HbDecorator);

    if (boundingRect().contains(event->pos())) {
        if (d->mMode != QIcon::Active) {
            d->mMode = QIcon::Active;
            updatePrimitives();
        }

    } else {
        if (d->mMode != QIcon::Normal) {
            d->mMode = QIcon::Normal;
            updatePrimitives();
        }
    }
}

/*
    \reimp
 */
void HbDecorator::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LayoutDirectionChange) {
        updatePrimitives();
    }
    HbWidget::changeEvent(event);
}
