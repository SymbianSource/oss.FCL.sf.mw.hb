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
#include <QVariant>

#include <hbapplication.h>
#include <hbtheme.h>
#include "hbstandarddirs_p.h"
#include "hbiniparser_p.h"
#include "hblayeredstyleloader_p.h"
#include "hbthemecommon_p.h"

#ifdef Q_OS_SYMBIAN
#include "hbthemecommon_symbian_p.h"
#include <e32std.h>
#endif

// Standard folder names

const char *HbThemeUtils::iconsResourceFolder = "icons";
const char *HbThemeUtils::effectsResourceFolder = "effects";
const char *HbThemeUtils::styleResourceFolder = "style";
const char *HbThemeUtils::themeResourceFolder = "theme";
const char *HbThemeUtils::operatorHierarchy = "operatortheme";
const char *HbThemeUtils::appHierarchy = "apptheme";
const char *HbThemeUtils::platformHierarchy = "themes";

const char *operatorBasePathKey = "OperatorBasePath";
static const char *themeSettingFile = "theme.theme";
static const char *baseThemeVariable = "BaseTheme";
static const char *defaultThemeVariable = "DefaultActiveTheme";

// These are the used setting names corresponding to HbThemeUtils::Setting enumeration.
// Value 0 is not used to be able to change the implementation to use Symbian's Cenrep if needed.
static const QString settingNames[6] = {"", "currenttheme", "defaulttheme", "defaultthemedir", "basetheme", "operatorbasepath"};

static const char *getResourceFolderName(Hb::ResourceType resType)
{
    switch(resType) {
    case Hb::IconResource:
        return HbThemeUtils::iconsResourceFolder;
    case Hb::EffectResource:
        return HbThemeUtils::effectsResourceFolder;
    case Hb::ThemeResource:
        return HbThemeUtils::themeResourceFolder;
    case Hb::StyleSheetResource:
        return HbThemeUtils::styleResourceFolder;
    default:
        break;
    }
    // This just to avoid warning
    return HbThemeUtils::iconsResourceFolder;
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
    HbThemeUtilsPrivate() : settingsRetrieved(false)
    {
         // add the operator level, app level and platform level hierarchies in the hierarchy list.
        hierarchies << HbHierarchy(HbThemeUtils::operatorHierarchy, HbLayeredStyleLoader::Priority_Operator)
#ifdef USE_APPTHEMES
                    << HbHierarchy(HbThemeUtils::appHierarchy, HbLayeredStyleLoader::Priority_Application)
#endif
                    << HbHierarchy(HbThemeUtils::platformHierarchy, HbLayeredStyleLoader::Priority_Theme);
        // @todo: The operator name has been hard-coded here. Will be removed once it is decided on how to
        // get the operator name.
        operatorName = "myoperator";
    }
    QString constructOperatorPath(const QString &basePath, const QString &resourcePath, const QString &fileName) const
    {
        return basePath + resourcePath + '/' + operatorName + '/' + fileName;
    }
    void initSettings();

    void readSettings();

public: // data
    QString operatorName;
    QVector<HbHierarchy> hierarchies;

    bool settingsRetrieved;
    // Setting values are stored here to avoid overhead of reading from QSettings every time.
    QString currentTheme;
    QString defaultTheme;
    QString defaultThemeRootDir;
    QString baseTheme;
    QString operatorBasePath;
};

void HbThemeUtilsPrivate::initSettings()
{
    //server gets and stores the operator path to settings, clients only read it.
    if (HbMemoryUtils::getCleanAppName()== THEME_SERVER_NAME) {
        QStringList operatorPath;
        operatorPath << QLatin1String(HbThemeUtils::operatorHierarchy) + '/';
        operatorPath = HbStandardDirs::findExistingFolderList(operatorPath, QString(), Hb::IconResource);
        if (operatorPath.size() > 0) {
            operatorBasePath = operatorPath.at(0);
        }
        HbThemeUtils::setThemeSetting(HbThemeUtils::OperatorBasePathSetting, operatorBasePath);
    } else {
        operatorBasePath = HbThemeUtils::getThemeSetting(HbThemeUtils::OperatorBasePathSetting).trimmed();
    }
}

void HbThemeUtilsPrivate::readSettings()
{
    // Read settings from QSettings and store them in member variables to
    // avoid slow instantiating of QSettings in advance.

    // The only changing setting is currentThemeSetting and its value is updated in theme change event.

    if (!settingsRetrieved) {
        QSettings settings(QLatin1String(ORGANIZATION), QLatin1String(THEME_COMPONENT));

        currentTheme = settings.value(settingNames[HbThemeUtils::CurrentThemeSetting]).toString();
        defaultTheme = settings.value(settingNames[HbThemeUtils::DefaultThemeSetting]).toString();
        defaultThemeRootDir = settings.value(settingNames[HbThemeUtils::DefaultThemeRootDirSetting]).toString();
        baseTheme = settings.value(settingNames[HbThemeUtils::BaseThemeSetting]).toString();
        operatorBasePath = settings.value(settingNames[HbThemeUtils::OperatorBasePathSetting]).toString();

        settingsRetrieved = true;
    }
}

static HbThemeUtilsPrivate d;

void HbThemeUtils::initSettings()
{
    d.initSettings();
}

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
        if(newHierarchy != HbThemeUtils::operatorHierarchy
            && newHierarchy != HbThemeUtils::appHierarchy
            && newHierarchy != HbThemeUtils::platformHierarchy){

            // if priority given is more than the number of hierarchies already existing, append the new
            // hierarchy at end.
            HbHierarchy add(newHierarchy, HbLayeredStyleLoader::Priority_Theme);
            if (priorityOrder > d.hierarchies.count()) {
                d.hierarchies.append(add);
                retValue = d.hierarchies.count() - 1;
            }
            // else insert it at the correct position
            else {
                d.hierarchies.insert(priorityOrder,add);
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
bool HbThemeUtils::removeHierarchy(const QString &hierarchyName)
{
    bool retValue = false;
    // check whether an attempt is made to remove operator level, app level or platform level hierarchy
    if (hierarchyName != HbThemeUtils::operatorHierarchy
        && hierarchyName != HbThemeUtils::appHierarchy
        && hierarchyName != HbThemeUtils::platformHierarchy) {
        QVector<HbHierarchy>::iterator end = d.hierarchies.end();
        for (QVector<HbHierarchy>::iterator i = d.hierarchies.begin(); i != end; ++i) {
            if (i->name == hierarchyName) {
                d.hierarchies.erase(i);
                retValue = true;
                break;
            }
        }
    }
    return retValue;
}

QString HbThemeUtils::operatorBasePath()
{
    return d.operatorBasePath;
}
/* @ret hierarchy of themes in priority.
 */
QVector<HbHierarchy> HbThemeUtils::hierarchies()
{
   return d.hierarchies;
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
    QMap<int,QString> hierarchyListWithPathInfo;

    // Map the resource enum to string here
    const QString &resourcePath = getResourceFolderName(resType);

    foreach (const HbHierarchy &hierarchy, d.hierarchies) {
        switch(hierarchy.layerPriority) {
        case HbLayeredStyleLoader::Priority_Operator:
            if (!d.operatorBasePath.isEmpty()) {
                hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Operator,
                                                 d.constructOperatorPath(d.operatorBasePath, resourcePath, fileName));
            }
            break;
        case HbLayeredStyleLoader::Priority_Application:
            hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Application, 
                    (hierarchy.name + '/' + HbMemoryUtils::getCleanAppName() + '/' + resourcePath + '/' + currentTheme + '/' + fileName));
            break;
        case HbLayeredStyleLoader::Priority_Theme:
            // Add platform theme folder only if it is different from base theme
            // Base theme is anyway added at the core priority
            if (currentTheme != baseTheme().name) {
                hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Theme, 
                        (hierarchy.name + '/' + resourcePath + '/' + currentTheme + '/' + fileName));
            }
            break;
        default:
            // this is for a new hierarchy level and for the time being HbLayeredStyleLoader::Priority_Theme prirority is used,since there is no enum defined in hblayeredstyleloader_p.h
            // priority should be replaced with respective enum.
            hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Theme, 
                    (hierarchy.name + '/' + resourcePath + '/' + currentTheme + '/' + fileName));
        }
    }
    
    if (resType == Hb::StyleSheetResource || resType == Hb::EffectResource) {
        // lets add base CSS path too in this list for now
        // This comes last in base hierarchy
        hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Core, 
                (QLatin1String("themes/") + resourcePath + '/' + baseTheme().name + '/' + fileName));
    }

    return hierarchyListWithPathInfo;
}

/* returns information of base theme
 */
const HbThemeInfo &HbThemeUtils::baseTheme()
{
    static HbThemeInfo baseThemeInfo;
   
    if (baseThemeInfo.name.isEmpty()) {
        // basetheme is empty, means it was not yet filled with appropriate values      
        // Check if its value is stored in settings.
        baseThemeInfo.name = getThemeSetting(BaseThemeSetting).trimmed();
        if ( baseThemeInfo.name.isEmpty() ) {
            // Settings not yet initialized
            // Check if Base theme in rom set
            baseThemeInfo = getBaseThemeFromFile(HbStandardDirs::themesDir());
            if (baseThemeInfo.name.isEmpty()) {
                // Base theme does not exists in rom
                // Get the base theme info from core resources
                baseThemeInfo = getBaseThemeFromFile(coreResourcesRootDir);
            }
        } else {
            // So settings are initialized, it will have other value as well
            baseThemeInfo.rootDir = getThemeSetting(DefaultThemeRootDirSetting).trimmed();            
        }
    }
    
    return baseThemeInfo;
}

/* returns name of default theme
 */
HbThemeInfo HbThemeUtils::defaultTheme()
{
    // getting base theme makes sure that default theme was added in
    // QSettings, if it was not already done
    const HbThemeInfo &themeInfo = baseTheme(); 

    // Assuming the path of default theme and base theme are same
    return HbThemeInfo(getThemeSetting(DefaultThemeSetting), themeInfo.rootDir);
}

QString HbThemeUtils::getThemeSetting(Setting setting)
{
    // Make sure settings are read from QSettings.
    d.readSettings();

    switch (setting) {
        case CurrentThemeSetting:
            return d.currentTheme;
        case DefaultThemeSetting:
            return d.defaultTheme;
        case DefaultThemeRootDirSetting:
            return d.defaultThemeRootDir;
        case BaseThemeSetting:
            return d.baseTheme;
        case OperatorBasePathSetting:
            return d.operatorBasePath;
        default:
            return QString();
    }
}

void HbThemeUtils::setThemeSetting(Setting setting, const QString &value)
{
    QSettings settings(QLatin1String(ORGANIZATION), QLatin1String(THEME_COMPONENT));
    settings.setValue(settingNames[setting], QVariant(value));
    // Destructor of QSettings flushes the changed setting in the INI file.
}   

/**
* Updates the setting's value in stored member variables.
* Normally the settings are loaded from QSettings when method getThemeSetting() is called for the first time.
* When there is a change in settings, this method can be used to sync the setting value stored in HbThemeUtilsPrivate.
* E.g. theme change event updates the current theme setting, currently no other settings are changing their values.
*/
void HbThemeUtils::updateThemeSetting(Setting setting, const QString &value)
{
    switch (setting) {
        case CurrentThemeSetting:
            d.currentTheme = value;
            break;
        case DefaultThemeSetting:
            d.defaultTheme = value;
            break;
        case DefaultThemeRootDirSetting:
            d.defaultThemeRootDir = value;
            break;
        case BaseThemeSetting:
            d.baseTheme = value;
            break;
        case OperatorBasePathSetting:
            d.operatorBasePath = value;
            break;
        default:
            break;
    }
}   
 
/* reads the theme name from theme.theme file, stores the same in theme settings,
   returns the pair of theme name and its root directory
 */
HbThemeInfo HbThemeUtils::getBaseThemeFromFile(const QString &rootDir)
{
    QFile themeSetting(rootDir + '/' + platformHierarchy + '/' + themeSettingFile);
    HbThemeInfo themeInfo;
    HbIniParser iniParser;

    if (themeSetting.open(QIODevice::ReadOnly) && iniParser.read(&themeSetting)){
        themeInfo.name = iniParser.value("Default", baseThemeVariable).trimmed();
        
        QString defaultTheme = iniParser.value("Default", defaultThemeVariable).trimmed();

        // default theme name may not exist, in which case using base theme as default theme
        if (defaultTheme.isEmpty()) {
            defaultTheme = themeInfo.name;
        }

        // If there is any base theme
        if (!themeInfo.name.isEmpty() && isThemeValid(HbThemeInfo(themeInfo.name,rootDir))) {
            // Save these theme names in settings
            setThemeSetting(BaseThemeSetting, themeInfo.name);
            setThemeSetting(DefaultThemeRootDirSetting, rootDir);

            // Store default theme also in settings, only if it is valid
            if (themeInfo.name == defaultTheme || isThemeValid(HbThemeInfo(defaultTheme, rootDir))) {
                setThemeSetting(DefaultThemeSetting, defaultTheme);
            }
            themeInfo.rootDir = rootDir;
            d.settingsRetrieved = false;
        }
    }
    return themeInfo;
}

/* checks whether the theme is valid
 */
bool HbThemeUtils::isThemeValid(const HbThemeInfo &themeInfo)
{
    // If the theme contains index.theme in icons resources
    // it will be assumed valid
    QFile themeIndexFile(themeInfo.rootDir + '/' + platformHierarchy + '/' + iconsResourceFolder + "/" + themeInfo.name + "/index.theme");
    return themeIndexFile.open(QIODevice::ReadOnly);
}
