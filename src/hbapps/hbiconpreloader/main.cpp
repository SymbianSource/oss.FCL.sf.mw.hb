/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbApps module of the UI Extensions for Mobile.
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

#include <hbapplication.h>
#include <hbmainwindow.h>
#include <QMainWindow>
#include <QDebug>

#ifdef Q_OS_SYMBIAN
#include <coemain.h>
#endif

/*
* This application exists temporarily to preload some time-consuming graphics
* when theme server is started in order to simulate the real scenario where
* such graphics get loaded in the boot when first app is shown.
* It creates more realistic results for the performance tests.
*/
int main(int argc, char *argv[])
{
    HbApplication app(argc, argv, Hb::NoSplash);
    // Create a mainwindow so its graphics get cached and next app startup is faster
    HbMainWindow window;
    window.show();
    
    qDebug() << "HB: HbIconPreloader started";
    
    // Render the window in a pixmap painter, it does not get drawn and icons do not get loaded
    // otherwise because the application runs in background.
    QImage image(1, 1, QImage::Format_ARGB32);
    QPainter painter(&image);

    window.render(&painter);

#ifdef Q_OS_SYMBIAN
    // To avoid crash in app exit
    CCoeEnv::Static()->DisableExitChecks(true);
#endif

    QApplication::processEvents(); // to prevent mysterious deadlocks when destroying the mainwindow

    // Exit the application
    return 0;
}
