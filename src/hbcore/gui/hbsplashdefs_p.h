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

#ifndef HBSPLASHDEFS_P_H
#define HBSPLASHDEFS_P_H

#include <qglobal.h>
#include <QDebug>

#ifdef Q_OS_SYMBIAN

#include <e32base.h>

const uint hbsplash_version_major = 1;
const uint hbsplash_version_minor = 0;
const uint hbsplash_version_build = 1;

_LIT(hbsplash_server_name, "hbsplashgenerator");
_LIT(hbsplash_server_exe, "hbsplashgenerator.exe");
const TUid hbsplash_server_uid3 = { 0x2002E68B };

enum HbSplashServerFuncs {
    HbSplashSrvGetSplashFile = 1,
    HbSplashSrvGetSplashData
};

enum HbSplashServerPanics {
    HbSplashSrvBadRequest,
    HbSplashSrvBadParam
};

// Keys for P&S properties containing the statusbar bitmap handles.
enum HbSplashStatusBarKeys {
    HbSplashSbBitmapPrtKey = 1,
    HbSplashSbBitmapLscKey
};

#endif // Q_OS_SYMBIAN

enum HbSplashStoredExtraFlags {
    HbSplashNonStandardStatusBar = 1
};

/*
  Enable the following to get more extensive debug prints from HbSplash and HbSplashScreen.
  Applies also to the server (hbsplashgenerator).
*/
//#define HB_SPLASH_VERBOSE_LOGGING

/*
  Enable the following to have splash screen data compressed before writing to file.
 */
#define HB_SPLASH_COMPRESSION

#ifdef HB_SPLASH_VERBOSE_LOGGING
inline QDebug splDeb() { return qDebug(); }
#define splDebug qDebug
#else
inline QNoDebug splDeb() { return QNoDebug(); }
#define splDebug QT_NO_QDEBUG_MACRO
#endif

#endif // HBSPLASHDEFS_P_H
