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

#include <QMainWindow>
#include <QPushButton>
#include <hbapplication.h>
#include "hbsplashgenerator_p.h"
#include "hbsplashdefs_p.h"

#if defined(Q_OS_SYMBIAN)
#include "hbsplashgen_server_symbian_p.h"
#include "hbsplashdefs_p.h"
#include <e32std.h>
#include <eikenv.h>
#include <apgwgnam.h>
CApaApplication *appFactory()
{
    return new HbSplashGenApplication;
}
#endif

int runMain(int argc, char **argv, void *mutexToSignal)
{
#if defined(Q_OS_SYMBIAN)
    HbApplication app(appFactory, argc, argv, Hb::NoSplash);
    CEikonEnv *env = CEikonEnv::Static();
    if (env) {
        CApaWindowGroupName *wgName = CApaWindowGroupName::NewLC(env->WsSession());
        env->RootWin().SetOrdinalPosition(0, ECoeWinPriorityNeverAtFront); // avoid coming to foreground
        wgName->SetHidden(ETrue); // hide from FSW, OOM fw, GOOM fw, etc.
        wgName->SetSystem(ETrue); // allow only apps with PowerManagement cap to shut us down
        wgName->SetCaptionL(hbsplash_server_name);
        wgName->SetAppUid(KNullUid);
        wgName->SetWindowGroupName(env->RootWin());
        CleanupStack::PopAndDestroy();
        RThread::RenameMe(hbsplash_server_name);
    }
#else
    Q_UNUSED(mutexToSignal);
    HbApplication app(argc, argv, Hb::NoSplash);
#endif

    qDebug("[hbsplashgenerator] initializing generator");
    HbSplashGenerator gen;
    bool forceRegen = false;
    char **args = argv;
    while (*++args) {
        if (!qstrcmp(*args, "-forcegen")) {
            forceRegen = true;
            break;
        }
    }

#if defined(Q_OS_SYMBIAN) || defined(HB_Q_WS_MAEMO)
    app.setQuitOnLastWindowClosed(false);
#else
    qDebug("[hbsplashgenerator] initializing ui");
    QMainWindow mw;
    QPushButton *btnRegen = new QPushButton("Regenerate");
    gen.connect(btnRegen, SIGNAL(clicked()), SLOT(uncachedRegenerate()));
    mw.setCentralWidget(btnRegen);
    mw.show();
#endif

    // The server must be initialized before calling HbSplashGenerator::start().
#ifdef Q_OS_SYMBIAN
    qDebug("[hbsplashgenerator] starting server");
    HbSplashGenServer server(&gen);
    // If there was an error (or an instance is already running (it is
    // possible in certain race conditions)) then exit right away.
    if (!server.startupSuccess()) {
        qDebug("[hbsplashgenerator] exiting due to failed server startup");
        return 0;
    }
#endif

    qDebug("[hbsplashgenerator] starting generator");
    gen.start(forceRegen);

#if defined(Q_OS_SYMBIAN)
    if (mutexToSignal) {
        qDebug("[hbsplashgenerator] signaling mutex");
        static_cast<RMutex *>(mutexToSignal)->Signal();
    }
#endif

    qDebug("[hbsplashgenerator] entering event loop");
    int ret = app.exec();

    qDebug("[hbsplashgenerator] exiting");
    return ret;
}

int main(int argc, char **argv)
{
    bool canStart = true;
    void *mutexToSignal = 0;
#if defined(Q_OS_SYMBIAN)
    RMutex mutex;
    _LIT(KGlobalLockName, "hbsplgen");
    bool mutexOk = mutex.OpenGlobal(KGlobalLockName) == KErrNone;
    if (!mutexOk) {
        mutexOk = mutex.CreateGlobal(KGlobalLockName) == KErrNone;
    }
    if (mutexOk) {
        mutex.Wait();
        mutexToSignal = &mutex;
    } else {
        qDebug("[hbsplashgenerator] failed to open/create mutex");
    }
    // Due to the mutex-based synchronization we should now be able to
    // use TFindServer to safely check if there is another instance
    // already running.
    TFindServer findServer(hbsplash_server_name);
    TFullName serverName;
    if (findServer.Next(serverName) == KErrNone) {
        qDebug("[hbsplashgenerator] found another instance, exiting");
        canStart = false;
        RProcess::Rendezvous(KErrAlreadyExists);
    }
#endif

    int result = 0;
    if (canStart) {
        try {
            result = runMain(argc, argv, mutexToSignal);
        } catch (...) {
            qDebug("[hbsplashgenerator] exiting due to exception in main()");
        }
    }

#if defined(Q_OS_SYMBIAN)
    if (mutexOk) {
        if (mutex.IsHeld()) {
            mutex.Signal();
        }
        mutex.Close();
    }
#endif

    return result;
}
