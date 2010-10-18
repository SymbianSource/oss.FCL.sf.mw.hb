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

#ifndef HBSTYLEFRAMEPRIMITIVEDATA_H
#define HBSTYLEFRAMEPRIMITIVEDATA_H

#include <QBitmap>
#include <hbglobal.h>
#include <hbstyleprimitivedata.h>
#include <hbframedrawer.h>
#include <hbstylevalue.h>

class HB_CORE_EXPORT HbStyleFramePrimitiveData : public HbStylePrimitiveData
{
public:
    HbStyleFramePrimitiveData();

    enum PrimitiveDataType { Type = SPD_Frame };
    enum PrimitiveDataVersion { Version = 1 };
    
    HbStyleValue<QString> frameGraphicsName;
    HbStyleValue<HbFrameDrawer::FrameType> frameType;
    HbStyleValue<qreal> borderWidthLeft;
    HbStyleValue<qreal> borderWidthTop;
    HbStyleValue<qreal> borderWidthRight;
    HbStyleValue<qreal> borderWidthBottom;
    HbStyleValue<bool> fillWholeRect;
    HbStyleValue<HbIcon::MirroringMode> mirroringMode;
    HbStyleValue<QStringList> fileNameSuffixList;
    HbStyleValue<QPixmap> pixmapMask;
    HbStyleValue<QBitmap> bitmapMask;
    HbStyleValue<QPainterPath> clipPath;
};

#endif // HBSTYLEFRAMEPRIMITIVEDATA_H
