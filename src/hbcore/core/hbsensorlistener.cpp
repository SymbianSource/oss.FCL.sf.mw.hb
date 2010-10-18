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

#include "hbsensorlistener_p.h"
#include "hborientationstatus_p.h"

#ifdef Q_OS_SYMBIAN
#include <sensrvchannelfinder.h>
#include <sensrvchannel.h>
#include <sensrvdatalistener.h>
#include <centralrepository.h>
#include <e32property.h>
#include "hbcommoncrkeys.h"
#include "hbsensornotifyhandler_p.h"
#else
#include <QSettings>
#endif

// Sensor listener construction for theme server app (listening sensor for
// writing the orientation value to PS)
HbSensorListener::HbSensorListener(HbSensorListenerObserver &observer,
                                   Qt::Orientation defaultOrientation,
                                   bool updateOrientation)
    :
    mObserver(observer),
    mDefaultOrientation(defaultOrientation),
    mOrientation(mDefaultOrientation),
    mEnabled(false),
    mSettingsEnabled(false)
#ifdef Q_OS_SYMBIAN
    ,
    mNotifyHandler(0),
    mSensrvChannel(0)
#endif
{
    checkCenrepValue();
    if (updateOrientation) {
        // Current device orientation from PS
        HbOrientationStatus::currentOrientation(mOrientation);
    }
}

HbSensorListener::~HbSensorListener()
{
#ifdef Q_OS_SYMBIAN
    if (mSensrvChannel) {
        mSensrvChannel->StopDataListening();
        mSensrvChannel->CloseChannel();
        delete mSensrvChannel;
    }
    if (mNotifyHandler) {
        delete mNotifyHandler;
    }
#endif
}

Qt::Orientation HbSensorListener::orientation() const
{
    return mOrientation;
}

void HbSensorListener::setSensorOrientation(Qt::Orientation dataOrientation)
{
    if (mOrientation != dataOrientation) {
        mOrientation = dataOrientation;
        mObserver.sensorOrientationChanged(mOrientation);
    }
}

void HbSensorListener::checkCenrepValue()
{
#ifdef Q_OS_SYMBIAN
    CRepository *repository = 0;
    TRAPD(err, repository = CRepository::NewL(KHbSensorCenrepUid));
    if (err) {
        hbWarning("HbSensorListener::checkCenrepValue; repository->NewL fails, error code = %d", err);
    } else {
        TInt value = 0;
        TInt err = repository->Get(KHbSensorCenrepKey, value);
        if (err == KErrNone) {
            cenrepValueChanged(value, false);
        } else {
            hbWarning("HbSensorListener::checkCenrepValue: repository->Get fails, error code = %d", err);
        }
    delete repository;
    }
    if (!mNotifyHandler) {
        TRAPD(err, mNotifyHandler = HbSensorNotifyHandler::NewL(*this));
        if (err) {
            hbWarning("HbSensorListener::HbSensorListener: HbSensorNotifyHandler::NewL failed = %d", err);
        } else {
            TRAPD(err, mNotifyHandler->startObservingL());
            if (err) {
                hbWarning("HbSensorListener::HbSensorListener: mNotifyHandler->startObservingL failed = %d", err);
            }
        }
    }
#else
    QSettings settings("Nokia", "Hb");
    settings.beginGroup("Sensors");
    bool enable = settings.value("SensorsEnabled").toBool();
    mSettingsEnabled = enable;
    enableSensors(enable, false);
    settings.endGroup();
#endif
}

void HbSensorListener::enableSensors(bool enable, bool notify, bool resetOrientation)
{
    mEnabled = enable;
#ifdef Q_OS_SYMBIAN
    enableSensorListening(enable);
#else
    mSettingsEnabled = enable;
#endif
    if (notify) {
        mObserver.sensorStatusChanged(enable, resetOrientation);
    }
}

bool HbSensorListener::isEnabled() const
{
    if (mSettingsEnabled) {
        return mEnabled;
    }
    return false;
}

#ifdef Q_OS_SYMBIAN
void HbSensorListener::enableSensorListening(bool enable)
{
    if (enable && !mSensrvChannel) {
        startSensorChannel();
    } else if (!enable && mSensrvChannel) {
        mSensrvChannel->StopDataListening();
        mSensrvChannel->CloseChannel();
        delete mSensrvChannel;
        mSensrvChannel = 0;
    }
}

void HbSensorListener::cenrepValueChanged(TInt aValue, bool notify)
{
    bool enable = (aValue == 0) ? false : true;
    mSettingsEnabled = enable;
    // When sensors are disabled in cenrep, reset orientation
    enableSensors(enable, notify, !enable);
}
#endif

void HbSensorListener::startSensorChannel()
{
#ifdef Q_OS_SYMBIAN
    if(!mSettingsEnabled) {
         return;
    }
    
    CSensrvChannelFinder *sensrvChannelFinder = CSensrvChannelFinder::NewLC();

    RSensrvChannelInfoList channelInfoList;
    CleanupClosePushL(channelInfoList);

    TSensrvChannelInfo mySearchConditions;

    //Search only Orientation events.
    mySearchConditions.iChannelType = KSensrvChannelTypeIdOrientationData;

    TRAPD(err, sensrvChannelFinder->FindChannelsL(channelInfoList, mySearchConditions));
    if (!err) {
        if (channelInfoList.Count()) {
            TRAP(err, mSensrvChannel = CSensrvChannel::NewL(channelInfoList[0]));
            if (!err) {
                TRAP(err, mSensrvChannel->OpenChannelL());
                if (!err) {
                    TRAP_IGNORE(mSensrvChannel->StartDataListeningL(this, 1, 1, 0));
                } else {
                    hbWarning("HbSensorListener::startSensorChannel fails, error code = %d", err);
                }
            }
        }

        channelInfoList.Close();
        CleanupStack::Pop(&channelInfoList);
        CleanupStack::PopAndDestroy(sensrvChannelFinder);
    } else {
        hbWarning("HbSensorListener::startSensorChannel fails, error code = %d", err);
    }
#endif
}

#ifdef Q_OS_SYMBIAN
Qt::Orientation HbSensorListener::orientationFromData(CSensrvChannel &aChannel, TInt aCount)
{
    Qt::Orientation orientation = mOrientation;
    if (aChannel.GetChannelInfo().iChannelType == KSensrvChannelTypeIdOrientationData) {
        TSensrvOrientationData data;
        for (TInt i = 0; i < aCount; ++i) {
            TPckgBuf<TSensrvOrientationData> dataBuf;
            aChannel.GetData(dataBuf);
            data = dataBuf();
            orientation = sensorOrientationToQtOrientation(data.iDeviceOrientation);
        }
    }
    return orientation;
}

//Returns only supported Qt orientations( Qt::vertical and Qt::Horizontal)
//Supported orientations form sensors are EOrientationDisplayRightUp and EOrientationDisplayUp.
Qt::Orientation HbSensorListener::sensorOrientationToQtOrientation(TSensrvOrientationData::TSensrvDeviceOrientation sensorOrientation) const
{
    if (mDefaultOrientation == Qt::Vertical || mDefaultOrientation == Qt::Horizontal) {
        if (sensorOrientation == TSensrvOrientationData::EOrientationDisplayRightUp) {
            return Qt::Horizontal;
        } else if (sensorOrientation == TSensrvOrientationData::EOrientationDisplayUp) {
            return Qt::Vertical;
        } else {
            //Return previous (supported) orientation
            return mOrientation;
        }
    } else {
        //Return previous (supported) orientation
        return mOrientation;
    }
}

// From MSensrvDataListener
void HbSensorListener::DataReceived(CSensrvChannel &aChannel, TInt aCount, TInt aDataLost)
{
    Q_UNUSED(aDataLost);
    Qt::Orientation dataOrientation = orientationFromData(aChannel, aCount);
    setSensorOrientation(dataOrientation);
}

// From MSensrvDataListener
void HbSensorListener::DataError(CSensrvChannel &aChannel, TSensrvErrorSeverity aError)
{
    Q_UNUSED(aChannel);
    Q_UNUSED(aError);
}

// From MSensrvDataListener
void HbSensorListener::GetDataListenerInterfaceL(TUid aInterfaceUid, TAny*& aInterface)
{
    Q_UNUSED(aInterfaceUid);
    Q_UNUSED(aInterface);
}
#endif
