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
#ifndef HBDDAPPFACTORYSYMBIAN_P_H
#define HBDDAPPFACTORYSYMBIAN_P_H

#include <hbglobal.h>
#include <hbapplication.h>

class CApaApplication;

// Device dialog server application needs a custom application factory in order to override
// CCoeAppUi::FrameworkCallsRendezvous()
CApaApplication *deviceDialogAppFactory();

// Derive from HbApplication to block exit
class HbDeviceDialogServerApp : public HbApplication
{
    Q_OBJECT

public:
    HbDeviceDialogServerApp(QApplication::QS60MainApplicationFactory factory,
        int &argc, char *argv[], Hb::ApplicationFlags flags = Hb::DefaultApplicationFlags);
    bool symbianEventFilter(const QSymbianEvent *event);

public slots:
  static void quit();
};

#endif // HBDDAPPFACTORYSYMBIAN_P_H
