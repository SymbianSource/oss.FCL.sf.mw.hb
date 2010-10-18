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

#ifndef HBMAINWINDOWORIENTATION_P_H
#define HBMAINWINDOWORIENTATION_P_H

#include <QObject>
#include <QList>
#include "hbsensorlistener_p.h"

class HbMainWindow;
class HbForegroundWatcher;

class HB_AUTOTEST_EXPORT HbMainWindowOrientation : public QObject,
    public HbSensorListenerObserver
{
    Q_OBJECT

public:
    static HbMainWindowOrientation *instance();
    ~HbMainWindowOrientation();
    void setFixedOrientation(Qt::Orientation fixedOrientation);
    Qt::Orientation sensorOrientation() const;
    bool isEnabled() const;

#ifndef Q_OS_SYMBIAN
    void forceSensorOrientationValue(Qt::Orientation orientation);
#endif

private slots:
    void handleForegroundGained();
    void handleForegroundLost();
    void handleWindowAdded(HbMainWindow *window);
    void handleWindowRemoved(HbMainWindow *window);

private:
    HbMainWindowOrientation(QObject *parent = 0);
    void sensorOrientationChanged(Qt::Orientation newOrientation);
    void sensorStatusChanged(bool status, bool resetOrientation);
    void notifyOrientationChange(bool animate, bool notifyWhenDisabled = false);

private:
    HbForegroundWatcher *mForegroundWatcher;
    bool mForeground;
    HbSensorListener *mSensorListener;
    bool mFixedOrientation;
    Qt::Orientation mOrientation;
    Qt::Orientation mDefaultOrientation;
    QList<HbMainWindow *> mWindowList;

#ifndef Q_OS_SYMBIAN
    friend class HbSettingsWindow;
    friend class TestHbSensorOrientation;
#endif
    friend class HbForegroundWatcher;
};

#endif //HBMAINWINDOWORIENTATION_P_H
