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

#include "hbicon.h"
#include "hbbadgeiconinfo_p.h"
#include "hbbadgeicon_p.h"
#include <QPainter>

#include <QDebug>


HbBadgeIconInfo::HbBadgeIconInfo(const HbIcon& badge,
                                 Qt::Alignment alignment,
                                 int zValue): mIcon(badge),mAlignment(alignment),mZValue(zValue)
{
}

HbBadgeIconInfo::HbBadgeIconInfo(const HbBadgeIconInfo& other)
{
    *this = other;
}

HbBadgeIconInfo::~HbBadgeIconInfo()
{
}

bool HbBadgeIconInfo::operator==(const HbBadgeIconInfo &other)
{
    return !(*this != other);
}

bool HbBadgeIconInfo::operator!=(const HbBadgeIconInfo &other)
{
    if ((icon() != other.icon()) ||
        (zValue() != other.zValue()) ||
        (alignment() != other.alignment())
       )
    {
        return true;
    }

    return false;
}

HbIcon HbBadgeIconInfo::icon() const
{
    return mIcon;
}

void HbBadgeIconInfo::setIcon(const HbIcon& icon)
{
    mIcon = icon;
}

int HbBadgeIconInfo::zValue() const
{
    return mZValue;
}

void HbBadgeIconInfo::setZValue(int zValue)
{
    mZValue = zValue;
}

Qt::Alignment HbBadgeIconInfo::alignment() const
{
    return mAlignment;
}

void HbBadgeIconInfo::setAlignment(Qt::Alignment align)
{
    mAlignment = align;
}

