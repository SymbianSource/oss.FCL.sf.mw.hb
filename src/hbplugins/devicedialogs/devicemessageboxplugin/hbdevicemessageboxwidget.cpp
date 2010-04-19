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

// Constructor
HbDeviceMessageBoxWidget::HbDeviceMessageBoxWidget(
    HbMessageBox::MessageBoxType type, const QVariantMap &parameters) : HbMessageBox(type)
{
    TRACE_ENTRY
    mLastError = NoError;
    mShowEventReceived = false;
    mPrimaryAction = 0;
    mSecondaryAction = 0;
    resetProperties();
    constructDialog(parameters);
    if (!mPrimaryAction) {
        // If default button provided by HbMessageBox is used, connect into its triggered signal.
        HbAction *action = primaryAction();
        if (action) {
            connect(action, SIGNAL(triggered()), SLOT(primaryActionTriggered()));
        }
    }
    if (!mSecondaryAction) {
        // If default button provided by HbMessageBox is used, connect into its triggered signal.
        HbAction *action = secondaryAction();
        if (action) {
            connect(action, SIGNAL(triggered()), SLOT(secondaryActionTriggered()));
        }
    }

    TRACE_EXIT
}

// Destructor
HbDeviceMessageBoxWidget::~HbDeviceMessageBoxWidget()
{
    delete mPrimaryAction;
    delete mSecondaryAction;
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
    QVariantMap::const_iterator i = parameters.constBegin();
    while (i != parameters.constEnd()) {
        QByteArray key = i.key().toAscii();
        if (property(key.constData()).isValid()) {
            setProperty(key.constData(), i.value());
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

QString HbDeviceMessageBoxWidget::primaryActionText() const
{
    HbAction *action = primaryAction();
    return action ? action->text() : QString();
}

void HbDeviceMessageBoxWidget::setPrimaryActionText(QString &actionText)
{
    TRACE_ENTRY
    HbAction *action = primaryAction();
    if (action) {
        action->setText(actionText);
    } else {
        if (!mPrimaryAction) {
            mPrimaryAction = new HbAction(actionText);
            connect(mPrimaryAction, SIGNAL(triggered()), this, SLOT(primaryActionTriggered()));
        } else {
            mPrimaryAction->setText(actionText);
        }
        setPrimaryAction(mPrimaryAction);
    }
    TRACE_EXIT
}

QString HbDeviceMessageBoxWidget::secondaryActionText() const
{
    HbAction *action = secondaryAction();
    return action ? action->text() : QString();
}

void HbDeviceMessageBoxWidget::setSecondaryActionText(QString &actionText)
{
    TRACE_ENTRY
    HbAction *action = secondaryAction();
    if (action) {
        action->setText(actionText);
    } else {
        if (!mSecondaryAction) {
            mSecondaryAction = new HbAction(actionText);
            connect(mSecondaryAction, SIGNAL(triggered()), this, SLOT(secondaryActionTriggered()));
        } else {
            mSecondaryAction->setText(actionText);
        }
        setSecondaryAction(mSecondaryAction);
    }
    TRACE_EXIT
}

bool HbDeviceMessageBoxWidget::primaryActionNull() const
{
    return primaryAction() == 0;
}

void HbDeviceMessageBoxWidget::setPrimaryActionNull(bool isNull)
{
    TRACE_ENTRY
    if (isNull) {
        // If there is a message box's default action, disconnect from it.
        HbAction *action = primaryAction();
        if (action && mPrimaryAction == 0) {
            action->disconnect(SIGNAL(triggered()), this, SLOT(primaryActionTriggered()));
        }
        setPrimaryAction(0);
    } else {
        QString text = mPrimaryAction ? mPrimaryAction->text() : QString();
        setPrimaryActionText(text);
    }
    TRACE_EXIT
}

bool HbDeviceMessageBoxWidget::secondaryActionNull() const
{
    return secondaryAction() == 0;
}

void HbDeviceMessageBoxWidget::setSecondaryActionNull(bool isNull)
{
    TRACE_ENTRY
    if (isNull) {
        // If there is a message box's default action, disconnect from it.
        HbAction *action = secondaryAction();
        if (action && mSecondaryAction == 0) {
            action->disconnect(SIGNAL(triggered()), this, SLOT(secondaryActionTriggered()));
        }
        setSecondaryAction(0);
    } else {
        QString text = mSecondaryAction ? mSecondaryAction->text() : QString();
        setSecondaryActionText(text);
    }
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

// Primary action triggered
void HbDeviceMessageBoxWidget::primaryActionTriggered()
{
    TRACE_ENTRY
    QVariantMap data;
    data.insert("act", "p");
    emit deviceDialogData(data);
    mSendAction = false;
    TRACE_EXIT
}

// Secondary action triggered
void HbDeviceMessageBoxWidget::secondaryActionTriggered()
{
    TRACE_ENTRY
    QVariantMap data;
    data.insert("act", "s");
    emit deviceDialogData(data);
    mSendAction = false;
    TRACE_EXIT
}
