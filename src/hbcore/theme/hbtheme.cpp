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
#include "hbtheme.h"
#include "hbtheme_p.h"
#include <qglobal.h>
#include "hbstandarddirs_p.h"
#include "hbicontheme_p.h"
#include "hbcolortheme_p.h"
#include "hbthemeutils_p.h"
#include "hbiconloader_p.h"
#include "hbcolortheme_p_p.h"
#include "hbcolortheme_p.h"
#include "hbeffecttheme_p.h"
/*!
    @proto
    @hbcore
    \class HbTheme

    \brief HbTheme provides an interface for changing the active theme.

    HbTheme is a part of hbinstance, which is accessed with the method hbinstance->theme().
    It provides means for retrieving information of the currently active theme.

    In addition, it provides two signals for handling a theme change event.

    More information about theming see S60QtProgrammersGuide.
*/

/*!
    \fn void HbTheme::changed()

    This signal is emitted when the theme change has started.
    Components using resources such as pixmaps from a theme should make sure that
    they are updated upon this signal. 
    Classes HbIcon and HbIconItem handle this automatically
    and if icons has no special size, position etc. widgets or applications don't need 
    to connect to this signal, icons are changed automatically.
*/

/*!
    \fn void HbTheme::changeFinished()

    This signal is emitted when theme change has finished and screen update is 
    required.
    It can be assumed that when this signal is emitted, components are already
    using resources (e.g. pixmaps) from the new theme.
    HbInstance updates the graphics scenes upon this signal.
*/

/*!
    Returns static instance
 */
HbTheme* HbTheme::instance()
{
    static HbTheme theInstance;
    return &theInstance;
}

/*!
    Returns the non localized name of the currently active theme.
    
*/
QString HbTheme::name() const
{
    return d_ptr->currentTheme;
}

/*!
    Returns the non localized description of the currently active theme.
*/
QString HbTheme::description() const
{
    return HbIconTheme::global()->description();
}

/*!
    Constructor
*/
HbTheme::HbTheme() : d_ptr(new HbThemePrivate) 
{
    d_ptr->q_ptr = this;
    d_ptr->fetchCurrentThemeFromSettings();
    d_ptr->handleThemeChange();
}

/*!
    Destructor
*/
HbTheme::~HbTheme()
{
    delete d_ptr;
}

/*!
    C'tor
*/
HbThemePrivate::HbThemePrivate()
{
    // Condition added to check if the client itself is server.
    if(THEME_SERVER_NAME != HbMemoryUtils::getCleanAppName()) {
        if(!HbThemeClient::global()->connectToServer()) {
	    	qWarning()<<"ThemeClient unable to connect to server in HbThemePrivate::HbThemePrivate.";
        }
    }
}

/*!
    D'tor
*/
HbThemePrivate::~HbThemePrivate()
{
    HbThemeClient::releaseInstance();
    GET_MEMORY_MANAGER( HbMemoryManager::HeapMemory )
    if (manager) {
        manager->releaseInstance(HbMemoryManager::HeapMemory);
    }
}

/*!
    Retrieves the current theme from setting
*/
void HbThemePrivate::fetchCurrentThemeFromSettings()
{
    QSettings&  settings = HbThemeUtils::getThemeSettings();
    currentTheme = settings.value("currenttheme").toString();
    
    QString cleanCurrentTheme = currentTheme.trimmed();
    
    if (cleanCurrentTheme.isEmpty()) {
        //cleanCurrentTheme = HbThemeUtils::defaultTheme();
        // TEMP CHANGE: Use "sfblacktheme" as default theme,
        // theme.theme still contains "hbdefault" as default theme, so that is used as parent theme in lookups
        cleanCurrentTheme = "sfblacktheme";
    }
    
    // If QSettings itself does not have clean name
    if (cleanCurrentTheme != currentTheme) {
        currentTheme = cleanCurrentTheme;
        settings.setValue("currenttheme", currentTheme) ;
        settings.sync();
    }
}

/*!
    Handles theme-change request
*/
void HbThemePrivate::handleThemeChange(const QString &str)
{
    Q_Q(HbTheme);
    QString newTheme;
    if (str.isEmpty()) {
    QSettings&  settings = HbThemeUtils::getThemeSettings();
    settings.sync();
    newTheme = settings.value("currenttheme").toString();
    } else {
        newTheme = str;
    }

    // Clear the theme index table offsets as the theme has changed.
    // New offsets will be retrieved from server when the index is accessed.
    themeIndex.clear();

    HbIconTheme::global()->setCurrentTheme(newTheme);
    HbColorTheme::global()->setCurrentTheme(newTheme);
    HbEffectTheme::global()->setCurrentTheme(newTheme);
    
    // The server sends the signal only if the theme is changed from the previous theme
    // Hence here, we need not check whether the theme differs from currentTheme or not.
    if(currentTheme != newTheme) {
        currentTheme = newTheme;
        // This signal should be used to replace pixmaps from the old theme with the pixmaps from the new theme
        // In application side this is needed only when icon size can be different in different theme.
        emit q->changed ();
        // This signal should be used to update the screen after the theme change - it's handled by HbInstance.
        emit q->changeFinished();
    }
}

/*!
    Clears the contents to reload new css files
*/
void HbThemePrivate::clearCache()
{
    // Clear the icon directory list 
    HbIconTheme::global()->clearDirList();

    // Reload the CSS
    HbColorTheme::global()->reloadCss();
    
    Q_Q(HbTheme);
    // Emit the theme changed signals so that the UI is refreshed.
    emit q->changed ();
    emit q->changeFinished();
}
