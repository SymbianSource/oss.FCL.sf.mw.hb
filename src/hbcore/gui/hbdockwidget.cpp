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

#include "hbdockwidget.h"
#include "hbdockwidget_p.h"

#include <QGraphicsSceneResizeEvent>

/*!
    @stable
    @hbcore
    \class HbDockWidget
    \brief HbDockWidget represents a dock widget decorator.

    The HbDockWidget class represents dock widget on each application. 
    It provides interface for setting the widget.

    The dock widget can be emptied with 0.
*/

/*!
    \reimp
    \fn int HbDockWidget::type() const
 */

HbDockWidgetPrivate::HbDockWidgetPrivate() :
    HbWidgetPrivate(),
    mWidget(0)
{
}

HbDockWidgetPrivate::~HbDockWidgetPrivate()
{
}

// ======== MEMBER FUNCTIONS ========

/*!
    Constructs a dock widget with \a parent.
*/

HbDockWidget::HbDockWidget( QGraphicsItem *parent )
    : HbWidget( *new HbDockWidgetPrivate, parent )
{
    Q_D(HbDockWidget);
    d->q_ptr = this;
    setFlag( QGraphicsItem::ItemClipsChildrenToShape, true );
}

/*!
    Protected constructor.
*/
HbDockWidget::HbDockWidget( HbDockWidgetPrivate &dd, QGraphicsItem *parent )
    : HbWidget( dd, parent )
{
    Q_D(HbDockWidget);
    d->q_ptr = this;
    setFlag( QGraphicsItem::ItemClipsChildrenToShape, true );
}

/*!
    Destructor
 */
HbDockWidget::~HbDockWidget()
{
    Q_D(HbDockWidget);
    if ( d->mWidget != 0 ) {
        d->mWidget->deleteLater();
    }
}

/*!
    Returns the widget for the dock widget. This function returns zero
    if the widget has not been set.

    \sa setWidget()
*/
QGraphicsWidget *HbDockWidget::widget() const
{
    Q_D( const HbDockWidget );
    return d->mWidget;
}

/*!
    Sets the widget for the dock widget to \a widget. Ownership of the widget \a widget is
    transferred to dock widget.

    \sa widget()
*/
void HbDockWidget::setWidget( QGraphicsWidget *widget )
{
    Q_D(HbDockWidget);
    if ( d->mWidget != widget ) {
        if ( d->mWidget != 0 ) {
            d->mWidget->deleteLater();
        }
        d->mWidget = widget;
        if ( d->mWidget != 0 ) {
            d->mWidget->setParentItem(this);
            style()->setItemName( d->mWidget, "content" );
            QMetaObject::invokeMethod( &d->core, "visibilityChanged", Qt::QueuedConnection );
        }
    }
}

/*!
    Emits visibilityChanged() whenever the dock widget's visibility or position changes.
*/
QVariant HbDockWidget::itemChange( GraphicsItemChange change, const QVariant &value )
{
    Q_D(HbDockWidget);
    QVariant result = HbWidget::itemChange( change, value );

    switch (change) {
        case ItemVisibleHasChanged:
            // Cannot emit signals directly from "itemChange", since it might
            // lead to e.g. item deletion that's not allowed in "itemChange".
            // Using QMetaObject to emit the signal asynchronously.
            if ( d->polished ) {
                QMetaObject::invokeMethod( &d->core, "visibilityChanged", Qt::QueuedConnection );
            }
            break;
        default:
            break;
    }

    return result;
}

#include "moc_hbdockwidget.cpp"
