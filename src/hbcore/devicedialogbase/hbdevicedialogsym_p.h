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

#ifndef HBDEVICEDIALOGSYM_P_H
#define HBDEVICEDIALOGSYM_P_H

#include "hbdevicedialog.h"
#include "hbdevicedialogclientsession_p.h"
#include <hbdevicedialogserverdefs_p.h>
#include <QEventLoop>
#include <e32base.h>


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

class HbDeviceDialogPrivate : public QObject,
                              public CActive
{
    Q_OBJECT

public:

    HbDeviceDialogPrivate();
    virtual ~HbDeviceDialogPrivate();

    void init(HbDeviceDialog::DeviceDialogFlags f);
    int connect();
    int initialize(bool connect = true);
    bool show(const QString &deviceDialogType, const QVariantMap &parameters);
    bool update(const QVariantMap &parameters);
    bool cancel();
    bool waitForClosed(QEventLoop::ProcessEventsFlag flags);
    QVariantMap receivedData() const;
    int error() const;
    void setError(int error);

    void RunL();
    void DoCancel();
    TInt RunError(TInt aError);
    void Start();
    int receiverCount();
    int symToDeviceDialogError(int errorCode);

public:

    Q_DECLARE_PUBLIC(HbDeviceDialog)


public:

    HbDeviceDialog *q_ptr;
    RHbDeviceDialogClientSession mHbSession;
    TPckgBuf<THbDeviceDialogSrvUpdateInfo> iUpdateInfo;
    TPckgBuf<int> iDeviceDialogId;
    HBufC8* iBuffer;
    TPtr8 iDataPtr;

    QVariantMap mDataReceived;
    int mLastError;
    CActiveSchedulerWait iWait;
    TBool iRequesting;
    TBool iConnecting;
    HbDeviceDialog::DeviceDialogFlags mDeviceDialogFlags;
    QEventLoop* mEventLoop;

    static HbDeviceDialogPrivate *d_ptr(HbDeviceDialog *dialog) {
        Q_ASSERT(dialog);
        return dialog->d_func();
    }
};

#endif // HBDEVICEDIALOGSYM_P_H
