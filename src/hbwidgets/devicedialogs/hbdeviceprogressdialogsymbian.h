/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
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

#ifndef HBDEVICEPROGRESSDIALOGSYMBIAN_H
#define HBDEVICEPROGRESSDIALOGSYMBIAN_H

#if defined(__SYMBIAN32__) || defined(SYMBIAN) || defined(HB_DOXYGEN)

#include <e32base.h>

class CHbDeviceProgressDialogSymbianPrivate;
class MHbDeviceProgressDialogObserver;

class CHbDeviceProgressDialogSymbian : public CBase
{
public:

    enum TType {
        EProgressDialog,
        EWaitDialog
    };

    static IMPORT_C CHbDeviceProgressDialogSymbian* NewL(TType aType = EProgressDialog,
            MHbDeviceProgressDialogObserver* aObserver = 0);
    IMPORT_C virtual ~CHbDeviceProgressDialogSymbian();

    IMPORT_C void ShowL();
    IMPORT_C void UpdateL();
    IMPORT_C void Close();

    IMPORT_C void SetRange(TInt aMin, TInt aMax);

    IMPORT_C void SetMaximum(TInt aMax);
    IMPORT_C TInt Maximum() const;

    IMPORT_C void SetMinimum(TInt aMin);
    IMPORT_C TInt Minimum() const;

    IMPORT_C void SetProgressValue(TInt aProgressValue);
    IMPORT_C TInt ProgressValue() const;

    IMPORT_C void SetAutoClose(TBool aAutoClose);
    IMPORT_C TBool AutoClose() const;

    IMPORT_C void SetProgressType(TType aType);
    IMPORT_C TType ProgressType() const;

    IMPORT_C void SetTextL(const TDesC& aText);
    IMPORT_C const TPtrC Text() const;

    IMPORT_C void SetIconNameL(const TDesC& aIconName);
    IMPORT_C const TPtrC IconName() const;

    IMPORT_C void SetAnimationDefinitionL(const TDesC& aAnimationDefinition);
    IMPORT_C TPtrC AnimationDefinition() const;

    IMPORT_C void SetButtonTextL(const TDesC& aText);
    IMPORT_C const TPtrC ButtonText() const;

    IMPORT_C void SetButton(TBool aEnable);
    IMPORT_C TBool HasButton() const;

    IMPORT_C void SetObserver(MHbDeviceProgressDialogObserver* aObserver);
private:

    CHbDeviceProgressDialogSymbian();
    void ConstructL(TType aType);
    CHbDeviceProgressDialogSymbianPrivate* d;
    friend class CHbDeviceProgressDialogSymbianPrivate;
};

class MHbDeviceProgressDialogObserver
{
public:
    virtual void ProgressDialogCancelled(const CHbDeviceProgressDialogSymbian* aProgressDialog) = 0;
    virtual void ProgressDialogClosed(const CHbDeviceProgressDialogSymbian* aProgressDialog) = 0;
};

#endif // defined(__SYMBIAN32__) || defined(SYMBIAN) || defined(HB_DOXYGEN)

#endif // HBDEVICEPROGRESSDIALOGSYMBIAN_H
