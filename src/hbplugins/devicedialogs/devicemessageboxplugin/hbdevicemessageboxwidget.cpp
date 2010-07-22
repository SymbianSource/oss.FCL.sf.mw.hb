/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbPlugins module of the UI Extensions for Mobile.
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

#include <hbaction.h>
#include <hbdialog.h>
#include <hbdevicedialogtrace_p.h>
#include <hbiconanimationmanager.h>
#include "hbdevicemessageboxwidget_p.h"
#include "hbdevicemessageboxpluginerrors_p.h"

static const char actionTextTag[] = "t:";

// Constructor
HbDeviceMessageBoxWidget::HbDeviceMessageBoxWidget(
    HbMessageBox::MessageBoxType type, const QVariantMap &parameters) : HbMessageBox(type)
{
    TRACE_ENTRY
    mLastError = NoError;
    mShowEventReceived = false;
    initActions();
    connectToActions();
    resetProperties();
    constructDialog(parameters);
    TRACE_EXIT
}

// Destructor
HbDeviceMessageBoxWidget::~HbDeviceMessageBoxWidget()
{
    deleteActions();
}

// Set parameters
bool HbDeviceMessageBoxWidget::setDeviceDialogParameters(
    const QVariantMap &parameters)
{
    TRACE_ENTRY
    mLastError = NoError;
    bool ret(false);
    if (checkProperties(parameters)) {
        setProperties(parameters);
        ret = true;
    }
    else {
        mLastError = ParameterError;
        ret = false;
    }
    TRACE_EXIT
    return ret;
}

// Get error
int HbDeviceMessageBoxWidget::deviceDialogError() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return mLastError;
}

// Close device dialog
void HbDeviceMessageBoxWidget::closeDeviceDialog(bool byClient)
{
    TRACE_ENTRY
    Q_UNUSED(byClient);
    // Closed by client or internally by server -> no action to be transmitted.
    mSendAction = false;
    close();
    // If show event has been received, close is signalled from hide event. If not,
    // hide event does not come and close is signalled from here.
    if (!mShowEventReceived) {
        emit deviceDialogClosed();
    }
    TRACE_EXIT
}

// Return display widget
HbDialog *HbDeviceMessageBoxWidget::deviceDialogWidget() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return const_cast<HbDeviceMessageBoxWidget*>(this);
}

// Construct dialog
bool HbDeviceMessageBoxWidget::constructDialog(const QVariantMap &parameters)
{
    TRACE_ENTRY
    bool ret(false);
    // Check that parameters are valid
    if (!checkProperties(parameters)) {
        mLastError = ParameterError;
        ret = false;
    }
    else {
        setProperties(parameters);
        ret = true;
    }
    TRACE_EXIT
    return ret;
}

// Check that device dialog parameters are valid
bool HbDeviceMessageBoxWidget::checkProperties(const QVariantMap &parameters)
{
    TRACE_ENTRY
    QVariantMap::const_iterator i = parameters.constBegin();
    while (i != parameters.constEnd()) {
        QByteArray key = i.key().toAscii();
        if (!property(key.constData()).isValid()) {
            TRACE_EXIT
            return false;
        }
        ++i;
    }
    TRACE_EXIT
    return true;
}

// Set properties
void HbDeviceMessageBoxWidget::setProperties(const QVariantMap &parameters)
{
    TRACE_ENTRY
    // Set properties other than accept/reject action first
    const char *acceptKey = "acceptAction";
    const char *rejectKey = "rejectAction";
    QVariantMap::const_iterator i = parameters.constBegin();
    while(i != parameters.constEnd()) {
        if (i.key() != acceptKey && i.key() != rejectKey) {
            QByteArray key = i.key().toAscii();
            if (property(key.constData()).isValid()) {
                setProperty(key.constData(), i.value());
            }
        }
        ++i;
    }
    // Set accept/reject action last
    i = parameters.constBegin();
    while(i != parameters.constEnd()) {
        if (i.key() == acceptKey || i.key() == rejectKey) {
            QByteArray key = i.key().toAscii();
            if (property(key.constData()).isValid()) {
                setProperty(key.constData(), i.value());
            }
        }
        ++i;
    }
    TRACE_EXIT
    return;
}

// Reset properties to default values
void HbDeviceMessageBoxWidget::resetProperties()
{
    TRACE_ENTRY
    mIconName.clear();
    setModal(true);
    setTimeout(HbPopup::NoTimeout);
    setDismissPolicy(HbPopup::NoDismiss);
    mSendAction = true;
    TRACE_EXIT
    return;
}

// Delete actions we own
void HbDeviceMessageBoxWidget::deleteActions()
{
    TRACE_ENTRY
    for(int i = 0; i < NumActions; i++) {
        if (mActions[i].mOwned) {
            delete mActions[i].mAction;
        }
        mActions[i].mAction = 0;
        mActions[i].mOwned = false; // we haven't created the action
        mActions[i].mInDialog = false; // action has not been inserted to the dialog
    }
    TRACE_EXIT
}

// Initialize actions
void HbDeviceMessageBoxWidget::initActions()
{
    TRACE_ENTRY
    for(int i = 0; i < NumActions; i++) {
        mActions[i].mAction = 0;
        mActions[i].mOwned = false; // we haven't created the action
        mActions[i].mInDialog = false; // action has not been inserted to the dialog
    }
    TRACE_EXIT
}

// Connect to message box triggered actions
void HbDeviceMessageBoxWidget::connectToActions()
{
    QList<QAction*> actList = actions();
    int count = qMin(static_cast<int>(NumActions), actList.count());
    for(int i = 0; i < count; i++) {
        mActions[i].mAction = actList[i];
        mActions[i].mInDialog = true;
        connect(mActions[i].mAction, SIGNAL(triggered()), SLOT(actionTriggered()));
    }
}

QString HbDeviceMessageBoxWidget::iconName() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return mIconName;
}

void HbDeviceMessageBoxWidget::setIconName(QString &iconName)
{
    TRACE_ENTRY
    if (mIconName != iconName) {
        mIconName = iconName;
        setIcon(HbIcon(mIconName));
    }
    TRACE_EXIT
    return;
}

QString HbDeviceMessageBoxWidget::acceptAction() const
{
    return actionData(Accept);
}

void HbDeviceMessageBoxWidget::setAcceptAction(QString &actionData)
{
    TRACE_ENTRY
    setAction(Accept, actionData);
    TRACE_EXIT
}

QString HbDeviceMessageBoxWidget::rejectAction() const
{
    return actionData(Reject);
}

void HbDeviceMessageBoxWidget::setRejectAction(QString &actionData)
{
    TRACE_ENTRY
    setAction(Reject, actionData);
    TRACE_EXIT
}

// Widget is about to hide. Closing effect has ended.
void HbDeviceMessageBoxWidget::hideEvent(QHideEvent *event)
{
    HbMessageBox::hideEvent(event);
    emit deviceDialogClosed();
}

// Widget is about to show
void HbDeviceMessageBoxWidget::showEvent(QShowEvent *event)
{
    HbMessageBox::showEvent(event);
    mShowEventReceived = true;
}

void HbDeviceMessageBoxWidget::setAnimationDefinition(QString &animationDefinition)
{
    // Load animation definition
    HbIconAnimationManager *manager = HbIconAnimationManager::global();
    manager->addDefinitionFile(animationDefinition);
    mAnimationDefinition = animationDefinition;
}

QString HbDeviceMessageBoxWidget::animationDefinition() const
{
    return mAnimationDefinition;
}

void HbDeviceMessageBoxWidget::setStandardButtons(HbMessageBox::StandardButtons buttons)
{
    // Clear buttons first. Otherwise display doesn't get updated always.
    HbMessageBox::setStandardButtons(HbMessageBox::NoButton);
    HbMessageBox::setStandardButtons(buttons);
    deleteActions();
    initActions();
    connectToActions();
}

HbMessageBox::StandardButtons HbDeviceMessageBoxWidget::standardButtons() const
{
    return HbMessageBox::standardButtons();
}

// Action (accept or reject) was triggered
void HbDeviceMessageBoxWidget::actionTriggered()
{
    TRACE_ENTRY
    bool accepted = (sender() == mActions[Accept].mAction);
    QVariantMap data;
    data.insert("act", accepted ? "p" : "s");
    emit deviceDialogData(data);
    mSendAction = false;
    TRACE_EXIT
}

// Parse action data and return button text
void HbDeviceMessageBoxWidget::parseActionData(QString &data)
{
    const QString textTag(actionTextTag);
    if (data.startsWith(textTag)) {
        data.remove(0, textTag.length());
    } else {
        data.clear();
    }
}

// Pack action into a string
QString HbDeviceMessageBoxWidget::actionData(ActionIndex index) const
{
    QAction *act = mActions[index].mInDialog ? mActions[index].mAction : 0;
    QString actionData;
    if (act) {
        actionData.append(actionTextTag);
        actionData.append(act->text());
    }
    return actionData;
}

// Set action
void HbDeviceMessageBoxWidget::setAction(ActionIndex index, QString &actionData)
{
    parseActionData(actionData); // parse data to get action text
    if (!actionData.isNull()) {
        // Setting action
        QAction *act = mActions[index].mAction;
        if (act) {
            act->setText(actionData);
        } else {
            // Create action
            mActions[index].mAction = new HbAction(actionData, 0);
            mActions[index].mOwned = true;
            //connect(mActions[index].mAction, SIGNAL(triggered()), this, SLOT(actionTriggered()));
        }
        // Add action to dialog if it's not there
        if (!mActions[index].mInDialog) {
            mActions[index].mInDialog = true;
            QAction *before = 0;
            if (index == Accept && mActions[Reject].mInDialog) {
                before = mActions[Reject].mAction; // insert to table head
            }
            insertAction(before, mActions[index].mAction);
            connect(mActions[index].mAction, SIGNAL(triggered()), this, SLOT(actionTriggered()));
        }
    } else { // Remove action
        mActions[index].mInDialog = false;
        if (mActions[index].mAction) {
            removeAction(mActions[index].mAction);
            mActions[index].mAction->disconnect(SIGNAL(triggered()), this, SLOT(actionTriggered()));
        }
    }
}
