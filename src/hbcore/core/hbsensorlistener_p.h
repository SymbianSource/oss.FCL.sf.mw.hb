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

#ifndef HBSENSORLISTENER_P_H
#define HBSENSORLISTENER_P_H

#include <qnamespace.h>
#include <hbglobal.h>

#ifdef Q_OS_SYMBIAN
#include <e32base.h>
#include <sensrvdatalistener.h>
#include <sensrvorientationsensor.h>
#endif

#ifdef Q_OS_SYMBIAN
class CSensrvChannel;
#endif
class HbSensorNotifyHandler;

class HbSensorListenerObserver {
public:
    virtual void sensorOrientationChanged(Qt::Orientation newOrientation) = 0;
    virtual void sensorStatusChanged(bool status, bool notify) = 0;
};

class HB_AUTOTEST_EXPORT HbSensorListener 
#ifdef Q_OS_SYMBIAN
    : public MSensrvDataListener
#endif
{

public:
    HbSensorListener(HbSensorListenerObserver &observer,
                     Qt::Orientation defaultOrientation,
                     bool updateOrientation = true);

    virtual ~HbSensorListener();
    Qt::Orientation orientation() const;

    void setSensorOrientation(Qt::Orientation dataOrientation);
    void enableSensors(bool enable, bool notify);
    bool isEnabled() const;
    	

#ifdef Q_OS_SYMBIAN
    void cenrepValueChanged(TInt aValue, bool notify = true);
#endif
    
private:
    void checkCenrepValue();
    void startSensorChannel();

#ifdef Q_OS_SYMBIAN
    Qt::Orientation orientationFromData(CSensrvChannel& aChannel, TInt aCount);
    Qt::Orientation sensorOrientationToQtOrientation(TSensrvOrientationData::TSensrvDeviceOrientation 
                                                      sensorOrientation) const;

    void enableSensorListening(bool enable);
    //From MSensrvDataListener
    void DataReceived(CSensrvChannel& aChannel, TInt aCount, TInt aDataLost);
    void DataError(CSensrvChannel& aChannel, TSensrvErrorSeverity aError);
    void GetDataListenerInterfaceL(TUid aInterfaceUid, TAny*& aInterface);
#endif
    
private:
    HbSensorListenerObserver& mObserver;
    Qt::Orientation mDefaultOrientation;
    Qt::Orientation mOrientation;
    bool mEnabled;
#ifdef Q_OS_SYMBIAN
    HbSensorNotifyHandler* mNotifyHandler;
    CSensrvChannel* mSensrvChannel;
    friend class TestHbSensorOrientation;
#endif
};

#endif //HB_SENSOR_LISTENER_H

