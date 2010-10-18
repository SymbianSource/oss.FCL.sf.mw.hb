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

#ifndef HBSPLASHSCREEN_H
#define HBSPLASHSCREEN_H

#include <hbglobal.h>

class HB_CORE_EXPORT HbSplashScreen
{
public:
    enum Flag {
        Default         = 0x00,
        FixedVertical   = 0x01,
        FixedHorizontal = 0x02,
        ForceQt         = 0x04,
        ShowWhenStartingToBackground = 0x08
    };

    Q_DECLARE_FLAGS(Flags, Flag)

    static void start(Flags flags = Default, int argc = 0, char *argv[] = 0);
    static void destroy();
    static bool exists();
    static void setFlags(Flags flags);
    static void setAppId(const QString &appId);
    static void setScreenId(const QString &screenId);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HbSplashScreen::Flags)

#endif // HBSPLASHSCREEN_H
