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


#include <QDebug>
#include "hbthemesystemeffectmap_p.h"

#ifdef Q_OS_SYMBIAN
#include <babitflags.h>
#include <w32std.h>
#include <graphics/wstfxconst.h>

const TInt tfxPurpose = Qt::Window;
#endif //Q_OS_SYMBIAN
const QString nullEffect = "NULL";

#ifdef HBTHEMESYSTEMEFFECT_DEBUG
const char *dbgRegister = "REGISTER ";
const char *dbgUnregister = "UNREGISTER ";
const char *dbgDefault = "Default effect ";
const char *dbgAppSpecific = "App specific effect ";
const char *dbgAppStart = "AppStart ";
const char *dbgAppExit = "AppExit ";
const char *dbgAppSwitch = "AppSwitch ";
const char *dbgInvalid = "Invalid ";
const char *dbgIncomingPrio = "Incoming has priority ";
const char *dgbFromBaseTheme = "From base theme ";
const char *dbgIn = " In: ";
const char *dbgOut = "Out: ";
#endif //HBTHEMESYSTEMEFFECT_DEBUG

HbThemeSystemEffectMap::HbThemeSystemEffectMap()
{
}

HbThemeSystemEffectMap::~HbThemeSystemEffectMap()
{
}

void HbThemeSystemEffectMap::setEffectsFolder(const QString &effectsFolder)
{
    mEffectsFolder = effectsFolder;
}

const QString &HbThemeSystemEffectMap::effectsFolder() const
{
    return mEffectsFolder;
}

void HbThemeSystemEffectMap::addEntry(uint appUid,
                                      SystemEffectId id,
                                      const QString &incomingFile,
                                      const QString &outgoingFile,
                                      bool incomingHasPriority,
                                      bool fromBaseTheme)
{
    SystemEffectKey key(id, appUid);
    SystemEffectValue value(incomingFile, outgoingFile, incomingHasPriority, fromBaseTheme);
    SystemEffectMap::iterator i = mSystemEffects.find(key);
    if (i == mSystemEffects.end()) {
        i = mSystemEffects.insert(key, value);
    }
}

int HbThemeSystemEffectMap::entryCount() const
{
    return mSystemEffects.count();
}

bool HbThemeSystemEffectMap::defaultEntryFound(SystemEffectId id) const
{
    SystemEffectKey key(id, 0);
    return mSystemEffects.find(key) != mSystemEffects.end();
}

void HbThemeSystemEffectMap::addDefaultEntry(SystemEffectId id, HbThemeSystemEffectMap *effects)
{
    SystemEffectKey key(id, 0);
    SystemEffectMap::iterator i = effects->mSystemEffects.find(key);
    if (i != effects->mSystemEffects.end()) {
        SystemEffectValue value = i.value();
        value.mFromBaseTheme = true;
        mSystemEffects.insert(key, value);
    }
}

#ifdef HBTHEMESYSTEMEFFECT_DEBUG
void HbThemeSystemEffectMap::traceEffects() const
{
    QString dbgString;
    QMapIterator<SystemEffectKey, SystemEffectValue> mapIt(mSystemEffects);
    while (mapIt.hasNext()) {
        mapIt.next();
        bool unregister = mapIt.value().mIncomingFile.isEmpty()
                          && mapIt.value().mOutgoingFile.isEmpty();
        if (unregister) {
            dbgString = dbgUnregister;
        } else {
            dbgString = dbgRegister;
        }
        if (!mapIt.key().mAppUid) {
            dbgString += dbgDefault;
        } else {
            dbgString += dbgAppSpecific + QString::number(mapIt.key().mAppUid) + " ";
        }
        if (mapIt.key().mEffectId == AppStart) {
            dbgString += dbgAppStart;
        } else if (mapIt.key().mEffectId == AppExit) {
            dbgString += dbgAppExit;
        } else if (mapIt.key().mEffectId == AppSwitch) {
            dbgString += dbgAppSwitch;
        } else {
            dbgString += dbgInvalid;
        }
        if (mapIt.value().mIncomingHasPriority) {
            dbgString += dbgIncomingPrio;
        }
        if (mapIt.value().mFromBaseTheme) {
            dbgString += dgbFromBaseTheme;
        }
        if (!unregister) {
            dbgString += dbgOut + mapIt.value().mOutgoingFile;
            dbgString += dbgIn + mapIt.value().mIncomingFile;
        }
        qDebug() << "HbThemeSystemEffectMap::traceEffects" << dbgString;
    }
}
#endif //HBTHEMESYSTEMEFFECT_DEBUG

#ifdef Q_OS_SYMBIAN
void HbThemeSystemEffectMap::unregisterAllEffects(RWsSession &wsSession) const
{
    // Unregister all previous theme effects
    wsSession.UnregisterAllEffects();
}

void HbThemeSystemEffectMap::registerEffects(RWsSession &wsSession,
        const QString &baseEffectsFolder) const
{
    unregisterAllEffects(wsSession);
    QMapIterator<SystemEffectKey, SystemEffectValue> mapIt(mSystemEffects);
    TPtrC themeResourceDir = mEffectsFolder.utf16();
    TPtrC baseResourceDir = baseEffectsFolder.utf16();
#ifdef HBTHEMESYSTEMEFFECT_DEBUG
    qDebug() << "HbThemeSystemEffectMap::registerEffects effect folder:" << mEffectsFolder
             << "base theme effect folder:" << baseEffectsFolder;
#endif //HBTHEMESYSTEMEFFECT_DEBUG
    while (mapIt.hasNext()) {
        mapIt.next();
        // Register entry
        TInt tfxAction = tfxTransitionAction(mapIt.key().mEffectId);
        QString incomingFile = mapIt.value().mIncomingFile;
        QString outgoingFile = mapIt.value().mOutgoingFile;
        // Do not register if no effect files are defined
        if (isValidEffectEntry(incomingFile, outgoingFile)) {
            TPtrC outgoingEffect = outgoingFile.utf16();
            TPtrC incomingEffect = incomingFile.utf16();
            TPtrC resourceDir = mapIt.value().mFromBaseTheme ? baseResourceDir : themeResourceDir;
            TBitFlags effectFlags;
            if (mapIt.value().mIncomingHasPriority) {
                effectFlags.Set(ETfxIncomingTakesPriority);
            }
            wsSession.RegisterEffect(tfxAction,
                                     tfxPurpose,
                                     resourceDir,
                                     outgoingEffect,
                                     incomingEffect,
                                     mapIt.key().mAppUid,
                                     effectFlags);
        }
    }
}

TInt HbThemeSystemEffectMap::tfxTransitionAction(SystemEffectId id) const
{
    TInt tfxTransitionAction = 0;
    switch (id) {
    case(AppStart) :
        tfxTransitionAction = ETfxActionStart;
        break;
    case(AppExit) :
        tfxTransitionAction = ETfxActionShutDown;
        break;
    case(AppSwitch) :
        tfxTransitionAction = ETfxActionSwitching;
        break;
    default:
        break;
    }
    return tfxTransitionAction;
}
#endif //Q_OS_SYMBIAN

bool HbThemeSystemEffectMap::isValidEffectEntry(QString &incomingFile, QString &outgoingFile) const
{
    bool outgoingEmpty = outgoingFile.isEmpty();
    bool incomingEmpty = incomingFile.isEmpty();
    bool validEntry = !(outgoingEmpty && incomingEmpty);
    if (validEntry) {
        // If no effect should be displayed on transition, register with "NULL" string
        if (outgoingEmpty) {
            outgoingFile = nullEffect;
        }
        if (incomingEmpty) {
            incomingFile = nullEffect;
        }
    }
    return validEntry;
}

