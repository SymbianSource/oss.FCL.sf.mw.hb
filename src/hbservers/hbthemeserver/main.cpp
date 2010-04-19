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
#if defined (Q_OS_SYMBIAN)
#include <eikenv.h>
#include <apgwgnam.h>
#endif
#include <qtsingleapplication.h>
#include <QWindowsStyle>

static const QLatin1String RESOURCE_LIB_NAME("HbCore");
static const QLatin1String TEST_RESOURCE_LIB_NAME("HbTestResources");
static const QLatin1String WIN32_DEBUG_SUFFIX("d");
static const QLatin1String MAC_DEBUG_SUFFIX("_debug");

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
#endif
	QtSingleApplication  app( argc, argv );
	if (app.sendMessage("Am Alive"))
         return 0;
    
    loadResourceLibrary(RESOURCE_LIB_NAME);
#ifdef BUILD_HB_INTERNAL
    loadResourceLibrary(TEST_RESOURCE_LIB_NAME);
#endif
    HbThemeServer server;
    bool success = server.startServer();
    if ( !success ) {
        return -1;
    }

#if defined (Q_OS_SYMBIAN)
    CEikonEnv * env = CEikonEnv::Static();
    if ( env ) {
        CApaWindowGroupName* wgName = CApaWindowGroupName::NewLC(env->WsSession());
        wgName->SetHidden(ETrue); // hides us from FSW and protects us from OOM FW etc.
        wgName->SetSystem(ETrue); // Allow only application with PowerManagement cap to shut us down    
        wgName->SetCaptionL(_L("HbThemeServer"));
        wgName->SetAppUid(KNullUid);
        RWindowGroup &rootWindowGroup = env->RootWin();
        wgName->SetWindowGroupName(rootWindowGroup);
        rootWindowGroup.SetOrdinalPosition(-1, ECoeWinPriorityNormal); //move to background.        
        CleanupStack::PopAndDestroy();
    }
#elif defined(QT_DEBUG)
    server.showMinimized();
#endif

    return app.exec();
}

