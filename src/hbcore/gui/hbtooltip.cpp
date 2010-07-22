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

#include "hbtooltip.h"
#include "hbtooltiplabel_p.h"
#include "hbinstance.h"
#include "hbgraphicsscene.h"
#include "hbgraphicsscene_p.h"

#include <QGraphicsItem>

/*!
    @stable
    @hbcore
    \class HbToolTip

    \brief The HbToolTip class provides tool tips (balloon help) for any graphics item.

    \mainclass

    The tip is a short piece of text reminding the user of the
    graphics item's function.

    The simplest and most common way to set a graphics item's tool tip is by
    calling its QGraphicsItem::setToolTip() function.

    When a tooltip is currently on
    display, isVisible() returns true and text() the currently visible
    text.

    By default the tool tip framework is responsible for showing and hiding the tool tips
    on the scene. When the framework detects a long mouse press event it checks what is the
    top most QGraphicsItem on the scene having non empty tooltip text under the press point and
    sends an QGraphicsSceneHelpEvent to it. This makes it possible that QGraphicsItem derived
    objects can implement their own tool tip showing logic. If the sent QGraphicsSceneHelpEvent
    event is accepted by the QGraphicsItem object then framework does nothing. Otherwise it shows
    the tool tip text with Qt::AlignTop alignment.

    In rare cases QGraphicsItem derived classes can handle QGraphicsSceneHelpEvent to provide
    custom tool tip showing logic. An example on how to get custom tooltip alignment
    on a QGraphicsItem derived class.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,25}

    \sa QGraphicsItem::toolTip, QAction::toolTip
*/

/*!
    \overload

    This is analogous to calling HbToolTip::showText(\a text, \a item,
                                                     item->boundingRect(), \a preferredAlignment)
*/
void HbToolTip::showText( const QString &text, QGraphicsItem *item,
                          Qt::Alignment preferredAlignment )
{
    showText( text, item, item ? item->boundingRect() : QRectF(), preferredAlignment );
}

/*!
    Shows \a text as a tool tip, with the prefererred alignment \a preferredAlignment in relation
    to \a item.

    If you specify a non-empty rect the tip will be hidden as soon
    as you move your cursor out of this area.

    The \a rect is in the coordinates of the graphics item you specify with
    \a item.

    If \a text is empty or the item 0 the tool tip is hidden.

    The supported alignment flags are Qt::AlignTop, Qt::AlignRight, Qt::AlignLeft,
    Qt::AlignTop|Qt::AlignRight, Qt::AlignTop|Qt::AlignLeft. Any other alignment flags are ignored.
    Alignment flags are referring to the side of \a item i.e. when \a preferredAlignment
    equals Qt::AlignTop the tool tip is aligned to the top of \a item and so on.
    When Qt::AlignTop|Qt::AlignLeft combination is used the bottom-right corner of the tool tip is
    aligned to the top-left corner of \a item. When Qt::AlignTop|Qt::AlignRight combination is used
    the bottom-left corner of the tool tip is aligned to the top-right corner of \a item.

    HbToolTip uses QStyle::visualAlignment() to transforms \a preferredAlignment according to the
    current layout direction. Qt::AlignAbsolute is supported the way is described
    in QStyle::visualAlignment().

    If the tool tip could not be aligned properly i.e. without intersecting \a item's bounding
    rectangle by using the provided \a preferredAlignment then it will be aligned by another
    supported alignment option.

    If none of the supported alignment options results proper alignment then Qt::AlignTop will
    be used.

*/
void HbToolTip::showText( const QString &text, QGraphicsItem *item, const QRectF &rect,
                          Qt::Alignment preferredAlignment )
{
    if (!item) {
        return;
    }
    HbGraphicsScene *scene = qobject_cast<HbGraphicsScene *>( item->scene() );
    if ( scene && text.isEmpty() ) { // empty text means hide current tip
        scene->d_ptr->mToolTip->hide();
    } else if (scene && scene->d_ptr->mToolTip) {

        scene->d_ptr->mToolTip->setText(text);
        scene->d_ptr->mToolTip->setRect( item->mapRectToScene(rect) );
        scene->d_ptr->mToolTip->showText( item, preferredAlignment );
    }
}

/*!
    \fn void HbToolTip::hideText()

    Hides the visible tooltip in the \a scene. This is the same as calling showText() with an
    empty string or item parameter as 0.

    \sa showText()
*/
void HbToolTip::hideText( HbGraphicsScene *scene )
{
    if ( scene && scene->d_ptr->mToolTip) {
        if (scene->d_ptr->mToolTip->isVisible()) {
            scene->d_ptr->mToolTip->hide();
        } else {
            //reset tooltip timers
            scene->d_ptr->mToolTip->hideTextImmediately();
        }
    }
}

/*!
  Returns true if this tooltip is currently shown.

  \sa showText()
 */
bool HbToolTip::isVisible( const HbGraphicsScene *scene )
{
    if ( scene && scene->d_ptr->mToolTip ) {
        return scene->d_ptr->mToolTip->isVisible();
    } else {
        return false;
    }
}

/*!
  Returns the tooltip text, if a tooltip is visible, or an
  empty string if a tooltip is not visible.
 */
QString HbToolTip::text( const HbGraphicsScene *scene )
{
    if ( HbToolTip::isVisible(scene) ) {
        return scene->d_ptr->mToolTip->text();
    }
    return QString();
}
