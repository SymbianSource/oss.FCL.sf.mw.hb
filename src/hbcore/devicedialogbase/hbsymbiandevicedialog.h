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

#ifndef HBSYMBIANDEVICEDIALOG_H
#define HBSYMBIANDEVICEDIALOG_H

#if defined(__SYMBIAN32__) || defined(SYMBIAN) || defined(HB_DOXYGEN)

#include <e32base.h>

class CHbDeviceDialogPrivate;
class CHbSymbianVariantMap;

class MHbDeviceDialogObserver
{
public:

    virtual void DataReceived(CHbSymbianVariantMap& aData) = 0;

    virtual void DeviceDialogClosed(TInt aCompletionCode) = 0;
};

class CHbDeviceDialog : public CBase
{
public:

    enum TDeviceDialogError{
        EFrameworkErrors = 0x00000000,
        EPluginErrors = 0x10000000,
        EErrorTypeMask = 0xf0000000,
        ECancelledError = 0x0fffffff,
        ESystemCancelledError = 0x0ffffffe
    };

    enum TDeviceDialogFlag{
        ENoDeviceDialogFlags = 0x0
    };

    IMPORT_C static CHbDeviceDialog* NewL(TInt aFlags = ENoDeviceDialogFlags);

    IMPORT_C ~CHbDeviceDialog();

    IMPORT_C TInt Show(const TDesC& aDeviceDialogType, const CHbSymbianVariantMap& aParameters, MHbDeviceDialogObserver* aObserver = NULL);

    IMPORT_C CHbSymbianVariantMap* ReceivedDataL() const;

    IMPORT_C TInt Update(const CHbSymbianVariantMap& aParameters);

    IMPORT_C void Cancel();

    IMPORT_C void SetObserver(MHbDeviceDialogObserver* aObserver);

private:

    CHbDeviceDialog(TInt aFlags = ENoDeviceDialogFlags);

    void ConstructL(TInt aFlags = ENoDeviceDialogFlags);

    CHbDeviceDialogPrivate* d;
    };

#endif // defined(__SYMBIAN32__) || defined(SYMBIAN) || defined(HB_DOXYGEN)

#endif // HBSYMBIANDEVICEDIALOG_H
