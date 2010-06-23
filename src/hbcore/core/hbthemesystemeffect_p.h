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

#include <QObject>
#include <QXmlStreamReader>
#include <hbglobal.h>

class HbThemeSystemEffectMap;
#ifdef Q_OS_SYMBIAN
class RWsSession;
#endif //Q_OS_SYMBIAN

class HB_CORE_PRIVATE_EXPORT HbThemeSystemEffect : public QObject
{
    Q_OBJECT

public:
    ~HbThemeSystemEffect();
    static void handleThemeChange(const QString &themeName);

private:
    HbThemeSystemEffect(QObject *parent);
    void setCurrentTheme(const QString &themeName);
    bool getThemeEffectFolder(QString &path);
    void verifyAllEffectsFound(HbThemeSystemEffectMap &effects);
    void registerEffects(const HbThemeSystemEffectMap *effects);
    HbThemeSystemEffectMap *parseBaseThemeEffects();
    HbThemeSystemEffectMap *baseThemeEffects();

    static HbThemeSystemEffect *instance();
    static HbThemeSystemEffectMap *parseConfigurationFile(const QString &effectsFolder);
    static void parseEffects(QXmlStreamReader &xml, HbThemeSystemEffectMap *effects);
    static bool checkStartElement(QXmlStreamReader &xml, const QLatin1String &startElement);
    static int idFromEffectIdString(const QString &effectIdString);
    static bool validEffectFile(const QString &effectFile,
                                HbThemeSystemEffectMap *effects,
                                bool &fromBaseTheme);
    static uint validApplicationUid(const QString &appUid);
    static bool booleanFromString(const QString &boolAttribute);

private:
    HbThemeSystemEffectMap *mBaseEffects;
    QString mBaseEffectsFolder;
    bool mBaseEffectsRegistered;
#ifdef Q_OS_SYMBIAN
    RWsSession &mWsSession;
#endif //Q_OS_SYMBIAN

    friend class TestHbThemeSystemEffect;
};

#endif // HBTHEMESYSTEMEFFECT_P_H
