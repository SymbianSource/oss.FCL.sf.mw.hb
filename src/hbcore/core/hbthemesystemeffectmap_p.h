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

#ifndef HBTHEMESYSTEMEFFECTMAP_P_H
#define HBTHEMESYSTEMEFFECTMAP_P_H

#include <QMap>
#include <QString>
#ifdef Q_OS_SYMBIAN
class RWsSession;
#endif //Q_OS_SYMBIAN

// Define this to enable debug traces
//#define HBTHEMESYSTEMEFFECT_DEBUG


class HbThemeSystemEffectMap
{
public:
    enum SystemEffectId {
        Invalid = 0,
        AppStart,
        AppExit,
        AppSwitch
    };

    HbThemeSystemEffectMap();
    virtual ~HbThemeSystemEffectMap();

    void setEffectsFolder(const QString &effectsFolder);
    const QString &effectsFolder() const;
    void addEntry(uint appUid,
                  SystemEffectId id,
                  const QString &incomingFile,
                  const QString &outgoingFile,
                  bool incomingHasPriority,
                  bool fromBaseTheme);

    int entryCount() const;
    bool defaultEntryFound(SystemEffectId id) const;
    void addDefaultEntry(SystemEffectId id, HbThemeSystemEffectMap *effects);
#ifdef HBTHEMESYSTEMEFFECT_DEBUG
    void traceEffects() const;
#endif //HBTHEMESYSTEMEFFECT_DEBUG

#ifdef Q_OS_SYMBIAN
    void unregisterAllEffects(RWsSession &wsSession) const;
    void registerEffects(RWsSession &wsSession, const QString &baseEffectsFolder) const;
    TInt tfxTransitionAction(SystemEffectId id) const;
#endif //Q_OS_SYMBIAN

private:
    class SystemEffectKey
    {
    public:
        inline SystemEffectKey(SystemEffectId effectId, uint appUid)
            : mEffectId(effectId), mAppUid(appUid) {}

        inline bool operator<(const SystemEffectKey &other) const {
            return other.mAppUid == mAppUid ? other.mEffectId > mEffectId : other.mAppUid > mAppUid;
        }

    public:
        SystemEffectId mEffectId;
        uint mAppUid;
    };

    class SystemEffectValue
    {
    public:
        inline SystemEffectValue(const QString &incomingFile,
                                 const QString &outgoingFile,
                                 bool incomingHasPriority,
                                 bool fromBaseTheme)
            : mIncomingFile(incomingFile),
              mOutgoingFile(outgoingFile),
              mIncomingHasPriority(incomingHasPriority),
              mFromBaseTheme(fromBaseTheme) {}

    public:
        QString mIncomingFile;
        QString mOutgoingFile;
        bool mIncomingHasPriority;
        bool mFromBaseTheme;
    };

    typedef QMap<SystemEffectKey, SystemEffectValue> SystemEffectMap;
    SystemEffectMap mSystemEffects;
    QString mEffectsFolder;

    friend class TestHbThemeSystemEffect;
};

#endif // HBTHEMESYSTEMEFFECTMAP_P_H
