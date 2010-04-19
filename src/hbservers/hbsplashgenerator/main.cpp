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

#if defined(Q_OS_SYMBIAN)
#include "hbsplashgen_server_symbian_p.h"
#include <eikenv.h>
#include <apgwgnam.h>
#endif

int main(int argc, char **argv)
{
    HbApplication app(argc, argv, Hb::NoSplash);

#if defined(Q_OS_SYMBIAN)
    CEikonEnv *env = CEikonEnv::Static();
    if (env) {
        CApaWindowGroupName *wgName = CApaWindowGroupName::NewLC(env->WsSession());
        env->RootWin().SetOrdinalPosition(0, ECoeWinPriorityNeverAtFront); // avoid coming to foreground
        wgName->SetHidden(ETrue); // hide from FSW, OOM fw, GOOM fw, etc.
        wgName->SetSystem(ETrue); // allow only apps with PowerManagement cap to shut us down
        _LIT(KCaption, "HbSplashGenerator");
        wgName->SetCaptionL(KCaption);
        wgName->SetAppUid(KNullUid);
        wgName->SetWindowGroupName(env->RootWin());
        CleanupStack::PopAndDestroy();
    }
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
    gen.connect(btnRegen, SIGNAL(clicked()), SLOT(regenerate()));
    mw.setCentralWidget(btnRegen);
    mw.show();
#endif

    // The server must be initialized before calling HbSplashGenerator::start().
#ifdef Q_OS_SYMBIAN
    qDebug("[hbsplashgenerator] starting server");
    HbSplashGenServer server(&gen);
#endif

    qDebug("[hbsplashgenerator] starting generator");
    gen.start(forceRegen);

    qDebug("[hbsplashgenerator] entering event loop");
    int ret = app.exec();

    qDebug("[hbsplashgenerator] exiting");
    return ret;
}
