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

#ifndef HBINDICATORSYM_P_H
#define HBINDICATORSYM_P_H

#include "hbglobal.h"
#include "hbindicator.h"
#include "hbdevicedialogclientsession_p.h"

#include "hbindicatorclientinfo_p.h"

const TInt KHbMessageBufferSize = 255;

class CAsyncListener;
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

class HB_AUTOTEST_EXPORT HbIndicatorPrivate : public QObject,
                           public CActive
{
    Q_OBJECT

private: // types
    enum {
        DefaultReceiveBufferSize = 256
    };

public: // methods

    HbIndicatorPrivate();
    virtual ~HbIndicatorPrivate();

    void init();
    void initializeL(bool forListening = false);
    bool activate(const QString &indicatorType, const QVariant &parameter = QVariant());
    bool deactivate(const QString &indicatorType, const QVariant &parameter = QVariant());
    bool startListen();
    bool listening() const;
    bool stopListen();
    void setError(int error);
    int error() const;

    void RunL();
    void DoCancel();
    TInt RunError(TInt aError);
signals:
    void activated(const QList<IndicatorClientInfo> &clientInfo);
    void updated(const QList<IndicatorClientInfo> &clientInfo);
    void allActivated(const QList<IndicatorClientInfo> &clientInfo);
    void deactivated(const QList<IndicatorClientInfo> &clientInfo);
    void error(int errorCode);

public:
    Q_DECLARE_PUBLIC(HbIndicator)
private:
    void Start();
    void Continue();
    void Close();
    TBool sendActivateMessage(const QString &indicatorType, TBool activate, const QVariant& data);
    void CreateReceiveBufferL(int size);

private: // data

    HbIndicator *q_ptr;
    RHbDeviceDialogClientSession mHbSession;
    TBool iInitialized;
    TBool iRequesting;

    TInt iMsgType;
    TPtr8 iMsgTypePtr;

    HBufC8* iBuffer;
    TPtr8 iDataPtr;

    QList<IndicatorClientInfo> iActivatedBuffer;

    int iLastError;
    CAsyncListener *iAsyncListener;
    friend class CAsyncListener;
};

#endif // HBINDICATORSYM_P_H
