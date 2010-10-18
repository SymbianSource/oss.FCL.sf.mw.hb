/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
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

#include "hbdevicedialogsextensionsymbian_r.h"
#include "hbdevicemessageboxsymbian.h"
#include "hbdeviceprogressdialogsymbian.h"
#include "hbdevicenotificationdialogsymbian.h"

class CHbDeviceMessageBoxPrivate;
class CHbDeviceProgressDialogSymbianPrivate;
class CHbDeviceNotificationDialogSymbianPrivate;
void SetShowLevel(CHbDeviceMessageBoxPrivate *aBox, TInt aLevel);
void SetShowLevel(CHbDeviceProgressDialogSymbianPrivate *aDialog, TInt aLevel);
void SetShowLevel(CHbDeviceNotificationDialogSymbianPrivate *aDialog, TInt aLevel);

#define ARG_UNUSED(x) (void)x;

/*!
    \class HbDeviceDialogsExtensionSymbian
    \brief HbDeviceDialogsExtensionSymbian extends property set of stock device dialogs.

    HbDeviceDialogsExtensionSymbian is a restricted API. It is intended for platform applications
    to set rarely needed properties of CHbDeviceMessageBoxSymbian, CHbDeviceProgressDialogSymbian
    and CHbDeviceNotificationDialogSymbian.

    Use of API functions maybe limited for applications having certain platform security
    capabilities.

    \sa CHbDeviceMessageBoxSymbian, CHbDeviceProgressDialogSymbian, CHbDeviceNotificationDialogSymbian

    @alpha
    @hbwidgets
*/

/*!
    Sets a device message box to show on top of normal device-dialogs.

    \param aDialog Device message box.
    \param aLevel Show level. Values are defined in HbDeviceDialogPlugin::ShowLevel.
    Symbian capability swEvent is required to set level higher than
    HbDeviceDialogPlugin::NormalLevel.

    \code
    // Show a message box on top of device lock
    const TInt KCriticalLevel = 2;
    CHbDeviceMessageBoxSymbian *box =
        CHbDeviceMessageBoxSymbian::NewL(CHbDeviceMessageBoxSymbian::EQuestion);
    HbDeviceDialogsExtensionSymbian::SetShowLevel(box, KCriticalLevel);
    \endcode

    \sa HbDeviceDialogPlugin::ShowLevel
*/
EXPORT_C void HbDeviceDialogsExtensionSymbian::SetShowLevel(CHbDeviceMessageBoxSymbian *aDialog,
    TInt aLevel)
{
    ::SetShowLevel(aDialog->d, aLevel);
}

/*!
    Sets a device progress dialog to show on top of normal device-dialogs.

    \param aDialog Device progress dialog.
    \param aLevel Show level. Values are defined in HbDeviceDialogPlugin::ShowLevel.
    Symbian capability swEvent is required to set level higher than
    HbDeviceDialogPlugin::NormalLevel.

    \code
    // Show a progress dialog on top of device lock
    const TInt KCriticalLevel = 2;
    CHbDeviceProgressDialogSymbian *dialog = CHbDeviceProgressDialogSymbian::NewL();
    HbDeviceDialogsExtensionSymbian::SetShowLevel(dialog, KCriticalLevel);
    \endcode

    \sa HbDeviceDialogPlugin::ShowLevel
*/
EXPORT_C void HbDeviceDialogsExtensionSymbian::SetShowLevel(CHbDeviceProgressDialogSymbian *aDialog,
    TInt aLevel)
{
    ::SetShowLevel(aDialog->d, aLevel);
}

/*!
    Sets a device notification dialog to show on top of normal device-dialogs.

    \param aDialog Device notification dialog.
    \param aLevel Show level. Values are defined in HbDeviceDialogPlugin::ShowLevel.
    Symbian capability swEvent is required to set level higher than
    HbDeviceDialogPlugin::NormalLevel.

    \code
    // Show a notification dialog on top of device lock
    const TInt KCriticalLevel = 2;
    CHbDeviceNotificationDialogSymbian *dialog = CHbDeviceNotificationDialogSymbian::NewL();
    HbDeviceDialogsExtensionSymbian::SetShowLevel(dialog, KCriticalLevel);
    \endcode

    \sa HbDeviceDialogPlugin::ShowLevel
*/
EXPORT_C void HbDeviceDialogsExtensionSymbian::SetShowLevel(CHbDeviceNotificationDialogSymbian *aDialog,
    TInt aLevel)
{
    ::SetShowLevel(aDialog->d, aLevel);
}
