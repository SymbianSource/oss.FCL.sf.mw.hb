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

    \brief The HbToolTip class provides a set of static functions that augment the
    standard features provided by the tooltip framework. 
    
    A tooltip is a short piece of text that opens in a balloon, usually in response to
    the user tapping and holding on a widget or action button. The text provides a brief 
    explanation of the purpose of the item. 
    
    The easiest way to set a tooltip on a widget or action is to call the \c setToolTip() 
    function provided by QGraphicsItem and QAction, respectively. Both QGraphicsItem and 
    QAction also provide a \c toolTip() getter function. After setting the tooltip, you 
    can generally simply let the tooltip framework handle showing the tooltip when the 
    user taps and holds on the item and hiding the tooltip afterwards. The framework 
    shows the tooltip with the \c Qt::AlignTop alignment flag, which means that the tooltip
    appears above the top edge of the widget. 
    
    When the tooltip framework detects a tap and hold event, it sends a 
    QGraphicsSceneHelpEvent to the QGraphicsItem that is topmost in the scene under the 
    press point and has a non-empty tooltip text. If the QGraphicsItem accepts 
    the event, the framework does not show the tooltip. This means that when necessary 
    classes that are derived from QGraphicsItem can implement their own logic to show 
    the tooltip; for example, if you want the tooltip to appear to the right of the widget 
    rather than in the default position above it. To do this, the implementation calls
    one of the HbToolTip::showText() static function overloads to display the 
    tooltip.

    The %HbToolTip class has other static functions that provide additional features.
    For example, you can call HbToolTip::isVisible() to find out whether a tooltip is 
    currently displayed and HbToolTip::text() to get the text that is currently visible.

    \section _usecases_hbtooltip Using the HbToolTip class

    This example shows how a custom widget class that is derived fromQGraphicsItem can 
    handle the QGraphicsSceneHelpEvent event to provide custom logic for showing a tooltip. 
    
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,25}

    \sa QGraphicsItem, QAction
*/

/*!
    This is analogous to calling 
    HbToolTip::showText(const QString&,QGraphicsItem*,const QRectF&,Qt::Alignment)
    with the \a rect argument set to the widget's bounding rectangle
    (QGraphicsItem::boundingRect()).

    \overload
*/
void HbToolTip::showText( const QString &text, QGraphicsItem *item,
                          Qt::Alignment preferredAlignment )
{
    showText( text, item, item ? item->boundingRect() : QRectF(), preferredAlignment );
}

/*!
    Shows a given text as a tooltip for a widget using a prefererred alignment. There
    is an additional option to specify a region within the widget that accepts the 
    events that trigger the appearance of the tooltip. If this argument is specified,
    the tooltip disappears when the user's finger moves out of the defined region.
    
    Specify the preferred alignment of the tooltip relative to the side of the widget 
    using one of the following flags or flag combinations:
    
    - \c Qt::AlignTop
    - \c Qt::AlignRight
    - \c Qt::AlignLeft
    - \c Qt::AlignTop | \c Qt::AlignRight
    - \c Qt::AlignTop | \c Qt::AlignLeft
    
    For example, \c Qt::AlignTop aligns the lower edge of the tool tip with the top edge
    of the widget; \c Qt::AlignTop | \c Qt::AlignLeft aligns the lower-right corner of the 
    tooltip with the top-left corner of the widget; and \c Qt::AlignTop | \c Qt::AlignRight
    aligns the lower-left corner of the tooltip with the top-right corner of the widget.

    If the layout is right-to-left, the horizontal alignments are effectively reversed 
    (for example, \c Qt::AlignRight becomes \c Qt::AlignLeft), using the rules described 
    in \c QStyle::visualAlignment().

    If the tooltip cannot be aligned properly using the specified alignment without the
    tooltip intersecting the widget's bounding rectangle, this function uses another of the 
    supported alignment options.
    
    You can use this function to hide a tooltip by setting \a text to empty or the \a item
    parameter to 0.
    
    \overload
        
    \param text The text to be displayed. If this is empty, the function hides the 
                tooltip.
    \param item The widget the tooltip applies to. If this is 0, the function 
                hides the tooltip.
    \param rect The region within the widget that the user can tap to display the tooltip.
                This is expressed in the coordinates of \a item.
    \param preferredAlignment The alignment of the tooltip relative to the side of \a item. 
                This must be one of the supported alignment flags or flag combinations 
                specified above. Any other alignment flags are ignored.
*/
void HbToolTip::showText( const QString &text, QGraphicsItem *item, const QRectF &rect,
                          Qt::Alignment preferredAlignment )
{
    if (!item) {
        return;
    }
    HbGraphicsScene *scene = qobject_cast<HbGraphicsScene *>( item->scene() );
    if ( scene ) {
        HbToolTipLabel *toolTip = scene->d_ptr->toolTip();
        if ( text.isEmpty() ) { // empty text means hide current tip
            toolTip->hide();
        } else {
            toolTip->setText(text);
            toolTip->setRect( item->mapRectToScene(rect) );
            toolTip->showText( item, preferredAlignment );
        }
    }
}

/*!
    Hides the visible tooltip in \a scene. This is equivalent to calling showText() 
    with an empty string or with the \a item parameter set to 0.

    \sa showText()
*/
void HbToolTip::hideText( HbGraphicsScene *scene )
{
    if ( scene ) {
        HbToolTipLabel *toolTip = scene->d_ptr->toolTip();
        if (toolTip->isVisible()) {
            toolTip->hide();
        } else {
            //reset tooltip timers
            toolTip->hideTextImmediately();
        }
    }
}

/*!
    Returns true if this tooltip is currently visible.

    \sa showText()
 */
bool HbToolTip::isVisible( const HbGraphicsScene *scene )
{
    if ( scene ) {
        return scene->d_ptr->toolTip()->isVisible();
    } else {
        return false;
    }
}

/*!
   If the tooltip is visible, this returns the tooltip text; otherwise this function
   returns an empty string.
 */
QString HbToolTip::text( const HbGraphicsScene *scene )
{
    if ( scene ) {
        return scene->d_ptr->toolTip()->text();
    }
    return QString();
}
