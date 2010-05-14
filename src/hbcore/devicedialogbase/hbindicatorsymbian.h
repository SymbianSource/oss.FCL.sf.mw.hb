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
#ifndef HBINDICATORSYMBIAN_H
#define HBINDICATORSYMBIAN_H

#if defined(__SYMBIAN32__) || defined(SYMBIAN) || defined(HB_DOXYGEN)

#include <e32base.h>

class CHbIndicatorSymbianPrivate;
class CHbSymbianVariantMap;
class CHbSymbianVariant;

class MHbIndicatorSymbianObserver
{
public:
    virtual void IndicatorUserActivated(const TDesC& aType, CHbSymbianVariantMap& aData) = 0;
};

class CHbIndicatorSymbian : public CBase
{
public:
    enum TIndicatorError {
        EFrameworkErrors = 0x00000000,
        EPluginErrors = 0x10000000,
        EErrorTypeMask = 0xf0000000
    };
    
    IMPORT_C static CHbIndicatorSymbian* NewL();
    IMPORT_C ~CHbIndicatorSymbian();

    IMPORT_C TBool Activate(const TDesC& aIndicatorType, const CHbSymbianVariant* aParameter = 0);
    IMPORT_C TBool Deactivate(const TDesC& aIndicatorType, const CHbSymbianVariant* aParameter = 0);

    IMPORT_C TInt Error() const;
    IMPORT_C void SetObserver(MHbIndicatorSymbianObserver* aObserver);

private:
    CHbIndicatorSymbian();
    void ConstructL();

private:
    CHbIndicatorSymbianPrivate *d;
};

#endif // defined(__SYMBIAN32__) || defined(SYMBIAN) || defined(HB_DOXYGEN)

#endif // HBINDICATORSYMBIAN_H
