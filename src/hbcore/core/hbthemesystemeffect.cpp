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

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStringList>
#ifdef Q_OS_SYMBIAN
#include <babitflags.h>
#include <coemain.h>
#include <w32std.h>
#endif //Q_OS_SYMBIAN

// Define this to enable debug traces
//#define HBTHEMESYSTEMEFFECT_DEBUG

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
class EffectInfoEntry {
public:
    HbThemeSystemEffect::SystemEffectId mEffectId;
    QLatin1String mEffectIdStr;
};

// Effect info array
const EffectInfoEntry effectInfoArray[] = {
    { HbThemeSystemEffect::AppStart, appStartEffectId },
    { HbThemeSystemEffect::AppExit, appEndEffectId },
    { HbThemeSystemEffect::AppSwitch, appSwitchEffectId }
};

const int effectInfoCount = sizeof(effectInfoArray) / sizeof(EffectInfoEntry);

#ifdef Q_OS_SYMBIAN
//const TInt tfxPurpose = Qt::Window;
#endif //Q_OS_SYMBIAN


static HbThemeSystemEffect *systemEffect = 0;

HbThemeSystemEffect::~HbThemeSystemEffect()
{
}

void HbThemeSystemEffect::handleThemeChange(const QString& themeName)
{
    HbThemeSystemEffect *effect = instance();
    if (effect) {
        effect->setCurrentTheme(themeName);
    }
}

HbThemeSystemEffect *HbThemeSystemEffect::instance()
{
    if (!systemEffect) {
        systemEffect = new HbThemeSystemEffect(qApp);
    }
    return systemEffect;
}

HbThemeSystemEffect::HbThemeSystemEffect(QObject *parent)
    : QObject(parent)
#ifdef Q_OS_SYMBIAN
    , mWsSession(CCoeEnv::Static()->WsSession())
#endif //Q_OS_SYMBIAN
{
}

void HbThemeSystemEffect::setCurrentTheme(const QString& themeName)
{
#ifdef HBTHEMESYSTEMEFFECT_DEBUG
    qDebug() << "HbThemeSystemEffect::setCurrentTheme:" << themeName;
#endif //HBTHEMESYSTEMEFFECT_DEBUG
    if (!themeName.isEmpty()) {
        QString confPath;
        if (getThemeEffectFolder(confPath, themeName)) {
            mThemeEffectFolder = QDir::toNativeSeparators(confPath);
            confPath += "conf/system_effects_configuration.xml";
#ifdef HBTHEMESYSTEMEFFECT_DEBUG
            qDebug() << "HbThemeSystemEffect::setCurrentTheme trying to  parse file" << confPath;
#endif //HBTHEMESYSTEMEFFECT_DEBUG
            bool parsingOk = parseConfigurationFile(confPath);
#ifdef HBTHEMESYSTEMEFFECT_DEBUG
            QMapIterator<SystemEffectKey, SystemEffectValue> mapIt(mSystemEffects);
            while (mapIt.hasNext()) {
                mapIt.next();
                qDebug() << "HbThemeSystemEffect::setCurrentTheme appUid:"
                        << mapIt.key().mAppUid << "effect id:" << mapIt.key().mEffectId
                        << "outgoing effect file:" << mapIt.value().mOutgoingFile
                        << "incoming effect file:" << mapIt.value().mIncomingFile;
            }
#endif //HBTHEMESYSTEMEFFECT_DEBUG
            // Register effects
            // TODO: what to do if conf file not found (or some effect file not found)?
            if (parsingOk) {
                registerEffects();
            }
        }
    }
}

void HbThemeSystemEffect::registerEffects()
{
#ifdef Q_OS_SYMBIAN
    // Unregister all previous theme effects
    //mWsSession.UnregisterAllEffects();
#endif //Q_OS_SYMBIAN
    QMapIterator<SystemEffectKey, SystemEffectValue> mapIt(mSystemEffects);
#ifdef Q_OS_SYMBIAN
    TPtrC resourceDir = mThemeEffectFolder.utf16();
#endif //Q_OS_SYMBIAN
    while (mapIt.hasNext()) {
        mapIt.next();
        // Register entry
#ifdef HBTHEMESYSTEMEFFECT_DEBUG
        if (!mapIt.key().mAppUid) {
            qDebug() << "HbThemeSystemEffect: Registering default system effect:"
                    << mapIt.key().mEffectId << mapIt.value().mOutgoingFile << mapIt.value().mIncomingFile;
        } else {
            qDebug() << "HbThemeSystemEffect: Registering application (" << mapIt.key().mAppUid
                     << ") specific system effect:" << mapIt.key().mEffectId
                     << mapIt.value().mOutgoingFile << mapIt.value().mIncomingFile;
        }
#endif //HBTHEMESYSTEMEFFECT_DEBUG
#ifdef Q_OS_SYMBIAN
        TInt tfxAction = tfxTransitionAction(mapIt.key().mEffectId);
        // If no effect files defined, unregister effect
        if (mapIt.value().mOutgoingFile.isEmpty()
                && mapIt.value().mIncomingFile.isEmpty()) {
//            mWsSession.UnregisterEffect(tfxAction, tfxPurpose, mapIt.key().mAppUid);
        } else {
            TPtrC outgoingEffect = mapIt.value().mOutgoingFile.utf16();
            TPtrC incomingEffect = mapIt.value().mIncomingFile.utf16();
            TBitFlags effectFlags;
            if (mapIt.value().mIncomingHasPriority) {
//                effectFlags.Set(TTfxFlags::ETfxIncomingTakesPriority);
            }
//            mWsSession.RegisterEffect(tfxAction,
//                                      tfxPurpose,
//                                      resourceDir,
//                                      outgoingEffect,
//                                      incomingEffect,
//                                      mapIt.key().mAppUid,
//                                      effectFlags);
        }
#endif //Q_OS_SYMBIAN
    }
}

bool HbThemeSystemEffect::parseConfigurationFile(const QString& filePath)
{
    bool success = true;
    mSystemEffects.clear();
    QFile confFile(filePath);
    success = confFile.open(QIODevice::ReadOnly);
    if (success) {
        QXmlStreamReader xml(&confFile);
        success = checkStartElement(xml, mainElement);
        if (success) {
            parseEffects(xml);
            if (xml.error()) {
                qWarning() << "HbThemeSystemEffect::parseConfigurationFile: Error when parsing xml " << xml.errorString();
                success = false;
            }
        }
        confFile.close();
    } else {
        qWarning() << "HbThemeSystemEffect::parseConfigurationFile:" << filePath << "not found.";
    }
    return success;
}

void HbThemeSystemEffect::parseEffects(QXmlStreamReader &xml)
{
    // Go through effects section
    while (!xml.atEnd()) {
        if (checkStartElement(xml, effectElement)) {

            SystemEffectId effectId = Invalid;
            uint appId = 0;
            QString incomingFile;
            QString outgoingFile;
            bool incomingHasPriority = false;
            bool validEntry = true;
            bool effectFileEntryFound = false;

            // Single effect entry
            while (validEntry && xml.readNextStartElement()) {
                // Effect id
                if (xml.name() == effectIdElement) {
                    effectId = (SystemEffectId)idFromEffectIdString(xml.readElementText());
                    validEntry = !(effectId == Invalid);
                // App id
                } else if (xml.name() == appIdElement) {
                    appId = validApplicationUid(xml.readElementText());
                // Outgoing effect file
                } else if (xml.name() == outgoingFileElement) {
                    effectFileEntryFound = true;
                    outgoingFile = xml.readElementText();
                    validEntry = validEffectFile(outgoingFile);
                // Incoming effect file
                } else if (xml.name() == incomingFileElement) {
                    effectFileEntryFound = true;
                    incomingFile = xml.readElementText();
                    validEntry = validEffectFile(incomingFile);
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
                addEntryToEffectMap(appId, effectId, incomingFile, outgoingFile, incomingHasPriority);
            }
        }
    }
}

bool HbThemeSystemEffect::checkStartElement(QXmlStreamReader &xml, const QLatin1String &startElement) const
{
    xml.readNext();
    while (!xml.isStartElement() && !xml.atEnd()) {
        xml.readNext();
    }
    bool success = xml.isStartElement();
    if (success && xml.name() != startElement) {
        success = false;
    } else if (xml.error()) {
        qWarning()
                << "HbThemeSystemEffect::checkStartElement: Error when parsing system effect configuration : "
                << xml.errorString();
    } else if (!success && !xml.name().isEmpty()) {
        qWarning()
                << "HbThemeSystemEffect::checkStartElement: Error when parsing system effect configuration with element: "
                << xml.name();
    }
    return success;
}

int HbThemeSystemEffect::idFromEffectIdString(const QString &effectIdString) const
{
    for (int i=0; i<effectInfoCount; i++) {
        if (effectInfoArray[i].mEffectIdStr == effectIdString) {
            return effectInfoArray[i].mEffectId;
        }
    }
    return Invalid;
}

bool HbThemeSystemEffect::validEffectFile(const QString &effectFile) const
{
    bool validFile = true;
    // Validate file existance only if effects folder is found
    // (no validating with unit tests nonexistent effect files)
    if (!mThemeEffectFolder.isEmpty()) {
        if (!QFile(mThemeEffectFolder + effectFile).exists()) {
            validFile = false;
        }
    }
    return validFile;
}

uint HbThemeSystemEffect::validApplicationUid(const QString &appUid) const
{
    bool ok = false;
    int base = 10;
    if (appUid.startsWith("0x")) {
        base = 16;
    }
    uint uid = appUid.toUInt(&ok, base);
    return uid;
}

bool HbThemeSystemEffect::booleanFromString(const QString &boolAttribute) const
{
    bool value = false;
    if (boolAttribute == QLatin1String("true") || boolAttribute == QLatin1String("1")) {
        value = true;
    }
    return value;
}

bool HbThemeSystemEffect::getThemeEffectFolder(QString &path, const QString &themeName) const
{
    bool pathFound = false;
    QString effectDir = "themes/effects/" + themeName + "/";
    QStringList queryList;
    queryList.append(effectDir);
    QStringList folderList = HbStandardDirs::findExistingFolderList(queryList, 
                                                                    themeName, 
                                                                    Hb::EffectResource);
    QString pathCandidate;
    for (int i=0; i<folderList.count(); i++) {
        pathCandidate = folderList.at(i);
        // Skip resource folders
        if (!pathCandidate.startsWith(":/")) {
            path = pathCandidate;
            pathFound = true;
            break;
        }
    }
    return pathFound;
}

void HbThemeSystemEffect::addEntryToEffectMap(uint appUid,
                                              SystemEffectId id,
                                              const QString &incomingFile,
                                              const QString &outgoingFile,
                                              bool incomingHasPriority)
{
    if (id != Invalid) {
        SystemEffectKey key(id, appUid);
        SystemEffectValue value(incomingFile, outgoingFile, incomingHasPriority);
        SystemEffectMap::iterator i = mSystemEffects.find(key);
        if (i == mSystemEffects.end()) {
            i = mSystemEffects.insert(key, value);
        }
    }
}

#ifdef Q_OS_SYMBIAN
TInt HbThemeSystemEffect::tfxTransitionAction(const SystemEffectId id) const
{
    TInt tfxTransitionAction = 0;
    switch (id) {
    case (AppStart) :
        //tfxTransitionAction = TTfxTransitionActions::ETfxActionStart;
        break;
    case (AppExit) :
        //tfxTransitionAction = TTfxTransitionActions::ETfxActionShutDown;
        break;
    case (AppSwitch) :
        //tfxTransitionAction = TTfxTransitionActions::ETfxActionSwitching;
        break;
    default:
        break;
    }
    return tfxTransitionAction;
}
#endif //Q_OS_SYMBIAN

