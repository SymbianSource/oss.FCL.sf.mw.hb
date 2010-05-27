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

#if defined (Q_OS_SYMBIAN)
#include "hbthemecommon_symbian_p.h"
#include <eikenv.h>
#include <apgwgnam.h>
#endif

static const QLatin1String APP_NAME("HbThemeServer");
static const QLatin1String RESOURCE_LIB_NAME("HbCore");
static const QLatin1String TEST_RESOURCE_LIB_NAME("HbTestResources");
static const QLatin1String WIN32_DEBUG_SUFFIX("d");
static const QLatin1String MAC_DEBUG_SUFFIX("_debug");
static const QLatin1String STOP_MESSAGE("stop");

bool HbThemeServerApplication::Options::help = false;
bool HbThemeServerApplication::Options::start = false;
bool HbThemeServerApplication::Options::stop = false;
bool HbThemeServerApplication::Options::persistent = false;
QString HbThemeServerApplication::Options::error;

HbThemeServerApplication::HbThemeServerApplication(int &argc, char *argv[]) :
    QtSingleApplication(argc, argv), server(0)
{
    setApplicationName(APP_NAME);

#ifdef QT_DEBUG
    //temporary solution until Hb specific style is ready
    setStyle(new QWindowsStyle);
#endif // QT_DEBUG

    // ignore command line arguments on Symbian
#ifdef Q_OS_SYMBIAN
    Options::start = true;
    Options::persistent = true;
#else
    QStringList args = arguments();
    args.removeFirst(); // ignore argv0
    const bool wasEmpty = args.isEmpty();
    const bool restart = args.removeAll(QLatin1String("-restart"));
    Options::start = args.removeAll(QLatin1String("-start")) || restart;
    Options::stop = args.removeAll(QLatin1String("-stop")) || restart;
    Options::persistent = args.removeAll(QLatin1String("-persistent"));
    Options::help = args.removeAll(QLatin1String("-help"))
                    || args.removeAll(QLatin1String("-h")) || !args.isEmpty() || wasEmpty;
    if (!args.isEmpty()) {
        Options::error = tr("Unknown option(s): '%1'").arg(args.join(QLatin1String(" ")));
    }
#endif // Q_OS_SYMBIAN
}

HbThemeServerApplication::~HbThemeServerApplication()
{
    delete server;
}

bool HbThemeServerApplication::initialize()
{
#if defined (Q_OS_SYMBIAN)
    CEikonEnv * env = CEikonEnv::Static();
    if ( env ) {
        _LIT(KHbThemeServer, "HbThemeServer");
        CApaWindowGroupName *wgName = CApaWindowGroupName::NewLC(env->WsSession());
        env->RootWin().SetOrdinalPosition(0, ECoeWinPriorityNeverAtFront); // avoid coming to foreground
        wgName->SetHidden(ETrue); // hides us from FSW and protects us from OOM FW etc.
        wgName->SetSystem(ETrue); // Allow only application with PowerManagement cap to shut us down    
        wgName->SetCaptionL(KHbThemeServer);
        wgName->SetAppUid(KNullUid);
        wgName->SetWindowGroupName(env->RootWin());
        CleanupStack::PopAndDestroy();
        RThread::RenameMe(KHbThemeServer);
    }
#endif

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
#if !defined(Q_OS_SYMBIAN) && defined(QT_DEBUG)
        server->showMinimized();
#endif
        return QtSingleApplication::exec();
    }

    return EXIT_FAILURE;
}

void HbThemeServerApplication::stop()
{
#ifndef Q_OS_SYMBIAN
    sendMessage(STOP_MESSAGE);
#endif // Q_OS_SYMBIAN
}

static bool hb_loadLibraryHelper(const QString &name)
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
#ifdef THEME_SERVER_TRACES
    if (!result) {
        qDebug() << "hb_loadLibraryHelper():" << library.errorString();
    }
#endif
    return result;
}

bool HbThemeServerApplication::loadLibrary(const QString &name)
{
    // To load resources embedded in hb library
    bool result = hb_loadLibraryHelper(name);
    if (!result) {
        // Library may not be loaded, if it was built in debug mode and the name in debug mode is
        // different, change the name to debug version in that scenario
        QString alternateName = name;
#ifdef Q_OS_WIN32
        alternateName += WIN32_DEBUG_SUFFIX;
#elif defined(Q_OS_MAC)
        alternateName += MAC_DEBUG_SUFFIX;
#endif
        // On symbian library name in debug mode is same as that in release mode,
        // so no need to do anything for that
        if (alternateName != name) {
            result = hb_loadLibraryHelper(alternateName);
        }
    }
#ifdef THEME_SERVER_TRACES
    if (result) {
        qDebug() << "HbThemeServerApplication::loadLibrary(): loaded library " << name;
    }
    else {
        qDebug() << "HbThemeServerApplication::loadLibrary(): could not load library " << name;
    }
#endif // THEME_SERVER_TRACES

    return result;
}

void HbThemeServerApplication::receiveMessage(const QString &message)
{
    if (!server) {
        return;
    }

    if (message == STOP_MESSAGE) {
        server->stopServer();
        quit();
    }
}

bool HbThemeServerApplication::acquireLock() {
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
        RProcess::Rendezvous(lockState == Lock::Reserved ? KErrAlreadyExists : KErrGeneral);
    }

    return (lockState == Lock::Acquired);
#else
    return true;
#endif
}
void HbThemeServerApplication::setPriority()
{
#ifdef Q_OS_SYMBIAN
    RProcess().SetPriority(EPriorityHigh);
#endif
}

#ifdef Q_OS_SYMBIAN
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
