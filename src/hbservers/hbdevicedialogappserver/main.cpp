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
#include <hbdevicedialogserverdefs_p.h>
#include "hbddappfactorysymbian.h"
#endif // Q_OS_SYMBIAN

#if defined (Q_OS_SYMBIAN)
class Lock
{
public:
    enum State {
        Reserved,
        Acquired,
        Error
    };
    Lock();
    ~Lock(){close();}
    void close(){mFile.Close(); mFs.Close();}
    State acquire();
    static bool serverExists();

private:
    RFs mFs;
    RFile mFile;
};

Lock::Lock()
{
    // Using a file for interprocess lock
    const int NumMessageSlots = 1;
    if (mFs.Connect(NumMessageSlots) == KErrNone) {
        mFs.CreatePrivatePath(EDriveC);
        if (mFs.SetSessionToPrivate(EDriveC) == KErrNone) {
            _LIT(KFileName, "lockFile");
            const TUint mode = EFileShareReadersOrWriters;
            if (mFile.Create(mFs, KFileName, mode) == KErrAlreadyExists) {
                mFile.Open(mFs, KFileName, mode);
            }
        }
    }
}

// Try to acquire lock
Lock::State Lock::acquire()
{
    State state = Error;
    // If process holding the lock crashes, file server releases the lock
    if (mFile.SubSessionHandle()) {
        TInt error = mFile.Lock(0, 1);
        if (error == KErrNone) {
            state = Acquired;
        } else if (error == KErrLocked) {
            state = Reserved;
        }
    }
    return state;
}

// Check if Symbian server exists
bool Lock::serverExists()
{
    TFindServer findHbServer(KHbServerName);
    TFullName name;
    return findHbServer.Next(name) == KErrNone;
}

#endif // Q_OS_SYMBIAN

#define USE_LOCKER 1
int main(int arg, char *args[])
{
    INSTALL_MESSAGE_HANDLER

#if defined (Q_OS_SYMBIAN)
    // Guard against starting multiple copies of the server
    Lock lock;
    Lock::State lockState;
    for(;;) {
        lockState = lock.acquire();
        if (lockState == Lock::Acquired) {
            break;
        } else if (lockState == Lock::Reserved) {
            // Process may be starting, wait for server object to be created
            if (Lock::serverExists()) {
                break;
            } else {
                const TInt KTimeout = 100000; // 100 ms
                User::After(KTimeout);
            }
        } else {
            break;
        }
    }
    if (lockState != Lock::Acquired) {
        // With KErrAlreadyExists client should try to connect, otherwise bail out.
        RProcess::Rendezvous(lockState == Lock::Reserved ? KErrAlreadyExists:KErrGeneral);
        return KErrNone;
    }
    _LIT(KThreadName, "hbdevdlgsrvapp");
    RThread().RenameMe(KThreadName); // nicer panic info
    RThread().SetProcessPriority(EPriorityHigh);
    
    HbApplication app(deviceDialogAppFactory, arg, args, Hb::NoSplash);
#else // Q_OS_SYMBIAN
    HbApplication app(arg, args);
#endif // Q_OS_SYMBIAN

    HbView* view = new HbView;
    view->hideItems(Hb::AllItems);
    view->setContentFullScreen();

    HbMainWindow mainWindow(0, Hb::WindowFlagTransparent);
        
#if defined (Q_OS_SYMBIAN)
    CCoeControl *c = mainWindow.effectiveWinId();
    RWindow *rw = static_cast<RWindow*>(c->DrawableWindow());
    rw->SetRequiredDisplayMode( EColor16MA );
    TInt err = rw->SetTransparencyAlphaChannel();
    if ( err == KErrNone ) {
        rw->SetBackgroundColor(~0);
    }
#endif // Q_OS_SYMBIAN

    HbTransparentWindow *transparentWindow = new HbTransparentWindow;
    HbStackedLayout *stackedLayout = new HbStackedLayout;
    stackedLayout->addItem(transparentWindow);
    view->setLayout(stackedLayout);

    mainWindow.addView(view);

#if defined (Q_OS_SYMBIAN)
    CEikonEnv* env = CEikonEnv::Static();

    if (env) {
        CApaWindowGroupName* wgName = CApaWindowGroupName::NewLC(env->WsSession());
        wgName->SetHidden(ETrue); // hides us from FSW and protects us from OOM FW etc.
        wgName->SetSystem(ETrue); // Allow only application with PowerManagement cap to shut us down
        wgName->SetCaptionL(_L("HbDeviceDialogAppServer"));
        wgName->SetAppUid(KNullUid);
        wgName->SetWindowGroupName(env->RootWin());
        CleanupStack::PopAndDestroy();       
    }
#endif // Q_OS_SYMBIAN

    // HbApplication must be created before the server to have environment ready
    HbDeviceDialogServer server;
    server.setMainWindow(&mainWindow);

#if defined (Q_OS_SYMBIAN)
    // Server is created, client may connect
    RProcess::Rendezvous(KErrNone);
#endif // Q_OS_SYMBIAN

    int returnValue = app.exec();

    UNINSTALL_MESSAGE_HANDLER

    return returnValue;
}
