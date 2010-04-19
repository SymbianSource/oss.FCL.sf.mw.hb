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

#ifndef HBVARIANT_P_H
#define HBVARIANT_P_H

#include <QColor>
#include <QString>
#include <QVariant>
#include <QAtomicInt>

#include "hbmemoryutils_p.h"

class HbString;

class HB_CORE_PRIVATE_EXPORT HbVariant
{

friend class TestHbVariant;

public:
    enum Type {
        Invalid = 0,
        Int = 1,
        Double = 2,
        String = 3,
        StringList = 4,
        Color = 5,
        LastType = 0xffffffff // need this so that gcc >= 3.4 allocates 32 bits for Type
    };

private:
    struct HbVariantData 
    {
        HbVariantData()
            : mDataType(Invalid),
              stringSize(0),
              mRef(1)
        {
        }
        union Data
        {
            int i;
            double d;
            int offset;
        } mData;

        Type mDataType;
        int stringSize;
        QAtomicInt mRef;
    };

public:
    HbVariant(HbMemoryManager::MemoryType type=HbMemoryManager::InvalidMemory);
    HbVariant(const HbVariant &other);
    HbVariant(int val, HbMemoryManager::MemoryType type);
    HbVariant(double val, HbMemoryManager::MemoryType type);
    HbVariant(const QString &str, HbMemoryManager::MemoryType type);
    HbVariant(const char *val, HbMemoryManager::MemoryType type);
    HbVariant(const QColor &col, HbMemoryManager::MemoryType type);
    ~HbVariant();

    HbMemoryManager::MemoryType memoryType()
    {
        return mMemoryType;
    }

    Type type()
    {
        HbVariantData *data = getAddress<HbVariantData>(mMemoryType, mDataOffset, mShared);
        return data->mDataType;
    }

#ifdef CSS_PARSER_TRACES
    bool supportsPrinting() const;
    void print() const;
#endif

    int toInt() const;
    QString toString() const;
    QColor toColor() const;
    QStringList toStringList () const;
    double toDouble() const;

    HbVariant & operator=( int val );
    HbVariant & operator=( double val );
    HbVariant & operator=( const QString& val );
    HbVariant & operator=( const HbString& val );
    HbVariant & operator=( const QColor& col );
    HbVariant & operator=( const QStringList& /*strList*/ );
    HbVariant & operator=( const HbVariant& other );
    
    bool canConvert ( HbVariant::Type t ) const;
    bool convert ( HbVariant::Type t );
    operator QVariant() const;

    void clear();
    
private:
    HbVariantData * initializeData();
    void fillStringData(const QString &str);
    void fillColorData(const QColor &col);
    QString getString() const;
    QColor getColor() const;
    /**
     * to get address in given memory type for given offset
     * @Todo : this function needs to be fixed as hbstring taking mshared as another argument.
     */
    template<typename T>
    T * getAddress( HbMemoryManager::MemoryType type, int offset, bool shared ) const
    {
        T* data = 0;
        if( shared == true ) {
            data = HbMemoryUtils::getAddress<T>(HbMemoryManager::SharedMemory, offset);
        } else {
            data = HbMemoryUtils::getAddress<T>(type, offset);
        }
        Q_ASSERT(data != 0);
        return data;
    }

    void detach();

private:
    HbMemoryManager::MemoryType mMemoryType;
    bool mShared;    
    int mDataOffset;
};

#endif // HBVARIANT_P_H
