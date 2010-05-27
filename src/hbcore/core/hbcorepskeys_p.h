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

#ifndef HB_COREPSKEYS_P_H
#define HB_COREPSKEYS_P_H

#ifdef Q_OS_SYMBIAN

#include <e32cmn.h>

/**
 * HbCore internal orientation PS category UID.
 * Intended only for orientation setting.
 */
const TUid KHbPsOrientationCategoryUid = {0x20022E82}; // Theme server UID

/**
 * HbCore internal foreground application orientation PS category UID.
 */
const TUid KHbPsForegroundAppOrientationCategoryUid = {0x20022FC5}; //device dialog UID

/**
 * KHbPsOrientationKey
 * Current orientation value recieved from sensor.
 * Qt::Orientation
 */
const TUint KHbPsOrientationKey = 'Orie';

/**
 * KHbPsForegroundAppOrientationKey
 * Current orientation value checked from foreground app.
 */
const TUint KHbPsForegroundAppOrientationKey = 'Fgor';

/**
 * KHbFixedOrientationMask
 * Indicates HbMainWindow has fixed orientation enabled
 */
const TUint KHbFixedOrientationMask = 0x100;

/**
 * KHbOrientationMask
 * Used for masking orientation in PS-key
 */
const TUint KHbOrientationMask = 0xFF;

#endif //Q_OS_SYMBIAN

#endif //HB_COREPSKEYS_P_H
