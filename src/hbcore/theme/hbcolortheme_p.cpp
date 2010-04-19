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

#include "hbcolortheme_p.h"
#include "hbcolortheme_p_p.h"
#include "hbstandarddirs_p.h"
#include "hbinstance.h"
#include "hbthemeutils_p.h"

#include <QDebug>
#include <QGraphicsWidget>

/*!
 *  Private class Constructor 
 */
HbColorThemePrivate::HbColorThemePrivate()
{
    
}

/*!
 *  HbColorThemePrivate::reloadColorFiles
 *
 *  \a sender can be true or false depending upon whether all css files need to be reloaded or is done selectively 
 */
void HbColorThemePrivate::reloadColorFiles(bool sender)
{
    QMap<int,QString> hierarchyVariableListWithPathInfo = HbThemeUtils::constructHierarchyListWithPathInfo("variables/color/hbcolorgroup.css", currentTheme, Hb::StyleSheetResource);
    QMap<int,QString> variableFileListWithInfo = HbStandardDirs::findResourceList(hierarchyVariableListWithPathInfo,Hb::StyleSheetResource);

#ifdef THEME_SERVER_TRACES    
    qDebug() << "CSS files:";
    foreach ( const QString& file, variableFileListWithInfo )
        qDebug() << file;
#endif // THEME_SERVER_TRACES

    cssif.initialise( variableFileListWithInfo, sender );

}

/*!
 *  HbColorThemePrivate::resolveColor
 *
 *  \a values vector containing color information 
 */
QColor HbColorThemePrivate::resolveColor( HbCss::Value value ) const
{
    return value.variant.toColor();
}

/*!
 *  Destructor
 */
HbColorThemePrivate::~HbColorThemePrivate()
{
    
}


/*!
    @proto
    @hbcore
    \class HbColorTheme
    \brief HbColorTheme class is used to query colors from theme.

    HbColorTheme is a singleton class which should be used to query colors
    from theme. Colors can be queried in two ways:
    
    - By passing widget pointer and one of its supported color attribute names.
    - By passing some predefined standard attribute name.
    
    First method makes it possible for theme developer to specify instance specific colors.
    For example, a theme developer can specify that a button with "ok" id (object-name) should use
    different colors than normal buttons. This method is appropriate for the widgets which comes as
    a part of Hb library, so that those widgets allow theme developer to write any complex theme.
    Following example shows how a color for textitem of popup can be queried from theme:
    \code
    // textItem is child of HbDialog widget here
    QColor col = HbColorTheme::global()->color(textItem->parentWidget(), "foreground");
    if (col.isValid()) {
        textItem->setTextColor(col);
    }
    \endcode
    Attribute name can include state name also (<attribute_name>.<state_name>). There are four
    supported states "enabled", "disabled", "focused" or "nonfocused". If no state is defined
    color for "enabled" state is returned.
    
    \warning  The list of states and default state may change in the future depending on the
    requirements. Also list of attributes may grow and change as more widgets come in future.
 
    Second method is more appropriate for custom widgets which want to be consistent with standard
    look and feel and  be themable. For example, a custom widget may want to have background color
    same as that of HbDialog. In such scenario the widget can query standard "popupbackground" color
    from theme.
    Following code queries color of popoup forground and applies it to a text item.
    \code
    QColor col = HbColorTheme::global()->color("popupforeground");
    if (col.isValid()) {
       mytextitem->setTextColor(col);
    }
    \endcode
    
    Note: It is custom widgets responsibility to query color again from theme when theme changes.
    This can be done by handing HbEvent::ThemeChanged event. Theming framework makes sure that this
    event is sent to all HbWidgets.
    
    \warning List of standard color roles is not yet finalyzed
*/

/*!
 * \fn HbColorTheme::global ()
 * This function returns singleton instanace of the class.
 */

/*!
 * \fn QColor HbColorTheme::color(const QGraphicsWidget * widget, const QString &colorAttribute,int state) const
 * This function returns value of some \a colorAttribute for a particular \a widget with particular \a state.
 *
 * See class level document for detailed example
 * 
 */

/*!
 * \fn HbColorTheme::color(const QString &colorRole,int state) const
 * This function returns value of some predefined \a colorRole with particular \a state.
 *
 * See class level document for detailed example.
 */
HbColorTheme *HbColorTheme::global ()
{
    static HbColorTheme instance;
    return &instance;
}

/*!
 *  HbColorTheme::color
 *
 *  \a widget           parent widget 
 *  \a colorAttribute   attribute for which color information is being requested
 */
QColor HbColorTheme::color( const QGraphicsWidget * widget,
                           const QString &colorAttribute ) const
{
    Q_D(const HbColorTheme);

    HbCss::Value value = d->cssif.findAttribute( widget, colorAttribute );
    return d->resolveColor( value );
}

/*!
 *  HbColorTheme::color
 *
 *  \a colorRole        colorRole can be either "foreground" or "background" etc. 
 */
QColor HbColorTheme::color( const QString &colorRole ) const
{
    Q_D(const HbColorTheme);
    HbCss::Value value = d->cssif.findVariable( colorRole );
    return d->resolveColor( value );
}

/*!
 *  HbColorTheme::HbColorTheme()
 *
 *  Constructor 
 */
HbColorTheme::HbColorTheme (): d_ptr( new HbColorThemePrivate )
{
    
}

/*!
 *  HbColorTheme::~HbColorTheme()
 *
 *  Destructor
 */
HbColorTheme::~HbColorTheme ()
{
    delete d_ptr;
}

/*!
 *  HbColorTheme::setCurrentTheme()
 *
 *  \a themeName    name of the new theme to be set
 */
void HbColorTheme::setCurrentTheme ( const QString& themeName )
{
    Q_D(HbColorTheme);
    // If new theme is different from earlier set theme
    if ( d->currentTheme != themeName ) {
        bool reloadAll = d->currentTheme.isEmpty();
        d->currentTheme = themeName;
        d->reloadColorFiles( reloadAll );
    }
}

/*!
 *  HbColorTheme::reloadCss()
 *
 *  Reloads the color-related css files for the current theme
 */
void HbColorTheme::reloadCss()
{
    Q_D(HbColorTheme);
    d->cssif.flush();
    d->reloadColorFiles( true );
}
