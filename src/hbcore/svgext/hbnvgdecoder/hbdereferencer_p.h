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

#ifndef HB_DEREFERENCER_P_H
#define HB_DEREFERENCER_P_H

#include <QByteArray>
#include <QtGlobal>

#include "hbnvg_p.h"
#include "hbnvgexception_p.h"

class HbDereferencer
{
#define DEREF_PTR(TOTYPE, Offset, Size) do {\
                                checkOutOfBound(Offset + Size); \
                                return * (TOTYPE *)&mReadStream[mTotalRead + Offset];\
                           } while (0)
public:
   
    HbDereferencer(quint8* buf, qint32 length)
        : mTotalRead(0),
        mDataLength(length),
        mReadStream((unsigned char*)buf)
    {
    }

    HbDereferencer(const QByteArray& buffer)
            : mTotalRead(0),
            mDataLength(buffer.length()),
            mReadStream((unsigned char*)buffer.data())
    {        
    }

    void skip(qint32 length)
    {
        checkOutOfBound(length);
        mTotalRead += length;
    }

    qint16 derefInt16(qint16 at = 0)
    {
        DEREF_PTR(qint16, at, sizeof(qint16));
    }
    operator qint16()
    {
        return derefInt16();    
    }

    qint32 derefInt32(qint32 at = 0)
    {
        DEREF_PTR(qint32, at, sizeof(qint32));
    }
    
    operator qint32()
    {
        return derefInt32();    
    }

    qint8 derefInt8(qint32 at = 0)
    {
        DEREF_PTR(qint8, at, sizeof(qint8));
    }
    
    operator qint8()
    {
        return derefInt8();    
    }

    quint8* derefInt8Array(qint32 length, qint32 at = 0)
    {
        checkOutOfBound(at + length);
        return (quint8 *)&mReadStream[mTotalRead + at];
    }

    float derefReal32(qint32 at = 0)
    {
        DEREF_PTR(float, at, sizeof(float));
    }
    operator float()
    {
        return derefReal32();
    }

    void assertBound(qint32 length, qint32 at = 0)
    {
        checkOutOfBound(at + length);
    }

    quint8* getPtr() const
    {
        return mReadStream;
    }

    qint32 getLength() const
    {
        return mDataLength;
    }

    qint32 getReadingPos() const
    {
        return mTotalRead;
    }
    
private:
    void checkOutOfBound(qint32 length)
    {
        if (mTotalRead + length > mDataLength ||
                mTotalRead + length < 0){
            throw HbNvgException(HbNvgEngine::NvgErrEof);
        }
    }

    qint32                mTotalRead;
    qint32                mDataLength;
    quint8*            mReadStream;
};

#endif

