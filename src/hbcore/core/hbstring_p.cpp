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

#include "hbstring_p.h"
#include "hbstringdata_p.h"
#include "hbsmartoffset_p.h"

#include <QLatin1String>
#include <QChar>
#include <QDebug>

static const int shared_null_offset = -2;
static HbStringData *shared_null = 0;

HbStringData::HbStringData() :
    mStartOffset(-1),
    mLength(0),
    mCapacity(0),
    mRef(1)
{
}

HbStringData *getStringData(HbMemoryManager::MemoryType type, int offset, bool shared = false)
{
    if (offset == shared_null_offset) {
        return shared_null;
    } else {
        return getAddress<HbStringData>(type, offset, shared);
    }
}

/*
    Constructs a new HbString with \a MemoryType (SharedMemory/HeapMemory).
*/
HbString::HbString(HbMemoryManager::MemoryType type)
    : mMemoryType(type), mShared(false)
{
    if (type == HbMemoryManager::HeapMemory) {
        // Set offset to point to default NULL instance of HbStringData to
        // avoid unnecessary heap allocation when constructing HbString instances.
        mDataOffset = shared_null_offset;
        if (!shared_null) {
            shared_null = new HbStringData;
        }
        shared_null->mRef.ref();
        return;
    }

    GET_MEMORY_MANAGER(type)
    RETURN_IF_READONLY(manager)

    mDataOffset = manager->alloc(sizeof(HbStringData));
    HbStringData* data = HbMemoryUtils::getAddress<HbStringData>(mMemoryType, mDataOffset);
    new(data) HbStringData();
}

/*
    Constructs a new HbString with \a QString and \a MemoryType (SharedMemory/HeapMemory).
*/
HbString::HbString(const QString& str, HbMemoryManager::MemoryType type)
    : mMemoryType(type), mShared(false), mDataOffset(-1)
 {   
    GET_MEMORY_MANAGER(type)
    RETURN_IF_READONLY(manager)
    HbSmartOffset dataOffset(manager->alloc(sizeof(HbStringData)), type);

    int length = str.length();
    HbStringData* data = getStringData(mMemoryType, dataOffset.get());
    new(data) HbStringData(); 
    if (length) {
        data->mStartOffset = manager->alloc(length*sizeof(QChar));
        ::memcpy((char*) manager->base() + data->mStartOffset, str.constData(), length*sizeof(QChar));
    } else {
        data->mStartOffset = -1;
    }
    data->mLength = length;
    data->mCapacity = length;
    mDataOffset = dataOffset.release();
}

/*
    Constructs a new HbString with \a HbString.
*/
HbString::HbString(const HbString& other)
{
    mMemoryType = other.mMemoryType;
    GET_MEMORY_MANAGER(other.mMemoryType)

    HbStringData* data = getStringData(mMemoryType, other.mDataOffset);
    mDataOffset = other.mDataOffset;

    if(!manager->isWritable() || other.mShared == true) {
       mShared = true;
       mMemoryType = HbMemoryManager::HeapMemory;
    } else {
        mShared = false;
        data->mRef.ref();
    }
}

/*
  Destructor for the HbString.
*/
HbString::~HbString()
{
    GET_MEMORY_MANAGER(mMemoryType);
    // if the memory where the string is not writable it means it's client process, so do nothing
    if (!manager->isWritable() || mDataOffset == shared_null_offset) 
       return;

    HbStringData* data = getStringData(mMemoryType, mDataOffset, mShared);

    if (mShared != true && !data->mRef.deref()) {
        clear();
        data->~HbStringData();
        HbMemoryUtils::freeMemory(mMemoryType, mDataOffset);
    }
}

static bool memEquals(const quint16 *a, const quint16 *b, int length)
{
    // Benchmarking indicates that doing memcmp is much slower than
    // executing the comparison ourselves.
    // To make it even faster, we do a 32-bit comparison, comparing
    // twice the amount of data as a normal word-by-word comparison.
    //
    // Benchmarking results on a 2.33 GHz Core2 Duo, with a 64-QChar
    // block of data, with 4194304 iterations (per iteration):
    //    operation             usec            cpu ticks
    //     memcmp                330               710
    //     16-bit                 79             167-171
    //  32-bit aligned            49             105-109
    //
    // Testing also indicates that unaligned 32-bit loads are as
    // performant as 32-bit aligned.
    if (a == b || !length)
        return true;

    register union {
        const quint16 *w;
        const quint32 *d;
        quintptr value;
    } sa, sb;
    sa.w = a;
    sb.w = b;

    // check alignment
    if ((sa.value & 2) == (sb.value & 2)) {
        // both addresses have the same alignment
        if (sa.value & 2) {
            // both addresses are not aligned to 4-bytes boundaries
            // compare the first character
            if (*sa.w != *sb.w)
                return false;
            --length;
            ++sa.w;
            ++sb.w;

            // now both addresses are 4-bytes aligned
        }

        // both addresses are 4-bytes aligned
        // do a fast 32-bit comparison
        register const quint32 *e = sa.d + (length >> 1);
        for ( ; sa.d != e; ++sa.d, ++sb.d) {
            if (*sa.d != *sb.d)
                return false;
        }

        // do we have a tail?
        return (length & 1) ? *sa.w == *sb.w : true;
    } else {
        // one of the addresses isn't 4-byte aligned but the other is
        register const quint16 *e = sa.w + length;
        for ( ; sa.w != e; ++sa.w, ++sb.w) {
            if (*sa.w != *sb.w)
                return false;
        }
    }
    return true;
}

static int compare_helper(const QChar *data1, int length1, const QLatin1String &s2)
{
    const ushort *uc = reinterpret_cast<const ushort *>(data1);
    const ushort *e = uc + length1;
    const uchar *c = (uchar *)s2.latin1();

    if (!c) {
        return length1;
    }

    // Case sensitive comparison
    while (uc != e && *c && *uc == *c) {
        uc++, c++;
    }

    if (uc == e) {
        // If both strings reached end, consider them equal
        if (!*c) {
            return 0;
        }
        // If this string reached end and other didn't, this string is considered "less" than the other
        else {
            return -1;
        }
    }

    return *uc - *c;
}



/*
    Private Utility function that copies the const char* to the HbString
*/
void HbString::copyString(const QChar *arr, int size)
{
    GET_MEMORY_MANAGER(mMemoryType)
    RETURN_IF_READONLY(manager)
    
    HbStringData* data = getStringData(mMemoryType, mDataOffset, mShared);
    // If source string is empty, simply make the destination string's length
    // as zero and keep the capacity as same for future use.
    if (size == 0){
        data->mLength = 0;
        return;
    }
    if (data->mStartOffset != -1) {
        //current size is more than the Capacity, realloc with the new size
        if (size > data->mCapacity ) {
            data->mStartOffset = manager->realloc(data->mStartOffset, size*sizeof(QChar));
            data->mCapacity = size;
        }
        // copies the string
        ::memcpy(getAddress<char>(mMemoryType, data->mStartOffset, mShared), arr, size*sizeof(QChar)); 
        data->mLength = size;
    } else {
        data->mStartOffset = manager->alloc(size*sizeof(QChar));

        // copies the string
        ::memcpy(getAddress<char>(mMemoryType, data->mStartOffset, mShared), arr, size*sizeof(QChar));
        data->mLength = size;
        data->mCapacity = size;
    }
}

/*
    Private Utility function that compares the two strings and returns boolean value
*/
bool HbString::compareString(const QChar *rhs, int len) const
{
    HbStringData* data = getStringData(mMemoryType, mDataOffset, mShared);
    
    if (len != data->mLength) {
        return false;
    } else if( data->mStartOffset == -1 && (rhs == 0 || len == 0)) { // Empty strings
        return true;
    } else if ( data->mStartOffset == -1 || len == 0 ) {
        return false;
    }
    
    QChar *ptr = getAddress<QChar>(mMemoryType, data->mStartOffset, mShared);
    return memEquals((quint16 *)ptr, (quint16 *)rhs, len);
}    

/*
    This function creates a copy of data based on the size
*/
void HbString::detach(int size)
{    
    
    HbStringData* data = getStringData(mMemoryType, mDataOffset, mShared);
    if(data->mRef > 1 || mShared == true) {
        if(data->mRef > 1) {
            data->mRef.deref();
        }
        GET_MEMORY_MANAGER(mMemoryType);
        HbSmartOffset dataOffset(manager->alloc(sizeof(HbStringData)), mMemoryType);

        HbStringData* newData = 0;
        newData = getStringData(mMemoryType,dataOffset.get());
        new(newData) HbStringData();
		
		// Allocate new string buffer if given size is greater than zero
		if (size > 0) {
			newData->mStartOffset = manager->alloc(size*sizeof(QChar));
			newData->mLength = qMin(data->mLength, size);
			// copy old string data contents in the new string data
			::memcpy(HbMemoryUtils::getAddress<char>(mMemoryType, newData->mStartOffset),
                getAddress<char>(mMemoryType, data->mStartOffset, mShared),
				newData->mLength * sizeof(QChar));
		} 

        newData->mCapacity = size;
        mShared = false;
        mDataOffset = dataOffset.release();
    }
}

/*
    This function returns the QString form of HbString
*/
HbString::operator QString() const
{
    HbStringData * data = getStringData(mMemoryType, mDataOffset, mShared);
    const QChar *ptr = getAddress<QChar>(mMemoryType, data->mStartOffset, mShared);
    if(ptr)
        return QString(ptr, data->mLength);
    else {
        // TODO: this should return QString() but currently it causes wierd behaviour.
        // Seems some clients assume it returns "".
        return QString("");
    }
}

/*
    Overloaded assignment operator that takes HbString as argument 
*/
HbString& HbString::operator=(const HbString &other)
{
    // self assignment check
    if (this != &other){
        GET_MEMORY_MANAGER(mMemoryType)

        if(!manager->isWritable())
            Q_ASSERT(HbMemoryManager::HeapMemory == mMemoryType);

        HbStringData* otherData = 0;
		otherData = getStringData(other.mMemoryType, other.mDataOffset, other.mShared);
        HbStringData* data = 0;
        data = getStringData(mMemoryType, mDataOffset, mShared);
    
        if(other.mMemoryType != mMemoryType || other.mShared == true) {
            if(mShared != true && data->mRef == 1) {                
                if(data->mRef == 1) {
                    clear();
                    data->~HbStringData();
                    HbMemoryUtils::freeMemory(mMemoryType,mDataOffset);
                } else {
                    data->mRef.deref();
                }
            }
            mShared = true;
            mMemoryType = HbMemoryManager::HeapMemory;
        } else {            
            otherData->mRef.ref();

            if(mShared != true && !data->mRef.deref() ) {
                clear(); 
                data->~HbStringData();
                HbMemoryUtils::freeMemory(mMemoryType,mDataOffset);
            }                 
            mMemoryType = other.mMemoryType;
            mShared = other.mShared;
        }
        mDataOffset = other.mDataOffset;
    }
    return *this;
}

/*
    Overloaded assignment operator that takes QLatin1String as argument 
*/
HbString& HbString::operator=(const QLatin1String &str)
{
    bool result = false;    
    
    // This creates a temp string from the latin1 string.
    // It causes one extra mem alloc but it does the conversion from latin1 that is needed.
    QString tempStr(str);
    int len = tempStr.length();

    // Compares both strings
    result = compareString(tempStr.constData(), len);

    // If both strings are not equal, then assigns QLatin1String to HbString
    if (!result) {
        detach(len);
        copyString(tempStr.constData(), len);
    }

    return *this;
}

/*
    Overloaded assignment operator that takes QString as argument 
*/
HbString& HbString::operator=(const QString& str)
{
    int len = 0;
    const QChar *ptr = str.constData();
    if(ptr) {
        len = str.length();
        // Compares both strings.
        // If both strings are not equal, then assigns QString to HbString
        if(!compareString(ptr, len)) {
            detach(len);
            copyString(ptr, len);
        }
    }
    return *this;
}

/*
    Overloaded "==" operator that takes HbString as argument
    and returns boolean value
*/
bool HbString::operator==(const HbString &other) const
{
    if(mDataOffset == other.mDataOffset) {
        return true;
    }

    HbStringData* data = getStringData(mMemoryType, mDataOffset, mShared);
    HbStringData* otherData = getStringData(other.mMemoryType, other.mDataOffset, other.mShared);;

    if (data->mLength != otherData->mLength) {
        return false;
    }

    QChar *ptr = getAddress<QChar>(mMemoryType, data->mStartOffset, mShared);
	QChar *otherPtr = getAddress<QChar>(other.mMemoryType, otherData->mStartOffset, other.mShared);

    return memEquals((quint16 *)ptr, (quint16 *)otherPtr, data->mLength);
}

/*
    Overloaded "==" operator that takes QLatin1String as 
    argument and returns boolean value.
*/
bool HbString::operator==(const QLatin1String& str) const
{

    HbStringData* data = getStringData(mMemoryType, mDataOffset, mShared);
    const ushort *uc = (const ushort *)getAddress<QChar>(mMemoryType, data->mStartOffset, mShared);
    const ushort *e = uc + data->mLength;
    const uchar *c = (uchar *)str.latin1();

    if (!c)
        return !data->mLength;

    while (*c) {
        if (uc == e || *uc != *c)
            return false;
        ++uc;
        ++c;
    }
    return (uc == e);
}

/*
    Overloaded "==" operator that takes QString as 
    argument and returns boolean value.
*/
bool HbString::operator==(const QString &str) const
{
    return compareString(str.constData(), str.length());
}

/*
    Overloaded "!=" operator that takes HbString as argument
    and returns boolean value
*/
bool HbString::operator!=(const HbString &str) const
{
    return !(*this == str);
}

/*
    Overloaded "!=" operator that takes QLatin1String as 
    argument and returns boolean value.
*/
bool HbString::operator!=(const QLatin1String& str) const
{
    return !(*this==str);
}

/*
    Overloaded "!=" operator that takes QString as 
    argument and returns boolean value.
*/
bool HbString::operator!=(const QString &str) const
{
    return !(*this == str);
}

/*
    This function clears the HbString by freeing the allocated memory.
*/
void HbString::clear()
{
    HbStringData* data = 0;

    data = getStringData(mMemoryType, mDataOffset, mShared);

    detach(0); // This will update the new mDataOffset.
    GET_MEMORY_MANAGER(mMemoryType)
    RETURN_IF_READONLY(manager);

    data = getStringData(mMemoryType, mDataOffset, mShared);

    if (data->mStartOffset != -1) {
        manager->free(data->mStartOffset);
        data->mStartOffset = -1;
        data->mLength = 0;
        data->mCapacity = 0;
    }
}

/*
    This function removes the last 'n' characters from the HbString.
*/
void HbString::chop(int n)
{
    GET_MEMORY_MANAGER(mMemoryType)
    RETURN_IF_READONLY(manager);

    HbStringData* data = 0;
    data = getStringData(mMemoryType, mDataOffset, mShared);

    detach(data->mLength);
    HbStringData* newData = 0;
    newData = getStringData(mMemoryType, mDataOffset, mShared);

    if (n > 0) {
        if(newData->mStartOffset != -1 && newData->mLength > n){
            newData->mLength -= n;
        } else {
            clear();
        }
    }
}

/*
    This function removes the 'n' no. of characters from the specified 
    position from the HbString.
*/
HbString& HbString::remove(int position, int n)
{
    GET_MEMORY_MANAGER(mMemoryType)
    RETURN_OBJECT_IF_READONLY(manager, *this)
    
    HbStringData* data = 0;
    data = getStringData(mMemoryType, mDataOffset, mShared);

    detach(data->mLength);

    HbStringData* newData = getStringData(mMemoryType, mDataOffset, mShared);
    QChar *ptr = 0;
    ptr = getAddress<QChar>(mMemoryType, newData->mStartOffset, mShared);
    int i = position;
    if(ptr){
        if(n > (newData->mLength - position)){
            n = newData->mLength - position;
            chop(n);
        } else{
            while (i <= (position + n)) {
                *(ptr + i) = *(ptr + i + n);
                i++;
            }
            newData->mLength -= n;
        }
    }
    return *this;
}

/*
    Checks whether the HbString is empty and returns the boolean value.
*/
bool HbString::isEmpty() const
{ 
    HbStringData* data = 0;
    data = getStringData(mMemoryType, mDataOffset, mShared);
    return (data->mLength == 0)? true:false;
}

/*
    Checks whether the HbString starts with the specified character and returns 
    the boolean value. Also considers the case sensitivity based on the argument
    passed to it.
*/
bool HbString::startsWith ( const QLatin1Char &s, Qt::CaseSensitivity cs ) const 
{
    HbStringData* data = 0;
    data = getStringData(mMemoryType, mDataOffset, mShared);

    if (data->mStartOffset == -1) {
        return false;
    }

    char* ptr = getAddress<char>(mMemoryType, data->mStartOffset, mShared);
    char ch = s.toLatin1();

    if(cs == Qt::CaseSensitive){
        return (*ptr == ch);
    }
    else{
        return QChar(*ptr).toLower() == QChar(ch).toLower();
    }
}

/*
    Prepends the passed QString to the existing HbString
*/
HbString& HbString::prepend(const QString &str)
{
    GET_MEMORY_MANAGER(mMemoryType)
    RETURN_OBJECT_IF_READONLY(manager, *this)
   
    HbStringData* data = 0;
    data = getStringData(mMemoryType, mDataOffset, mShared);

    int size = data->mLength + str.length();

    detach(size);
    HbStringData* newData = 0;
    newData = getStringData(mMemoryType, mDataOffset, mShared);

    if(size > newData->mCapacity ) {
        newData->mStartOffset = manager->realloc(newData->mStartOffset, size*sizeof(QChar));
        newData->mCapacity = size;
    }

    if(newData->mStartOffset != -1) {
        char *dest = getAddress<char>(mMemoryType, newData->mStartOffset, mShared);
        memmove(dest + str.length() * sizeof(QChar), dest, newData->mLength*sizeof(QChar));
        memcpy(dest, str.constData(), str.length() * sizeof(QChar));
        newData->mLength = size;
    }
    return *this;
}

/*
    Appends the passed QString to the existing HbString
*/
HbString& HbString::append(const QString &str) 
{
    GET_MEMORY_MANAGER(mMemoryType)
    RETURN_OBJECT_IF_READONLY(manager, *this)
    
    HbStringData* data = 0;
    data = getStringData(mMemoryType, mDataOffset, mShared);

    int size = data->mLength + str.length();

    detach(size);

    HbStringData* newData = 0;
    newData = getStringData(mMemoryType, mDataOffset, mShared);

    if(size > newData->mCapacity ) {
        newData->mStartOffset = manager->realloc(newData->mStartOffset, size*sizeof(QChar));
        newData->mCapacity = size;
    }

    if(newData->mStartOffset != -1) {
        char *dest = getAddress<char>(mMemoryType, newData->mStartOffset, mShared);
        memcpy(dest + newData->mLength*sizeof(QChar), str.constData(), str.length()*sizeof(QChar));
        newData->mLength = size;
    }
    return *this;
}

/*
    Converts the HbString into lowercase and returns the converted copy of 
    the HbString
*/
HbString HbString::toLower() const
{
    GET_MEMORY_MANAGER(mMemoryType)
    HbStringData* data = 0;
    data = getStringData(mMemoryType, mDataOffset, mShared);

    QChar *src = getAddress<QChar>(mMemoryType, data->mStartOffset, mShared);
    
    // check whether source memory type is writable if yes, create temporary HbString using same
    // memory manager else use Heap memory
    HbMemoryManager::MemoryType tempMemoryType;
    if (manager->isWritable()) {
        tempMemoryType = mMemoryType;
    } else {
        tempMemoryType = HbMemoryManager::HeapMemory;
    }
    HbString strTemp(tempMemoryType);

    HbStringData* newData = 0;
	newData = getStringData(strTemp.mMemoryType, strTemp.mDataOffset, strTemp.mShared);
    
    if(data->mLength > 0){
        // get memory manager of tempMemoryType
        GET_MEMORY_MANAGER(strTemp.mMemoryType)
        newData->mStartOffset = manager->alloc(data->mLength*sizeof(QChar));
        newData->mLength = data->mLength;
        newData->mCapacity = data->mLength;

		QChar *dest = getAddress<QChar>(tempMemoryType, newData->mStartOffset, strTemp.mShared);

        int count = 0;
        while( count < data->mLength){
            *dest = src->toLower();
            dest++;
            src++;
            count++;
        }
    }
    return strTemp;
}

/*
    Returns QByteArray keeping latin representation of string
*/
QByteArray HbString::toLatin1 () const
{
    HbStringData* data = getStringData(mMemoryType, mDataOffset, mShared);
    QChar * ptr = getAddress<QChar>(mMemoryType, data->mStartOffset, mShared);
    return QString::fromRawData(ptr, data->mLength).toLatin1();
}

/*
    Returns const pointer to string data
*/
const QChar *HbString::constData() const
{
    HbStringData* data = getStringData(mMemoryType, mDataOffset, mShared);
    return getAddress<QChar>(mMemoryType, data->mStartOffset, mShared);
}

/*
    Returns length of string
*/
int HbString::length() const
{
    HbStringData* data = getStringData(mMemoryType, mDataOffset, mShared);
    return data->mLength;
}

/*
    Test whether string contains character ch. Check is case sensitive.
*/
bool HbString::contains(const QChar &ch) const
{
    HbStringData* data = getStringData(mMemoryType, mDataOffset, mShared);
    if (data->mLength) {
        QChar *stringData = getAddress<QChar>(mMemoryType, data->mStartOffset, mShared);
        int index = 0;
        while (index < data->mLength) {
            if (*(stringData + index++) == ch) {
                return true;
            }  
        }
    }
    return false;
}

/*
    Compare the string with other QLatin1String. Comparison is Check is case sensitive.
*/
int HbString::compare(const QLatin1String &other)const
{
    HbStringData* data = getStringData(mMemoryType, mDataOffset, mShared);
    QChar *stringData = getAddress<QChar>(mMemoryType, data->mStartOffset, mShared);

    return compare_helper(stringData, data->mLength, other);
}

/*
    Reads a HbString from the QDataStream
*/
QDataStream& operator>>(QDataStream &in, HbString &str)
{
    GET_MEMORY_MANAGER(str.mMemoryType)
    char *sTemp;
    uint len;
    in.readBytes(sTemp, len);
    
    HbStringData* data = 0;
    if(str.mShared == true) {
        data = getStringData(HbMemoryManager::SharedMemory, str.mDataOffset);
    }
    else {
        data = getStringData(str.mMemoryType,str.mDataOffset);
    }

    if(len > 0){
        str.clear();
        data->mStartOffset = manager->alloc(len);

        char *dest =0;
        if(str.mShared == true) {
            dest = HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                    data->mStartOffset);
        } else {
            dest = HbMemoryUtils::getAddress<char>(str.mMemoryType, data->mStartOffset);
        }
        ::memcpy(dest, sTemp,len); 
        data->mLength = len/sizeof(QChar);
        data->mCapacity = data->mLength;
    }  
    delete[] sTemp;
    return in;
}

/*
    Writes the HbString to the QDataStream
*/
QDataStream& operator<<(QDataStream &out,const HbString &str)
{
    HbStringData* data = 0;
    if(str.mShared == true)
        data = getStringData(HbMemoryManager::SharedMemory, str.mDataOffset);
    else
        data = getStringData(str.mMemoryType, str.mDataOffset);
    if(data->mLength > 0){
        char *sTemp = 0;
        if(str.mShared == true) {
            sTemp = HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, 
                                                    data->mStartOffset);
        } else {
            sTemp = HbMemoryUtils::getAddress<char>(str.mMemoryType, data->mStartOffset);
        }
        out.writeBytes(sTemp,(uint)data->mLength*sizeof(QChar));
    }
    return out;
}
