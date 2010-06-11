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
    \class HbDeviceDialogPlugin
    \brief HbDeviceDialogPlugin is an abstract base class for device dialog widget plugins.

    Device dialogs are widgets which are shown on top of all applications by a device dialog
    service. The service queues and displays device dialogs according to client requests and
    system state. The service displays widgets in it's window (HbApplication window).

    <b>Platform dependent.</b> Currently server is implemented only for Symbian platform. On other platforms,
    device dialog are displayed on client's mainwindow.

    Device dialog widgets are created by plugins. The service loads a plugin to create and display
    a device dialog. A single plugin may implement several different dialogs or just one.
    Dialogs are identified by a string. By convention the string should follow
    inverted domain name format. For example com.nokia.hb.devicedialog.note/1.0.
    Function deviceDialogTypes() returns a list of dialog types the plugin implements.

    Appending version number into device dialog type string enables versioning of device dialogs.
    A plugin should handle versioning by returning device dialog type string for all versions it
    implements in deviceDialogTypes() and then in createDeviceDialog() create a device dialog
    instance compatible with the version requested. This could always be the latest version if it
    is backwards compatible with older versions. Device dialog framework is unaware of version
    numbers in type strings. It performs string comparison of the whole string when searching
    for a plugin.

    Device dialogs are divided into five groups: generic, notification, indicator-menu, security
    and critical groups. The group dictates how and when a dialog is shown. Generic and
    notification dialogs are intended for application use. Indicator, security and critical dialogs are
    for platform. Each dialog indicates which group it belongs to. The group need not be fixed.
    It may change depending on create parameters.

    Device dialog groups may prioritize (queueing) device dialogs in respect to other groups.
    In addition each device dialog has an individual queueing priority that may affect how it is
    shown inside a group. Currently there is no queueing implemented for device dialogs. All
    dialogs are displayed in order requests are received. Z-order of dialogs on display is
    determined by popup framework. Last dialog created is on top. Device dialog framework
    manipulates Z-order of dialogs. Device dialogs themselves should not change the popup Z-order.

    Device dialog widgets have to be derived from HbPopup either directly or by ancestry.

    <div style="color:gray">
    <b>For future needs. Not implemented.</b>
    Device dialog may be shared by several device dialog clients. Each client can update the
    device dialog and receive signals from it equally. Clients attach to a shared device dialog
    instance by agreeing on an unique tag. The tag is appended into device dialog type with a %
    separator. For example here the tag "#1" is appended into device dialog type
    "hb.devicedialog.note/1.0%#1". If a device dialog instance exists, a client is attached into it
    instead of creating a new one. To allow sharing, plugin must set
    HbDeviceDialogPlugin::SharedDeviceDialog.
    </div>

    <b>Symbian.</b>Plugins are run by a server with platform security capabilities ProtServ, SwEvent,
    TrustedUI and ReadDeviceData.

    Plugins are responsible for maintaining system security for their own part. If plugin
    performs operations that may compromise security or want's to limit access to specific
    clients, it should check client security credentials in accessAllowed() function.
    Device dialog framework calls this function before creating or attaching client to a
    plugin if HbDeviceDialogPlugin::SecurityCheck is set. Access is denied if the
    function returns false. In addition, HbDeviceDialogPlugin constructor has a check which
    allows only device dialog service to load plugins derived from it.

    <b>Symbian.</b>Device dialog plugins can be installed into a device. If a plugin doesn't have
    required platform security capabilities it will not load. Plugins are searched from device's
    local drives. The search order is rom-drives, non-removable drives and removable drives. Once
    a pluging is found the search stops. This implies that plugins on rom drives cannot be
    overriden by plugins on other drives.

    <b>Platform dependent.</b>Plugin location differs depending on platform. On Symbian, device dialog
    plugin stubs (.qtplugin) are located in /resource/plugins/devicedialogs directory and executables
    in /sys/bin directory. On other platforms plugin executables are searched from application's
    current directory and HB_PLUGINS_DIR/devicedialogs directory.

    Well behaving dialog widget should observe following rules:
    - Should not create and show other dialogs
    - No calls to show(), hide() or setVisible()
    - Not setting popup Z-order
    - No time consuming operations performed

    Creating a device dialog plugin and widget involves following steps.
    - Set in .pro file TEMPLATE = lib and CONFIG += hb plugin
    - Derive a class from HbPopup or derivatives and HbDeviceDialogInterface
    \code
    class HbSampleMessageBoxWidget : public HbMessageBox, public HbDeviceDialogInterface
    {
        Q_OBJECT
    \endcode
    - Implement device dialog widget and HbDeviceDialogInterface interface. Declare and emit
      deviceDialogClosed and optionally deviceDialogData signals. Do not call show(), hide() or
      setVisible() in the plugin. Device dialog framework calls show() to display the widget.
    \code
    public:
        bool setDeviceDialogParameters(const QVariantMap &parameters);
        int deviceDialogError() const;
        void closeDeviceDialog(bool byClient);
        HbPopup *deviceDialogWidget() const;
    signals:
        void deviceDialogClosed();
        void deviceDialogData(QVariantMap data);
    \endcode
    - Wrap the widget into a plugin derived from HbDeviceDialogPlugin
    \code
    class HbSampleMessageBoxPlugin : public HbDeviceDialogPlugin
    {
        Q_OBJECT
    \endcode
    - Implement HbDeviceDialogPlugin pure virtual functions
    \code
        bool accessAllowed(const QString &deviceDialogType,
            const QVariantMap &parameters, const QVariantMap &securityInfo) const;
        HbDeviceDialogInterface *createDeviceDialog(const QString &deviceDialogType, const QVariantMap &parameters);
        bool deviceDialogInfo(const QString &deviceDialogType,
            const QVariantMap &parameters, DeviceDialogInfo *info) const;
        QStringList deviceDialogTypes() const;
        PluginFlags pluginFlags() const;
        int error() const;
    \endcode

    <b> Class diagram of the sample plugin:</b>
    \dot
    digraph G {
        rankdir=LR;

        subgraph cluster_class_diagram {
            style=solid;

            node [shape = box, style=solid, fontsize = 10];
            HbSampleMessageBoxPlugin [label = "HbSampleMessageBoxPlugin"];
            QObject [label = "QObject"];
            HbDeviceDialogPluginInterface [label = "HbDeviceDialogPluginInterface"];
            HbDeviceDialogPlugin [label = "HbDeviceDialogPlugin"];
            edge [fontsize = 10, style = filled];
            QObject -> HbDeviceDialogPlugin [label = "is a"];
            HbDeviceDialogPluginInterface -> HbDeviceDialogPlugin [label = "is a"];
            HbDeviceDialogPlugin -> HbSampleMessageBoxPlugin [label = "is a"];

            HbSampleMessageBoxWidget [label = "HbSampleMessageBoxWidget"];
            HbMessageBox [label = "HbMessageBox"];
            HbDeviceDialogInterface [label = "HbDeviceDialogInterface"];
            HbMessageBox -> HbSampleMessageBoxWidget [label = "is a"];
            HbDeviceDialogInterface -> HbSampleMessageBoxWidget [label = "is a"];

            edge [fontsize = 10, style = dotted];
            HbSampleMessageBoxPlugin -> HbSampleMessageBoxWidget  [label = "creates"];
        }

        subgraph cluster_key {
            label = "Key";
            style=solid;
            node [shape = box, style=solid, fontsize = 10];
            Class1 [label = "Class"];
            Class2 [label = "Class"];
            Class3 [label = "Class"];

            edge [fontsize = 10, style = filled];
            Class2 -> Class1 [label = "generalization"];
            edge [fontsize = 10, style = dotted];
            Class3 -> Class1 [label = "dependency"];
            }
    }
    \enddot

    Sample plugin implementations can be found in src/hbplugins/devicedialogs directory.

    \sa HbDeviceDialogPluginInterface HbDeviceDialogInterface HbDeviceDialog HbPopup

    \stable
    \hbcore
*/

/*!
    \enum HbDeviceDialogPlugin::PluginFlag
    Defines flags for the plugin.

    \sa pluginFlags
*/
/*!
    \var HbDeviceDialogPlugin::PluginFlag HbDeviceDialogPlugin::NoPluginFlags
    No flags specified.
*/
/*!
    \var HbDeviceDialogPlugin::PluginFlag HbDeviceDialogPlugin::PreloadPlugin
    Load plugin on startup. This also implies KeepPluginLoaded flag.
*/
/*!
    \var HbDeviceDialogPlugin::PluginFlag HbDeviceDialogPlugin::KeepPluginLoaded
    Keep the plugin loaded in memory. Flag is ignored for third party plugins.
*/

/*!
    \enum HbDeviceDialogPlugin::DeviceDialogGroup
    Defines device dialog groups. Each of these groups have different rules for showing
    device dialogs.

    \sa deviceDialogInfo DeviceDialogInfo
*/
/*!
    \var HbDeviceDialogPlugin::DeviceDialogGroup HbDeviceDialogPlugin::GenericDeviceDialogGroup
    Common device dialogs.
*/
/*!
    \var HbDeviceDialogPlugin::DeviceDialogGroup HbDeviceDialogPlugin::DeviceNotificationDialogGroup
    Device notification dialogs.
*/
/*!
    \var HbDeviceDialogPlugin::DeviceDialogGroup HbDeviceDialogPlugin::IndicatorGroup
    Indicators. Only platform device dialogs may belong to this group.
*/
/*!
    \var HbDeviceDialogPlugin::DeviceDialogGroup HbDeviceDialogPlugin::SecurityGroup
    Screen saver and alarm dialogs. Only platform device dialogs may belong to this group.
*/
/*!
    \var HbDeviceDialogPlugin::DeviceDialogGroup HbDeviceDialogPlugin::CriticalGroup
    High priority dialogs shown on top other dialogs. Only platform device dialogs may
    belong to this group.
*/

/*!
    \enum HbDeviceDialogPlugin::DeviceDialogFlag
    Defines flags for a device dialog created by the plugin.

    \sa deviceDialogInfo DeviceDialogInfo
*/
/*!
    \var HbDeviceDialogPlugin::DeviceDialogFlag HbDeviceDialogPlugin::NoDeviceDialogFlags
    No flags specified.
*/
/*!
    \var HbDeviceDialogPlugin::DeviceDialogFlag HbDeviceDialogPlugin::SharedDeviceDialog
    <em>For future needs. Not used.</em>
    The device dialog may be shared. Several clients may share the same device dialog instance.
    If the flag is not set, device dialog service doesn't allow sharing of the device dialog.

*/
/*!
    \var HbDeviceDialogPlugin::DeviceDialogFlag HbDeviceDialogPlugin::SecurityCheck
    If the flag is set, device dialog service calls accessAllowed() before creating or
    attaching to a device dialog widget.
*/
/*!
    \var HbDeviceDialogPlugin::DeviceDialogFlag HbDeviceDialogPlugin::SingleInstance
    If the flag is set, only one instance of the device dialog widget is allowed at
    one time. Attempt to launch the widget while one exists results in an error to be
    returned to the client.
*/


/*!
    \var HbDeviceDialogPlugin::DefaultPriority
    Default priority value to return in DeviceDialogInfo.
*/

/*!
    \struct HbDeviceDialogPlugin::DeviceDialogInfo
    \brief Structure containing device dialog widget information.

    Passed as a parameter to HbDeviceDialogPlugin::deviceDialogInfo().

    \sa HbDeviceDialogPlugin
*/
/*!
    \var HbDeviceDialogPlugin::DeviceDialogInfo::group
    Group the device dialog widget belongs to. HbDeviceDialogPlugin::DeviceDialogGroup.
*/
/*!
    \var HbDeviceDialogPlugin::DeviceDialogInfo::flags
    Device dialog flags. HbDeviceDialogPlugin::DeviceDialogFlag.
*/
/*!
    \var HbDeviceDialogPlugin::DeviceDialogInfo::priority
    <em>For future needs. Not used</em>.
    Device dialog display priority. Determines queuing order of dialogs. Fill with
    HbDeviceDialogPlugin::DefaultPriority.
*/
/*!
    \var HbDeviceDialogPlugin::DeviceDialogInfo::spare
    Spare space for future needs.
*/

/*!
    \fn virtual bool HbDeviceDialogPlugin::accessAllowed(const QString &deviceDialogType,
        const QVariantMap &parameters, const QVariantMap &securityInfo) const = 0

    Checks if client is allowed use the device dialog that the plugin creates. Device dialog service
    calls this function before creating a device dialog or attaching into an existing one if flag
    HbDeviceDialogPlugin::SecurityCheck is set. The implementation is operating system dependent.
    On Symbian this may involve checking client's platform security capabilities or secure ID for
    example. Returns true if client is allowed to use the device dialog.

     \param deviceDialogType Device dialog type.
     \param parameters Device dialog parameters.
     \param securityInfo Information for security check. Content is operating system dependent.
     The format is <key, value> pairs.
     Currently only Symbian is defined.
     <table>
         <caption> Symbian security information </caption>
         <tr>
         <th>Key</th>
         <th>Value type</th>
         <th>Description</th>
         </tr>
         <tr>
         <td>"sym-secureId"</td>
         <td>quint32</td>
         <td>Client's secure ID</td>
         </tr>
         <tr>
         <td>"sym-vendorId"</td>
         <td>quint32</td>
         <td>Client's vendor ID</td>
         </tr>
         <tr>
         <td>"sym-caps"</td>
         <td>quint32</td>
         <td>Client's capability set as a bitmap</td>
         </tr>
     </table>
*/

/*!
    \fn virtual bool HbDeviceDialogPlugin::deviceDialogInfo(const QString &deviceDialogType,
        const QVariantMap &parameters, DeviceDialogInfo *info) const = 0

    Gets information of the device dialog created by the plugin. Device dialog manager calls this
    function before creating the device dialog widget to check
    HbDeviceDialogPlugin::DeviceDialogGroup, HbDeviceDialogPlugin::DeviceDialogFlag and priority.
    Returns true if device dialog information returned is valid.

    \param deviceDialogType Device dialog type.
    \param parameters Device dialog parameters.
    \param info Structure the plugin fills with an information of the device dialog it
    creates with the deviceDialogType and parameters.
*/

/*!
    \fn virtual QStringList HbDeviceDialogPlugin::deviceDialogTypes() const = 0

    Returns a list of device dialog types the plugin implements. A plugin may implement several
    device dialog types. By convention device dialog type strings should follow inverted domain
    name format. For example com.nokia.hb.devicedialog.note/1.0
*/

/*!
    \fn virtual HbDeviceDialogPlugin::Flags HbDeviceDialogPlugin::pluginFlags() const = 0

    Returns plugin flags.
*/

/*!
    \fn virtual int HbDeviceDialogPlugin::error() const = 0

    Returns an error last occurred. Error code ranges are defined in HbDeviceDialog. The code is
    passed to a client by device dialog framework.
*/

/*************************************************************************************************/

/*!
    \class HbDeviceDialogInterface
    \brief HbDeviceDialogInterface is an abstract interface class for a device dialog widget.

    Device dialogs are widgets which are shown on top of all applications by a device dialog service.
    Widgets are created by plugins. Device dialog service uses this interface to update widget
    parameters and close widget.

    The widget implementation must implement this interface. In addition it must provide a
    deviceDialogClosed() signal and optionally deviceDialogData() signal if data needs to be
    passed to a client.

    Device dialog widgets must be derived from HbPopup or derivatives as described in
    HbDeviceDialogPlugin.

    Following popup convention, the widget is not visible immediately after is created.
    Device dialog framework calls HbPopup::show() to display the widget. The service may change
    The service may change visibility of the widget by HbPopup::setVisible function.

    The widget should close itself by either a timeout or user interaction. It may also be closed
    gracefully either by system or the client by a closeDeviceDialog() function. It is assumed that
    the widget object can be ungracefully destroyed (delete) at any time.

    \b Signals:

    \b void \b deviceDialogClosed()

    Device dialog widget must implement this signal. The widget should emit this signal
    when it has disappeared from a display and can be deleted. It is important this signal is
    emitted as device dialog service relies on it to detect the widget closing.

    \b void \b deviceDialogData(QVariantMap data)

    This signal is emitted by a device dialog widget to send data to a client. The signal
    is optional. If the widget does not provide any data, the signal is not needed. \a data
    contains data from the widget. The data is passed unmodified to the client. The structure
    and meaning of the data is a contract between the widget and a client.
    \endcode

    \sa HbDeviceDialogPlugin HbDeviceDialog

    \stable
    \hbcore
*/

/*!
    \fn virtual bool HbDeviceDialogInterface::setDeviceDialogParameters(
        const QVariantMap &parameters) = 0

    Set device dialog widget parameters. This function may be called after the widget is created
    to update widget parameters. \a parameters contains widget parameters. The structure and
    meaning of parameters is a contract between the plugin and a client. Returns true on success
    or false on failure.
*/

/*!
    \fn virtual int HbDeviceDialogInterface::deviceDialogError() const = 0

    Returns an error last occurred. Error code ranges are defined in HbDeviceDialog. The code is
    passed to a client by device dialog framework.

    \sa HbDeviceDialog::DeviceDialogError
*/

/*!
    \fn virtual void HbDeviceDialogInterface::closeDeviceDialog(bool byClient) = 0

    Closes a device dialog widget gracefully. This function may called due to device dialog
    client calling HbDeviceDialog::cancel() or by device dialog service if it decides
    that a dialog needs to be closed. Device dialog widget may also close itself without
    this function being called. \a byClient is true if client initiated the closing.
    Otherwise the closing was initiated by the device dialog framework. The widget may also be
    deleted at any time ungracefully without closeDeviceDialog() being called.

    \sa deviceDialogClosed()
*/

/*!
    \fn virtual HbPopup *HbDeviceDialogInterface::deviceDialogWidget() const = 0

    Returns a pointer to a widget that implements the user interface of the device dialog. The
    widget must be derived from a HbPopup or it's derivatives.

    \sa HbDeviceDialogPlugin
*/

/*!
    \fn virtual QObject *HbDeviceDialogInterface::signalSender() const

    Returns a pointer to an object that is a sender of deviceDialogClosed() and deviceDialogData()
    signals. There is a default implementation which returns null. In this case it is assumed that
    the device dialog widget is the signal source. This function may be overriden for example in
    a case where the dialog widget is encapsulated by a container class which sends the required
    signals. Note that there has to be one to one relationship between the widget and the sender.
    I.e. there is a one sender object per widget.

    \sa deviceDialogWidget deviceDialogClosed() deviceDialogData()
*/

/*************************************************************************************************/

/*!
    \class HbDeviceDialogPluginInterface
    \brief HbDeviceDialogPluginInterface is an abstract interface that identifies a device dialog plugin.

    This class is not intended for direct derivation. Instead a device dialog plugin should be derived
    from HbDeviceDialogPlugin. This class declares createDeviceDialog() function which the plugin must
    implement.

    \stable
    \hbcore
*/

/*!
    \fn virtual HbDeviceDialogPluginInterface *HbDeviceDialogPluginInterface::createDeviceDialog(
        const QString &deviceDialogType, const QVariantMap &parameters) = 0

    Creates a device dialog widget. The widget becomes visible immediately. Returns a pointer to
    the device dialog widget interface or null if widget cannot be created.

    \param deviceDialogType Device dialog type to create.
    \param parameters Device dialog parameters. The structure and meaning of parameters is a
    contract between the plugin and a client.

    \sa HbDeviceDialogPlugin
*/

/*************************************************************************************************/

#include <hbdevicedialogplugin.h>
#include <hbdevicedialoginterface.h>
#ifdef Q_OS_SYMBIAN
#include <e32base.h>
#endif // Q_OS_SYMBIAN

// Constructor
HbDeviceDialogPlugin::HbDeviceDialogPlugin()
{
#ifdef Q_OS_SYMBIAN
    // Check loading process secure ID. Panic if not device dialog server.
    const TUint32 sid = 0x20022FC5;
    RProcess process;
    if (process.SecureId().iId != sid) {
        _LIT(KCategory, "HbDeviceDialogPlugin");
        User::Panic(KCategory, 1);
    }
#endif // Q_OS_SYMBIAN
}

// Destructor
HbDeviceDialogPlugin::~HbDeviceDialogPlugin()
{
}

HbDeviceDialogInterface::~HbDeviceDialogInterface()
{
}

QObject *HbDeviceDialogInterface::signalSender() const
{
    return 0;
}
