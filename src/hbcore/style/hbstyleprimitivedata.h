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

#ifndef HBSTYLEPRIMITIVEDATA_H
#define HBSTYLEPRIMITIVEDATA_H

#include <QStyle>
#include <hbglobal.h>
#include <hbstylevalue.h>

class HB_CORE_EXPORT HbStylePrimitiveData
{
public:

    enum DataType {
            SPD_Base = 0,
            SPD_Text = 1,
            SPD_RichText = 2,
            SPD_Frame = 3,
            SPD_Icon = 4,
            SPD_Marquee = 5,
            SPD_TouchArea = 6,
        };
    
    enum PrimitiveDataType { Type = SPD_Base };
    enum PrimitiveDataVersion { Version = 1 };
    
    HbStylePrimitiveData(int type = SPD_Base, int version = 1);

    HbStyleValue<QRectF> geometry;

    QStyle::State state;
    int version;
    int type;
};



template <typename T>
T hbstyleprimitivedata_cast(const HbStylePrimitiveData *data)
{
    // data must be valid, version must be the same or greater, and the type must match
    if (data && data->version >= static_cast<T>(0)->Version && 
        (data->type == static_cast<T>(0)->Type))
        return static_cast<T>(data);
    return 0;
}

template <typename T>
T hbstyleprimitivedata_cast(HbStylePrimitiveData *data)
{
    if (data && data->version >= static_cast<T>(0)->Version && 
        (data->type == static_cast<T>(0)->Type))
        return static_cast<T>(data);
    return 0;
}


#endif // HBSTYLEPRIMITIVEDATA_H
