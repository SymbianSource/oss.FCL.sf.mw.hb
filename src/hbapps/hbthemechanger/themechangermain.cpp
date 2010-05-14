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
#include "themechangerdefs.h"
#include "themeselectionlist.h"
#include <QTextStream>
#ifdef Q_OS_SYMBIAN
#include "themeclientsymbian.h"
#else
#include "themeclientqt.h"
#endif

#ifdef THEME_CHANGER_TIMER_LOG
void debugOutput(QtMsgType type, const char *msg)
 {
    QFile file("c:/data/logs/hbthemechanger.txt");

    if (!file.open(QIODevice::Append))
        return;
    QTextStream out(&file);

    switch (type)
    {
    case QtDebugMsg:
        out << msg << QChar::LineSeparator;
        break;
    case QtWarningMsg:
        break;
    case QtCriticalMsg:
        break;
    case QtFatalMsg:
        break;
    }
 }
#endif //THEME_CHANGER_TIMER_LOG

int main(int argc, char *argv[])
{
#ifdef THEME_CHANGER_TIMER_LOG
    qInstallMsgHandler(debugOutput);
#endif
    // Initialization
    HbApplication app(argc, argv);
    app.setApplicationName("ThemeChanger");
    // Main window widget. 
    // Includes decorators such as signal strength and battery life indicator.
    HbMainWindow mainWindow;
    
#ifdef Q_OS_SYMBIAN
    ThemeClientSymbian* client = new ThemeClientSymbian();
#else
    // Create the theme server that does the notifiation to all the QT apps
    ThemeClientQt* client = new ThemeClientQt();
#endif

    // Show the list of themes available
    ThemeSelectionList *themelist=new ThemeSelectionList(client);
    themelist->displayThemes();
    
    mainWindow.addView( themelist );

    // Show widget
    mainWindow.show();
    // Enter event loop
    return app.exec();
}

