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

#ifndef HBDEVICEPROGRESSDIALOG_H
#define HBDEVICEPROGRESSDIALOG_H

#include <QObject>
#include <hbglobal.h>
#include <hbprogressdialog.h>

class HbDeviceProgressDialogPrivate;

class HB_WIDGETS_EXPORT HbDeviceProgressDialog : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int value READ progressValue WRITE setProgressValue)
    Q_PROPERTY(bool autoClose READ autoClose WRITE setAutoClose)
    Q_PROPERTY(HbProgressDialog::ProgressDialogType progressType
        READ progressType WRITE setProgressType)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(Qt::Alignment textAlignment READ textAlignment WRITE setTextAlignment)
    Q_PROPERTY(bool textWrapping READ textWrapping WRITE setTextWrapping) // deprecated
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName)
    Q_PROPERTY(Qt::Alignment iconAlignment READ iconAlignment WRITE setIconAlignment)
    Q_PROPERTY(QString animationDefinition READ animationDefinition WRITE setAnimationDefinition)

public:
    enum ActionRole {
        InvalidRole = -1,
        CancelButtonRole
    };

    explicit HbDeviceProgressDialog(HbProgressDialog::ProgressDialogType type, QObject *parent = 0);
    explicit HbDeviceProgressDialog(QObject *parent = 0);
    virtual ~HbDeviceProgressDialog();

    void show();
    void update();
    void cancel(); // deprecated
    void close();
    const QAction *triggeredAction() const;

    void setMaximum(int max);
    int maximum() const;
    void setMinimum(int min);
    int minimum() const;
    void setRange(int min, int max);

    void setProgressValue(int progressValue);
    int progressValue() const;

    void setAutoClose(bool autoClose);
    bool autoClose() const;

    void setProgressType(HbProgressDialog::ProgressDialogType type);
    HbProgressDialog::ProgressDialogType progressType() const;

    void setText(const QString &text);
    QString text() const;

    void setTextAlignment(Qt::Alignment align);
    Qt::Alignment textAlignment() const;

    void setTextWrapping(bool wrap); // deprecated
    bool textWrapping() const; // deprecated

    void setIconName(const QString &iconName);
    QString iconName() const;

    void setIconAlignment(Qt::Alignment align);
    Qt::Alignment iconAlignment() const;

    void setAnimationDefinition(QString &animationDefinition);
    QString animationDefinition() const;

    void setAction(QAction *action, ActionRole role = CancelButtonRole);
    QAction *action(ActionRole role = CancelButtonRole) const;

signals:
    void cancelled();
    void aboutToClose();

private:
    HbDeviceProgressDialogPrivate * const d;
    friend class HbDeviceProgressDialogPrivate;

    Q_DISABLE_COPY(HbDeviceProgressDialog)
};

#endif // HBDEVICEPROGRESSDIALOG_H
