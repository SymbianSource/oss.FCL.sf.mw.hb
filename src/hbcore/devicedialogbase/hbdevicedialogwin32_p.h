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

#ifndef HBDEVICEDIALOGWIN32_P_H
#define HBDEVICEDIALOGWIN32_P_H

#include <QtGlobal>

#include <QObject>
#include <QEventLoop>
#include <QVariant>
#include <QTimer>

#include "hbdevicedialog.h"

#include "hbdevicedialogmanager_p.h"

// Device dialog are implemented only for Symbian/S60 OS. All others use a stub which shows
// device dialogs in the calling process.

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class HbDeviceDialogPrivate : public QObject
{
    Q_OBJECT

public:
    HbDeviceDialogPrivate();
    virtual ~HbDeviceDialogPrivate();

    void init(HbDeviceDialog::DeviceDialogFlags f);

    bool show(const QString &deviceDialogType, const QVariantMap &parameters);
    bool update(const QVariantMap &parameters);
    bool waitForClosed(QEventLoop::ProcessEventsFlag flags);
    bool cancel();
    QVariantMap receivedData() const;
    int error() const;

    void setError(int error);

public slots:

    void deviceDialogData(int dialogId, QVariantMap data);
    void widgetClosed(int dialogId, int closeReason);
    void doWidgetClose();

public:

    Q_DECLARE_PUBLIC(HbDeviceDialog)

public:
    HbDeviceDialog *q_ptr;
    int mLastError;
    QVariantMap mDataReceived;
    QEventLoop* mEventLoop;
    QTimer mDoWidgetCloseTimer;
    struct {
        int mDialogId;
        int mCloseReason;
    } mClosedInfo;
    static HbDeviceDialogManager *mDeviceDialogManager;
    int mDialogId;
    bool mInitDone;
};

#endif // HBDEVICEDIALOGWIN32_P_H
