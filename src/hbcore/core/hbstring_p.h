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

#ifndef HBSTRING_P_H
#define HBSTRING_P_H

#include <QAtomicInt>
#include "hbmemoryutils_p.h"

#ifdef CSS_PARSER_TRACES
#include "hbstringdata_p.h"
#include <QByteArray>
#include <QDebug>
#endif // CSS_PARSER_TRACES

class QDataStream;

class HB_CORE_PRIVATE_EXPORT HbString
{   
    HB_CORE_PRIVATE_EXPORT friend QDataStream & operator>>( QDataStream &in, HbString &str );
    HB_CORE_PRIVATE_EXPORT friend QDataStream & operator<<( QDataStream &out, const HbString &str );
    friend class TestHbString;

public:
    HbString( HbMemoryManager::MemoryType type = HbMemoryManager::InvalidMemory ); 
    HbString( const QString &str, HbMemoryManager::MemoryType type );
    HbString( const HbString &Other );
    ~HbString();

    HbMemoryManager::MemoryType memoryType() const
    {
        return mMemoryType;
    }

    operator QString() const;
    HbString & operator=( const HbString &str) ;
    HbString & operator=( const QLatin1String &str );
    HbString & operator= ( const QString& str );
    bool operator==( const HbString &str ) const;
    bool operator==( const QLatin1String &str ) const;
    bool operator==( const QString &str ) const;
    bool operator!=( const HbString &str ) const;
    bool operator!=( const QLatin1String &str ) const;
    bool operator!=( const QString &str ) const;

    void clear();
    void chop( int n );
    HbString & remove ( int position, int n );
    bool isEmpty() const;
    bool startsWith ( const QLatin1Char &s, Qt::CaseSensitivity cs = Qt::CaseSensitive ) const;
    HbString & prepend ( const QString &str );
    HbString & append ( const QString &str );
    HbString toLower () const;
    QByteArray toLatin1 () const;
    const QChar *constData() const;
    int length() const;
    bool contains( const QChar &ch ) const;
    int compare( const QLatin1String &other ) const;

#ifdef CSS_PARSER_TRACES
    bool supportsPrinting() const
    {
        return true; 
    }
    void print() const
    {
        if (mDataOffset != -1 && mDataOffset != -2) {
            GET_MEMORY_MANAGER(mMemoryType)
            HbStringData * mData = HbMemoryUtils::getAddress<HbStringData>( mMemoryType, mDataOffset);
            qDebug() << QString::fromRawData( (QChar*)((char*)manager->base() 
                                               + mData->mStartOffset), mData->mLength );
        }
    }
#endif //CSS_PARSER_TRACES

private:
    void copyString( const QChar *arr, int size );
    bool compareString( const QChar *rhs, int len ) const;
    void detach( int size );

private: // Data

    HbMemoryManager::MemoryType mMemoryType;
    bool mShared;

    int mDataOffset;
};

#endif // HBSTRING_P_H
