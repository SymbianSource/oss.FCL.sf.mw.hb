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

#ifndef HBDEVICENOTIFICATIONDIALOGSYMBIAN_H
#define HBDEVICENOTIFICATIONDIALOGSYMBIAN_H

#if defined(__SYMBIAN32__) || defined(SYMBIAN) || defined(HB_DOXYGEN)

#include <e32base.h>

const TInt KHbShortNotificationDialogTimeout = 1500; // HbPopup::ConfirmationNoteTimeout
const TInt KHbLongNotificationDialogTimeout = 3000;  // HbPopup::StandardTimeout

class CHbDeviceNotificationDialogSymbian;
class CHbDeviceNotificationDialogSymbianPrivate;

class MHbDeviceNotificationDialogObserver  
{
public:
    virtual void NotificationDialogActivated(const CHbDeviceNotificationDialogSymbian* aDialog) = 0;
    virtual void NotificationDialogClosed(const CHbDeviceNotificationDialogSymbian* aDialog, TInt aCompletionCode) = 0;
};

class CHbDeviceNotificationDialogSymbian
#if !defined(HB_DOXYGEN)
: public CBase
#endif // !defined(HB_DOXYGEN)
{
public:
    /*!
        A type definition for text wrapping
    */
    enum TextWrapping
    {
        TextNoWrap = 0,
        TextWordWrap = 1,
        TextWrapAnywhere = 3
    };
    
    IMPORT_C static CHbDeviceNotificationDialogSymbian* NewL(
            MHbDeviceNotificationDialogObserver* aObserver = 0);
    IMPORT_C virtual ~CHbDeviceNotificationDialogSymbian();

    IMPORT_C static void NotificationL(const TDesC& aIconName, const TDesC& aTitle);
    IMPORT_C static void NotificationL(const TDesC& aIconName, const TDesC& aText, const TDesC& aTitle);

    IMPORT_C void SetIconNameL(const TDesC& aIconName);
    IMPORT_C const TPtrC IconName() const;
    
    IMPORT_C void SetAnimationDefinitionL(const TDesC& aAnimationDefinition);
    IMPORT_C TPtrC AnimationDefinition() const;
    
    IMPORT_C void SetTextL(const TDesC& aText);
    IMPORT_C const TPtrC Text() const;    
    
    IMPORT_C void SetTitleL(const TDesC& aTitle);
    IMPORT_C const TPtrC Title() const;
    
    IMPORT_C void EnableTouchActivation(TBool aEnable);
    IMPORT_C TBool IsTouchActivating() const;
        
    IMPORT_C void SetTimeout(TInt aTimeout);
    IMPORT_C TInt Timeout() const;
    
    IMPORT_C void SetTitleTextWrapping(TextWrapping aWrapping); 
    IMPORT_C TextWrapping TitleTextWrapping() const;
    
    IMPORT_C void ShowL();
    IMPORT_C void UpdateL();
    IMPORT_C void Close();

private:
    CHbDeviceNotificationDialogSymbian();
    void ConstructL(MHbDeviceNotificationDialogObserver* aObserver);
    
    CHbDeviceNotificationDialogSymbianPrivate* d;
    friend class CHbDeviceNotificationDialogSymbianPrivate;
    friend class HbDeviceDialogsExtensionSymbian;
};

#endif // (__SYMBIAN32__) || defined(SYMBIAN) || defined(HB_DOXYGEN)

#endif // HBDEVICENOTIFICATIONDIALOGSYMBIAN_H
