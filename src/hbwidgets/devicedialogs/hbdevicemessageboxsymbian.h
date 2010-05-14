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

#ifndef HBDEVICEMESSAGEBOXSYMBIAN_H
#define HBDEVICEMESSAGEBOXSYMBIAN_H

#if defined(__SYMBIAN32__) || defined(SYMBIAN) || defined(HB_DOXYGEN)

#include <e32base.h>

class CHbDeviceMessageBoxPrivate;
class MHbDeviceMessageBoxObserver;

class CHbDeviceMessageBoxSymbian : public CBase
{
public:
    enum TType {
        EInformation,
        EQuestion,
        EWarning
    };

    enum TButtonId {
        EInvalidButton = -1,
        EAcceptButton,
        ERejectButton
    };

public:

    IMPORT_C static CHbDeviceMessageBoxSymbian* NewL(TType aType = EInformation,
        MHbDeviceMessageBoxObserver *aObserver = 0);
    IMPORT_C ~CHbDeviceMessageBoxSymbian();

    IMPORT_C static TButtonId QuestionL(const TDesC& aText,
        const TDesC& aAcceptButtonText, const TDesC& aRejectButtonText);
    IMPORT_C static void InformationL(const TDesC& aText);
    IMPORT_C static void WarningL(const TDesC& aText);

    IMPORT_C void ShowL();
    IMPORT_C void UpdateL();
    IMPORT_C void Close();
    IMPORT_C TButtonId ExecL();

    IMPORT_C void SetTypeL(TType aType);
    IMPORT_C TType Type() const;

    IMPORT_C void SetTextL(const TDesC& aText);
    IMPORT_C const TPtrC Text() const;

    IMPORT_C void SetIconNameL(const TDesC& aIconName);
    IMPORT_C const TPtrC IconName() const;

    IMPORT_C void SetAnimationDefinitionL(const TDesC& aAnimationDefinition);
    IMPORT_C TPtrC AnimationDefinition() const;

    void SetIconVisible(TBool aVisible);
    TBool IconVisible() const;

    IMPORT_C void SetTimeout(TInt aTimeout);
    IMPORT_C TInt Timeout() const;

    IMPORT_C void SetDismissPolicy(TInt aHbPopupDismissPolicy);
    IMPORT_C TInt DismissPolicy() const;

    IMPORT_C void SetButtonTextL(TButtonId aButton, const TDesC& aText);
    IMPORT_C const TPtrC ButtonText(TButtonId aButton) const;

    IMPORT_C void SetButton(TButtonId aButton, TBool aEnable);
    IMPORT_C TBool HasButton(TButtonId aButton) const;

    IMPORT_C void SetObserver(MHbDeviceMessageBoxObserver *aObserver);

private:
    friend class CHbDeviceMessageBoxPrivate;
    CHbDeviceMessageBoxSymbian();
    CHbDeviceMessageBoxPrivate* d;
};

class MHbDeviceMessageBoxObserver
{
public:
    virtual void MessageBoxClosed(const CHbDeviceMessageBoxSymbian* aMessageBox,
        CHbDeviceMessageBoxSymbian::TButtonId aButton) = 0;
};

#endif // defined(__SYMBIAN32__) || defined(SYMBIAN) || defined(HB_DOXYGEN)
#endif // HBDEVICEMESSAGEBOXSYMBIAN_H
