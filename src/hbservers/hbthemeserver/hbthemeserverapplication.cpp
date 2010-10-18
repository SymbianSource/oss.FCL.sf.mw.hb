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
#include "hbthemeserverapplication_p.h"
#include "hbthemeserver_p.h"
#include "hbthemecommon_p.h"
#include "hbtheme.h"

#include <QWindowsStyle>
#include <QLibrary>
#include <QDebug>
#include <QDir>

#include "hbthemecommon_symbian_p.h"
#include <eikenv.h>
#include <apgwgnam.h>

static const QLatin1String APP_NAME("HbThemeServer");
static const QLatin1String RESOURCE_LIB_NAME("HbCore");
static const QLatin1String TEST_RESOURCE_LIB_NAME("HbTestResources");

HbThemeServerApplication::HbThemeServerApplication(int &argc, char *argv[]) :
    QtSingleApplication(argc, argv), server(0)
{
    setApplicationName(APP_NAME);

#ifdef QT_DEBUG
    //temporary solution until Hb specific style is ready
    setStyle(new QWindowsStyle);
#endif // QT_DEBUG
}

HbThemeServerApplication::~HbThemeServerApplication()
{
    delete server;
}

bool HbThemeServerApplication::initialize()
{
    CEikonEnv * env = CEikonEnv::Static();
    if ( env ) {
        _LIT(KHbThemeServer, "HbThemeServer");
        CApaWindowGroupName *wgName = CApaWindowGroupName::NewLC(env->WsSession());
        env->RootWin().SetOrdinalPosition(0, ECoeWinPriorityNeverAtFront); // avoid coming to foreground
        env->WsSession().ComputeMode(RWsSession::EPriorityControlDisabled);
        setPriority();
        wgName->SetHidden(ETrue); // hides us from FSW and protects us from OOM FW etc.
        wgName->SetSystem(ETrue); // Allow only application with PowerManagement cap to shut us down    
        wgName->SetCaptionL(KHbThemeServer);
        wgName->SetAppUid(KNullUid);
        wgName->SetWindowGroupName(env->RootWin());
        CleanupStack::PopAndDestroy();
        RThread::RenameMe(KHbThemeServer);
        User::SetProcessCritical(User::ESystemCritical);
        User::SetCritical(User::ESystemCritical);
    }

    // load resource libraries in order to make binary resources accessible
    bool result = loadLibrary(RESOURCE_LIB_NAME);
#ifdef HB_DEVELOPER
    loadLibrary(TEST_RESOURCE_LIB_NAME);
#endif
    return result;
}

int HbThemeServerApplication::exec()
{
    if (!server) {
        server = new HbThemeServer;
    }

    if (server->startServer()) {
        return QtSingleApplication::exec();
    }

    return EXIT_FAILURE;
}

bool HbThemeServerApplication::loadLibrary(const QString &name)
{
    QLibrary library(name);
    // rely on dynamic loader (LD_LIBRARY_PATH)
    bool result = library.load();
    if (!result) {
        // try from prefix/lib dir
        library.setFileName(QDir(HB_LIB_DIR).filePath(name));
        result = library.load();
        if (!result) {
            // try from build/lib dir
            QString path = QLatin1String(HB_BUILD_DIR) + QDir::separator() + QLatin1String("lib");
            library.setFileName(QDir(path).filePath(name));
            result = library.load();
        }
    }
    if (!result) {
        THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "Error: " << library.errorString();
    }
    return result;
}

bool HbThemeServerLocker::lock()
{
    Lock::State lockState;
    
    Q_FOREVER {
        lockState = mLock.acquire();
        if (lockState == Lock::Reserved) {
            // Process may be starting, wait for server object to be created
            if (serverExists()) {
                THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "server already exists.";
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
        THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "Lock not acquired.";
        RProcess::Rendezvous(lockState == Lock::Reserved ? KErrAlreadyExists : KErrGeneral);
    }

    return (lockState == Lock::Acquired);
}
void HbThemeServerApplication::setPriority()
{
    RProcess().SetPriority(EPriorityHigh);
}

_LIT(KLockFileName, "lockFile");

Lock::Lock()
{
    // Using a file for interprocess lock
    const int NumMessageSlots = 1;
    if (mFs.Connect(NumMessageSlots) == KErrNone) {
        mFs.CreatePrivatePath(EDriveC);
        if (mFs.SetSessionToPrivate(EDriveC) == KErrNone) {
            RFile file;
            file.Create(mFs, KLockFileName, EFileShareReadersOrWriters);
            file.Close();
        }
    }
}

// Try to acquire lock
Lock::State Lock::acquire()
{
    State state = Error;
    TInt status = mFile.Open(mFs, KLockFileName, EFileShareExclusive);
    if (status == KErrNone) {
        state = Acquired;
    } else if (status == KErrInUse) {
        state = Reserved;
    }
    return state;
}

// Check if Symbian server exists
bool HbThemeServerLocker::serverExists()
{
    TFindServer findHbServer(KThemeServerName);
    TFullName name;
    return findHbServer.Next(name) == KErrNone;
}
