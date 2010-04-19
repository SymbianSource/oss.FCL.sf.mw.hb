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


#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <QtCore/QDir>

namespace QtLP_Private {
#include "qtlockedfile.h"
}

class QtLocalPeer : public QObject
{
    Q_OBJECT

public:
    QtLocalPeer(QObject *parent = 0, const QString &appId = QString());
    bool isClient();
    bool sendMessage(const QString &message, int timeout);
    QString applicationId() const
        { return id; }

Q_SIGNALS:
    void messageReceived(const QString &message);

protected Q_SLOTS:
    void receiveConnection();

protected:
    QString id;
    QString socketName;
    QLocalServer* server;
    QtLP_Private::QtLockedFile lockFile;

private:
    static const char* ack;
};
