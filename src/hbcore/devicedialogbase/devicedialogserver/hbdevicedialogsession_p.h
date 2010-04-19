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

#ifndef HBDEVICEDIALOGSESSION_P_H
#define HBDEVICEDIALOGSESSION_P_H

#include <e32base.h>
#include <QObject>
#include <QVariantMap>
#include <QList>
#include <QString>

class HbDeviceDialogServerPrivate;
class HbIndicatorSessionHandler;

class HbDeviceDialogSession : public CSession2
    {
    friend class HbIndicatorSessionHandler;
public: // Construction and destruction

    static HbDeviceDialogSession *NewL();
    ~HbDeviceDialogSession();

public:

    virtual void CreateL();
    void ServiceL( const RMessage2 &aMessage );
    void DispatchMessageL( const RMessage2 &aMessage );

    TInt DeviceDialogIdentifier() const;
    void DeviceDialogUpdate( const QVariantMap &parameters );
    void DeviceDialogClosed( TInt identifier, TInt closeReason );

private:

    HbDeviceDialogServerPrivate& Server();
    virtual void Disconnect( const RMessage2 &aMessage );

    void ShowDeviceDialogL( const RMessage2 &aMessage );
    void UpdateDeviceDialogL( const RMessage2 &aMessage );
    void CancelDeviceDialog( const RMessage2 &aMessage);
    void ClientClosing( const RMessage2 &aMessage );
    void CancelRequest( const RMessage2 &aMessage );
    void UpdateChannelRequestL( const RMessage2 &aMessage );
    void UpdateDataRequestL( const RMessage2 &aMessage );
    void CancelUpdateChannel(const RMessage2 aMessage);
    void WriteUpdateData(const QVariantMap &parameters, int deviceDialogId);
    int WriteCloseData(int deviceDialogId, int closeReason);
    QString indicatorTypeFromMessageL(const RMessage2 &aMessage) const;
    HbDeviceDialogSession();

private:
    bool iKeepAfterClose;
    int iWriteCloseId;
    int iWriteCloseReason;
    RMessage2 iUpdateChannel;
    QList<QVariantMap> mEventList;
    QList<int> mIdList;
    TBool iUpdateChannelOpen;
    HbIndicatorSessionHandler *iIndicatorSessionHandler;
};

#endif  // HBDEVICEDIALOGSESSION_P_H

