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

#ifdef Q_OS_SYMBIAN

#include <e32base.h>

const uint hbsplash_version_major = 1;
const uint hbsplash_version_minor = 0;
const uint hbsplash_version_build = 1;

_LIT(hbsplash_server_name, "hbsplashgenerator");
_LIT(hbsplash_server_exe, "hbsplashgenerator.exe");
const TUid hbsplash_server_uid3 = { 0x2002E68B };

enum HbSplashServerFuncs {
    HbSplashSrvGetSplash = 1
};

enum HbSplashServerPanics {
    HbSplashSrvBadRequest,
    HbSplashSrvBadParam
};

#endif // Q_OS_SYMBIAN

#endif // HBSPLASHDEFS_P_H
