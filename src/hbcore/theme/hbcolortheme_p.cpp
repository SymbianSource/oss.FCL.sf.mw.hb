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

void HbColorThemePrivate::setCurrentTheme(const QString& themeName)
{
    // If new theme is different from earlier set theme
    if (currentTheme != themeName) {
        bool reloadAll = currentTheme.isEmpty();
        currentTheme = themeName;
        reloadColorFiles( reloadAll );
    }
}

/*!
 *  HbColorThemePrivate::reloadColorFiles
 *
 *  \a sender can be true or false depending upon whether all css files need to be reloaded or is done selectively 
 */
void HbColorThemePrivate::reloadColorFiles(bool sender)
{
    QMap<int,QString> hierarchyVariableListWithPathInfo =
            HbThemeUtils::constructHierarchyListWithPathInfo("variables/color/hbcolorgroup.css", currentTheme, Hb::StyleSheetResource);
    HbStandardDirs::findResourceList(hierarchyVariableListWithPathInfo,Hb::StyleSheetResource, true);

#ifdef THEME_SERVER_TRACES    
    qDebug() << "CSS files:";
    foreach ( const QString& file, hierarchyVariableListWithPathInfo )
        qDebug() << file;
#endif // THEME_SERVER_TRACES

    cssif.initialise(hierarchyVariableListWithPathInfo, sender);

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

Q_GLOBAL_STATIC(HbColorTheme, globalColorTheme)
HbColorTheme *HbColorTheme::self = 0;

HbColorTheme *HbColorTheme::instance ()
{
    return globalColorTheme();
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
    self = this;
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
    d->setCurrentTheme(themeName);
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

void HbColorTheme::flushVariableCache()
{
    Q_D(HbColorTheme);
    d->cssif.flushVariableCache();
}
