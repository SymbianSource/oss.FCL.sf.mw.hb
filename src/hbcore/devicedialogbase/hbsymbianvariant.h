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

#ifndef HBSYMBIANVARIANT_H
#define HBSYMBIANVARIANT_H

#if defined(__SYMBIAN32__) || defined(SYMBIAN) || defined(HB_DOXYGEN)

#include <e32base.h>
#include <badesca.h>

class CHbSymbianVariantPrivate;
class CHbSymbianVariantMapPrivate;

class CHbSymbianVariant
#if !defined(HB_DOXYGEN)
: public CBase
#endif // !defined(HB_DOXYGEN)
{
public:

    enum TType{
        EInt, EBool, EUint, EReal, ERect, EPoint, ESize, EChar,
        EDes, EBinary, EDesArray, EVariantMap
    };

    IMPORT_C static CHbSymbianVariant* NewL(const TAny* aData, TType aDataType);
    IMPORT_C virtual ~CHbSymbianVariant();

    IMPORT_C CHbSymbianVariant::TType Type() const;

    template<typename T>
        T* Value() const {
            if(IsValid() ) {
                T* value = reinterpret_cast<T*>(Data());
                return value;
                }
            else return 0;
        }

    IMPORT_C TAny* Data() const;
    IMPORT_C void SetData(TAny* aData, TType aDataType);
    IMPORT_C TBool IsValid() const;

private:
    CHbSymbianVariant(const TAny* aData, TType aDataType);
    void ConstructL(const TAny* aData, TType aDataType);
    CHbSymbianVariant(const CHbSymbianVariant& aNativeVariant);

private:
    CHbSymbianVariantPrivate* d;
};

class CHbSymbianVariantMap
#if !defined(HB_DOXYGEN)
: public CBase
#endif // !defined(HB_DOXYGEN)
{
public:
    IMPORT_C static CHbSymbianVariantMap* NewL();
    IMPORT_C virtual ~CHbSymbianVariantMap();

    IMPORT_C TInt Add(const TDesC& aKey, CHbSymbianVariant* aVariant);
    IMPORT_C TInt Delete(const TDesC& aKey);
    IMPORT_C const CHbSymbianVariant* Get(const TDesC& aKey) const;
    IMPORT_C MDesCArray& Keys() const;

private:
    CHbSymbianVariantMap();
    void ConstructL();

    CHbSymbianVariantMapPrivate* d;
};

#endif // defined(__SYMBIAN32__) || defined(SYMBIAN) || defined(HB_DOXYGEN)

#endif // HBSYMBIANVARIANT_H
