/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbServers module of the UI Extensions for Mobile.
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

#ifndef HBTHEMEWATCHER_SYMBIAN_P_H
#define HBTHEMEWATCHER_SYMBIAN_P_H

#include <QString>

#include "hbthemecommon_p.h"

#include <e32property.h>
#include <e32base.h>
#include <f32file.h>

class HbThemeServerPrivate;

//**********************************
//CHbThemeWatcher
//**********************************
/**
This class is for watching changes in active theme e.g. ejection of the MMC.
*/
class CHbThemeWatcher : public CActive
{
public:
    static CHbThemeWatcher* NewL(HbThemeServerPrivate& aObserver);
    ~CHbThemeWatcher();
    void startWatchingL(const QString &file);

protected: // From CActive
    void RunL();
    void DoCancel();

private:
    CHbThemeWatcher(HbThemeServerPrivate& aObserver);
    void ConstructL();    

private: // data
    RFs iFs;
    QString iFile;
    HbThemeServerPrivate& iObserver;
};

//**********************************
//CHbThemeChangeNotificationListener
//**********************************
/**
This class represents a listener for Pub/Sub events sent from the clients.
Functions are provided to parse clients messages.
*/
class CHbThemeChangeNotificationListener : public CActive
{
public:
    static CHbThemeChangeNotificationListener* NewL(HbThemeServerPrivate& aObserver);
    virtual ~CHbThemeChangeNotificationListener();
    void startListeningL();
    void stopListening();

protected: // From CActive
    void RunL();
    void DoCancel();

private:
    CHbThemeChangeNotificationListener(HbThemeServerPrivate& aObserver);
    void ConstructL();
    bool parseData(const TDesC& requestData, HbThemeServerRequest& etype, TDes& data);

private: // data
    RProperty themeRequestProp;
    HbThemeServerPrivate& iObserver;
};

#endif // HBTHEMEWATCHER_SYMBIAN_P_H
