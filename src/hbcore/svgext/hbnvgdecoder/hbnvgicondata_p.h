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

#ifndef HB_NVGICONDATA_P_H
#define HB_NVGICONDATA_P_H

#include <QByteArray>
#include <QtGlobal>

#include "hbnvg_p.h"
#include "hbnvgexception_p.h"

class HbNvgIconData
{
public:
    HbNvgIconData(uint length = 0);

    HbNvgIconData(const QByteArray &buffer);

    virtual ~HbNvgIconData();

    int encodeInt8(quint8 val); 
   
    int encodeInt16(quint16 val); 
   
    int encodeInt32(quint32 val);
   
    int encodeReal32(float val); 
   
    int encodeReal64(qreal val);
   
    int encodeData(const void *data, quint32 length);

    void beginRead();

    void endRead();

    qint16 readInt16();

    qint32 readInt32();

    qint8 readInt8();

    void read(quint8 *ptr, int length);

    float readReal32();

    qreal readReal64();

    int readPos();
    
    void skip(int length);

    const char* buffer() {
        return mNvgData->data() ;
    }

    bool eof();
    
    int dataLength();

private:

	void checkOutOfBound(int length);
	    
    QByteArray*        mNvgData;
    int                totalRead;
    int                dataSize;
    quint8 *           readStream;
};

class HbDereferencer
    {
public:
    
    HbDereferencer(quint8* buf, int length);
    
    HbDereferencer(const QByteArray& buffer);
    
    void skip(int length);
    
    qint16 derefInt16(int at = 0);
    operator qint16();
    
    qint32 derefInt32(int at = 0);
    operator qint32();
    
    qint8 derefInt8(int at = 0);
    operator qint8();

    quint8 * derefInt8Array(int length, int at = 0);
       
    float derefReal32(int at = 0);
    operator float();
    
    void isSafe(int length, int at = 0);

    quint8* getPtr();

    int getLength();
    
    int getReadingPos();

private:
    void checkOutOfBound(int length);

    int                mTotalRead;
    int                mDataLength;
    quint8*            mReadStream;
    };

#include "hbnvgicondata_p.inl"
#endif
