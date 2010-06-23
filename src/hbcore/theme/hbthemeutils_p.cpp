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

#include <QLocale>
#include <QSettings>
#include <QFile>
#include <QtDebug>
#include <QDir>
#include <QMap>
#include <QVariant>

#ifdef Q_OS_SYMBIAN
#include "hbthemecommon_symbian_p.h"
#include <e32std.h>
#include <centralrepository.h>
#endif

#include "hbthemeutils_p.h"
#include "hbtheme.h"
#include "hbiniparser_p.h"
#include "hbthemecommon_p.h"
#include "hbthemeclient_p.h"


// Standard folder names
const char *HbThemeUtils::themeResourceFolder = "theme";
const char *HbThemeUtils::platformHierarchy = "themes";
const char *HbThemeUtils::operatorHierarchy = "prioritytheme";
const char *HbThemeUtils::iconsResourceFolder = "icons";
const char *HbThemeUtils::effectsResourceFolder = "effects";
const char *HbThemeUtils::styleResourceFolder = "style";

static const char *themeSettingFile = "theme.theme";
static const char *baseThemeVariable = "BaseTheme";
static const char *defaultThemeVariable = "DefaultActiveTheme";

// Core resource root dir
static const char *coreResourcesRootDir = ":";

// These are the used setting names corresponding to HbThemeUtils::Setting enumeration.
static const QString settingNames[6] = {"", "basetheme", "defaulttheme",
                                        "defaultthemedir", "currenttheme", "operatorbasepath"};

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
    HbThemeUtilsPrivate() : settingsRetrieved(false), mHeapThemeOffset(0)
    {
    }

    ~HbThemeUtilsPrivate();

    void readSettings();
    int heapThemeOffset(const HbThemeIndexInfo &info);

public: // data
    bool settingsRetrieved;
    // Setting values are stored here to avoid overhead of reading from QSettings every time.
    QString currentTheme;
    QString defaultTheme;
    QString defaultThemeRootDir;
    QString baseTheme;
    QString operatorName;

private:
    int mHeapThemeOffset;
};

HbThemeUtilsPrivate::~HbThemeUtilsPrivate()
{
    if (mHeapThemeOffset > 0) {
        GET_MEMORY_MANAGER(HbMemoryManager::HeapMemory);
        manager->free(mHeapThemeOffset);
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
                defaultThemeRootDir = HbThemeUtils::themesDir();           
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

int HbThemeUtilsPrivate::heapThemeOffset(const HbThemeIndexInfo &info)
{
    if (mHeapThemeOffset == 0) {
        QString themeindexfile = info.path + "/" + info.name +".themeindex";
        QFile indexFile(themeindexfile);
        if (indexFile.open(QIODevice::ReadOnly)) {
            GET_MEMORY_MANAGER(HbMemoryManager::HeapMemory);
            qint64 byteSize = indexFile.size();
            mHeapThemeOffset = manager->alloc(byteSize);
            if (mHeapThemeOffset >= 0) {
                char *address = HbMemoryUtils::getAddress<char>(HbMemoryManager::HeapMemory, mHeapThemeOffset);
                indexFile.read(address, byteSize);
                indexFile.close();
            }
        }
    }
    return mHeapThemeOffset;
}

static HbThemeUtilsPrivate d;

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
            baseThemeInfo = getBaseThemeFromFile(HbThemeUtils::themesDir());
            if (baseThemeInfo.name.isEmpty()) {
                // Base theme does not exists in rom
                // Get the base theme info from core resources
                baseThemeInfo = getBaseThemeFromFile(coreResourcesRootDir);
            }
        } else {
            // So settings are initialized, it will have other value as well
            baseThemeInfo.rootDir = getThemeSetting(DefaultThemeRootDirSetting).trimmed();
            // On desktop platforms try the HB_THEMES_DIR environment variable instead of
            // blindly sticking to the previous stored setting, the theme directory may have been
            // moved meanwhile and that usually results in a changed HB_THEMES_DIR but nobody will
            // update the our settings stored via QSettings.
#ifndef Q_OS_SYMBIAN            
            QString themesDirFromEnv = HbThemeUtils::themesDir();
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
    return QFile::exists(themeInfo.rootDir + '/' + platformHierarchy + '/' +
                         iconsResourceFolder + '/' + themeInfo.name + "/index.theme");
}

HbThemeIndexInfo HbThemeUtils::getThemeIndexInfo(const HbThemeType &type)
{
    HbThemeIndexInfo info;

#ifndef Q_OS_SYMBIAN
    // If there is no themeserver connection load theme to client's heap
    if (!HbThemeClient::global()->clientConnected()) {
        HbThemeInfo baseinfo = baseTheme();
        if (baseinfo.name.isEmpty() || baseinfo.name == "hbdefault") {
            info.name = "hbdefault";
            info.path = ":/themes";
        } else {
            info.name = baseinfo.name;
            info.path = baseinfo.rootDir + "/themes";
        }

        info.address = HbMemoryUtils::getAddress<char>(HbMemoryManager::HeapMemory,
                                                       d.heapThemeOffset(info));
        return info;
    }
#endif // Q_OS_SYMBIAN

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
                info.address = HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory,
                                                               chunkHeader->baseThemeIndexOffset);
            }
            break;
        case OperatorC:
            if (chunkHeader->operatorThemeDriveCIndexOffset > 0) {
                info.name = QString(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                               chunkHeader->operatorThemeDriveCNameOffset));
                info.path = QString(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                                chunkHeader->operatorThemeDriveCPathOffset));
                info.address = HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory,
                                                               chunkHeader->operatorThemeDriveCIndexOffset);
            }
            break;
        case OperatorROM:
            if (chunkHeader->operatorThemeRomIndexOffset > 0) {
                info.name = QString(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                               chunkHeader->operatorThemeRomNameOffset));
                info.path = QString(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                                chunkHeader->operatorThemeRomPathOffset));
                info.address = HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory,
                                                               chunkHeader->operatorThemeRomIndexOffset);
            }
            break;
        case ActiveTheme:
            if (chunkHeader->activeThemeIndexOffset > 0) {
                info.name = QString(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                               chunkHeader->activeThemeNameOffset));
                info.path = QString(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                                chunkHeader->activeThemePathOffset));
                info.address = HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory,
                                                               chunkHeader->activeThemeIndexOffset);
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

QString HbThemeUtils::themesDir()
{
#ifdef Q_OS_SYMBIAN
    static QString mainThemesDir("Z:/resource/hb");
#else
    static QString mainThemesDir = QDir::fromNativeSeparators(qgetenv("HB_THEMES_DIR"));
    // Do not call absolutePath if the path is empty,
    // because it would return current path in that case.
    if (!mainThemesDir.isEmpty()) {
        mainThemesDir = QDir(mainThemesDir).absolutePath();
    }
#endif
    return mainThemesDir;
}
