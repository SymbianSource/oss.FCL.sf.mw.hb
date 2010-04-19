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

/*!
    \class HbDeviceDialog
    \brief HbDeviceDialog displays dialogs on top of applications

    HbDeviceDialog is a concrete class. It can be used directly or as a part of
    specialization classes for different device dialog types. It's not a widget but interfaces
    to a device dialog service which is responsible for displaying device dialogs.
    Device dialog widgets are implemented in plugins loaded by the service.

    Below is a runtime view of device dialog framework.

    \dot
    digraph G {

        rankdir=LR;

        subgraph cluster_devicedialog_server {
            label = "Device Dialog Server";
            style=filled;
            color=lightgrey;
            rank=sink;
            node [shape = polygon, style=filled, color=white, fontsize = 10];
            widget1 [label = "hb.devdlg.errordialog/1.0"];
            widget2 [label = "hb.devdlg.warningdialog/1.0"];
            node [shape = ellipse, style=filled, color=white, fontsize = 14];
            deviceDialogManager [label = "DeviceDialog\nmanager"];
            edge [fontsize = 10, style = dotted];
            deviceDialogManager -> widget1 [label = ""];
            deviceDialogManager -> widget2 [label = ""];
            widget1 -> deviceDialogManager [label = ""];
            widget2 -> deviceDialogManager [label = ""];
        }

        subgraph cluster_client_1 {
            label = "Client 1";
            style=filled;
            color=lightgrey;
            node [shape = ellipse, style=filled, color=white, fontsize = 14];
            deviceDialog1 [label = "HbDeviceDialog"];
        }

        subgraph cluster_client_2 {
            label = "Client 2";
            style=filled;
            color=lightgrey;
            node [shape = ellipse, style=filled, color=white, fontsize = 14];
            deviceDialog2 [label = "HbDeviceDialog"];
        }

        subgraph cluster_key {
            label = "Key";
            subgraph cluster_key_process1 {
                label = "";
                style=filled;
                color=lightgrey;
                node [shape = polygon, style=filled, color=white, fontsize = 10];
                key_process1_c0 [label = "Plugin widget"];
                node [shape = ellipse, style=filled, color=white, fontsize = 10];
                key_process1_c1 [label = "Component"];
                edge [style = dotted, fontsize = 10];
                key_process1_c1 -> key_process1_c0 [label = "Function call"];
            }
            subgraph cluster_key_process2 {
                label = "Process";
                style=filled;
                color=lightgrey;
                node [shape = ellipse, style=filled, color=white, fontsize = 10];
                key_process2_c0 [label = "Component"];
                edge [fontsize = 10];
                key_process1_c1 -> key_process2_c0 [label = "Interprocess\ncommunication"];
            }
        }
        edge [style = filled, fontsize = 10];
        deviceDialog1 -> deviceDialogManager [label = "update"];
        deviceDialog2 -> deviceDialogManager [label = "update"];

        edge [style = filled, fontsize = 10];
        deviceDialogManager -> deviceDialog1 [label = "data"];
        deviceDialogManager -> deviceDialog2 [label = "data"];
    }
    \enddot

    HbDeviceDialog has no dependencies to other parts of the framework. Only to Qt. Thus it can be used
    also from engine components that have no user interface.

    There are three classes of device dialogs. Generic device dialogs which are displayed on top
    of all applications (incoming call is an exception). Examples of these are "Low memory"
    message box and "Receive message via bluetooth" query. Device notification dialogs are
    notifications displayed in a top left corner of a display. Examples are "New message" and
    "Low battery" notifications. Universal indicator menu shows enhanced status indicator
    information.

    Generic device dialogs and indicator menu interrupt current foreground application.
    User cannot interact with the application with a touch or keyboard until the dialog
    is dismissed. Device notification dialog behaves differently allowing interaction with the
    current foreground application.

    Device dialog widgets are constructed dynamically by the device dialog service. Construction
    takes two parameters: a type and a set of parameters appropriate to the dialog type.
    Widgets are implemented by plugins. Device dialog type is an unique string identifying the
    dialog. A search is made to find a plugin that can instantiate the requested
    dialog. The service loads the plugin and creates an instance of the widget.

    Parameter set is encapsulated in QMap<QString, QVariant>. Each device dialog implementation has
    a default value for all parameters. Only parameters that differ from the default needs to be
    given. Parameters are <name, value> pairs. How the parameters are used depends on
    the plugin implementing the dialog. Suggested usage is <name, value> used as property
    name and value pairs of the dialog implementation. This makes it easy to set properties using
    QObject::setProperty(const char*, const QVariant&). If data types supported by QVariant are
    not suitable for a specific device dialog, Q_DECLARE_METATYPE can be used to add data types.

    Data can be sent to a device dialog after it has been launched using update() function. Data
    sent by the dialog is received by dataReceived signal. A copy of the last data received
    is held by the class and can be fetched by a receivedData() function. This allows to receive
    data without having to connect to a signal by waiting for a dialog to close and then
    getting the received data.

    It is possible to launch multiple device dialogs from a single HbDeviceDialog object.
    However if there is a need to update a dialog or receive data from it, only the last one
    launched is able to do it. In this case instantiate HbDeviceDialog class for each device dialog
    that needs communication (update or data receive) after a launch.

    HbDeviceDialog::show() method returns after device dialog service has accepted the dialog.
    The service decides when the dialog is actually displayed. If there is no need to receive
    data from the dialog widget, HbDeviceDialog object can be destroyed after show method returns
    (can be allocated in a stack). A function is provided to wait for device dialog to be dismissed
    making the dialog display synchronous.

    If any signals of HbDeviceDialog are connected, then the instance needs to exist until the
    dialog is dismissed. In this case device dialog service will close all dialogs
    launched by the instance when it is deleted making it inappropriate to allocate HbDeviceDialog
    into a stack.

    Several clients can share the same device dialog widget by agreeing on an unique tag. See
    HbDeviceDialogWidgetPlugin for more information.

    When HbDeviceDialog object is created, it can reserve a communication channel to device dialog
    service or the channel creation may be delayed until show(). This is controlled by a constructor
    flag. Default is the delayed resource reservation.

    HbDeviceDialog class is not thread safe. If cancel() needs to called across threads, a queued
    signal slot connection can be used.

    \sa HbDeviceDialogPlugin

    \beta
    \hbcore
*/

/*!
    \enum HbDeviceDialog::DeviceDialogError
    Defines device dialog error codes and ranges.
*/
/*!
    \var HbDeviceDialog::DeviceDialogError HbDeviceDialog::FrameworkErrors
    Start of an error range for errors originating from device dialog framework (client or server).
*/
/*!
    \var HbDeviceDialog::DeviceDialogError HbDeviceDialog::PluginErrors
    Start of an error range for errors originating from device dialog plugins. The framework passes
    these from the plugin unmodified.
*/
/*!
    \var HbDeviceDialog::DeviceDialogError HbDeviceDialog::ErrorTypeMask
    Mask for error type part of the error code.
*/
/*!
    \var HbDeviceDialog::DeviceDialogError HbDeviceDialog::CancelledError
    Operation was cancelled by cancel().
*/
/*!
    \var HbDeviceDialog::DeviceDialogError HbDeviceDialog::SystemCancelledError
    Operation was cancelled by device dialog framework.
*/

/*!
    \enum HbDeviceDialog::DeviceDialogFlags
    Defines construct flags.
*/
/*!
    \var HbDeviceDialog::DeviceDialogFlags HbDeviceDialog::NoDeviceDialogFlags
    No flags specified.
*/
/*!
    \var HbDeviceDialog::DeviceDialogFlags HbDeviceDialog::NoFlags
    No flags specified.
*/
/*!
    \var HbDeviceDialog::DeviceDialogFlags HbDeviceDialog::ImmediateResourceReservationFlag
    Reserves resources immediately instead of delaying until show() is called.
*/

/*!
     \fn void HbDeviceDialog::dataReceived(QVariantMap data)

      This signal is emitted when data is received from a device dialog. \a data contains data from
      the dialog. The structure and meaning of the data is a contract between the dialog and
      a client. Suggested usage is a set of signal names and parameters.
*/

/*!
    \fn void HbDeviceDialog::deviceDialogClosed()

    This signal is emitted when a device dialog is closed. Any data sent by the dialog is indicated by
    the dataReceived() signal. If the signal is not connected, latest data received is saved and
    can be retrieved by a receivedData() function.

    \sa dataReceived() receivedData()
*/

/*!
    \fn void HbDeviceDialog::error(int error)

    This signal is emitted when an error has occurred. \a error contains an error code.
*/

#include "hbdevicedialog.h"

#include <QtGlobal>

// Device dialogs are implemented only for Symbian/S60 OS. All others use a stub which shows
// device dialogs in the calling process.
#if defined(Q_OS_SYMBIAN)
#include "hbdevicedialogsym_p.h"
#else
#include "hbdevicedialogwin32_p.h"
#endif // defined(Q_OS_SYMBIAN)

/*!
    Constructs an object. \a f contains construct flags. \a parent is a parent pointer.
    HbDeviceDialog can be allocated into a stack if no signals are to be connected. Device dialog
    service keeps dialogs launched when the object goes out of scope. If any signals
    are connected, device dialog service will clean all dialogs launched when the instance is
    deleted. In this case the object must remain in existence until the dialog widget is
    dismissed.
*/
HbDeviceDialog::HbDeviceDialog(DeviceDialogFlags f, QObject *parent) :
    QObject(parent), d_ptr(new HbDeviceDialogPrivate)
{
    Q_D(HbDeviceDialog);
    d->q_ptr = this;
    d->init(f);
}

HbDeviceDialog::HbDeviceDialog(HbDeviceDialogPrivate &dd, DeviceDialogFlags f, QObject *parent) :
    QObject(parent), d_ptr(&dd)
{
    Q_D(HbDeviceDialog);
    d->q_ptr = this;
    d->init(f);
}

HbDeviceDialog::~HbDeviceDialog()
{
    delete d_ptr;
}

/*!
    Shows a device dialog using a device dialog service. The function returns immediately after the
    service has accepted the dialog. Returns true if dialog was accepted, false if error occurred.
    The service decides when the dialog is actually displayed.

    \arg deviceDialogType identifies a device dialog to be displayed by a name.
    \arg parameters defines properties of the dialog.

    \sa update(), waitForClosed(), cancel()

    \code
    // Below code launches a device dialog and continues execution.

    HbDeviceDialog deviceDialog;
    QVariantMap parameters;
    parameters.insertMulti(QString("text"), QVariant(QString("Warning text")));
    const char *deviceDialogType = "hb.devdlg.examplemessagebox/1.0";
    deviceDialog.show(QString(deviceDialogType), parameters);
    \endcode

    \code
    // Below code has HbDeviceDialog as a member variable. It connects signals and launches a device dialog.

    connect(mDeviceDialog, SIGNAL(dataReceived(QVariantMap)),
        this, SLOT(dataReceived(QVariantMap)));
    connect(mDeviceDialog, SIGNAL(deviceDialogClosed()), this, SLOT(deviceDialogClosed()));
    connect(mDeviceDialog, SIGNAL(error(int)), this, SLOT(error(int)));
    QVariantMap parameters;
    mDeviceDialog.show(QString("hb.devdlg.examplequery/1.0"), parameters);
    \endcode
*/
bool HbDeviceDialog::show(const QString &deviceDialogType, const QVariantMap &parameters)
{
    return d_func()->show(deviceDialogType, parameters);
}

/*!
    Updates device dialog parameters by a set of new values. show() must be called before an
    update() can be called. If multiple dialogs have been launched by a single HbDeviceDialog
    instance, the last dialog launched receives the update. Returns true on success and false
    if error occurred.

    \sa show()
*/
bool HbDeviceDialog::update(const QVariantMap &parameters)
{
    return d_func()->update(parameters);
}

/*!
    Waits for a device dialog to be displayed and dismissed. \a flags specifies flags passed to
    QEventLoop::exec() function. Returns true on success and false if error occurred.

    The wait is implemented by starting a new event loop. Consider following caveats before using
    this function. Stack usage increases. Depending on application program flow, several event
    loops may get instantiated on top of each other. Application event processing continues while
    waitForClosed() executes. When it returns application state may have changed. For example some
    objects may have been deleted or application may have exited.

    <b>Note that starting an event loop isn't compatible with gestures.</b> Therefore if an application
    has an user interface, please don't use this function. Instead connect to signals.

    \sa cancel()

    \code
    // Below code launches a query and then waits for it to close.

    // Launch query
    HbDeviceDialog deviceDialog;
    QVariantMap parameters;
    deviceDialog.show(QString("hb.devdlg.samplequery/1.0"), parameters);
    // Wait for note to close
    deviceDialog.waitForClosed();
    // Get data the dialog sent
    QVariantMap data = deviceDialog.receivedData();
    \endcode
*/
bool HbDeviceDialog::waitForClosed(QEventLoop::ProcessEventsFlag flags)
{
    return d_func()->waitForClosed(flags);
}

/*!
    Returns the latest data received from a device dialog. This function can be called instead of
    connecting to dataReceived() signal. If dataReceived() signal has connections, latest data is
    not saved and this function returns empty data structure.

    \sa waitForClosed()
*/
QVariantMap HbDeviceDialog::receivedData() const
{
    return d_func()->receivedData();
}

/*!
    Returns a last error. Last error is cleared when any other API function than error() is called.
*/
int HbDeviceDialog::error() const
{
    return d_func()->error();
}

/*!
    Cancels a device dialog. The dialog is removed from the device dialog service. Does nothing if
    the dialog has already been dismissed. Removes dialog if it's waiting to be displayed or
    currently on display. If multiple dialogs have been launched by a single HbDeviceDialog
    instance, the last dialog launched is cancelled. deviceDialogClosed() signal is emitted if
    a dialog was closed. Returns true on success and false if error occurred.

    \sa show(), deviceDialogClosed()
*/
bool HbDeviceDialog::cancel()
{
    return d_func()->cancel();
}
