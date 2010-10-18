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
    @stable
    @hbcore
    \class HbDeviceDialog
    \brief HbDeviceDialog displays a dialog on top of all running applications
    
    Device Dialogs are used to display information, queries and alerts for the device rather than
    for an application. Some example device dialogs are:
    - Low battery
    - Low memory
    - Incoming bluetooth message
    - Text message received
    
    Due to the importance of these dialogs they are
    displayed on top of all applications. The HbDeviceDialog class provides the client
    interface for creating these dialogs. This differs from other dialog classes such as 
    HbMessageBox in that 
    HbDeviceDialog is not a widget but is the interface to a framework that handles the dialogs.
    
    \section _framework Device Dialog Framework
    
    Any application is able to generate device dialogs. However, in
    order to display the dialogs on top of all applications, the device dialog framework
    uses a server. To display a dialog an application uses the HbDeviceDialog client interface
    to send requests for dialogs to the server. The HbDeviceDialog class can also be used or
    extended as part of a set of specialization classes to create different device dialog types. 
    
    Device dialog widgets are implemented in plugins loaded by the server.

    Shown below is a runtime view of the device dialog framework.

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

    HbDeviceDialog has no dependencies to other parts of the framework. Only to Qt.
    Thus it can be used also from engine components that have no user interface.

    \section _types Types of device dialogs

    There are two classes of device dialogs intended for application use:
     -# Generic device dialogs are displayed on top
    of all applications, except in the case of an incoming call. Examples of these are a "Low memory"
    message box and a "Receive message via bluetooth?" query. 
     -# Device notification dialogs are
    notifications displayed in the top left corner of the display. Examples are "New message" and
    "Low battery" notifications.

    Generic device dialogs interrupt the current foreground application.
    The user cannot interact with the application with a touch or keyboard until the dialog
    is dismissed. Device notification dialogs behave differently allowing interaction with the
    current foreground application. Another difference is that notification dialogs are shown
    serially instead of on top of each other.

    In addition, there are two device dialogs classes intended for platform use: Indicator menu and security.

    \section _operation Operation of device dialogs

    \subsection _construction Construction and identification

    Device dialog widgets are constructed dynamically by the device dialog service. Construction
    takes two parameters: a type and a set of parameters appropriate to the dialog type.
    The device dialog type is a unique string identifying the dialog. A search is made to find a
    plugin that can instantiate the requested dialog. The service loads the plugin and creates an
    instance of the widget.

    The parameter set is encapsulated in QMap<QString, QVariant>. Each device dialog implementation
    has a default value for all parameters. Only parameters that differ from the default need to be
    given. Parameters are <name, value> pairs. How the parameters are used depends on
    the plugin implementing the dialog. Suggested usage is <name, value> used as property
    name and value pairs of the dialog implementation. This makes it easy to set properties using
    QObject::setProperty(const char*, const QVariant&). If data types supported by QVariant are
    not suitable for a specific device dialog, Q_DECLARE_METATYPE can be used to add data types.

    \subsection _updating Updating and receiving data

    %Data can be sent to a device dialog after it has been launched using the update() function.
    %Data sent by the dialog is received by the dataReceived signal. A copy of the last data
    received is held by the class and can be fetched by a call to the receivedData() function.
    This allows data to be received without having to connect to a signal by waiting for a dialog
    to close and then getting the received data.

    \subsection _showing Showing device dialogs

    It is possible to launch multiple device dialogs from a single HbDeviceDialog object.
    However if there is a need to update a dialog or receive data from it, only the last dialog
    launched supports this. In this case instantiate the HbDeviceDialog class for each device
    dialog that needs communication (update or data receive) after a launch.

    HbDeviceDialog::show() method returns after the device dialog service has accepted the dialog.
    The service decides when the dialog is actually displayed. No assumption should be made
    about the timing when the dialog will display. It may get delayed depending on system state.
    If there is no need to receive data from the dialog widget, the HbDeviceDialog object can be
    destroyed after the show() method returns (can be allocated in a stack). In this case,
    it is assumed that the dialog widget closes itself by a timeout. System will close dialog
    widgets left running this way without a corresponding HbDialog instance, after 10 seconds.

    If any signals of HbDeviceDialog are connected, then the instance needs to exist until the
    dialog widget is dismissed. In this case the device dialog service will close all dialogs
    launched by the HbDeviceDialog instance when it is deleted making it inappropriate to allocate
    it into a stack.

    A function is provided to wait for the device dialog to be dismissed, making the dialog
    display synchronous.

    \subsection _misc Miscellaneous notes

    When the HbDeviceDialog object is created, it can reserve a communication channel to the device dialog
    service or the channel creation may be delayed until show(). This is controlled by a constructor
    flag. The default is the delayed resource reservation.

    The HbDeviceDialog class is not thread safe. If cancel() needs to called across threads, a queued
    signal slot connection can be used.

    \section _platform_hbdevicedialog Platform-specific implementation notes for HbDeviceDialog

    \subsection _nonsymbian_hbdevicedialog Non-Symbian
    The server is implemented only for the Symbian platform. On other platforms device dialogs are
    displayed on client's main window.

    \section _usecases Using the HbDeviceDialog class
    
    The code below launches a device dialog and continues execution
    \code
    // Launch a device dialog and continue execution. Assumes that the dialog widget closes
    // itself by a timeout.

    HbDeviceDialog deviceDialog;
    QVariantMap parameters;
    parameters.insertMulti(QString("text"), QVariant(QString("Warning text")));
    const char *deviceDialogType = "hb.devdlg.examplemessagebox/1.0";
    deviceDialog.show(QString(deviceDialogType), parameters);
    \endcode

    The code below demonstrates how to use HbDeviceDialog as a member variable and connect signals before launching the device dialog
    \code
    // Connect signals and launch a device dialog.

    connect(mDeviceDialog, SIGNAL(dataReceived(QVariantMap)),
        this, SLOT(dataReceived(QVariantMap)));
    connect(mDeviceDialog, SIGNAL(deviceDialogClosed()), this, SLOT(deviceDialogClosed()));
    connect(mDeviceDialog, SIGNAL(error(int)), this, SLOT(error(int)));
    QVariantMap parameters;
    mDeviceDialog.show(QString("hb.devdlg.examplequery/1.0"), parameters);
    \endcode    
    
    The code below shows how to wait until a dialog has closed
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

    \sa HbDeviceDialogPlugin, CHbDeviceDialogSymbian
*/

/*!
    \enum HbDeviceDialog::DeviceDialogError
    Defines device dialog error codes and ranges.
*/

/*!
    \var HbDeviceDialog::DeviceDialogError HbDeviceDialog::FrameworkErrors
    Start of an error range for errors originating from the device dialog framework (client or server).
*/

/*!
    \var HbDeviceDialog::DeviceDialogError HbDeviceDialog::PluginErrors
    Start of an error range for errors originating from the device dialog plugins. The framework passes
    these from the plugin unmodified.
*/

/*!
    \var HbDeviceDialog::DeviceDialogError HbDeviceDialog::ErrorTypeMask
    Mask for the error type part of the error code.
*/

/*!
    \var HbDeviceDialog::DeviceDialogError HbDeviceDialog::CancelledError
    Operation was cancelled by cancel().
*/

/*!
    \var HbDeviceDialog::DeviceDialogError HbDeviceDialog::SystemCancelledError
    Operation was cancelled by the device dialog framework.
*/

/*!
    \var HbDeviceDialog::DeviceDialogError HbDeviceDialog::InstanceExistsError
    A device dialog widget is a single instance and already exists (has been launched).
    See HbDeviceDialogPlugin::SingleInstance.
*/

/*!
    \enum HbDeviceDialog::DeviceDialogFlag
    Defines construct flags.
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

      This signal is emitted when data is received from a device dialog.
      
      \a data contains data from
      the dialog. The structure and meaning of the data is a contract between the dialog and
      a client. Suggested usage is a set of signal names and parameters.
*/

/*!
    \fn void HbDeviceDialog::deviceDialogClosed()

    This signal is emitted when a device dialog is closed. Any data sent by the dialog is indicated by
    the dataReceived() signal. If the signal is not connected, the latest data received is saved and
    can be retrieved using the receivedData() function.

    \sa dataReceived() receivedData()
*/

/*!
    \fn void HbDeviceDialog::error(int error)

    This signal is emitted when an error has occurred.
    
    \a error contains an error code.
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
    Constructs HbDeviceDialog.
    
    \param f Contains the construct flags. See DeviceDialogFlag
    \param parent Parent pointer.
    
    HbDeviceDialog can be allocated on the stack if no signals are to be connected. The Device dialog
    service keeps dialogs launched when the object goes out of scope. If any signals
    are connected, the device dialog service will clean all dialogs launched when the instance is
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

/*!
    Constructs HbDeviceDialog. Allows class derivation by shared d-pointer paradigm.
*/
HbDeviceDialog::HbDeviceDialog(HbDeviceDialogPrivate &dd, DeviceDialogFlags f, QObject *parent) :
    QObject(parent), d_ptr(&dd)
{
    Q_D(HbDeviceDialog);
    d->q_ptr = this;
    d->init(f);
}

/*!
    Destructs HbDeviceDialog. The sialog launched by show() is closed if deviceDialogClosed()
    or dataReceived() signals are connected to by an application. Otherwise the dialog is
    left executing and should close itself by timeout.
*/
HbDeviceDialog::~HbDeviceDialog()
{
    delete d_ptr;
}

/*!
    Shows a device dialog using the device dialog service.
    
    The function returns immediately after the service has accepted the dialog.
    Returns \c true if the dialog was accepted, \c false if an error occurred.
    The service decides when the dialog is actually displayed. Displaying may be delayed
    depending on system state.

    \param deviceDialogType Identifies a device dialog to be displayed by a name.
    \param parameters Defines properties of the dialog.

    \sa update(), waitForClosed(), cancel()
*/
bool HbDeviceDialog::show(const QString &deviceDialogType, const QVariantMap &parameters)
{
    return d_func()->show(deviceDialogType, parameters);
}

/*!
    Updates device dialog parameters by a set of new values. You must call show() before calling
    update(). If multiple dialogs have been launched by a single HbDeviceDialog
    instance, the last dialog launched receives the update. 
    
    \return Returns \c true on success and \c false if an error occurred.

    \sa show()
*/
bool HbDeviceDialog::update(const QVariantMap &parameters)
{
    return d_func()->update(parameters);
}

/*!
    Waits for a device dialog to be displayed and subsequently dismissed.
    
    \a flags specifies the flags passed to the
    QEventLoop::exec() function. 

    The wait is implemented by starting a new event loop. The following potential problems should be considered before using
    this function: 
    - Stack usage increases.
    - Depending on the application program flow, several event
    loops may get instantiated on top of each other. 
    - Application event processing continues while
    waitForClosed() executes. 
    - When the function returns, the application state may have changed. For example some
    objects may have been deleted or the application may have exited.

    <b>Note that starting an event loop isn't compatible with gestures.</b> Therefore if an application
    has a user interface, please don't use this function. Instead connect to signals.
    \return Returns \c true on success and \c false if an error occurred.
    \sa cancel()
*/
bool HbDeviceDialog::waitForClosed(QEventLoop::ProcessEventsFlag flags)
{
    return d_func()->waitForClosed(flags);
}

/*!
    Returns the latest data received from a device dialog.

    This function can be called instead of
    connecting to the dataReceived() signal. If the dataReceived() signal has connections, the latest data is
    not saved and this function returns an empty data structure.

    \sa waitForClosed()
*/
QVariantMap HbDeviceDialog::receivedData() const
{
    return d_func()->receivedData();
}

/*!
    Returns the last error.

    The last error is cleared when any other API function except error() is called.
*/
int HbDeviceDialog::error() const
{
    return d_func()->error();
}

/*!
    Cancels a device dialog and removes the dialog from the device dialog service.

    Removes the dialog if the dialog is waiting to be displayed or
    currently on display. Has no effect if
    the dialog has already been dismissed. If multiple dialogs have been launched by a single HbDeviceDialog
    instance, the last dialog launched is cancelled. The deviceDialogClosed() signal is emitted if
    a dialog was closed.

    \return Returns \c true on success and \c false if an error occurred.

    \sa show(), deviceDialogClosed()
*/
bool HbDeviceDialog::cancel()
{
    return d_func()->cancel();
}
