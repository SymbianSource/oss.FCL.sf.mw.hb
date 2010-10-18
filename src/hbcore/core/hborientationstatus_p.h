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

#ifndef HBORIENTATIONSTATUS_P_H
#define HBORIENTATIONSTATUS_P_H

#include <QObject>
#include "hbsensorlistener_p.h"
#ifdef Q_OS_SYMBIAN
#include <e32property.h>
#endif

class HbWsOrientationListenerPrivate;

class HbOrientationStatus : public QObject, public HbSensorListenerObserver
{
    Q_OBJECT

public:
    static void init(QObject *parent, Qt::Orientation defaultOrientation);
    ~HbOrientationStatus();

    static bool currentOrientation(Qt::Orientation &orientation);

    static int mapOriToRenderOri(Qt::Orientation orientation);
    static bool mapRenderOriToOri(int renderOrientation, Qt::Orientation &orientation);
    static QString oriToString(Qt::Orientation orientation);
    static QString renderOriToString(int renderOrientation);

    class HbWsOrientationListener {
    public:
        void start(QObject *receiver, const char *member);
        void stop();
        bool get(int &renderOrientation);
        HbWsOrientationListener();
        ~HbWsOrientationListener();
    private:
        HbWsOrientationListenerPrivate *d;
    };

private slots:
    void storeOrientation();

private:
    HbOrientationStatus(QObject *parent, Qt::Orientation defaultOrientation);
    void sensorOrientationChanged(Qt::Orientation newOrientation);
    void sensorStatusChanged(bool status, bool resetOrientation);

private:
#ifdef Q_OS_SYMBIAN
    HbSensorListener *mSensorListener;
    RProperty mQtProperty;
    RProperty mWsProperty;
    Qt::Orientation mDefaultOrientation;
#endif
    Qt::Orientation mOrientationToBeStored;
};

#endif // HBORIENTATIONSTATUS_P_H
