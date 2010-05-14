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
#include "hbdevicedialogwin32_p.h"

#include <string.h>

#include <QtGlobal>

#include <QVariant>
#include <QList>
#include <QLineEdit>
#include <QGraphicsProxyWidget>

#include <hbinstance.h>
#include <hbwidget.h>
#include <hbdevicedialog.h>
#include <hbdevicedialogerrors_p.h>
#include <hbdevicedialogplugin.h>
#include <hbdevicedialoginterface.h>

// Device dialogs are implemented only for Symbian/S60 OS. All others use a stub which shows
// device dialogs in the calling process.

HbDeviceDialogManager *HbDeviceDialogPrivate::mDeviceDialogManager = 0;

HbDeviceDialogPrivate::HbDeviceDialogPrivate()
{
    mInitDone = false;
    if (!mDeviceDialogManager) {
        mDeviceDialogManager = new HbDeviceDialogManager;
    }
}

HbDeviceDialogPrivate::~HbDeviceDialogPrivate()
{
    if (mDeviceDialogManager) {
        mDeviceDialogManager->deviceDialogClientClosing(reinterpret_cast<quintptr>(this));
    }
    if (mEventLoop && mEventLoop->isRunning()) {
        mEventLoop->exit(HbDeviceDialog::CancelledError);
    }
}

void HbDeviceDialogPrivate::init(HbDeviceDialog::DeviceDialogFlags f)
{
    Q_UNUSED(f);

    mLastError = HbDeviceDialogNoError;
    mEventLoop = 0;
    mClosedInfo.mDialogId = 0;
    mClosedInfo.mCloseReason = HbDeviceDialogNoError;
    mDialogId = 0;

    QList<HbMainWindow*> mainWindows = HbInstance::instance()->allMainWindows();
    // Unit tests may run without main window
    if (mainWindows.count()) {
        mDeviceDialogManager->setMainWindow(mainWindows.at(0));
    } else {
        return;
    }

    mDoWidgetCloseTimer.setSingleShot(true);
    connect(&mDoWidgetCloseTimer, SIGNAL(timeout()), this, SLOT(doWidgetClose()));
    connect(mDeviceDialogManager, SIGNAL(deviceDialogClosed(int, int)), this, SLOT(widgetClosed(int, int)));
    connect(mDeviceDialogManager, SIGNAL(deviceDialogUpdate(int, const QVariantMap &)), this, SLOT(deviceDialogData(int, const QVariantMap &)));

    mInitDone = true;
}

// Show device dialog
bool HbDeviceDialogPrivate::show(const QString &deviceDialogType,
    const QVariantMap &parameters)
{
    if (!mInitDone) {
        setError(HbDeviceDialogIllegalOperationError);
        return false;
    }

    mLastError = HbDeviceDialogNoError;
    mDataReceived.clear();

    HbDeviceDialogServer::ClientCredentials clientCredentials = 0;
    HbDeviceDialogServer::DialogParameters showParameters(deviceDialogType, clientCredentials,
        parameters, reinterpret_cast<quintptr>(this));
    int dialogId = mDeviceDialogManager->showDeviceDialog(showParameters);
    if (dialogId == 0) {
        Q_ASSERT(showParameters.mError != HbDeviceDialogNoError);
        setError(showParameters.mError);
        return false;
    }
    mDoWidgetCloseTimer.stop();
    mDialogId = dialogId;

    return true;
}

// Update device dialog
bool HbDeviceDialogPrivate::update(const QVariantMap &parameters)
{
    mLastError = HbDeviceDialogNoError;

    if (mDialogId) {
        int error = mDeviceDialogManager->updateDeviceDialog(mDialogId, parameters);
        if (error == HbDeviceDialogNoError) {
            return true;
        } else {
            setError(error);
            return false;
        }
    } else {
        setError(HbDeviceDialogIllegalOperationError);
        return false;
    }
}

// Cancel device dialog
bool HbDeviceDialogPrivate::cancel()
{
    mLastError = HbDeviceDialogNoError;
    int error = HbDeviceDialogNotFoundError;

    if (mDialogId) {
        error = mDeviceDialogManager->closeDeviceDialog(mDialogId);
    }
    if (error != HbDeviceDialogNoError) {
        setError(error);
    }
    return error == HbDeviceDialogNoError;
}

// Wait for the dialog to be closed
bool HbDeviceDialogPrivate::waitForClosed(QEventLoop::ProcessEventsFlag flags)
{
    mLastError = HbDeviceDialogNoError;

    if (mDialogId) {
        QEventLoop eventLoop;
        mEventLoop = &eventLoop;
        QPointer<HbDeviceDialog> guard = q_ptr;
        int ret = mEventLoop->exec(flags);
        if (guard.isNull()) {
            return false;
        }
        mEventLoop = 0;
        if (ret != HbDeviceDialogNoError) {
            return false;
        } else {
            return true;
        }
    } else {
        setError(HbDeviceDialogIllegalOperationError);
        return false;
    }
}

// Return last data received
QVariantMap HbDeviceDialogPrivate::receivedData() const
{
    return mDataReceived;
}

// Return last error
int HbDeviceDialogPrivate::error() const
{
    return mLastError;
}

// Set error
void HbDeviceDialogPrivate::setError(int error)
{
    mLastError = error;
    emit q_func()->error(error);
}

void HbDeviceDialogPrivate::deviceDialogData(int dialogId, QVariantMap data)
{
    if (dialogId == mDialogId) {
        // Signal data if there are connections. Otherwise keep a copy.
        if (q_ptr->receivers(SIGNAL(dataReceived(QVariantMap))) > 0) {
            emit q_func()->dataReceived(data);
        } else {
            mDataReceived = data;
        }
    }
}

// Device dialog widget closed
void HbDeviceDialogPrivate::widgetClosed(int dialogId, int closeReason)
{
    //printf("widgetClosed cur %d %d\n", mDialogId, noteId);
    if (dialogId == mDialogId) {
        mClosedInfo.mDialogId = dialogId;
        mClosedInfo.mCloseReason = closeReason;

        // Emit deviceDialogClosed() signal from a timer event to allow HbDeviceDialog API call to return
        // to caller before signal comes
        mDoWidgetCloseTimer.start();
    }
}

// emit closed signal
void HbDeviceDialogPrivate::doWidgetClose()
{
    if (mDialogId && mClosedInfo.mDialogId == mDialogId) {
        if (mClosedInfo.mCloseReason != HbDeviceDialogNoError) {
            setError(mClosedInfo.mCloseReason);
        }
        emit q_func()->deviceDialogClosed();
        if (mEventLoop) {
            mEventLoop->exit(mClosedInfo.mCloseReason);
        }
        mDialogId = 0;
    }
}
