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

#include <QApplication>
#include <QLibrary>
#include <QDebug>

#include "hbthemeserver_p.h"
#include "hbthemecommon_p.h"
#include "hbtheme.h"
#if defined (Q_OS_SYMBIAN)
#include "hbthemecommon_symbian_p.h"
#include <eikenv.h>
#include <apgwgnam.h>
#endif
#include <qtsingleapplication.h>
#include <QWindowsStyle>

static const QLatin1String RESOURCE_LIB_NAME("HbCore");
static const QLatin1String TEST_RESOURCE_LIB_NAME("HbTestResources");
static const QLatin1String WIN32_DEBUG_SUFFIX("d");
static const QLatin1String MAC_DEBUG_SUFFIX("_debug");

#ifdef Q_OS_SYMBIAN
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
    TFindServer findHbServer(KThemeServerName);
    TFullName name;
    return findHbServer.Next(name) == KErrNone;
}

#endif
/* 
    This function loads library which keeps resources of default theme
*/
bool loadResourceLibrary(QString resourceLibName)
{
    bool loadSuccess;
    // To load resources embedded in hb library
    QLibrary hbLib(resourceLibName);
    loadSuccess = hbLib.load();
    
    if ( !loadSuccess ) {
        // Library may not be loaded, if it was built in debug mode and the name in debug mode is
        // different, change the name to debug version in that scenario
#ifdef Q_OS_WIN32
        resourceLibName += WIN32_DEBUG_SUFFIX;
#elif defined(Q_OS_MAC)
        resourceLibName += MAC_DEBUG_SUFFIX;
#endif
        // On symbian library name in debug mode is same as that in release mode,
        // so no need to do anything for that
        hbLib.setFileName(resourceLibName);
        loadSuccess = hbLib.load();
    }
#ifdef THEME_SERVER_TRACES
    if (loadSuccess) {
        qDebug() << "Loaded library " << resourceLibName;
    }
    else {
        qDebug() << "Could not load library " << resourceLibName;
    }
#endif // THEME_SERVER_TRACES

    return loadSuccess;
}

int main(int argc, char *argv[])
{
#ifdef QT_DEBUG
    //temporary solution until Hb specific style is ready
    QApplication::setStyle( new QWindowsStyle );
#endif // QT_DEBUG
#if QT_VERSION >= 0x040601
    QApplication::setAttribute(Qt::AA_S60DontConstructApplicationPanes);
#endif // QT_VERSION
#ifdef THEME_SERVER_TRACES
        qDebug() << "HbThemeServer::main: START!!!";
#endif
#ifdef Q_OS_SYMBIAN
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
#ifdef THEME_SERVER_TRACES
        qDebug() << "HbThemeServer::main: serverExists!!!";
#endif
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
#ifdef THEME_SERVER_TRACES
        qDebug() << "HbThemeServer::main: Lock not acquired!!!";
#endif
        RProcess::Rendezvous(lockState == Lock::Reserved ? KErrAlreadyExists:KErrGeneral);
        return KErrNone;
    }
#endif // Q_OS_SYMBIAN
#ifdef THEME_SERVER_TRACES
        qDebug() << "HbThemeServer::main: start construction QtSingleApplication!!!";
#endif
    QtSingleApplication  app(argc, argv );

    if (app.isRunning()) {
#ifdef THEME_SERVER_TRACES
        qDebug() << "HbThemeServer::main: first instance already running!!!";
#endif
        return 0;
    }

#ifdef THEME_SERVER_TRACES
    qDebug() << "HbThemeServer::main: I'm first instance!!!";
#endif
        
#if defined (Q_OS_SYMBIAN)
    CEikonEnv * env = CEikonEnv::Static();
    if ( env ) {
        CApaWindowGroupName* wgName = CApaWindowGroupName::NewLC(env->WsSession());
        env->RootWin().SetOrdinalPosition(0, ECoeWinPriorityNeverAtFront); // avoid coming to foreground
        wgName->SetHidden(ETrue); // hides us from FSW and protects us from OOM FW etc.
        wgName->SetSystem(ETrue); // Allow only application with PowerManagement cap to shut us down    
        wgName->SetCaptionL(_L("HbThemeServer"));
        wgName->SetAppUid(KNullUid);
        wgName->SetWindowGroupName(env->RootWin());
        CleanupStack::PopAndDestroy();
        RThread::RenameMe(_L("HbThemeServer"));
    }
#endif
    HbTheme::instance(); //for theme initialization, instance needs to be created before starting the server.
    loadResourceLibrary(RESOURCE_LIB_NAME);
#ifdef HB_DEVELOPER
    loadResourceLibrary(TEST_RESOURCE_LIB_NAME);
#endif

    HbThemeServer server;

    bool success = server.startServer();
    
    if ( !success ) {
#ifdef THEME_SERVER_TRACES
        qDebug() << "HbThemeServer::main: server not started!!!";
#endif
        return -1;
    }
    
#ifdef THEME_SERVER_TRACES
    qDebug() << "HbThemeServer::main: server started!!!";
#endif

#ifndef Q_OS_SYMBIAN
#ifdef QT_DEBUG
    server.showMinimized();
#endif
#endif // Q_OS_SYMBIAN

    int result = app.exec();
#ifdef THEME_SERVER_TRACES
        qDebug() << "HbThemeServer::main: out from exec, with result code: " << result;
#endif
    return result; 
}
