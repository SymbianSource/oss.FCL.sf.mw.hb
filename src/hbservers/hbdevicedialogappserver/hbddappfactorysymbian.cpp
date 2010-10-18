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

#include "hbddappfactorysymbian_p.h"

#include <QApplication>
#include <qs60mainapplication.h>
#include <qs60maindocument.h>
#include <qs60mainappui.h>
#include <QSymbianEvent>

/// \cond

// In order to override CCoeAppUi::FrameworkCallsRendezvous() Application/Document/AppUi needs to be
// derived from.

class DeviceDialogMainAppUi : public QS60MainAppUi
{
public:
    TBool FrameworkCallsRendezvous() const {return false;}
};

class DeviceDialogMainDocument : public QS60MainDocument
{
public:
    DeviceDialogMainDocument(CEikApplication &mainApp) : QS60MainDocument(mainApp) {}
    CEikAppUi *CreateAppUiL()
        {return (static_cast <CEikAppUi*>(new (ELeave) DeviceDialogMainAppUi));}
};

class DeviceDialogMainApplication : public QS60MainApplication
{
protected:
    CApaDocument *CreateDocumentL()
        {return new (ELeave) DeviceDialogMainDocument(*this);}
};

CApaApplication *deviceDialogAppFactory()
{
    return new DeviceDialogMainApplication;
}

HbDeviceDialogServerApp::HbDeviceDialogServerApp(QApplication::QS60MainApplicationFactory factory,
    int &argc, char *argv[], Hb::ApplicationFlags flags) :
    HbApplication(factory, argc, argv, flags)
{
}

// Block application quit() slot
void HbDeviceDialogServerApp::quit()
{
}

/// \endcond
