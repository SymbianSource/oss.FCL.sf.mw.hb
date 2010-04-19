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

#include <hbsymbianvariant.h>
#include <hbdevicedialogsymbian.h>
#include "hbdevicenotificationdialogsymbian.h"

_LIT(KKeyTimeOut, "timeout");
_LIT(KKeyIconName, "iconName");
_LIT(KKeyText, "text");
_LIT(KKeyTitle, "title");
_LIT(KKeyTouchActivation, "touchActivation");
_LIT(KKeyActivated, "result");
_LIT(KKeyActivatedValue, "activated");
_LIT(KPluginIdentifier, "com.nokia.hb.devicenotificationdialog/1.0");
_LIT(KKeyTitleTextWrapping, "titleTextWrapping");
_LIT(KKeyAnimationDefinition, "animationDefinition");

NONSHARABLE_CLASS(CHbDeviceNotificationDialogSymbianPrivate) : public CBase,
                                                  public MHbDeviceDialogObserver
{
public:
    CHbDeviceNotificationDialogSymbianPrivate();
    virtual ~CHbDeviceNotificationDialogSymbianPrivate();
    
public:
    void ShowL();
    void UpdateL();
    void Close();
    
    void AddVariantL(const TDesC& aKey, const TAny* aData,
            CHbSymbianVariant::TType aDataType);
    const CHbSymbianVariant* Variant(const TDesC& aKey) const;
    
public: // MHbDeviceDialogObserver
    void DataReceived(CHbSymbianVariantMap& aData);
    void DeviceDialogClosed(TInt aCompletionCode);

    void ConstructL(CHbDeviceNotificationDialogSymbian* aDialog);
    
public:
    MHbDeviceNotificationDialogObserver* iObserver;
    CHbDeviceDialogSymbian *iDeviceDialog;
    CHbSymbianVariantMap* iVariantMap;
    CHbDeviceNotificationDialogSymbian* q; 
    
    TBool iEnable;
    TInt iTimeout;
    CHbDeviceNotificationDialogSymbian::TextWrapping iWrap;
};

CHbDeviceNotificationDialogSymbianPrivate::CHbDeviceNotificationDialogSymbianPrivate()
    { 
    }

CHbDeviceNotificationDialogSymbianPrivate::~CHbDeviceNotificationDialogSymbianPrivate()
    {
    if (!iObserver && q->Timeout() != 0) {
        iDeviceDialog->SetObserver(NULL);
        }
    
    delete iDeviceDialog;
    delete iVariantMap;
    }

void CHbDeviceNotificationDialogSymbianPrivate::ConstructL(CHbDeviceNotificationDialogSymbian* aDialog)
    {
    q = aDialog;
    iDeviceDialog = CHbDeviceDialogSymbian::NewL();
    iVariantMap = CHbSymbianVariantMap::NewL(); 
    }

void CHbDeviceNotificationDialogSymbianPrivate::ShowL()
    {
    AddVariantL(KKeyTouchActivation, &iEnable, CHbSymbianVariant::EBool);
    AddVariantL(KKeyTimeOut, &iTimeout, CHbSymbianVariant::EInt);
    AddVariantL(KKeyTitleTextWrapping, &iWrap, CHbSymbianVariant::EInt);
    User::LeaveIfError(iDeviceDialog->Show(KPluginIdentifier, *iVariantMap, this));   
    }

void CHbDeviceNotificationDialogSymbianPrivate::UpdateL()
    {
    AddVariantL(KKeyTouchActivation, &iEnable, CHbSymbianVariant::EBool);
    AddVariantL(KKeyTimeOut, &iTimeout, CHbSymbianVariant::EInt);
    AddVariantL(KKeyTitleTextWrapping, &iWrap, CHbSymbianVariant::EInt);
    User::LeaveIfError(iDeviceDialog->Update(*iVariantMap));
    }

void CHbDeviceNotificationDialogSymbianPrivate::Close()
    {
    iDeviceDialog->Cancel();
    }

void CHbDeviceNotificationDialogSymbianPrivate::AddVariantL(
    const TDesC& aKey, 
    const TAny* aData,
    CHbSymbianVariant::TType aDataType)
    {
    CHbSymbianVariant *variant = CHbSymbianVariant::NewL(aData, aDataType);
    CleanupStack::PushL(variant);
    User::LeaveIfError(iVariantMap->Add(aKey, variant));
    CleanupStack::Pop(variant);
    }

const CHbSymbianVariant* CHbDeviceNotificationDialogSymbianPrivate::Variant(
    const TDesC& aKey) const
    {              
    return iVariantMap->Get(aKey);
    }

void CHbDeviceNotificationDialogSymbianPrivate::DataReceived(CHbSymbianVariantMap& aData)
    {    
    const CHbSymbianVariant* variant = aData.Get(KKeyActivated);
    if (variant && variant->IsValid())
        {
        TPtrC p = *variant->Value<TDesC>();
        bool result = p.Compare(KKeyActivatedValue()) == 0;
        if (iObserver && result)
            {
            // Observer callback may delete this object. Don't touch it after.
            iObserver->NotificationDialogActivated(q);
            }
        }    
    }

void CHbDeviceNotificationDialogSymbianPrivate::DeviceDialogClosed(TInt aCompletionCode)
    {
    if (iObserver)
        {
        // Observer callback may delete this object. Don't touch it after.
        iObserver->NotificationDialogClosed(q, aCompletionCode);
        }
    }

/*!
    \class MHbDeviceNotificationDialogObserver
    \brief MHbDeviceNotificationDialogObserver is an observer interface for observing CHbDeviceNotificationDialogSymbian.
*/

/*!
    \fn void MHbDeviceNotificationDialogObserver::NotificationDialogActivated(const CHbDeviceNotificationDialogSymbian* aDialog) = 0;

    This callback is called when notification dialog is activated.
    \a aDialog - Pointer to dialog instance which received activation event.
*/

/*!
   \fn void MHbDeviceNotificationDialogObserver::NotificationDialogClosed(const CHbDeviceNotificationDialogSymbian* aDialog, TInt aCompletionCode) = 0;

    This callback is called when notification dialog is closed.
    \a aDialog - Pointer to dialog instance which received closed event.
    \a aCompletionCode - Result of the dialog completion.
*/

/*!
   \fn void MHbDeviceNotificationDialogObserver::NotificationDialogError(const CHbDeviceNotificationDialogSymbian* aDialog, TInt aError) = 0;

    This callback is called when an error has occured.
    \a aDialog - Pointer to dialog instance which received closed event.
    \a aError - Error code.
*/

/*!
    \class CHbDeviceNotificationDialogSymbian
    \brief CHbDeviceNotificationDialogSymbian is a Symbian implementation of HbDeviceNotificationDialog.

    It is a client interface for s60 native application to show a non-modal dialog on top of applications when
    HbDeviceNotificationDialog can not be used.

    CHbDeviceNotificationDialogSymbian provides similar interface and functionality as HbDeviceNotificationDialog.
    Main difference is that signals are replaced by an observer interface CHbDeviceNotificationDialogSymbianObserver.

    For the content CHbDeviceNotificationDialogSymbian provides two rows of text, an image or an animation and for the usage
    same rules as for the HbDeviceNotificationDialog apply. Dialog is shown when show() is called. It is recommended that 
    the dialog data is initialized before calling ShowL() or UpdateL() methods, because those methods use interprocess communication.

    \code
    Following code snippet creates a device notification dialog containing title, text and icon.

    _LIT(KDialogText, "Dialog text");
    _LIT(KDialogTitle, "Dialog title");
    _LIT(KDialogIcon, "note_info.svg");

    CHbDeviceNotificationDialogSymbian* dialog = CHbDeviceNotificationDialogSymbian::NewL();
    CleanupStack::PushL(dialog);
    dialog->SetTextL(KDialogText);
    dialog->SetTitleL(KDialogTitle);
    dialog->SetIconNameL(KDialogIcon);
    dialog->Show();
    CleanupStack::PopAndDestroy(dialog);
    \endcode

    or equivalent dialog can be created using one of the provided convenience methods:

    \code
    _LIT(KDialogText, "Dialog text");
    _LIT(KDialogTitle, "Dialog title");
    _LIT(KDialogIcon, "note_info.svg");
    
    CHbDeviceNotificationDialogSymbian::Notification(KDialogIcon, KDialogText, KDialogTitle);
    \endcode

    When using convenience methods, it is not possible to receive user interaction events, because the 
    HbDeviceNotificationDialog instance is destroyed when the call returns. Ownership and handling of the
    dialog is transferred to HbDeviceDialog framework. Also dialog can not be closed nor updated.

    Below is an example of receiving user interaction events from device notification dialog. With following example 
    user is able to receive activated and close events. Note that dialog is cancelled, if it goes out of scope.

    \code
    _LIT(KDialogText, "Dialog text");
    _LIT(KDialogTitle, "Dialog title");
    _LIT(KDialogIcon, "note_info.svg");

    iDialog = CHbDeviceNotificationDialogSymbian::NewL(this);
    iDialog->SetTextL(KDialogText);
    iDialog->SetTitleL(KDialogTitle);
    iDialog->SetIconNameL(KDialogIcon);
    iDialog->ShowL();
    \endcode
    
    CHbDeviceNotificationDialogSymbian supports. 
    Supported formats are the following.

    - GIF (.gif)
    - MNG (.mng)
	- Frame animations
	
	There is a built-in support for GIF and MNG animations. 
	
	Frame animations can be created by following way:
		
	\code
	Create an animation definition file.	
	
	<animations>
		<icon name="frame_anim_looping" playmode="loop">
			<frame duration="100">c:\icon1.svg</frame>
			<frame duration="200">c:\icon2.svg</frame>
			<frame duration="300">c:\icon3.svg</frame>
		</icon>
	</animations>	
	
	Create CHbDeviceNotificationDialogSymbian in a way described before and
	set definition file and animation's logical name.
	
	_LIT(KAnimationDefinitionXML, "C:\animation.axml");	
	_LITK(KLogicalIconName, "frame_anim_looping");
		
	iDialog->SetAnimationDefinitionL(KAnimationDefinitionXML);
	iDialog->SetIconNameL(KIconName);
	iDialog->ShowL();		
	\endcode
	\sa HbIconAnimationManager::addDefinitionFile
	\note Animation definition files must be stored to a place where they
	can be accessed.    
    
    \proto
    \hbwidgets
*/

/*!
    Symbian two phase constructor. Returns a pointer to CHbDeviceNotificationDialogSymbian instance.
    \a aObserver Pointer to the observer.
*/
EXPORT_C CHbDeviceNotificationDialogSymbian* CHbDeviceNotificationDialogSymbian::NewL(
    MHbDeviceNotificationDialogObserver* aObserver)
    {
    CHbDeviceNotificationDialogSymbian* self = new (ELeave) CHbDeviceNotificationDialogSymbian();
    CleanupStack::PushL(self);
    self->ConstructL(aObserver);
    CleanupStack::Pop(self);
    return self;
    }

/*!
    Destructor.
*/
EXPORT_C CHbDeviceNotificationDialogSymbian::~CHbDeviceNotificationDialogSymbian()
    {
    delete d;
    }

/*!
    Convenience method for showing notification dialog with icon and title.
    \param aIconName - path and name of the icon shown on dialog.
    \param aTitle - title shown on dialog.
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::NotificationL(const TDesC& aIconName, const TDesC& aTitle)
    {
    CHbDeviceNotificationDialogSymbian* self = CHbDeviceNotificationDialogSymbian::NewL();
    CleanupStack::PushL(self);
    self->SetIconNameL(aIconName);
    self->SetTitleL(aTitle);
    self->ShowL();
    CleanupStack::PopAndDestroy(self);
    }

/*!
    Convenience method for showing notification dialog with icon, text and title.
    \param aIconName - path and name of the icon shown on dialog.
    \param aText - text shown on dialog.
    \param aTitle - title shown on dialog.
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::NotificationL(const TDesC &aIconName, const TDesC& aText, const TDesC& aTitle)
    {
    CHbDeviceNotificationDialogSymbian* self = CHbDeviceNotificationDialogSymbian::NewL();
    CleanupStack::PushL(self);
    self->SetIconNameL(aIconName);
    self->SetTextL(aText);
    self->SetTitleL(aTitle);
    self->ShowL();
    CleanupStack::PopAndDestroy(self);
    }

/*!
    Sets message box icon name or animation logical name. The dialog gets updated when method is called.

    \param aIconName Icon name. Icon can be from Hb resources or themes. Or can be a file in
    a file system.

    \sa IconName()
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::SetIconNameL(const TDesC& aIconName)
    {
    d->AddVariantL(KKeyIconName, &aIconName, CHbSymbianVariant::EDes);
    }


/*!
    Get name and path of the icon shown on dialog or animation's logical name. 
    If not set, returns empty iconName.

    \sa SetIconNameL
*/
EXPORT_C const TPtrC CHbDeviceNotificationDialogSymbian::IconName() const
    {    
    const CHbSymbianVariant* variant = d->Variant(KKeyIconName);
    if (variant && variant->IsValid())
        {
        return *variant->Value<TDesC>();
        }
    return TPtrC(KNullDesC());    
    }

/*!
    Sets notification dialog's animation definition name.  The dialogs get updated next when time ShowL() or UpdateL()
    is called.

    \param aAnimationDefinition Animation definition file name. Definition can be from Hb resources or themes. 
    Or can be a file in a file system.

    \sa AnimationDefinition()
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::SetAnimationDefinitionL(const TDesC& aAnimationDefinition)
{
	d->AddVariantL(KKeyAnimationDefinition, &aAnimationDefinition, CHbSymbianVariant::EDes);
}

/*!
    Returns an animation definition set for a dialog. If not set, return string.

    \sa SetAnimationDefinitionL()
*/
EXPORT_C TPtrC CHbDeviceNotificationDialogSymbian::AnimationDefinition() const
{
    const CHbSymbianVariant* variant = d->Variant(KKeyAnimationDefinition);
    if (variant && variant->IsValid())
        {
        return *variant->Value<TDesC>();
        }
    return TPtrC(KNullDesC());
}


/*!
    Set and update text on dialog. Text is not set, if Show() or Update()
    is not called.
    \param aText - text shown on dialog.
    \sa ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::SetTextL(const TDesC& aText)
    {
    d->AddVariantL(KKeyText, &aText, CHbSymbianVariant::EDes);
    }

/*!
    Get text shown on dialog. If not set, returns empty string.

    \sa SetTextL()
*/
EXPORT_C const TPtrC CHbDeviceNotificationDialogSymbian::Text() const
    {
    const CHbSymbianVariant* variant = d->Variant(KKeyText);
    if (variant && variant->IsValid())
        {
        return *variant->Value<TDesC>();
        }
    return TPtrC(KNullDesC());    
    }    

/*!
    Set and update title on dialog. Title is not set, if Show() or Update()
    is not called.
    \param title - aTitle shown on dialog.
    \sa ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::SetTitleL(const TDesC& aTitle)
    {
    d->AddVariantL(KKeyTitle, &aTitle, CHbSymbianVariant::EDes);
    }

/*!
    Get title shown on dialog. If not set, returns empty string.

    \sa SetTitleL()
*/
EXPORT_C const TPtrC CHbDeviceNotificationDialogSymbian::Title() const
    {
    TPtrC ptr = KKeyTitle();
    const CHbSymbianVariant* variant = d->Variant(KKeyTitle);
    if (variant && variant->IsValid())
        {
        return *variant->Value<TDesC>();
        }
    return TPtrC(KNullDesC());
    }
    
/*!
    \deprecated void CHbDeviceNotificationDialogSymbian::EnableTouchActivationL(TBool aEnable)
    is deprecated. Use CHbDeviceNotificationDialogSymbian::EnableTouchActivation(TBool aEnable)
    instead.
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::EnableTouchActivationL(TBool aEnable)
    {
    EnableTouchActivation(aEnable);
    }

/*!
    Enable user interaction on dialog. Setting is not set, if Show() or Update()
    is not called.
    \param aEnable - When enabled, activation event is notified via observer interface.

    Default value is false.
    \sa ShowL(), UpdateL(), MCHbDeviceNotificationDialogSymbianObserver.
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::EnableTouchActivation(TBool aEnable)
    {
    d->iEnable = aEnable;    
    }

/*!
    Get touch activation setting.

    \sa EnableTouchActivation()
*/
EXPORT_C TBool CHbDeviceNotificationDialogSymbian::IsTouchActivating() const
    {        
    return d->iEnable; 
    }

/*!
    \deprecated void CHbDeviceNotificationDialogSymbian::SetTimeoutL(TInt aTimeout)
    is deprecated. Use CHbDeviceNotificationDialogSymbian::SetTimeout(TInt aTimeout)
    instead.
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::SetTimeoutL(TInt aTimeout)
    {
    SetTimeout(aTimeout);       
    }

/*!
    Set dialog timeout. Timeout is not set, if Show() or Update()
    is not called.
    \param aTimeout - Set timeout for dialog.

    Default value is HbPopup::StandardTimeout (3000 ms).
    \sa ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::SetTimeout(TInt aTimeout)
    {
    d->iTimeout = aTimeout;
    }

/*!
    Get timeout setting.

    \sa SetTimeout()
*/
EXPORT_C TInt CHbDeviceNotificationDialogSymbian::Timeout() const
    {    
    return d->iTimeout;
    }

/*!
    \deprecated void CHbDeviceNotificationDialogSymbian::SetTitleTextWrappingL(
    TextWrapping aWrapping) is deprecated. Use CHbDeviceNotificationDialogSymbian::SetTitleTextWrapping(
    TextWrapping aWrapping) instead.
    instead.
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::SetTitleTextWrappingL(
    TextWrapping aWrapping)
    {
    SetTitleTextWrapping(aWrapping);
    } 

/*!
    Set dialog title text wrapping. Text wrapping is not set, if Show() or Update()
    is not called.
    \param aWrapping - Defines title text wrapping.

    Default value is CHbDeviceNotificationDialogSymbian::TextNoWrap.
    \sa ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::SetTitleTextWrapping(
    TextWrapping aWrapping)
    {
    d->iWrap = aWrapping;    
    }

/*!
    Get title text wrapping setting.

    \sa SetTitleTextWrapping()
*/
EXPORT_C CHbDeviceNotificationDialogSymbian::TextWrapping CHbDeviceNotificationDialogSymbian::TitleTextWrapping() const
    {    
    return d->iWrap;
    }
    
/*!
    Show the dialog.
    \code
    // example to show dialog.
    iNotificationDialog->SetTitleL("Dialog title text");
    iNotificationDialog->ShowL();
    \endcode
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::ShowL()
    {
    d->ShowL();
    }

/*!
    Update the dialog. Show must be called before update can be used.
    \code
    // example to update already showing dialog.
    iNotificationDialog->SetTitleL("Update title");
    iNotificationDialog->UpdateL();
    \endcode
*/

EXPORT_C void CHbDeviceNotificationDialogSymbian::UpdateL()
    {
    d->UpdateL();
    }

/*!
    Close the dialog. Method has no effect if convenience methods
    are used to show device notification dialog.
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::Close()
    {
    d->Close();
    }

/*!
    Constructor. 
*/
CHbDeviceNotificationDialogSymbian::CHbDeviceNotificationDialogSymbian()
    {
    }

/*!
    Symbian 2nd phase constructor. 
*/
void CHbDeviceNotificationDialogSymbian::ConstructL(
    MHbDeviceNotificationDialogObserver* aObserver)
    {
    d = new (ELeave) CHbDeviceNotificationDialogSymbianPrivate;
    d->ConstructL(this);
    d->iObserver = aObserver;
    EnableTouchActivation(EFalse);
    SetTimeout(KHbLongNotificationDialogTimeout);
    SetTitleTextWrapping(CHbDeviceNotificationDialogSymbian::TextWordWrap);
    }
