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

#include "hbdeviceprofilemanager_p.h"
#include "hbdeviceprofile.h"
#include "hbinstance_p.h"
#include "hbmainwindow_p.h"

/*
    \class HbDeviceProfileManager
    \brief HbDeviceProfileManager allows to select specific profile to be used.

    This class is mostly for development purposes where there is need to test
    applications in different resolutions and settings.

    \sa HbDeviceProfile

    \proto
*/

/*!
    Modifies application to use selected profile and resizes main windows.
    In Symbian, only fullscreen main windows are modified.
    If main window has already specific profile, then it's not overridden.
*/
void HbDeviceProfileManager::select(const HbDeviceProfile &profile)
{
    HbInstancePrivate::d_ptr()->select(profile);
}

/*!
    Modifies main window to use selected profile and resizes the window.
    In Symbian, only fullscreen main windows are modified.
    \param window main window to be modified.
    \param profile profile for window.
*/
void HbDeviceProfileManager::select(HbMainWindow &window, const HbDeviceProfile &profile)
{
    HbMainWindowPrivate::d_ptr(&window)->select(profile);
}


// end of file

