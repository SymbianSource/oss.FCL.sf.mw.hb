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

#ifndef HBDATETIMEPICKER_H
#define HBDATETIMEPICKER_H

#include <hbwidget.h>

#include <QString>
#include <QDateTimeEdit>

class HbDateTimePickerPrivate;

class HB_WIDGETS_EXPORT HbDateTimePicker : public HbWidget
{
    Q_OBJECT
    Q_PROPERTY(QString displayFormat READ displayFormat WRITE setDisplayFormat)
    Q_PROPERTY(QDate date READ date WRITE setDate)
    Q_PROPERTY(QDateTime dateTime READ dateTime WRITE setDateTime)
    Q_PROPERTY(QTime time READ time WRITE setTime)
    Q_PROPERTY(QDate minimumDate READ minimumDate WRITE setMinimumDate)
    Q_PROPERTY(QDate maximumDate READ maximumDate WRITE setMaximumDate)
    Q_PROPERTY(QTime minimumTime READ minimumTime WRITE setMinimumTime)
    Q_PROPERTY(QTime maximumTime READ maximumTime WRITE setMaximumTime)
    Q_PROPERTY(QDateTime minimumDateTime READ minimumDateTime WRITE setMinimumDateTime)
    Q_PROPERTY(QDateTime maximumDateTime READ maximumDateTime WRITE setMaximumDateTime)

public:
    explicit HbDateTimePicker(QGraphicsItem *parent = 0);
    explicit HbDateTimePicker(const QDateTime & datetime, QGraphicsItem * parent = 0);
    explicit HbDateTimePicker(const QDate & date, QGraphicsItem * parent = 0);
    explicit HbDateTimePicker(const QTime & time, QGraphicsItem * parent = 0);
    ~HbDateTimePicker();

    QString displayFormat() const;
    void setDisplayFormat(const QString &displayFormat);

    QDate date() const;
    
    QDate minimumDate()const;
    void setMinimumDate(const QDate &date);

    QDate maximumDate()const;
    void setMaximumDate(const QDate &date);

    void setDateRange(const QDate &minDate, const QDate &maxDate);

    QDateTime dateTime()const;

    QDateTime minimumDateTime()const;
    void setMinimumDateTime(const QDateTime &date);

    QDateTime maximumDateTime()const;
    void setMaximumDateTime(const QDateTime &date);

    void setDateTimeRange(const QDateTime &minDateTime, const QDateTime &maxDateTime);

    QTime time() const;

    QTime minimumTime()const;
    void setMinimumTime(const QTime &date);

    QTime maximumTime()const;
    void setMaximumTime(const QTime &date);

    void setTimeRange(const QTime &minTime, const QTime &maxTime);

    void setInterval(QDateTimeEdit::Section section,int difference);
    int interval(QDateTimeEdit::Section section) const;

    enum { Type = Hb::ItemType_DateTimePicker };
    int type() const { return Type; }

    void setGeometry(const QRectF &rect);

    QGraphicsItem *primitive(const QString &itemName) const;

public slots:
    void setDate(const QDate &date);
    void setDateTime(const QDateTime &datetime);
    void setTime(const QTime&);
    void updatePrimitives();

signals:
    void dateChanged(const QDate &date);
    void dateTimeChanged(const QDateTime &dateTime);
    void timeChanged(const QTime &time);

protected:
    HbDateTimePicker(HbDateTimePickerPrivate &dd, QGraphicsItem *parent = 0);
    bool event(QEvent *e);
    void initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive);

private:
    Q_DISABLE_COPY(HbDateTimePicker)  
    Q_DECLARE_PRIVATE_D(d_ptr, HbDateTimePicker)
    Q_PRIVATE_SLOT(d_func(), void _q_dayChanged(int index))
    Q_PRIVATE_SLOT(d_func(), void _q_monthChanged(int index))
    Q_PRIVATE_SLOT(d_func(), void _q_yearChanged(int index))
    Q_PRIVATE_SLOT(d_func(), void _q_hoursChanged(int index))
    Q_PRIVATE_SLOT(d_func(), void _q_minutesChanged(int index))
    Q_PRIVATE_SLOT(d_func(), void _q_secondsChanged(int index))
    Q_PRIVATE_SLOT(d_func(), void _q_ampmChanged(int index))
};

#endif //HBDATETIMEPICKER_H
