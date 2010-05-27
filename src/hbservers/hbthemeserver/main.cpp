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
#include <QTextStream>
#include <cstdlib>

void showHelp(const QString &argv0, const QString &error = QString())
{
    QTextStream qerr(stderr);
    qerr << "Hb Theme Server" << endl;
    if (!error.isEmpty()) {
        qerr << argv0 << ": " << error << endl;
    }
    qerr << "Usage: " << argv0 << " [options]" << endl;
    qerr << endl;
    qerr << "Options:" << endl;
    qerr << "  -start               start a new server" << endl;
    qerr << "  -restart             same as -stop -start" << endl;
    qerr << "  -stop                stop an existing server instance" << endl;
    qerr << "  -persistent          keep server running when the last client disconnects" << endl;
    qerr << "  -h, -help            display this information" << endl;
}

int main(int argc, char *argv[])
{
    // Hiding theme server from the start up in first phase
#if QT_VERSION >= 0x040601
    QApplication::setAttribute(Qt::AA_S60DontConstructApplicationPanes);
#endif // QT_VERSION

    // We need to be up and running fast
    HbThemeServerApplication::setPriority();

    if(!HbThemeServerApplication::acquireLock()) {
        return 0;
    }
    
    HbThemeServerApplication app(argc, argv);

    if (HbThemeServerApplication::Options::help) {
        showHelp(argv[0], HbThemeServerApplication::Options::error);
        return EXIT_SUCCESS;
    }

    if (HbThemeServerApplication::Options::stop) {
        app.stop();
    }

    if (app.isRunning() || !HbThemeServerApplication::Options::start) {
        return EXIT_SUCCESS;
    }

    if (app.initialize()) {
        return app.exec();
    }

    return EXIT_FAILURE;
}
