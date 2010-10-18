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

#ifndef HBINPUTSERVER_METHODS_P_H
#define HBINPUTSERVER_METHODS_P_H

#include <QObject>
#include "hbinputserver_p.h"

class QString;
class QSharedMemory;
class HbInputMethodListItem;
class QByteArray;
class QFileSystemWatcher;

class HbInputServerMethods : public QObject
{
    Q_OBJECT

public:
    explicit HbInputServerMethods(HbInputServer *server);
    ~HbInputServerMethods();

    bool initialize();

private:
    void updateLocalInputMethodList(const QString &path = QString());
    void startMonitoringMethodListChanges();
    void updateSharedMethodList();

public slots:
    void changeInMethodDirectory(const QString &path);

private:
    HbInputServer *mServer;
    QSharedMemory *mSharedMethods;
    QSharedMemory *mSharedMethodsModTime;
    QList<HbInputMethodListItem> *mMethods;
    QByteArray *mMethodsSerialized;
    QFileSystemWatcher *mMethodPathWatcher;
};

#endif //HBINPUTSERVER_METHODS_P_H
