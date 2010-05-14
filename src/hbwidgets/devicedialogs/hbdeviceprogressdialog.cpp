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
#include "hbdeviceprogressdialog.h"
#include "hbdeviceprogressdialog_p.h"

#include <QTimer>
#include <QAction>

HbDeviceProgressDialogPrivate::HbDeviceProgressDialogPrivate() : QObject(),
    mVisible(false),
    mUpdateTimerId(0)
{
    TRACE_ENTRY
    for(int i = 0; i < NumActions; i++) {
        mDefaultActions[i] = 0;
    }
    TRACE_EXIT
}

HbDeviceProgressDialogPrivate::~HbDeviceProgressDialogPrivate()
{
    TRACE_ENTRY
    cancel();
    killTimer(mUpdateTimerId);
    for(int i = 0; i < NumActions; i++) {
        delete mDefaultActions[i];
    }
    TRACE_EXIT
}

void HbDeviceProgressDialogPrivate::initProperties(HbProgressDialog::ProgressDialogType type)
{
    for(int i = 0; i < NumProperties; i++) {
        mProperties[i].mFlags = NoFlags;
    }
    clearActions();
    if (!mDefaultActions[CancelButton]) {
        mDefaultActions[CancelButton] = new QAction(0);
    }
    mActions[CancelButton].mAction = mDefaultActions[CancelButton];

    QString text;
    mProperties[ProgressType].mValue.setValue(static_cast<int>(type));
    mProperties[ProgressType].mFlags = Modified;
    if (type == HbProgressDialog::WaitDialog){
        q->setRange(0,0);
        q->setAutoClose(false);
    } else {
        q->setRange(0,100);
        q->setAutoClose(true);
    }
    q->setProgressValue(0);
    q->setText(text);
    q->setIconName(text);
    q->setAnimationDefinition(text);
}

// Send properties to server
void HbDeviceProgressDialogPrivate::sendToServer(bool show)
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

    static const char * const propertyNames[] = {
        "progressDialogType",
        "maximum",
        "minimum",
        "value",
        "autoClose",
        "text",
        "iconName",
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
        "cancelAction"
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
        if (mDeviceDialog.show("com.nokia.hb.deviceprogressdialog/1.0", parameters)) {
            mVisible = true;
        } else {
            // Failed to show the device dialog. Start a one shot to emit aboutToClose() signal.
            QTimer::singleShot(0, this, SLOT(aboutToClose()));
        }
    } else {
        mDeviceDialog.update(parameters);
    }
}

// Check if any properties have been modified
bool HbDeviceProgressDialogPrivate::propertiesModified() const
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
void HbDeviceProgressDialogPrivate::clearActions()
{
    for(int i = 0; i < NumActions; i++) {
        mActions[i].mAction = 0;
        mActions[i].mFlags = NoFlags;
        mActions[i].mTriggered = false;
    }
}

// Set int property
void HbDeviceProgressDialogPrivate::setProperty(PropertySelector propertySelector, int value)
{
    Property &property = mProperties[propertySelector];
    property.mValue.setValue(value);
    property.mFlags = Modified;
    scheduleUpdateEvent();
}

// Set string property
void HbDeviceProgressDialogPrivate::setProperty(PropertySelector propertySelector,
    const QString &value)
{
    Property &property = mProperties[propertySelector];
    property.mValue.setValue(value);
    property.mFlags = Modified;
    scheduleUpdateEvent();
}

void HbDeviceProgressDialogPrivate::init(HbProgressDialog::ProgressDialogType type)
{
    TRACE_ENTRY

    initProperties(type);
    connect(&mDeviceDialog, SIGNAL(deviceDialogClosed()), this, SLOT(aboutToClose()));
    connect(&mDeviceDialog, SIGNAL(dataReceived(QVariantMap)), this, SLOT(dataReceived(QVariantMap)));

    TRACE_EXIT
}

void HbDeviceProgressDialogPrivate::cancel()
{
    TRACE_ENTRY
    mDeviceDialog.cancel();
    TRACE_EXIT
    return;
}

// Schedule event to update changed properties to device dialog server. update() is not
// called after each time a property is set. Instead an event is scheduled in order to
// update all changed properties in one shot.
void HbDeviceProgressDialogPrivate::scheduleUpdateEvent()
{
    if (mVisible && mUpdateTimerId == 0) {
        mUpdateTimerId = startTimer(0);
    }
}

void HbDeviceProgressDialogPrivate::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mUpdateTimerId) {
        sendToServer(false);
    }
}

HbDeviceProgressDialogPrivate::ActionSelector HbDeviceProgressDialogPrivate::actionSelector(
    HbDeviceProgressDialog::ActionRole role)
{
    static const ActionSelector selectors[] = {
        CancelButton
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

void HbDeviceProgressDialogPrivate::aboutToClose()
{
    TRACE_ENTRY

    mVisible = false;
    emit q->aboutToClose();

    TRACE_EXIT
    return;
}

void HbDeviceProgressDialogPrivate::dataReceived(QVariantMap data)
{
    const char *key = "act";
    QVariantMap::const_iterator i = data.find(key);
    if (i != data.constEnd()) {
        if (i.value().toString() == "c") {
            // Client has pressed button. Signal action if one is set. Otherwise emit
            // cancelled() signal.
            mActions[CancelButton].mTriggered = true;
            QAction *action = mActions[CancelButton].mAction;
            if (action) {
                action->trigger();
                emit q->cancelled();
            }
        }
    }
}

/*!
    \class HbDeviceProgressDialog
    \brief HbDeviceProgressDialog is a device dialog version of HbProgressDialog.

    It displays a dialog with a wait animation or progress bar, text, icon or animation and a
    cancel button.

    Device dialogs are shown on top of any running applications and are always modal by nature.

    Two different dialogs are supported: wait and progress. The wait dialog displays an animated
    bar and progress dialog a progress bar indicating progress of the operation.

    Wait dialog is used when length of an operation cannot be determined beforehand. It displays
    an animated bar to indicate an action. The dialog is closed by the user pressing the cancel
    button, the application closing the dialog after the operation is finished.

    Progress dialog is used when the length of operation can be determined. For example
    when deleting a number of files, the progress of the operation could be shown as a
    percentage of the files deleted. Application updates the progress bar during the
    operation. The dialog closes by user pressing the cancel button, the application closing the
    dialog after the operation is finished or automatically when progress value reaches a
    maximum.

    HbDeviceProgressDialog provides a similar kind of interface as HbProgressDialog, excluding
    functions which handle concrete UI-component related information. Progress dialogs are
    always asynchronous by as the client needs to perform the operation and update the dialog
    while the dialog is showing.

    Device progress dialog is launched when show() is called. Launched dialog can be updated by setters.
    Because updating a dialog requires interprocess communication, it's advisable to fully construct the
    progress dialog before calling show().

    Supported icon animation formats are following:
    - GIF (.gif)
    - MNG (.mng)
        - Frame animations

    Sample code:

    An example showing the wait dialog:

    \code
    mDialog = new HbDeviceProgressDialog(HbProgressDialog::WaitDialog);
    mDialog->setText("Connecting...");
    mDialog->show();
    \endcode

    An example showing the progress dialog:

    \include deviceprogressdialog/main.cpp

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

    \sa HbProgressDialog, HbDialog, HbDeviceDialog
    \stable
    \hbwidgets
*/

/*!
    \fn void HbDeviceProgressDialog::aboutToClose();

    This signal is emitted when a device progress dialog has closed. The closing may
    be a result of close() being called, a dialog with autoClose property has reached
    its maximum value or user pressing cancel button. It is not emitted if close() is
    called before show().
 */

/*!
    \fn void HbDeviceProgressDialog::cancelled();

    This signal is emitted when the device progress dialog is closed by user pressing the
    "cancel" button. It is not emitted if dialog is closed for any other reason.
 */

/*!
    \enum HbDeviceProgressDialog::ActionRole
    Defines roles for actions set into a progress dialog.
*/
/*!
    \var HbDeviceProgressDialog::ActionRole HbDeviceProgressDialog::InvalidRole
    No action.
*/
/*!
    \var HbDeviceProgressDialog::ActionRole HbDeviceProgressDialog::CancelButtonRole
    Cancel button action.
*/

/*!
    Constructor.
    \param type Must be one of the defined HbProgressDialog::ProgressDialogType enumerations.
    \param parent An optional parameter.
*/
HbDeviceProgressDialog::HbDeviceProgressDialog(HbProgressDialog::ProgressDialogType type, QObject *parent) :
    QObject(parent), d(new HbDeviceProgressDialogPrivate)
{
    TRACE_ENTRY
    d->q = this;
    d->init(type);
    TRACE_EXIT
}

/*!
    Constructor.
    \param parent An optional parameter.
*/
HbDeviceProgressDialog::HbDeviceProgressDialog(QObject *parent) :
    QObject(parent), d(new HbDeviceProgressDialogPrivate)
{
    TRACE_ENTRY
    d->q = this;
    d->init(HbProgressDialog::ProgressDialog);
    TRACE_EXIT
}

/*!
    Destructs the class.
*/
HbDeviceProgressDialog::~HbDeviceProgressDialog()
{
    TRACE_ENTRY
    delete d;
    TRACE_EXIT
}

/*!
    Executes the dialog asynchronously.
*/
void HbDeviceProgressDialog::show()
{
    TRACE_ENTRY
    d->sendToServer(true);
    TRACE_EXIT
 }

/*!
    Updates changed properties of a launched progress dialog to device dialog service using
    interprocess communication. Has no effect if show() has not been called or dialog has
    closed already. Calling show() is optional as updating any property schedules an event
    and the dialog is updated next time Qt event loop executes.

    \sa show()
*/
void HbDeviceProgressDialog::update()
{
    TRACE_ENTRY
    d->sendToServer(false);
    TRACE_EXIT
}

/*!
    Closes the dialog.
*/
void HbDeviceProgressDialog::close()
{
    TRACE_ENTRY
    return d->cancel();
    TRACE_EXIT
}

/*!
    Returns an action user triggered causing the dialog to close. Returns 0 if none of the actions were
    triggered and dialog was closed for other reason.

    \sa show(), action()
*/
const QAction *HbDeviceProgressDialog::triggeredAction() const
{
    for(int i = 0; i < HbDeviceProgressDialogPrivate::NumActions; i++) {
        if (d->mActions[i].mTriggered) {
            return d->mActions[i].mAction;
        }
    }
    return 0;
}

/*!
    Sets the maximum value of the progress bar within the dialog.

    \sa maximum()
*/
void HbDeviceProgressDialog::setMaximum(int max)
{
    TRACE_ENTRY
    // Don't allow wait dialog to set max/min other than zero as wait
    // animation bar doesn't work in that case.
    if (progressType() == HbProgressDialog::WaitDialog) {
        max = 0;
    }
    d->setProperty(HbDeviceProgressDialogPrivate::Maximum, max);
    TRACE_EXIT
}

/*!
    Returns the maximum value of the progress bar within the dialog. Default value is 100.

    \sa setMaximum()
*/
int HbDeviceProgressDialog::maximum() const
{
    return d->mProperties[HbDeviceProgressDialogPrivate::Maximum].mValue.toInt();
}

/*!
    Sets the minimum value of the progress bar within the dialog.

    \sa minimum()
*/
void HbDeviceProgressDialog::setMinimum(int min)
{
    TRACE_ENTRY
    // Don't allow wait dialog to set max/min other than zero as wait
    // animation bar doesn't work in that case.
    if (progressType() == HbProgressDialog::WaitDialog) {
        min = 0;
    }
    d->setProperty(HbDeviceProgressDialogPrivate::Minimum, min);
    TRACE_EXIT
}

/*!
    Returns the minimum value of the progress bar within the dialog. Default value is 0.

    \sa setMinimum()
*/
int HbDeviceProgressDialog::minimum() const
{
    return d->mProperties[HbDeviceProgressDialogPrivate::Minimum].mValue.toInt();
}

/*!
    Sets the minimum and maximum value of the progress bar within the dialog.

    \sa minimum(), maximum()
*/
void HbDeviceProgressDialog::setRange(int min, int max)
{
    setMinimum(min);
    setMaximum(max);
}

/*!
    Sets the value of the progress bar within the dialog.

    \sa progressValue()
*/
void HbDeviceProgressDialog::setProgressValue(int progressValue)
{
    TRACE_ENTRY
    d->setProperty(HbDeviceProgressDialogPrivate::Value, progressValue);
    TRACE_EXIT
}

/*!
    Returns the value of the progress bar within the dialog.

    \sa setProgressValue()
 */
int HbDeviceProgressDialog::progressValue() const
{
    return d->mProperties[HbDeviceProgressDialogPrivate::Value].mValue.toInt();
}

/*!
    Sets the autoClose property value of the dialog.

    \param autoClose When set, the dialog is closed when value of the progress bar reaches
    the maximum value of the progress bar.

    \sa autoClose()
*/
void HbDeviceProgressDialog::setAutoClose(bool autoClose)
{
    TRACE_ENTRY
    d->setProperty(HbDeviceProgressDialogPrivate::AutoClose, autoClose);
    TRACE_EXIT
}

/*!
    Returns the value of the autoClose property of the dialog.

    The default value is true for HbProgressDialog::ProgressDialog and false
    for HbProgressDialog::WaitDialog.

    \sa setAutoClose()
 */
bool HbDeviceProgressDialog::autoClose() const
{
    return d->mProperties[HbDeviceProgressDialogPrivate::AutoClose].mValue.toInt();
}

/*!
    Sets dialog's progress type. After setProgressType(), a new dialog is launched by a show().

    \sa progressType()
*/
void HbDeviceProgressDialog::setProgressType(HbProgressDialog::ProgressDialogType type)
{
    TRACE_ENTRY
    // After setProgressType(), a new dialog is launched by a show()
    d->mVisible = false;
    // All properties initialized to default
    d->initProperties(type);
    TRACE_EXIT
}

/*!
    Returns dialog's progress type.

    \sa setProgressType()
*/
HbProgressDialog::ProgressDialogType HbDeviceProgressDialog::progressType() const
{
    return static_cast<HbProgressDialog::ProgressDialogType>
        (d->mProperties[HbDeviceProgressDialogPrivate::ProgressType].mValue.toInt());
}

/*!
    Sets text of the dialog.

    \sa text()
*/
void HbDeviceProgressDialog::setText(const QString &text)
{
    TRACE_ENTRY
    d->setProperty(HbDeviceProgressDialogPrivate::Text, text);
    TRACE_EXIT
}

/*!
    Returns text of the dialog.
    \sa setText()
*/
QString HbDeviceProgressDialog::text() const
{
    return d->mProperties[HbDeviceProgressDialogPrivate::Text].mValue.toString();
}

/*!
    Sets message box icon name or animation logical name.

    \param aIconName Icon name. Icon can be from Hb resources or themes. Or can be a file in
    a file system.

    \sa IconName()
*/
void HbDeviceProgressDialog::setIconName(const QString &iconName)
{
    TRACE_ENTRY
    d->setProperty(HbDeviceProgressDialogPrivate::IconName, iconName);
    TRACE_EXIT
}

/*!
    Returns name and path of the icon or animation's logical name.

    \sa setIconName()
*/
QString HbDeviceProgressDialog::iconName() const
{
    return d->mProperties[HbDeviceProgressDialogPrivate::IconName].mValue.toString();
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
void HbDeviceProgressDialog::setAnimationDefinition(QString &animationDefinition)
{
    TRACE_ENTRY
    d->setProperty(HbDeviceProgressDialogPrivate::AnimationDefinition, animationDefinition);
    TRACE_EXIT
}

/*!
    Returns animation definition file name.

    \sa setAnimationDefinition()
*/
QString HbDeviceProgressDialog::animationDefinition() const
{
    return d->mProperties[HbDeviceProgressDialogPrivate::AnimationDefinition].mValue.toString();
}

/*!
    Sets a new action into progress dialog. When users presses a button on dialog, triggered()
    signal of the action is emitted. HbDeviceProgressDialog constructor sets a default action
    into a dialog.

    \param action Action or Null. Ownership is not transferred.
    \param role Selects an action to set.

    \sa action()
*/
void HbDeviceProgressDialog::setAction(QAction *action, ActionRole role)
{
    TRACE_ENTRY
    HbDeviceProgressDialogPrivate::ActionSelector  actionSelector =
        HbDeviceProgressDialogPrivate::actionSelector(role);
    if (actionSelector != HbDeviceProgressDialogPrivate::InvalidSelector) {
        HbDeviceProgressDialogPrivate::Action &dialogAction = d->mActions[actionSelector];
        dialogAction.mAction = action;
        dialogAction.mFlags = HbDeviceProgressDialogPrivate::Modified;
        d->scheduleUpdateEvent();
    }
    TRACE_EXIT
}

/*!
    Returns progress dialog action. The action may be a default action owned by the dialog
    or the one set by setAction().

    \param role Selects an action to get.

    \sa setAction()
*/
QAction *HbDeviceProgressDialog::action(ActionRole role) const
{
    HbDeviceProgressDialogPrivate::ActionSelector  actionSelector =
        HbDeviceProgressDialogPrivate::actionSelector(role);
    return actionSelector != HbDeviceProgressDialogPrivate::InvalidSelector ?
        d->mActions[actionSelector].mAction : 0;
}
