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

#include <QMap>
#include <QFile>
#include <QDebug>

#include "hbdevicemodeinfo_p.h"
#include "hbwsiniparser_p.h"

class HbDeviceModeInfoPrivate
    {

public:
    HbDeviceModeInfoPrivate();
    void init(const QString &wsIniFile);

public:
    QMap<int, HbScreenMode> mModes;
    };

void HbDeviceModeInfoPrivate::init(const QString &wsIniFile)
{
    HbWsiniParser::parseModes(mModes, wsIniFile);
}

HbDeviceModeInfoPrivate::HbDeviceModeInfoPrivate()
{
}

/*!
    \class HbDeviceModeInfo
    \brief Provides information abstracted from the platform information on all the valid device modes

    All valid hardware and distinct user physical configurations should be accessed here.
    
*/

HbDeviceModeInfo::HbDeviceModeInfo(const QString &wsIniFile)
:   d_ptr(new HbDeviceModeInfoPrivate())
{
    d_ptr->init(wsIniFile);
}

HbDeviceModeInfo::~HbDeviceModeInfo()
{
    delete d_ptr;
}

/*!
    Returns the list of device mode numbers
*/
QList<int> HbDeviceModeInfo::modeNumbers() const
{
    return d_ptr->mModes.keys();
}


HbScreenMode *HbDeviceModeInfo::mode(int key)
{
    HbScreenMode *result(0);
    if (d_ptr->mModes.contains(key)){
        HbScreenMode &resultRef = d_ptr->mModes[key];
        result = &resultRef;
    }

    return result;
}



