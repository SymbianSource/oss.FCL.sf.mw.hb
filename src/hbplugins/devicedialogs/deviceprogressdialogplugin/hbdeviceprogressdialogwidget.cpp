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

#include <hbdialog.h>
#include <hbaction.h>
#include <hbprogressdialog.h>
#include <hbdevicedialogtrace_p.h>
#include <hbiconanimationmanager.h>
#include "hbdeviceprogressdialogwidget_p.h"
#include "hbdeviceprogressdialogpluginerrors_p.h"

static const char actionTextTag[] = "t:";

// Constructor
HbDeviceProgressDialogWidget::HbDeviceProgressDialogWidget(HbProgressDialog::ProgressDialogType progressDialogType,
    const QVariantMap &parameters) : HbProgressDialog(progressDialogType)
{
    TRACE_ENTRY
    mLastError = NoError;
    mProgressDialogType = progressDialogType;
    mShowEventReceived = false;
    mAction = 0;
    resetProperties();
    constructDialog(parameters);
    if (!mAction) {
        // If HbProgressDialog default button is used, connect into its triggered signal.
        QAction *act = action(Cancel);
        if (act) {
            connect(act, SIGNAL(triggered()), SLOT(cancelTriggered()));
        }
    }
    setBackgroundFaded(true);
    TRACE_EXIT
}

// Destructor
HbDeviceProgressDialogWidget::~HbDeviceProgressDialogWidget()
{
    delete mAction;
}

// Set parameters
bool HbDeviceProgressDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
{
    TRACE_ENTRY
    mLastError = NoError;
    bool ret(false);
    if (checkProperties(parameters)) {
        setProperties(parameters);
        ret = true;
    } else {
        mLastError = ParameterError;
        ret = false;
    }
    TRACE_EXIT
    return ret;
}

// Get error
int HbDeviceProgressDialogWidget::deviceDialogError() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return mLastError;
}

// Close device dialog
void HbDeviceProgressDialogWidget::closeDeviceDialog(bool byClient)
{
    TRACE_ENTRY
    Q_UNUSED(byClient);
    cancel();
    // If show event has been received, close is signalled from hide event. If not,
    // hide event does not come and close is signalled from here.
    if (!mShowEventReceived) {
        emit deviceDialogClosed();
    }
    TRACE_EXIT
}

// Return display widget
HbDialog *HbDeviceProgressDialogWidget::deviceDialogWidget() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return const_cast<HbDeviceProgressDialogWidget*>(this);
}

// Get dialog type from parameters
bool HbDeviceProgressDialogWidget::getDialogType(HbProgressDialog::ProgressDialogType &progressDialogType,
    QVariantMap &parameters)
{
    TRACE_STATIC_ENTRY
    const char *key = "progressDialogType";
    progressDialogType = HbProgressDialog::WaitDialog;
    bool dialogTypeOk = true;
    QVariantMap::const_iterator i = parameters.find(key);
    if (i != parameters.constEnd()) {
        progressDialogType = static_cast<HbProgressDialog::ProgressDialogType>(i.value().toInt(&dialogTypeOk));
        dialogTypeOk = dialogTypeOk && progressDialogType >= HbProgressDialog::ProgressDialog &&
            progressDialogType <= HbProgressDialog::WaitDialog;
        parameters.remove(key);
    }
    TRACE_EXIT
    return dialogTypeOk;
}

// Construct device dialog
bool HbDeviceProgressDialogWidget::constructDialog(const QVariantMap &parameters)
{
    TRACE_ENTRY
    bool ret(false);
    // Check that parameters are valid
    if (!checkProperties(parameters)) {
        mLastError = ParameterError;
        ret = false;
    } else {
        setProperties(parameters);
        ret = true;
    }
    TRACE_EXIT
    return ret;
}

// Check that device dialog parameters are valid
bool HbDeviceProgressDialogWidget::checkProperties(const QVariantMap &parameters)
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
void HbDeviceProgressDialogWidget::setProperties(const QVariantMap &parameters)
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
void HbDeviceProgressDialogWidget::resetProperties()
{
    TRACE_ENTRY
    mIconName.clear();
    setModal(true);
    setTimeout(HbPopup::NoTimeout);
    setDismissPolicy(HbPopup::NoDismiss);
    TRACE_EXIT
    return;
}

QString HbDeviceProgressDialogWidget::iconName() const
{
    return mIconName;
}

void HbDeviceProgressDialogWidget::setIconName(QString &iconName)
{
    TRACE_ENTRY
    if (mIconName != iconName) {
        mIconName = iconName;
        setIcon(HbIcon(mIconName));
    }
    TRACE_EXIT
}

QString HbDeviceProgressDialogWidget::cancelAction() const
{
    TRACE_ENTRY
    QAction *act = action(Cancel);
	QString actionData;
    if (act) {
        actionData.append(actionTextTag);
        actionData.append(act->text());
    }
    TRACE_EXIT
    return actionData;
}

void HbDeviceProgressDialogWidget::setCancelAction(QString &actionData)
{
    TRACE_ENTRY
    parseActionData(actionData); // parse data to get action text
    if (!actionData.isNull()) {
        // Setting action
        QAction *act = action(Cancel);
        if (act) {
            act->setText(actionData);
        } else {
            if (!mAction) {
                mAction = new HbAction(actionData, 0);
                connect(mAction, SIGNAL(triggered()), this, SLOT(cancelTriggered()));
            } else {
                mAction->setText(actionData);
            }
            addAction(mAction);
        }
    } else { // Remove action
        // If there is a message box's default action, disconnect from it.
        QAction *act = action(Cancel);
        if (act && mAction == 0) {
            act->disconnect(SIGNAL(triggered()), this, SLOT(cancelTriggered()));
        }
        if (act) {
            removeAction(act);
        }
    }
    TRACE_EXIT
}

QAction *HbDeviceProgressDialogWidget::action(ActionIndex index) const
{
    QList<QAction*> actionList = actions();
    return actionList.count() > index ? actionList[index] : 0;
}

void HbDeviceProgressDialogWidget::parseActionData(QString &data)
{
    const QString textTag(actionTextTag);
    if (data.startsWith(textTag)) {
        data.remove(0, textTag.length());
    } else {
        data.clear();
    }
}

QString HbDeviceProgressDialogWidget::animationDefinition() const
{
    return mAnimationDefinition;
}

void HbDeviceProgressDialogWidget::setAnimationDefinition(QString &animationDefinition)
{
    // Load animation definition
    HbIconAnimationManager *manager = HbIconAnimationManager::global();
    manager->addDefinitionFile(animationDefinition);
    mAnimationDefinition = animationDefinition;
}

// Widget is about to hide. Closing effect has ended.
void HbDeviceProgressDialogWidget::hideEvent(QHideEvent *event)
{
    HbProgressDialog::hideEvent(event);
    emit deviceDialogClosed();
}

// Widget is about to show
void HbDeviceProgressDialogWidget::showEvent(QShowEvent *event)
{
    HbProgressDialog::showEvent(event);
    mShowEventReceived = true;
}

// Primary action triggered
void HbDeviceProgressDialogWidget::cancelTriggered()
{
    TRACE_ENTRY
    QVariantMap data;
    data.insert("act", "c");
    emit deviceDialogData(data);
    TRACE_EXIT
}
