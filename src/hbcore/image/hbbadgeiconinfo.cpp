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

#include "hbbadgeiconinfo_p.h"
#include "hbicon.h"
#include "hbbadgeicon_p.h"
#include <QPainter>
#include <QDebug>
#include <QPointF>

HbBadgeIconInfo::HbBadgeIconInfo(const HbIcon &badge,
                                 Qt::Alignment alignment,
                                 int zValue,
                                 const QSizeF &sizeFactor,
                                 Qt::AspectRatioMode aspectRatio)
    : mIcon(badge), mAlignment(alignment), mZValue(zValue),
      mSizeFactor(sizeFactor), mAspectRatio(aspectRatio)
{
}

bool HbBadgeIconInfo::operator==(const HbBadgeIconInfo &other) const
{
    return !(*this != other);
}

bool HbBadgeIconInfo::operator!=(const HbBadgeIconInfo &other) const
{
    if ((icon() != other.icon()) ||
            (zValue() != other.zValue()) ||
            (alignment() != other.alignment())
       ) {
        return true;
    }

    return false;
}

void HbBadgeIconInfo::setIcon(const HbIcon &icon)
{
    mIcon = icon;
}

void HbBadgeIconInfo::setZValue(int zValue)
{
    mZValue = zValue;
}

void HbBadgeIconInfo::setAlignment(Qt::Alignment align)
{
    mAlignment = align;
}

void HbBadgeIconInfo::setSizeFactor(const QSizeF &sizeFactor)
{
    mSizeFactor = sizeFactor;
}

void HbBadgeIconInfo::setAspectRatio(Qt::AspectRatioMode aspectRatio)
{
    mAspectRatio = aspectRatio;
}
