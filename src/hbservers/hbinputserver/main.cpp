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
#include <QMainWindow>
#include <QPlainTextEdit>
#include "hbinputserver_p.h"

#if defined(Q_OS_SYMBIAN)
#include <e32std.h>
#include <eikenv.h>
#include <apgwgnam.h>

_LIT(hbinputserver_name, "hbinputserver");
#endif // Q_OS_SYMBIAN

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    HbInputServer server;
#if defined(Q_OS_SYMBIAN)
    // Set necessary window group etc. parameters to hide the input server
    CEikonEnv *env = CEikonEnv::Static();
    if (env) {
        CApaWindowGroupName *wgName = CApaWindowGroupName::NewLC(env->WsSession());
        env->RootWin().SetOrdinalPosition(0, ECoeWinPriorityNeverAtFront); // avoid coming to foreground
        wgName->SetHidden(ETrue); // hide from FSW, OOM fw, GOOM fw, etc.
        wgName->SetSystem(ETrue); // allow only apps with PowerManagement cap to shut us down
        wgName->SetCaptionL(hbinputserver_name);
        wgName->SetAppUid(KNullUid);
        wgName->SetWindowGroupName(env->RootWin());
        CleanupStack::PopAndDestroy(wgName);
        RThread::RenameMe(hbinputserver_name);
    }
#else
    QMainWindow window;
    QPlainTextEdit *textViewer = new QPlainTextEdit(&window);
    window.setCentralWidget(textViewer);
    textViewer->setReadOnly(true);
    QObject::connect(&server, SIGNAL(debugMessage(QString)), textViewer, SLOT(appendPlainText(QString)));
    window.show();
#endif // Q_OS_SYMBIAN

    // Server start can fail if someone has already created the shared memory blocks,
    // i.e. the server is already running. In that case just exit.
    int returnValue = 0;
    if (server.start()) {
        returnValue = app.exec();
    }
    return returnValue;
}
