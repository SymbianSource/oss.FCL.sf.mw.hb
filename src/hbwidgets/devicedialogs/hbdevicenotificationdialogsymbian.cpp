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
_LIT(KKeyShowLevel, "showLevel");

NONSHARABLE_CLASS(CHbDeviceNotificationDialogSymbianPrivate) : public CBase,
                                                  public MHbDeviceDialogObserver
{
public:
    struct TIntProperty{
        TInt iValue;
        TBool iChanged;
    };

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
    
    TIntProperty iEnable;
    TIntProperty iTimeout;
    TIntProperty iShowLevel;
    TIntProperty iWrap;
};

CHbDeviceNotificationDialogSymbianPrivate::CHbDeviceNotificationDialogSymbianPrivate()
    {
    iTimeout.iValue = 3000; // HbPopup::StandardTimeout
    }

CHbDeviceNotificationDialogSymbianPrivate::~CHbDeviceNotificationDialogSymbianPrivate()
    {
    if (!iObserver && q->Timeout() != 0) {
        if (iDeviceDialog) {
            iDeviceDialog->SetObserver(NULL);
        }
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
    if (iEnable.iChanged) {
        AddVariantL(KKeyTouchActivation, &iEnable.iValue, CHbSymbianVariant::EBool);
    }
    if (iTimeout.iChanged) {
        AddVariantL(KKeyTimeOut, &iTimeout.iValue, CHbSymbianVariant::EInt);
    }
    if (iWrap.iChanged) {
        AddVariantL(KKeyTitleTextWrapping, &iWrap.iValue, CHbSymbianVariant::EInt);
    }
    if (iShowLevel.iChanged) {
        AddVariantL(KKeyShowLevel, &iShowLevel.iValue, CHbSymbianVariant::EInt);
    }

    TInt error = iDeviceDialog->Show(KPluginIdentifier, *iVariantMap, this);
    if (error != KErrNone) {
        User::Leave(error); // error can be positive or negative
    }
    }

void CHbDeviceNotificationDialogSymbianPrivate::UpdateL()
    {
    if (iEnable.iChanged) {
        AddVariantL(KKeyTouchActivation, &iEnable, CHbSymbianVariant::EBool);
    }
    if (iTimeout.iChanged) {
        AddVariantL(KKeyTimeOut, &iTimeout, CHbSymbianVariant::EInt);
    }
    if (iWrap.iChanged) {
        AddVariantL(KKeyTitleTextWrapping, &iWrap, CHbSymbianVariant::EInt);
    }
    TInt error = iDeviceDialog->Update(*iVariantMap);
    if (error != KErrNone) {
        User::Leave(error); // error can be positive or negative
    }
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

// Set dialog show level
void SetShowLevel(CHbDeviceNotificationDialogSymbianPrivate *aDialog, TInt aLevel)
{
    aDialog->iShowLevel.iValue = aLevel;
    aDialog->iShowLevel.iChanged = ETrue;
}

/*!
    \class MHbDeviceNotificationDialogObserver
    \brief MHbDeviceNotificationDialogObserver is an observer interface for observing CHbDeviceNotificationDialogSymbian.
*/
/*!
    \fn void MHbDeviceNotificationDialogObserver::NotificationDialogActivated(const CHbDeviceNotificationDialogSymbian* aDialog) = 0;

    This callback is called when notification dialog is activated.
    \a aDialog is a pointer to a dialog instance which received activation event.

    \sa CHbDeviceNotificationDialogSymbian::EnableTouchActivation()
*/
/*!
   \fn void MHbDeviceNotificationDialogObserver::NotificationDialogClosed(const CHbDeviceNotificationDialogSymbian* aDialog, TInt aCompletionCode) = 0;

    This callback is called when notification dialog has closed.
    \a aDialog is a pointer to a dialog instance which received closed event.
    \a aCompletionCode can be either Symbian error code or device dialog error code.
*/

/*!
    \stable
    \hbwidgets

    \class CHbDeviceNotificationDialogSymbian
    \brief CHbDeviceNotificationDialogSymbian is a Symbian implementation of HbDeviceNotificationDialog.

    <b>This class is Symbian only. Not available on other platforms.</b>

    CHbDeviceNotificationDialogSymbian is intended for use by servers that don't run Qt event loop
    and cannot use HbDeviceNotificationDialog.

    See HbDeviceNotificationDialog documentation to find out more about device notification
    dialogs.

    CHbDeviceNotificationDialogSymbian provides similar interface and functionality as
    HbDeviceNotificationDialog. Main difference is that signals are replaced by an observer
    interface CHbDeviceNotificationDialogSymbianObserver.

    An asynchronous ShowL() method launches a dialog. Device dialog framework decides when the
    dialog is actually shown. There may be a delay untill dialog appears on display. Close()
    closes a dialog.

    After dialog has been lauched, properties may be updated by setters and calling UpdateL().
    It is recommended that dialog properties are set before calling ShowL() as updating them after
    causes interprocess communication.

    If there is no need to update or receive activation from a launched dialog,
    CHbDeviceNotificationDialogSymbian object can be deleted after ShowL() returns. Device
    dialog framework takes care of displaying the dialog.

    For maintaining consistent look and feel, two timeout constants are provided for setting
    dialog timeout: KHbShortNotificationDialogTimeout and KHbLongNotificationDialogTimeout.
    The first one is equivalent to HbPopup::ConfirmationNoteTimeout and the latter 
    to HbPopup::StandardTimeout.

    \code
    Following code snippet creates a device notification dialog containing title, text and icon.

    _LIT(KDialogText, "Dialog text");
    _LIT(KDialogTitle, "Dialog title");
    _LIT(KDialogIcon, "qtg_large_info");

    CHbDeviceNotificationDialogSymbian* dialog = CHbDeviceNotificationDialogSymbian::NewL();
    CleanupStack::PushL(dialog);
    dialog->SetTextL(KDialogText);
    dialog->SetTitleL(KDialogTitle);
    dialog->SetIconNameL(KDialogIcon);
    dialog->ShowL();
    CleanupStack::PopAndDestroy(dialog);
    \endcode

    or equivalent dialog can be created using one of the provided convenience methods:

    \code
    _LIT(KDialogText, "Dialog text");
    _LIT(KDialogTitle, "Dialog title");
    _LIT(KDialogIcon, "qtg_large_info");
    
    CHbDeviceNotificationDialogSymbian::Notification(KDialogIcon, KDialogText, KDialogTitle);
    \endcode

    When using convenience methods, it is not possible to receive user interaction events,
    because the HbDeviceNotificationDialog instance is destroyed when the call returns.

    Below is an example of receiving user interaction events from device notification dialog.
    With following example user is able to receive activated and close events.
    Note that in this case the dialog is closed by device dialog framework if
    CHbDeviceNotificationDialogSymbian object is deleted.

    \code
    _LIT(KDialogText, "Dialog text");
    _LIT(KDialogTitle, "Dialog title");
    _LIT(KDialogIcon, "qtg_large_info");

    class DialogObserver : public MHbDeviceNotificationDialogObserver
    {
    public:
        DialogObserver() {}
        ~DialogObserver() { delete iDialog; }
        void ShowDialog();
    ...
    private:
        virtual void NotificationDialogActivated(const CHbDeviceNotificationDialogSymbian* aDialog);
        virtual void NotificationDialogClosed(const CHbDeviceNotificationDialogSymbian* aDialog, TInt aCompletionCode);
    private:
        CHbDeviceNotificationDialogSymbian* iDialog;
    };
    
    void DialogObserver::NotificationDialogActivated(const CHbDeviceNotificationDialogSymbian* aDialog)
    {
        CEikonEnv::Static()->InfoMsg(_L("Device notification dialog activated"));
        delete aDialog;
        aDialog = 0;
    }
    
    void NotificationDialogClosed(const CHbDeviceNotificationDialogSymbian* aDialog, TInt aCompletionCode)
    {
        CEikonEnv::Static()->InfoMsg(_L("Device notification dialog deactivated"));
        delete aDialog;
        aDialog = 0;
    }
    
    void DialogObserver::ShowDialog()
    {
        iDialog = CHbDeviceNotificationDialogSymbian::NewL(this);
        iDialog->SetTextL(KDialogText);
        iDialog->SetTitleL(KDialogTitle);
        iDialog->SetIconNameL(KDialogIcon);
        iDialog->ShowL();
    }
    \endcode

    Following example sets an icon animation into a notification dialog.
        
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
    _LIT(KLogicalIconName, "frame_anim_looping");
        
    iDialog->SetAnimationDefinitionL(KAnimationDefinitionXML);
    iDialog->SetIconNameL(KLogicalIconName);
    iDialog->ShowL();       
    \endcode

    \sa HbDeviceNotificationDialog, MHbDeviceNotificationDialogObserver 
*/

/*!
    \enum CHbDeviceNotificationDialogSymbian::TextWrapping
    Title text wrapping.

    \sa Hb::TextWrapping
*/
/*!
    \var CHbDeviceNotificationDialogSymbian::TextWrapping CHbDeviceNotificationDialogSymbian::TextNoWrap
    No wrapping.
*/
/*!
    \var CHbDeviceNotificationDialogSymbian::TextWrapping CHbDeviceNotificationDialogSymbian::TextWordWrap
    Word wrapping.
*/
/*!
    \var CHbDeviceNotificationDialogSymbian::TextWrapping CHbDeviceNotificationDialogSymbian::TextWrapAnywhere
    Wrap anywhere.
*/

/*!
    Constructs a new CHbDeviceNotificationDialog and returns a pointer it.
    \a aObserver is a pointer to event observer or 0 if no observer.
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
    Destructs CHbDeviceNotificationDialogSymbian. The notification launched by ShowL()
    is closed if observer is set. If there is no observer, it is
    left executing and closes itself by timeout.
*/
EXPORT_C CHbDeviceNotificationDialogSymbian::~CHbDeviceNotificationDialogSymbian()
    {
    delete d;
    }

/*!
    Convenience method for showing notification dialog with icon and title.

    \param aIconName Path and name of the icon to show on dialog.
    \param aTitle Title text.
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

    \param aIconName Path and name of icon to show on dialog.
    \param aText Dialog text.
    \param aTitle Title text.
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
    Sets dialog icon name or animation logical name.

    \param aIconName Icon name. Icon can be from Hb resources or themes. Or can be a file in
    a file system.

    \sa IconName(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::SetIconNameL(const TDesC& aIconName)
    {
    d->AddVariantL(KKeyIconName, &aIconName, CHbSymbianVariant::EDes);
    }


/*!
    Returns icon or animation name. Default is empty.

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
    Sets dialog animation definition file name.

    Supported icon animation formats are following:
    - GIF (.gif)
    - MNG (.mng)
    - Frame animations

    \param aAnimationDefinition Animation definition file name. Definition can be from Hb resources or themes.
    Or can be a file in a file system. The definition must be stored to a place where it can be accessed by
    device dialog service.

    \sa AnimationDefinition() SetIconNameL() HbIconAnimationManager::addDefinitionFile(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::SetAnimationDefinitionL(const TDesC& aAnimationDefinition)
{
    d->AddVariantL(KKeyAnimationDefinition, &aAnimationDefinition, CHbSymbianVariant::EDes);
}

/*!
    Returns dialog animation definition file name.

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
    Sets dialog text.

    \param aText Dialog text.

    \sa Text(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::SetTextL(const TDesC& aText)
    {
    d->AddVariantL(KKeyText, &aText, CHbSymbianVariant::EDes);
    }

/*!
    Returns dialog text.

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
    Sets title text.

    \param aTitle Title text.

    \sa Title(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::SetTitleL(const TDesC& aTitle)
    {
    d->AddVariantL(KKeyTitle, &aTitle, CHbSymbianVariant::EDes);
    }

/*!
    Returns title text.

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
    Enables user interaction on dialog.

    \param aEnable True enables activation event notification via observer interface.

    \sa IsTouchActivating(), ShowL(), UpdateL(), MCHbDeviceNotificationDialogSymbianObserver
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::EnableTouchActivation(TBool aEnable)
    {
    d->iEnable.iValue = aEnable;
    d->iEnable.iChanged = ETrue;
    }

/*!
    Returns touch activation setting.  Default value is false.

    \sa EnableTouchActivation()
*/
EXPORT_C TBool CHbDeviceNotificationDialogSymbian::IsTouchActivating() const
    {        
    return d->iEnable.iValue; 
    }

/*!
    Set dialog timeout. 

    \param aTimeout Dialog timeout in milliseconds.

    \sa Timeout(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::SetTimeout(TInt aTimeout)
    {
    d->iTimeout.iValue = aTimeout;
    d->iTimeout.iChanged = ETrue;
    }

/*!
    Returns timeout. Default value is KHbLongNotificationDialogTimeout (3000 ms).

    \sa SetTimeout()
*/
EXPORT_C TInt CHbDeviceNotificationDialogSymbian::Timeout() const
    {    
    return d->iTimeout.iValue;
    }

/*!
    Sets title text wrapping. The title can wrap only if there is no text for the dialog.
    The title can wrap to a maximum of two lines. SetTextL() also changes title text wrapping.


    \param aWrapping Title text wrapping.

    \sa TitleTextWrapping(), SetTextL(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::SetTitleTextWrapping(
    TextWrapping aWrapping)
    {
    d->iWrap.iValue = aWrapping;
    d->iWrap.iChanged = ETrue;
    }

/*!
    Returns title text wrapping.

    The title can wrap only if dialog text is empty. The title can wrap to a maximum of two lines.
    The default is CHbDeviceNotificationDialogSymbian::TextWordWrap.

    \sa SetTitleTextWrapping()
*/
EXPORT_C CHbDeviceNotificationDialogSymbian::TextWrapping CHbDeviceNotificationDialogSymbian::TitleTextWrapping() const
    {
    if (d->iWrap.iChanged) {
        return static_cast<TextWrapping>(d->iWrap.iValue);
    } else {
        return Text().Length() == 0 ? TextWordWrap : TextNoWrap;
    }
    }
    
/*!
    Shows a notification dialog and returns immediately without waiting for it to close.
    Closing of the dialog and tapping of dialog is indicated by MHbDeviceNotificationDialogObserver
    callback interface. Dialog can be updated while showing by property setters and then calling
    UpdateL().

    \sa UpdateL(), Close(), MHbDeviceNotificationDialogObserver
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::ShowL()
    {
    d->ShowL();
    }

/*!
    Updates changed properties to a showing notification dialog via interprocess
    communication. Has no effect if ShowL() has not been called or the dialog has
    closed already.

    \sa ShowL()
*/

EXPORT_C void CHbDeviceNotificationDialogSymbian::UpdateL()
    {
    d->UpdateL();
    }

/*!
    Closes a device notification dialog.
*/
EXPORT_C void CHbDeviceNotificationDialogSymbian::Close()
    {
    d->Close();
    }

/*!
    Constructs CHbDeviceNotificationDialogSymbian.
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
    }
