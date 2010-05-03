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

#ifndef HBTHEMESYSTEMEFFECT_P_H
#define HBTHEMESYSTEMEFFECT_P_H

#include <hbglobal.h>
#include <QMap>
#include <QObject>
#include <QXmlStreamReader>

#ifdef Q_OS_SYMBIAN
class RWsSession;
#endif //Q_OS_SYMBIAN

class HB_CORE_PRIVATE_EXPORT HbThemeSystemEffect : public QObject
{
    Q_OBJECT

public:
    enum SystemEffectId {
        Invalid = 0,
        AppStart,
        AppExit,
        AppSwitch
    };

    ~HbThemeSystemEffect();
    static void handleThemeChange(const QString &themeName);

private:
    static HbThemeSystemEffect *instance();
    HbThemeSystemEffect(QObject *parent);
    void setCurrentTheme(const QString &themeName);
    void registerEffects();
    bool parseConfigurationFile(const QString& filePath);
    void parseEffects(QXmlStreamReader &xml);
    bool checkStartElement(QXmlStreamReader &xml, const QLatin1String &startElement) const;
    int idFromEffectIdString(const QString &effectIdString) const;
    bool validEffectFile(const QString &effectFile) const;
    uint validApplicationUid(const QString &appUid) const;
    bool booleanFromString(const QString &boolAttribute) const;
    bool getThemeEffectFolder(QString &path, const QString &themeName) const;
    void addEntryToEffectMap(uint appUid,
                             SystemEffectId id,
                             const QString &incomingFile,
                             const QString &outgoingFile,
                             bool incomingHasPriority);

#ifdef Q_OS_SYMBIAN
    TInt tfxTransitionAction(const SystemEffectId id) const;
#endif //Q_OS_SYMBIAN

private:
    class SystemEffectKey {
    public:
        inline SystemEffectKey(SystemEffectId effectId, uint appUid)
            : mEffectId(effectId), mAppUid(appUid){}

        inline bool operator<(const SystemEffectKey &other) const {
            return other.mAppUid == mAppUid ? other.mEffectId > mEffectId : other.mAppUid > mAppUid;
        }

    public:
        SystemEffectId mEffectId;
        uint mAppUid;
    };

    class SystemEffectValue {
    public:
        inline SystemEffectValue(const QString &incomingFile,
                                 const QString &outgoingFile,
                                 bool incomingHasPriority)
            : mIncomingFile(incomingFile),
            mOutgoingFile(outgoingFile),
            mIncomingHasPriority(incomingHasPriority) {}

    public:
        QString mIncomingFile;
        QString mOutgoingFile;
        bool mIncomingHasPriority;
    };

    typedef QMap<SystemEffectKey, SystemEffectValue> SystemEffectMap;
    SystemEffectMap mSystemEffects;
    QString mThemeEffectFolder;
#ifdef Q_OS_SYMBIAN
    RWsSession &mWsSession;
#endif //Q_OS_SYMBIAN

    friend class TestHbThemeSystemEffect;
};

#endif // HBTHEMESYSTEMEFFECT_P_H
