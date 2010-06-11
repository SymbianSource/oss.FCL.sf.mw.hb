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

#include "hbmainwindoworientation_p.h"
#include "hbmainwindow_p.h"
#include "hbinstance_p.h"
#include "hbforegroundwatcher_p.h"
#include <QSettings>

HbMainWindowOrientation *HbMainWindowOrientation::instance()
{
    static HbMainWindowOrientation *hbOrientation = new HbMainWindowOrientation(qApp);
    return hbOrientation;
}

HbMainWindowOrientation::~HbMainWindowOrientation()
{
    delete mSensorListener;
}

//Informs sensor listener about fixed mainWindow orientation
void HbMainWindowOrientation::setFixedOrientation(Qt::Orientation fixedOrientation)
{
    mOrientation = fixedOrientation;
    mFixedOrientation = true;
}

//Returns the latest orientation interpreted from sensor data
Qt::Orientation HbMainWindowOrientation::sensorOrientation() const
{
    if (isEnabled()) {
        return mSensorListener->orientation();
    } else {
        return mDefaultOrientation;
    }
}

bool HbMainWindowOrientation::isEnabled() const
{
    return mSensorListener->isEnabled();
}

#ifndef Q_OS_SYMBIAN
void HbMainWindowOrientation::forceSensorOrientationValue(Qt::Orientation orientation)
{
    QSettings mSettings("Nokia", "HbStartUpDeskTopSensors");
    mSensorListener->setSensorOrientation(orientation);

    switch (orientation) {
    case Qt::Vertical:
        mSettings.setValue("Orientation", 2);
        break;
    case Qt::Horizontal:
        mSettings.setValue("Orientation", 1);
        break;
    default:
        break;
    }
}
#endif

void HbMainWindowOrientation::handleForegroundGained()
{
    mForeground = true;
    notifyOrientationChange(false, true);
}

void HbMainWindowOrientation::handleForegroundLost()
{
    mForeground = false;
}

void HbMainWindowOrientation::handleWindowAdded(HbMainWindow *window)
{
    mWindowList.append(window);
    if (isEnabled()) {
        mDefaultOrientation = HbMainWindowPrivate::d_ptr(window)->mDefaultOrientation;
    }
}

void HbMainWindowOrientation::handleWindowRemoved(HbMainWindow *window)
{
    mWindowList.removeOne(window);
}

HbMainWindowOrientation::HbMainWindowOrientation(QObject *parent)
    : QObject(parent),
      mSensorListener(0),
      mFixedOrientation(false)
{
    //Get mainwindows from instance. Can't use mainWindow() method from HbWidget,
    //because this class is not inherited from it
    mWindowList = hbInstance->allMainWindows();

    mForeground = true;

    //Get a current device profile
    HbDeviceProfile profile(HbDeviceProfile::current());
    // this is the default profile size
    QSize pSize(profile.logicalSize());

    //Determine the default orientation width < height -> portrait
    if (pSize.width() < pSize.height()) {
        mDefaultOrientation = Qt::Vertical;
    } else {
        mDefaultOrientation = Qt::Horizontal;
    }
    mOrientation = mDefaultOrientation;

    mSensorListener = new HbSensorListener(*this, mDefaultOrientation);

    mForegroundWatcher = HbForegroundWatcher::instance();
    mForegroundWatcher->setSensorListener(mSensorListener);

    connect(mForegroundWatcher, SIGNAL(foregroundGained()), SLOT(handleForegroundGained()));
    connect(mForegroundWatcher, SIGNAL(foregroundLost()), SLOT(handleForegroundLost()));

    // Need a notification when a mainwindow is added or removed in the future.
    connect(HbInstancePrivate::d_ptr(), SIGNAL(windowAdded(HbMainWindow *)), SLOT(handleWindowAdded(HbMainWindow *)));
    connect(HbInstancePrivate::d_ptr(), SIGNAL(windowRemoved(HbMainWindow *)), SLOT(handleWindowRemoved(HbMainWindow *)));

    // Update current orientation if sensors are enabled
    if (isEnabled()) {
        mOrientation = mSensorListener->orientation();
    }
}

void HbMainWindowOrientation::sensorOrientationChanged(Qt::Orientation newOrientation)
{
    mOrientation = newOrientation;
    notifyOrientationChange(true, false);
}

void HbMainWindowOrientation::sensorStatusChanged(bool status, bool notify)
{
    if (status) {
        foreach(HbMainWindow * window, mWindowList) {
            if (!mFixedOrientation && !HbMainWindowPrivate::d_ptr(window)->mUserOrientationSwitch) {
                HbMainWindowPrivate::d_ptr(window)->mAutomaticOrientationSwitch = true;
            }
        }
    }
    notifyOrientationChange(false, notify);
}

// Notifies orientation change only if
// 1) automatic orientation change is enabled
// 2) application is in foreground
// 3) application has not set fixed orientation for the main window
// 4) mainwindow's windowSurface exists
void HbMainWindowOrientation::notifyOrientationChange(bool animate, bool notifyWhenDisabled)
{
    Qt::Orientation newOrientation = mOrientation;
    // Default orientation when sensors are disabled
    if (!isEnabled() && notifyWhenDisabled) {
        newOrientation = mDefaultOrientation;
    }
    foreach(HbMainWindow * window, mWindowList) {
        void *surface(0);
        if (window) {
            surface = (void *)window->windowSurface();
        }
        if (!HbMainWindowPrivate::d_ptr(window)->mAutomaticOrientationChangeAnimation) {
            animate = false;
        }

        if ((isEnabled() || notifyWhenDisabled) && (mForeground || surface)
                && HbMainWindowPrivate::d_ptr(window)->mAutomaticOrientationSwitch) {
            HbMainWindowPrivate::d_ptr(window)->setTransformedOrientation(newOrientation, animate);
        }
    }
}
