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

#ifndef HB_COREPSKEYS_R_H
#define HB_COREPSKEYS_R_H

#ifdef Q_OS_SYMBIAN

#include <e32cmn.h>

/**
 * HbCore orientation PS category UID.
 */
const TUid KHbPsHardwareCoarseOrientationCategoryUid = {0x20022E82}; // Theme server UID

/**
 * HbCore internal foreground application orientation PS category UID.
 */
const TUid KHbPsForegroundAppOrientationCategoryUid = {0x20022FC5}; // Device dialog server UID

/**
 * Contains the current orientation value based on the hw sensors and
 * other components.
 *
 * The value is not necessarily based solely on the accelerometer, it
 * may be different than that due to the hw keyboard state, lock
 * switch, etc. In any case this is the suggested orientation for
 * applications and the framework will use this orientation when the
 * automatic orientation management is in use by a Hb application
 * (i.e. by default).
 *
 * The sensors are tracked only while the device is not in sleep
 * mode. This means that the value is not guaranteed to be immediately
 * up-to-date with the current accelerometer orientation after coming
 * out of sleep mode.
 *
 * The value is a TRenderOrientation.
 *
 * Published by theme server. Used by Hb apps themselves (e.g. when
 * showing splash screens) and possibly by wserv.
 */
const TUint KHbPsHardwareCoarseWsOrientationKey = 0x4F726965;

/**
 * Contains the current orientation value based on the hw sensors and
 * other components.
 *
 * The value is not necessarily based solely on the accelerometer, it
 * may be different than that due to the hw keyboard state, lock
 * switch, etc. In any case this is the suggested orientation for
 * applications and the framework will use this orientation when the
 * automatic orientation management is in use by a Hb application
 * (i.e. by default).
 *
 * The sensors are tracked only while the device is not in sleep
 * mode. This means that the value is not guaranteed to be immediately
 * up-to-date with the current accelerometer orientation after coming
 * out of sleep mode.
 *
 * The value is a Qt::Orientation.
 *
 * Published by theme server. Used by Hb apps themselves (e.g. when
 * showing splash screens) and possibly by wserv.
 */
const TUint KHbPsHardwareCoarseQtOrientationKey = 0x4F726966;

/**
 * Current orientation value from the foreground app.
 * Applies only to Hb applications, non-Hb apps are not taken into account.
 *
 * Bits 0-7 contain the Qt::Orientation value.
 * If bit 8 is set then the orientation is a fixed (forced) one.
 * If bit 8 is not set then the orientation is managed automatically by the framework.
 *
 * Published by device dialog server.
 */
const TUint KHbPsForegroundAppOrientationKey = 0x46676F72;

/**
 * Mask for the bit that indicates HbMainWindow has fixed orientation
 * enabled.
 */
const TUint KHbFixedOrientationMask = 0x100;

/**
 * Used for masking orientation in PS-key.
 */
const TUint KHbOrientationMask = 0xFF;

#endif //Q_OS_SYMBIAN

#endif //HB_COREPSKEYS_R_H
