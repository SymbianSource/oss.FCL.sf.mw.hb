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
#include <centralrepository.h>
#endif

// Standard folder names

const char *HbThemeUtils::iconsResourceFolder = "icons";
const char *HbThemeUtils::effectsResourceFolder = "effects";
const char *HbThemeUtils::styleResourceFolder = "style";
const char *HbThemeUtils::themeResourceFolder = "theme";
const char *HbThemeUtils::operatorHierarchy = "prioritytheme";
const char *HbThemeUtils::appHierarchy = "apptheme";
const char *HbThemeUtils::platformHierarchy = "themes";

const char *operatorNameKey = "OperatorName";
static const char *themeSettingFile = "theme.theme";
static const char *baseThemeVariable = "BaseTheme";
static const char *defaultThemeVariable = "DefaultActiveTheme";

// These are the used setting names corresponding to HbThemeUtils::Setting enumeration.
static const QString settingNames[6] = {"", "basetheme", "defaulttheme",
                                        "defaultthemedir", "currenttheme", "operatorbasepath"};
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
        hierarchies << HbHierarchy(HbThemeUtils::operatorHierarchy,
                                   HbLayeredStyleLoader::Priority_Operator)
#ifdef USE_APPTHEMES
                    << HbHierarchy(HbThemeUtils::appHierarchy,
                                   HbLayeredStyleLoader::Priority_Application)
#endif
                    << HbHierarchy(HbThemeUtils::platformHierarchy,
                                   HbLayeredStyleLoader::Priority_Theme);
    }
    QString constructOperatorPath(const QString &operatorPath, const QString &fileName) const
    {
        return operatorPath + '/' + fileName;
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
    QString operatorPath;
};

void HbThemeUtilsPrivate::initSettings()
{
    // read the operator name from settings
    operatorName = HbThemeUtils::getThemeSetting(HbThemeUtils::OperatorNameSetting).trimmed();
    
    // construct operator path
    if (!operatorName.isEmpty()) {
        QStringList operatorPaths;
        operatorPaths << QLatin1String(HbThemeUtils::operatorHierarchy) + '/';
        operatorPaths = HbStandardDirs::findExistingFolderList(operatorPaths, QString(),
                                                               Hb::IconResource);
        for (int i=0;i < operatorPaths.size();i++) {
            if (operatorPaths[i] == operatorName) {
            operatorPath = operatorPaths[i] + '/' + operatorName;
            break;
            }
        }
    }    
}

void HbThemeUtilsPrivate::readSettings()
{
    // The only changing setting is currentThemeSetting and its value is updated in server side in theme change event.

    if (!settingsRetrieved) {
#ifdef Q_OS_SYMBIAN
        CRepository *repository = 0;
        TRAP_IGNORE(repository = CRepository::NewL(KServerUid3));
        if (repository) {
            TBuf<256> value;
            if (KErrNone == repository->Get(HbThemeUtils::CurrentThemeSetting, value)) {
                QString qvalue((QChar*)value.Ptr(), value.Length());
                currentTheme = qvalue.trimmed();
            }
            value.Zero();            
            if (KErrNone == repository->Get(HbThemeUtils::DefaultThemeSetting, value)) {
                QString qvalue((QChar*)value.Ptr(), value.Length());
                defaultTheme = qvalue.trimmed();
            }
            value.Zero();
            if (KErrNone == repository->Get(HbThemeUtils::DefaultThemeRootDirSetting, value)) {
                QString qvalue((QChar*)value.Ptr(), value.Length());
                defaultThemeRootDir = qvalue.trimmed();
            } else {
                // Use the default value
                defaultThemeRootDir = HbStandardDirs::themesDir();           
            }
            value.Zero();
            if (KErrNone == repository->Get(HbThemeUtils::BaseThemeSetting, value)) {
                QString qvalue((QChar*)value.Ptr(), value.Length());
                baseTheme = qvalue.trimmed();
            }
            value.Zero();
            if (KErrNone == repository->Get(HbThemeUtils::OperatorNameSetting, value)) {
                QString qvalue((QChar*)value.Ptr(), value.Length());
                operatorName = qvalue.trimmed();
            }
            delete repository;
        }
#else
        QSettings settings(QLatin1String(ORGANIZATION), QLatin1String(THEME_COMPONENT));

        currentTheme = settings.value(settingNames[HbThemeUtils::CurrentThemeSetting]).toString();
        defaultTheme = settings.value(settingNames[HbThemeUtils::DefaultThemeSetting]).toString();
        defaultThemeRootDir =
                settings.value(settingNames[HbThemeUtils::DefaultThemeRootDirSetting]).toString();
        baseTheme = settings.value(settingNames[HbThemeUtils::BaseThemeSetting]).toString();
        operatorName = settings.value(settingNames[HbThemeUtils::OperatorNameSetting]).toString();
#endif
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
 * @return the position in the new hierarchy in the hierarchy list. -1 if the new hierarchy is not added.
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
            } else { // insert at the correct position
                d.hierarchies.insert(priorityOrder, add);
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
    return d.operatorPath;
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
#ifdef Q_OS_SYMBIAN
    foreach (const HbHierarchy &hierarchy, d.hierarchies) {
        switch(hierarchy.layerPriority) {
        case HbLayeredStyleLoader::Priority_Operator: {
                // Operator C drive path
                HbThemeIndexInfo info = HbThemeUtils::getThemeIndexInfo(OperatorC);
                if (info.themeIndexOffset > 0) {

                    hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Operator,
                            (info.path + '/' + resourcePath + '/' + info.name + '/' + fileName));
                }
                // Operator ROM path
                info = HbThemeUtils::getThemeIndexInfo(OperatorROM);
                if (info.themeIndexOffset > 0) {
                    hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Operator,
                            (info.path + '/' + resourcePath + '/' + info.name + '/' + fileName));
                }
                break;
            }
        case HbLayeredStyleLoader::Priority_Theme: {
                HbThemeIndexInfo info = HbThemeUtils::getThemeIndexInfo(ActiveTheme);
                if (info.themeIndexOffset > 0) {
                    hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Theme,
                            (info.path + '/' + resourcePath + '/' + info.name + '/' + fileName));
                }
                break;
            }
        default: {
                // this is for a new hierarchy level and for the time being HbLayeredStyleLoader::Priority_Theme prirority
                // is used,since there is no enum defined in hblayeredstyleloader_p.h
                // priority should be replaced with respective enum.
                hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Theme,
                        (hierarchy.name + '/' + resourcePath + '/' + currentTheme + '/' + fileName));
            }
        }
    }
    // lets add base CSS path too in this list for now
    // This comes last in base hierarchy
    HbThemeIndexInfo info = HbThemeUtils::getThemeIndexInfo(BaseTheme);
    if (info.themeIndexOffset > 0) {
    hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Core,
            (info.path + '/' + resourcePath + '/' + info.name + '/' + fileName));
    }
    
#else
    foreach (const HbHierarchy &hierarchy, d.hierarchies) {
        switch(hierarchy.layerPriority) {
        case HbLayeredStyleLoader::Priority_Operator: {
                if (!d.operatorPath.isEmpty()) {
                    hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Operator,
                                                     d.constructOperatorPath(operatorBasePath(), fileName));
                }
                break;
            }
        case HbLayeredStyleLoader::Priority_Application: {
                hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Application,
                    (hierarchy.name + '/' + HbMemoryUtils::getCleanAppName() + '/' +
                     resourcePath + '/' + currentTheme + '/' + fileName));
                break;
            }
        case HbLayeredStyleLoader::Priority_Theme: {
                if (currentTheme != baseTheme().name) {
                    // Add platform theme folder only if it is different from base theme
                    // Base theme is anyway added at the core priority
                    hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Theme,
                            (hierarchy.name + '/' + resourcePath + '/' + currentTheme + '/' + fileName));
                }
                break;
            }
        default: {
                // this is for a new hierarchy level and for the time being
                // HbLayeredStyleLoader::Priority_Theme prirority is used,
                // since there is no enum defined in hblayeredstyleloader_p.h
                // priority should be replaced with respective enum.
                hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Theme,
                        (hierarchy.name + '/' + resourcePath + '/' + currentTheme + '/' + fileName));
            }
        }
    }
    
    if (resType == Hb::StyleSheetResource || resType == Hb::EffectResource) {
        // lets add base CSS path too in this list for now
        // This comes last in base hierarchy
        hierarchyListWithPathInfo.insert(HbLayeredStyleLoader::Priority_Core, 
                (QLatin1String("themes/") + resourcePath + '/' + baseTheme().name + '/' + fileName));
    }
#endif // Q_OS_SYMBIAN
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
            // Check if Base theme is defined in theme.theme
            baseThemeInfo = getBaseThemeFromFile(HbStandardDirs::themesDir());
            if (baseThemeInfo.name.isEmpty()) {
                // Base theme does not exists in rom
                // Get the base theme info from core resources
                baseThemeInfo = getBaseThemeFromFile(CoreResourcesRootDir);
            }
        } else {
            // So settings are initialized, it will have other value as well
            baseThemeInfo.rootDir = getThemeSetting(DefaultThemeRootDirSetting).trimmed();
            // On desktop platforms try the HB_THEMES_DIR environment variable instead of
            // blindly sticking to the previous stored setting, the theme directory may have been
            // moved meanwhile and that usually results in a changed HB_THEMES_DIR but nobody will
            // update the our settings stored via QSettings.
#ifndef Q_OS_SYMBIAN            
            QString themesDirFromEnv = HbStandardDirs::themesDir();
            if (!themesDirFromEnv.isEmpty()) {
                baseThemeInfo.rootDir = themesDirFromEnv;
            }
#endif
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
        case OperatorNameSetting:
            return d.operatorName;
        default:
            return QString();
    }
}

void HbThemeUtils::setThemeSetting(Setting setting, const QString &value)
{
#ifdef Q_OS_SYMBIAN
    CRepository *repository = 0;
    TRAP_IGNORE(repository = CRepository::NewL(KServerUid3));
    if (repository) {
        TPtrC valueptr(reinterpret_cast<const TUint16 *>(value.constData()));
        if (KErrNotFound == repository->Set(setting, valueptr)) {
            repository->Create(setting, valueptr);
        }

        delete repository;
    }
#else
    QSettings settings(QLatin1String(ORGANIZATION), QLatin1String(THEME_COMPONENT));
    settings.setValue(settingNames[setting], QVariant(value));
    // Destructor of QSettings flushes the changed setting in the INI file.
#endif
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
        case OperatorNameSetting:
            d.operatorName = value;
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

        // Save theme names in settings
        saveBaseThemeSettings(themeInfo, defaultTheme, rootDir);
    }
    return themeInfo;
}

void HbThemeUtils::saveBaseThemeSettings(HbThemeInfo &baseThemeInfo,
                                                const QString &defaultTheme,
                                                const QString &rootDir)
{
    // If there is any base theme
    if ((!baseThemeInfo.name.isEmpty()) && isThemeValid(HbThemeInfo(baseThemeInfo.name, rootDir))) {
        // Save these theme names in settings
        setThemeSetting(BaseThemeSetting, baseThemeInfo.name);
        setThemeSetting(DefaultThemeRootDirSetting, rootDir);

        // Store default theme also in settings, only if it is valid
        if (baseThemeInfo.name == defaultTheme || isThemeValid(HbThemeInfo(defaultTheme, rootDir))) {
            setThemeSetting(DefaultThemeSetting, defaultTheme);
        }
        baseThemeInfo.rootDir = rootDir;
        d.settingsRetrieved = false;
    }
}

/* checks whether the theme is valid
 */
bool HbThemeUtils::isThemeValid(const HbThemeInfo &themeInfo)
{
    // If the theme contains index.theme in icons resources
    // it will be assumed valid
    QFile themeIndexFile(themeInfo.rootDir + '/' + platformHierarchy + '/' +
                         iconsResourceFolder + '/' + themeInfo.name + "/index.theme");
    return themeIndexFile.open(QIODevice::ReadOnly);
}

HbThemeIndexInfo HbThemeUtils::getThemeIndexInfo(const HbThemeType &type)
{
    HbThemeIndexInfo info;
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    if (manager) { 
        HbSharedChunkHeader *chunkHeader = (HbSharedChunkHeader*)(manager->base());
        
        switch(type) {
        case BaseTheme:
            if (chunkHeader->baseThemeIndexOffset > 0) {
                info.name = QString(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                               chunkHeader->baseThemeNameOffset));
                info.path = QString(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                                chunkHeader->baseThemePathOffset));
                info.themeIndexOffset = chunkHeader->baseThemeIndexOffset;
            }
            break;
        case OperatorC:
            if (chunkHeader->operatorThemeDriveCIndexOffset > 0) {
                info.name = QString(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                               chunkHeader->operatorThemeDriveCNameOffset));
                info.path = QString(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                                chunkHeader->operatorThemeDriveCPathOffset));
                info.themeIndexOffset = chunkHeader->operatorThemeDriveCIndexOffset;
            }
            break;
        case OperatorROM:
            if (chunkHeader->operatorThemeRomIndexOffset > 0) {
                info.name = QString(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                               chunkHeader->operatorThemeRomNameOffset));
                info.path = QString(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                                chunkHeader->operatorThemeRomPathOffset));
                info.themeIndexOffset = chunkHeader->operatorThemeRomIndexOffset;
            }
            break;
        case ActiveTheme:
            if (chunkHeader->activeThemeIndexOffset > 0) {
                info.name = QString(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                               chunkHeader->activeThemeNameOffset));
                info.path = QString(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                                chunkHeader->activeThemePathOffset));
                info.themeIndexOffset = chunkHeader->activeThemeIndexOffset;
            }
            break;
        default:
            break;
        }
    }
    return info;
}

bool HbThemeUtils::isLogicalName(const QString &fileName)
{
    return !(fileName.contains(QChar('/'), Qt::CaseSensitive) || fileName.contains(QChar('\\'), Qt::CaseSensitive));
}
