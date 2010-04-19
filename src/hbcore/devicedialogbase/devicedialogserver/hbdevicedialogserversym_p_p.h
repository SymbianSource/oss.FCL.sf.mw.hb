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
#ifndef HBDEVICEDIALOGSERVERSYM_P_P_H
#define HBDEVICEDIALOGSERVERSYM_P_P_H

#include <e32base.h>
#include <QVariantMap>

#include "hbdevicedialogserver_p.h"
#include "hbindicatorclientinfo_p.h"

class HbDeviceDialogSession;
class HbIndicatorSessionHandler;

class HbDeviceDialogServerPrivate: public CPolicyServer
{
public:

    HbDeviceDialogServerPrivate();
    ~HbDeviceDialogServerPrivate();

    void AddSession();
    void RemoveSession();
    TInt AddSessionToList(const HbDeviceDialogSession *aSession);
    void RemoveSessionFromList(const HbDeviceDialogSession *aSession);

    void deviceDialogUpdate(int id, const QVariantMap &data) const;
    void deviceDialogClosed(int identifier, int closeReason);

    int showDeviceDialog(HbDeviceDialogServer::DialogParameters &parameters);
    int updateDeviceDialog(int id, QVariantMap &data);
    int closeDeviceDialog(int id);
    void deviceDialogClientClosing(quintptr sessionTag);
    int activateIndicator(HbDeviceDialogServer::IndicatorParameters &parameters);
    int deactivateIndicator(HbDeviceDialogServer::IndicatorParameters &parameters);
    QList<IndicatorClientInfo> indicatorClientInfoList() const;
    TInt AddIndicatorHandlerToList(const HbIndicatorSessionHandler *aSession);
    void RemoveIndicatorHandlerFromList(const HbIndicatorSessionHandler *aSession);
    void IndicatorsActivated(const QList<IndicatorClientInfo> &indicatorInfoList);
    void IndicatorsUpdated(const QList<IndicatorClientInfo> &indicatorInfoList);
    void IndicatorsDeactivated(const QList<IndicatorClientInfo> &indicatorInfoList);

    static TInt exitTimerCallback(TAny *aServer);

    CSession2* NewSessionL(const TVersion &aVersion,
                           const RMessage2 &aMessage) const;

    Q_DECLARE_PUBLIC(HbDeviceDialogServer)

public:

    HbDeviceDialogServer *q_ptr;
    void Init();

public:

    RPointerArray<HbDeviceDialogSession> iSessionList;
    RPointerArray<HbIndicatorSessionHandler> iIndicatorSessionList;
    int mSessionCount;
    // CPeriodic *mExitTimer; Not used as server not closing automatically
};

#endif // HBDEVICEDIALOGSERVERSYM_P_P_H
