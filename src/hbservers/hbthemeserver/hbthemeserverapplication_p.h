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
#ifndef HBTHEMESERVERAPPLICATION_P_H
#define HBTHEMESERVERAPPLICATION_P_H

#include <qtsingleapplication.h>

class HbThemeServer;

class HbThemeServerApplication : public QtSingleApplication
{
    Q_OBJECT

public:
    HbThemeServerApplication(int &argc, char *argv[]);
    ~HbThemeServerApplication();

    struct Options {
        static bool help;
        static bool start;
        static bool stop;
        static bool persistent;
        static QString error;
    };

    bool initialize();
    int exec();
    static void setPriority();

public slots:
    void stop();

private slots:
    void receiveMessage(const QString &message);

private:
    bool loadLibrary(const QString &name);

    HbThemeServer *server;
};

#ifdef Q_OS_SYMBIAN
#include <f32file.h>
class Lock
{
public:
    enum State {
        Reserved,
        Acquired,
        Error
    };
    Lock();
    ~Lock(){
        close();
    }
    void close()
    {
        mFile.Close();
        mFs.Close();
    }
    State acquire();

private:
    RFs mFs;
    RFile mFile;
};
#endif

// Guard against starting multiple copies of the server
class HbThemeServerLocker
{
public:
    HbThemeServerLocker() {}
    bool lock();
private:
    static bool serverExists();
private:
#ifdef Q_OS_SYMBIAN
    Lock mLock;
#endif
};

#endif // HBTHEMESERVERAPPLICATION_P_H
