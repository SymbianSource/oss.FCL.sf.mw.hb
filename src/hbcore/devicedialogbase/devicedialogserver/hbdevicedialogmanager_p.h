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

#ifndef HBDEVICEDIALOGMANAGER_P_H
#define HBDEVICEDIALOGMANAGER_P_H

#include <QtGlobal>
#include <QObject>
#include <QVariantMap>

#include "hbdevicedialogserver_p.h"
#include "hbindicatorclientinfo_p.h"

class HbMainWindow;
class HbDeviceDialogManagerPrivate;

class HB_AUTOTEST_EXPORT HbDeviceDialogManager : public QObject
{
    Q_OBJECT

public:

    explicit HbDeviceDialogManager(QObject *parent = 0);
    virtual ~HbDeviceDialogManager();

    void setMainWindow(HbMainWindow *mainWindow);

    // Device dialog client related API
    int showDeviceDialog(HbDeviceDialogServer::DialogParameters &parameters);
    int updateDeviceDialog(int identifier, const QVariantMap &parameters);
    int closeDeviceDialog(int identifier);
    void deviceDialogClientClosing(quintptr clientTag);
    int activateIndicator(HbDeviceDialogServer::IndicatorParameters &parameters);
    int deactivateIndicator(HbDeviceDialogServer::IndicatorParameters &parameters);
    QList<IndicatorClientInfo> indicatorClientInfoList() const;

 signals:

     void deviceDialogUpdate(int identifier, const QVariantMap &data) const;
     void deviceDialogClosed(int identifier, int closeReason) const;
     void indicatorActivated(const QList<IndicatorClientInfo> &indicatorInfoList);
     void indicatorUpdated(const QList<IndicatorClientInfo> &clientInfo);
     void indicatorRemoved(const QList<IndicatorClientInfo> &indicatorInfoList);

private:

    // Private implementation
    HbDeviceDialogManagerPrivate * const d;
    friend class HbDeviceDialogManagerPrivate;

    Q_DISABLE_COPY(HbDeviceDialogManager)
};

#endif // HBDEVICEDIALOGMANAGER_P_H
