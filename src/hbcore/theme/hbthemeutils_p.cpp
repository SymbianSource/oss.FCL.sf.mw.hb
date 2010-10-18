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
#include "hbtheme.h"
#include "hbthemecommon_p.h"
#include "hbthemeclient_p.h"
#include "hbtheme_p.h"

#ifdef Q_OS_SYMBIAN
#include "hbthemecommon_symbian_p.h"
#include <e32std.h>
#include <centralrepository.h>
#endif

#include <QLocale>
#include <QSettings>
#include <QFile>
#include <QtDebug>
#include <QDir>
#include <QMap>
#include <QVariant>

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
                                        "", "currenttheme", "operatorbasepath"};

static HbHeapIndexInfo *heapIndex = 0;

QString themesDir()
{
#ifdef Q_OS_SYMBIAN
    static QString mainThemesDir("z:/resource/hb");
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


/*!
    @proto
    @hbcore
    \class HbThemeUtils

    \brief HbThemeUtils provides some helper function to be used by theming system.

    Currently HbThemeUtils class has functions to add and remove theme hierarchies
    and query current list of theme hierarchies. It also has a function to provide hierarchylist.
    In future the class can include more functionalities as and when reuired.
*/


class HbThemeSettings
{
public:
    HbThemeSettings() : settingsRetrieved(false)
    {
    }

    ~HbThemeSettings();

    void readSettings();
    void initSettings();
    QString getThemeFromFile(const QString &variable, const QString &rootDir);

public: // data
    bool settingsRetrieved;
    // Setting values are stored here to avoid overhead of reading from QSettings every time.
    QString currentTheme;
    QString defaultTheme;
    QString baseTheme;
    QString operatorName;
};

HbThemeSettings::~HbThemeSettings()
{
    if (heapIndex) {
        // Free allocated memory
        if (heapIndex->baseTheme.address) {
            delete heapIndex->baseTheme.address;
        }
        if (heapIndex->priorityTheme.address) {
            delete heapIndex->priorityTheme.address;
        }
        if (heapIndex->activeTheme.address) {
            delete heapIndex->activeTheme.address;
        }
        delete heapIndex;
    }
}

void HbThemeSettings::readSettings()
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
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, QLatin1String(ORGANIZATION), QLatin1String(THEME_COMPONENT));

        currentTheme = settings.value(settingNames[HbThemeUtils::CurrentThemeSetting]).toString();
        defaultTheme = settings.value(settingNames[HbThemeUtils::DefaultThemeSetting]).toString();
        baseTheme = settings.value(settingNames[HbThemeUtils::BaseThemeSetting]).toString();
        operatorName = settings.value(settingNames[HbThemeUtils::OperatorNameSetting]).toString();
#endif
        initSettings();

        settingsRetrieved = true;
    }
}

void HbThemeSettings::initSettings()
{
    // Validate base theme
    bool modified = false;
    if (baseTheme.isEmpty()) {
        modified = true;
        baseTheme = getThemeFromFile(baseThemeVariable, themesDir());
        if (baseTheme.isEmpty()) {
            baseTheme = getThemeFromFile(baseThemeVariable, coreResourcesRootDir);
        }
    }
    if (!HbThemeUtils::isThemeValid(baseTheme)) {
        modified = true;
        // check if the theme name is logical
        baseTheme = themesDir() + '/' + HbThemeUtils::platformHierarchy + '/' +
                    HbThemeUtils::iconsResourceFolder + '/' + baseTheme;
        if (!HbThemeUtils::isThemeValid(baseTheme)) {
            baseTheme = getThemeFromFile(baseThemeVariable, coreResourcesRootDir);
        }
    }
    // Save if needed
    if (modified) {
        HbThemeUtils::setThemeSetting(HbThemeUtils::BaseThemeSetting, baseTheme);
        modified = false;
    }

    // Validate default theme    
    if (defaultTheme.isEmpty()) {
        modified = true;
        defaultTheme = getThemeFromFile(defaultThemeVariable, themesDir());
        if (defaultTheme.isEmpty()) {
            defaultTheme = getThemeFromFile(defaultThemeVariable, coreResourcesRootDir);
        }
    }
    if (!HbThemeUtils::isThemeValid(defaultTheme)) {
        modified = true;
        // check if the theme name is logical
        defaultTheme = themesDir() + '/' + HbThemeUtils::platformHierarchy + '/' +
                    HbThemeUtils::iconsResourceFolder + '/' + defaultTheme;
        if (!HbThemeUtils::isThemeValid(defaultTheme)) {
            defaultTheme = getThemeFromFile(defaultThemeVariable, coreResourcesRootDir);
        }
    }
    if (modified) {
        HbThemeUtils::setThemeSetting(HbThemeUtils::DefaultThemeSetting, defaultTheme);
        modified = false;
    }

    // Validate current theme
    if (!HbThemeUtils::isThemeValid(currentTheme)) {
        currentTheme = defaultTheme;
        HbThemeUtils::setThemeSetting(HbThemeUtils::CurrentThemeSetting, currentTheme);
    }
}

/* reads the theme from theme.theme file
 */
QString HbThemeSettings::getThemeFromFile(const QString &variable, const QString &rootDir)
{
    QString themeSetting(rootDir + '/' + HbThemeUtils::platformHierarchy + '/' + themeSettingFile);
    QString theme;
    if (QFile::exists(themeSetting)) {
        QSettings iniParser(themeSetting, QSettings::IniFormat);
        iniParser.beginGroup("Default");
        theme = rootDir + '/' + HbThemeUtils::platformHierarchy + '/' +
                HbThemeUtils::iconsResourceFolder + '/' +
                iniParser.value(variable).toString().trimmed();
        iniParser.endGroup();
    }
    return theme;
}

Q_GLOBAL_STATIC(HbThemeSettings, themeSettings);

QString HbThemeUtils::getThemeSetting(Setting setting)
{
    // Make sure settings are read from QSettings.
    themeSettings()->readSettings();

    switch (setting) {
        case CurrentThemeSetting:
            return themeSettings()->currentTheme;
        case DefaultThemeSetting:
            return themeSettings()->defaultTheme;
        case BaseThemeSetting:
            return themeSettings()->baseTheme;
        case OperatorNameSetting:
            return themeSettings()->operatorName;
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
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QLatin1String(ORGANIZATION), QLatin1String(THEME_COMPONENT));
    settings.setValue(settingNames[setting], QVariant(value));
    // Destructor of QSettings flushes the changed setting in the INI file.
#endif
    updateThemeSetting(setting, value);
}   

/**
* Updates the setting's value in stored member variables.
* Normally the settings are loaded from QSettings when method getThemeSetting() is called for the first time.
* When there is a change in settings, this method can be used to sync the setting value stored in HbThemeSettings.
* E.g. theme change event updates the current theme setting, currently no other settings are changing their values.
*/
void HbThemeUtils::updateThemeSetting(Setting setting, const QString &value)
{
    switch (setting) {
        case CurrentThemeSetting:
            themeSettings()->currentTheme = value;
            break;
        case DefaultThemeSetting:
            themeSettings()->defaultTheme = value;
            break;
        case BaseThemeSetting:
            themeSettings()->baseTheme = value;
            break;
        case OperatorNameSetting:
            themeSettings()->operatorName = value;
            break;
        default:
            break;
    }
}   

/* checks whether the theme is valid
 */
bool HbThemeUtils::isThemeValid(const QString &themePath)
{
    const QString themePathFixed = QDir::fromNativeSeparators(themePath);
    // If the theme contains .themeindex and index.theme files
    // it will be assumed valid
    QString indexFile = QString(themePathFixed).replace("/icons/", QString('/')) + ".themeindex";
    return (QFile::exists(themePathFixed + "/index.theme") && QFile::exists(indexFile));
}

HbThemeIndexInfo HbThemeUtils::getThemeIndexInfo(const HbThemeType &type)
{
    HbThemeIndexInfo info;

#ifndef Q_OS_SYMBIAN
    if (!heapIndex) {
        heapIndex = new HbHeapIndexInfo();
        HbThemeUtils::loadHeapThemeIndexes();
    }

    if (heapIndex) {
        switch(type) {
        case BaseTheme:
            info = heapIndex->baseTheme;
            break;
        case OperatorC:
            info = heapIndex->priorityTheme;
            break;
        case ActiveTheme:
            info = heapIndex->activeTheme;
            break;
        default:
            break;
        }
    }
#else
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
#endif // Q_OS_SYMBIAN
    return info;
}

bool HbThemeUtils::isLogicalName(const QString &fileName)
{
    return !(fileName.contains(QChar(':'), Qt::CaseSensitive) ||
             fileName.contains(QChar('/'), Qt::CaseSensitive) ||
             fileName.contains(QChar('\\'), Qt::CaseSensitive));
}

char *HbThemeUtils::createHeapThemeIndex(const HbThemeInfo &theme)
{
    char *address = 0;

    QString path = QDir::toNativeSeparators(theme.rootDir);
    QString filename;

    filename.append(path);
    filename.append('/');
    filename.append(theme.name);
    filename.append(".themeindex");

    QFile indexFile(filename);
    if (indexFile.open(QIODevice::ReadOnly)) {
        GET_MEMORY_MANAGER(HbMemoryManager::HeapMemory);
        qint64 byteSize = indexFile.size();
        qptrdiff indexOffset = manager->alloc(byteSize);
        if (indexOffset != -1) {
            address = HbMemoryUtils::getAddress<char>(HbMemoryManager::HeapMemory, indexOffset);
            indexFile.read(address, byteSize);
            indexFile.close();
        }
    }

    return address;
}

void HbThemeUtils::loadHeapThemeIndex(HbThemeType type)
{
    if (heapIndex) {
        switch(type) {
        case BaseTheme: {
            if (heapIndex->baseTheme.address) {
                delete heapIndex->baseTheme.address;
            }
            QString baseThemeName = getThemeSetting(BaseThemeSetting);
            HbThemeInfo baseInfo;
            QDir path(baseThemeName);
            baseInfo.name = path.dirName();
            QString absolutePath = path.absolutePath();
            baseInfo.rootDir = absolutePath.left(absolutePath.indexOf("/icons/"));

            heapIndex->baseTheme.address = createHeapThemeIndex(baseInfo);
            if (heapIndex->baseTheme.address) {
                heapIndex->baseTheme.name = baseInfo.name;
                heapIndex->baseTheme.path = baseInfo.rootDir;
            }
            break;
        }
        case OperatorC: {
            if (heapIndex->priorityTheme.address) {
                delete heapIndex->priorityTheme.address;
            }
            HbThemeInfo operatorInfo;
            operatorInfo.name = getThemeSetting(OperatorNameSetting);
            if (!operatorInfo.name.isEmpty()) {
                operatorInfo.rootDir.append(themesDir() + '/' + QLatin1String(operatorHierarchy));
                heapIndex->priorityTheme.address =  createHeapThemeIndex(operatorInfo);
                if (heapIndex->priorityTheme.address) {
                    heapIndex->priorityTheme.name = operatorInfo.name;
                    heapIndex->priorityTheme.path = operatorInfo.rootDir;
                }
            }
            break;
        }
        case ActiveTheme: {
            if (heapIndex->activeTheme.address) {
                delete heapIndex->activeTheme.address;
            }
            QString currentThemeName = getThemeSetting(CurrentThemeSetting);
            QDir path(currentThemeName);
            HbThemeInfo activeInfo;
            activeInfo.name = path.dirName();
            QString absolutePath = path.absolutePath();
            activeInfo.rootDir = absolutePath.left(absolutePath.indexOf("/icons/"));

            heapIndex->activeTheme.address = createHeapThemeIndex(activeInfo);
            if (heapIndex->activeTheme.address) {
                heapIndex->activeTheme.name = activeInfo.name;
                heapIndex->activeTheme.path = activeInfo.rootDir;
            }
            break;
        }
        default:
            break;
        }
    }
}

void HbThemeUtils::loadHeapThemeIndexes()
{
    // Process base theme index, it is used as parent index also when the current theme is something else
    loadHeapThemeIndex(BaseTheme);
    // Process operator theme indexes
    loadHeapThemeIndex(OperatorC);
    // Process current theme index
    loadHeapThemeIndex(ActiveTheme);
}
