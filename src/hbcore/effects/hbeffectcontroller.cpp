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

#include "hbeffectcontroller_p.h"
#include "hbeffectfxmldata_p.h"
#include "hbeffectxmlparser_p.h"
#include "hbeffectdef_p.h"
#include "hbeffecttheme_p.h"
#include "hbinstance.h"
#include "hbiconloader_p.h"
#include "hbtheme.h"
#include "hbtheme_p.h"
#include "hbthemeclient_p.h"
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QDebug>

/*
  \class HbEffectController hbeffectcontroller.h
  \brief HbEffectController is used to read and store effects data. 
*/

/* 
   Constructs a HbEffectController.
*/
HbEffectController::HbEffectController()
    : mParser(0),
      mShared(false),
      mSharingSet(false)
{
}

/* 
   Destructor.
*/
HbEffectController::~HbEffectController()
{
    delete mParser;
}

HbEffectFxmlData HbEffectController::fetchFxmlData(
    const QString &componentType,
    QGraphicsItem *component,
    const QString &effectEvent,
    bool shared ) const
{
    Q_UNUSED(shared); // not yet needed since we have a list of added items in local process
    int componentTypeMatchIndex = -1;

    // Find the definition for the component
    for (int i = 0; i < mEffectEntries.count(); ++i) {
        const HbEffectInfo &info = mEffectEntries.at(i);
        // If found a definition matching the graphics item, return that.
        if (info.item() // not null
            && info.item() == component // matches the component
            && info.inUse() // the effect is in use (not removed)
            && info.effectEvent() == effectEvent) // matches the event
        {
            // sharing support -  if it's shared get the FxML data from server and return the object
            // with parsed FxML data
            if( info.shared() ) {
                HbEffectFxmlData *dataFromSrv = HbThemeClient::global()->getSharedEffect(info.mDefFileFullPath);
                if(dataFromSrv) {
                    return *dataFromSrv;
                }
            } else {
                return info.effectData();
            }
        }
        // If found a definition matching the component type, mark index
        // The whole list is still scanned for a match for the graphics item. This is
        // needed to be able to override components effects for one graphics item.
        else if (!info.componentType().isEmpty() && info.componentType() == componentType &&
                 info.inUse() && info.effectEvent() == effectEvent) {
            componentTypeMatchIndex = i;
        }
    }
    if (componentTypeMatchIndex >= 0) {
        const HbEffectInfo &info = mEffectEntries.at(componentTypeMatchIndex);
        if ( info.shared() ) {
            HbEffectFxmlData *dataFromSrv =
                HbThemeClient::global()->getSharedEffect(info.mDefFileFullPath);
            if(dataFromSrv) {
                return *dataFromSrv;
            }
        } else {
            return mEffectEntries.at(componentTypeMatchIndex).effectData();
        }
    }
    return HbEffectFxmlData();
}

bool HbEffectController::addFXML(const QString &componentType, const QString &filePath, const QString &effectEvent,
                                 bool shared)
{
    return addEffectDefinitionFile(componentType, 0, filePath, effectEvent, shared);
}

bool HbEffectController::addFXML(QGraphicsItem *item, const QString &filePath, const QString &effectEvent,
                                 bool shared)
{
    return addEffectDefinitionFile(QString(), item, filePath, effectEvent, shared);
}

void HbEffectController::removeFXML(const QString &componentType, const QString &filePath, const QString &effectEvent)
{
    QString fullPath = expandFileName(filePath);
    for (int i = mEffectEntries.count() - 1; i >= 0; i--) {
        const HbEffectInfo &info = mEffectEntries[i];
        if (info.componentType() == componentType &&
            info.xmlFileFullPath() == fullPath &&
            info.effectEvent() == effectEvent)
        {
            // Remove the definition for the given component type
            mEffectEntries.removeAt(i);
        }
    }
}

void HbEffectController::removeFXML(QGraphicsItem *item, const QString &filePath, const QString &effectEvent)
{
    QString fullPath = expandFileName(filePath);
    for (int i = mEffectEntries.count() - 1; i >= 0; i--) {
        const HbEffectInfo &info = mEffectEntries[i];
        if (info.item() == item &&
            info.xmlFileFullPath() == fullPath &&
            info.effectEvent() == effectEvent)
        {
            // Remove the definition for the given item
            mEffectEntries.removeAt(i);
        }
    }
}

void HbEffectController::removeFXML(QGraphicsItem *item)
{
    for (int i = mEffectEntries.count() - 1; i >= 0; i--) {
        const HbEffectInfo &info = mEffectEntries[i];
        if (info.item() == item) {
            // Remove the definition for the given item
            mEffectEntries.removeAt(i);
        }
    }
}

QList<HbEffectInfo> HbEffectController::effectEntries() const
{
    return mEffectEntries;
}
void HbEffectController::setSharing(bool enabled)
{
    mShared = enabled;
    mSharingSet = true;
}

void HbEffectController::unsetSharing()
{
    mSharingSet = false;
}

QString HbEffectController::expandFileName(const QString &fn, bool *fromTheme, bool *shared)
{
    // Sharing not supported if the file is in Qt resource.
    if (fn.startsWith(":/") && shared && *shared) {
        *shared = false;
        return fn;
    } else {
        // Try the theme, will return the same name if not found in the theme.
        bool temp;
        return HbEffectTheme::instance()->getEffectXml(fn, fromTheme ? *fromTheme : temp);
    }
}

bool HbEffectController::addEffectDefinitionFile(
    const QString &componentType, QGraphicsItem *item,
    const QString &fileName, const QString &effectEvent, bool shared)
{
    if (mSharingSet) {
        shared = mShared;
    }
    // If client is not connected due to any reason, any kind of sharing is not possible
    if ( !HbThemeClient::global()->clientConnected() ) {
        shared = mShared = mSharingSet = false;
    }
    bool fromTheme = false;
    QString filePath = expandFileName(fileName, &fromTheme, &shared);

    // Check if there's already a definition parsed for given componentType or item
    int indexForSetInUseWhenFail = -1;
    for (int i = 0, ie = mEffectEntries.count(); i != ie; ++i) {
        // non-const reference needed here to modify the actual data item
        HbEffectInfo &info = mEffectEntries[i];

        if ((item && info.item() == item)
            || (!componentType.isEmpty() && info.componentType() == componentType))
        {
            if (info.effectEvent() == effectEvent) {    
                if (info.xmlFileFullPath() == filePath) {
                    // parsed already, just set in use
                    info.mInUse = true;
                    return true;
                } else {
                    // New definition replaces this one, so mark this 'not used'.
                    info.mInUse = false;
                    // We may need to set the in-use flag back to true if file
                    // access fails, so store the index.
                    indexForSetInUseWhenFail = i;
                }
            }
        }
    }

    if ( shared ) {
        if (!HbThemeClient::global()->addSharedEffect(filePath)) {
            // Themeserver failed, try to parse locally.
            shared = false;
        }
    }

    if (!mParser && !shared) {
        mParser = new HbEffectXmlParser;
    }

    HbEffectInfo effectEntry;
    effectEntry.mComponentType = componentType;
    effectEntry.mItem = item;
    effectEntry.mInUse = true;
    effectEntry.mFromTheme = fromTheme;
    effectEntry.mDefFileFullPath = filePath;
    effectEntry.mEffectEvent = effectEvent;
    effectEntry.mShared = shared;

    if (!shared) {
        HbEffectFxmlData fxmlData;
        QFile file(filePath);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            qWarning("HbEffect: Opening of %s failed", qPrintable(filePath));
            if (indexForSetInUseWhenFail >= 0) {
                mEffectEntries[indexForSetInUseWhenFail].mInUse = true;
            }
            return false;
        }
        mParser->read(&file, &fxmlData);
        file.close();

        if (mParser->error() != QXmlStreamReader::NoError) {
            qWarning() << "HbEffect: Parsing of file" << qPrintable(filePath) << "failed:"
                       << mParser->errorString();
            if (indexForSetInUseWhenFail >= 0) {
                mEffectEntries[indexForSetInUseWhenFail].mInUse = true;
            }
            return false;
        }

        effectEntry.setEffectData(fxmlData);
    }

    mEffectEntries.append(effectEntry);
    return true;
}
