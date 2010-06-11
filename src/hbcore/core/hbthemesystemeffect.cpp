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


#include "hbstandarddirs_p.h"
#include "hbthemesystemeffect_p.h"
#include "hbthemesystemeffectmap_p.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStringList>
#ifdef Q_OS_SYMBIAN
#include <coemain.h>
#endif //Q_OS_SYMBIAN

const char *confFilePath = "conf/system_effects_configuration.xml";

// Configuration XML elements
const QLatin1String mainElement("effects_configuration");
const QLatin1String effectElement("system_effect");
const QLatin1String effectIdElement("effect_id");
const QLatin1String appIdElement("app_id");
const QLatin1String incomingFileElement("incoming_file");
const QLatin1String outgoingFileElement("outgoing_file");
const QLatin1String incomingPriorityElement("incoming_priority");
// Configuration XML values
const QLatin1String appStartEffectId("app_start");
const QLatin1String appEndEffectId("app_exit");
const QLatin1String appSwitchEffectId("app_switch");

// Helper class for storing effect info
class EffectInfoEntry
{
public:
    HbThemeSystemEffectMap::SystemEffectId mEffectId;
    QLatin1String mEffectIdStr;
};

// Effect info array
const EffectInfoEntry effectInfoArray[] = {
    { HbThemeSystemEffectMap::AppStart, appStartEffectId },
    { HbThemeSystemEffectMap::AppExit, appEndEffectId },
    { HbThemeSystemEffectMap::AppSwitch, appSwitchEffectId }
};

const int effectInfoCount = sizeof(effectInfoArray) / sizeof(EffectInfoEntry);

static HbThemeSystemEffect *systemEffect = 0;

HbThemeSystemEffect::~HbThemeSystemEffect()
{
    delete mBaseEffects;
}

void HbThemeSystemEffect::handleThemeChange(const QString &themeName)
{
    HbThemeSystemEffect *effect = instance();
    if (effect) {
        effect->setCurrentTheme(themeName);
    }
}

HbThemeSystemEffect::HbThemeSystemEffect(QObject *parent)
    : QObject(parent),
      mBaseEffects(0),
      mBaseEffectsRegistered(false)
#ifdef Q_OS_SYMBIAN
      , mWsSession(CCoeEnv::Static()->WsSession())
#endif //Q_OS_SYMBIAN
{
}

void HbThemeSystemEffect::setCurrentTheme(const QString &themeName)
{
#ifdef HBTHEMESYSTEMEFFECT_DEBUG
    qDebug() << "HbThemeSystemEffect::setCurrentTheme:" << themeName;
#endif //HBTHEMESYSTEMEFFECT_DEBUG

    if (!themeName.isEmpty()) {

        bool registeringOk = false;
        QString confPath;

        // Try registering the theme specific effects
        if (getThemeEffectFolder(confPath, themeName)) {
            HbThemeSystemEffectMap *themeEffects = parseConfigurationFile(confPath);
            if (themeEffects->entryCount()) {
                verifyAllEffectsFound(*themeEffects);
                registerEffects(themeEffects);
                registeringOk = true;
            }
            delete themeEffects;
            themeEffects = 0;
        }
        // Theme specific effects registration failed
        // Try registering the base theme effects
        if (!registeringOk) {
            // If base theme configuration does not exist, all effects are unregistered
            registerEffects(baseThemeEffects());
        }
    }
}

bool HbThemeSystemEffect::getThemeEffectFolder(QString &path, const QString &themeName)
{
    bool pathFound = false;
    QString effectDir = "themes/effects/" + themeName + "/";
    QStringList queryList;
    queryList.append(effectDir);
    int baseThemeIndex = -1;
    QStringList folderList = HbStandardDirs::findExistingFolderList(queryList,
                             themeName,
                             Hb::EffectResource,
                             baseThemeIndex);

    // Search for current theme effect folder
    QString pathCandidate;
    for (int i = 0; i < folderList.count(); i++) {
        pathCandidate = folderList.at(i);
        // Skip resource folders
        if (!pathCandidate.startsWith(":/") && i != baseThemeIndex) {
            path = pathCandidate;
            pathFound = true;
            break;
        }
    }

    // Save base theme effect folder path if not saved already
    if (baseThemeIndex >= 0 && mBaseEffectsFolder.isEmpty()) {
        QString baseEffectsFolder = folderList.at(baseThemeIndex);
        if (!baseEffectsFolder.startsWith(":/")) {
            mBaseEffectsFolder = baseEffectsFolder;
        }
    }
    return pathFound;
}

void HbThemeSystemEffect::verifyAllEffectsFound(HbThemeSystemEffectMap &effects)
{
    if (effects.entryCount()) {
        for (int i = 0; i < effectInfoCount; i++) {
            HbThemeSystemEffectMap::SystemEffectId id = effectInfoArray[i].mEffectId;
            if (!effects.defaultEntryFound(id)) {
                if (mBaseEffects || baseThemeEffects()) {
                    effects.addDefaultEntry(id, mBaseEffects);
                } else {
                    break;
                }
            }
        }
    }
}

void HbThemeSystemEffect::registerEffects(const HbThemeSystemEffectMap *effects)
{
    if (effects == mBaseEffects && mBaseEffectsRegistered) {
#ifdef HBTHEMESYSTEMEFFECT_DEBUG
        qDebug() << "HbThemeSystemEffect::registerEffects: not registering base theme effects"
                 << "since already registered!";
#endif //HBTHEMESYSTEMEFFECT_DEBUG
        return;
    }
    mBaseEffectsRegistered = false;
    if (effects && effects->entryCount()) {
        mBaseEffectsRegistered = effects == mBaseEffects;
#ifdef HBTHEMESYSTEMEFFECT_DEBUG
        if (mBaseEffectsRegistered) {
            qDebug() << "HbThemeSystemEffect::registerEffects: registering base theme effects";
        } else {
            qDebug() << "HbThemeSystemEffect::registerEffects: registering theme specific effects";
        }
        effects->traceEffects();
#endif //HBTHEMESYSTEMEFFECT_DEBUG
#ifdef Q_OS_SYMBIAN
        effects->registerEffects(mWsSession, mBaseEffectsFolder);
#endif //Q_OS_SYMBIAN
    } else {
        // Disable warning until themes have the conf file
        //qWarning() << "HbThemeSystemEffect::registerEffects: no system effects configuration"
        //           << "found - unregistering all system effects.";
#ifdef Q_OS_SYMBIAN
        effects->unregisterAllEffects(mWsSession);
#endif //Q_OS_SYMBIAN
    }
}

HbThemeSystemEffectMap *HbThemeSystemEffect::baseThemeEffects()
{
    // Base theme effects haven't been parsed yet
    if (!mBaseEffects && !mBaseEffectsFolder.isEmpty()) {
        mBaseEffects = parseConfigurationFile(mBaseEffectsFolder);
    }
    return mBaseEffects;
}

HbThemeSystemEffect *HbThemeSystemEffect::instance()
{
    if (!systemEffect) {
        systemEffect = new HbThemeSystemEffect(qApp);
    }
    return systemEffect;
}

HbThemeSystemEffectMap *HbThemeSystemEffect::parseConfigurationFile(const QString &effectsFolder)
{
    HbThemeSystemEffectMap *effects = new HbThemeSystemEffectMap;
    effects->setEffectsFolder(QDir::toNativeSeparators(effectsFolder));
    QString filePath = effectsFolder + confFilePath;
#ifdef HBTHEMESYSTEMEFFECT_DEBUG
    qDebug() << "HbThemeSystemEffect::parseConfigurationFile trying to  parse file" << filePath;
#endif //HBTHEMESYSTEMEFFECT_DEBUG
    bool success = true;
    QFile confFile(filePath);
    success = confFile.open(QIODevice::ReadOnly);
    if (success) {
        QXmlStreamReader xml(&confFile);
        success = checkStartElement(xml, mainElement);
        if (success) {
            parseEffects(xml, effects);
            if (xml.error()) {
                qWarning() << "HbThemeSystemEffect::parseConfigurationFile: Error when parsing xml "
                           << xml.errorString();
                success = false;
            }
        }
        confFile.close();
    } else {
#ifdef HBTHEMESYSTEMEFFECT_DEBUG
        qDebug() << "HbThemeSystemEffect::parseConfigurationFile:"
                 << confFile.fileName() << "not found.";
#endif //HBTHEMESYSTEMEFFECT_DEBUG
    }
    return effects;
}

void HbThemeSystemEffect::parseEffects(QXmlStreamReader &xml, HbThemeSystemEffectMap *effects)
{
    // Go through effects section
    while (!xml.atEnd()) {
        if (checkStartElement(xml, effectElement)) {

            HbThemeSystemEffectMap::SystemEffectId effectId = HbThemeSystemEffectMap::Invalid;
            uint appId = 0;
            QString incomingFile;
            QString outgoingFile;
            bool incomingHasPriority = false;
            bool validEntry = true;
            bool effectFileEntryFound = false;
            bool fromBaseTheme = false;

            // Single effect entry
            while (validEntry && xml.readNextStartElement()) {
                // Effect id
                if (xml.name() == effectIdElement) {
                    effectId = (HbThemeSystemEffectMap::SystemEffectId)
                               idFromEffectIdString(xml.readElementText());
                    validEntry = !(effectId == HbThemeSystemEffectMap::Invalid);
                    // App id
                } else if (xml.name() == appIdElement) {
                    appId = validApplicationUid(xml.readElementText());
                    // Outgoing effect file
                } else if (xml.name() == outgoingFileElement) {
                    effectFileEntryFound = true;
                    outgoingFile = xml.readElementText();
                    validEntry = validEffectFile(outgoingFile, effects, fromBaseTheme);
                    // Incoming effect file
                } else if (xml.name() == incomingFileElement) {
                    effectFileEntryFound = true;
                    incomingFile = xml.readElementText();
                    validEntry = validEffectFile(incomingFile, effects, fromBaseTheme);
                    // If incoming file has the priority
                } else if (xml.name() == incomingPriorityElement) {
                    incomingHasPriority = booleanFromString(xml.readElementText());
                }
                // Read end element
                xml.readNext();
            }
            if (!effectFileEntryFound) {
                validEntry = false;
            }
            // If valid entry was found, store it to system effects map
            if (validEntry) {
                effects->addEntry(appId,
                                  effectId,
                                  incomingFile,
                                  outgoingFile,
                                  incomingHasPriority,
                                  fromBaseTheme);
            }
        }
    }
}

bool HbThemeSystemEffect::checkStartElement(QXmlStreamReader &xml,
        const QLatin1String &startElement)
{
    xml.readNext();
    while (!xml.isStartElement() && !xml.atEnd()) {
        xml.readNext();
    }
    bool success = xml.isStartElement();
    if (success && xml.name() != startElement) {
        success = false;
    } else if (xml.error()) {
        qWarning() << "HbThemeSystemEffect::checkStartElement:"
                   << " Error when parsing system effect configuration : " << xml.errorString();
    } else if (!success && !xml.name().isEmpty()) {
        qWarning() << "HbThemeSystemEffect::checkStartElement: "
                   << "Error when parsing system effect configuration with element: " << xml.name();
    }
    return success;
}

int HbThemeSystemEffect::idFromEffectIdString(const QString &effectIdString)
{
    for (int i = 0; i < effectInfoCount; i++) {
        if (effectInfoArray[i].mEffectIdStr == effectIdString) {
            return effectInfoArray[i].mEffectId;
        }
    }
    return HbThemeSystemEffectMap::Invalid;
}

bool HbThemeSystemEffect::validEffectFile(const QString &effectFile,
        HbThemeSystemEffectMap *effects, bool &fromBaseTheme)
{
    bool validFile = false;
    // Check if file is found in theme effect folder
    if (!fromBaseTheme) {
        validFile = QFile(effects->effectsFolder() + effectFile).exists();
        // Not found -> Check if file is found in base theme effect folder
        if (!validFile && !instance()->mBaseEffectsFolder.isEmpty()
                && instance()->mBaseEffectsFolder != effects->effectsFolder()) {
            validFile = QFile(instance()->mBaseEffectsFolder + effectFile).exists();
            if (validFile) {
                fromBaseTheme = true;
            }
        }
        // Other entry files found in base theme -> this should be found there too
    } else {
        validFile = QFile(instance()->mBaseEffectsFolder + effectFile).exists();
    }
    return validFile;
}

uint HbThemeSystemEffect::validApplicationUid(const QString &appUid)
{
    bool ok = false;
    int base = 10;
    if (appUid.startsWith("0x")) {
        base = 16;
    }
    uint uid = appUid.toUInt(&ok, base);
    return uid;
}

bool HbThemeSystemEffect::booleanFromString(const QString &boolAttribute)
{
    bool value = false;
    if (boolAttribute == QLatin1String("true") || boolAttribute == QLatin1String("1")) {
        value = true;
    }
    return value;
}

