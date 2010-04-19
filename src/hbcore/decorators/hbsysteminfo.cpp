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

#include "hbsysteminfo_p.h"

#if defined(Q_OS_SYMBIAN)
#include "hbsysteminfo_sym_p_p.h"
#elif defined(Q_OS_WIN)
#include "hbsysteminfo_win_p_p.h"
#elif defined(Q_OS_UNIX)
#include "hbsysteminfo_linux_p_p.h"
#endif

#include <QStringList>
#include <QSize>
#include <QFile>
#include <QTextStream>
#include <QLocale>
#include <QLibraryInfo>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

#include <locale.h>

  /*
    \class HbSystemNetworkInfo

    \ingroup systeminfo

    \brief The HbSystemNetworkInfo class provides access to network information from the system.

  */
/*
    \enum HbSystemNetworkInfo::NetworkStatus
    This enum describes the status of the network connection:

    \value UndefinedStatus        There is no network device, or error.
    \value NoNetworkAvailable     There is no network available.
    \value EmergencyOnly          Emergency calls only.
    \value Searching              Searching for or connecting with the network.
    \value Busy                   Network is busy.
    \value Connected              Connected to newtwork.
    \value HomeNetwork            On Home Network.
    \value Denied                 Network access denied.
    \value Roaming                On Roaming network.

  */
/*
    \enum HbSystemNetworkInfo::NetworkMode
    This enum describes the type of network:

    \value UnknownMode             Unknown network.or netowrk error.
    \value GsmMode                 Global System for Mobile (GSM) network.
    \value CdmaMode                Code division multiple access (CDMA) network.
    \value WcdmaMode               Wideband Code Division Multiple Access (W-CDMA) network.
    \value WlanMode                Wireless Local Area Network (WLAN) network.
    \value EthernetMode            Wired Local Area network.
    \value BluetoothMode           Bluetooth network.
    \value WimaxMode               Wimax network.

  */

/*
    \class QSystemDisplayInfo

    \ingroup systeminfo

    \brief The QSystemDisplayInfo class provides access to display information from the system.

  */

  /*
    \class QSystemStorageInfo

    \ingroup systeminfo

    \brief The QSystemStorageInfo class provides access to memory and disk information from the system.

  */

/*
    \enum QSystemStorageInfo::DriveType
    This enum describes the type of drive or volume

    \value NoDrive               Drive type undetermined.
    \value InternalDrive         Is internal drive.
    \value RemovableDrive        Is removable.
    \value RemoteDrive           Is a network drive.
    \value CdromDrive            Is a cd rom drive.
*/


/*
    \class HbSystemDeviceInfo

    \ingroup systeminfo
    
    \brief The HbSystemDeviceInfo class provides access to device
information from the system.

  */
/*
  \fn void HbSystemDeviceInfo::batteryLevelChanged(int level)

  This signal is emitted when battery level has changed.
  \a level is the new level.
 */

/*
  \fn void HbSystemDeviceInfo::batteryStatusChanged(HbSystemDeviceInfo::BatteryStatus status)

  This signal is emitted when battery status has changed.
  \a status is the new status.
 */

   /*
  \fn void HbSystemDeviceInfo::powerStateChanged(HbSystemDeviceInfo::PowerState state)

  This signal is emitted when the power state has changed, such as when a phone gets plugged qint32o the wall.
  \a state is the new power state.
 */

/*
  \fn  void HbSystemDeviceInfo::currentProfileChanged(HbSystemDeviceInfo::Profile profile)

  This signal is emitted whenever the network profile changes, specified by \a profile.
*/


/*
    \enum HbSystemDeviceInfo::BatteryStatus
    This enum describes the status of the main battery.

    \value NoBatteryLevel          Battery level undetermined.
    \value BatteryCritical         Battery level is critical 3% or less.
    \value BatteryVeryLow          Battery level is very low, 10% or less.
    \value BatteryLow              Battery level is low 40% or less.
    \value BatteryNormal           Battery level is above 40%.

  */
/*
    \enum HbSystemDeviceInfo::PowerState
    This enum describes the power state:

    \value UnknownPower                   Power error.
    \value BatteryPower                   On battery power.
    \value WallPower                      On wall power.
    \value WallPowerChargingBattery       On wall power and charging main battery.

  */
/*
    \enum HbSystemDeviceInfo::Profile
    This enum describes the current operating profile of the device or computer.

    \value UnknownProfile          Profile unknown or error.
    \value SilentProfile           Silent profile.
    \value NormalProfile           Normal profile.
    \value LoudProfile             Loud profile.
    \value VibProfile              Vibrate profile.
    \value OfflineProfile          Offline profile.
    \value PowersaveProfile        Powersave profile.
    \value CustomProfile           Custom profile.

  */

/*
    \enum HbSystemDeviceInfo::SimStatus
    This enum describes the status is the sim card or cards.

    \value SimNotAvailable         SIM is not available on this device.
    \value SingleSimAvailable         One SIM card is available on this.
    \value DualSimAvailable           Two SIM cards are available on this device.
    \value SimLocked                  Device has SIM lock enabled.
*/

/*
    \enum HbSystemDeviceInfo::InputMethod
    This enum describes the device method of user input.

    \value Keys               Device has key/buttons.
    \value Keypad             Device has keypad (1,2,3, etc).
    \value Keyboard           Device has qwerty keyboard.
    \value SingleTouch        Device has single touch screen.
    \value MultiTouch         Device has muti touch screen.
    \value Mouse              Device has a mouse.
*/

/*
    \class QSystemScreenSaver

    \ingroup systeminfo

    \brief The QSystemScreenSaver class provides access to screen saver and blanking.

  */

/*
  \fn void QSystemInfo::currentLanguageChanged(const QString &lang)

  This signal is emitted whenever the current language changes, specified by \a lang,
  which is in 2 letter, ISO 639-1 specification form.
  */

/*
  \fn void HbSystemNetworkInfo::networkStatusChanged(HbSystemNetworkInfo::NetworkMode mode, HbSystemNetworkInfo::NetworkStatus status)

  This signal is emitted whenever the network status of \a mode changes, specified by \a status.
  */

/*
  \fn void HbSystemNetworkInfo::networkSignalStrengthChanged(HbSystemNetworkInfo::NetworkMode mode,int strength)

  This signal is emitted whenever the network \a mode signal strength changes, specified by \a strength.
  */

/*
  \fn void HbSystemNetworkInfo::currentMobileCountryCodeChanged(const QString &mcc)

  This signal is emitted whenever the Mobile Country Code changes, specified by \a mcc.
*/

/*
  \fn void HbSystemNetworkInfo::currentMobileNetworkCodeChanged(const QString &mnc)

  This signal is emitted whenever the network Mobile Network Code changes, specified by \a mnc.
*/

/*
  \fn void HbSystemNetworkInfo::networkNameChanged(HbSystemNetworkInfo::NetworkMode mode,const QString & netName)

  This signal is emitted whenever the network \a mode name changes, specified by \a netName.

*/

/*
  \fn void HbSystemNetworkInfo::networkModeChanged(HbSystemNetworkInfo::NetworkMode mode)

  This signal is emitted whenever the network mode changes, specified by \a mode.
*/

/*
  \fn void HbSystemDeviceInfo::bluetoothStateChanged(bool on)

  This signal is emitted whenever bluetooth state changes, specified by \a on.
*/

 /*
   Constructs a HbSystemNetworkInfo object with the given \a parent.
 */
HbSystemNetworkInfo::HbSystemNetworkInfo(QObject *parent)
{
    d = new HbSystemNetworkInfoPrivate(parent);
    connect(d,SIGNAL(currentMobileCountryCodeChanged(QString)),
            this,SIGNAL(currentMobileCountryCodeChanged(QString)));

    connect(d,SIGNAL(currentMobileNetworkCodeChanged(QString)),
            this,SIGNAL(currentMobileNetworkCodeChanged(QString)));

    connect(d,SIGNAL(networkModeChanged(HbSystemNetworkInfo::NetworkMode)),
            this,SIGNAL(networkModeChanged(HbSystemNetworkInfo::NetworkMode)));

    connect(d,SIGNAL(networkNameChanged(HbSystemNetworkInfo::NetworkMode,QString)),
            this,SIGNAL(networkNameChanged(HbSystemNetworkInfo::NetworkMode,QString)));

    connect(d,SIGNAL(networkSignalStrengthChanged(HbSystemNetworkInfo::NetworkMode,int)),
            this,SIGNAL(networkSignalStrengthChanged(HbSystemNetworkInfo::NetworkMode,int)));

    connect(d,SIGNAL(networkStatusChanged(HbSystemNetworkInfo::NetworkMode,HbSystemNetworkInfo::NetworkStatus)),
            this,SIGNAL(networkStatusChanged(HbSystemNetworkInfo::NetworkMode,HbSystemNetworkInfo::NetworkStatus)));
}

/*
  Destroys the HbSystemNetworkInfo object.
 */
HbSystemNetworkInfo::~HbSystemNetworkInfo()
{
    delete d;
}

/*
    Returns the status of the network \a mode.
*/
HbSystemNetworkInfo::NetworkStatus HbSystemNetworkInfo::networkStatus(HbSystemNetworkInfo::NetworkMode mode) const
{
    return d->networkStatus(mode);
}

/*
    Returns the strength of the network signal, per network \a mode , 0 - 100 linear scaling,
    or -1 in the case of unknown network mode or error.

    In the case of HbSystemNetworkInfo::EthMode, it will either be 100 for carrier active, or 0 for when
    there is no carrier or cable connected.
*/
int HbSystemNetworkInfo::networkSignalStrength(HbSystemNetworkInfo::NetworkMode mode) const
{
	return d->networkSignalStrength(mode);
}

/*
    Returns the Cell ID of the connected tower or based station.
*/
int HbSystemNetworkInfo::cellId() const
{
    return d->cellId();
}

/*
    Returns the Location Area Code. In the case of none such as a Desktop, "No Mobile Network."
*/
int HbSystemNetworkInfo::locationAreaCode() const
{
    return d->locationAreaCode();
}

 /*
    Returns the current Mobile Country Code. In the case of none such as a Desktop, "No Mobile Network."
*/
QString HbSystemNetworkInfo::currentMobileCountryCode() const
{
    return d->currentMobileCountryCode();
}

/*
    Returns the current Mobile Network Code. In the case of none such as a Desktop, "No Mobile Network."
*/
QString HbSystemNetworkInfo::currentMobileNetworkCode() const
{
    return d->currentMobileNetworkCode();
}

/*
    Returns the home Mobile Network Code. In the case of none such as a Desktop, "No Mobile Network."
*/
QString HbSystemNetworkInfo::homeMobileCountryCode() const
{
    return d->homeMobileCountryCode();
}

/*
    Returns the home Mobile Country Code. In the case of none such as a Desktop, "No Mobile Network."
*/
QString HbSystemNetworkInfo::homeMobileNetworkCode() const
{
    return d->homeMobileNetworkCode();
}

/*
  Returns the name of the operator for the network \a mode.  For wlan this returns the network's current SSID.
In the case of no network such as a desktop, "No Operator".
*/
QString HbSystemNetworkInfo::networkName(HbSystemNetworkInfo::NetworkMode mode) const
{
    return d->networkName(mode);
}

/*
  Returns the MAC address for the interface servicing the network \a mode.
  */
QString HbSystemNetworkInfo::macAddress(HbSystemNetworkInfo::NetworkMode mode) const
{
    return d->macAddress(mode);
}

/*
  Returns the first found QNetworkInterface for type \a mode.
  */
QNetworkInterface HbSystemNetworkInfo::interfaceForMode(HbSystemNetworkInfo::NetworkMode mode) const
{
    return d->interfaceForMode(mode);
}

// device
 /*
   Constructs a HbSystemDeviceInfo with the given \a parent.
 */
HbSystemDeviceInfo::HbSystemDeviceInfo(QObject *parent)
{
    d = new HbSystemDeviceInfoPrivate(parent);
    connect(d,SIGNAL(batteryLevelChanged(int)),
            this,SIGNAL(batteryLevelChanged(int)));

    connect(d,SIGNAL(batteryStatusChanged(HbSystemDeviceInfo::BatteryStatus)),
            this,SIGNAL(batteryStatusChanged(HbSystemDeviceInfo::BatteryStatus)));

    connect(d,SIGNAL(bluetoothStateChanged(bool)),
            this,SIGNAL(bluetoothStateChanged(bool)));

    connect(d,SIGNAL(currentProfileChanged(HbSystemDeviceInfo::Profile)),
            this,SIGNAL(currentProfileChanged(HbSystemDeviceInfo::Profile)));

    connect(d,SIGNAL(powerStateChanged(HbSystemDeviceInfo::PowerState)),
            this,SIGNAL(powerStateChanged(HbSystemDeviceInfo::PowerState)));
    }

/*
  Destroys the HbSystemDeviceInfo object.
 */
HbSystemDeviceInfo::~HbSystemDeviceInfo()
{
    delete d;
}


/*
    Returns the HbSystemDeviceInfo::InputMethodFlags InputMethodType that the system uses.
*/
HbSystemDeviceInfo::InputMethodFlags HbSystemDeviceInfo::inputMethodType() const
{
	return d->inputMethodType();
}
/*
    Returns the International Mobile Equipment Identity (IMEI), or a null QString in the case of none.
*/
QString HbSystemDeviceInfo::imei() const
{
    return d->imei();
}

/*
    Returns the International Mobile Subscriber Identity (IMSI), or a null QString in the case of none.
*/
QString HbSystemDeviceInfo::imsi() const
{
	return d->imsi();
}

/*
    Returns the name of the manufacturer of this device. In the case of desktops, the name of the vendor
    of the motherboard.
*/
QString HbSystemDeviceInfo::manufacturer() const
{
    return d->manufacturer();
}

/*
    Returns the model information of the device. In the case of desktops where no
    model information is present, the CPU architect, such as i686, and machine type, such as Server,
    Desktop or Laptop.
*/
QString HbSystemDeviceInfo::model() const
{
	return d->model();
}

/*
    Returns the product name of the device. In the case where no product information is available,

*/
QString HbSystemDeviceInfo::productName() const
{
    return d->productName();
}
/*
    Returns the battery charge level as percentage 1 - 100 scale.
*/
int HbSystemDeviceInfo::batteryLevel() const
{
    return d->batteryLevel();
}

  /*
    Returns the battery charge status.
*/
HbSystemDeviceInfo::BatteryStatus HbSystemDeviceInfo::batteryStatus() const
{
    return d->batteryStatus();
}

/*
  \property HbSystemDeviceInfo::simStatus
  \brief the status of the sim card.
  Returns the HbSystemDeviceInfo::simStatus status of SIM card.
*/
HbSystemDeviceInfo::SimStatus HbSystemDeviceInfo::simStatus() const
{
    return d->simStatus();
}
/*
  Returns true if the device is locked, otherwise false.
*/
bool HbSystemDeviceInfo::isDeviceLocked() const
{
    return d->isDeviceLocked();
}

/*
  \property HbSystemDeviceInfo::currentProfile
  \brief the device profile
  Gets the current HbSystemDeviceInfo::currentProfile device profile.
*/
HbSystemDeviceInfo::Profile HbSystemDeviceInfo::currentProfile() const
{
    return d->currentProfile();
}

/*
  \property HbSystemDeviceInfo::currentPowerState
  \brief the power state.

  Gets the current HbSystemDeviceInfo::currentPowerState state.
*/
HbSystemDeviceInfo::PowerState HbSystemDeviceInfo::currentPowerState() const
{
    return d->currentPowerState();
}
