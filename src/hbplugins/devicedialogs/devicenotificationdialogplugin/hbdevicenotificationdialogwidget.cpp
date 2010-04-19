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

#include <QtDebug>
#include <QIcon>
#include <QVariant>
#include <QList>
#include <hbicon.h>
#include <hbiconanimationmanager.h>
#include <hbdevicedialogtrace_p.h>
#include "hbdevicenotificationdialogwidget_p.h"
#include "hbdevicenotificationdialogpluginerrors_p.h"

#ifdef HB_EFFECTS
#include "hbeffectinternal_p.h"
#endif

// Constructor
HbDeviceNotificationDialogWidget::HbDeviceNotificationDialogWidget(const QVariantMap &parameters)
: HbNotificationDialog()
{
    TRACE_ENTRY
    mLastError = NoError;
    mShowEventReceived = false;
    resetProperties();
    constructDialog(parameters);

#ifdef HB_EFFECTS
    HbEffectInternal::add(this, "devicenotificationdialog_appear", "appear");
    HbEffectInternal::add(this, "devicenotificationdialog_disappear", "disappear");
#endif

    TRACE_EXIT
}

HbDeviceNotificationDialogWidget::~HbDeviceNotificationDialogWidget()
{
#ifdef HB_EFFECTS
    HbEffectInternal::remove(this);
#endif
}

// Set parameters
bool HbDeviceNotificationDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
{
    TRACE_ENTRY
    bool ret(false);
    mLastError = NoError;
    if (checkProperties(parameters)) {
        setProperties(parameters);
        ret = true;
    }
    TRACE_EXIT
    return ret;
}

// Get error
int HbDeviceNotificationDialogWidget::deviceDialogError() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return mLastError;
}

// Close device dialog
void HbDeviceNotificationDialogWidget::closeDeviceDialog(bool byClient)
{
    TRACE_ENTRY
    Q_UNUSED(byClient);
    close();
    // If show event has been received, close is signalled from hide event. If not,
    // hide event does not come and close is signalled from here.
    if (!mShowEventReceived) {
        emit deviceDialogClosed();
    }
    TRACE_EXIT
}

// Return display widget
HbDialog *HbDeviceNotificationDialogWidget::deviceDialogWidget() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return const_cast<HbDeviceNotificationDialogWidget*>(this);
}

// Construct device dialog
bool HbDeviceNotificationDialogWidget::constructDialog(const QVariantMap &parameters)
{
    TRACE_ENTRY
    mLastError = NoError;
    bool ret = checkProperties(parameters);

    if (ret) {
        connect(this, SIGNAL(activated()), this, SLOT(widgetActivated()));
        setProperties(parameters);
    }

    TRACE_EXIT
    return ret;
}

// Check that device dialog parameters are valid
bool HbDeviceNotificationDialogWidget::checkProperties(const QVariantMap &parameters)
{
    TRACE_ENTRY
    bool ret(true);

    QVariantMap::const_iterator i = parameters.constBegin();
    while (i != parameters.constEnd()) {
        QByteArray key = i.key().toAscii();
        if (!property(key.constData()).isValid()) {
            TRACE_EXIT
            mLastError = ParameterError;
            ret = false;
            break;
        }
        ++i;
    }

    TRACE_EXIT
    return ret;
}

// Set properties
void HbDeviceNotificationDialogWidget::setProperties(const QVariantMap &parameters)
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
}

// Reset properties to default values
void HbDeviceNotificationDialogWidget::resetProperties()
{
    TRACE_ENTRY
    mIconFilename.clear();
    TRACE_EXIT
    return;
}

QString HbDeviceNotificationDialogWidget::iconName() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return mIconFilename;
}

void HbDeviceNotificationDialogWidget::setIconName(QString iconName)
{
    TRACE_ENTRY
    HbIcon hbicon(iconName);
    setIcon(hbicon);
    TRACE_EXIT
}

// Widget is about to hide. Closing effect has ended.
void HbDeviceNotificationDialogWidget::hideEvent(QHideEvent *event)
{
    HbNotificationDialog::hideEvent(event);
    emit deviceDialogClosed();
}

// Widget is about to show
void HbDeviceNotificationDialogWidget::showEvent(QShowEvent *event)
{
    HbNotificationDialog::showEvent(event);
    mShowEventReceived = true;
}

QString HbDeviceNotificationDialogWidget::animationDefinition() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return mAnimationDefinition;
}

void HbDeviceNotificationDialogWidget::setAnimationDefinition(QString &animationDefinition)
{
    TRACE_ENTRY
    HbIconAnimationManager *manager = HbIconAnimationManager::global();
    manager->addDefinitionFile(animationDefinition);
    mAnimationDefinition = animationDefinition;
    TRACE_EXIT
}

void HbDeviceNotificationDialogWidget::widgetActivated()
{
    TRACE_ENTRY
    QVariantMap map;
    map[QString("result")] = "activated";
    emit deviceDialogData(map);
    TRACE_EXIT
}
