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
#include <hbaction.h>
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

void HbDeviceMessageBoxPrivate::initProperties()
{
    for(int i = 0; i < NumProperties; i++) {
        mProperties[i].mFlags = NoFlags;
    }
    clearActions();

    QString text;
    q_ptr->setText(text);
    q_ptr->setIconName(text);
    q_ptr->setIconAlignment(Qt::AlignCenter);
    q_ptr->setIconVisible(true);
    q_ptr->setAnimationDefinition(text);

    switch(mProperties[Type].mValue.toInt()) {
    case HbMessageBox::MessageTypeInformation:
    case HbMessageBox::MessageTypeWarning:
        q_ptr->setDismissPolicy(HbPopup::TapAnywhere);
        q_ptr->setTimeout(timeoutValue(HbPopup::StandardTimeout));
        // Use default primary button, secondary button is empty
        if (!mDefaultActions[AcceptButton]) {
            mDefaultActions[AcceptButton] = new HbAction(0);
        }
        mActions[AcceptButton].mAction = mDefaultActions[AcceptButton];
        break;
    case HbMessageBox::MessageTypeQuestion:
        q_ptr->setTimeout(HbPopup::NoTimeout);
        q_ptr->setDismissPolicy(HbPopup::NoDismiss);
        // Use default primary and secondary buttons
        for(int i = 0; i < NumActions; i++) {
            if (!mDefaultActions[i]) {
                mDefaultActions[i] = new HbAction(0);
            }
            mActions[i].mAction = mDefaultActions[i];
        }
        break;
    default:
        Q_ASSERT(false);
    }
}

void HbDeviceMessageBoxPrivate::setAction(ActionSelector select, QAction *action)
{
    TRACE_ENTRY
    Action &dialogAction = mActions[select];
    dialogAction.mFlags = Modified;
    dialogAction.mAction = action;
    TRACE_EXIT
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
        "text",
        "iconName",
        "iconAlignment",
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
        "primaryActionText",
        "secondaryActionText"
    };
    static const char * const nullActionNames[] = {
        "primaryActionNull",
        "secondaryActionNull"
    };

    for(int i = 0; i < NumActions; i++) {
        if (mActions[i].mFlags & Modified) {
            if (show || !(mActions[i].mFlags & SentToServer)) {
                if (mActions[i].mAction) {
                    parameters.insert(actionNames[i], mActions[i].mAction->text());
                } else {
                    parameters.insert(nullActionNames[i], true);
                }
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

// Clear actions
void HbDeviceMessageBoxPrivate::clearActions()
{
    for(int i = 0; i < NumActions; i++) {
        mActions[i].mAction = 0;
        mActions[i].mFlags = NoFlags;
        mActions[i].mTriggered = false;
    }
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

// Temporary until HbAction is deprecated out from device message box API
// Tbd. remove when deprecation period ends
HbAction *HbDeviceMessageBoxPrivate::toHbAction(QAction *action)
{
    // Upcast to HbAction
    HbAction *hbAction = qobject_cast<HbAction*>(action);
    // Warn if user has mixed HbAction and QAction APis
    if (action != hbAction) {
        //qWarning("HbDeviceMessageBox: HbAction API:s deprecated! Returning QAction instead of HbAction");
        // Best option is to return QAction and hope caller doesn't use HbAction parts
        hbAction = reinterpret_cast<HbAction*>(action);
    }
    return hbAction;
}

/*!
    \class HbDeviceMessageBox
    \brief HbDeviceMessageBox is a device dialog version of HbMessageBox.

    It displays a message box with text, icon or animation and optional reply button(s).

    Device dialogs are shown on top of any running applications and are always modal by nature.

    HbDeviceMessageBox provides a similar kind of interface as HbMessageBox,
    excluding functions which handle concrete UI-component related information.

    A device message box is launched when exec() for synchronous dialog or show() for asynchronous
    dialog is called. Launched dialog can be updated by setters. Because updating a dialog
    requires interprocess communication, it's advisable to fully construct the device message box before
    calling show(). Device message box is closed when user dismisses it with pressing a button, when
    client calls close() or the dialog reaches timeout. If the system must close the device message
    box while it is executing, it will have the same effect than having message box's secondary action
    activated.

    Static convenience functions are provided for launching message boxes.
    Dialogs created by them contain a default property values appropriate for
    the message box type and their contents cannot be updated. Information and
    warning convenience methods return immediately. Question waits for a message box
    to close.

    Supported icon animation formats are following:
    - GIF (.gif)
    - MNG (.mng)
        - Frame animations

    Sample code:

    \code
    // Ask from user whether to continue operation or not.
    // Uses default accept/reject buttons (yes/no).

    bool value = HbDeviceMessageBox::question("Continue operation ?");
    if (value) {
        continueOperation();
    }
    \endcode

    Alter the appearance of the message box with the methods provided by the class.

    \code
    // Code below modifies the default properties of the message box.

    HbDeviceMessageBox messageBox(HbMessageBox::MessageTypeQuestion);
    messageBox.setText("End game?");
    QString fileName("note_warning");
    messageBox.setIconName(fileName);
    Qt::Alignment align(Qt::AlignLeft|Qt::AlignTop);
    messageBox.setIconAlignment(align);

    QAction acceptAction("Ok", 0);
    messageBox.setAction(&acceptAction, HbDeviceMessageBox::AcceptButtonRole);
    QAction rejectAction("Cancel", 0);
    messageBox.setAction(&rejectAction, HbDeviceMessageBox::RejectButtonRole);

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

    @proto
    @hbwidgets
*/

/*!
    \fn void HbDeviceMessageBox::aboutToClose();

    This signal is emitted when the device message box is about to be closed i.e.
    when the question type of message is replied to by the user.
 */

/*!
    \enum HbDeviceMessageBox::ActionRole
    Defines roles for actions set into a message box.
*/
/*!
    \var HbDeviceMessageBox::ActionRole HbDeviceMessageBox::InvalidRole
    No action.
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
    \param text Text can be set in the constructor.
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
    Executes the dialog asynchronously.
*/
void HbDeviceMessageBox::show()
{
    TRACE_ENTRY
    d_ptr->sendToServer(true);
    TRACE_EXIT
}

/*!
    Updates changed properties of a launched message box to device dialog service using
    interprocess communication. Has no effect if show() has not been called or dialog has
    closed already. Calling show() is optional as updating any property schedules an event
    and the dialog is updated next time Qt event loop executes.

    \sa show()
*/
void HbDeviceMessageBox::update()
{
    d_ptr->sendToServer(false);
}

/*!
    Closes the dialog if the dialog is shown asynchronously.
*/
void HbDeviceMessageBox::close()
{
    TRACE_ENTRY
    d_ptr->close();
    TRACE_EXIT
}

/*!
    Executes the dialog synchronously.

    Returns a pointer to the action that was activated by the user, i.e. dialog's
    primary or secondary action.

    This functions starts a new event loop. Consider following caveats before using it
    Stack usage increases. Depending on application program flow, several event
    loops may get instantiated on top of each other. Application event processing continues while
    exec() executes. When it returns application state may have changed. For example some
    objects may have been deleted or application may have exited.

    \sa primaryAction(), setPrimaryAction(), secondaryAction(), setSecondaryAction()

    <b>Note that starting an event loop isn't compatible with gestures.</b> Therefore if an application
    has an user interface, please don't use this function. Instead connect to signals and use
    asynchronous show().

    \deprecated HbDeviceMessageBox::exec()
    is deprecated. Replaced by const QAction *HbDeviceMessageBox::exec() const.

*/
HbAction *HbDeviceMessageBox::exec()
{
    TRACE_ENTRY
    HbDeviceMessageBox &box = const_cast<HbDeviceMessageBox&>(*this);
    QAction *action;
    QPointer<HbDeviceMessageBoxPrivate> guard = d_ptr;
    box.d_ptr->exec();
    if (guard.isNull()) {
        action = 0;
    } else if (box.d_ptr->mActions[HbDeviceMessageBoxPrivate::AcceptButton].mTriggered) {
        action = box.d_ptr->mActions[HbDeviceMessageBoxPrivate::AcceptButton].mAction;
    } else {
        action = box.d_ptr->mActions[HbDeviceMessageBoxPrivate::RejectButton].mAction;
    }
    TRACE_EXIT
    return HbDeviceMessageBoxPrivate::toHbAction(action);
}

/*!
    Executes the dialog synchronously.

    Returns a pointer to the action that was activated by the user, i.e. dialog's
    accept or reject action. Return 0 if object was deleted during a call.

    This functions starts a new event loop. Consider following caveats before using it
    Stack usage increases. Depending on application program flow, several event
    loops may get instantiated on top of each other. Application event processing continues while
    exec() executes. When it returns application state may have changed. For example some
    objects may have been deleted or application may have exited.

    <b>Note that starting an event loop isn't compatible with gestures.</b> Therefore if an application
    has an user interface, please don't use this function. Instead connect to signals and use
    asynchronous show().

    \sa action(), setAction()
*/
const QAction *HbDeviceMessageBox::exec() const
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
    A show() must be called to launch a message box after setMessageBoxType() has been called.

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
    Convenience method for showing question message box and waiting for it to close.

    This functions starts a new event loop. Consider following caveats before using it
    Stack usage increases. Depending on application program flow, several event
    loops may get instantiated on top of each other. Application event processing continues while
    question() executes. When it returns application state may have changed. For example some
    objects may have been deleted or application may have exited.

    <b>Note that starting an event loop isn't compatible with gestures.</b> Therefore if an application
    has an user interface, please don't use this function. Instead connect to signals and use
    asynchronous show().

    \param text - text shown in message box.
    \param acceptButtonText Accept button text. If string is null, a default button is used.
    \param rejectButtonText Reject button text. If string is null, a default button is used.

    Returns true if user triggered accept action, otherwise false.

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

    if (!acceptButtonText.isNull()) {
        messageBox.setAction(new QAction(acceptButtonText, &messageBox), AcceptButtonRole);
    }
    if (!rejectButtonText.isNull()) {
        messageBox.setAction(new QAction(rejectButtonText, &messageBox), RejectButtonRole);
    }

    // Tbd. Temporary until deprecated HbAction *HbDeviceMessageBox::exec() is removed.
    const HbDeviceMessageBox &tmp = const_cast<const HbDeviceMessageBox&>(messageBox);
    tmp.exec();
    TRACE_EXIT
    // Return true if accept action was triggered
    return messageBox.isAcceptAction(messageBox.triggeredAction());
}

/*!
    Convenience method for showing information message box. Launches a message box and
    returns immediately.

    \param text - text shown in message box.
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

    \param text - text shown in message box.
*/
void HbDeviceMessageBox::warning(const QString &text)
{
    TRACE_STATIC_ENTRY
    HbDeviceMessageBox(text, HbMessageBox::MessageTypeWarning).show();
    TRACE_EXIT
}

/*!
    Returns device message box primary action.

    \sa setPrimaryAction()

    \deprecated HbDeviceMessageBox::primaryAction()
    is deprecated. Replaced by HbDeviceMessageBox::action(ActionRole role) const.

*/
HbAction* HbDeviceMessageBox::primaryAction() const
{
    //qWarning("HbDeviceMessageBox: HbAction API:s deprecated! Use action()");
    return HbDeviceMessageBoxPrivate::toHbAction(
        d_ptr->mActions[HbDeviceMessageBoxPrivate::AcceptButton].mAction);
}

/*!
    Sets the given action to the device message box.
    Action is added to the left side of the dialog if the layout direction of the
    application is left-to-right and in the vice-versa if the layout direction
    of the application is right-to-left. Action can be set to null which means
    that action and a button related to this action are removed from the dialog.
    Only the text of the action is shown in the message box. Icon of the action
    is not used. The message box does not take ownership of the given QAction.

    HbDeviceMessageBox constructor sets a default action into the message box.

    \param action Action or null. Ownership doesn't transfer.

    \sa primaryAction()

    \deprecated HbDeviceMessageBox::setPrimaryAction(HbAction *action)
    is deprecated. HbDeviceMessageBox::setAction(QAction *action, ActionRole role).

*/
void HbDeviceMessageBox::setPrimaryAction(HbAction *action)
{
    TRACE_ENTRY
    //qWarning("HbDeviceMessageBox: HbAction API:s deprecated! Use setAction()");
    d_ptr->setAction(HbDeviceMessageBoxPrivate::AcceptButton, action);
    d_ptr->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Returns device message box secondary action.

    \sa setSecondaryAction()

    \deprecated HbDeviceMessageBox::secondaryAction()
    is deprecated. Replaced by HbDeviceMessageBox::action(ActionRole role) const.

*/
HbAction* HbDeviceMessageBox::secondaryAction() const
{
    //qWarning("HbDeviceMessageBox: HbAction API:s deprecated! Use action()");
    return HbDeviceMessageBoxPrivate::toHbAction(
        d_ptr->mActions[HbDeviceMessageBoxPrivate::RejectButton].mAction);
}

/*!
    Adds the given action to the device message box.
    Action is added to the right side of the dialog if the layout direction of the
    application is left-to-right and in the vice-versa if the layout direction
    of the application is right-to-left. Action can be set to null which means
    that action and a button related to this action are removed from the dialog.
    Only the text of the action is shown in the message box. Icon of the action
    is not used. The message box does not take ownership of given QAction.

    HbDeviceMessageBox constructor sets a default action into question message
    box. Information and warning message boxes don't have a default secondary action.

    \param action Action or null.

    \sa secondaryAction()

    \deprecated HbDeviceMessageBox::setSecondaryAction(HbAction *action)
    is deprecated. HbDeviceMessageBox::setAction(QAction *action, ActionRole role).
*/
void HbDeviceMessageBox::setSecondaryAction(HbAction *action)
{
    TRACE_ENTRY
    //qWarning("HbDeviceMessageBox: HbAction API:s deprecated! Use setAction()");
    d_ptr->setAction(HbDeviceMessageBoxPrivate::RejectButton, action);
    d_ptr->scheduleUpdateEvent();
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
    Sets an action into the device message box. Action role is either an
    accept (left button in left-to-right layout) or reject (right button).
    Action can be null which removes corresponding button from the message
    box. Only text of the action is shown in the message box button. Icon
    of the action is not used. The message box does not take ownership of
    the action.

    HbDeviceMessageBox constructor sets a default accept and reject actions into
    question message box. Information and warning message boxes have only accept
    action by default. Default actions are owned and deleted by the message box.

    \param action Action or null. Ownership does not transfer.
    \param role Selects an action to set.

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
    Sets text of the message dialog.

    \param text Message text.

    \sa text()
*/
void HbDeviceMessageBox::setText(const QString &text)
{
    TRACE_ENTRY
    d_ptr->setProperty(HbDeviceMessageBoxPrivate::Text, text);
    TRACE_EXIT
}

/*!
    Returns text of the dialog.

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
    Sets the icon's alignment in the icon area of the dialog.

    \param align Qt defined alignment options can used.

    \sa iconAlignment()
*/
void HbDeviceMessageBox::setIconAlignment(Qt::Alignment align)
{
    TRACE_ENTRY
    d_ptr->setProperty(HbDeviceMessageBoxPrivate::IconAlignment, align);
    TRACE_EXIT
}

/*!
    Returns the icon alignment in the icon area of the dialog.

    The default value is \c Qt::AlignCenter.

    \sa setIconAlignment()
*/
Qt::Alignment HbDeviceMessageBox::iconAlignment() const
{
    return static_cast<Qt::Alignment>
        (d_ptr->mProperties[HbDeviceMessageBoxPrivate::IconAlignment].mValue.toInt());
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
    Sets the timeout property in milliseconds.
    If timeout <= 0 then the note is permanent and not closed automatically.

    \param timeout Timeout in milliseconds.

    \sa timeout() setTimeout(HbPopup::DefaultTimeout)
*/
void HbDeviceMessageBox::setTimeout(int timeout)
{
    TRACE_ENTRY
    d_ptr->setProperty(HbDeviceMessageBoxPrivate::Timeout, timeout);
    TRACE_EXIT
}

/*!
    This is a convenience overload of \a timeout() for setting HbPopup::DefaultTimeout values
    to achive common look & feel.

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
    Returns the timeout property in milliseconds.
    If this property is not set the default is HbPopup::StandardTimeout except for
    HbMessageBox::MessageTypeQuestion type of message box for which default is HbPopup::NoTimeout.

    \sa setTimeout()
*/
int HbDeviceMessageBox::timeout() const
{
    return d_ptr->mProperties[HbDeviceMessageBoxPrivate::Timeout].mValue.toInt();
}

/*!
    Sets the dismiss policy for the dialog.

    \param dismissPolicy Dismiss policy of the message box.

    \sa dismissPolicy()
*/
void HbDeviceMessageBox::setDismissPolicy(HbPopup::DismissPolicy dismissPolicy)
{
    TRACE_ENTRY
    d_ptr->setProperty(HbDeviceMessageBoxPrivate::DismissPolicy, dismissPolicy);
    TRACE_EXIT
}

/*!
    Returns the dismiss policy of the dialog.
    Default is HbPopup::TapAnywhere except for HbMessageBox::MessageTypeQuestion type of
    message box for which default is HbPopup::NoDismiss.

    \sa setDismissPolicy()
*/
HbPopup::DismissPolicy HbDeviceMessageBox::dismissPolicy() const
{
    return static_cast<HbPopup::DismissPolicy>
        (d_ptr->mProperties[HbDeviceMessageBoxPrivate::DismissPolicy].mValue.toInt());
}

/*!
    Sets animation definition to a dialog. Animation's logical name has to be set
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
