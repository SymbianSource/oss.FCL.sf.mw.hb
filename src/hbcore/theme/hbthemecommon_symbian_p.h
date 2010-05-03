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

#ifndef HBTHEMECOMMON_SYMBIAN_P_H
#define HBTHEMECOMMON_SYMBIAN_P_H

#include <e32base.h>
#include <hbthemecommon_p.h>
// To enable/disable debug messages for theme listener
#undef THEME_LISTENER_TRACES
struct TIconParams
{
    TBuf<256> fileName;
    TReal width;
    TReal height;
    TUint8 aspectRatioMode;
    TUint8 mode;
    TUint8 options;
    TBool mirrored;
    TUint32 rgba;
    TBool colorflag;
 };

struct TMultiIconSymbParams
{
    TBuf<256> multiPartIconId;
    TBuf<256> multiPartIconList[9];
    TRect sources[9];
    TRect targets[9];
    TSize pixmapSizes[9];
    QSizeF size;
    TInt aspectRatioMode;
    TInt mode;
    TInt options;
    TBool mirrored;
    TInt rgba;
    TBool colorflag;
 };

struct TIconListParams
{
    TBuf<256> iconList[9];
    QSizeF sizeList[9];
    TInt aspectRatioMode;
    TInt mode;
    TBool mirrored;
    TInt rgba;
    TBool colorflag;
    TInt iconCount;
};

// server name
_LIT(KThemeServerName,"hbthemeserver");
const TUid KServerUid3={0x20022E82};
// Common unique ID for Pub/Sub
const TInt KNewThemeForThemeChanger = 9;

// A version must be specifyed when creating a session with the server


const TUint KThemeServerMajorVersionNumber=0;
const TUint KThemeServerMinorVersionNumber=1;
const TUint KThemeServerBuildVersionNumber=1;

enum TThemeServerLeave
{
    ENonNumericString = 99
};

#endif // HBTHEMECOMMON_SYMBIAN_P_H

