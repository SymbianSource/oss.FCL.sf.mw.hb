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

#include "hbforegroundwatcher_p.h"
#include <hbevent.h>
#include <hbnamespace.h>
#include <hbeffectinternal_p.h>
#include <hbinstance.h>
#include <hbsensorlistener_p.h>
#include "hbsleepmodelistener_p.h"
#include "hbmainwindow_p.h"
#ifdef HB_EFFECTS_OPENVG
#include <hbvgeffect_p.h>
#endif
#include <QEvent>
#include <QApplication>
#include "hbmemorymanager_p.h"
#include "hbthemecommon_p.h"

/*!
  \class HbForegroundWatcher

  \brief Listens for Symbian foreground-background notifications via CCoeEnv.

  Note that this class cannot be used to safely determine if the application is
  in foreground or not. For example there may not be a foreground-gained
  notification when the application is starting and we always assume that the
  application is started to foreground. Therefore use this class only to get
  notifications about loosing/gaining foreground after app startup.

  \internal
*/

/*!
  \fn void HbForegroundWatcher::foregroundLost()

  This signal is emitted when application is moved to background
*/

/*!
  \fn void HbForegroundWatcher::foregroundGained()

  This signal is emitted when application comes to foreground
*/

/*!
  \fn void HbForegroundWatcher::resumeAnimation()

  This signal is emitted when lights are switched on and the app is in foreground.
*/

/*!
  \fn void HbForegroundWatcher::stopAnimation()

  This signal is emitted when lights are switched off and the app is in foreground.
*/

HbForegroundWatcher *HbForegroundWatcher::instance()
{
    static HbForegroundWatcher *watcher = new HbForegroundWatcher(qApp);
    return watcher;
}

HbForegroundWatcher::HbForegroundWatcher(QObject *parent)
    : QObject(parent), mForeground(true), mLights(true), mSensorListener(0)
{
    connect(QApplication::instance(), SIGNAL(aboutToQuit()), SLOT(handleAboutToQuit()));
    QApplication::instance()->installEventFilter(this);
    HbSleepModeListener::instance(); // make sure the instance is created
    mSleepModeTimer.setSingleShot(true);
    connect(&mSleepModeTimer, SIGNAL(timeout()), this, SLOT(handleSensors()));
}

void HbForegroundWatcher::setSensorListener(HbSensorListener *sensorListener)
{
    mSensorListener = sensorListener;
}

/*!
  Callback function from MCoeForegroundObserver

  \internal
*/
void HbForegroundWatcher::HandleGainingForeground()
{
    if (THEME_SERVER_NAME == HbMemoryUtils::getCleanAppName()) {
        return;
    }

    if (!mForeground) {
        emit foregroundGained();
        if (!hbInstance->allMainWindows().isEmpty()) {
            emit resumeAnimation();
            if (!signalsBlocked()) {
                HbEffectInternal::resumeEffects();
            }
#ifdef Q_OS_SYMBIAN
            HbMainWindow *mWindow = HbInstance::instance()->allMainWindows().first();
            if (mWindow) {
                HbMainWindowPrivate::d_ptr(mWindow)->updateForegroundOrientationPSKey();
            }
#endif //Q_OS_SYMBIAN
        }
        mForeground = true;
    }
}

/*!
  Callback function from MCoeForegroundObserver

  \internal
*/
void HbForegroundWatcher::HandleLosingForeground()
{
    if (THEME_SERVER_NAME == HbMemoryUtils::getCleanAppName()) {
        return;
    }

    if (mForeground) {
        emit foregroundLost();
        if (!hbInstance->allMainWindows().isEmpty()) {
            emit stopAnimation();
            if (!signalsBlocked()) {
                HbEffectInternal::stopEffects();
            }
        }
        mForeground = false;
#ifdef HB_EFFECTS_OPENVG
        // Destroy the cached pixmaps of effects to prevent wasting resources
        // (system/graphics memory) while sitting in background. This is also
        // necessary to make the OpenVG filter effect caching working
        // properly. (if Qt would free the pixmaps' underlying VGImage when
        // going to background then this would render the filter effects' cached
        // pixmaps invalid so we just drop them here)
        HbVgEffect::releaseCachedResources();
#endif
    }
}

/*!
  \internal
*/
void HbForegroundWatcher::handleAboutToQuit()
{
    // Refrain from emitting foreground gained/lost signals during app shutdown.
    blockSignals(true);
}

/*!
  \internal
*/
void HbForegroundWatcher::handleSensors()
{
    if (!mLights && mSensorListener && mSensorListener->isEnabled()) {
        mSensorListener->enableSensors(false, true);
    }
    if (mLights && mSensorListener && !mSensorListener->isEnabled()) {
        mSensorListener->enableSensors(true, true);
    }
}

/*!
  \internal
*/
bool HbForegroundWatcher::eventFilter(QObject *obj, QEvent *event)
{
    // Some of the events we get here are broadcast to all widgets so we must be
    // careful to react only once => hence the [!]mSomething guards in the
    // conditions.

    if (event->type() == HbEvent::SleepModeEnter && mLights) {
        if (mForeground) {
            HbEffectInternal::stopEffects();
            emit stopAnimation();
        }
        mLights = false;
        mSleepModeTimer.start(2000);
    } else if (event->type() == HbEvent::SleepModeExit && !mLights) {
        if (mForeground) {
            HbEffectInternal::resumeEffects();
            emit resumeAnimation();
        }
        mSleepModeTimer.stop();
        mLights = true;
        handleSensors();
    } else if (event->type() == QEvent::ApplicationActivate && !mForeground) {
        HandleGainingForeground();
    } else if (event->type() == QEvent::ApplicationDeactivate && mForeground) {
        HandleLosingForeground();
    }
    return QObject::eventFilter(obj, event);
}
