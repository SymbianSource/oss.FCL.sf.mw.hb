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

#include <hbdevicedialog.h>
#include <hbdevicedialogtrace_p.h>
#include "hbdevicemessagebox.h"
#include "hbdevicemessagebox_p.h"

#include <QTimer>
#include <QTimerEvent>
#include <QAction>
#include <QPointer>

HbDeviceMessageBoxPrivate::HbDeviceMessageBoxPrivate() : QObject(),
    mDeviceDialog(0),
    mVisible(false),
    mUpdateTimerId(0)
{
    TRACE_ENTRY
    for(int i = 0; i < NumActions; i++) {
        mDefaultActions[i] = 0;
        mActions[i].mAction = 0;
    }
    TRACE_EXIT
}

HbDeviceMessageBoxPrivate::~HbDeviceMessageBoxPrivate()
{
    TRACE_ENTRY
    // information/warning dialogs may be left at server when client is deleted. If there are no connected
    // signals, device dialog widget is not deleted but runs at server and is closed by a timeout.
    if (mProperties[Type].mValue.toInt() != HbMessageBox::MessageTypeQuestion &&
        q_ptr->receivers(SIGNAL(aboutToClose())) <= 0) {
        mDeviceDialog->disconnect(q_ptr, SLOT(aboutToClose()));
        mDeviceDialog->disconnect(this, SLOT(triggerAction(QVariantMap)));
        if (mUpdateTimerId != 0) {
            // There's a pending update
            sendToServer(false); // this also kills the timer
        }
    }
    delete mDeviceDialog;
    for(int i = 0; i < NumActions; i++) {
        delete mDefaultActions[i];
    }
    TRACE_EXIT
}

void HbDeviceMessageBoxPrivate::init()
{
    TRACE_ENTRY

    initProperties();

    mDeviceDialog = new HbDeviceDialog;
    connect(mDeviceDialog, SIGNAL(deviceDialogClosed()), q_ptr, SIGNAL(aboutToClose()));
    connect(mDeviceDialog, SIGNAL(dataReceived(QVariantMap)), this, SLOT(triggerAction(QVariantMap)));

    TRACE_EXIT
}

void HbDeviceMessageBoxPrivate::initAction(int index)
{
    if (!mActions[index].mAction) {
        if (!mDefaultActions[index]) {
            mDefaultActions[index] = new QAction(0);
        }
        mActions[index].mAction = mDefaultActions[index];
        connect(mActions[index].mAction, SIGNAL(changed()), SLOT(actionChanged()));
    }
}

void HbDeviceMessageBoxPrivate::initProperties()
{
    for(int i = 0; i < NumProperties; i++) {
        mProperties[i].mFlags = NoFlags;
    }
    for(int i = 0; i < NumActions; i++) {
        clearAction(mActions[i]);
    }

    QString text;
    q_ptr->setText(text);
    q_ptr->setIconName(text);
    q_ptr->setIconVisible(true);
    q_ptr->setAnimationDefinition(text);

    bool useActions[NumActions];
    for(int i = 0; i < NumActions; i++) {
        useActions[i] = false;
    }

    switch(mProperties[Type].mValue.toInt()) {
    case HbMessageBox::MessageTypeNone:
        q_ptr->setIconVisible(false);
        // Fall trough
    case HbMessageBox::MessageTypeInformation:
    case HbMessageBox::MessageTypeWarning:
        q_ptr->setDismissPolicy(HbPopup::TapAnywhere);
        q_ptr->setTimeout(timeoutValue(HbPopup::StandardTimeout));
        // HbMessageBox sets by default accept button to "Ok", reject button is empty
        mProperties[StandardButtons].mValue.setValue(static_cast<int>(HbMessageBox::Ok));
        useActions[AcceptButton] = true;
        break;
    case HbMessageBox::MessageTypeQuestion:
        q_ptr->setTimeout(HbPopup::NoTimeout);
        q_ptr->setDismissPolicy(HbPopup::NoDismiss);
        // HbMessageBox sets by default accept button to "Ok", reject button is empty.
        // Set default buttons to yes/no
        setProperty(StandardButtons, HbMessageBox::Yes|HbMessageBox::No);
        useActions[AcceptButton] = true;
        useActions[RejectButton] = true;
        break;
    default:
        Q_ASSERT(false);
    }

    for(int i = 0; i < NumActions; i++) {
        if (useActions[i]) {
            initAction(i);
        }
    }
}

void HbDeviceMessageBoxPrivate::setAction(ActionSelector select, QAction *action)
{
    TRACE_ENTRY
    Action &dialogAction = mActions[select];
    bool saveTriggered = dialogAction.mTriggered;
    clearAction(dialogAction);
    dialogAction.mTriggered = saveTriggered;
    dialogAction.mFlags = Modified;
    dialogAction.mAction = action;
    if (dialogAction.mAction) {
        connect(dialogAction.mAction, SIGNAL(changed()), SLOT(actionChanged()));
    }
    TRACE_EXIT
}

void HbDeviceMessageBoxPrivate::setStandardButtons(HbMessageBox::StandardButtons buttons)
{
    // Create actions for buttons. These will be signaled on button press.
    int buttonCount = qMin(static_cast<int>(NumActions), countBits(buttons));
    int i = 0;
    for(; i < buttonCount; i++) {
        initAction(i);
    }
    // Clear extra actions
    for(; i < NumActions; i++) {
      clearAction(mActions[i]);
    }
    setProperty(StandardButtons, buttons);
}

// Send properties to server
void HbDeviceMessageBoxPrivate::sendToServer(bool show)
{
    killTimer(mUpdateTimerId);
    mUpdateTimerId = 0;

    // If this is update but show has not been called, return.
    if (!show && !mVisible) {
      return;
    }

    // If this is update but no properties have been mofified, return
    if (!show && !propertiesModified()) {
        return;
    }

    // If this is a show, send type property
    if (show) {
        mProperties[Type].mFlags = Modified;
    }

    static const char * const propertyNames[] = {
        "type",
        "standardButtons",
        "text",
        "iconName",
        "iconVisible",
        "timeout",
        "dismissPolicy",
        "animationDefinition"
    };

    QVariantMap parameters;
    for(int i = 0; i < NumProperties; i++) {
        if (mProperties[i].mFlags & Modified) {
            if (show || !(mProperties[i].mFlags & SentToServer)) {
                parameters.insert(propertyNames[i], mProperties[i].mValue);
                mProperties[i].mFlags |= SentToServer;
            }
        }
    }
    static const char * const actionNames[] = {
        "acceptAction",
        "rejectAction"
    };
    for(int i = 0; i < NumActions; i++) {
        if (mActions[i].mFlags & Modified) {
            if (show || !(mActions[i].mFlags & SentToServer)) {
                QString actionData; // empty removes action at the plugin
                if (mActions[i].mAction) {
                    actionData.append("t:").append(mActions[i].mAction->text());
                }
                parameters.insert(actionNames[i], actionData);
                mActions[i].mFlags |= SentToServer;
            }
        }
    }

    if (show) {
        for(int i = 0; i < NumActions; i++) {
            mActions[i].mTriggered = false;
        }
        if (mDeviceDialog->show("com.nokia.hb.devicemessagebox/1.0", parameters)) {
            mVisible = true;
        } else { // failed to show device dialog. Start a one shot to emit aboutToClose() signal.
            QTimer::singleShot(0, q_ptr, SIGNAL(aboutToClose()));
        }
    } else {
        mDeviceDialog->update(parameters);
    }
}

// Check if any properties have been modified
bool HbDeviceMessageBoxPrivate::propertiesModified() const
{
    for(int i = 0; i < NumProperties; i++) {
        if ((mProperties[i].mFlags & Modified) && !(mProperties[i].mFlags & SentToServer)) {
            return true;
        }
    }
    for(int i = 0; i < NumActions; i++) {
        if ((mActions[i].mFlags & Modified) && !(mActions[i].mFlags & SentToServer)) {
            return true;
        }
    }
    return false;
}

// Clear action
void HbDeviceMessageBoxPrivate::clearAction(Action &action)
{
    if (action.mAction) {
        disconnect(action.mAction, SIGNAL(changed()), this, SLOT(actionChanged()));
    }
    action.mAction = 0;
    action.mFlags = NoFlags;
    action.mTriggered = false;

}

void HbDeviceMessageBoxPrivate::close()
{
    TRACE_ENTRY
    mDeviceDialog->cancel();
    mVisible = false;
    TRACE_EXIT
    return;
}

void HbDeviceMessageBoxPrivate::exec()
{
    TRACE_ENTRY
    sendToServer(true);
    mDeviceDialog->waitForClosed();
    TRACE_EXIT
}

void HbDeviceMessageBoxPrivate::triggerAction(QVariantMap data)
{
    TRACE_ENTRY
    const char *key = "act";
    QVariantMap::const_iterator i = data.find(key);
    if (i != data.constEnd()) {
        QAction *action;
        if (i.value().toString() == "p") {
            mActions[AcceptButton].mTriggered = true;
            action = mActions[AcceptButton].mAction;
        } else {
            mActions[RejectButton].mTriggered = true;
            action = mActions[RejectButton].mAction;
        }
        if (action) {
            action->trigger();
        }
    }
    TRACE_EXIT
}

void HbDeviceMessageBoxPrivate::actionChanged()
{
    QObject *action = sender();
    for(int i = 0; i < NumActions; i++) {
        if (mActions[i].mAction == action) {
            mActions[i].mFlags = Modified;
            scheduleUpdateEvent();
            break;
        }
    }
}

void HbDeviceMessageBoxPrivate::setProperty(PropertySelector propertySelector, int value)
{
    Property &property = mProperties[propertySelector];
    property.mValue.setValue(value);
    property.mFlags = Modified;
    scheduleUpdateEvent();
}

void HbDeviceMessageBoxPrivate::setProperty(PropertySelector propertySelector,
    const QString &value)
{
    Property &property = mProperties[propertySelector];
    property.mValue.setValue(value);
    property.mFlags = Modified;
    scheduleUpdateEvent();
}

// Schedule event to update changed properties to device dialog server. update() is not
// called after each time a property is set. Instead an event is scheduled in order to
// update all changed properties in one shot.
void HbDeviceMessageBoxPrivate::scheduleUpdateEvent()
{
    if (mVisible && mUpdateTimerId == 0) {
        mUpdateTimerId = startTimer(0);
    }
}

void HbDeviceMessageBoxPrivate::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mUpdateTimerId) {
        sendToServer(false);
    }
}

int HbDeviceMessageBoxPrivate::timeoutValue(HbPopup::DefaultTimeout timeout)
{
    TRACE_STATIC_ENTRY

    static const struct { HbPopup::DefaultTimeout timeout; int value; } timeoutValues[] = {
        {HbPopup::NoTimeout,0},
        {HbPopup::ConfirmationNoteTimeout,1500},
        {HbPopup::StandardTimeout,3000},
        {HbPopup::ContextMenuTimeout,6000},
    };
    int count = sizeof(timeoutValues) / sizeof(timeoutValues[0]);
    if (timeout < 0 || timeout >= count) {
        TRACE_EXIT
        return timeoutValues[HbPopup::NoTimeout].value;
    }
    TRACE_EXIT
    return timeoutValues[timeout].value;
}

int HbDeviceMessageBoxPrivate::countBits(unsigned int value)
{
    int count = 0;
    while(value) {
        count += value & 1;
        value >>= 1;
    }
    return count;
}

HbDeviceMessageBoxPrivate::ActionSelector HbDeviceMessageBoxPrivate::actionSelector(
    HbDeviceMessageBox::ActionRole role)
{
    static const ActionSelector selectors[] = {
        AcceptButton, RejectButton
    };
    const unsigned numSelectors = sizeof(selectors) / sizeof(selectors[0]);
    unsigned index = role;
    if (index >= numSelectors) {
        Q_ASSERT(false);
        return InvalidSelector;
    }
    else {
        return selectors[index];
    }
}

/*!
    @stable
    @hbwidgets
    \class HbDeviceMessageBox
    \brief HbDeviceMessageBox displays a message box on top of any running applications.

    HbDeviceMessageBox is a device dialog version of HbMessageBox. It displays a message box
    with text, icon or animation and optional accept and reject buttons. It is not a widget.
    The message box is displayed by a device dialog service which HbDeviceMessageBox is a
    client of.

    Device dialogs are shown on top of any running applications and are always modal by nature.

    A device message box is lauched by a show(). A new message box is lauched every time show()
    is called. aboutToClose() signal is emitted when the box has closed. There is also syncronous
    exec() function which launches a message box and waits for it close. exec() is not compatible
    with gestures and cannot be used from applications that have an user interface.

    After message box has been launched, updating it's properties causes interprocess
    communication. Therefore it's best to construct the message box fully before
    calling show(). Changing a property while a message box is displayed, causes the property
    to be updated to the displayed widget automatically next time event loop is entered
    or updated values can be sent immediately by calling update().

    Message box buttons are represented by two actions. Left button corrensponds to accept
    action and the right one to reject action. triggered() signal of action is emitted when
    corresponding button is pressed. Changing action text changes corresponding button text.
    Icons on buttons are not suppported.

    Device message box is closed when user dismisses it, when HbDeviceMessageBox::close() is
    called, timeout expires and message box closes itself or system closes the message box for
    some reason. aboutToClose() signal is always emitted after the box has closed.

    Static convenience functions are provided for launching message boxes. They launch message boxes
    containing default properties appropriate for the message box type and their contents cannot
    be updated. Information and warning convenience methods return immediately. Question waits for
    a message box to close and is not compatible with gestures.

    Four types of message boxes are predefined. The type determines a set of default properties that
    are set on construction. Below is a table listing types and their default properties.

    <table border="1">
        <caption><b>HbDeviceMessageBox types and default properties</b></caption>
        <tr><th>Type</th><th>Icon</th><th>Buttons</th><th>Timeout</th><th>Dismiss policy</th><th>Sound</th></tr>
        <tr><td>None</td><td>No icon</td><td>"Ok" button</td><td>Standard timeout</td><td>Tap anywhere</td><td>No sound</td></tr>
        <tr><td>Information</td><td>Info icon</td><td>"Ok" button</td><td>Standard timeout</td><td>Tap anywhere</td><td>Info sound</td></tr>
        <tr><td>Warning</td><td>Warning icon</td><td>"Ok" button</td><td>Standard timeout</td><td>Tap anywhere</td><td>Warn sound</td></tr>
        <tr><td>Question</td><td>Question icon</td><td>"Yes" and "No" buttons</td><td>No timeout</td><td>Button press</td><td>Question sound</td></tr>
    </table>

    In place of an icon, message box may conatain an animation. Supported icon animation formats are:
    - GIF (.gif)
    - MNG (.mng)
    - Frame animations (.axml)

    Sample code:

    \code
    // Ask from user whether to continue operation or not.
    // Uses default accept/reject buttons (yes/no).

    bool value = HbDeviceMessageBox::question("Continue operation ?");
    if (value) {
        continueOperation();
    }
    \endcode

    Modify appearance of a message box with the methods provided by the class.

    \code
    // Code below modifies the default properties of the message box.

    HbDeviceMessageBox messageBox(HbMessageBox::MessageTypeQuestion);
    messageBox.setText("End game?");
    QString fileName("note_warning");
    messageBox.setIconName(fileName);

    // Change button text
    messageBox.action(HbDeviceMessageBox::AcceptButtonRole)->setText("Ok");
    messageBox.action(HbDeviceMessageBox::RejectButtonRole)->setText("Cancel");

    // Set new actions (buttons)
    QAction acceptAction("Ok", 0);
    messageBox.setAction(&acceptAction, HbDeviceMessageBox::AcceptButtonRole);
    QAction rejectAction("Cancel", 0);
    messageBox.setAction(&rejectAction, HbDeviceMessageBox::RejectButtonRole);

    // Set standard buttons
    messageBox.setStandardButtons(HbMessageBox::Ok|HbMessageBox::Cancel);

    // Beware, application may exit during exec().
    // Beware, exec() is not compatible with gestures.
    QAction *result = messageBox.exec();
    bool quit = messageBox.isAcceptAction(result);
    if (quit) {
        quit();
    }
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
    QString animationDefinitionXML("c:\animation.axml");
    QString logicalIconName("frame_anim_looping");

    msg->setAnimationDefinition(animationDefinitionXML);
    msg->setIconName(logicalIconName);
    msg->show();
    \endcode

    \sa HbMessageBox, HbDialog, HbDeviceDialog
*/

/*!
    \fn void HbDeviceMessageBox::aboutToClose();

    This signal is emitted when the message box has closed. If a button was pressed,
    corresponding action's triggered() signal is emitted before this signal.
 */

/*!
    \enum HbDeviceMessageBox::ActionRole
    Defines roles for actions set into a message box.
*/
/*!
    \var HbDeviceMessageBox::ActionRole HbDeviceMessageBox::InvalidRole
    No role.
*/
/*!
    \var HbDeviceMessageBox::ActionRole HbDeviceMessageBox::AcceptButtonRole
    Accept button action.
*/
/*!
    \var HbDeviceMessageBox::ActionRole HbDeviceMessageBox::RejectButtonRole
    Reject button action.
*/

/*!
    Constructor.
    \param type Type of the message box.
    \param parent An optional parameter.
*/
HbDeviceMessageBox::HbDeviceMessageBox(HbMessageBox::MessageBoxType type, QObject *parent) :
    QObject(parent), d_ptr(new HbDeviceMessageBoxPrivate)
{
    TRACE_ENTRY
    d_ptr->q_ptr = this;
    d_ptr->mProperties[HbDeviceMessageBoxPrivate::Type].mValue.setValue(static_cast<int>(type));
    d_ptr->init();
    TRACE_EXIT
}

/*!
    Constructor.
    \param text Message box text.
    \param type Type of the message box.
    \param parent An optional parameter.
*/
HbDeviceMessageBox::HbDeviceMessageBox(const QString &text, HbMessageBox::MessageBoxType type,
    QObject *parent) : QObject(parent), d_ptr(new HbDeviceMessageBoxPrivate)
{
    TRACE_ENTRY
    d_ptr->q_ptr = this;
    d_ptr->mProperties[HbDeviceMessageBoxPrivate::Type].mValue.setValue(static_cast<int>(type));
    d_ptr->init();
    setText(text);
    TRACE_EXIT
}

/*!
    Destructs the class.
*/
HbDeviceMessageBox::~HbDeviceMessageBox()
{
    TRACE_ENTRY
    delete d_ptr;
    TRACE_EXIT
}

/*!
    Shows a message box and returns immediately without waiting for it to close. Closing of the
    message box is indicated by aboutToClose() signal. Button presses are indicated by
    QAction::triggered() signals. The message box can be updated while showing by property
    setters.

    \sa update(), aboutToClose()
*/
void HbDeviceMessageBox::show()
{
    TRACE_ENTRY
    d_ptr->sendToServer(true);
    TRACE_EXIT
}

/*!
    Updates changed properties to a showing message box via interprocess communication.
    Has no effect if show() has not been called or the message box has closed already.
    Calling update() is optional as setting any property schedules an event and the
    showing message box is updated next time Qt event loop executes.

    \sa show()
*/
void HbDeviceMessageBox::update()
{
    d_ptr->sendToServer(false);
}

/*!
    Closes a message box shown asynchronously.
*/
void HbDeviceMessageBox::close()
{
    TRACE_ENTRY
    d_ptr->close();
    TRACE_EXIT
}

/*!
    Executes the dialog synchronously.

    Returns a pointer to an action corresponding to a button pressed. Returns 0 if no button
    was pressed (the dialog closed for other reason).

    This functions starts a new event loop. Consider following caveats before using it.
    Stack usage increases. Depending on application program flow, several event
    loops may get instantiated on top of each other. Application event processing continues while
    exec() executes. When it returns application state may have changed. For example some
    objects may have been deleted or application may have exited.

    <b>Note that starting an event loop isn't compatible with gestures.</b> Therefore if an application
    has an user interface, please don't use this function. Instead connect to signals and use
    asynchronous show().

    \sa action(), setAction()
*/
const QAction *HbDeviceMessageBox::exec()
{
    TRACE_ENTRY
    const QAction *action;
    QPointer<HbDeviceMessageBoxPrivate> guard = d_ptr;
    d_ptr->exec();
    if (guard.isNull()) {
        action = 0;
    } else {
        action = triggeredAction();
    }
    TRACE_EXIT
    return action;
}

/*!
    Returns an action user triggered causing the dialog to close. Returns 0 if none of the actions were
    triggered and dialog was closed for other reason.

    \sa show(), exec(), action(), isAcceptAction()
*/
const QAction *HbDeviceMessageBox::triggeredAction() const
{
    for(int i = 0; i < HbDeviceMessageBoxPrivate::NumActions; i++) {
        if (d_ptr->mActions[i].mTriggered) {
            return d_ptr->mActions[i].mAction;
        }
    }
    return 0;
}

/*!
    Returns true if an action is an accept action. This function should be used to check whether
    user accepted message box by triggering accept action. In the future there could be several
    accept actions instead of only one.

    \sa exec(), action(), triggeredAction()
*/
bool HbDeviceMessageBox::isAcceptAction(const QAction *action) const
{
    return action && action == d_ptr->mActions[HbDeviceMessageBoxPrivate::AcceptButton].mAction;
}

/*!
    Sets message box type. All message box properties are reset to a default values for the type.
    Type of a showing message box cannot be changed on the fly. show() must be called to launch
    a new message box after setMessageBoxType() has been called.

    \param type Message box type.

    \sa setMessageBoxType()
*/
void HbDeviceMessageBox::setMessageBoxType(HbMessageBox::MessageBoxType type)
{
    TRACE_ENTRY
    // After setType(), a new message is launched by a show()
    d_ptr->mVisible = false;
    d_ptr->mProperties[HbDeviceMessageBoxPrivate::Type].mValue.setValue(static_cast<int>(type));
    // All properties initialized to default
    d_ptr->initProperties();
    TRACE_EXIT
}

/*!
    Returns message box type.

    \sa setMessageBoxType()
*/
HbMessageBox::MessageBoxType HbDeviceMessageBox::messageBoxType() const
{
    return static_cast<HbMessageBox::MessageBoxType>
        (d_ptr->mProperties[HbDeviceMessageBoxPrivate::Type].mValue.toInt());
}

/*!
    Sets message box buttons to standard buttons.

    \param buttons Message box buttons. A combination of flags,
    eg. HbMessageBox::Yes | HbMessageBox::No. Button flags are scanned starting from lsb.
    First button found goes to accept position and so forth.

    \sa standardButtons()
*/
void HbDeviceMessageBox::setStandardButtons(HbMessageBox::StandardButtons buttons)
{
    TRACE_ENTRY
    d_ptr->setStandardButtons(buttons);
    d_ptr->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Returns standard buttons set to a message box. A default value for question message box is
    HbMessageBox::Yes|HbMessageBox::No. For all other message box types the default is
    HbMessageBox::Ok.

    \sa setStandardButtons()
*/
HbMessageBox::StandardButtons HbDeviceMessageBox::standardButtons() const
{
    return static_cast<HbMessageBox::StandardButtons>
        (d_ptr->mProperties[HbDeviceMessageBoxPrivate::StandardButtons].mValue.toInt());
}

/*!
    Convenience method for showing question message box and waiting for it to close.

    This functions starts a new event loop. Consider following caveats before using it.
    Stack usage increases. Depending on application program flow, several event
    loops may get instantiated on top of each other. Application event processing continues while
    question() executes. When it returns application state may have changed. For example some
    objects may have been deleted or application may have exited.

    <b>Note that starting an event loop isn't compatible with gestures.</b> Therefore if an application
    has an user interface, please don't use this function. Instead connect to signals and use
    asynchronous show().

    \param text Message box text.
    \param acceptButtonText Accept button text. If string is null, a default (Yes) button is used.
    \param rejectButtonText Reject button text. If string is null, a default (No) button is used.

    Returns true if user pressed accept button, otherwise false.

    \sa show()
*/
bool HbDeviceMessageBox::question(
    const QString &text,
    const QString &acceptButtonText,
    const QString &rejectButtonText)
{
    TRACE_STATIC_ENTRY
    HbDeviceMessageBox messageBox(HbMessageBox::MessageTypeQuestion);
    messageBox.setText(text);
    messageBox.setStandardButtons(HbMessageBox::Yes|HbMessageBox::No);
    if (!acceptButtonText.isNull()) {
        messageBox.action(AcceptButtonRole)->setText(acceptButtonText);
    }
    if (!rejectButtonText.isNull()) {
        messageBox.action(RejectButtonRole)->setText(rejectButtonText);
    }
    messageBox.exec();
    TRACE_EXIT
    // Return true if accept action was triggered
    return messageBox.isAcceptAction(messageBox.triggeredAction());
}

/*!
    Convenience method for showing question message box and waiting for it to close.

    This functions starts a new event loop. Consider following caveats before using it.
    Stack usage increases. Depending on application program flow, several event
    loops may get instantiated on top of each other. Application event processing continues while
    question() executes. When it returns application state may have changed. For example some
    objects may have been deleted or application may have exited.

    <b>Note that starting an event loop isn't compatible with gestures.</b> Therefore if an application
    has an user interface, please don't use this function. Instead connect to signals and use
    asynchronous show().

    \param text Message box text.
    \param buttons Specifies message box buttons. See setStandardButtons() for format.

    Returns true if user pressed accept button, otherwise false.

    \sa show()
*/
bool HbDeviceMessageBox::question(const QString &text, HbMessageBox::StandardButtons buttons)
{
    TRACE_STATIC_ENTRY
    HbDeviceMessageBox messageBox(HbMessageBox::MessageTypeQuestion);
    messageBox.setText(text);
    messageBox.setStandardButtons(buttons);
    messageBox.exec();
    TRACE_EXIT
    // Return true if accept action was triggered
    return messageBox.isAcceptAction(messageBox.triggeredAction());
}

/*!
    Convenience method for showing information message box. Launches a message box and
    returns immediately.

    \param text Message box text.
*/
void HbDeviceMessageBox::information(const QString &text)
{
    TRACE_STATIC_ENTRY
    HbDeviceMessageBox(text, HbMessageBox::MessageTypeInformation).show();
    TRACE_EXIT
}

/*!
    Convenience method for showing warning message box. Launches a message box and returns
    immediately.

    \param text Message box text.
*/
void HbDeviceMessageBox::warning(const QString &text)
{
    TRACE_STATIC_ENTRY
    HbDeviceMessageBox(text, HbMessageBox::MessageTypeWarning).show();
    TRACE_EXIT
}

/*!
    Returns device message box action. The action returned may be a default
    action that message box has set at constructor or one set by setAction().

    \param role Selects an action to get.

    \sa setAction()
*/
QAction *HbDeviceMessageBox::action(ActionRole role) const
{
    HbDeviceMessageBoxPrivate::ActionSelector  actionSelector =
        HbDeviceMessageBoxPrivate::actionSelector(role);
    return actionSelector != HbDeviceMessageBoxPrivate::InvalidSelector ?
        d_ptr->mActions[actionSelector].mAction : 0;
}

/*!
    Sets an action into device message box. Action role is either an
    accept (left button in left-to-right layout) or reject (right button).
    Action can be null which removes corresponding button from the message
    box. Only text of the action is shown in the message box button. Icon
    of the action is not used. The message box does not take ownership of
    the action.

    HbDeviceMessageBox constructor sets a default accept and reject actions into
    question message box. Other message boxes have only accept action by default.
    Default actions are owned and deleted by the message box.

    \param action Action or null. Ownership does not transfer.
    \param role Selects an action.

    \sa action()
*/
void HbDeviceMessageBox::setAction(QAction *action, ActionRole role)
{
    TRACE_ENTRY
    HbDeviceMessageBoxPrivate::ActionSelector  actionSelector =
        HbDeviceMessageBoxPrivate::actionSelector(role);
    if (actionSelector != HbDeviceMessageBoxPrivate::InvalidSelector) {
        d_ptr->setAction(HbDeviceMessageBoxPrivate::actionSelector(role), action);
        d_ptr->scheduleUpdateEvent();
    }
    TRACE_EXIT
}

/*!
    Sets message box text. Supported text formats are the same as HbMessageBox::setText().

    \param text Message box text.

    \sa text(), HbMessageBox::setText()
*/
void HbDeviceMessageBox::setText(const QString &text)
{
    TRACE_ENTRY
    d_ptr->setProperty(HbDeviceMessageBoxPrivate::Text, text);
    TRACE_EXIT
}

/*!
    Returns message box text.

    \sa setText()
*/
QString HbDeviceMessageBox::text() const
{
    return d_ptr->mProperties[HbDeviceMessageBoxPrivate::Text].mValue.toString();
}

/*!
    Sets message box icon name or animation logical name.

    \param aIconName Icon name. Icon can be from Hb resources or themes. Or can be a file in
    a file system.

    \sa IconName()
*/
void HbDeviceMessageBox::setIconName(const QString &iconName)
{
    TRACE_ENTRY
    d_ptr->setProperty(HbDeviceMessageBoxPrivate::IconName, iconName);
    TRACE_EXIT
}

/*!
    Returns name and path of the icon or animation's logical name.

    \sa setIconName()
*/
QString HbDeviceMessageBox::iconName() const
{
    return d_ptr->mProperties[HbDeviceMessageBoxPrivate::IconName].mValue.toString();
}

/*!
    Enables/disables icon visibility.

    \param visible Enables icon visibility.

    \sa iconVisible()
*/
void HbDeviceMessageBox::setIconVisible(bool visible)
{
    TRACE_ENTRY
    d_ptr->setProperty(HbDeviceMessageBoxPrivate::IconVisible, visible);
    TRACE_EXIT
}

/*!
    Returns true if icon is visible.

    \sa setIconVisible()
*/
bool HbDeviceMessageBox::iconVisible() const
{
    return d_ptr->mProperties[HbDeviceMessageBoxPrivate::IconVisible].mValue.toBool();
}

/*!
    Sets message box timeout.

    \param timeout Timeout in milliseconds. 0 denotes no timeout (infinite).

    \sa timeout() setTimeout(HbPopup::DefaultTimeout)
*/
void HbDeviceMessageBox::setTimeout(int timeout)
{
    TRACE_ENTRY
    d_ptr->setProperty(HbDeviceMessageBoxPrivate::Timeout, timeout);
    TRACE_EXIT
}

/*!
    Sets timeout to one of HbPopup::DefaultTimeout values. Helps achieving
    common look and feel of message boxes.

    \param timeout Timeout as an enumerated constant.

    \sa enum HbPopup::DefaultTimeout timeout() setTimeout(int)
*/
void HbDeviceMessageBox::setTimeout(HbPopup::DefaultTimeout timeout)
{
    TRACE_ENTRY
    setTimeout(HbDeviceMessageBoxPrivate::timeoutValue(timeout));
    TRACE_EXIT
}

/*!
    Returns message box timeout in milliseconds. Default value depends on message box type.
    Question message box has a default of HbPopup::NoTimeout. All other boxes, the default is
    HbPopup::StandardTimeout.

    \sa setTimeout()
*/
int HbDeviceMessageBox::timeout() const
{
    return d_ptr->mProperties[HbDeviceMessageBoxPrivate::Timeout].mValue.toInt();
}

/*!
    Sets message box dismiss policy.

    \param dismissPolicy Dismiss policy.

    \sa dismissPolicy()
*/
void HbDeviceMessageBox::setDismissPolicy(HbPopup::DismissPolicy dismissPolicy)
{
    TRACE_ENTRY
    d_ptr->setProperty(HbDeviceMessageBoxPrivate::DismissPolicy, dismissPolicy);
    TRACE_EXIT
}

/*!
    Returns dismiss policy of a message box. Default depends on message box type.
    Question box has default HbPopup::NoDismiss and all other boxes HbPopup::TapAnywhere.

    \sa setDismissPolicy()
*/
HbPopup::DismissPolicy HbDeviceMessageBox::dismissPolicy() const
{
    return static_cast<HbPopup::DismissPolicy>
        (d_ptr->mProperties[HbDeviceMessageBoxPrivate::DismissPolicy].mValue.toInt());
}

/*!
    Sets animation a message box. Animation's logical name has to be set
    using setIcon(). Animation definition files must be stored to a place where they
    can be accessed by device dialog service.

    Supported animation formats are following:
    - GIF (.gif)
    - MNG (.mng)
        - Frame animations

    \param animationDefinition Path and name of the animation definition file.

    \sa setIcon(), animationDefinition(), HbIconAnimationManager::addDefinitionFile()
*/
void HbDeviceMessageBox::setAnimationDefinition(QString &animationDefinition)
{
    TRACE_ENTRY
    d_ptr->setProperty(HbDeviceMessageBoxPrivate::AnimationDefinition, animationDefinition);
    TRACE_EXIT
}

/*!
    Returns animation definition file name.

    \sa setAnimationDefinition()
*/
QString HbDeviceMessageBox::animationDefinition() const
{
    return d_ptr->mProperties[HbDeviceMessageBoxPrivate::AnimationDefinition].mValue.toString();
}
