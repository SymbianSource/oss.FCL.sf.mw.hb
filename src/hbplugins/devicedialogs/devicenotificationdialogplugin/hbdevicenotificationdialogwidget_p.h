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

#ifndef HBDEVICENOTIFICATIONDIALOGWIDGET_P_H
#define HBDEVICENOTIFICATIONDIALOGWIDGET_P_H

#include <QObject>
#include <QVariantMap>

#include <hbdevicedialoginterface.h>
#include <hbdevicedialog.h>
#include <hbnotificationdialog.h>

class HbDeviceNotificationDialogWidget : public HbNotificationDialog, public HbDeviceDialogInterface
{
    Q_OBJECT
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName)
    Q_PROPERTY(bool touchActivation READ isTouchActivating WRITE enableTouchActivation)
    Q_PROPERTY(QString animationDefinition READ animationDefinition WRITE setAnimationDefinition)
    Q_PROPERTY(int showLevel READ showLevel WRITE setShowLevel)

public:
    HbDeviceNotificationDialogWidget(const QVariantMap &parameters);
    virtual ~HbDeviceNotificationDialogWidget();

    bool setDeviceDialogParameters(const QVariantMap &parameters);
    int deviceDialogError() const;
    void closeDeviceDialog(bool byClient);
    HbDialog *deviceDialogWidget() const;

public slots:
    void widgetActivated();

private:
    bool constructDialog(const QVariantMap &parameters);
    bool checkProperties(const QVariantMap &parameters);
    void setProperties(const QVariantMap &parameters);
    void resetProperties();

    QString iconName() const;
    void setIconName(QString iconName);

    void hideEvent(QHideEvent *event);
    void closeEvent(QCloseEvent *event);

    void setShowLevel(int level);
    int showLevel() const;

    QString animationDefinition() const;
    void setAnimationDefinition(QString &animationDefinition);

signals:
    void deviceDialogClosed();
    void deviceDialogData(QVariantMap data);

private:
    Q_DISABLE_COPY(HbDeviceNotificationDialogWidget)

    int mLastError;
    QString mIconFilename;
    bool mCloseEventReceived;
    QString mAnimationDefinition;
};

#endif // HBDEVICENOTIFICATIONDIALOGWIDGET_P_H
