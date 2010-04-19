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

#include "hbdevicedialogserver_p.h"

#include "hbdevicedialogtrace_p.h"
#include <hbapplication.h>
#include <hbmainwindow.h>
#include <hbview.h>
#include <hbtransparentwindow.h>
#include <hbstackedlayout.h>
#include <QTranslator>
#if defined (Q_OS_SYMBIAN)
#include <aknappui.h>
#include <eikenv.h>
#include <apgwgnam.h>
#endif // Q_OS_SYMBIAN

// QApplication calls RProcess::Rendezvous() before server has been created.
// Semaphores are signaled when server initialization is done.
#if defined (Q_OS_SYMBIAN)
static void SignalClientSemaphores()
{
    // Initialisation complete, now signal client(s)
    _LIT(KFindPattern, "hbdevdlgcli_?*");
    TFindSemaphore find(KFindPattern);
    RSemaphore sema;
    TFullName findResult;
    while(find.Next(findResult) == KErrNone) {
        sema.Open(find);
        sema.Signal();
        sema.Close();
    }
}
#endif // Q_OS_SYMBIAN

int main(int arg, char *args[])
{
    INSTALL_MESSAGE_HANDLER

#if defined (Q_OS_SYMBIAN)
    // Guard against starting multiple copies of the server
    RSemaphore serverExistsSema;
    _LIT(KSemaName, "hbdevdlgsrv");
    TInt error = serverExistsSema.CreateGlobal(KSemaName, 0);
    if (error != KErrNone) {
        RProcess::Rendezvous(error);
        return error;
    }
#endif // Q_OS_SYMBIAN

    HbApplication app(arg, args, Hb::NoSplash);

    //QTranslator translator;
    //QString lang_id = QLocale::system().name();
    //translator.load(path_to + "common_" + lang_id);
    //app.installTranslator(&translator);

    HbView* view = new HbView;
    view->hideItems(Hb::AllItems);
    view->setContentFullScreen();

    // Workaround to get device dialogs visible until transparency works
    HbMainWindow mainWindow(0); //, Hb::WindowFlagTransparent);

    HbTransparentWindow *transparentWindow = new HbTransparentWindow;
    HbStackedLayout *stackedLayout = new HbStackedLayout;
    stackedLayout->addItem(transparentWindow);
    view->setLayout(stackedLayout);

    mainWindow.addView(view);

    HbDeviceDialogServer server;
    server.setMainWindow(&mainWindow);

#if defined (Q_OS_SYMBIAN)
    CEikonEnv* env = CEikonEnv::Static();

    if ( env )
        {
        CApaWindowGroupName* wgName = CApaWindowGroupName::NewLC(env->WsSession());
        wgName->SetHidden(ETrue); // hides us from FSW and protects us from OOM FW etc.
        wgName->SetSystem(ETrue); // Allow only application with PowerManagement cap to shut us down
        wgName->SetCaptionL(_L("HbDeviceDialogAppServer"));
        wgName->SetAppUid(KNullUid);
        wgName->SetWindowGroupName(env->RootWin());
        CleanupStack::PopAndDestroy();

        }
    SignalClientSemaphores();
#endif // Q_OS_SYMBIAN

    int returnValue = app.exec();

#if defined (Q_OS_SYMBIAN)
    serverExistsSema.Close();
#endif // Q_OS_SYMBIAN

    UNINSTALL_MESSAGE_HANDLER

    return returnValue;
}
