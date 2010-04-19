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



/*!
    @proto
    @hbcore
    \class HbColorScheme
    \brief HbColorScheme class is used to query colors from theme.

    HbColorScheme class has a single static functions to query colors from theme. 
     
     \sa QColor HbColorScheme::color(const QString &colorRole)
     
    This method is appropriate for custom widgets which want to be consistent with standard 
    look and feel and be themable. For example, a custom widget may want to have background color 
    same as that of HbDialog. In such scenario the widget can query standard "popupbackground" color
    from theme.
    Following code queries color of popoup forground and applies it to a text item.
    \code
    QColor col = HbColorScheme::color("popupforeground");
    if (col.isValid()) {
       mytextitem->setTextColor(col);
    }
    \endcode
    
    Note:  
       - Theming framework schedules redraw for all graphics items when theme changes, so widgets 
    need not to do it again by themselves. It is sufficient that widgets make sure that colors being
    used to paint the graphics item has been updated in the handler of HbEvent::ThemeChanged.
       
       - The color-role may contain information about the state also, e.g. "foreground_enabled".
    
    \warning List of standard color roles is not yet finalized
*/ 

/*!
 * \fn QColor HbColorScheme::color(const QString &colorRole))
 * This function returns value of some predefined \a colorRole. 
 *
 * See class level document for detailed example
 *.
 */

#include <hbcolorscheme.h>
#include "hbcolortheme_p.h"

QColor HbColorScheme::color( const QString &colorRole )
{
    return HbColorTheme::global()->color(colorRole);
}
