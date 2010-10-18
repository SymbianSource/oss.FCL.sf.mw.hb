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

#ifndef HBINDICATORSESSIONHANDLER_H
#define HBINDICATORSESSIONHANDLER_H

#include <e32base.h>
#include <QObject>
#include <QVariantMap>
#include <QList>
#include <QString>
#include <QStringList>
#include "hbindicatorclientinfo_p.h"

class HbDeviceDialogServerPrivate;

class IndicatorQueueItem
{
public:

    IndicatorQueueItem(const IndicatorClientInfo &clientInfo,
                       IndicatorState state) :
        clientInfo(clientInfo), state(state) {}

public:

    IndicatorClientInfo clientInfo;
    IndicatorState state;
};

//stream operators for IndicatorQueueItem
QDataStream& operator << (QDataStream &outStream,
        const IndicatorQueueItem &obj);

QDataStream& operator >> (QDataStream &inStream,
        IndicatorQueueItem &obj);

class HbDeviceDialogSession;

class HbIndicatorSessionHandler : public CBase
    {

public: // Construction and destruction

    static HbIndicatorSessionHandler *NewL(HbDeviceDialogSession *aSession);
    ~HbIndicatorSessionHandler();

public:

    void HandleMessageL( const RMessage2 &aMessage );

    void IndicatorsActivated(const QList<IndicatorClientInfo> &activated);
    void IndicatorsUpdated(const QList<IndicatorClientInfo> &activated);
    void IndicatorsDeactivated(const QList<IndicatorClientInfo> &deactivated);
    void IndicatorUserActivated(const QVariantMap& data);
private:

    HbDeviceDialogServerPrivate& Server();

    void ActivateIndicatorL( const RMessage2 &aMessage );
    void DeactivateIndicatorL( const RMessage2 &aMessage );
    void IndicatorChannelRequestL( const RMessage2 &aMessage );

    void addToIndicatorQueue(
            const QList<IndicatorClientInfo> &clientInfoList,
            IndicatorState state);

    void WriteIndicatorInfoL();
    void WriteIndicatorDataL(const RMessage2 &aMessage);
    TInt DoWriteIndicatorInfoL(TInt &error);
    void indicatorTypeFromMessageL(const RMessage2 &aMessage, QString &indicatorType,
        QVariant &parameter) const;
    HbIndicatorSessionHandler(HbDeviceDialogSession *aSession);

private:
    HbDeviceDialogSession *iSession;
    TInt iMsgType;
    TBool iIndicatorChannelOpen;
    RMessage2 iIndicatorChannel;
    QList<IndicatorQueueItem> iIndicatorQueue;
    QStringList indicatorTypes;
    QVariantMap indicatorDataMap;
    };

#endif  // HBINDICATORSESSIONHANDLER_H
