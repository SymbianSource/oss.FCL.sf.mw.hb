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

#include "hbnvgicondata_p.h"

HbNvgIconData::HbNvgIconData(quint32 length)
        : mNvgData(0),
        totalRead(0),
        readStream(0)
{
    mNvgData = new QByteArray(0, length);
    Q_CHECK_PTR(mNvgData);
}

HbNvgIconData::HbNvgIconData(const QByteArray &buffer)
        : mNvgData(0),
        totalRead(0),
        readStream(0)
{
    mNvgData = new QByteArray(buffer);
    Q_CHECK_PTR(mNvgData);
    dataSize = mNvgData->length();

    //set the reading pointers
    beginRead();
}

HbNvgIconData::~HbNvgIconData()
{
    delete mNvgData;
}

qint32 HbNvgIconData::encodeData(const void *data, quint32 length)
{
    mNvgData->append((const char*)data , length);
    return (qint32)HbNvgEngine::NvgErrNone; //in error case, exception will be thrown
}

void HbNvgIconData::beginRead()
{
    dataSize = mNvgData->length();
    totalRead = 0;
    readStream = (quint8 *)mNvgData->data();
}

void HbNvgIconData::endRead()
{
}

#define STR_TO_OTHER_DIR(TOTYPE) do {\
        TOTYPE data = *(TOTYPE *)&readStream[totalRead];\
        totalRead += sizeof(TOTYPE);\
        return data;\
    } while (0)


#define STR_TO_OTHER_IDIR(TOTYPE) do {\
        TOTYPE data;\
        quint8 * dataPtr = (quint8 *)&data;\
        for (qint32 i = 0; i < sizeof(TOTYPE); ++i)\
        {\
            dataPtr[i] = readStream[totalRead+i];\
        }\
        totalRead += sizeof(TOTYPE);\
        return data;\
    } while (0)

#define STR_TO_OTHER(TOTYPE) do {\
        checkOutOfBound(sizeof(TOTYPE));\
        if (reinterpret_cast<qint32>(&readStream[totalRead]) & (sizeof(TOTYPE) - 1))\
        {\
            STR_TO_OTHER_IDIR(TOTYPE);\
        }\
        else\
        {\
            STR_TO_OTHER_DIR(TOTYPE);\
        }\
    } while (0)

qint16 HbNvgIconData::readInt16()
{
    STR_TO_OTHER(qint16);
}

qint32 HbNvgIconData::readInt32()
{
    STR_TO_OTHER(qint32);
}

qint8 HbNvgIconData::readInt8()
{
    STR_TO_OTHER_DIR(qint8);
}

float HbNvgIconData::readReal32()
{
    STR_TO_OTHER(float);
}

qreal HbNvgIconData::readReal64()
{
    checkOutOfBound(sizeof(qreal));
    STR_TO_OTHER(qreal);
}

void HbNvgIconData::read(quint8 *ptr, qint32 length)
{
    checkOutOfBound(length);
    memcpy(ptr, &readStream[totalRead], length);
    totalRead += length;
}

void HbNvgIconData::skip(qint32 length)
{
    checkOutOfBound(length);
    totalRead += length;
}
