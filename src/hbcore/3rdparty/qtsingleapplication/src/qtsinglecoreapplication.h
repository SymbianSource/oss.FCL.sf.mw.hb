/****************************************************************************
**
** This file is part of a Qt Solutions component.
** 
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** 
** Contact:  Qt Software Information (qt-info@nokia.com)
** 
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** 
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
** 
****************************************************************************/


#include <QtCore/QCoreApplication>

class QtLocalPeer;

class QtSingleCoreApplication : public QCoreApplication
{
    Q_OBJECT

public:
    QtSingleCoreApplication(int &argc, char **argv);
    QtSingleCoreApplication(const QString &id, int &argc, char **argv);

    bool isRunning();
    QString id() const;

public Q_SLOTS:
    bool sendMessage(const QString &message, int timeout = 5000);


Q_SIGNALS:
    void messageReceived(const QString &message);


private:
    QtLocalPeer* peer;
};
