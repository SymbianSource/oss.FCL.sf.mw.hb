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

#ifndef HBNOTIFICATIONDIALOG_H
#define HBNOTIFICATIONDIALOG_H

#include <hbdialog.h>

class HbNotificationDialogPrivate;
class HbIcon;
class QGestureEvent;

class HB_WIDGETS_EXPORT HbNotificationDialog : public HbDialog
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(int wrapMode READ wrapMode WRITE setWrapMode)     //deprecated
    Q_PROPERTY(HbIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(Hb::TextWrapping titleTextWrapping READ titleTextWrapping WRITE setTitleTextWrapping)
    Q_PROPERTY(bool sequentialShow READ isSequentialShow WRITE setSequentialShow)

public:

    //deprecated
    enum WrapMode
    {
        NoWrap = 0,
        TitleWrapsTwoLines
    };

public:
    HbNotificationDialog();
    ~HbNotificationDialog();

    static void launchDialog(const QString &title, QGraphicsScene *scene = 0);

    //deprecated.
    static void launchDialog(const HbIcon &icon, QGraphicsScene *scene = 0);
    static void launchDialog(const QString &title, const QString &text, QGraphicsScene *scene = 0);
    static void launchDialog(const HbIcon &icon, const QString &title, const QString &text, QGraphicsScene *scene = 0);

    void enableTouchActivation(bool enabled);

    bool isTouchActivating() const;

    void setTitle(const QString &text);
    void setText(const QString &text);
    void setIcon(const HbIcon &icon);

    QString title() const;
    QString text() const;
    HbIcon icon() const;

    //deprecated
    int wrapMode() const;

    //deprecated
    void setWrapMode(int mode);

    void setTitleTextWrapping(Hb::TextWrapping wrapping);
    Hb::TextWrapping titleTextWrapping() const;

    void setSequentialShow(bool sequential);
    bool isSequentialShow() const;

    enum { Type = Hb::ItemType_NotificationDialog };
    int type() const { return Type; }

signals:
    void activated();

protected:
    HbNotificationDialog( HbNotificationDialogPrivate &dd, QGraphicsItem *parent);
    void gestureEvent(QGestureEvent *event);
    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private slots:
    void orientationChanged(Qt::Orientation orientation);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbNotificationDialog)
    Q_DISABLE_COPY(HbNotificationDialog)
};

#endif /*HBNOTIFICATIONDIALOG_H*/
