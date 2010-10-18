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

#include <hbdevicedialogsymbian.h>
#include <hbsymbianvariant.h>
#include <hbdeleteguardsymbian_p.h>
#include "hbdeviceprogressdialogsymbian.h"

_LIT(KDialogType, "progressDialogType");
_LIT(KDialogMaximum, "maximum");
_LIT(KDialogMinimum, "minimum");
_LIT(KDialogValue, "value");
_LIT(KDialogAutoClose, "autoClose");
_LIT(KDialogText, "text");
_LIT(KDialogIconName, "iconName");
_LIT(KCancelAction, "cancelAction");
_LIT(KPluginIdentifier, "com.nokia.hb.deviceprogressdialog/1.0");
_LIT(KAnimationDefinition, "animationDefinition");
_LIT(KShowLevel, "showLevel");

NONSHARABLE_CLASS(CHbDeviceProgressDialogSymbianPrivate) : public CBase,
                                                           public MHbDeviceDialogObserver
{
public:
    CHbDeviceProgressDialogSymbianPrivate();
    virtual ~CHbDeviceProgressDialogSymbianPrivate();

    void ShowL();
    void UpdateL();
    void Close();

    bool Visible() const;

    void AddVariantL(const TDesC& aKey, const TAny* aData,
            CHbSymbianVariant::TType aDataType, CHbSymbianVariantMap* map);
    const CHbSymbianVariant* Variant(const TDesC& aKey) const;
    static HBufC *CreateActionDataLC(TBool aNull, const TDesC &text);

public: // MHbDeviceDialogObserver
    void DataReceived(CHbSymbianVariantMap& aData);
    void DeviceDialogClosed(TInt aCompletionCode);

public: // data

    void ConstructL(CHbDeviceProgressDialogSymbian* aDialog);

    MHbDeviceProgressDialogObserver* iObserver;
    CHbDeviceDialogSymbian *iDeviceDialog;
    CHbSymbianVariantMap* iVariantMap;
    CHbDeviceProgressDialogSymbian* q;

    TBool iVisible;

    TInt iMinimum;
    TInt iMaximum;
    TBool iAutoClose;
    TInt iType;
    TInt iProgressValue;
    RBuf iText;
    RBuf iIconName;
    RBuf iAnimationDefinition;
    TInt iShowLevel;
    struct {
        RBuf iText;
        struct {
            bool iTextValid:1; // property value has has been set
            bool iTextModified:1; // property has been modified
            bool iIsNull:1; // button is null
            bool iIsNullValid:1;
            bool iIsNullModified:1;
            bool iPressed:1; // true is button was pressed
        } iFlags;
    } iButton;
    bool *iDeleted;
};

CHbDeviceProgressDialogSymbianPrivate::CHbDeviceProgressDialogSymbianPrivate()
    {
    }

CHbDeviceProgressDialogSymbianPrivate::~CHbDeviceProgressDialogSymbianPrivate()
    {
    iText.Close();
    iIconName.Close();
    iAnimationDefinition.Close();
    iButton.iText.Close();
    delete iDeviceDialog;
    delete iVariantMap;
    // Set object deleted flag
    if (iDeleted)
        {
        // Mark the object as deleted.
        *iDeleted = true;
        iDeleted = 0;
        }
    }

void CHbDeviceProgressDialogSymbianPrivate::ConstructL(
    CHbDeviceProgressDialogSymbian* aDialog)
    {
    q = aDialog;
    iDeviceDialog = CHbDeviceDialogSymbian::NewL();
    iVariantMap = CHbSymbianVariantMap::NewL();
    iText.Create(KNullDesC);
    iIconName.Create(KNullDesC);
    iAnimationDefinition.Create(KNullDesC);
    iButton.iText.Create(KNullDesC);
    }

void CHbDeviceProgressDialogSymbianPrivate::ShowL()
    {
    AddVariantL(KDialogType, &iType, CHbSymbianVariant::EInt, iVariantMap);
    AddVariantL(KDialogMaximum, &iMaximum, CHbSymbianVariant::EInt, iVariantMap);
    AddVariantL(KDialogMinimum, &iMinimum, CHbSymbianVariant::EInt, iVariantMap);
    AddVariantL(KDialogAutoClose, &iAutoClose, CHbSymbianVariant::EBool, iVariantMap);
    AddVariantL(KDialogValue, &iProgressValue, CHbSymbianVariant::EInt, iVariantMap);
    if (iShowLevel) { // send only if non-default
        AddVariantL(KShowLevel, &iShowLevel, CHbSymbianVariant::EInt, iVariantMap);
    }

    if (iText.Length() > 0)
        {
        AddVariantL(KDialogText, &iText, CHbSymbianVariant::EDes, iVariantMap);
        }
    if (iIconName.Length() > 0)
        {
        AddVariantL(KDialogIconName, &iIconName, CHbSymbianVariant::EDes, iVariantMap);
        }
    if (iAnimationDefinition.Length() > 0)
        {
        AddVariantL(KAnimationDefinition, &iAnimationDefinition, CHbSymbianVariant::EDes, iVariantMap);
        }
    if (iButton.iFlags.iTextValid || iButton.iFlags.iIsNullValid)
        {
        iButton.iFlags.iTextModified = false;
        iButton.iFlags.iIsNullModified = false;
        HBufC *actionData = CreateActionDataLC(iButton.iFlags.iIsNull, iButton.iText);
        AddVariantL(KCancelAction, actionData, CHbSymbianVariant::EDes, iVariantMap);
        CleanupStack::PopAndDestroy(); // actionData
        }
    iButton.iFlags.iPressed = false;
    TInt error = iDeviceDialog->Show(KPluginIdentifier, *iVariantMap, this);
    if (error != KErrNone) {
        User::Leave(error); // error can be positive or negative
    }
    iVisible = ETrue;
    }

void CHbDeviceProgressDialogSymbianPrivate::Close()
    {
    iDeviceDialog->Cancel();
    }

void CHbDeviceProgressDialogSymbianPrivate::UpdateL()
    {
    CHbSymbianVariantMap* map = CHbSymbianVariantMap::NewL();
    CleanupStack::PushL(map);

    const CHbSymbianVariant* variant = Variant(KDialogMaximum);
    if (variant && *variant->Value<TInt>() != iMaximum)
        {
        AddVariantL(KDialogMaximum, &iMaximum, CHbSymbianVariant::EInt, iVariantMap);
        AddVariantL(KDialogMaximum, &iMaximum, CHbSymbianVariant::EInt, map);
        }

    variant = Variant(KDialogMinimum);
    if (variant && *variant->Value<TInt>() != iMinimum)
        {
        AddVariantL(KDialogMinimum, &iMinimum, CHbSymbianVariant::EInt, iVariantMap);
        AddVariantL(KDialogMinimum, &iMinimum, CHbSymbianVariant::EInt, map);
        }

    variant = Variant(KDialogAutoClose);
    if (variant && *variant->Value<TBool>() != iAutoClose)
        {
        AddVariantL(KDialogAutoClose, &iAutoClose, CHbSymbianVariant::EInt, iVariantMap);
        AddVariantL(KDialogAutoClose, &iAutoClose, CHbSymbianVariant::EInt, map);
        }

    variant = Variant(KDialogType);
    if (variant && *variant->Value<TInt>() != iType)
        {
        AddVariantL(KDialogType, &iType, CHbSymbianVariant::EInt, iVariantMap);
        AddVariantL(KDialogType, &iType, CHbSymbianVariant::EInt, map);
        }

    variant = Variant(KDialogValue);
    if (variant && *variant->Value<TInt>() != iProgressValue)
        {
        AddVariantL(KDialogValue, &iProgressValue, CHbSymbianVariant::EInt, iVariantMap);
        AddVariantL(KDialogValue, &iProgressValue, CHbSymbianVariant::EInt, map);
        }

    variant = Variant(KDialogText);
    if (variant && iText == *variant->Value<TDesC>())
        {
        AddVariantL(KDialogText, &iText, CHbSymbianVariant::EDes, iVariantMap);
        AddVariantL(KDialogText, &iText, CHbSymbianVariant::EDes, map);
        }

    variant = Variant(KDialogIconName);
    if (variant && iIconName == *variant->Value<TDesC>())
        {
        AddVariantL(KDialogIconName, &iIconName, CHbSymbianVariant::EDes, iVariantMap);
        AddVariantL(KDialogIconName, &iIconName, CHbSymbianVariant::EDes, map);
        }

    variant = Variant(KAnimationDefinition);
    if (variant && iAnimationDefinition == *variant->Value<TDesC>())
        {
        AddVariantL(KAnimationDefinition, &iAnimationDefinition, CHbSymbianVariant::EDes, iVariantMap);
        AddVariantL(KAnimationDefinition, &iAnimationDefinition, CHbSymbianVariant::EDes, map);
        }

    if (iButton.iFlags.iTextModified || iButton.iFlags.iIsNullModified)
        {
        iButton.iFlags.iTextModified = false;
        iButton.iFlags.iIsNullModified = false;
        HBufC *actionData = CreateActionDataLC(iButton.iFlags.iIsNull, iButton.iText);
        AddVariantL(KCancelAction, actionData, CHbSymbianVariant::EDes, iVariantMap);
        CleanupStack::PopAndDestroy(); // actionData
        }

    TInt error = iDeviceDialog->Update(*map);
    CleanupStack::PopAndDestroy(map);
    map = 0;
    if (error != KErrNone) {
        User::Leave(error); // error can be positive or negative
    }
    }

void CHbDeviceProgressDialogSymbianPrivate::DataReceived(CHbSymbianVariantMap& aData)
    {
    _LIT(KKey, "act");
    const CHbSymbianVariant* variant = aData.Get(KKey);
    if (variant)
        {
        const TDesC *value = variant->Value<const TDesC>();
        _LIT(KCancel, "c");
        if (value && *value == KCancel)
            {
            iButton.iFlags.iPressed = true;
            if (iObserver)
                {
                iObserver->ProgressDialogCancelled(q);
                // Don't touch this object anymore, observer callback may have deleted it.
                }
            }
        }
    }

void CHbDeviceProgressDialogSymbianPrivate::DeviceDialogClosed(TInt /*aCompletionCode*/)
    {
    // Send closed signal if the device dialog has been actually shown, as it is possible
    // to create and cancel a device dialog without showing it at all.
    if (iVisible)
        {
        iVisible = EFalse;
        if (iObserver)
            {
            iObserver->ProgressDialogClosed(q);
            // Don't touch this object anymore, observer callback may have deleted it.
            }
        }
    }

void CHbDeviceProgressDialogSymbianPrivate::AddVariantL(
    const TDesC& aKey,
    const TAny* aData,
    CHbSymbianVariant::TType aDataType,
    CHbSymbianVariantMap* map
    )
    {
    CHbSymbianVariant *variant = CHbSymbianVariant::NewL(aData, aDataType);
    CleanupStack::PushL(variant);
    User::LeaveIfError(map->Add(aKey, variant));
    CleanupStack::Pop(variant);
    }

const CHbSymbianVariant* CHbDeviceProgressDialogSymbianPrivate::Variant(const TDesC& aKey) const
    {
    return iVariantMap->Get(aKey);
    }

// Pack into a string a data for cancel button
HBufC *CHbDeviceProgressDialogSymbianPrivate::CreateActionDataLC(TBool aNull, const TDesC &text)
{
    HBufC *actionData;
    if (aNull)
        {
        actionData = HBufC::NewL(0);
        }
    else
        {
        _LIT(KtextTag, "t:");
        actionData = HBufC::NewL(text.Length() + KtextTag().Length());
        TPtr ptr = actionData->Des();
        ptr.Append(KtextTag);
        ptr.Append(text);
        }
    CleanupStack::PushL(actionData);
    return actionData;
}

// Set dialog show level
void SetShowLevel(CHbDeviceProgressDialogSymbianPrivate *aDialog, TInt aLevel)
{
    aDialog->iShowLevel = aLevel;
}

/*!
    \class MHbDeviceProgressDialogObserver
    \brief MHbDeviceProgressDialogObserver is an observer interface for CHbDeviceProgressDialogSymbian.

    \sa CHbDeviceProgressDialogSymbian
*/

/*!
    \fn void MHbDeviceProgressDialogObserver::ProgressDialogCancelled(const CHbDeviceProgressDialogSymbian* aDialog) = 0

    This callback is called when the device progress dialog is closed by user pressing the "cancel" button.

    \param aDialog Pointer to dialog that was cancelled.
*/

/*!
   \fn void MHbDeviceProgressDialogObserver::ProgressDialogClosed(const CHbDeviceProgressDialogSymbian* aDialog) = 0

    This callback is called when a device progress dialog is has closed. The closing may
    be a result of Close() being called, a auto-closing dialog has reached
    its maximum value or user pressing cancel button. It is not called if Close() is
    called before ShowL().

    \param aDialog Pointer to dialog instance that has closed.
*/

/*!
    \stable
    \hbwidgets

    \class CHbDeviceProgressDialogSymbian
    \brief CHbDeviceProgressDialogSymbian is a Symbian implementation of HbDeviceProgressDialog.

    <b>This class is Symbian only. Not available on other platforms.</b>

    CHbDeviceProgressDialogSymbian is intended for use by servers that don't run Qt event loop
    and cannot use HbDeviceProgressDialog.

    Device progress dialog is a modal dialog. It is shown on top of applications by device dialog
    server. CHbDeviceProgressDialogSymbian is a client of the server.

    To learn more about usage of device progress dialogs see HbDeviceProgressDialog
    documentation.

    CHbDeviceProgressDialogSymbian provides similar interface and functionality as
    HbDeviceProgressDialog. Main difference is that signals are replaced by observer
    interface MHbDeviceProgressDialogObserver. Also parameters of methods using
    Qt enumerations are changed to TInt.

    Showing of a progress dialog is always asynchronous as an application needs to update the dialog
    according to current progress.

    ShowL() displays a progress dialog asynchronously. The function returns immediately.
    A new dialog is lauched every time ShowL() is called. The launched
    dialog can be updated by setters and then calling UpdateL(). Closing and cancellation
    can be observed by a callbacks. Because each UpdateL() after the ShowL() requires
    interprocess communication, it's advisable to fully construct the dialog before displaying it.

    Below is an example of using a device wait dialog:

    \code
    _LIT(KConnectText, "Connecting...");
    CHbDeviceProgressDialogSymbian *iDialog = CHbDeviceProgressDialogSymbian::NewL(CHbDeviceProgressDialogSymbian::EWaitDialog);
    iDialog->SetTextL(KConnectText);
    iDialog->ShowL();
    \endcode

    Below is an example of using a device progress dialog:

    \code
    CHbDeviceProgressDialogSymbian* iDialog = CHbDeviceProgressDialogSymbian::NewL();
    _LIT(KUpdatingText, "Updating...");
    iDialog->SetTextL(KUpdatingText);
    iDialog->ShowL();

    iDialog->SetMinimum(1);
    iDialog->SetMaximum(76);
    iDialog->SetProgressValue(25);
    iDialog->UpdateL();
    \endcode

    An example of showing an icon animation:

    \code
    Create an animation definition file.

    <animations>
        <icon name="frame_anim_looping" playmode="loop">
            <frame duration="100">c:\icon1.svg</frame>
            <frame duration="200">c:\icon2.svg</frame>
            <frame duration="300">c:\icon3.svg</frame>
        </icon>
    </animations>

    Create CHbDeviceProgressDialogSymbian in a way described before and
    set definition file and animation's logical name.

    _LIT(KAnimationDefinitionXML, "C:\animation.axml");
    _LITK(KLogicalIconName, "frame_anim_looping");

    iDialog->SetAnimationDefinitionL(KAnimationDefinitionXML);
    iDialog->SetIconNameL(KIconName);
    iDialog->ShowL();
    \endcode

    \sa MHbDeviceProgressDialogObserver, HbDeviceProgressDialog, HbDeviceDialog
*/
/*!
    \enum CHbDeviceProgressDialogSymbian::TType
    Progress dialog types.

    \sa HbProgressDialog::ProgressDialogType
*/
/*!
    \var CHbDeviceProgressDialogSymbian::TType CHbDeviceProgressDialogSymbian::EProgressDialog
    Progress dialog.
*/
/*!
    \var CHbDeviceProgressDialogSymbian::TType CHbDeviceProgressDialogSymbian::EWaitDialog
    Wait dialog.
*/

/*!
    Constructs a new CHbDeviceNotificationDialogSymbian and returns a pointer it. 

    \param aType Must be one of the defined CHbDeviceProgressDialogSymbian::TType enumerations.
    Default value is CHbDeviceProgressDialogSymbian::EProgressDialog.
    \param aObserver Pointer to event observer. 0 if no observer.
*/
EXPORT_C CHbDeviceProgressDialogSymbian* CHbDeviceProgressDialogSymbian::NewL(
    TType aType,
    MHbDeviceProgressDialogObserver* aObserver)
    {
    CHbDeviceProgressDialogSymbian* self = new (ELeave) CHbDeviceProgressDialogSymbian();
    CleanupStack::PushL(self);
    self->ConstructL(aType);
    self->SetObserver(aObserver);
    CleanupStack::Pop(self);
    return self;
    }

/*!
    Destructs CHbDeviceProgressDialogSymbian. Closes also the dialog widget launched by
    ShowL().
*/
EXPORT_C CHbDeviceProgressDialogSymbian::~CHbDeviceProgressDialogSymbian()
    {
    delete d;
    }

/*!
    Shows a device progress dialog asyncronously. Function launches the dialog and returns
    immediately. Closing of the dialog can be observer with MHbDeviceProgressDialogObserver.
    CHbDeviceProgressDialogSymbian object can be used to launch several dialogs. A new one
    is launched every time ShowL() is called. Observer receives events for the latest dialog
    launched.

    \sa SetObserver(), UpdateL()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::ShowL()
    {
    d->ShowL();
    }

/*!
    Updates a device progress dialog asyncronously. Dialog that was launched with a lates ShowL()
    is updated. Properties that were modified since the last ShowL() or UpdateL() are sent to
    device dialog server.

    \sa ShowL()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::UpdateL()
    {
    d->UpdateL();
    }

/*!
    Closes a device progress dialog.
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::Close()
    {
    d->Close();
    }

/*!
    Sets progress bar minimum and maximum values.

    \param aMin Progress bar minimum value.
    \param aMax Progress bar maximum value.

    \sa Minimum(), Maximum(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetRange(TInt aMin, TInt aMax)
    {
    SetMinimum(aMin);
    SetMaximum(aMax);
    }

/*!
    Sets progress bar maximum value.

    \param aMax Progress bar maximum value.

    \sa Maximum(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetMaximum(TInt aMax)
    {
    // Don't allow wait dialog to set max/min other than zero as wait
    // animation bar doesn't work in that case.
    d->iMaximum = d->iType == EWaitDialog ? 0 : aMax;
    }

/*!
    Returns progress bar maximum value. Default value is 100.

    \sa SetMaximum()
*/
EXPORT_C TInt CHbDeviceProgressDialogSymbian::Maximum() const
    {
    return d->iMaximum;
    }

/*!
    Sets progress bar minimum value.

    \param aMin Progress bar minimum value.

    \sa Minimum(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetMinimum(TInt aMin)
    {
    // Don't allow wait dialog to set max/min other than zero as wait
    // animation bar doesn't work in that case.
    d->iMinimum = d->iType == EWaitDialog ? 0 : aMin;
    }

/*!
    Returns progress bar minimum value. Default value is 0.

    \sa SetMinimumL()
*/
EXPORT_C TInt CHbDeviceProgressDialogSymbian::Minimum() const
    {
    return d->iMinimum;
    }

/*!
    Sets dialog auto-closing.

    \param aAutoClose When set, the dialog is closed when value of the progress bar reaches
    the maximum value of the progress bar.

    \sa AutoClose(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetAutoClose(TBool aAutoClose)
    {
    d->iAutoClose = aAutoClose;
    }

/*!
    Returns dialog auto-closing property. Default value is true for progress dialog and false
    for wait dialog.

    \sa SetAutoCloseL()
*/
EXPORT_C TBool CHbDeviceProgressDialogSymbian::AutoClose() const
    {
    return d->iAutoClose;
    }

/*!
    Sets dialog type. Dialog properties are set to default values. After setting type,
    ShowL() must be called to launch a new dialog.

    \param aType Dialog type, wait or progress,

    \sa ProgressType(), ShowL()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetProgressType(
    CHbDeviceProgressDialogSymbian::TType aType)
    {
    d->iType = aType;

    if (aType == EWaitDialog)
        {
        SetRange(0,0);
        SetAutoClose(false);
        }
    else
        {
        SetRange(0,100);
        SetAutoClose(true);
        }
    }

/*!
    Returns dialog's type.

    \sa SetProgressType()
*/
EXPORT_C CHbDeviceProgressDialogSymbian::TType CHbDeviceProgressDialogSymbian::ProgressType() const
    {
    return static_cast<TType>(d->iType);
    }

/*!
    Sets progress bar value.

    \param aProgressValue Progress bar value.

    \sa ProgressValue(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetProgressValue(TInt aProgressValue)
    {
    d->iProgressValue = aProgressValue;
    }

/*!
    Returns progress bar value.

    \sa SetProgressValue()
*/
EXPORT_C TInt CHbDeviceProgressDialogSymbian::ProgressValue() const
    {
    return d->iProgressValue;
    }

/*!
    Sets dialog text.

    \param aText Dialog text.

    \sa Text(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetTextL(const TDesC& aText)
    {
    TInt length = aText.Length();
    if (length > d->iText.MaxLength())
        {
        d->iText.ReAllocL(length);
        }
    d->iText.Copy(aText);
    }

/*!
    Returns dialog text.

    \sa SetTextL()
*/
EXPORT_C const TPtrC CHbDeviceProgressDialogSymbian::Text() const
    {
    return d->iText;
    }

/*!
    Sets dialog icon name or animation logical name.

    \param aIconName Icon or animation name. Icon can be from Hb resources or themes. Or it can be a file in
    a file system.

    \sa IconName(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetIconNameL(const TDesC& aIconName)
    {
    TInt length = aIconName.Length();
    if (length > d->iIconName.MaxLength())
        {
        d->iIconName.ReAllocL(length);
        }
    d->iIconName.Copy(aIconName);
    }

/*!
    Returns name and path of icon or animation.

    \sa SetIconNameL()
*/
EXPORT_C const TPtrC CHbDeviceProgressDialogSymbian::IconName() const
    {
    return d->iIconName;
    }

/*!
    Sets dialog animation definition name.

    Supported icon animation formats are following:
    - GIF (.gif)
    - MNG (.mng)
    - Frame animations

    \param aAnimationDefinition Animation definition file name. Definition can be from Hb resources or themes.
    Or can be a file in a file system. The definition must be stored to a place where it can be accessed by
    device dialog service.

    \sa AnimationDefinition() SetIconNameL() HbIconAnimationManager::addDefinitionFile(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetAnimationDefinitionL(const TDesC& aAnimationDefinition)
    {
    TInt length = aAnimationDefinition.Length();
    if (length > d->iAnimationDefinition.MaxLength())
        {
        d->iAnimationDefinition.ReAllocL(length);
        }
    d->iAnimationDefinition.Copy(aAnimationDefinition);
    }

/*!
    Returns dialog icon animation definition name.

    \sa SetAnimationDefinitionL()
*/
EXPORT_C TPtrC CHbDeviceProgressDialogSymbian::AnimationDefinition() const
    {
    return d->iAnimationDefinition;
    }

/*!
    Sets dialog button text.

    \param aText Button text.

    \sa ButtonText(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetButtonTextL(const TDesC& aText)
{
    TInt length = aText.Length();
    if (length > d->iButton.iText.MaxLength()) {
        d->iButton.iText.ReAllocL(length);
    }
    d->iButton.iText.Copy(aText);
    d->iButton.iFlags.iTextValid = true;
    d->iButton.iFlags.iTextModified = true;
}

/*!
    Returns dialog button text.

    \sa SetButtonTextL()
*/
EXPORT_C const TPtrC CHbDeviceProgressDialogSymbian::ButtonText() const
{
    return d->iButton.iText;
}

/*!
    Sets progress dialog button presence.

    \param aEnable True enables (makes visible) the dialog button.

    \sa HasButton(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetButton(TBool aEnable)
{
    d->iButton.iFlags.iIsNull = !aEnable;
    d->iButton.iFlags.iIsNullValid = true;
    d->iButton.iFlags.iIsNullModified = true;
}

/*!
    Returns dialog button presence.

    \sa SetButton()
*/
EXPORT_C TBool CHbDeviceProgressDialogSymbian::HasButton() const
{
    return !d->iButton.iFlags.iIsNull;
}

/*!
    Sets dialog observer. The observer is called when the dialog is closed or cancelled.

    \param aObserver Pointer to observer.

    \sa MHbDeviceProgressDialogObserver
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetObserver(MHbDeviceProgressDialogObserver* aObserver)
    {
    d->iObserver = aObserver;
    }

/*!
    Constructs CHbDeviceProgressDialogSymbian.
*/
CHbDeviceProgressDialogSymbian::CHbDeviceProgressDialogSymbian()
    {
    }

/*
    2nd Constructor.
*/
void CHbDeviceProgressDialogSymbian::ConstructL(TType aType)
    {
    d = new (ELeave) CHbDeviceProgressDialogSymbianPrivate;
    d->ConstructL(this);
    SetProgressType(aType);
    SetProgressValue(0);
    }
