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
#ifndef HBDEVICENOTIFICATIONDIALOG_H
#define HBDEVICENOTIFICATIONDIALOG_H

#include <hbglobal.h>

#include <QObject>
#include <QString>
#include <hbicon.h>
#include <hbnamespace.h>

class HbDeviceNotificationDialogPrivate;

class HB_WIDGETS_EXPORT HbDeviceNotificationDialog : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(Hb::TextWrapping titleTextWrapping READ titleTextWrapping WRITE setTitleTextWrapping)
    Q_PROPERTY(bool enableTouchActivation READ isTouchActivating WRITE enableTouchActivation)
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout)
    Q_PROPERTY(QString animationDefinition READ animationDefinition WRITE setAnimationDefinition)

public:
    explicit HbDeviceNotificationDialog(QObject *parent = 0);
    virtual ~HbDeviceNotificationDialog();

    static void notification(const QString &iconName, const QString &title = QString());
    static void notification(const QString &iconName, const QString &text, const QString &title);

    void setIconName(const QString &iconName);
    void setText(const QString &text);
    void setTitle(const QString &title);
    void enableTouchActivation(bool enable);
    void setTimeout(int timeout);
    void setTitleTextWrapping(Hb::TextWrapping wrapping);
    void setAnimationDefinition(QString &animationDefinition);

    QString iconName() const;
    QString text() const;
    QString title() const;
    bool isTouchActivating() const;
    int timeout() const;
    Hb::TextWrapping titleTextWrapping() const;
    QString animationDefinition() const;

    void show();
    void update();
    void close();

signals:
    void aboutToClose();
    void activated();

protected:
    HbDeviceNotificationDialogPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbDeviceNotificationDialog)
    Q_DISABLE_COPY(HbDeviceNotificationDialog)

};

#endif // HBDEVICENOTIFICATIONDIALOG_H
