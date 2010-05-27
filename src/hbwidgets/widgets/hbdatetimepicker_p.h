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

#ifndef HBDATETIMEPICKER_P_H
#define HBDATETIMEPICKER_P_H

#include "hbwidget_p.h"
#include "hbtumbleview.h"
#include "hbdatetimepicker.h"

#include "hbextendedlocale.h"

#include <QStringListModel>
#include <QGraphicsLinearLayout>
#include <QDateTime>
#include <QGraphicsWidget>
#include <QList>
#include <QHash>


#define HBDATETIMEPICKER_TIME_MIN QTime(0, 0, 0, 0)
#define HBDATETIMEPICKER_TIME_MAX QTime(23, 59, 59, 999)
#define HBDATETIMEPICKER_DATE_MIN QDate(1900, 1, 1) //15th Oct 4713 B.C.
//#define HBDATETIMEPICKER_DATE_MIN QDate(-4713, 10, 15) //15th Oct 4713 B.C.
#define HBDATETIMEPICKER_DATE_MAX QDate(2099, 1, 1)
//#define HBDATETIMEPICKER_DATE_MAX QDate(9999, 12, 31)
#define HBDATETIMEPICKER_DATETIME_MAX QDateTime(HBDATETIMEPICKER_DATE_MAX, HBDATETIMEPICKER_TIME_MAX)
#define HBDATETIMEPICKER_DATETIME_MIN QDateTime(HBDATETIMEPICKER_DATE_MIN, HBDATETIMEPICKER_TIME_MIN)

class HbDateTimeParser
{
public:
    HbDateTimeParser(){
    }

    ~HbDateTimeParser(){
    }

    enum Section {
        NoSection = 0x00000,
        AmPmSection = 0x00001,
        MSecSection = 0x00002,
        SecondSection = 0x00004,
        MinuteSection = 0x00008,
        Hour12Section   = 0x00010,
        Hour24Section   = 0x00020,
        TimeSectionMask = (AmPmSection|MSecSection|SecondSection|MinuteSection|Hour12Section|Hour24Section),
//        Internal = 0x10000,
        DaySection = 0x00100,
        MonthSection = 0x00200,
        YearSection = 0x00400,
        YearSection2Digits = 0x00800,
        DayOfWeekSection = 0x01000,
        DateSectionMask = (DaySection|MonthSection|YearSection|YearSection2Digits|DayOfWeekSection),
        //FirstSection = 0x02000|Internal,
        //LastSection = 0x04000|Internal,
        //CalendarPopupSection = 0x08000|Internal,

        //NoSectionIndex = -1,
        //FirstSectionIndex = -2,
        //LastSectionIndex = -3,
        //CalendarPopupIndex = -4
    }; // duplicated from qdatetimeedit.h
    Q_DECLARE_FLAGS(Sections, Section)

    struct SectionNode {
        Section type;
        mutable int pos;
        int count;
    };

    bool parseFormat(const QString& format);
    int sectionSize(int index) const;
    int sectionPos(int index) const;
    int sectionPos(const SectionNode &sn) const;
    const SectionNode &sectionNode(int index) const;

    QString mDisplayFormat;
    Sections mDisplaySections;
    QVector<SectionNode> mSectionNodes;
    QStringList mSeparators;
};

class HbDateTimePickerPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbDateTimePicker)

public:
    HbDateTimePickerPrivate();
    ~HbDateTimePickerPrivate();

    void init(QVariant::Type type = QVariant::Date);
    QString localeDateTimeFormat(const QVariant::Type &mode);
    bool isFormatValid(const QString &newFormat);
    void parseDisplayFormat(const QString &format);
    void rearrangeTumbleViews();
    void makeConnections();
    void removeConnections();
    void setRanges();
    void emitDateChange();
    void emitTimeChange();
    void emitDateTimeChange();
    void syncVisualDate();

    void setDateTimeRange(const QDateTime &start
        , const QDateTime &end);
    void setDateTime(const QDateTime &newDateTime);
    void setMinimumDateTime(const QDateTime &newMinDateTime);
    void setMaximumDateTime(const QDateTime &newMaxDateTime);

    //helpers
    void setYearRange(int start,int end);
    void setMonthRange(int start,int end);
    void setDayRange(int start,int end);
    void setHourRange(int start,int end);
    void setMinuteRange(int start,int end);
    void setSecondRange(int start,int end);
    void setAmPm();

    //mix max testers
    inline bool isMinimumYear() { return mMinimumDate.date().year() == mDateTime.date().year(); }
    inline bool isMaximumYear() { return mMaximumDate.date().year() == mDateTime.date().year(); }
    inline bool isMinimumMonth() { return mMinimumDate.date().month() == mDateTime.date().month(); }
    inline bool isMaximumMonth() { return mMaximumDate.date().month() == mDateTime.date().month(); }
    inline bool isMinimumHour() { return mMinimumDate.time().hour() == mDateTime.time().hour(); }
    inline bool isMaximumHour() { return mMaximumDate.time().hour() == mDateTime.time().hour(); }
    inline bool isMinimumMinute() { return mMinimumDate.time().minute() == mDateTime.time().minute(); }
    inline bool isMaximumMinute() { return mMaximumDate.time().minute() == mDateTime.time().minute(); }

    QString localeYear(int year);//depending on current format returns the year
    QString localeMonth(int month);
    QString localeDay(int day);
    QString localeHour(int hour);
    QString localeMinute(int minute);
    QString localeSecond(int second);
    QString localeAmPm(bool getAm);

    //helper routine
    void resizeModel(QStringListModel *model,
        int oldStart, int oldEnd,
        int newStart, int newEnd,
        QString (HbDateTimePickerPrivate::*localeFuncPtr)(int), int interval = 1);

    void createPrimitives();
    void deleteAndNull(HbTumbleView*& t) {
        delete t;t=0;
    }
    void deleteAndNull(QStringListModel*& t) {
        delete t;t=0;
    }

public: 
    void _q_dayChanged(int index);
    void _q_monthChanged(int index);
    void _q_yearChanged(int index);
    void _q_hoursChanged(int index);
    void _q_minutesChanged(int index);
    void _q_secondsChanged(int index);
    void _q_ampmChanged(int index);

public:
    HbTumbleView *mDayPicker;
    HbTumbleView *mMonthPicker;
    HbTumbleView *mYearPicker;
    HbTumbleView *mHourPicker;
    HbTumbleView *mMinutePicker;
    HbTumbleView *mSecondPicker;
    HbTumbleView *mAmPmPicker;

    QStringListModel *mDayModel;
    QStringListModel *mMonthModel;
    QStringListModel *mYearModel;
    QStringListModel *mHourModel;
    QStringListModel *mMinuteModel;
    QStringListModel *mSecondModel;
    QStringListModel *mAmPmModel;

    int mYearOffset; 
    int mMonthOffset;
    int mDayOffset;
    int mHourOffset;
    int mMinuteOffset;
    int mSecondOffset;

    QDateTime mDateTime;
    QDateTime mMinimumDate;
    QDateTime mMaximumDate;
    QVariant::Type mDateTimeMode;

    QGraphicsLinearLayout *mLayout;
    QString mFormat;

    HbDateTimeParser mParser;
    QList<HbDateTimeParser::Section> mDisplaySections;
    QString mYearFormat;
    QString mMonthFormat;
    QString mDayFormat;
    QString mHourFormat;
    QString mMinuteFormat;
    QString mSecondFormat;
    bool mIs24HourFormat;
    bool mIsTwoDigitYearFormat;

    HbExtendedLocale mLocale;

    //graphics items/style items
    QGraphicsItem *mBackground;
    QGraphicsItem *mFrame;
    HbWidget *mContent;
    QHash<QDateTimeEdit::Section, int> mIntervals;
    QGraphicsItem   *mHighlight;
};

#endif //HBDATETIMEPICKER_P_H
