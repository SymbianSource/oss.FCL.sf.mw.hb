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

#ifndef HBDATETIMEEDIT_H
#define HBDATETIMEEDIT_H

#include <hbglobal.h>
#include <hbnamespace.h>
#include <hblineedit.h>
#include <QDateTime>
#include <QLocale>

class HbDateTimeEditPrivate;

class HB_WIDGETS_EXPORT HbDateTimeEdit: public HbLineEdit
{
    Q_OBJECT
    Q_PROPERTY(QDateTime dateTime READ dateTime WRITE setDateTime)
    Q_PROPERTY(QString displayFormat READ displayFormat WRITE setDisplayFormat)

public:

    explicit HbDateTimeEdit (QGraphicsItem *parent = 0);
    HbDateTimeEdit (const QDate &date, QGraphicsItem *parent = 0);
    HbDateTimeEdit (const QTime &time, QGraphicsItem *parent = 0);
    HbDateTimeEdit (const QDateTime &dateTime, QGraphicsItem *parent = 0);

    virtual ~HbDateTimeEdit ();

    enum {
        Type = Hb::ItemType_DateTimeEdit
    };
    int type () const;

    void setDateTime (const QDateTime &dateTime);
    QDateTime dateTime () const;

    QString displayFormat () const;

    void setDisplayFormat (const QString &format);

    void setDateFormat(const QLocale& locale = QLocale());
    void setTimeFormat(const QLocale& locale = QLocale());
    void setDateTimeFormat(const QLocale& locale = QLocale());

protected:
    HbDateTimeEdit (HbDateTimeEditPrivate &dd, QGraphicsItem *parent);

    void setMaxLength (int length);
    void setText (const QString &text);
    void focusOutEvent(QFocusEvent *event);

private:
    Q_DISABLE_COPY(HbDateTimeEdit)
    Q_DECLARE_PRIVATE_D(d_ptr, HbDateTimeEdit)
    Q_PRIVATE_SLOT(d_func(), void _q_regainFocus())
};

#endif // HBDATETIMEEDIT_H
