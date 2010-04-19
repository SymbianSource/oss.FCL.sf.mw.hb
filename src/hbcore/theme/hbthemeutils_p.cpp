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

#include "hbthemeutils_p.h"

#include <QLocale>
#include <QSettings>
#include <QFile>
#include <QtDebug>
#include <QDir>
#include <QMap>

#include <hbapplication.h>

#include "hbstandarddirs_p.h"
#include "hbiniparser_p.h"
#include "hblayeredstyleloader_p.h"
#include "hbthemecommon_p.h"

static const QString iconsResourceFolder("icons");
static const QString effectsResourceFolder("effects");
static const QString styleResourceFolder("style");
static const QString themeResourceFolder("theme");

static const QString &getResourceFolderName(Hb::ResourceType resType)
{
    switch(resType) {
    case Hb::IconResource:
        return iconsResourceFolder;
    case Hb::EffectResource:
        return effectsResourceFolder;
    case Hb::ThemeResource:
        return themeResourceFolder;
    case Hb::StyleSheetResource:
        return styleResourceFolder;
    default:
        break;
    }
    // This just to avoid warning
    return iconsResourceFolder;
}

/*!
    @proto
    @hbcore
    \class HbThemeUtils

    \brief HbThemeUtils provides some helper function to be used by theming system.

    Currently HbThemeUtils class has functions to add and remove theme hierarchies
    and query current list of theme hierarchies. It also has a function to provide hierarchylist.
    In future the class can include more functionalities as and when reuired.
*/


class HbThemeUtilsPrivate
{
public:
    HbThemeUtilsPrivate()
    {
         // add the operator level, app level and platform level hierarchies in the hierarchy list.
        hierarchy<<HbThemeUtils::operatorHierarchy()
                <<HbThemeUtils::appHierarchy()
                <<HbThemeUtils::platformHierarchy();
        // @todo: The operator name has been hard-coded here. Will be removed once it is decided on how to
        // get the operator name.
        operatorName = "myoperator";
    }

    QString operatorName;
    QStringList hierarchy;
};

static HbThemeUtilsPrivate d;

/* Adds a new hierarchy level to be used for attribute look-up
 * 
 * @param newHierrachy the name of the new hierrachy
 * @param priorityOrder priority order of the new hierarchy top be added.
 * if priorityOrder is greater than the currently existing hierarchies, this hierarchy will be appended
 * at the end of the hierarchy list
 *
 * @return the positon in the new hierarchy in the hierarchy list. -1 if the new hierarchy is not added.
 */
int HbThemeUtils::addHierarchy(const QString &newHierarchy, int priorityOrder)
{    
    int retValue = -1;
    if (priorityOrder >= 0) {
        // check that the hierarchy to be added is neither of opertor level,app level and platform level.
        if(newHierarchy != HbThemeUtils::operatorHierarchy() 
           && newHierarchy != HbThemeUtils::appHierarchy() 
           && newHierarchy != HbThemeUtils::platformHierarchy()){
            // if priority given is more than the number of hierarchies already existing, append the new
            // hierarchy at end.
            if (priorityOrder > d.hierarchy.count()) {
                d.hierarchy.append(newHierarchy);
                retValue = d.hierarchy.count() - 1;
            }
            // else insert it at the correct position
            else {
                d.hierarchy.insert(priorityOrder, newHierarchy);
                retValue = priorityOrder;
            }
        }
    }
    return retValue;
}


/* Removes a hierarchy level from the hierarchy list
 *
 * @param newHierrachy the name of the hierrachy to be removed.
 *
 * @ret true if the hierarchy has been removed, else false.
 */
bool HbThemeUtils::removeHierarchy(const QString &hierarchy)
{
    bool retValue = false;
    // check whether an attempt is made to remove operator level, app level or platform level hierarchy
    if (hierarchy != HbThemeUtils::operatorHierarchy() 
        && hierarchy != HbThemeUtils::appHierarchy() 
        && hierarchy != HbThemeUtils::platformHierarchy()) {
        retValue = d.hierarchy.removeOne(hierarchy);
    }
    return retValue;
}

/* @ret hierarchy of themes in priority.
 */
QStringList HbThemeUtils::hierarchy()
{
   return d.hierarchy; 
}

/* It constructs the hierarchy list with complete path info using the existing hierarchy list.
 * 
 * @param fileName  name of the file to be appended at end of all hierarchy levels
 * @param currentTheme Name of the currently used theme
 * @param resType type of Resource whether "style" or "icons"
 *
 * @ret list of hierarchy of themes in priority.Also appends the default path with least priority.
 */
QMap<int, QString> HbThemeUtils::constructHierarchyListWithPathInfo(const QString &fileName,
                                                                   const QString &currentTheme,
                                                                   const Hb::ResourceType resType)
{
    Q_UNUSED(currentTheme);
    QMap<int,QString> hierarchyListWithPathInfo;

    // Map the resource enum to string here
    const QString &resourceFolder = getResourceFolderName(resType);
    
    foreach (const QString &hierarchy, HbThemeUtils::hierarchy()) {
        if (hierarchy == HbThemeUtils::operatorHierarchy()) {
            hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Operator, (hierarchy + '/' + resourceFolder + '/' + d.operatorName + '/' + fileName));
        }
        else if (hierarchy == HbThemeUtils::appHierarchy()) {
            QString exebasename = QFileInfo(QCoreApplication::applicationFilePath()).baseName();
            hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Application, (hierarchy + '/' + exebasename + '/' + resourceFolder + '/' + currentTheme + '/' + fileName));
        }
        else if(hierarchy == HbThemeUtils::platformHierarchy()) {
            hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Theme, (hierarchy + '/' + resourceFolder + '/' + currentTheme + '/' + fileName));
        }
        else {
            // this is for a new hierarchy level and for the time being HbLayeredStyleLoader::Priority_Theme prirority is used,since there is no enum defined in hblayeredstyleloader_p.h
            // priority should be replaced with respective enum.
            hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Theme, (hierarchy + '/' + resourceFolder + '/' + currentTheme + '/' + fileName));
        }
    }
    
    if (resType == Hb::StyleSheetResource) {
        // lets add default CSS path too in this list for now
        // This comes last in fallback hierarchy
        hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Core, ("themes/" + resourceFolder + '/' + defaultTheme() + '/' + fileName));
    }

    return hierarchyListWithPathInfo;
}



/* returns name of default theme
 */

QString HbThemeUtils::defaultTheme()
{
    static QString defaultThemeName;
    
    // defaultThemeName is empty, means it was not yet filled with appropriate value
    if (defaultThemeName.isEmpty()) {
        HbIniParser iniParser;

        // First check whether it is already stored in QSettings
        QSettings&  settings = getThemeSettings();
        defaultThemeName = settings.value("defaulttheme").toString();

        // if not in QSettings, read from theme.theme file
        if (defaultThemeName.isEmpty()) {
            // Find theme.theme file
            QString dir = "themes";
            QString masterThemeFile = HbStandardDirs::findResource( dir + '/' + "themes" +
                '/' + "theme.theme", Hb::ThemeResource);

            // Try to read file and get parameters
            QFile themeFile(masterThemeFile);
            if (!themeFile.open(QIODevice::ReadOnly) || !iniParser.read(&themeFile)){
                qDebug() << "Can't access file";
                return false;
            } 

            //Find default theme index.theme file and clean it
            defaultThemeName = iniParser.value("Default Theme", "Name").trimmed();
            //Save Default theme
            settings.setValue("defaulttheme", defaultThemeName);        
        }
        else {
        
            QString cleanDefThemeName = defaultThemeName.trimmed();         
            // if stored default theme name is not clean, store the cleaned theme name
            // (stored theme name may not be clean in case old implementaion which did not
            //  handle dirty theme name, was run on the same device earlier.)
            if (cleanDefThemeName != defaultThemeName) {
                defaultThemeName = cleanDefThemeName;
                settings.setValue("defaulttheme", defaultThemeName);
            }
        }
    }
    
    return defaultThemeName;
}

/* returns settings for the theme
 */
QSettings& HbThemeUtils::getThemeSettings()
    {
     static QSettings settings(QLatin1String(ORGANIZATION), QLatin1String(THEME_COMPONENT));
     return settings;
    }
   
