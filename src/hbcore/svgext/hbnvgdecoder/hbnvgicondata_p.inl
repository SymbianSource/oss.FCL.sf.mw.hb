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

inline void HbNvgIconData::checkOutOfBound(int length)
{
    if (totalRead + length > dataSize ||
            totalRead + length < 0){
        throw HbNvgException(HbNvgEngine::NvgErrEof);
    }
}

inline int HbNvgIconData::readPos()
    {
    return totalRead;
    }

inline int HbNvgIconData::dataLength()
    {
    return dataSize;
    }

inline bool HbNvgIconData::eof()
    {
    return (totalRead >= dataSize);
    }

inline int HbNvgIconData::encodeInt8(quint8 aVal)
    {
    return encodeData(&aVal, sizeof(aVal));
    }

inline int HbNvgIconData::encodeInt16(quint16 aVal)
    {
    return encodeData(&aVal, sizeof(aVal));
    }

inline int HbNvgIconData::encodeInt32(quint32 aVal)
    {
    return encodeData(&aVal, sizeof(aVal));
    }

inline int HbNvgIconData::encodeReal32(float aVal)
    {
    return encodeData(&aVal, sizeof(aVal));
    }

inline int HbNvgIconData::encodeReal64(qreal aVal)
    {
    return encodeData(&aVal, sizeof(aVal));
    }

inline HbDereferencer::HbDereferencer(quint8* buf, int length)
    : mTotalRead(0),
    mDataLength(length),
    mReadStream((unsigned char*)buf)
{
}

inline HbDereferencer::HbDereferencer(const QByteArray& buf)
    : mTotalRead(0),
    mDataLength(buf.length()),
    mReadStream((unsigned char*)buf.data())
    {        
    }

inline void HbDereferencer::checkOutOfBound(int length)
{
    if (mTotalRead + length > mDataLength ||
            mTotalRead + length < 0){
        throw HbNvgException(HbNvgEngine::NvgErrEof);
    }
}

inline void HbDereferencer::skip(int length)
    {
    checkOutOfBound(length);
    mTotalRead += length;
    }

#define DEREF_PTR(TOTYPE, Offset, Size) do {\
                                checkOutOfBound(Offset + Size); \
                                return * (TOTYPE *)&mReadStream[mTotalRead + Offset];\
                           } while (0)

inline float HbDereferencer::derefReal32(int at)
    {
    DEREF_PTR(float, at, sizeof(float));
    }

inline quint8* HbDereferencer::derefInt8Array(int length, int at)
    {
    checkOutOfBound(at + length);
    return (quint8 *)&mReadStream[mTotalRead + at];
    }

inline void HbDereferencer::isSafe(int length, int at)
    {
    checkOutOfBound(at + length);
    }

inline qint8 HbDereferencer::derefInt8(int at)
    {
    DEREF_PTR(qint8, at, sizeof(qint8));
    }

inline qint32 HbDereferencer::derefInt32(int at)
    {
    DEREF_PTR(int, at, sizeof(int));
    }

inline qint16 HbDereferencer::derefInt16(int at)
    {
    DEREF_PTR(qint16, at, sizeof(qint16));
    }

inline HbDereferencer::operator float()
    {
    return derefReal32();    
    }

inline HbDereferencer::operator qint8()
    {
    return derefInt8();    
    }

inline HbDereferencer::operator qint32()
    {
    return derefInt32();    
    }

inline HbDereferencer::operator qint16()
    {
    return derefInt16();    
    }

inline quint8* HbDereferencer::getPtr()
    {
    return mReadStream;
    }

inline int HbDereferencer::getLength()
    {
    return mDataLength;
    }

inline int HbDereferencer::getReadingPos()
    {
    return mTotalRead;
    }

