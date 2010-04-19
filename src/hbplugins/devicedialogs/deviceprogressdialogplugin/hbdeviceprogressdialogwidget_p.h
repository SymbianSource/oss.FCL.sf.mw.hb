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

#ifndef HBDEVICEPROGRESSDIALOGWIDGET_P_H
#define HBDEVICEPROGRESSDIALOGWIDGET_P_H

#include <QObject>
#include <QVariantMap>

#include <hbdevicedialoginterface.h>
#include <hbdevicedialog.h>
#include <hbprogressdialog.h>

class HbDialog;
class HbAction;

class HbDeviceProgressDialogWidget : public HbProgressDialog, public HbDeviceDialogInterface
{
    Q_OBJECT
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName)
    Q_PROPERTY(QString primaryActionText READ primaryActionText WRITE setPrimaryActionText)
    Q_PROPERTY(bool primaryActionNull READ primaryActionNull WRITE setPrimaryActionNull)
    Q_PROPERTY(QString animationDefinition READ animationDefinition WRITE setAnimationDefinition)

public:
    HbDeviceProgressDialogWidget(HbProgressDialog::ProgressDialogType progressDialogType, const QVariantMap &parameters);
    ~HbDeviceProgressDialogWidget();

    bool setDeviceDialogParameters(const QVariantMap &parameters);
    int deviceDialogError() const;
    void closeDeviceDialog(bool byClient);
    HbDialog *deviceDialogWidget() const;
    static bool getDialogType(HbProgressDialog::ProgressDialogType &dialogType, QVariantMap &parameters);

public slots:
    void primaryActionTriggered();

private:
    bool constructDialog(const QVariantMap &parameters);
    bool checkProperties(const QVariantMap& parameters);
    void setProperties(const QVariantMap& parameters);
    void resetProperties();
    QString iconName() const;
    void setIconName(QString &iconName);
    QString primaryActionText() const;
    void setPrimaryActionText(QString &actionText);
    bool primaryActionNull() const;
    void setPrimaryActionNull(bool isNull);
    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);
    void setAnimationDefinition(QString &animationDefinition);
    QString animationDefinition() const;

signals:
    void deviceDialogClosed();
    void deviceDialogData(QVariantMap data);

private:
    Q_DISABLE_COPY(HbDeviceProgressDialogWidget)

    int mLastError;
    QString mIconName;
    HbAction *mPrimaryAction;
    HbProgressDialog::ProgressDialogType mProgressDialogType;
    bool mShowEventReceived;
    QString mAnimationDefinition;
};

#endif // HBDEVICEPROGRESSDIALOGWIDGET_P_H
