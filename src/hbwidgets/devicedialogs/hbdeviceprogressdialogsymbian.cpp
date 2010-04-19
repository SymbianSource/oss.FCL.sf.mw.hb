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
_LIT(KDialogTextAlign, "textAlignment");
_LIT(KDialogIconAlign, "iconAlignment");
_LIT(KPrimaryActionText, "primaryActionText");
_LIT(KPrimaryActionNull, "primaryActionNull");
_LIT(KPluginIdentifier, "com.nokia.hb.deviceprogressdialog/1.0");
_LIT(KAnimationDefinition, "animationDefinition");

const TInt KAlignCenterLeft = 0x0080 | 0x0001;  //Qt::AlignVCenter | Qt::AlignLeft
const TInt KAlignCenter = 0x0080 | 0x0004;      //Qt::AlignCenter

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
    TInt iTextAlign;
    TInt iIconAlign;
    TBool iWrap;
    RBuf iText;
    RBuf iIconName;
    RBuf iAnimationDefinition;
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
    AddVariantL(KDialogTextAlign, &iTextAlign, CHbSymbianVariant::EInt, iVariantMap);
    AddVariantL(KDialogIconAlign, &iIconAlign, CHbSymbianVariant::EInt, iVariantMap);

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
        TBool isNull = iButton.iFlags.iIsNull;
        if (isNull)
            {
            AddVariantL(KPrimaryActionNull, &isNull, CHbSymbianVariant::EBool, iVariantMap);
            }
        else
            {
            AddVariantL(KPrimaryActionText, &iButton.iText, CHbSymbianVariant::EDes, iVariantMap);
            }
        }
    iButton.iFlags.iPressed = false;
    User::LeaveIfError(iDeviceDialog->Show(KPluginIdentifier, *iVariantMap, this));

    iVisible = ETrue;
    }

void CHbDeviceProgressDialogSymbianPrivate::Close()
    {
    iDeviceDialog->Cancel();
    }

void CHbDeviceProgressDialogSymbianPrivate::UpdateL()
    {
    CHbSymbianVariantMap* map = CHbSymbianVariantMap::NewL();

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

    variant = Variant(KDialogTextAlign);
    if (variant && *variant->Value<TInt>() != iTextAlign)
        {
        AddVariantL(KDialogTextAlign, &iTextAlign, CHbSymbianVariant::EInt, iVariantMap);
        AddVariantL(KDialogTextAlign, &iTextAlign, CHbSymbianVariant::EInt, map);
        }

    variant = Variant(KDialogIconAlign);
    if (variant && *variant->Value<TInt>() != iIconAlign)
        {
        AddVariantL(KDialogIconAlign, &iIconAlign, CHbSymbianVariant::EInt, iVariantMap);
        AddVariantL(KDialogIconAlign, &iIconAlign, CHbSymbianVariant::EInt, map);
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
        TBool isNull = iButton.iFlags.iIsNull;
        if (isNull)
            {
            AddVariantL(KPrimaryActionNull, &isNull, CHbSymbianVariant::EBool, iVariantMap);
            }
        else
            {
            AddVariantL(KPrimaryActionText, &iButton.iText, CHbSymbianVariant::EDes, iVariantMap);
            }
        }

    User::LeaveIfError(iDeviceDialog->Update(*map));
    delete map;
    map = 0;
    }

void CHbDeviceProgressDialogSymbianPrivate::DataReceived(CHbSymbianVariantMap& aData)
    {
    _LIT(KKey, "act");
    const CHbSymbianVariant* variant = aData.Get(KKey);
    if (variant)
        {
        const TDesC *value = variant->Value<const TDesC>();
        _LIT(KPrimary, "p");
        if (value && *value == KPrimary)
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

/*!
    \class MHbDeviceProgressDialogObserver
    \brief MHbDeviceProgressDialogObserver is an observer interface for observing CHbDeviceProgressDialogSymbian.
*/

/*!
    \fn void MHbDeviceProgressDialogObserver::ProgressDialogCancelled(const CHbDeviceProgressDialogSymbian* aDialog) = 0

    This callback is called when the device progress dialog is closed by user pressing the "cancel" button.

    \a aDialog - Pointer to dialog that was cancelled.
*/

/*!
   \fn void MHbDeviceProgressDialogObserver::ProgressDialogClosed(const CHbDeviceProgressDialogSymbian* aDialog) = 0

    This callback is called when a device progress dialog is has closed. The closing may
    be a result of Close() being called, a dialog with autoClose property has reached
    its maximum value or user pressing cancel button. It is not called if Close() is
    called before ShowL().

    \a aDialog - Pointer to dialog instance that has closed.
*/

/*!
    \class CHbDeviceProgressDialogSymbian
    \brief CHbDeviceProgressDialogSymbian is a Symbian implementation of HbDeviceProgressDialog.

    It is a client interface for s60 native application to notify user with a text message, icon
    and progress bar, when HbDeviceProgressDialog can not be used.

    CHbDeviceProgressDialogSymbian provides similar interface and functionality as HbDeviceProgressDialog.
    Main difference is that signals are replaced by observer interface. Also the parameters of methods using Qt enumerations are changed to TInt.

    Two progress dialog types are supported: wait and progress dialog.

    Wait dialog can be used when the progress of operation cannot be determined.
    As the delay is unknown, the waiting progress bar will be shown until either a user
    cancels the dialog by pressing the "cancel" button or the application showing the device
    wait dialog closes the dialog after the operation has finished or in case of timeout.

    Progress dialog can be used when the progress of operation is known. For example
    when deleting a number of files, the progress of the operation could be shown as a
    percentage of the files deleted. Application sets the progress value during the
    operation and the dialog will be shown until either a user cancels the dialog by pressing
    the "cancel" button or the application showing the device progress dialog closes the
    dialog after the operation is finished or in case of error.

    Progress dialogs are always asynchronous by nature as the client is responsible of updating the dialog
    according to current progress status.

    The device dialog is only shown when ShowL() is called. Because each dialog update requires
    IPC calls to be made, it's advisable to fully construct the device dialog before calling
    ShowL() or UpdateL().

    Here is an example of using the infinite wait note:

    \code
    _LIT(KConnectText, "Connecting...");
    CHbDeviceProgressDialogSymbian *iDialog = CHbDeviceProgressDialogSymbian::NewL(CHbDeviceProgressDialogSymbian::EWaitDialog);
    iDialog->SetTextL(KConnectText);
    iDialog->ShowL();
    \endcode

    Below is an example of using the device progress dialog:

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

    CHbDeviceProgressDialogSymbian supports animations.
    Supported formats are the following:

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

	Create CHbDeviceProgressDialogSymbian in a way described before and
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

    \sa HbDeviceProgressDialog, HbDeviceDialog
    \proto
    \hbwidgets
*/
/*!
    \enum CHbDeviceProgressDialogSymbian::TType
    Progress dialog types.
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
    Symbian two phase constructor. Returns a pointer to CHbDeviceNotificationDialogSymbian instance.
    \param aType Must be one of the defined CHbDeviceProgressDialogSymbian::TType enumerations.
    \param aObserver Pointer to observer.

    Default value is CHbDeviceProgressDialogSymbian::EProgressDialog.
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
    Destructs the class.
*/
EXPORT_C CHbDeviceProgressDialogSymbian::~CHbDeviceProgressDialogSymbian()
    {
    delete d;
    }

/*!
    Executes the dialog asynchronously.
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::ShowL()
    {
    d->ShowL();
    }

/*!
    Updates the dialog asynchronously.
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::UpdateL()
    {
    d->UpdateL();
    }

/*!
    Cancels and closes the dialog.

    \deprecated CHbDeviceProgressDialogSymbian::Cancel()
    is deprecated. Replaced by CHbDeviceProgressDialogSymbian::Close().

*/
EXPORT_C void CHbDeviceProgressDialogSymbian::Cancel()
    {
    d->Close();
    }

/*!
    Closes the dialog.
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::Close()
    {
    d->Close();
    }

/*!
    Sets the minimum and maximum value of the progress bar within the dialog.
    \param aMin minimum value of the progress bar.
    \param aMax maximum value of the progress bar.

    \sa Minimum(), Maximum()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetRange(TInt aMin, TInt aMax)
    {
    SetMinimum(aMin);
    SetMaximum(aMax);
    }

/*!
    Sets the maximum value of the progress bar within the dialog.
    \param aMax maximum value of the progress bar.
    \sa Maximum()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetMaximum(TInt aMax)
    {
    // Don't allow wait dialog to set max/min other than zero as wait
    // animation bar doesn't work in that case.
    d->iMaximum = d->iType == EWaitDialog ? 0 : aMax;
    }

/*!
    Returns the maximum value of the progress bar within the dialog.

    The default value is 100.
    \sa SetMaximum()
*/
EXPORT_C TInt CHbDeviceProgressDialogSymbian::Maximum() const
    {
    return d->iMaximum;
    }

/*!
    Sets the minimum value of the progress bar within the dialog.
    \param aMin minimum value of the progress bar.
    \sa Minimum()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetMinimum(TInt aMin)
    {
    // Don't allow wait dialog to set max/min other than zero as wait
    // animation bar doesn't work in that case.
    d->iMinimum = d->iType == EWaitDialog ? 0 : aMin;
    }

/*!
    Returns the minimum value of the progress bar within the dialog.

    The default value is 0.
    \sa SetMinimumL()
*/
EXPORT_C TInt CHbDeviceProgressDialogSymbian::Minimum() const
    {
    return d->iMinimum;
    }

/*!
    Sets the autoClose property value of the dialog.
    \param aAutoClose When set, the dialog is closed when value of the progress bar reaches
    the maximum value of the progress bar.
    \sa AutoClose()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetAutoClose(TBool aAutoClose)
    {
    d->iAutoClose = aAutoClose;
    }

/*!
    Returns the value of the autoClose property of the dialog.

    The default value is true for CHbDeviceProgressDialogSymbian::ProgressDialod and false
    for CHbDeviceProgressDialogSymbian::WaitDialog.
    \sa SetAutoCloseL()
*/
EXPORT_C TBool CHbDeviceProgressDialogSymbian::AutoClose() const
    {
    return d->iAutoClose;
    }

/*!
    Sets dialog type.
    \param aType defines the type of the dialog.
    \sa ProgressType()
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
    Sets the value of the progress bar within the dialog.
    \param aProgressValue value of the progress bar.
    \sa ProgressValue()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetProgressValue(TInt aProgressValue)
    {
    d->iProgressValue = aProgressValue;
    }

/*!
    Returns the value of the progress bar within the dialog.
    \sa SetProgressValue()
*/
EXPORT_C TInt CHbDeviceProgressDialogSymbian::ProgressValue() const
    {
    return d->iProgressValue;
    }

/*!
    Sets text of the dialog.
    \param aText dialog text.
    \sa Text()
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
    Returns text of the note.
    \sa SetTextL()
*/
EXPORT_C const TPtrC CHbDeviceProgressDialogSymbian::Text() const
    {
    return d->iText;
    }

/*!
    Sets message box icon name or animation logical name. The message box gets updated next time ShowL() or UpdateL()
    is called.

    \param aIconName Icon name. Icon can be from Hb resources or themes. Or it can be a file in
    a file system.

    \sa IconName()
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
    Returns name and path of the icon shown on dialog or animation's logical name.

    \sa SetIconNameL()
*/
EXPORT_C const TPtrC CHbDeviceProgressDialogSymbian::IconName() const
    {
    return d->iIconName;
    }

/*!
    Sets notification dialog's animation definition name.  The dialogs get updated next when time ShowL() or UpdateL()
    is called.

    \param aAnimationDefinition Animation definition file name. Definition can be from Hb resources or themes.
    Or can be a file in a file system.

    \sa AnimationDefinition()
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
    Returns an animation definition set for a dialog. If not set, return string.

    \sa SetAnimationDefinitionL()
*/
EXPORT_C TPtrC CHbDeviceProgressDialogSymbian::AnimationDefinition() const
	{
	return d->iAnimationDefinition;
	}

/*!
    Sets the text alignment.
    \param aAlign Qt defined alignment options can used.
    \sa TextAlignment()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetTextAlignment(TInt aAlign)
    {
    d->iTextAlign = aAlign;
    }

/*!
    Returns the text alignment.

    The default value is Qt::AlignLeft|Qt::AlignVCenter.
    \sa SetTextAlignment()
*/
EXPORT_C TInt CHbDeviceProgressDialogSymbian::TextAlignment() const
    {
    return d->iTextAlign;
    }

/*!
    Sets the icon alignment.
    \param aAlign Qt defined alignment options can used.
    \sa IconAlignment()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetIconAlignment(TInt aAlign)
    {
    d->iIconAlign = aAlign;
    }

/*!
    Returns the icon alignment.

    The default value is Qt::AlignCenter.
    \sa SetIconAlignment()
*/
EXPORT_C TInt CHbDeviceProgressDialogSymbian::IconAlignment() const
    {
    return d->iIconAlign;
    }

/*!
    Sets the text wrapping.
    \param aWrap When set, the text is drawn with Qt::TextWordWrap enabled meaning that
    lines breaks are at appropriate point, e.g. at word boundaries.
    \sa TextWrapping()

    \deprecated CHbDeviceProgressDialogSymbian::SetTextWrapping()
    is deprecated. Will be removed.

*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetTextWrapping(TBool aWrap)
    {
    d->iWrap = aWrap;
    }

/*!
    Returns the text wrapping setting.

    The default value is true.
    \sa SetTextWrapping()

    \deprecated CHbDeviceProgressDialogSymbian::TextWrapping()
    is deprecated. Will be removed.
*/
EXPORT_C TBool CHbDeviceProgressDialogSymbian::TextWrapping() const
    {
    return d->iWrap;
    }

/*!
    Sets progress dialog box button text. The dialog gets updated next time ShowL() or UpdateL()
    is called.

    \param aText Button text.

    \sa ButtonText()
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
    Returns progress dialog button text.

    \sa SetButtonTextL()
*/
EXPORT_C const TPtrC CHbDeviceProgressDialogSymbian::ButtonText() const
{
    return d->iButton.iText;
}

/*!
    Sets progress dialog button presence. The dialog updated next time ShowL() or UpdateL()
    is called.

    \param aEnable True enables (makes visible) the dialog button.

    \sa HasButton()
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetButton(TBool aEnable)
{
    d->iButton.iFlags.iIsNull = !aEnable;
    d->iButton.iFlags.iIsNullValid = true;
    d->iButton.iFlags.iIsNullModified = true;
}

/*!
    Returns progress dialog button presence.

    \param aButton Selects the button.

    \sa SetButton()
*/
EXPORT_C TBool CHbDeviceProgressDialogSymbian::HasButton() const
{
    return !d->iButton.iFlags.iIsNull;
}

/*!
    Sets progress dialog observer. The observer is called when progress dialog is closed.
    \param aObserver Pointer to observer.

    \sa MHbDeviceProgressDialogObserver
*/
EXPORT_C void CHbDeviceProgressDialogSymbian::SetObserver(MHbDeviceProgressDialogObserver* aObserver)
    {
    d->iObserver = aObserver;
    }

/*
    Constructor.
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
    SetTextAlignment(KAlignCenterLeft);
    SetIconAlignment(KAlignCenter);
    SetTextWrapping(ETrue);
    }
