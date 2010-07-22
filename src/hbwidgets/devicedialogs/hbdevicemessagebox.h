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

#ifndef HBDEVICEMESSAGEBOX_H
#define HBDEVICEMESSAGEBOX_H

#include <QObject>
#include <hbglobal.h>
#include <hbmessagebox.h>

class HbDeviceMessageBoxPrivate;
class QAction;

class HB_WIDGETS_EXPORT HbDeviceMessageBox : public QObject
{
    Q_OBJECT

    Q_PROPERTY(HbMessageBox::MessageBoxType messageBoxType READ messageBoxType WRITE setMessageBoxType)
    Q_PROPERTY(HbMessageBox::StandardButtons standardButtons READ standardButtons WRITE setStandardButtons)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName)
    Q_PROPERTY(bool iconVisible READ iconVisible WRITE setIconVisible)
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout)
    Q_PROPERTY(QString animationDefinition READ animationDefinition WRITE setAnimationDefinition)

public:
    enum ActionRole {
        InvalidRole = -1,
        AcceptButtonRole,
        RejectButtonRole
    };

public:
    explicit HbDeviceMessageBox(
        HbMessageBox::MessageBoxType type = HbMessageBox::MessageTypeNone,
        QObject *parent = 0);
    explicit HbDeviceMessageBox(const QString &text,
        HbMessageBox::MessageBoxType type = HbMessageBox::MessageTypeNone,
        QObject *parent=0);
    virtual ~HbDeviceMessageBox();

    static bool question(
        const QString &text,
        const QString &acceptButtonText,
        const QString &rejectButtonText);
    static bool question(const QString &text, HbMessageBox::StandardButtons buttons =
        HbMessageBox::Yes|HbMessageBox::No);
    static void information(const QString &text);
    static void warning(const QString &text);

    void show();
    void update();
    void close();
    const QAction *exec();
    const QAction *triggeredAction() const;
    bool isAcceptAction(const QAction *action) const;

    void setMessageBoxType(HbMessageBox::MessageBoxType type);
    HbMessageBox::MessageBoxType messageBoxType() const;

    void setStandardButtons(HbMessageBox::StandardButtons buttons);
    HbMessageBox::StandardButtons standardButtons() const;

    void setText(const QString &text);
    QString text() const;

    void setIconName(const QString &iconName);
    QString iconName() const;

    void setIconVisible(bool visible);
    bool iconVisible() const;

    void setAnimationDefinition(QString &animationDefinition);
    QString animationDefinition() const;

    void setTimeout(int timeout);
    void setTimeout(HbPopup::DefaultTimeout timeout);
    int timeout() const;

    void setDismissPolicy(HbPopup::DismissPolicy dismissPolicy);
    HbPopup::DismissPolicy dismissPolicy() const;

    void setAction(QAction *action, ActionRole role);
    QAction *action(ActionRole role) const;

signals:
    void aboutToClose();

private:
    HbDeviceMessageBoxPrivate *d_ptr;
    Q_DECLARE_PRIVATE_D(d_ptr, HbDeviceMessageBox)
    Q_DISABLE_COPY(HbDeviceMessageBox)

};

#endif // HBDEVICEMESSAGEBOX_H
