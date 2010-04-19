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

#include "hbvariant_p.h"

#include <QVariant>
#include <QDebug>

#include "hbmemoryutils_p.h"
#include "hbstring_p.h"
#include "hbsmartoffset_p.h"



/**
 * detach
 * used to support implicit sharing
 */
void HbVariant::detach()
{
    HbVariantData *data = getAddress<HbVariantData>( mMemoryType, mDataOffset, mShared);
    if( data->mRef > 1 || mShared == true ) {
        if( data->mRef > 1 ) {
            data->mRef.deref();
        }
        GET_MEMORY_MANAGER(mMemoryType);
        try{
            mDataOffset = manager->alloc( sizeof(HbVariantData) );

            HbVariantData *newData = new( (char*)manager->base() + mDataOffset ) HbVariantData();
            newData->mData = data->mData;
            newData->mDataType = data->mDataType;

            if ( data->mDataType == String ) {
                newData->mData.offset = manager->alloc( data->stringSize*sizeof(QChar) );

				::memcpy(HbMemoryUtils::getAddress<char>( mMemoryType, newData->mData.offset ),
                    getAddress<char>( mMemoryType, data->mData.offset, mShared ),
                    data->stringSize * sizeof( QChar) );
                newData->stringSize = data->stringSize;
            } else if( data->mDataType == Color ) {
                newData->mData.offset = manager->alloc( sizeof(QColor) );
                new (HbMemoryUtils::getAddress<QColor>( mMemoryType, newData->mData.offset) ) 
                        QColor(*getAddress<QColor>( mMemoryType, data->mData.offset, mShared) );
            }
            mShared = false;
    }
        catch(std::bad_alloc &badAlloc){
            Q_UNUSED(badAlloc)
            qCritical("HbVariant::detach() failed!");
            throw;
        }
    }
}


/*
* C'tor
*/
HbVariant::HbVariant( HbMemoryManager::MemoryType type )
    : mMemoryType( type ), mShared( false )
{
    initializeData();
}

/*
* C'tor taking int
*/
HbVariant::HbVariant( int val, HbMemoryManager::MemoryType type )
    : mMemoryType( type ), mShared( false )
{
    HbVariantData * data = initializeData();
    if(data){
        data->mData.i = val;
        data->mDataType = Int;
    }
}

/*
* C'tor taking double
*/
HbVariant::HbVariant( double val, HbMemoryManager::MemoryType type )
    : mMemoryType( type ),mShared( false )
{
    HbVariantData * data = initializeData();
    if(data){
        data->mData.d = val;
        data->mDataType = Double;
    }
}

/*
* C'tor taking Qstring
*/
HbVariant::HbVariant( const QString &str, HbMemoryManager::MemoryType type )
    : mMemoryType( type ), mShared( false )
{
    initializeData();
    fillStringData(str);
}

/*
* C'tor taking string
*/
HbVariant::HbVariant( const char *val, HbMemoryManager::MemoryType type )
    : mMemoryType( type ), mShared( false )
{
    initializeData();
    QString str = QString::fromAscii(val);
    fillStringData(str);
}


/*
* C'tor taking QColor
*/
HbVariant::HbVariant( const QColor &col, HbMemoryManager::MemoryType type )
    : mMemoryType(type), mShared(false)
{
    initializeData();
    fillColorData(col);
}

/*
* copy C'tor
*/

HbVariant::HbVariant( const HbVariant &other )
{

    mMemoryType = other.mMemoryType;
    GET_MEMORY_MANAGER(other.mMemoryType)

    HbVariantData* data = HbMemoryUtils::getAddress<HbVariantData>( mMemoryType, other.mDataOffset );
    mDataOffset = other.mDataOffset;

    if ( !manager->isWritable() || other.mShared == true ) {
        mShared = true;
        mMemoryType = HbMemoryManager::HeapMemory;
    } else {
        mShared = false;
        data->mRef.ref();
    }    
    Q_ASSERT( mMemoryType == HbMemoryManager::SharedMemory 
            || mMemoryType == HbMemoryManager::HeapMemory );
}

/*
* D'tor
*/
HbVariant::~HbVariant()
{
    GET_MEMORY_MANAGER(mMemoryType);
    // if the memory where the variant is not writable it means it's client process, so do nothing
    if( !manager->isWritable() ) 
        return;
    HbVariantData *data= getAddress<HbVariantData>( mMemoryType, mDataOffset, mShared );
    if( mShared != true && !data->mRef.deref() ) {
        clear();
        HbMemoryUtils::freeMemory( mMemoryType, mDataOffset );
    }
}

/*
* to convert to int
*/
int HbVariant::toInt() const
{
    HbVariantData *data = getAddress<HbVariantData>( mMemoryType, mDataOffset, mShared );
   
    switch(data->mDataType) {
    case HbVariant::Int:
        return data->mData.i;
    case HbVariant::Double:
        return int(data->mData.d);
    case HbVariant::String:
        return int(getString().toLongLong());
    case HbVariant::Color:
    case HbVariant::StringList:
        return 0;
    default:
        return 0;
    }
}

/*
* to convert to string
*/
QString HbVariant::toString() const
{
    HbVariantData *data = getAddress<HbVariantData>( mMemoryType, mDataOffset, mShared );
   
    switch(data->mDataType) {
    case HbVariant::Int:
        return QString::number(data->mData.i);
    case HbVariant::Double:
        return QString::number(data->mData.d);
    case HbVariant::String:
        return getString();
    case HbVariant::Color:
        return getColor().name();
    case HbVariant::StringList:
        return QString();
    default:
        return QString();
    }
}

/*
* to get QString
*/
QString HbVariant::getString() const
{
    HbVariantData *data = getAddress<HbVariantData>( mMemoryType, mDataOffset, mShared );
   
    if (data->mData.offset != -1) { 
        QChar* dataPtr = getAddress<QChar>( mMemoryType, data->mData.offset, mShared );
        return QString::fromRawData( dataPtr, data->stringSize );
    } else { // data->mData.offset == -1 is empty String Variant.
        return QString("");
    }
}

/*
* to get QColor
*/
QColor HbVariant::toColor() const
{
    HbVariantData *data = getAddress<HbVariantData>( mMemoryType, mDataOffset, mShared );
   
    switch(data->mDataType) {
    case HbVariant::String: {
        QColor col = QColor();
        col.setNamedColor(getString());
        return col;
        }
    case HbVariant::Color:
        return getColor();
    case HbVariant::StringList:
    case HbVariant::Int:
    case HbVariant::Double:
        return QColor();
    default:
        return QColor();
    }
}

/*
* to get QColor
*/
QColor HbVariant::getColor() const
{
    HbVariantData *data = getAddress<HbVariantData>( mMemoryType, mDataOffset, mShared );
   
    return *( getAddress<QColor>(mMemoryType, data->mData.offset, mShared) );
}

/*
* to get stringlist
*/
QStringList HbVariant::toStringList () const
{
    // ToDo: Implement it
    return QStringList();
}

/*
* to get double 
*/
double HbVariant::toDouble() const
{
    HbVariantData *data = getAddress<HbVariantData>( mMemoryType, mDataOffset, mShared );
   
    switch(data->mDataType) {
    case HbVariant::Int:
        return double(data->mData.i);
    case HbVariant::Double:
        return data->mData.d;
    case HbVariant::String:
        return double(getString().toDouble());
    case HbVariant::Color:
    case HbVariant::StringList:
        return double();
    default:
        return double();
    }
}

/*
* initializeData 
*/
HbVariant::HbVariantData * HbVariant::initializeData()
{
    GET_MEMORY_MANAGER(mMemoryType);
    try{
        mDataOffset = manager->alloc( sizeof(HbVariantData) );
        //Q_ASSERT(mDataOffset != -1);
        HbVariantData *data = getAddress<HbVariantData>( mMemoryType, mDataOffset, mShared );
        return new(data) HbVariantData();
    }
    catch(std::bad_alloc &badAlloc){        
              Q_UNUSED(badAlloc)
        qCritical("HbVariant::initializeData() failed!");
        return NULL;
    }
}

/*
* fillStringData 
*/
void HbVariant::fillStringData(const QString &str)
{
    GET_MEMORY_MANAGER(mMemoryType);
    HbVariantData *data = getAddress<HbVariantData>(mMemoryType, mDataOffset, mShared);
    if (data->mDataType==HbVariant::String 
            && data->mData.offset != -1) {
        //clean old string data when assigend with a new string value
        HbMemoryUtils::freeMemory(mMemoryType, data->mData.offset);
    }

    data->stringSize = str.length();
    // If the string is not null.. alocate memory for it and copy data
    if (data->stringSize) {
        try{
        data->mData.offset = manager->alloc(data->stringSize*sizeof(QChar));

        memcpy(getAddress<char>(mMemoryType, data->mData.offset, mShared),
            str.constData(),
            data->stringSize*sizeof(QChar));
        }
        catch(std::bad_alloc &badAlloc){
            Q_UNUSED(badAlloc)
            qCritical("HbVariant::fillStringData() failed!");
            data->mData.offset = -1;
            throw;
        }
    } else {
        data->mData.offset = -1;
    }

    data->mDataType = String;
}

/*
* fillColorData 
*/
void HbVariant::fillColorData( const QColor &col )
{
    GET_MEMORY_MANAGER(mMemoryType);
    HbVariantData *data = getAddress<HbVariantData>(mMemoryType, mDataOffset, mShared);
    
    int colDataSize = sizeof(col);
    try{

        if(data->mDataType!=HbVariant::Color) {
            data->mData.offset = manager->alloc(colDataSize);
        }
        new(getAddress<char>(mMemoryType, data->mData.offset, mShared)) QColor(col);
        data->mDataType = Color;
    }
    catch(std::bad_alloc &badAlloc){
              Q_UNUSED(badAlloc)
        qCritical("HbVariant::fillColorData failed!");
        throw;
    }

}

/*
* = operator taking int 
*/
HbVariant& HbVariant::operator=(int val)
{
    detach(); // This will update the mDataOffset to new location if ref > 1.

    HbVariantData *data = getAddress<HbVariantData>(mMemoryType, mDataOffset, mShared);

    if(data->mDataType==HbVariant::String || data->mDataType==HbVariant::Color) {
        HbMemoryUtils::freeMemory(mMemoryType, data->mData.offset);
        data->stringSize=0;
    }

    data->mData.i = val;
    data->mDataType = Int;
    return *this;
}

/*
* = operator taking double 
*/
HbVariant& HbVariant::operator=(double val)
{
    detach(); // This will update the mDataOffset to new location if ref > 1.

    HbVariantData *data = getAddress<HbVariantData>(mMemoryType, mDataOffset, mShared);

    if(data->mDataType==HbVariant::String || data->mDataType==HbVariant::Color) {
        HbMemoryUtils::freeMemory(mMemoryType, data->mData.offset);
        data->stringSize=0;
    }

    data->mData.d = val;
    data->mDataType = Double;
    return *this;
}

/*
* = operator taking QString 
*/
HbVariant& HbVariant::operator=(const QString& val)
{
    detach(); // This will update the mDataOffset to new location if ref > 1.
    HbVariantData *data = getAddress<HbVariantData>(mMemoryType, mDataOffset, mShared);
    if(data->mDataType == String || data->mDataType == Color) {
        HbMemoryUtils::freeMemory(mMemoryType, data->mData.offset);
        data->stringSize=0;
    }
    fillStringData(val);
    data->mDataType = String;
    return *this;
}

/*
* = operator taking HbString 
*/
HbVariant& HbVariant::operator=(const HbString& val)
{
    detach(); // This will update the mDataOffset to new location if ref > 1.
    HbVariantData *data = getAddress<HbVariantData>(mMemoryType, mDataOffset, mShared);
    if(data->mDataType == String || data->mDataType == Color) {
        HbMemoryUtils::freeMemory(mMemoryType, data->mData.offset);
        data->stringSize=0;
    }

    data->stringSize = val.length();
    // If the string is not null.. alocate memory for it and copy data
    if (data->stringSize) {
        try{
            GET_MEMORY_MANAGER(mMemoryType)
            data->mData.offset = manager->alloc(data->stringSize*sizeof(QChar));
            //Q_ASSERT(data->mData.offset != -1);
            memcpy(getAddress<char>(mMemoryType,data->mData.offset,mShared),
                   val.constData(),
            data->stringSize*sizeof(QChar));
    } 
        catch(std::bad_alloc &badAlloc){
            Q_UNUSED(badAlloc)
            qCritical("HbVariant::operator= failed!");
            data->mData.offset = -1;
            throw;
        }
    } else {
        data->mData.offset = -1;
    }

    data->mDataType = String;
    return *this;
}

/*
* = operator taking QColor 
*/
HbVariant& HbVariant::operator=(const QColor& col)
{
    detach(); // This will update the mDataOffset to new location if ref > 1.
    HbVariantData *data = getAddress<HbVariantData>(mMemoryType, mDataOffset, mShared);
    if(data->mDataType==String) {
        HbMemoryUtils::freeMemory(mMemoryType,data->mData.offset);
        data->stringSize=0;
    }

    fillColorData(col);
    data->mDataType = Color;
    return *this;
}

/*
* = operator taking QStringList 
*/
HbVariant& HbVariant::operator=(const QStringList& /*strList*/)
{
    // ToDo: Implement it
    return *this;
}

/*
* = operator taking HbVariant 
*/
HbVariant& HbVariant::operator=(const HbVariant &other)
{

    GET_MEMORY_MANAGER(mMemoryType)

    if(!manager->isWritable())
        Q_ASSERT(false);

	HbVariantData *otherData = getAddress<HbVariantData>(other.mMemoryType, other.mDataOffset, other.mShared);
    HbVariantData *data = getAddress<HbVariantData>(mMemoryType, mDataOffset, mShared);

    if(other.mMemoryType != mMemoryType || other.mShared == true) {
        if(mShared != true) {
            if(data->mRef == 1) {
                clear();
                HbMemoryUtils::freeMemory(mMemoryType, mDataOffset);
            }else {
                data->mRef.deref();
            }
        }
        mShared = true;
        mMemoryType = HbMemoryManager::HeapMemory;
    } else {
        
        otherData->mRef.ref();

        if(mShared != true && !data->mRef.deref() ) {
            clear();
            HbMemoryUtils::freeMemory(mMemoryType, mDataOffset);
        }
		mShared = other.mShared;
        mMemoryType = other.mMemoryType;
    }
    mDataOffset = other.mDataOffset;

    Q_ASSERT(mMemoryType == HbMemoryManager::SharedMemory || mMemoryType == HbMemoryManager::HeapMemory);
    return *this;
}

/*
* returns true if the value of variant can be converted to the type passed
*/
bool HbVariant::canConvert (HbVariant::Type t) const
{
    HbVariantData *data = getAddress<HbVariantData>(mMemoryType, mDataOffset, mShared);

    if(data->mDataType == t) {
        return true;
    }
    switch(uint(t)) {
    case HbVariant::Int:
        return data->mDataType == HbVariant::Double 
            || data->mDataType == HbVariant::String;
    case HbVariant::Double:
        return data->mDataType == HbVariant::Int
            || data->mDataType == HbVariant::String;
    case HbVariant::String:
        return data->mDataType == HbVariant::Int
            || data->mDataType == HbVariant::StringList
            || data->mDataType == HbVariant::Double
            || data->mDataType == HbVariant::Color;
    case HbVariant::StringList:
    case HbVariant::Color:
        return data->mDataType == HbVariant::String;
    default :
        return false;
    }
}

/*
* converts the variant to given type
*/
bool HbVariant::convert (HbVariant::Type t)
{
    detach();
    HbVariantData *data = getAddress<HbVariantData>(mMemoryType, mDataOffset, mShared);

    bool ok = true;
    int tempOffset = -1;
    QColor col; // Removing the Linux build error;
    QString str;
    if(data->mDataType == t) {
        return true;
    }
    if (!this->canConvert(t)) {
        return false;
    }
    switch (t) {
    case HbVariant::Int: 
        switch(data->mDataType) {
        case HbVariant::Double: 
            data->mData.i = qRound64(data->mData.d);
            data->mDataType = HbVariant::Int;
            return true;
        case HbVariant::String:
            str = getString();
            data->stringSize = 0;
            tempOffset = data->mData.offset;
            data->mData.i = str.toLongLong(&ok);
            HbMemoryUtils::freeMemory(mMemoryType, tempOffset);
            data->mDataType = HbVariant::Int;
            return ok;
        default:
            return false;
        }

    case HbVariant::Double: 
        switch(data->mDataType) {
        case HbVariant::Int: 
            data->mData.d = double(data->mData.i);
            data->mDataType = HbVariant::Double;
            return true;
        case HbVariant::String:
            str = getString();
            data->stringSize = 0;
            tempOffset = data->mData.offset;
            data->mData.d = str.toDouble(&ok);
            HbMemoryUtils::freeMemory(mMemoryType, tempOffset);
            data->mDataType = HbVariant::Double;
            return ok;
    default:
        return false;
        }

    case HbVariant::String: 
        switch(data->mDataType) {
        case HbVariant::Int : 
            fillStringData(QString::number(data->mData.i));
            data->mDataType = HbVariant::String;
            return true;
        case HbVariant::Double: 
            fillStringData(QString::number(data->mData.d));
            data->mDataType = HbVariant::String;
            return true;
        case HbVariant::StringList:
            // ToDo: Handle it
            return false;
        case HbVariant::Color: 
            col = getColor();
            HbMemoryUtils::freeMemory(mMemoryType,data->mData.offset);
            fillStringData(col.name()); 
            data->mDataType = HbVariant::String;
            return true;
        default:
            return false;
        }

    case HbVariant::Color: 
        switch(data->mDataType) {
        case HbVariant::String: 
            col.setNamedColor(getString());
            HbMemoryUtils::freeMemory(mMemoryType,data->mData.offset);
            data->stringSize = 0;
            fillColorData(col);
            data->mDataType = HbVariant::Color;
            // ToDo: should it return true as type is converted to Color
            // for invalid color also.
            return col.isValid();
        default:
            return false;
        }

    case HbVariant::StringList:
        return false;

    default:
        return false;
    }
}

/*
* converts to QVariant
*/
HbVariant::operator QVariant() const
{
    HbVariantData *data = getAddress<HbVariantData>(mMemoryType, mDataOffset, mShared);

    switch(data->mDataType) {
    case HbVariant::Int :
        return QVariant(data->mData.i);
    case HbVariant::Double :
        return QVariant(data->mData.d);
    case HbVariant::String : 
        return QVariant(getString());
    case HbVariant::StringList :
        return QVariant(QVariant::StringList);
    case HbVariant::Color : {
        QVariant var = getColor();
        return var;
    }
    default:
        return QVariant();
    }
}


/*
* clears the variant, frees any alocated memory
*/
void HbVariant::clear()
{
    detach(); // This can be optimise.
    HbVariantData *data = getAddress<HbVariantData>(mMemoryType,mDataOffset, mShared);
    
    switch(data->mDataType) {
    case HbVariant::Int : 
        data->mData.i=0;
        break;
    case HbVariant::Double :
        data->mData.d=double(0);
        break;
    case HbVariant::String :
    case HbVariant::Color :
        HbMemoryUtils::freeMemory(mMemoryType, data->mData.offset);
        break;
    case HbVariant::StringList : 
        // ToDo: Handle it
        break;
    default: 
        break;
    }
    data->stringSize = 0;
    data->mDataType=HbVariant::Invalid;
}

#ifdef CSS_PARSER_TRACES
/*
* These routines are there to support debugging
*/
bool HbVariant::supportsPrinting() const 
{
    return true;
}
    
void HbVariant::print() const
{
    HbVariantData * data = getAddress<HbVariantData>( mMemoryType, mDataOffset, mShared);
    switch(data->mDataType)
    {
    case HbVariant::Int:
        qDebug() << data->mData.i;
        break;
    case HbVariant::Double:
        qDebug() << (data->mData.d);
        break;
    case HbVariant::String:
        qDebug() << int(getString().toLongLong());
        break;
    case HbVariant::Color:
        qDebug() << getColor();
        break;
    case HbVariant::StringList:
    default:
        qDebug() << "Invalid Type";
    }
}

#endif
