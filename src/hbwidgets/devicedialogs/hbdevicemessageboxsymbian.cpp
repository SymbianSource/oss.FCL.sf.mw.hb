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

#include "hbdevicemessageboxsymbian.h"

#include <hbsymbianvariant.h>
#include <hbdevicedialogsymbian.h>
#include <hbdeleteguardsymbian_p.h>

#define ARG_UNUSED(x) (void)x;

enum TMessageBoxPanic {
    ETypePanic,
    EPropertyPanic,
    EButtonIdPanic
};

NONSHARABLE_CLASS(TMessageBoxProperty)
{
public:
    enum TType {
        EIntProperty,
        EStringProperty
    };
    TMessageBoxProperty(){mValue.mString = 0;}
    void Init(TType aId);
    ~TMessageBoxProperty();
    const TPtrC StringValue() const;
    TInt IntValue() const;
    void SetL(const TDesC &aValue);
    void Set(TInt aValue);
    bool Modified() const{return mFlags.mModified;}
    void SetModified(bool aModified){mFlags.mModified = aModified;}
    bool Valid() const{return mFlags.mValid;}
    void SetValid(bool aValid){mFlags.mValid = aValid;}
    bool IsNullAction() const{return mFlags.mNullAction;}
    void SetNullAction(bool aNull){mFlags.mNullAction = aNull;}
    TType Type() const{return mType;}
    void SetType(TType aType){mType = aType;}
private:
    TType mType;
    union {
        TInt mInt;
        HBufC *mString;
    } mValue;
    struct {
        bool mNullAction:1; // marks action (button) null
        bool mValid:1; // property value has has been set
        bool mModified:1; // property has been modified
    } mFlags;
};

NONSHARABLE_CLASS(CHbDeviceMessageBoxPrivate)  : public CBase, public MHbDeviceDialogObserver
{
public:
    enum TPropertyId {
        EFirstIntProperty,
        EType = EFirstIntProperty,
        EIconVisible,
        ETimeout,
        EStandardButtons,
        EShowLevel,
        EDismissPolicy,
        ELastIntProperty = EDismissPolicy,
        EFirstStringProperty,
        EText = EFirstStringProperty,
        EIconName,
        EAcceptText,
        ERejectText,
        EAnimationDefinition,
        ELastStringProperty = EAnimationDefinition,
        ENumProperties
    };
    void ConstructL(CHbDeviceMessageBoxSymbian::TType aType,
        MHbDeviceMessageBoxObserver *aObserver);
    ~CHbDeviceMessageBoxPrivate();
    void InitProperties(CHbDeviceMessageBoxSymbian::TType aType);
    bool PropertiesModified(TInt aIgnoreMask) const;
    void SetPropertyValueL(TPropertyId aId, const TDesC &aValue);
    void SetPropertyValue(TPropertyId aId, TInt aValue);
    void SetButtonNull(CHbDeviceMessageBoxSymbian::TButtonId aButtonId, bool aValue);
    void SetButtonTextL(CHbDeviceMessageBoxSymbian::TButtonId aButtonId, const TDesC &aValue);
    void SetStandardButtons(TUint aButtons);
    void SendToServerL(bool aShow = false);
    void Close();
    bool WaitForClosed();
    static const TPtrC PropertyName(TPropertyId aId);
    static TPropertyId ButtonPropertyId(TPropertyId aId, CHbDeviceMessageBoxSymbian::TButtonId aButtonId);
    static HBufC *CreateActionDataLC(TBool aNull, const TDesC &text);
    static TInt CountBits(TUint aValue);

public: // MHbDeviceDialogObserver
    void DataReceived(CHbSymbianVariantMap& aData);
    void DeviceDialogClosed(TInt aCompletionCode);

public: // data

    CHbDeviceMessageBoxSymbian *q;
    TMessageBoxProperty mProperties[ENumProperties];
    CHbDeviceDialogSymbian *mDeviceDialog;
    bool mShowCalled;
    CHbDeviceMessageBoxSymbian::TButtonId mReceivedButton;
    MHbDeviceMessageBoxObserver *mObserver;
    bool *mDeleted;
    CActiveSchedulerWait mActiveSchedulerWait;
};

// Panic application
static void Panic(TInt aCode)
{
    _LIT(KCategory, "CHbDeviceMessageBoxSymbian");
    User::Panic(KCategory, aCode);
}

// Initialize property
void TMessageBoxProperty::Init(TType aType)
{
    if (mType == EStringProperty) {
        delete mValue.mString;
        // NULL string property denotes an empty string
        mValue.mString = 0;
    } else {
        mValue.mInt = 0;
    }
    mType = aType;
    mFlags.mNullAction = false;
    mFlags.mValid = false;
    mFlags.mModified = false;
}

// Destructor
TMessageBoxProperty::~TMessageBoxProperty()
{
    if (mType == EStringProperty) {
        delete mValue.mString;
    }
}

// Get string value
const TPtrC TMessageBoxProperty::StringValue() const
{
    __ASSERT_DEBUG(mType == EStringProperty, Panic(EPropertyPanic));

    if (mValue.mString) {
        return TPtrC(*mValue.mString);
    } else {
        return TPtrC(KNullDesC);
    }
}

// Get integer value
TInt TMessageBoxProperty::IntValue() const
{
    __ASSERT_DEBUG(mType == EIntProperty, Panic(EPropertyPanic));

    return mValue.mInt;
}

// Set string value
void TMessageBoxProperty::SetL(const TDesC &aValue)
{
    __ASSERT_DEBUG(mType == EStringProperty, Panic(EPropertyPanic));

    // Check that value changes
    if (mValue.mString && *mValue.mString == aValue) {
        return;
    }
    HBufC *newValue = HBufC::NewL(aValue.Length());
    *newValue = aValue;
    delete mValue.mString;
    mValue.mString = newValue;
    mFlags.mValid = true;
    mFlags.mModified = true;
}

// Set integer value
void TMessageBoxProperty::Set(TInt aValue)
{
    __ASSERT_DEBUG(mType == EIntProperty, Panic(EPropertyPanic));

    mFlags.mValid = true;

    if (mValue.mInt == aValue) {
        return;
    }
    mValue.mInt = aValue;
    mFlags.mModified = true;
}

// Construct private
void CHbDeviceMessageBoxPrivate::ConstructL(CHbDeviceMessageBoxSymbian::TType aType,
    MHbDeviceMessageBoxObserver *aObserver)
{
    mDeviceDialog = CHbDeviceDialogSymbian::NewL();

    mReceivedButton = CHbDeviceMessageBoxSymbian::EInvalidButton;
    mObserver = aObserver;

    InitProperties(aType);
}

// Destructor
CHbDeviceMessageBoxPrivate::~CHbDeviceMessageBoxPrivate()
{
    // If no observer, remove observer from device dialog. This leaves the
    // dialog widget executing at server when client closes connection.
    if (!mObserver) {
        mDeviceDialog->SetObserver(0);
    }
    delete mDeviceDialog;

    // Set object deleted flag
    if (mDeleted) {
        // Mark the object as deleted.
        *mDeleted = true;
        mDeleted = 0;
    }
}

// Initialize properties
void CHbDeviceMessageBoxPrivate::InitProperties(CHbDeviceMessageBoxSymbian::TType aType)
{
    // Initialize all properties to default values
    for(int i = 0; i < ENumProperties; i++) {
        TMessageBoxProperty &property = mProperties[i];
        property.Init(i >= EFirstStringProperty ?
            TMessageBoxProperty::EStringProperty : TMessageBoxProperty::EIntProperty);
    }

    // Set properties to default values
    mProperties[EType].Set(aType);
    mProperties[EIconVisible].Set(ETrue);

    switch(aType) {
    case CHbDeviceMessageBoxSymbian::ENone:
        mProperties[EIconVisible].Set(EFalse);
        // Fall through
    case CHbDeviceMessageBoxSymbian::EInformation:
    case CHbDeviceMessageBoxSymbian::EWarning: {
        const TInt KStandardTimeout = 3000; // 3s
        mProperties[ETimeout].Set(KStandardTimeout);
        const TInt KTapAnywhere = 0x03; // HbPopup::TapAnywhere
        mProperties[EDismissPolicy].Set(KTapAnywhere);
        // Initialize standard buttons but do not send to server as
        // HbMessageBox sets standard buttons by default
        const TInt KStandardButtons = 0x00000400; // Ok
        mProperties[EStandardButtons].Set(KStandardButtons);
        mProperties[EStandardButtons].SetModified(false);
        mProperties[EStandardButtons].SetValid(false);

        // Plugin has accept button by default
        mProperties[ERejectText].SetNullAction(true);
        break;
    }
    case CHbDeviceMessageBoxSymbian::EQuestion: {
        const TInt KNoTimeout = 0;
        mProperties[ETimeout].Set(KNoTimeout);
        const TInt KNoDismiss = 0; // HbPopup::NoDismiss
        mProperties[EDismissPolicy].Set(KNoDismiss);
        // Initialize standard buttons and send to server as
        // HbMessageBox currently sets standard buttons to "Ok" by default
        const TInt KStandardButtons = 0x00002000|0x00010000; // Yes|No
        mProperties[EStandardButtons].Set(KStandardButtons);
        break;
    }
    default:
        Panic(ETypePanic);
    }
}

bool CHbDeviceMessageBoxPrivate::PropertiesModified(TInt aIgnoreMask) const
{
    for(int i = 0; i < ENumProperties; i++) {
        if (mProperties[i].Modified() && (aIgnoreMask & 1) == 0) {
            return true;
        }
        aIgnoreMask >>= 1;
    }
    return false;
}

// Set string property value
void CHbDeviceMessageBoxPrivate::SetPropertyValueL(TPropertyId aId, const TDesC &aValue)
{
    mProperties[aId].SetL(aValue);
}

// Set int property value
void CHbDeviceMessageBoxPrivate::SetPropertyValue(TPropertyId aId, TInt aValue)
{
    mProperties[aId].Set(aValue);
}

// Set button null property value
void CHbDeviceMessageBoxPrivate::SetButtonNull(CHbDeviceMessageBoxSymbian::TButtonId aButtonId,
    bool aValue)
{
    TPropertyId id = ButtonPropertyId(EAcceptText, aButtonId);
    mProperties[id].SetNullAction(aValue);
    mProperties[id].SetValid(true);
    mProperties[id].SetModified(true);
}

// Set button text property
void CHbDeviceMessageBoxPrivate::SetButtonTextL(
    CHbDeviceMessageBoxSymbian::TButtonId aButtonId, const TDesC &aValue)
{
    TPropertyId id = ButtonPropertyId(EAcceptText, aButtonId);
    mProperties[id].SetL(aValue);
}

// Set standard buttons property
void CHbDeviceMessageBoxPrivate::SetStandardButtons(TUint aButtons)
{
    static const CHbDeviceMessageBoxSymbian::TButtonId buttonIds[] = {
        CHbDeviceMessageBoxSymbian::EAcceptButton, CHbDeviceMessageBoxSymbian::ERejectButton
    };
    const TInt KNumButtonIds = sizeof(buttonIds) / sizeof(buttonIds[0]);
    TInt buttonCount = Min(KNumButtonIds, CountBits(aButtons));
    // Mark buttons present
    TInt i = 0;
    for(; i < buttonCount; i++) {
        TPropertyId id = ButtonPropertyId(EAcceptText, buttonIds[i]);
        // Set property value but don't send to server
        mProperties[id].SetNullAction(false);
        mProperties[id].SetValid(false);
        mProperties[id].SetModified(false);
    }
    // Mark extra buttons not present
    for(; i < KNumButtonIds; i++) {
        TPropertyId id = ButtonPropertyId(EAcceptText, buttonIds[i]);
        // Set property value but don't send to server
        mProperties[id].SetNullAction(true);
        mProperties[id].SetValid(false);
        mProperties[id].SetModified(false);
    }
    SetPropertyValue(EStandardButtons, aButtons);

}

// Send properties to server. Show or update.
void CHbDeviceMessageBoxPrivate::SendToServerL(bool aShow)
{
    // If this is update but ShowL() has not been called, return.
    if (!aShow && !mShowCalled) {
      return;
    }

    // If this is update but no properties have been mofified, return
    const TInt ignoreMask = 0;
    if (!aShow && !PropertiesModified(ignoreMask)) {
        return;
    }

    CHbSymbianVariantMap *parameters = CHbSymbianVariantMap::NewL();
    CleanupStack::PushL(parameters);

    for(int i = 0; i < ENumProperties; i++) {
        TMessageBoxProperty &property = mProperties[i];
        // ShowL() send all valid properties. Update only sends modified properties.
        bool sendProperty = aShow ? property.Valid() : property.Modified();
        if (sendProperty) {
            CHbSymbianVariant* parameter;
            if (i == EAcceptText || i == ERejectText) {
                HBufC *actionData = CreateActionDataLC(property.IsNullAction(), property.StringValue());
                parameter = CHbSymbianVariant::NewL(actionData, CHbSymbianVariant::EDes);
                CleanupStack::PopAndDestroy(); // actionData
            } else if (property.Type() == TMessageBoxProperty::EIntProperty) {
                TInt value = property.IntValue();
                parameter = CHbSymbianVariant::NewL(&value, CHbSymbianVariant::EInt);
            } else {
                TPtrC value = property.StringValue();
                parameter = CHbSymbianVariant::NewL(&value, CHbSymbianVariant::EDes);
            }
            CleanupStack::PushL(parameter);
            User::LeaveIfError(parameters->Add(
                PropertyName(static_cast<TPropertyId>(i)), parameter));
            CleanupStack::Pop(); // parameter
            property.SetModified(false);
        }
    }

    _LIT(KDeviceDialogType, "com.nokia.hb.devicemessagebox/1.0");
    TInt error;
    if (aShow) {
        mReceivedButton = CHbDeviceMessageBoxSymbian::EInvalidButton;
        error = mDeviceDialog->Show(KDeviceDialogType, *parameters, this);
        if (error != KErrNone) {
            User::Leave(error); // error can be positive or negative
        }
        mShowCalled = true;
    } else {
        error = mDeviceDialog->Update(*parameters);
        if (error != KErrNone) {
            User::Leave(error); // error can be positive or negative
        }
    }
    CleanupStack::PopAndDestroy(); // parameters
}

// Close message box
void CHbDeviceMessageBoxPrivate::Close()
{
    mDeviceDialog->Cancel();
}

// Wait for message box to close
bool CHbDeviceMessageBoxPrivate::WaitForClosed()
{
    // Returns true if object was not deleted during wait
    if (!mShowCalled) {
        return true;
    }
    RHbDeleteGuardSymbian guard;
    guard.OpenAndPushL(&mDeleted);
    mActiveSchedulerWait.Start();
    return !guard.PopAndClose();
}

// Get property name
const TPtrC CHbDeviceMessageBoxPrivate::PropertyName(TPropertyId aId)
{
    static const wchar_t * const names[] = {
        L"type",
        L"iconVisible",
        L"timeout",
        L"standardButtons",
        L"showLevel",
        L"dismissPolicy",
        L"text",
        L"iconName",
        L"acceptAction",
        L"rejectAction",
        L"animationDefinition"
    };
    __ASSERT_DEBUG(aId >= 0 && aId < sizeof(names)/sizeof(names[0]), Panic(EPropertyPanic));

    return TPtrC((const TUint16 *)names[aId]);
}

// Modify property id according to button
CHbDeviceMessageBoxPrivate::TPropertyId CHbDeviceMessageBoxPrivate::ButtonPropertyId(
    TPropertyId aId, CHbDeviceMessageBoxSymbian::TButtonId aButtonId)
{
    __ASSERT_DEBUG(aButtonId == CHbDeviceMessageBoxSymbian::EAcceptButton ||
        aButtonId == CHbDeviceMessageBoxSymbian::ERejectButton, Panic(EButtonIdPanic));
    switch(aId) {
    case EAcceptText:
        return aButtonId == CHbDeviceMessageBoxSymbian::EAcceptButton ? EAcceptText : ERejectText;
    default:
        Panic(EPropertyPanic);
        return aId; // suppress warning
    }
}

// Pack into a string a data for a button
HBufC *CHbDeviceMessageBoxPrivate::CreateActionDataLC(TBool aNull, const TDesC &text)
{
    HBufC *actionData;
    if (aNull) {
        actionData = HBufC::NewL(0);
    } else {
        _LIT(KtextTag, "t:");
        actionData = HBufC::NewL(text.Length() + KtextTag().Length());
        TPtr ptr = actionData->Des();
        ptr.Append(KtextTag);
        ptr.Append(text);
    }
    CleanupStack::PushL(actionData);
    return actionData;
}

// Count number of bits on
TInt CHbDeviceMessageBoxPrivate::CountBits(TUint aValue)
{
    TInt count = 0;
    while(aValue) {
        count += aValue & 1;
        aValue >>= 1;
    }
    return count;
}

// Observer, data received from device message box
void CHbDeviceMessageBoxPrivate::DataReceived(CHbSymbianVariantMap& aData)
{
    _LIT(KKey, "act");
    const CHbSymbianVariant* variant = aData.Get(KKey);
    if (variant) {
        const TDesC *value = variant->Value<const TDesC>();
        if (value) {
            _LIT(KPrimary, "p");
            if (*value == KPrimary) {
                mReceivedButton = CHbDeviceMessageBoxSymbian::EAcceptButton;
            } else {
                mReceivedButton = CHbDeviceMessageBoxSymbian::ERejectButton;
            }
        }
    }
}

// Observer, device message box closed
void CHbDeviceMessageBoxPrivate::DeviceDialogClosed(TInt aCompletionCode)
{
    ARG_UNUSED(aCompletionCode)

    mShowCalled = false;
    if (mObserver) {
        RHbDeleteGuardSymbian guard;
        guard.OpenAndPushL(&mDeleted);
        mObserver->MessageBoxClosed(q, mReceivedButton);
        if (guard.PopAndClose()) {
            return; // observer callback deleted this object. Do not touch it anymore.
        }
    }
    if (mActiveSchedulerWait.IsStarted()) {
        mActiveSchedulerWait.AsyncStop();
    }
}

// Set dialog show level
void SetShowLevel(CHbDeviceMessageBoxPrivate *aBox, TInt aLevel)
{
    aBox->SetPropertyValue(CHbDeviceMessageBoxPrivate::EShowLevel, aLevel);
}

/*!
    @stable
    @hbwidgets

    \class CHbDeviceMessageBoxSymbian
    \brief CHbDeviceMessageBoxSymbian is a Symbian implementation of HbDeviceMessageBox.

    <b>This class is Symbian only. Not available on other platforms.</b>

    CHbDeviceMessageBoxSymbian is intended for use by servers that don't run Qt event loop
    and cannot use HbDeviceMessageBox.

    It displays a message box with text, icon or animation and two optional buttons,
    accept and reject.

    It provides a similar interface as HbDeviceMessageBox. The main
    differences are:
    - Signals are replaced by an observer interface
    - HbAction/QAction is replaced by button text.
    - Function parameters whose type is a Qt or Hb enumeration are replaced by an integer.
      As suitable default values are set at construction, it is assumed that application rarely
      needs to use these.

    Device message box is a modal dialog. It is shown on top of applications by device dialog
    server. CHbDeviceMessageBoxSymbian is a client of the server.

    Four types of message boxes are predefined. The type determines a set of default properties that
    are set on construction. Below is a table listing types and their default properties.

    <table border="1">
        <caption><b>HbDeviceMessageBox types and default properties</b></caption>
        <tr><th>Type</th><th>Icon</th><th>Buttons</th><th>Timeout</th><th>Dismiss policy</th><th>Sound</th></tr>
        <tr><td>ENone</td><td>No icon</td><td>"Ok" button</td><td>Standard timeout</td><td>Tap anywhere</td><td>No sound</td></tr>
        <tr><td>EInformation</td><td>Info icon</td><td>"Ok" button</td><td>Standard timeout</td><td>Tap anywhere</td><td>Info sound</td></tr>
        <tr><td>EWarning</td><td>Warning icon</td><td>"Ok" button</td><td>Standard timeout</td><td>Tap anywhere</td><td>Warn sound</td></tr>
        <tr><td>EQuestion</td><td>Question icon</td><td>"Yes" and "No" buttons</td><td>No timeout</td><td>Button press</td><td>Question sound</td></tr>
    </table>

    ShowL() displays a message box asynchronously. The function returns immediately.
    A new message box is lauched every time ShowL() is called. The launched
    dialog can be updated by setters and then calling UpdateL(). Closing can be observed by a
    callback. Because each UpdateL() after the ShowL() requires interprocess communication,
    it's advisable to fully construct the device message box before displaying it.

    A device message box is launched synchronously by ExecL() function. The function returns when
    the dialog is closed and return value indicates the button selected. The function uses
    active scheduler wait to wait for the message box to close. ExecL() should be used
    carefully as application event processing continues during the wait and program state may
    change, e.g. objects may get deleted. Preferred way is to use asynchoronous ShowL() instead.

    Device message box is closed when user dismisses it, when Close()
    is called, timeout triggers or system closes the dialog. Default return value is
    CHbDeviceMessageBoxSymbian::EInvalidButton. The default is returned in all other cases
    than user pressing an accept or reject button.

    Static convenience functions are provided for ease of use. Message boxes created by these
    static functions contain a default set of properties depending on the message box type and their
    contents cannot be updated while the message box executes.

    If CHbDeviceMessageBoxSymbian has no observer set, the object can be deleted after
    ShowL() has been called and device-dialog service takes care showing the message box. If
    observer is set, message box is closed when CHbDeviceMessageBoxSymbian object goes out of
    scope.  

    An animation can replace an icon on device message box. Supported icon animation formats are
    following:

    - GIF (.gif)
    - MNG (.mng)
        - Frame animations

    Sample code:

    \code
    // Launch an information message box and continue execution while it is on display

    CHbDeviceMessageBoxSymbian* messageBox = CHbDeviceMessageBoxSymbian::NewL(
        CHbDeviceMessageBoxSymbian::EInformation);
    CleanupStack::PushL(messageBox);
    _LIT(KText, "Connection established");
    messageBox->SetTextL(KText);
    messageBox->ShowL();
    CleanupStack::PopAndDestroy(); // messageBox
    \endcode

    \code
    // Above example using a static helper function

    _LIT(KText, "Connection established");
    CHbDeviceMessageBoxSymbian::InformationL(KText);
    \endcode

    \code
    // Ask from user whether to accept connection. Use default buttons (Yes and No). Beware that
    // Symbian event loop executes while QuestionL() is executing. For example application may exit.

    _LIT(KText, "Accept connection ?");
    CHbDeviceMessageBoxSymbian::TButtonId selection =
        CHbDeviceMessageBoxSymbian::QuestionL(KText);
    if (selection == CHbDeviceMessageBoxSymbian::EAcceptButton) {
        // user pressed yes
    }
    \endcode

    \code
    // Modify default properties of the message box.

    iMessageBox = CHbDeviceMessageBoxSymbian::NewL(CHbDeviceMessageBoxSymbian::EQuestion);
    _LIT(KText, "Accept connection ?");
    iMessageBox->SetTextL(KText);
    _LIT(KIconName, "qtg_small_smiley_wondering");
    iMessageBox->SetIconNameL(KIconName);

    _LIT(KAcceptText, "Yes");
    iMessageBox->SetButtonTextL(CHbDeviceMessageBoxSymbian::EAcceptButton, KAcceptText);
    _LIT(KRejectText, "No");
    iMessageBox->SetButtonTextL(CHbDeviceMessageBoxSymbian::ERejectButton, KRejectText);

    iMessageBox->ShowL();
    \endcode

    Creating a frame animation.

    Create an animation definition file:
    \code
    <animations>
        <icon name="frame_anim_looping" playmode="loop">
            <frame duration="100">c:\icon1.svg</frame>
            <frame duration="200">c:\icon2.svg</frame>
            <frame duration="300">c:\icon3.svg</frame>
            </icon>
    </animations>
    \endcode

    Create HbDeviceMessageBox in a way described before and
    set definition file and animation's logical name.

    \code
    _LIT(KAnimationDefinitionXML, "C:\animation.axml");
    _LITK(KLogicalIconName, "frame_anim_looping");

    iMessageBox->SetAnimationDefinitionL(KAnimationDefinitionXML);
    iMessageBox->SetIconNameL(KIconName);
    iMessageBox->ShowL();
    \endcode

    \sa HbDeviceMessageBox, HbMessageBox, MHbDeviceMessageBoxObserver
*/

/*!
    \enum CHbDeviceMessageBoxSymbian::TType
    Message box type. Determines a set of default properties for the message box.
*/
/*!
    \var CHbDeviceMessageBoxSymbian::TType CHbDeviceMessageBoxSymbian::ENone
    Message box with no icon and audio defined.
*/
/*!
    \var CHbDeviceMessageBoxSymbian::TType CHbDeviceMessageBoxSymbian::EInformation
    Information message box.
*/
/*!
    \var CHbDeviceMessageBoxSymbian::TType CHbDeviceMessageBoxSymbian::EQuestion
    Question message box.
*/
/*!
    \var CHbDeviceMessageBoxSymbian::TType CHbDeviceMessageBoxSymbian::EWarning
    Warning message box.
*/

/*!
    \enum CHbDeviceMessageBoxSymbian::TButtonId
    Selects message box button.
*/
/*!
    \var CHbDeviceMessageBoxSymbian::TButtonId CHbDeviceMessageBoxSymbian::EInvalidButton
    No button.
*/
/*!
    \var CHbDeviceMessageBoxSymbian::TButtonId CHbDeviceMessageBoxSymbian::EAcceptButton
    Accept button.
*/
/*!
    \var CHbDeviceMessageBoxSymbian::TButtonId CHbDeviceMessageBoxSymbian::ERejectButton
    Reject button.
*/

/*!
    \class MHbDeviceMessageBoxObserver
    \brief MHbDeviceMessageBoxObserver is an observer interface for observing CHbDeviceMessageBoxSymbian.

    \sa CHbDeviceMessageBoxSymbian
*/

/*!
    \fn virtual void MHbDeviceMessageBoxObserver::MessageBoxClosed(
        const CHbDeviceMessageBoxSymbian *aMessageBox,
        CHbDeviceMessageBoxSymbian::TButtonId aButton) = 0

    Call back function to observe device message box closing.

    \param aMessageBox Pointer to object that launched the closing device message box.
    \param aButton Button that was pressed.

    \sa CHbDeviceMessageBoxSymbian::SetObserver()
*/

/*!
    Constructs CHbDeviceMessageBoxSymbian.
*/
CHbDeviceMessageBoxSymbian::CHbDeviceMessageBoxSymbian()
{
}

/*!
    Destructs CHbDeviceMessageBoxSymbian. The message box launched by ShowL()
    is closed if observer is set. If there is no observer, it is
    left executing and closes itself by timeout.
*/
EXPORT_C CHbDeviceMessageBoxSymbian::~CHbDeviceMessageBoxSymbian()
{
    delete d;
}

/*!
    Constructs a new CHbDeviceMessageBoxSymbian and returns a pointer it.

    \param aType Selects a template for the message box. None, information, question or warning.
    \param aObserver Observer for message box close event. 0 if no observer.
*/
EXPORT_C CHbDeviceMessageBoxSymbian* CHbDeviceMessageBoxSymbian::NewL(TType aType,
    MHbDeviceMessageBoxObserver *aObserver)
{
    ARG_UNUSED(aObserver)
    CHbDeviceMessageBoxSymbian *messageBox = new (ELeave) CHbDeviceMessageBoxSymbian;
    CleanupStack::PushL(messageBox);
    CHbDeviceMessageBoxPrivate *mboxPrivate = new (ELeave) CHbDeviceMessageBoxPrivate;
    mboxPrivate->q = messageBox;
    CleanupStack::PushL(mboxPrivate);
    mboxPrivate->ConstructL(aType, aObserver);
    messageBox->d = mboxPrivate;
    CleanupStack::Pop(2); // messageBox, mboxPrivate
    messageBox->SetObserver(aObserver);
    return messageBox;
}

/*!
    Static convenience function to create and show a question device message box. Waits for
    the message box to close and returns button selected. If message box was closed for other
    reason than button press, returns CHbDeviceMessageBoxSymbian::EInvalidButton.

    <b> Beware that Symbian event processing is running while the function executes. For example
    application may have exited when the function returns.</b>

    \param aText Message box text.
    \param aAcceptButtonText Accept button text. If text is empty, a default (Yes) button is used.
    \param aRejectButtonText Reject button text. If text is empty, a default (No) button is used.
*/
EXPORT_C CHbDeviceMessageBoxSymbian::TButtonId CHbDeviceMessageBoxSymbian::QuestionL(
    const TDesC& aText, const TDesC& aAcceptButtonText, const TDesC& aRejectButtonText)
{
    CHbDeviceMessageBoxSymbian* messageBox = NewL(EQuestion);
    CleanupStack::PushL(messageBox);
    messageBox->SetTextL(aText);
    messageBox->SetStandardButtons(0x00002000|0x00010000); // Yes|No
    if (aAcceptButtonText.Length()) {
        messageBox->SetButtonTextL(EAcceptButton, aAcceptButtonText);
    }
    if (aRejectButtonText.Length()) {
        messageBox->SetButtonTextL(ERejectButton, aRejectButtonText);
    }
    TButtonId buttonId = messageBox->ExecL();
    CleanupStack::PopAndDestroy(); // messageBox
    return buttonId;
}

/*!
    Static convenience function to create and show a question device message box. Waits for
    the message box to close and returns button selected. If message box was closed for other
    reason than button press, returns CHbDeviceMessageBoxSymbian::EInvalidButton.

    <b> Beware that Symbian event processing is running while the function executes. For example
    application may have exited when the function returns.</b>

    \param aText Message box text.
    \param aStandardButtons Specifies message box buttons. If 0, default buttons "Yes" and "No" are
    used. See SetStandardButtons() for format.
*/
EXPORT_C CHbDeviceMessageBoxSymbian::TButtonId CHbDeviceMessageBoxSymbian::QuestionL(
    const TDesC& aText, TUint aStandardButtons)
{
    CHbDeviceMessageBoxSymbian* messageBox = NewL(EQuestion);
    CleanupStack::PushL(messageBox);
    messageBox->SetTextL(aText);
    if (!aStandardButtons) {
        aStandardButtons = 0x00002000|0x00010000; // Yes|No
    }
    messageBox->SetStandardButtons(aStandardButtons);
    TButtonId buttonId = messageBox->ExecL();
    CleanupStack::PopAndDestroy(); // messageBox
    return buttonId;
}

/*!
    Static convenience function to create and show an information device message box. Returns
    immediately (doesn't wait for the box to close).

    \param aText Message box text.
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::InformationL(const TDesC& aText)
{
    CHbDeviceMessageBoxSymbian* messageBox = NewL(EInformation);
    CleanupStack::PushL(messageBox);
    messageBox->SetTextL(aText);
    messageBox->ShowL();
    CleanupStack::PopAndDestroy(); // messageBox
}

/*!
    Static convenience function to create and show a warning device message box. Returns
    immediately (doesn't wait for the box to close).

    \param aText Message box text.
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::WarningL(const TDesC& aText)
{
    CHbDeviceMessageBoxSymbian* messageBox = NewL(EWarning);
    CleanupStack::PushL(messageBox);
    messageBox->SetTextL(aText);
    messageBox->ShowL();
    CleanupStack::PopAndDestroy(); // messageBox
}

/*!
    Shows a device message box asyncronously. Function launches the message box and returns immediately.
    Closing of the message box can be observer with MHbDeviceMessageBoxObserver.
    CHbDeviceMessageBoxSymbian object can be used to launch several message boxes. A new one
    is launched every time ShowL() is called. Observer receives events for the latest message box launched.

    \sa SetObserver(), UpdateL()
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::ShowL()
{
    d->SendToServerL(true);
}

/*!
    Updates a device message box asyncronously. Message box that was launched with a lates ShowL()
    is updated. Properties that were modified since the last ShowL() or UpdateL() are sent to
    device dialog server. Fails if no message box has not been launched.

    \sa ShowL()
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::UpdateL()
{
    d->SendToServerL(false);
}

/*!
    Closes a device message box. Closing of the message box can be observer with
    MHbDeviceMessageBoxObserver.

    \sa ShowL(),  SetObserver()
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::Close()
{
    d->Close();
}

/*!
    Shows a device message box synchronously. Function launches the message box and waits for it
    to close. Returns a button that was selected. If message box was closed for other reason
    than button press, returns CHbDeviceMessageBoxSymbian::EInvalidButton.

    <b> Beware that Symbian event processing is running while the function executes. For example
    application may have exited when the function returns.</b>

    \sa ShowL()
*/
EXPORT_C CHbDeviceMessageBoxSymbian::TButtonId CHbDeviceMessageBoxSymbian::ExecL()
{
    TButtonId buttonId = EInvalidButton;
    ShowL();
    if (d->WaitForClosed()) {
        buttonId = d->mReceivedButton;
    } else {
        // This object has been deleted during wait, don't touch it
    }
    return buttonId;
}

/*!
    Sets message box type. All message box properties are reset to a default values for the type.
    Type of a showing message box cannot be changed on the fly. A ShowL() must be called to launch
    a new message box after SetTypeL() has been called.

    \param aType Message box type.

    \sa Type(), ShowL()
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::SetTypeL(TType aType)
{
    d->InitProperties(aType);
    d->mShowCalled = false;
}

/*!
    Returns message box type.

    \sa SetTypeL()
*/
EXPORT_C CHbDeviceMessageBoxSymbian::TType CHbDeviceMessageBoxSymbian::Type() const
{
    return static_cast<TType>(
        d->mProperties[CHbDeviceMessageBoxPrivate::EType].IntValue());
}

/*!
    Sets message box text.

    \param aText Message box text.

    \sa Text(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::SetTextL(const TDesC& aText)
{
    d->SetPropertyValueL(CHbDeviceMessageBoxPrivate::EText, aText);
}

/*!
    Returns message box text.

    \sa SetTextL()
*/
EXPORT_C const TPtrC CHbDeviceMessageBoxSymbian::Text() const
{
    return d->mProperties[CHbDeviceMessageBoxPrivate::EText].StringValue();
}

/*!
    Sets message box icon name or animation logical name.

    \param aIconName Icon name. Icon can be from Hb resources or themes. Or can be a file in
    a file system.

    \sa IconName(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::SetIconNameL(const TDesC& aIconName)
{
    d->SetPropertyValueL(CHbDeviceMessageBoxPrivate::EIconName, aIconName);
}

/*!
    Returns name and path of the icon shown on dialog or animation's logical name.

    \sa SetIconNameL()
*/
EXPORT_C const TPtrC CHbDeviceMessageBoxSymbian::IconName() const
{
    return d->mProperties[CHbDeviceMessageBoxPrivate::EIconName].StringValue();
}

/*!
    Sets message box animation definition name.

    Supported icon animation formats are following:
    - GIF (.gif)
    - MNG (.mng)
    - Frame animations

    \param aAnimationDefinition Animation definition file name. Definition can be from Hb resources or themes.
    Or can be a file in a file system. The definition must be stored to a place where it can be accessed by
    device dialog service.

    \sa AnimationDefinition() SetIconNameL() HbIconAnimationManager::addDefinitionFile(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::SetAnimationDefinitionL(const TDesC& aAnimationDefinition)
{
    d->SetPropertyValueL(CHbDeviceMessageBoxPrivate::EAnimationDefinition, aAnimationDefinition);
}

/*!
    Returns device message box animation definition name.

    \sa SetAnimationDefinitionL()
*/
EXPORT_C TPtrC CHbDeviceMessageBoxSymbian::AnimationDefinition() const
{
    return d->mProperties[CHbDeviceMessageBoxPrivate::EAnimationDefinition].StringValue();
}

/*!
    Sets message box icon visibility.

    \param aVisible Icon visibility.

    \sa IconVisible(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::SetIconVisible(TBool aVisible)
{
    d->SetPropertyValue(CHbDeviceMessageBoxPrivate::EIconVisible, aVisible);
}

/*!
    Returns message box icon visibility. Default value is true for message box
    types other than ENone.

    \sa SetIconVisible()
*/
EXPORT_C TBool CHbDeviceMessageBoxSymbian::IconVisible() const
{
    return d->mProperties[CHbDeviceMessageBoxPrivate::EIconVisible].IntValue();
}

/*!
    Sets message box timeout.

    \param aTimeout Timeout in milliseconds. Zero denotes no timeout.

    \sa Timeout(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::SetTimeout(TInt aTimeout)
{
    d->SetPropertyValue(CHbDeviceMessageBoxPrivate::ETimeout, aTimeout);
}

/*!
    Returns message box timeout value in milliseconds. Default value depends on message box type.
    Question message box has a default of HbPopup::NoTimeout. All other boxes, the default is
    HbPopup::StandardTimeout.

    \sa SetTimeout()
*/
EXPORT_C TInt CHbDeviceMessageBoxSymbian::Timeout() const
{
    return d->mProperties[CHbDeviceMessageBoxPrivate::ETimeout].IntValue();
}

/*!
    Sets message box dismiss policy.

    \param aHbPopupDismissPolicy Dismiss policy. Values are HbPopup::DismissPolicy flags.

    \sa HbPopup::DismissPolicy, DismissPolicy(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::SetDismissPolicy(TInt aHbPopupDismissPolicy)
{
    d->SetPropertyValue(CHbDeviceMessageBoxPrivate::EDismissPolicy,
        aHbPopupDismissPolicy);
}

/*!
    Returns message box dismiss policy. Returned values are HbPopup::DismissPolicy flags.
    Default depends on message box type. Question box has default HbPopup::NoDismiss and
    all other boxes HbPopup::TapAnywhere.

    \sa HbPopup::DismissPolicy, SetDismissPolicy()
*/
EXPORT_C TInt CHbDeviceMessageBoxSymbian::DismissPolicy() const
{
    return d->mProperties[CHbDeviceMessageBoxPrivate::EDismissPolicy].IntValue();
}

/*!
    Sets message box button text.

    \param aButton Selects the button.
    \param aText Button text.

    \sa ButtonText(), SetButton(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::SetButtonTextL(TButtonId aButton, const TDesC& aText)
{
    d->SetButtonTextL(aButton, aText);
}

/*!
    Returns message box button text. Question message box by default has "Yes" and "No" buttons.
    Other message boxes have a single "Ok" button.

    \param aButton Selects the button.

    \sa SetButtonTextL()
*/
EXPORT_C const TPtrC CHbDeviceMessageBoxSymbian::ButtonText(TButtonId aButton) const
{
    CHbDeviceMessageBoxPrivate::TPropertyId id =
        CHbDeviceMessageBoxPrivate::ButtonPropertyId(
            CHbDeviceMessageBoxPrivate::EAcceptText, aButton);
    return d->mProperties[id].StringValue();
}

/*!
    Sets message box button presence.

    \param aButton Selects the button.
    \param aEnable True enables (makes visible) message box button.

    \sa HasButton(), SetButtonTextL(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::SetButton(TButtonId aButton, TBool aEnable)
{
    d->SetButtonNull(aButton, !aEnable);
}

/*!
    Returns message box button presence. Question message box by default has accept and reject
    buttons. Other message boxes have a single accept button.

    \param aButton Selects the button.

    \sa SetButton(), ButtonText()
*/
EXPORT_C TBool CHbDeviceMessageBoxSymbian::HasButton(TButtonId aButton) const
{
    CHbDeviceMessageBoxPrivate::TPropertyId id =
        CHbDeviceMessageBoxPrivate::ButtonPropertyId(
            CHbDeviceMessageBoxPrivate::EAcceptText, aButton);
    return !d->mProperties[id].IsNullAction();
}

/*!
    Sets message box buttons to standard buttons.

    \param aButtons Message box buttons. A combination of flags, eg.
    HbMessageBox::Yes | HbMessageBox::No.
    Button flags are scanned starting from lsb. First button found goes to accept position
    and so forth.

    \sa HbMessageBox::StandardButton, StandardButtons(), ShowL(), UpdateL()
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::SetStandardButtons(TUint aButtons)
{
    return d->SetStandardButtons(aButtons);
}

/*!
    Returns standard buttons set to a message box. A default value for question message box is
    HbMessageBox::Yes | HbMessageBox::No. For all other message box types the default is
    HbMessageBox::Ok.

    \sa HbMessageBox::StandardButton, SetStandardButtons()
*/
EXPORT_C TUint CHbDeviceMessageBoxSymbian::StandardButtons() const
{
    return d->mProperties[CHbDeviceMessageBoxPrivate::EStandardButtons].IntValue();
}

/*!
    Sets message box observer. The observer is called when message box closes.

    \param aObserver Pointer to observer or null.

    \sa MHbDeviceMessageBoxObserver
*/
EXPORT_C void CHbDeviceMessageBoxSymbian::SetObserver(MHbDeviceMessageBoxObserver *aObserver)
{
    d->mObserver = aObserver;
}
