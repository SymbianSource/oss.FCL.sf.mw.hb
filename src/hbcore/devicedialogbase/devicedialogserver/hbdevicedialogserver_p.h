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

#ifndef HBDEVICEDIALOGSERVER_P_H
#define HBDEVICEDIALOGSERVER_P_H

#include <hbglobal.h>

#include <QVariantMap>
#include <QObject>
#include <QtGlobal>

#include "hbindicatorclientinfo_p.h"

class HbMainWindow;
class HbDeviceDialogServerPrivate;
class HbDeviceDialogManager;
#if defined(Q_OS_SYMBIAN)
class RMessage2;
#endif // Q_OS_SYMBIAN

class HB_CORE_PRIVATE_EXPORT HbDeviceDialogServer: public QObject
{
    Q_OBJECT

public:

#if defined(Q_OS_SYMBIAN)
    typedef RMessage2 ClientCredentials;
#else // Q_OS_SYMBIAN
    typedef int ClientCredentials; // not used
#endif // Q_OS_SYMBIAN

    struct Parameters {
        const QString &mType; // in
        const ClientCredentials &mClientCredentials; // in
        Parameters(const QString &type, const ClientCredentials &cred) :
            mType(type), mClientCredentials(cred){}
    };
    struct DialogParameters : public Parameters {
        const QVariantMap &mData; // in
        quintptr mClientTag; // in
        int mError; // out
        DialogParameters(const QString &type, const ClientCredentials &cred,
            const QVariantMap &data, quintptr clientTag) :
                Parameters(type, cred), mData(data), mClientTag(clientTag), mError(0){}
    };
    struct IndicatorParameters : public Parameters {
        const QVariant &mData; // in
        IndicatorParameters(const QString &type, const ClientCredentials &cred,
            const QVariant &data) : Parameters(type, cred), mData(data){}
    };

public:
    HbDeviceDialogServer();
    virtual ~HbDeviceDialogServer();

    int error() const {return mError;}
    void setMainWindow(HbMainWindow *mainWindow);
    int showDeviceDialog(DialogParameters &parameters);
    int updateDeviceDialog(int identifier, const QVariantMap &data);
    int publishOrientation(int orientation);
    int closeDeviceDialog(int identifier);
    void deviceDialogClientClosing(quintptr sessionTag);
    int activateIndicator(IndicatorParameters &parameters);
    int deactivateIndicator(IndicatorParameters &parameters);
    QList<IndicatorClientInfo> indicatorClientInfoList() const;

public slots:
    void deviceDialogUpdate(int identifier, const QVariantMap &data) const;
    void deviceDialogClosed(int identifier, int closeReason);
    void indicatorActivated(const QList<IndicatorClientInfo> &indicatorInfoList);
    void indicatorUpdated(const QList<IndicatorClientInfo> &indicatorInfoList);
    void indicatorRemoved(const QList<IndicatorClientInfo> &indicatorInfoList);
    void indicatorUserActivated(const QVariantMap &data);
private:
    HbDeviceDialogManager *iManager;
    HbDeviceDialogServerPrivate *d_ptr;
    int mError;
    Q_DECLARE_PRIVATE_D( d_ptr, HbDeviceDialogServer )

};

#endif // HBDEVICEDIALOGSERVER_P_H
