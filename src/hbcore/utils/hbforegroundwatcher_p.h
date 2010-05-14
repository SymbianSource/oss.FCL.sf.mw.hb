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

#ifndef HBFOREGROUNDWATCHER_P_H
#define HBFOREGROUNDWATCHER_P_H

#include <QObject>
#include <QTimer>
#include <hbglobal.h>

#ifdef Q_OS_SYMBIAN
#include <coemain.h>
#else
class CCoeEnv;
#endif

class HbMainWindowOrientation;
class HbSensorListener;

class HB_CORE_PRIVATE_EXPORT HbForegroundWatcher : public QObject
#ifdef Q_OS_SYMBIAN
, public MCoeForegroundObserver
#endif
{
    Q_OBJECT

public:
    static HbForegroundWatcher *instance();
    void setSensorListener(HbSensorListener *sensorListener);
        
signals:
    void foregroundGained();
    void foregroundLost();
    void stopAnimation();
    void resumeAnimation();

public: // from MCoeForegroundObserver
    void HandleGainingForeground();
    void HandleLosingForeground();

private slots:
    void handleAboutToQuit();
    void handleSensors();

private:
    HbForegroundWatcher(QObject *parent = 0);
    bool eventFilter(QObject *obj, QEvent *event);

    bool mForeground;
    bool mLights;
    QTimer mSleepModeTimer;
    HbSensorListener *mSensorListener;
    CCoeEnv *mStaticEnv;
};

#endif
