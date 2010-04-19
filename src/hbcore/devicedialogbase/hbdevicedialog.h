/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
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
#ifndef HBDEVICEDIALOG_H
#define HBDEVICEDIALOG_H

#include <hbglobal.h>

#include <QObject>
#include <QVariant>
#include <QtGlobal>
#include <QMap>
#include <QEventLoop>
#include <QFlags>

class HbDeviceDialogPrivate;

class HB_CORE_EXPORT HbDeviceDialog : public QObject
{
    Q_OBJECT

public:

    enum DeviceDialogError{
        FrameworkErrors = 0x00000000,
        PluginErrors = 0x10000000,
        ErrorTypeMask = 0xf0000000,
        CancelledError = 0x0fffffff,
        SystemCancelledError = 0x0ffffffe
    };

    enum DeviceDialogFlag{
        NoFlags = 0,
        ImmediateResourceReservationFlag = 1
    };
    Q_DECLARE_FLAGS(DeviceDialogFlags, DeviceDialogFlag)

    explicit HbDeviceDialog(DeviceDialogFlags f = NoFlags, QObject *parent = 0);
    ~HbDeviceDialog();

    bool show(const QString &deviceDialogType, const QVariantMap &parameters);
    bool update(const QVariantMap &parameters);
    bool waitForClosed(QEventLoop::ProcessEventsFlag flags = QEventLoop::AllEvents);
    QVariantMap receivedData() const;
    int error() const;
    bool cancel();

signals:
    void dataReceived(QVariantMap data);
    void deviceDialogClosed();
    void error(int error);

protected:
    HbDeviceDialog(HbDeviceDialogPrivate &dd, DeviceDialogFlags f = NoFlags,
        QObject *parent = 0);
    HbDeviceDialogPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbDeviceDialog)
    Q_DISABLE_COPY(HbDeviceDialog)
};

#endif // HBDEVICEDIALOG_H
