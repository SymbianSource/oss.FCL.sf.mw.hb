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

#include "hbspaceritem_p.h"
#include <QWidget> // QWIDGETSIZE_MAX

class HbSpacerItemPrivate
{
};

/*!
    @proto
    @hbcore
    \class HbSpacerItem

    \brief HbSpacerItem class is used in layouts to fill the void.

    The spacer itself is invisble and therefore it can be used to fill empty areas in layouts.

    Modifying the size hint and the policy, the spacer and occupy different areas.
*/


/*!
    Constructs a spacer. By default, the preferred size is (0,0) and the size policy is (preferred,preferred).
*/
HbSpacerItem::HbSpacerItem(QGraphicsLayoutItem *parent ) : QGraphicsLayoutItem( parent ), d_ptr(0)
{
}

/*!
    Destructor
*/
HbSpacerItem::~HbSpacerItem()
{
    delete d_ptr;
}

/*!
    \reimp
 */
QSizeF HbSpacerItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED( constraint );
    if ( which == Qt::MaximumSize ) {
        return QSizeF( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
    }
    return QSizeF( 0,0 );
}

/*!
    \reimp
 */
void HbSpacerItem::updateGeometry()
{
    QGraphicsLayoutItem::updateGeometry();
    QGraphicsLayoutItem *parentItem = parentLayoutItem();

    if (parentItem && parentItem->isLayout()) {
        parentItem->updateGeometry();
    }
}

//end of file

