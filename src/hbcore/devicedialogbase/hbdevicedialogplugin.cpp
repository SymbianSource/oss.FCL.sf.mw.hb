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

    \section _creation Creation of device dialogs

    Device dialog widgets are created by plugins. The service loads a plugin to create and display
    a device dialog. A single plugin may implement several different dialogs or just one.

    \subsection _identification Identification

    Dialogs are identified by a string. By convention the string should follow
    inverted domain name format. For example com.nokia.hb.devicedialog.note/1.0.
    Function deviceDialogTypes() returns a list of dialog types the plugin implements.

    \subsection _versioning Versioning

    Appending version number into device dialog type string enables versioning of device dialogs.
    A plugin should handle versioning by returning device dialog type string for all versions it
    implements in deviceDialogTypes() and then in createDeviceDialog() create a device dialog
    instance compatible with the version requested. This could always be the latest version if it
    is backwards compatible with older versions. Device dialog framework is unaware of version
    numbers in type strings. It performs string comparison of the whole string when searching
    for a plugin.

    \subsection _widgets Dialog widgets

    Device dialog widgets have to be derived from HbPopup either directly or by ancestry.

    Device dialog widget can be derived from existing Hb class, eg. HbDialog or constructed from
    a docml using HbDocumentLoader. Structure of the plugin differs depending on which approach
    is selected. If docml is used the dialog widget cannot be multiple inherited from HbDialog
    and HbDeviceDialogInterface. In this case a container can be used to to create and manage the
    widget. See code example at the end.

    \section _types Types of device dialogs

    Device dialogs are divided into four groups:

    - Generic
    - Notification
    - Indicator-menu
    - Security

    The group dictates how and when a dialog is shown. Generic and notification dialogs are
    intended for application use. Indicator-menu and security dialogs are for platform.
    Each dialog indicates which group it belongs to. The group need not be fixed.
    It may change depending on create parameters.

    \subsection _priority Priority

    Device dialog groups prioritize device dialogs (z-order) in respect to other groups.
    Dialogs belonging to the same group are displayed in order requests are received.
    Popup framework is utilized to manage z-order of dialogs. Z-order is set by devicedialog
    framework and device dialogs themselves should not change the popup z-order.

    \section _security Security

    Plugins are responsible for maintaining system security for their own part. If plugin
    performs operations that may compromise security or want's to limit access to specific
    clients, it should check client security credentials in accessAllowed() function.
    Device dialog framework calls this function before creating or attaching client to a
    plugin if HbDeviceDialogPlugin::SecurityCheck is set. Access is denied if the
    function returns false. In addition, HbDeviceDialogPlugin constructor has a check which
    allows only device dialog service to load plugins derived from it.

    \section _exceptions Exception handling

    Device dialog service uses two strategies in exception handling: Avoidance and trapping.
    Memory allocation exceptions while a dialog is running are avoided by ensuring there is
    sufficient heap space available before allowing new dialogs. Trapping is used while
    a dialog is created. A call to createDeviceDialog() is enclosed in try/catch block.
    Memory allocation exception causes dialog creation to fail and an error is returned
    to a client. Plugin should take care there are no memory leaks if exception is thrown
    inside createDeviceDialog(). All calls to HbDeviceDialogInterface functions are trapped
    and thrown allocation exceptions are ignored. Plugins can provide more fine grained
    exception handling by trapping exceptions themselves. 

    \section _checklist Checklist for well behaving device dialogs

    Well behaving plugin should observe following rules:
    - Delay plugin initializations until HbDeviceDialogPluginInterface::createDeviceDialog() is
    called. Plugin may be loaded/unloaded several times by different threads during plugin search. 
    The search is slowed down by unnessary initializations by plugins.

    Well behaving dialog widget should observe following rules:
    - Should not create and show other dialogs
    - No calls to show(), hide() or setVisible()
    - Not setting popup Z-order
    - No time consuming operations performed

    \section _platform_hbdevicedialogplugin Platform-specific implementation notes for HbDeviceDialogPlugin

    \subsection _nonsymbian Non-Symbian

    The server is implemented only for the Symbian platform. On other platforms, plugins are loaded into client
    process and dialogs shown on client's main window.

    Plugin executables are searched from application's current directory and HB_PLUGINS_DIR/devicedialogs
    directory.

    \subsection _symbian Symbian

    Plugins are run by a server with platform security capabilities ProtServ, SwEvent,
    TrustedUI and ReadDeviceData. If a plugin doesn't have required platform security
    capabilities it will not load.

    Device dialog plugins can be installed into a device. Plugins are searched from device's
    local drives. The search order is rom-drives, non-removable drives and removable drives.
    Search order inside each group is alphabethical. Eclipsing is allowed only if plugin file
    names are the same. This means that in order to replace a device dialog in a plugin earlier
    in search order, the file name of the replacement plugin has to be the same as the the one
    it replaces.

    Device dialog plugin stubs (.qtplugin) are searched from /resource/plugins/devicedialogs directory
    and executables in /sys/bin directory in each drive.

    \section _creation Creating a plugin

    Creating a device dialog plugin and widget involves following steps.
    - Set in .pro file TEMPLATE = lib and CONFIG += hb plugin
    - Derive a class from HbPopup or derivatives and HbDeviceDialogInterface
    The example below is <b>not using docml</b> and dialog widget is derived from HbDialog and
    HbDeviceDialogInterface.
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

    The example below is <b>using docml</b> and dialog widget is managed by a container.
    \code
    class HbSampleDialogContainer : public QObject, public HbDeviceDialogInterface
    {
        Q_OBJECT
    \endcode
    - In the constructor create the dialog widget using HbDocumentLoader
    - Implement HbDeviceDialogInterface functions
      - From deviceDialogWidget() function return the created (docml) widget
      - From signalSender() function return the container (HbSampleDialogContainer)
    - Declare and emit deviceDialogClosed and optionally deviceDialogData signals.
    - Do not call show(), hide() or setVisible() in the plugin. Device dialog
      framework calls show() to display the widget.
    \code
    public:
        bool setDeviceDialogParameters(const QVariantMap &parameters);
        int deviceDialogError() const;
        void closeDeviceDialog(bool byClient);
        HbPopup *deviceDialogWidget() const;
        QObject *signalSender() const;
    signals:
        void deviceDialogClosed();
        void deviceDialogData(QVariantMap data);
    \endcode
    - Wrap the widget into a plugin derived from HbDeviceDialogPlugin
    \code
    class HbSampleDeviceDialogPlugin : public HbDeviceDialogPlugin
    {
        Q_OBJECT
    \endcode
    - Implement HbDeviceDialogPlugin pure virtual functions
      - Return the container (HbSampleDialogContainer) from createDeviceDialog() function
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

    <b> Class diagram of the sample plugin using docml:</b>
    \dot
    digraph G {
        rankdir=LR;

        subgraph cluster_class_diagram {
            style=solid;

            node [shape = box, style=solid, fontsize = 10];
            HbSampleDeviceDialogPlugin [label = "HbSampleDeviceDialogPlugin"];
            QObject [label = "QObject"];
            HbDeviceDialogPluginInterface [label = "HbDeviceDialogPluginInterface"];
            HbDeviceDialogPlugin [label = "HbDeviceDialogPlugin"];
            edge [fontsize = 10, style = filled];
            QObject -> HbDeviceDialogPlugin [label = "is a"];
            HbDeviceDialogPluginInterface -> HbDeviceDialogPlugin [label = "is a"];
            HbDeviceDialogPlugin -> HbSampleDeviceDialogPlugin [label = "is a"];

            HbSampleDialogContainer [label = "HbSampleDialogContainer"];
            QObject2 [label = "QObject"];
            HbDeviceDialogInterface [label = "HbDeviceDialogInterface"];
            QObject2 -> HbSampleDialogContainer [label = "is a"];
            HbDeviceDialogInterface -> HbSampleDialogContainer [label = "is a"];

            edge [fontsize = 10, style = dotted];
            HbSampleDeviceDialogPlugin -> HbSampleDialogContainer [label = "creates"];

            DialogWidget [label = "Dialog widget (docml)"];
            edge [fontsize = 10, style = dotted];
            HbSampleDialogContainer -> DialogWidget [label = "creates"];
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

    \sa deviceDialogInfo() DeviceDialogInfo
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
    Reserved for platform use, indicator menu. Not to be used by any other dialog.
*/
/*!
    \var HbDeviceDialogPlugin::DeviceDialogGroup HbDeviceDialogPlugin::SecurityGroup
    Reserved for platform use, screen saver. Not to be used by any other dialog.
*/
/*!
    \var HbDeviceDialogPlugin::DeviceDialogGroup HbDeviceDialogPlugin::CriticalGroup
    High priority dialogs shown on top other dialogs. Only platform device dialogs may
    belong to this group.

    \deprecated HbDeviceDialogPlugin::CriticalGroup
    is deprecated. Instead use HbDeviceDialogPlugin::GenericDeviceDialogGroup and set
    HbDeviceDialogPlugin::CriticalLayer. 
*/

/*!
    \enum HbDeviceDialogPlugin::ShowLevel
    Defines a level (z-value) a device dialog is shown.

    \sa deviceDialogInfo() DeviceDialogInfo
*/
/*!
    \var HbDeviceDialogPlugin::ShowLevel HbDeviceDialogPlugin::NormalLevel
    A default value to be used by applications.
*/
/*!
    \var HbDeviceDialogPlugin::ShowLevel HbDeviceDialogPlugin::SecurityLevel
    Reserved for platform use, screen saver. Not to be used by other dialogs.
    Dialog is shown on top of normal level dialogs.
*/
/*!
    \var HbDeviceDialogPlugin::ShowLevel HbDeviceDialogPlugin::CriticalLevel
    Reserved for platform use. Dialog is shown on top of security level dialogs.
*/

/*!
    \enum HbDeviceDialogPlugin::DeviceDialogFlag
    Defines flags for a device dialog created by the plugin.

    \sa deviceDialogInfo() HbIndicatorInterface
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
    \var HbDeviceDialogPlugin::DeviceDialogFlag HbDeviceDialogPlugin::ReceiveIndicatorStatus
    Indicates that the device dialog is interested in indicator activation/deactivation events.
    To receive indicator status the dialog has to implement two slots. Indicator activation is
    received by a slot indicatorActivated(HbIndicatorInterface*) and deactivation by a slot
    indicatorDeactivated(HbIndicatorInterface*).
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
    \var HbDeviceDialogPlugin::DeviceDialogInfo::showLevel
    Level (z-value) the device dialog widget is shown. HbDeviceDialogPlugin::ShowLevel.
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
         <tr><th>Key</th><th>Value type</th><th>Description</th></tr>
         <tr><td>"sym-secureId"</td><td>quint32</td><td>Client's secure ID</td></tr>
         <tr><td>"sym-vendorId"</td><td>quint32</td><td>Client's vendor ID</td></tr>
         <tr><td>"sym-caps"</td><td>quint32</td>
         <td>Client's capability set as a bitmap. Bit positions correspond to Symbian enum TCapability</td>
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
    Device dialog framework calls HbPopup::show() to display the widget.
    The service may change visibility of the widget by HbPopup::setVisible() function.

    The widget should close itself by either a timeout or user interaction. It may also be closed
    gracefully either by system or the client by a closeDeviceDialog() function. It is assumed that
    the widget object can be ungracefully destroyed (delete) at any time.

    \section _checklist Checklist for well behaving device dialog widgets

    - Should not create and show other dialogs
    - No calls to show(), hide() or setVisible()
    - Not setting popup Z-order
    - No time consuming operations performed

    \section _implementation_tips Implementation tips

    In order to allow possible disappear effect to complete, deviceDialogClosed() can be emitted
    from hideEvent(). In order not to signal close if the widget is only hidden but not closed,
    following pattern can be followed:
    - Reimplement QGraphicsWidget::hideEvent() and QGraphicsWidget::closeEvent() functions
    - In closeEvent(). Set a flag that event has been received. If isVisible() returns false, emit deviceDialogClosed().
    - In hideEvent(). If close event has has been received (flag set), emit deviceDialogClosed().
    - Don't forget to call base class closeEvent() and hideEvent().

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
    the device dialog widget is the signal source. This function may be overridden for example in
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

    Called by device dialog framework to create a device dialog widget. The widget should be
    left unvisible. The framework calls show() at a suitable time to make it visible.
    Returns a pointer to the device dialog widget interface or null if widget cannot be created.

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

/*!
    Constructs HbDeviceDialogPlugin.
*/
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

/*!
    Destructs HbDeviceDialogPlugin.
*/
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
