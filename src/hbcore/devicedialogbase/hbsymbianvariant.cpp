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

#include "hbsymbianvariant.h"

class CHbSymbianVariantPrivate : public CBase
{
public:
    CHbSymbianVariantPrivate();
    ~CHbSymbianVariantPrivate();

    void InitializeDataL(const TAny* aData, CHbSymbianVariant::TType aDataType);

    void ResetData();

    void* Data();

public:

    struct Data {
        union BasicData {
        TInt i;
        TUint u;
        TReal r;
        TAny* ptr;
        } iBasicData;
    } iData;

    CHbSymbianVariant::TType iType;
    TBool iNull;
};

CHbSymbianVariantPrivate::CHbSymbianVariantPrivate() : iType(CHbSymbianVariant::EInt), iNull(ETrue)
{
    iData.iBasicData.i = 0;
}

CHbSymbianVariantPrivate::~CHbSymbianVariantPrivate()
{
    ResetData();
};

void CHbSymbianVariantPrivate::InitializeDataL(const TAny* aData, CHbSymbianVariant::TType aDataType)
{
    switch(aDataType) {
        case CHbSymbianVariant::EInt: {
            iData.iBasicData.i = *reinterpret_cast<const TInt*>(aData);
            break;
        }
        case CHbSymbianVariant::EBool: {
            iData.iBasicData.i = *reinterpret_cast<const TBool*>(aData);
            break;
        }
        case CHbSymbianVariant::EUint: {
            iData.iBasicData.u = *reinterpret_cast<const TUint*>(aData);
            break;
        }
        case CHbSymbianVariant::EReal: {
            iData.iBasicData.r = *reinterpret_cast<const TReal*>(aData);
            break;
        }
        case CHbSymbianVariant::EChar: {
            const TChar* chPtr = reinterpret_cast<const TChar*>(aData);
            TChar* ch = new TChar(*chPtr);
            iData.iBasicData.ptr = ch;
            break;
        }
        case CHbSymbianVariant::ERect: {
            const TRect* rectPtr = reinterpret_cast<const TRect*>(aData);
            TRect* rect = new TRect(rectPtr->iTl, rectPtr->iBr);
            iData.iBasicData.ptr = rect;
            break;
        }
        case CHbSymbianVariant::EDes: {
            const TDesC* desPtr = reinterpret_cast<const TDesC*>(aData);
            HBufC* buf = desPtr->Alloc();
            iData.iBasicData.ptr = buf;
            break;
        }
        case CHbSymbianVariant::EPoint: {
            const TPoint* pointPtr = reinterpret_cast<const TPoint*>(aData);
            TPoint* point = new TPoint(pointPtr->iX, pointPtr->iY);
            iData.iBasicData.ptr = point;
            break;
        }
        case CHbSymbianVariant::ESize: {
            const TSize* sizePtr = reinterpret_cast<const TSize*>(aData);
            TSize* size = new TSize(sizePtr->iWidth, sizePtr->iHeight);
            iData.iBasicData.ptr = size;
            break;
        }
        case CHbSymbianVariant::EBinary: {
            const TDesC8* binPtr = reinterpret_cast<const TDesC8*>(aData);
            HBufC8* binBuf = binPtr->Alloc();
            iData.iBasicData.ptr = binBuf;
            break;
        }
        case CHbSymbianVariant::EDesArray: {
            const MDesCArray* arrayPtr = reinterpret_cast<const MDesCArray*>(aData);
            TInt count = arrayPtr->MdcaCount();
            CDesC16ArrayFlat* array = new CDesC16ArrayFlat(16);
            for(TInt i = 0; i < count; i++) {
                array->AppendL(arrayPtr->MdcaPoint(i));
            }
            iData.iBasicData.ptr = array;
            break;
        }
        case CHbSymbianVariant::EVariantMap: {
            const CHbSymbianVariantMap* mapPtr = 
                reinterpret_cast<const CHbSymbianVariantMap*>(aData);
            CHbSymbianVariantMap* copy = CHbSymbianVariantMap::NewL();
            CleanupStack::PushL(copy);
            MDesCArray& keys = mapPtr->Keys();
            TInt count = keys.MdcaCount();
            for(TInt i = 0; i < count ; i++) {
                TPtrC ptr = keys.MdcaPoint(i);
                const CHbSymbianVariant* var = mapPtr->Get(ptr);
                copy->Add(ptr, CHbSymbianVariant::NewL(var->Data(), var->Type()));
            }
            iData.iBasicData.ptr = copy;
            CleanupStack::Pop(copy);
            break;
        }
        default: {
            User::Panic(_L("CHbSymbianVariant: Not supported type"),aDataType);
            break;
        }
    }
    iType = aDataType;
    iNull = EFalse;
}

void CHbSymbianVariantPrivate::ResetData()
{
    switch(iType) {
        case CHbSymbianVariant::EInt:
        case CHbSymbianVariant::EBool:
        case CHbSymbianVariant::EUint:
        case CHbSymbianVariant::EReal:
            // no need to destroy anything
            break;
        case CHbSymbianVariant::EChar: {
            TChar* del_ptr = static_cast<TChar*>(iData.iBasicData.ptr);
            delete del_ptr;
            iData.iBasicData.ptr = NULL;
            break;
            }
        case CHbSymbianVariant::EDes: {
            TDesC* del_ptr = static_cast<TDesC*>(iData.iBasicData.ptr);
            delete del_ptr;
            iData.iBasicData.ptr = NULL;
            break;
            }
        case CHbSymbianVariant::ERect: {
            TRect* del_ptr = static_cast<TRect*>(iData.iBasicData.ptr);
            delete del_ptr;
            iData.iBasicData.ptr = NULL;
            break;
            }
        case CHbSymbianVariant::EPoint: {
            TPoint* del_ptr = static_cast<TPoint*>(iData.iBasicData.ptr);
            delete del_ptr;
            iData.iBasicData.ptr = NULL;
            break;
            }
        case CHbSymbianVariant::ESize: {
            TSize* del_ptr = static_cast<TSize*>(iData.iBasicData.ptr);
            delete del_ptr;
            iData.iBasicData.ptr = NULL;
            break;
            }
        case CHbSymbianVariant::EBinary: {
            TDesC8* del_ptr = static_cast<TDesC8*>(iData.iBasicData.ptr);
            delete del_ptr;
            iData.iBasicData.ptr = NULL;
            break;
            }
        case CHbSymbianVariant::EDesArray: {
            CDesC16ArrayFlat* del_ptr = static_cast<CDesC16ArrayFlat*>(iData.iBasicData.ptr);
            delete del_ptr;
            iData.iBasicData.ptr = NULL;
            break; // dynamic data
            }
        default:
            break;
    };
}

void* CHbSymbianVariantPrivate::Data() {

    void* return_value;

    switch(iType) {
    case CHbSymbianVariant::EInt:
        return_value = &iData.iBasicData.i;
        break;
    case CHbSymbianVariant::EBool:
        return_value = &iData.iBasicData.i;
        break;
    case CHbSymbianVariant::EUint:
        return_value = &iData.iBasicData.u;
        break;
    case CHbSymbianVariant::EReal:
        return_value = &iData.iBasicData.r;
        break;
    case CHbSymbianVariant::EChar:
    case CHbSymbianVariant::EDes:
    case CHbSymbianVariant::ERect:
    case CHbSymbianVariant::EPoint:
    case CHbSymbianVariant::ESize:
    case CHbSymbianVariant::EBinary:
    case CHbSymbianVariant::EDesArray:
    case CHbSymbianVariant::EVariantMap:
        return_value = iData.iBasicData.ptr;
        break; // dynamic data
    default:
        return_value = NULL;
        break;
    }
    return return_value;
};

/*!
 \class CHbSymbianVariant
 \brief CHbSymbianVariant implements QVariant for Symbian.

 It supports part of the data types supported in QVariant:
 QString (TDesC)
 QChar (TChar)
 QPoint(TPoint)
 QRect(TRect)
 QSize(TSize)
 QByteArray(TDesC8)
 QStringList (MDesCArray)

 The data passed to the constructor is copied to the CHbSymbianVariant.

 \sa CHbSymbianVariantMap, CHbDeviceDialog
 \stable

 \hbcore

 */

/*!
   \enum CHbSymbianVariant::TType
   Data types the CHbSymbianVariant can contain
*/
/*!
   \var CHbSymbianVariant::EInt
   a TInt
*/
/*!
    \var CHbSymbianVariant::EBool
    a TBool
*/
/*!
    \var CHbSymbianVariant::EUint
    a TUint
*/
/*!
    \var CHbSymbianVariant::EReal
    a TReal
*/
/*!
    \var CHbSymbianVariant::ERect
    a TRect
*/
/*!
    \var CHbSymbianVariant::EPoint
    a TPoint
*/
/*!
    \var CHbSymbianVariant::ESize
    a TSize
*/
/*!
    \var CHbSymbianVariant::EChar
    a TChar
*/
/*!
    \var CHbSymbianVariant::EDes
    a TDesC
*/
/*!
    \var CHbSymbianVariant::EBinary
    a TDesC8
*/
/*!
    \var CHbSymbianVariant::EDesArray
    a MDesCArray
*/
/*!
    \var CHbSymbianVariant::EVariantMap
    a symbian variant map
*/

 /*!
    \fn T * CHbSymbianVariant::Value()

    Template class to get variants value casted to the correct pointer type. See the
    CHbSymbianVariant::TType for the supported classes.

    \sa CHbSymbianVariant::TType
*/

/*!
    Creates a new CHbSymbianVariant. Note that if CHbSymbianVariant::EDesArray
    is used the \aData must be casted to MDesCArray pointer when passed to the method.
*/

EXPORT_C CHbSymbianVariant* CHbSymbianVariant::NewL(const TAny* aData, TType aDataType)
{
    CHbSymbianVariant* variant = new (ELeave) CHbSymbianVariant(aData, aDataType);
    CleanupStack::PushL(variant);
    variant->ConstructL(aData, aDataType);
    CleanupStack::Pop(variant); //variant
    return variant;
}


/*!
    Returns the data type stored in CHbSymbianVariant

    \sa CHbSymbianVariant::TType
*/
EXPORT_C CHbSymbianVariant::TType CHbSymbianVariant::Type() const
{
    return d->iType;
}

/*!
    Get TAny pointer to the CHbSymbianVariant data.
    The CHbSymbianVariant::Value() should be used instead of this method.

    Ownership is not changed.
*/
EXPORT_C TAny* CHbSymbianVariant::Data() const
{
    return d->Data();
}

/*!
    Set new data to CHbSymbianVariant. Old data is deleted.

    Ownership is not changed.
*/
EXPORT_C void CHbSymbianVariant::SetData(TAny* aData, TType aDataType)
{
    if(d) {
        d->ResetData();
        d->InitializeDataL(aData, aDataType);
    }
}
/*!
    Is the the data in CHbSymbianVariant valid. data is valid if the variant
    is created with valid data and data type
*/
EXPORT_C TBool CHbSymbianVariant::IsValid() const
{
    if(d && !(d->iNull))
        return ETrue;
    else
        return EFalse;
}

EXPORT_C CHbSymbianVariant::~CHbSymbianVariant()
{
    delete d;
    d = 0;
}

CHbSymbianVariant::CHbSymbianVariant(const TAny* /* aData */, TType /* aDataType */) : d(0)
{
}

void CHbSymbianVariant::ConstructL(const TAny* aData, TType aDataType)
{
    d = new (ELeave) CHbSymbianVariantPrivate;
    d->InitializeDataL(aData, aDataType);
}

class CHbSymbianVariantMapPrivate : public CBase
{
public:
    CHbSymbianVariantMapPrivate():iKeyArray(NULL),iVariantArray() {
        iKeyArray = new CDesC16ArrayFlat(16);
    };
    ~CHbSymbianVariantMapPrivate(){
        delete iKeyArray;
        iKeyArray = NULL;
        iVariantArray.ResetAndDestroy();
        iVariantArray.Close();
    };

public:
    CDesC16Array* iKeyArray;
    RPointerArray<CHbSymbianVariant> iVariantArray;
};

/*!
  \class CHbSymbianVariantMap
  \brief CHbSymbianVariantMap is a container for CHbSymbianVariant objects.

  It replicates QVariantMap functionality to Symbian native programming side.

  \sa CHbSymbianVariant, CHbDeviceDialog
  \stable
*/

/*!
  Create a new CHbSymbianVariantMap. Map is empty after creation and
  ready for use.
 */
EXPORT_C CHbSymbianVariantMap* CHbSymbianVariantMap::NewL()
{
    CHbSymbianVariantMap* map = new (ELeave) CHbSymbianVariantMap();
    CleanupStack::PushL(map);
    map->ConstructL();
    CleanupStack::Pop(map); // map
    return map;
}

EXPORT_C CHbSymbianVariantMap::~CHbSymbianVariantMap()
{
    delete d;
    d = 0;
}

/*!
 Adds a key-value pairing to the map. Takes ownership of the variant,
 key is copied. If key is already in the map, the value connected to the key
 is replaced with the new value. the old value is deleted.
 */
EXPORT_C TInt CHbSymbianVariantMap::Add(const TDesC& aKey, CHbSymbianVariant* aVariant)
{
    TInt pos = KErrNotFound;
    TInt result = d->iKeyArray->Find(aKey, pos,ECmpNormal);

    // there is already a key-value pairing in the map
    if((result == KErrNone) && (pos >= 0)) {

        // for some reason directly deleting the pointer and
        // assigning a new one to position does not work
        CHbSymbianVariant* remove = d->iVariantArray[pos];
        d->iVariantArray.Remove(pos);

        d->iVariantArray.Insert(aVariant, pos);

        delete remove;
        remove = 0;
    }
    // new key-value pair
    else {
        result = KErrNone;
        TRAP(result, d->iKeyArray->AppendL(aKey));
        if(result == KErrNone){
            result = d->iVariantArray.Append(aVariant);
            if(result) {
                d->iKeyArray->Delete(d->iKeyArray->MdcaCount()-1);
            }
        }
    }
    return result;
}
/*!
 Deletes value mapped to the key given as parameter \a aKey

 On error returns possible Symbian error code.
 */
EXPORT_C TInt CHbSymbianVariantMap::Delete(const TDesC& aKey)
{
    TInt pos = 0;
    TInt result = d->iKeyArray->Find(aKey, pos,ECmpNormal);
    if((result == KErrNone) && (pos >= 0)) {
        // first remove the key so no-one can find the key when value is deleted
        d->iKeyArray->Delete(pos);
        delete d->iVariantArray[pos];
        d->iVariantArray[pos] = 0;
        d->iVariantArray.Remove(pos);
    }
    return result;
}

/*!
  Gets value mapped to the key. CHbSymbianVariantMap keeps the ownership of the object.
  Returns NULL if not found.
 */
EXPORT_C const CHbSymbianVariant* CHbSymbianVariantMap::Get(const TDesC& aKey) const
{
    TInt pos = 0;
    CHbSymbianVariant* variant = 0;
    TInt result = d->iKeyArray->Find(aKey, pos,ECmpNormal);
    if((result == KErrNone) && (pos >= 0))  {
        variant = d->iVariantArray[pos];
    }
    return variant;
}

/*!
 Get all keys in the map. CHbSymbianVariantMap keeps the ownership of the object.
 */
EXPORT_C MDesCArray& CHbSymbianVariantMap::Keys() const
{
    return *(d->iKeyArray);
}

CHbSymbianVariantMap::CHbSymbianVariantMap()
{

}

void CHbSymbianVariantMap::ConstructL()
{
    d = new CHbSymbianVariantMapPrivate();
}

