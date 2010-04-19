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
#ifndef HBDEVICEDIALOGERRORS_P_H
#define HBDEVICEDIALOGERRORS_P_H

#include <hbdevicedialog.h>

// No error
const int HbDeviceDialogNoError = 0;
// Illegal parameter error
const int HbDeviceDialogParameterError = HbDeviceDialog::FrameworkErrors + 1;
// General error
const int HbDeviceDialogGeneralError = HbDeviceDialog::FrameworkErrors + 2;
// Device dialog is not found. Plugin missing or closed.
const int HbDeviceDialogNotFoundError = HbDeviceDialog::FrameworkErrors + 3;
// Illegal operation error
const int HbDeviceDialogIllegalOperationError = HbDeviceDialog::FrameworkErrors + 4;
// Connection error
const int HbDeviceDialogConnectError = HbDeviceDialog::FrameworkErrors + 5;
// Plugin denies client access of a plugin
const int HbDeviceDialogAccessDeniedError = HbDeviceDialog::FrameworkErrors + 6;
//unknown indicator error
const int HbDeviceDialogUnknownIndicatorError = HbDeviceDialog::FrameworkErrors + 7;

#endif // HBDEVICEDIALOGERRORS_P_H
