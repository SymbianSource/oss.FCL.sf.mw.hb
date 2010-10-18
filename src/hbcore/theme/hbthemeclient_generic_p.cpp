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

#include <hbinstance.h>
#include <hbtheme_p.h>
#include <hbtheme.h>
#include <hbthemeutils_p.h>
#include <hbevent.h>

#include "hbthemeclient_p_p.h"
#include "hbthemecommon_p.h"

#include <QDebug>
#include <QFileSystemWatcher>
#include <QSettings>

/**
 * Constructor
 */
HbThemeClientPrivate::HbThemeClientPrivate() :
        iniFileWatcher(0),
        clientConnected(false)
{
    THEME_GENERIC_DEBUG() << Q_FUNC_INFO;

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QLatin1String(ORGANIZATION), QLatin1String(THEME_COMPONENT));
    iniFileWatcher = new QFileSystemWatcher(this);
    iniFileWatcher->addPath(settings.fileName());
    connect(iniFileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(iniFileChanged(QString)));
}

/**
 * Destructor
 */
HbThemeClientPrivate::~HbThemeClientPrivate()
{

}

/**
 * HbThemeClientPrivate::iniFileChanged()
 */
void HbThemeClientPrivate::iniFileChanged(QString iniFile)
{
    Q_UNUSED(iniFile);
    THEME_GENERIC_DEBUG() << Q_FUNC_INFO;
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QLatin1String(ORGANIZATION), QLatin1String(THEME_COMPONENT));
    QString newTheme = settings.value("currenttheme").toString();

    if (!HbThemeUtils::isThemeValid(newTheme)) {
        // check if the theme name is logical
        newTheme = QDir::fromNativeSeparators(qgetenv("HB_THEMES_DIR")) +
                   '/' + HbThemeUtils::platformHierarchy + '/' +
                    HbThemeUtils::iconsResourceFolder + '/' + newTheme;
        if (!HbThemeUtils::isThemeValid(newTheme)) {
            // use default theme since new theme cannot be resolved
            newTheme = HbThemeUtils::getThemeSetting(HbThemeUtils::DefaultThemeSetting);
        }
        disconnect(iniFileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(iniFileChanged(QString)));
        HbThemeUtils::setThemeSetting(HbThemeUtils::CurrentThemeSetting, newTheme);
        connect(iniFileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(iniFileChanged(QString)));
    } else {
        HbThemeUtils::updateThemeSetting(HbThemeUtils::CurrentThemeSetting, newTheme);
    }

    HbThemeUtils::loadHeapThemeIndex(ActiveTheme);
    hbInstance->theme()->d_ptr->handleThemeChange(newTheme);
}


/**
 * HbThemeClientPrivate::setTheme() 
 */
void HbThemeClientPrivate::setTheme(const QString &theme)
{
    if (HbThemeUtils::isThemeValid(theme)) {
        HbThemeUtils::setThemeSetting(HbThemeUtils::CurrentThemeSetting, theme);
    }
}
