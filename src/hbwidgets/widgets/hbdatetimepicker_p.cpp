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

#include "hbdatetimepicker_p.h"
#include "hbdatetimepicker.h"
#include "hbstyleoption_p.h"
#include "hbfeaturemanager_r.h"

//TODO:remove frameitem dependency
#include "hbframeitem.h"

#include <QStringListModel>
#include <QGraphicsLinearLayout>
#include <QModelIndex>
#include <QDate>
#include <QLocale>
#include <QDebug>
#include <QStringListModel>
#include <QPointer>

#define HBDATETIMEPICKER_DEBUG

//////////HbDateTimeParser - Implementaion may change in future.//////////////

static inline int countRepeat(const QString &str, int index, int maxCount)
{
    int count = 1;
    const QChar ch(str.at(index));
    const int max = qMin(index + maxCount, str.size());
    while (index + count < max && str.at(index + count) == ch) {
        ++count;
    }
    return count;
}

static QString unquote(const QString &str)
{
    const QChar quote(QLatin1Char('\''));
    const QChar slash(QLatin1Char('\\'));
    const QChar zero(QLatin1Char('0'));
    QString ret;
    QChar status(zero);
    const int max = str.size();
    for (int i=0; i<max; ++i) { 
        if (str.at(i) == quote) {
            if (status != quote) {
                status = quote;
            } else if (!ret.isEmpty() && str.at(i - 1) == slash) {
                ret[ret.size() - 1] = quote;
            } else {
                status = zero;
            }
        } else {
            ret += str.at(i);
        }
    }
    return ret;
}

static inline void appendSeparator(QStringList *list, const QString &string, int from, int size, int lastQuote)
{
    QString str(string.mid(from, size));
    if (lastQuote >= from)
        str = unquote(str);
    list->append(str);
}

bool HbDateTimeParser::parseFormat(const QString &format)
{
    const QLatin1Char quote('\'');
    const QLatin1Char slash('\\');
    const QLatin1Char zero('0');

    if (format == mDisplayFormat && !format.isEmpty()) {
        return true;
    }

    QVector<SectionNode> newSectionNodes;
    Sections newDisplay = 0;
    QStringList newSeparators;
    int i, index = 0;
    int add = 0;
    QChar status(zero);
    const int max = format.size();
    int lastQuote = -1;
    for (i = 0; i<max; ++i) {
        if (format.at(i) == quote) {
            lastQuote = i;
            ++add;
            if (status != quote) {
                status = quote;
            } else if (format.at(i - 1) != slash) {
                status = zero;
            }
        } else if (status != quote) {
            const char sect = format.at(i).toLatin1();
            switch (sect) {
            case 'H':
            case 'h':
                {
                    const Section hour = (sect == 'h') ? Hour12Section : Hour24Section;
                    const SectionNode sn = { hour, i - add, countRepeat(format, i, 2) };
                    newSectionNodes.append(sn);
                    appendSeparator(&newSeparators, format, index, i - index, lastQuote);
                    i += sn.count - 1;
                    index = i + 1;
                    newDisplay |= hour;
                }
                break;
            case 'm':
                {
                    const SectionNode sn = { MinuteSection, i - add, countRepeat(format, i, 2) };
                    newSectionNodes.append(sn);
                    appendSeparator(&newSeparators, format, index, i - index, lastQuote);
                    i += sn.count - 1;
                    index = i + 1;
                    newDisplay |= MinuteSection;
                }
                break;
            case 's':
                {
                    const SectionNode sn = { SecondSection, i - add, countRepeat(format, i, 2) };
                    newSectionNodes.append(sn);
                    appendSeparator(&newSeparators, format, index, i - index, lastQuote);
                    i += sn.count - 1;
                    index = i + 1;
                    newDisplay |= SecondSection;
                }
                break;

            case 'z':
                {
                    const SectionNode sn = { MSecSection, i - add, countRepeat(format, i, 3) < 3 ? 1 : 3 };
                    newSectionNodes.append(sn);
                    appendSeparator(&newSeparators, format, index, i - index, lastQuote);
                    i += sn.count - 1;
                    index = i + 1;
                    newDisplay |= MSecSection;
                }
                break;
            case 'A':
            case 'a':
                {
                    const bool cap = (sect == 'A');
                    const SectionNode sn = { AmPmSection, i - add, (cap ? 1 : 0) };
                    newSectionNodes.append(sn);
                    appendSeparator(&newSeparators, format, index, i - index, lastQuote);
                    newDisplay |= AmPmSection;
                    if (i + 1 < format.size()
                        && format.at(i+1) == (cap ? QLatin1Char('P') : QLatin1Char('p'))) {
                            ++i;
                    }
                    index = i + 1;
                }
                break;
            case 'y':
                {
                    const int repeat = countRepeat(format, i, 4);
                    if (repeat >= 2) {
                        const SectionNode sn = { repeat == 4 ? YearSection : YearSection2Digits,
                            i - add, repeat == 4 ? 4 : 2 };
                        newSectionNodes.append(sn);
                        appendSeparator(&newSeparators, format, index, i - index, lastQuote);
                        i += sn.count - 1;
                        index = i + 1;
                        newDisplay |= sn.type;
                    }
                }
                break;
            case 'M':
                {
                    const SectionNode sn = { MonthSection, i - add, countRepeat(format, i, 4) };
                    newSectionNodes.append(sn);
                    newSeparators.append(unquote(format.mid(index, i - index)));
                    i += sn.count - 1;
                    index = i + 1;
                    newDisplay |= MonthSection;
                }
                break;
            case 'd':
                {
                    const int repeat = countRepeat(format, i, 4);
                    const SectionNode sn = { repeat >= 3 ? DayOfWeekSection : DaySection, i - add, repeat };
                    newSectionNodes.append(sn);
                    appendSeparator(&newSeparators, format, index, i - index, lastQuote);
                    i += sn.count - 1;
                    index = i + 1;
                    newDisplay |= sn.type;
                }
                break;

            default:
                break;
            }
        }
    }

    if ((newDisplay & (AmPmSection|Hour12Section)) == Hour12Section) {
        const int max = newSectionNodes.size();
        for (int i=0; i<max; ++i) {
            SectionNode &node = newSectionNodes[i];
            if (node.type == Hour12Section)
                node.type = Hour24Section;
        }
    }

    if (index < format.size()) {
        appendSeparator(&newSeparators, format, index, index - max, lastQuote);
    } else {
        newSeparators.append(QString());
    }

    mDisplayFormat = format;
    mSeparators = newSeparators;
    mSectionNodes = newSectionNodes;
    mDisplaySections = newDisplay;
    
    return true;
}

int HbDateTimeParser::sectionSize(int sectionIndex) const
{
    if (sectionIndex < 0)
        return 0;

    if (sectionIndex >= mSectionNodes.size()) {
        qWarning("QDateTimeParser::sectionSize Internal error (%d)", sectionIndex);
        return -1;
    }
    if (sectionIndex == mSectionNodes.size() - 1) {
        return mDisplayFormat.size() - sectionPos(sectionIndex) - mSeparators.last().size();
    } else {
        return sectionPos(sectionIndex + 1) - sectionPos(sectionIndex)
            - mSeparators.at(sectionIndex + 1).size();
    }
}

int HbDateTimeParser::sectionPos(int sectionIndex) const
{
    return sectionPos(sectionNode(sectionIndex));
}

int HbDateTimeParser::sectionPos(const SectionNode &sn) const
{
    if (sn.pos == -1) {
        return -1;
    }

    return sn.pos;
}

const HbDateTimeParser::SectionNode &HbDateTimeParser::sectionNode(int sectionIndex) const
{
    return mSectionNodes.at(sectionIndex);
}
/////////////////////////////////////////////////


HbDateTimePickerPrivate::HbDateTimePickerPrivate()
:HbWidgetPrivate()
    ,mDayPicker(0)
    ,mMonthPicker(0)
    ,mYearPicker(0)
    ,mHourPicker(0)
    ,mMinutePicker(0)
    ,mSecondPicker(0)
    ,mAmPmPicker(0)
    ,mDayModel(0)
    ,mMonthModel(0)
    ,mYearModel(0)
    ,mHourModel(0)
    ,mMinuteModel(0)
    ,mSecondModel(0)
    ,mAmPmModel(0)
    ,mYearOffset(-1)
    ,mMonthOffset(-1)
    ,mDayOffset(-1)
    ,mHourOffset(-1)
    ,mMinuteOffset(-1)
    ,mSecondOffset(-1)
    ,mDateTime(QDateTime::currentDateTime())
    ,mDateTimeMode(QVariant::Date) //default is date mode
    ,mLayout(0)         
    //,mFormat() //set the format again in init()
    //,mDisplaySecions() //is blank by default
    ,mParser()
    ,mYearFormat()
    ,mMonthFormat()
    ,mDayFormat()
    ,mHourFormat()
    ,mMinuteFormat()
    ,mSecondFormat() 
    ,mIs24HourFormat(false)
    ,mIsTwoDigitYearFormat(false)
    ,mBackground(0)
    ,mFrame(0)
    ,mContent(0)
    ,mIntervals()
    ,mHighlight(0)
{
    mMinimumDate = HBDATETIMEPICKER_DATETIME_MIN;
    mMaximumDate = HBDATETIMEPICKER_DATETIME_MAX;
    mDateTime = mMinimumDate;
}

HbDateTimePickerPrivate::~HbDateTimePickerPrivate()
{
}

/*
   called only once. while the widget is constructing.
*/

void HbDateTimePickerPrivate::init(QVariant::Type dateTimeMode)
{
    Q_Q(HbDateTimePicker);

    //create base content widget which contains the tumble views
    mContent=new HbWidget(q);
    mLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    mLayout->setSpacing(1);
    mLayout->setContentsMargins(0,0,0,0);
    mContent->setLayout(mLayout);
    q->style()->setItemName(mContent,"content");

    mDateTimeMode = dateTimeMode;

    //read the format from locale
    mFormat = localeDateTimeFormat(dateTimeMode);

    //parse the format and set the sections in order
    parseDisplayFormat(mFormat);

    //create primitives
    createPrimitives();

    //recreate and rearrange depending on the format
    rearrangeTumbleViews();
}

/*!
  \internal

  Getting the format from the local settings.
*/
QString HbDateTimePickerPrivate::localeDateTimeFormat(const QVariant::Type &dateTimeMode)
{
    if(dateTimeMode == QVariant::Date) {
        return mLocale.dateFormat(QLocale::ShortFormat);
    } else if(dateTimeMode == QVariant::Time) {
        return mLocale.timeFormat(QLocale::ShortFormat);
    }
    return mLocale.dateTimeFormat(QLocale::ShortFormat);
}

bool HbDateTimePickerPrivate::isFormatValid(const QString &newDisplayFormat)
{
    if(newDisplayFormat == mFormat) {
        return false;
    }
    return true;
}

/* 
   this will reset the display sections and re add them in order
   mentioned in the display format passed. this also sets the mIs24HourFormat var.
*/
void HbDateTimePickerPrivate::parseDisplayFormat(const QString &format)
{
    if(mParser.parseFormat(format)) {
        for(int i=0;i<mParser.mSectionNodes.count();++i) {
            switch(mParser.mSectionNodes[i].type) {
                case HbDateTimeParser::DaySection:
                case HbDateTimeParser::DayOfWeekSection:
                    mDayFormat = QString(mParser.mSectionNodes[i].count,'d');
                    break;

                case HbDateTimeParser::MonthSection:
                    mMonthFormat = QString(mParser.mSectionNodes[i].count,'M');
                    break;

                case HbDateTimeParser::YearSection:
                    mIsTwoDigitYearFormat = false;
                    mYearFormat = QString(mParser.mSectionNodes[i].count,'y');
                    break;

                case HbDateTimeParser::YearSection2Digits:
                    mIsTwoDigitYearFormat = true;
                    mYearFormat = QString(mParser.mSectionNodes[i].count,'y');
                    break;

                case HbDateTimeParser::SecondSection:
                    mSecondFormat = QString(mParser.mSectionNodes[i].count,'s');
                    break;

                case HbDateTimeParser::MinuteSection:
                    mMinuteFormat = QString(mParser.mSectionNodes[i].count,'m');
                    break;

                case HbDateTimeParser::Hour12Section:
                    mIs24HourFormat = false;
                    mHourFormat = QString(mParser.mSectionNodes[i].count,'h');
                    break;

                case HbDateTimeParser::Hour24Section:
                    mIs24HourFormat = true;
                    mHourFormat = QString(mParser.mSectionNodes[i].count,'h');
                    break;

                default:
                    break;
                    /*case HbDateTimeParser::DayOfWeekSection: not supported */
            }
        }
    }
}

/*
   this is called whenever the setDisplayFormat changes.
   function deletes all tumbleviews which currently exist and
   it creates the ones which are required and makes the connections.
*/
void HbDateTimePickerPrivate::rearrangeTumbleViews()
{                  
    Q_Q(HbDateTimePicker);

    deleteAndNull(mYearPicker);
    deleteAndNull(mMonthPicker);
    deleteAndNull(mDayPicker);
    deleteAndNull(mHourPicker);
    deleteAndNull(mMinutePicker);
    deleteAndNull(mSecondPicker);
    deleteAndNull(mAmPmPicker);

    deleteAndNull(mYearModel);
    deleteAndNull(mDayModel);
    deleteAndNull(mMonthModel);
    deleteAndNull(mHourModel);
    deleteAndNull(mMinuteModel);
    deleteAndNull(mSecondModel);
    deleteAndNull(mAmPmModel);

    mYearOffset = -1;
    mMonthOffset = -1;
    mDayOffset = -1;
    mHourOffset = -1;
    mMinuteOffset = -1;
    mSecondOffset = -1;

    QPointer<HbTumbleView> lastAdded;

    for(int i=0;i<mParser.mSectionNodes.count();i++) {
        switch(mParser.mSectionNodes[i].type) {
            case HbDateTimeParser::AmPmSection:
                mAmPmPicker = new HbTumbleView(q);
                mAmPmModel = new QStringListModel(q);
                mAmPmPicker->setModel(mAmPmModel);
                mAmPmPicker->setLoopingEnabled(true);
                mLayout->addItem(mAmPmPicker);
                mAmPmPicker->primitive("highlight")->hide();
                mAmPmPicker->primitive("separator")->show();
                lastAdded = mAmPmPicker;
                break;

            case HbDateTimeParser::DaySection:
            case HbDateTimeParser::DayOfWeekSection:
                mDayPicker = new HbTumbleView(q);
                mDayModel = new QStringListModel(q);
                mDayPicker->setModel(mDayModel);
                mDayPicker->setLoopingEnabled(true);
                mLayout->addItem(mDayPicker);
                mDayPicker->primitive("separator")->show();
                lastAdded = mDayPicker;
                break;

            case HbDateTimeParser::MonthSection:
                mMonthPicker = new HbTumbleView(q);
                mMonthModel = new QStringListModel(q);
                mMonthPicker->setModel(mMonthModel);
                mMonthPicker->setLoopingEnabled(true);
                mLayout->addItem(mMonthPicker);
                mMonthPicker->primitive("separator")->show();
                lastAdded = mMonthPicker;
                break;

            case HbDateTimeParser::YearSection:
            case HbDateTimeParser::YearSection2Digits:
                mYearPicker = new HbTumbleView(q);
                mYearModel = new QStringListModel(q);
                mYearPicker->setModel(mYearModel);
                mYearPicker->setLoopingEnabled(true);
                mLayout->addItem(mYearPicker);
                mYearPicker->primitive("separator")->show();
                lastAdded = mYearPicker;
                break;

            case HbDateTimeParser::SecondSection:
                mSecondPicker = new HbTumbleView(q);
                mSecondModel = new QStringListModel(q);
                mSecondPicker->setModel(mSecondModel);
                mSecondPicker->setLoopingEnabled(true);
                mLayout->addItem(mSecondPicker);
                mSecondPicker->primitive("separator")->show();
                lastAdded = mSecondPicker;
                break;

            case HbDateTimeParser::MinuteSection:
                mMinutePicker = new HbTumbleView(q);
                mMinuteModel = new QStringListModel(q);
                mMinutePicker->setModel(mMinuteModel);
                mMinutePicker->setLoopingEnabled(true);
                mLayout->addItem(mMinutePicker);
                mMinutePicker->primitive("separator")->show();
                lastAdded = mMinutePicker;
                break;

            case HbDateTimeParser::Hour12Section:
            case HbDateTimeParser::Hour24Section:
                mHourPicker = new HbTumbleView(q);
                mHourModel = new QStringListModel(q);
                mHourPicker->setModel(mHourModel);
                mHourPicker->setLoopingEnabled(true);
                mLayout->addItem(mHourPicker);
                mHourPicker->primitive("separator")->show();
                lastAdded = mHourPicker;
                break;

            default:
                break;
        }
        
        if(lastAdded){
            lastAdded->primitive("highlight")->hide();
            lastAdded->primitive("separator")->show();
        }

    }

    //For the last added tumble view, hide the separator.
    if(lastAdded){
        lastAdded->primitive("separator")->hide();
    }

    setRanges();
    makeConnections();
    syncVisualDate();
    //TODO:what to do with current date, should reset ?
}

void HbDateTimePickerPrivate::makeConnections()
{
    Q_Q(HbDateTimePicker);
    bool b=false;
    if(mYearPicker) {
        b=QObject::connect(mYearPicker,SIGNAL(itemSelected(int)),q,SLOT(_q_yearChanged(int)));
        Q_ASSERT(b);
    }
    if(mMonthPicker) {
        b=QObject::connect(mMonthPicker,SIGNAL(itemSelected(int)),q,SLOT(_q_monthChanged(int)));
        Q_ASSERT(b);
    }
    if(mDayPicker) {
        b=QObject::connect(mDayPicker,SIGNAL(itemSelected(int)),q,SLOT(_q_dayChanged(int)));
        Q_ASSERT(b);
    }
    if(mHourPicker) {
        b=QObject::connect(mHourPicker,SIGNAL(itemSelected(int)),q,SLOT(_q_hoursChanged(int)));
        Q_ASSERT(b);
    }
    if(mMinutePicker) {
        b=QObject::connect(mMinutePicker,SIGNAL(itemSelected(int)),q,SLOT(_q_minutesChanged(int)));
        Q_ASSERT(b);
    }
    if(mSecondPicker) {
        b=QObject::connect(mSecondPicker,SIGNAL(itemSelected(int)),q,SLOT(_q_secondsChanged(int)));
        Q_ASSERT(b);
    }
    if(mAmPmPicker) {
        b=QObject::connect(mAmPmPicker,SIGNAL(itemSelected(int)),q,SLOT(_q_ampmChanged(int)));
        Q_ASSERT(b);
    }
    Q_UNUSED(b);
}
void HbDateTimePickerPrivate::removeConnections()
{
    Q_Q(HbDateTimePicker);    
    if(mYearPicker) {
        QObject::disconnect(mYearPicker,SIGNAL(itemSelected(int)),q,SLOT(_q_yearChanged(int)));
    }
    if(mMonthPicker) {
        QObject::disconnect(mMonthPicker,SIGNAL(itemSelected(int)),q,SLOT(_q_monthChanged(int)));
    }
    if(mDayPicker) {
        QObject::disconnect(mDayPicker,SIGNAL(itemSelected(int)),q,SLOT(_q_dayChanged(int)));
    }
    if(mHourPicker) {
        QObject::disconnect(mHourPicker,SIGNAL(itemSelected(int)),q,SLOT(_q_hoursChanged(int)));
    }
    if(mMinutePicker) {
        QObject::disconnect(mMinutePicker,SIGNAL(itemSelected(int)),q,SLOT(_q_minutesChanged(int)));
    }
    if(mSecondPicker) {
        QObject::disconnect(mSecondPicker,SIGNAL(itemSelected(int)),q,SLOT(_q_secondsChanged(int)));
    }
    if(mAmPmPicker) {
        QObject::disconnect(mAmPmPicker,SIGNAL(itemSelected(int)),q,SLOT(_q_ampmChanged(int)));
    }
}


void HbDateTimePickerPrivate::setRanges() 
{
    if(mIsTwoDigitYearFormat) {
        mYearOffset = mMinimumDate.date().year()%100;
        setYearRange(mMinimumDate.date().year()%100,mMaximumDate.date().year()%100); 
    } else {
        mYearOffset = mMinimumDate.date().year();
        setYearRange(mMinimumDate.date().year(),mMaximumDate.date().year()); 
    }

    mMonthOffset = 1;
    setMonthRange(1,12);//default all months

    mDayOffset = 1;
    setDayRange(1,31);//default all days

    mHourOffset = 0;
    setHourRange(0,23);

    mMinuteOffset = 0;
    setMinuteRange(0,59);

    mSecondOffset = 0;
    setSecondRange(0,59);

    setAmPm();
}

void HbDateTimePickerPrivate::syncVisualDate()
{
    //no matter how good this is written , there always seems to be a gap
    //between visual and actual selected data. creating syncup with visual.

    int y=1,m=1,d=1,h=0,n=0,s=0;

    if(mYearPicker) {
        y = mYearOffset+mYearPicker->selected();
    }
    if(mMonthPicker) {
        m = mMonthOffset+mMonthPicker->selected();
    }
    if(mDayPicker) {
        d = mDayOffset +mDayPicker->selected();
    }

    if(mHourPicker) {
        h=mHourOffset+mHourPicker->selected();
    }

    if(mMinutePicker) {
        n=mMinuteOffset+mMinutePicker->selected();
    }
    if(mSecondPicker) {
        s=mSecondOffset+mSecondPicker->selected();
    }
    QDateTime dt(QDate(y,m,d),QTime(h,n,s));
    if(dt.isValid()) {
        mDateTime = dt;
    }
}

void HbDateTimePickerPrivate::emitDateChange()
{
    Q_Q(HbDateTimePicker);

    emit q->dateChanged(mDateTime.date());
    emit q->dateTimeChanged(mDateTime);
}

void HbDateTimePickerPrivate::emitTimeChange()
{
    Q_Q(HbDateTimePicker);

    emit q->timeChanged(mDateTime.time());
    emit q->dateTimeChanged(mDateTime);

}
void HbDateTimePickerPrivate::emitDateTimeChange()
{
    Q_Q(HbDateTimePicker);

    emit q->dateChanged(mDateTime.date());
    emit q->timeChanged(mDateTime.time());
    emit q->dateTimeChanged(mDateTime);

}


void HbDateTimePickerPrivate::setDateTimeRange(const QDateTime &startdt,
        const QDateTime &enddt)
{
    Q_Q(HbDateTimePicker);
    QDateTime start(startdt);
    QDateTime end(enddt);
    if(start.isValid() && end.isValid()) {

        removeConnections();

        //change the date range
        if(start.date() > end.date()) {
            end.setDate(start.date());
        }
        if(mIsTwoDigitYearFormat) {
            setYearRange(start.date().year()%100,end.date().year()%100);
        } else {
            setYearRange(start.date().year(),end.date().year());
        }

        //change the time range
        if(start.time() > end.time()) {
            end.setTime(start.time());
        }
        setHourRange(start.time().hour(),end.time().hour());

        mMinimumDate = start;
        mMaximumDate = end;

        //scroll to the clamped date
        bool dirty=false;
        if(mDateTime < mMinimumDate) {
            dirty = true;
            mDateTime = mMinimumDate;
        } else if(mDateTime > mMaximumDate) {
            dirty = true;
            mDateTime = mMaximumDate;
        }
        if(dirty) {
            setDateTime(mDateTime);
            //force the year change so month range gets set appropriately
            //TODO:fix after modularization of entire code
        }

        if(mYearPicker) {
            _q_yearChanged(mYearPicker->selected());
        }

        //force the hour change so minute range gets set appropriately
        if(mHourPicker) {
            _q_hoursChanged(mHourPicker->selected());
        }

        //emit the changes
        emit q->dateChanged(mDateTime.date());
        emit q->timeChanged(mDateTime.time());
        emit q->dateTimeChanged(mDateTime);          

        makeConnections();
    }
}
void HbDateTimePickerPrivate::setDateTime(const QDateTime &newDate)
{
    QDateTime newDateTime(newDate);
    //TODO: validity and bounds check for selected
    if(newDateTime.isValid()) {

        //update the mem var
        if(newDateTime.date() < mMinimumDate.date()) {
            newDateTime.setDate(mMinimumDate.date());
        } else if(newDateTime.date() > mMaximumDate.date()) {
            newDateTime.setDate(mMaximumDate.date());
        }

        if(newDateTime.time() < mMinimumDate.time()) {
            newDateTime.setTime(mMinimumDate.time());
        }else if(newDateTime.time() > mMaximumDate.time()) {
            newDateTime.setTime(mMaximumDate.time());
        }

        //set the selections
        if(mYearPicker) {
#ifdef HBDATETIMEPICKER_DEBUG
            qDebug() << "setDateTime: yearOffset=" << mYearOffset;
#endif
            if(mIsTwoDigitYearFormat) {
                mYearPicker->setSelected((newDateTime.date().year()%100)-mYearOffset);
            } else {
                mYearPicker->setSelected(newDateTime.date().year()-mYearOffset);
            }
        }
        if(mMonthPicker) {
            mMonthPicker->setSelected(newDateTime.date().month()-mMonthOffset);
        }
        if(mDayPicker) {
            mDayPicker->setSelected(newDateTime.date().day()-mDayOffset);
        }
        if(mHourPicker) {
            mHourPicker->setSelected(newDateTime.time().hour()-mHourOffset);
        }

        if(mMinutePicker) {
            int index = newDateTime.time().minute()-mMinuteOffset;
            if(mIntervals[QDateTimeEdit::MinuteSection] > 1){
                for(int i = 0; i < mMinuteModel->rowCount(); i++){

                    if(mMinuteModel->index(i,0).data().toInt() <= newDateTime.time().minute()){
                        index = i;
                    }
                    else{
                        break;
                    }

                    //TODO: if minute is not in the model data then nearest value should be selected.
                }
            }
            else{
                index = newDateTime.time().minute()-mMinuteOffset;
            }
            mMinutePicker->setSelected(index);
        }

        if(mSecondPicker) {
#ifdef HBDATETIMEPICKER_DEBUG
            qDebug() << "setDateTime before: secondOffset=" << mSecondOffset << " time=" << newDateTime.time();
#endif
            mSecondPicker->setSelected(newDateTime.time().second()-mSecondOffset);
#ifdef HBDATETIMEPICKER_DEBUG
            qDebug() << "setDateTime after: secondOffset=" << mSecondOffset << " time=" << newDateTime.time();
#endif
        }
        if(mAmPmPicker){
            if(newDate.time().hour() >= 12){
                mAmPmPicker->setSelected(1);
            }
            else{
                mAmPmPicker->setSelected(0);
            }
        }

        mDateTime = newDateTime;

    }
}
void HbDateTimePickerPrivate::setMinimumDateTime(const QDateTime &newMinDateTime)
{
    setDateTimeRange(newMinDateTime,mMaximumDate);
}
void HbDateTimePickerPrivate::setMaximumDateTime(const QDateTime &newMaxDateTime)
{
    setDateTimeRange(mMinimumDate,newMaxDateTime);
}


void HbDateTimePickerPrivate::setYearRange(int start,int end)
{
    if(!mYearPicker) {
        return;
    }
    //calculate the index it should be after resize
    //the currentIndex gets reset after the resize and gets set to 0
    //to work around that issue this is added
    int newIndex = mYearPicker->selected()-(start-mYearOffset);
    if(newIndex < 0) {
        newIndex = 0;
    }
    if(newIndex > (end-start)) {
        newIndex = end-start;
    }

#ifdef HBDATETIMEPICKER_DEBUG
    qDebug() << "setyear range (" << start << "," << end << ")" ;
#endif



    resizeModel(mYearModel, mYearOffset, 
        mYearOffset+mYearModel->rowCount()-1, start, 
        end, &HbDateTimePickerPrivate::localeYear);

    mYearOffset = start;

    mYearPicker->setSelected(newIndex);
}

void HbDateTimePickerPrivate::setMonthRange(int start,int end)
{
    if(!mMonthPicker) {
        return;
    }
#ifdef HBDATETIMEPICKER_DEBUG
    qDebug() << "setMonthRange: " << start << " ," << end;
#endif
    //calculate the index it should be after resize
    //the currentIndex gets reset after the resize and gets set to 0
    //to work around that issue this is added
    int newIndex = mMonthPicker->selected()-(start-mMonthOffset);
    if(newIndex < 0) {
        newIndex = 0;
    }
    if(newIndex > (end-start)) {
        newIndex = end-start;
    }
    resizeModel(mMonthModel,
            mMonthOffset,mMonthOffset+mMonthModel->rowCount()-1,
            start,end,
            &HbDateTimePickerPrivate::localeMonth);
    mMonthOffset = start;

    mMonthPicker->setSelected(newIndex);

    //check if current month is valid
    if(mDateTime.date().month() < start) {
        mDateTime.setDate(QDate(mDateTime.date().year(),start,mDateTime.date().day()));
    }
    else if(mDateTime.date().month() > end) {
        mDateTime.setDate(QDate(mDateTime.date().year(),end,mDateTime.date().day()));
    }
}


void HbDateTimePickerPrivate::setDayRange(int start,int end)
{
    if(!mDayPicker) {
        return;
    }
#ifdef HBDATETIMEPICKER_DEBUG
    qDebug() << "setDayRange: " << start << " ," << end;
#endif
    //calculate the index it should be after resize
    //the currentIndex gets reset after the resize and gets set to 0
    //to work around that issue this is added
    int newIndex = mDayPicker->selected()-(start-mDayOffset);
    if(newIndex < 0) {
        newIndex = 0;
    }
    if(newIndex > (end-start)) {
        newIndex = end-start;
    }

    resizeModel(mDayModel,
            mDayOffset,mDayOffset+mDayModel->rowCount()-1,
            start,end,
            &HbDateTimePickerPrivate::localeDay);
    mDayOffset = start;

    mDayPicker->setSelected(newIndex);


    //check if current day is valid
    if(mDateTime.date().day() < start) {
        mDateTime.setDate(QDate(mDateTime.date().year(),mDateTime.date().month(),start));
    }
    else if(mDateTime.date().day() > end) {
        mDateTime.setDate(QDate(mDateTime.date().year(),mDateTime.date().month(),end));
    }
}

void HbDateTimePickerPrivate::setHourRange(int start,int end)
{
    
    if(!mHourPicker) {
        return;
    }
#ifdef HBDATETIMEPICKER_DEBUG
    qDebug() << "setHourRange: " << start << " ," << end;
#endif
    //calculate the index it should be after resize
    //the currentIndex gets reset after the resize and gets set to 0
    //to work around that issue this is added
    int newIndex = mHourPicker->selected()-(start-mHourOffset);
    if(newIndex < 0) {
        newIndex = 0;
    }
    if(newIndex > (end-start)) {
        newIndex = end-start;
    }

    resizeModel(mHourModel,
            mHourOffset,mHourOffset+mHourModel->rowCount()-1,
            start,end,
            &HbDateTimePickerPrivate::localeHour);
    mHourOffset = start;

    mHourPicker->setSelected(newIndex);

    //check if hour is valid
    if(mDateTime.time().hour() < start) {
        mDateTime.setTime(QTime(start,mDateTime.time().minute(),mDateTime.time().second()));
    }
    else if(mDateTime.time().hour() > end) {
        mDateTime.setTime(QTime(end,mDateTime.time().minute(),mDateTime.time().second()));
    }
}
void HbDateTimePickerPrivate::setMinuteRange(int start,int end)
{
    
    if(!mMinutePicker) {
        return;
    }
#ifdef HBDATETIMEPICKER_DEBUG
    qDebug() << "setMinuteRange: " << start << " ," << end;
#endif
    //calculate the index it should be after resize
    //the currentIndex gets reset after the resize and gets set to 0
    //to work around that issue this is added
    int newIndex = mMinutePicker->selected()-(start-mMinuteOffset);
    if(newIndex < 0) {
        newIndex = 0;
    }
    if(newIndex > (end-start)) {
        newIndex = end-start;
    }

    //Store the value before resizing the model.
    int value = mMinuteModel->index(mMinutePicker->selected()).data().toInt();

    if(mIntervals[QDateTimeEdit::MinuteSection] > 1){ 
        if((mIntervals[QDateTimeEdit::MinuteSection] <= mMinimumDate.time().minute()) &&
            !isMinimumHour()){
                int i = 0;
                for(i = start; i > 0; i -= mIntervals[QDateTimeEdit::MinuteSection]){

                }

                start = i;
        }
        else{
            start = mMinimumDate.time().minute();
        }
    }

    resizeModel(mMinuteModel, mMinuteOffset,mMinuteModel->index(mMinuteModel->rowCount() - 1).data().toInt()/*mMinuteOffset+mMinuteModel->mMinuteModel->rowCount()-1*/,
        start,end,
        &HbDateTimePickerPrivate::localeMinute, mIntervals[QDateTimeEdit::MinuteSection]);
    mMinuteOffset = start;

    //Select the nearest value when the range is set.
    int index = newIndex;
    if(mIntervals[QDateTimeEdit::MinuteSection] > 1){
        for(int i = 0; i < mMinuteModel->rowCount(); i++){

            if(mMinuteModel->index(i,0).data().toInt() <= value){
                index = i;
            }
            else{
                break;
            }
        }
    }
    mMinutePicker->setSelected(index);

    mDateTime.setTime(QTime(mDateTime.time().hour(), localeMinute(mMinuteModel->index(index,0).data().toInt()).toInt(), mDateTime.time().second()));

    //check if minute is valid
    if(mDateTime.time().minute() < start) {
        mDateTime.setTime(QTime(mDateTime.time().hour(),start,mDateTime.time().second()));
    }
    else if(mDateTime.time().minute() > end) {
        mDateTime.setTime(QTime(mDateTime.time().hour(),end,mDateTime.time().second()));
    }
}
void HbDateTimePickerPrivate::setSecondRange(int start,int end)
{
    
    if(!mSecondPicker) {
        return;
    }
#ifdef HBDATETIMEPICKER_DEBUG
    qDebug() << "setSecondRange: " << start << " ," << end;
#endif
    //calculate the index it should be after resize
    //the currentIndex gets reset after the resize and gets set to 0
    //to work around that issue this is added
    int newIndex = mSecondPicker->selected()-(start-mSecondOffset);
    if(newIndex < 0) {
        newIndex = 0;
    }
    if(newIndex > (end-start)) {
        newIndex = end-start;
    }

    resizeModel(mSecondModel,
            mSecondOffset,mSecondOffset+mSecondModel->rowCount()-1,
            start,end,
            &HbDateTimePickerPrivate::localeSecond);
    mSecondOffset = start;

    mSecondPicker->setSelected(newIndex);

    //check if second is valid
    if(mDateTime.time().second() < start) {
        mDateTime.setTime(QTime(mDateTime.time().hour(),mDateTime.time().minute(),start));
    }
    else if(mDateTime.time().second() > end) {
        mDateTime.setTime(QTime(mDateTime.time().hour(),mDateTime.time().minute(),end));
    }
}

void HbDateTimePickerPrivate::setAmPm()
{
    if(!mAmPmPicker) {
        return;
    }
    //TODO: check if range has both am and pm
    QStringList amList;
    amList << localeAmPm(true) << localeAmPm(false);
    mAmPmModel->setStringList(amList);
}

QString HbDateTimePickerPrivate::localeYear(int year)
{
    //TODO:locale and format stuff
    if(mIsTwoDigitYearFormat) {
        return mLocale.toString(QDate(1900+year,1,1),mYearFormat);//year 00 is invalid
    }
    return mLocale.toString(QDate(year,1,1),mYearFormat);
}

QString HbDateTimePickerPrivate::localeMonth(int month)
{
    return mLocale.toString(QDate(2000,month,1),mMonthFormat);

}
QString HbDateTimePickerPrivate::localeDay(int day)
{
    return mLocale.toString(QDate(2000,1,day),mDayFormat);
}

QString HbDateTimePickerPrivate::localeHour(int hour)
{
    if(mIs24HourFormat) {
        return mLocale.toString(QTime(hour,0,0),mHourFormat);
    } 

    QString hourStr=mLocale.toString(QTime(hour,0,0),QString("%1:%2").arg(mHourFormat).arg("ap"));
    QStringList hourAm=hourStr.split(":");
    if(hourAm.count() > 1) {
        return hourAm.at(0);
    }
    return QString("Format Err");
}

QString HbDateTimePickerPrivate::localeMinute(int minute)
{
    return mLocale.toString(QTime(0,minute,0),mMinuteFormat);
}

QString HbDateTimePickerPrivate::localeSecond(int second)
{
    return mLocale.toString(QTime(0,0,second),mSecondFormat);
}

QString HbDateTimePickerPrivate::localeAmPm(bool isAm)
{
    QString text = isAm ? mLocale.amText() : mLocale.pmText();
#ifdef HB_TEXT_MEASUREMENT_UTILITY
    if ( HbFeatureManager::instance()->featureStatus( HbFeatureManager::TextMeasurement ) ) {
        text.append(QChar(LOC_TEST_START));
        text.append("qtl_datetimepicker_popup_ampm_sec");
        text.append(QChar(LOC_TEST_END));
    }
#endif
    return text;
}


/* there are seven different models for seven pickers. insertion and removal
   to each of the models to resize them its the same logic. but to populate the
   data need the appropriate locale and format converted data. which is passed
   as a function pointer instead of creating seven different QStringListModel derived
   model classes with one interface/virtual fuction specialization.
*/
void HbDateTimePickerPrivate::resizeModel(QStringListModel *model,
            int oldStart, int oldEnd,
            int newStart, int newEnd,
            QString (HbDateTimePickerPrivate::*localeFunc)(int), int interval)
{
    if((model->rowCount() == 0) && (newEnd-newStart>=0)) {
        //initialize condition
        int previous = newStart;
        for(int i=0;i<=newEnd-newStart;i++) {
            QString text;

            if(interval > 1){

                if(previous <= newEnd){
                    model->insertRow(i);
                    text = (this->*localeFunc)(previous);
                }
                else{
                    break;
                }

                previous += interval;
            }
            else{
                model->insertRow(i);
                text = (this->*localeFunc)(i+newStart);
            }

#ifdef HB_TEXT_MEASUREMENT_UTILITY
            if ( localeFunc == &HbDateTimePickerPrivate::localeMonth &&
                HbFeatureManager::instance()->featureStatus( HbFeatureManager::TextMeasurement ) ) {
                    text.append(QChar(LOC_TEST_START));
                    text.append("qtl_datetimepicker_popup_month_sec");
                    text.append(QChar(LOC_TEST_END));
            }
#endif
            QModelIndex index=model->index(i,0);
            if(index.isValid()) {
                model->setData(index,text);//TODO:add a readable typedef
            }

        }
        return;
    }

    if(newStart < oldStart) {
        int previous = newStart;

        for(int i=0;i<oldStart-newStart;++i) {
            QString text;

            if(interval > 1){

                if(previous < oldStart){
                    model->insertRow(i);
                    text = (this->*localeFunc)(previous);
                }
                else{
                    break;
                }

                previous += interval;
            }
            else{
                model->insertRow(i);
                text = (this->*localeFunc)(i+newStart);
            }

            QModelIndex index=model->index(i,0);
            if(index.isValid()) {
                model->setData(index,text);
            }
        }
    }
    if(newEnd > oldEnd) {

        int rowCount = model->rowCount(); 
        int previous = oldEnd+interval;
        for(int i=0;i<newEnd-oldEnd;++i) {
            QString text;

            if(interval > 1){

                if(previous <= newEnd){
                    model->insertRows(rowCount+i,1);
                    text = (this->*localeFunc)(previous);
                }
                else{
                    break;
                }

                previous += interval;
            }
            else{
                model->insertRows(rowCount+i,1);
                text = (this->*localeFunc)(oldEnd+i+1);
            }

            QModelIndex index=model->index(rowCount+i,0);
            if(index.isValid()) {
                model->setData(index,text);
            }
        }
    }

    if(newStart > oldStart) {
        if(interval > 1){
            for(int i = oldStart; i < newStart; i += interval){
                model->removeRows(0, 1);
            }
        }
        else{
            model->removeRows(0,newStart-oldStart);
        }
    }

    if(oldEnd > newEnd) {
        if(interval > 1){
            for(int i = oldEnd; i > newEnd; i -= interval){
                model->removeRows(model->rowCount()-1, 1);
            }
        }
        else{
            model->removeRows((model->rowCount()-(oldEnd-newEnd)),oldEnd-newEnd);
        }
    }

    if(interval > 1){
        //Check if there's any mismatch between actual rows in the model and the supposed rows.
        int previous = newStart;
        int actualRowCount = 0;
        for(actualRowCount=0;actualRowCount<=newEnd-newStart;actualRowCount++) {
            if(previous <= newEnd){
            }
            else{
                break;
            }

            previous += interval;
        }

        if(actualRowCount > model->rowCount()){
            model->insertRows(model->rowCount(), actualRowCount - model->rowCount());
        }
        else if( actualRowCount < model->rowCount()){
            model->removeRows(model->rowCount()-1, model->rowCount() - actualRowCount);
        }

        //Populate the data in the model.
        previous = newStart;
        for(int i = 0; i < model->rowCount(); i++)
        {
            if(previous <= newEnd){
                model->setData(model->index(i), (this->*localeFunc)(previous));
            }
            else{
                break;
            }
            previous += interval;
        }
    }
}

void HbDateTimePickerPrivate::createPrimitives()
{
    Q_Q(HbDateTimePicker);
    if(!mBackground) {
        mBackground = q->style()->createPrimitive(HbStyle::P_DateTimePicker_background,q);
        q->style()->setItemName(mBackground,"background");
    }
    if(!mFrame) {
        mFrame = q->style()->createPrimitive(HbStyle::P_DateTimePicker_frame,q);
        q->style()->setItemName(mFrame,"frame");
    }

    if(!mHighlight){
        mHighlight = q->style()->createPrimitive(HbStyle::P_TumbleView_highlight,q);
        q->style()->setItemName(mHighlight,"highlight");
    }
}

void HbDateTimePickerPrivate::_q_dayChanged(int index)
{
#ifdef HBDATETIMEPICKER_DEBUG
    qDebug() << "_q_dayChanged:" << index;
#endif
    QDate newDate(mDateTime.date().year(),mDateTime.date().month(),index+mDayOffset);
    if(newDate.isValid()) {
        mDateTime.setDate(newDate);
#ifdef HBDATETIMEPICKER_DEBUG
        qDebug() << "dayChange:currentDate:" << mDateTime;
#endif
    }
    else {
#ifdef HBDATETIMEPICKER_DEBUG
        qDebug() << "got invalid day change:" << index;
#endif
        return;
    }

    emitDateChange();

    Q_UNUSED(index);
}

void HbDateTimePickerPrivate::_q_monthChanged(int index)
{
#ifdef HBDATETIMEPICKER_DEBUG
    qDebug() << "_q_monthChanged:" << index;
#endif
    QDate newDate(mDateTime.date());
    if(mMonthOffset >= 0) {
#ifdef HBDATETIMEPICKER_DEBUG
        qDebug() << "month: before:" << newDate << " ,off-ind:" << newDate.month()-mMonthOffset << "-" << index;
#endif
        newDate = newDate.addMonths(index+mMonthOffset-newDate.month());
#ifdef HBDATETIMEPICKER_DEBUG
        qDebug() << "month: after:" << newDate;
#endif
    }else {
#ifdef HBDATETIMEPICKER_DEBUG
        qDebug() << "month: before else:" << newDate;
#endif
        newDate = newDate.addMonths(index);
#ifdef HBDATETIMEPICKER_DEBUG
        qDebug() << "month: after else:" << newDate;
#endif
    }
    if(newDate.isValid()) {
        mDateTime.setDate(newDate);
#ifdef HBDATETIMEPICKER_DEBUG
        qDebug() << "monthChange:currentDate:" << mDateTime << " ind:" << index;
#endif
    }
    else {
#ifdef HBDATETIMEPICKER_DEBUG
        qDebug() << "got invalid month change:" << index;
        qDebug() << "got invalid month change:" << mMonthOffset;
#endif
        return;
    }

    //check if day range changed
    if(mDayPicker) {
        int start=mDayOffset;
        int end=mDayOffset+mDayModel->rowCount()-1;
        if(isMinimumYear() && isMinimumMonth()) {
            start = mMinimumDate.date().day();
        } else {
            start = 1;
        }
        if(isMaximumYear() && isMaximumMonth()) {
            end = mMaximumDate.date().day();
        } else {
            end = mDateTime.date().daysInMonth();
        }

        //set if dayrange changed
        if((start != mDayOffset)
                ||(end !=mDayOffset+mDayModel->rowCount()-1)) {
            setDayRange(start,end);
        }
    }

    emitDateChange();
}

void HbDateTimePickerPrivate::_q_yearChanged(int index)
{
    qDebug() << "_q_yearChanged:" << index;
    //Q_Q(HbDateTimePicker);
    QDate newDate(mDateTime.date());
    if(mIsTwoDigitYearFormat) {
        newDate = newDate.addYears(index+mYearOffset-(newDate.year()%100));
    }
    else {
        newDate = newDate.addYears(index+mYearOffset-newDate.year());
    }
    if(newDate.isValid()) {
#ifdef HBDATETIMEPICKER_DEBUG
        qDebug() << "yearChange:currentDate before:" << mDateTime << " ind:" << index << " yeOff:" << mYearOffset;
#endif
        mDateTime.setDate(newDate);
#ifdef HBDATETIMEPICKER_DEBUG
        qDebug() << "yearChange:currentDate:" << mDateTime << " ind:" << index;
#endif
    }
    else {
#ifdef HBDATETIMEPICKER_DEBUG
        qDebug() << "got invalid month change:" << index;
        qDebug() << "got invalid month change:" << mMonthOffset;
#endif
        return;
    }

    //check if month range changed
    int start=0,end=0;
    if(mMonthPicker) {
        start=mMonthOffset;
        end=start+mMonthModel->rowCount()-1;
        if(isMinimumYear())  {
            start = mMinimumDate.date().month();
        } else {
            start = 1;
        }

        if(isMaximumYear()) {
            end = mMaximumDate.date().month();
        } else {
            end = 12;
        }

        //set if range changed
        if((start != mMonthOffset)
                || (end != mMonthModel->rowCount()-1)) {
            setMonthRange(start,end);
        }

    }
    
    //check if day range changed
    if(mDayPicker) {
        int start=mDayOffset;
        int end=mDayOffset+mDayModel->rowCount()-1;
        if(isMinimumYear() && isMinimumMonth()) {
            start = mMinimumDate.date().day();
        } else {
            start = 1;
        }
        if(isMaximumYear() && isMaximumMonth()) {
            end = mMaximumDate.date().day();
        } else {
            end = mDateTime.date().daysInMonth();
        }

        //set if dayrange changed
        if((start != mDayOffset)
                ||(end !=mDayOffset+mDayModel->rowCount()-1)) {
            setDayRange(start,end);
        }
    }

    emitDateChange();

}

void HbDateTimePickerPrivate::_q_hoursChanged(int index)
{
#ifdef HBDATETIMEPICKER_DEBUG
    qDebug() << "_q_hoursChanged:" << index;
#endif
    QTime newTime(mHourOffset+index,mDateTime.time().minute(),mDateTime.time().second());
    if(newTime.isValid()) {
        mDateTime.setTime(newTime);
    }
    else {
#ifdef HBDATETIMEPICKER_DEBUG
        qDebug() << "got invalid hour change:" << index;
        qDebug() << "got invalid hour change offset:" << mHourOffset;
#endif
        return;
    }

    //check if minute range changed
    int start=0,end=0;
    if(mMinutePicker) {
        start=mMinuteOffset;
        end=start+mMinuteModel->rowCount()-1;
        if(isMinimumHour())  {
            start = mMinimumDate.time().minute();
        } else {
            start = 0;
        }
        if(isMaximumHour()) {
            end = mMaximumDate.time().minute();
        } else {
            end = 59;
        }

        //set if range changed
        if((start != mMinuteOffset)
                || (end != start+mMinuteModel->rowCount()-1)) {
            setMinuteRange(start,end);
        }

    }
    
    //check if seconds range changed
    if(mSecondPicker) {
        start=mSecondOffset;
        end=mSecondOffset+mSecondModel->rowCount()-1;
        if(isMinimumHour() && isMinimumMinute()) {
            start = mMinimumDate.time().second();
        } else {
            start = 0;
        }
        if(isMaximumHour() && isMaximumMinute()) {
            end = mMaximumDate.time().second();
        } else {
            end = 59;
        }

        //set if seconds range changed
        if((start != mSecondOffset)
                ||(end !=mSecondOffset+mSecondModel->rowCount()-1)) {
            setSecondRange(start,end);
        }
    }

    //check if am or pm and scroll to respective time
    if((!mIs24HourFormat) && mAmPmPicker) {
        if(mAmPmPicker->selected() == 0) {
            //is AM
            if(mDateTime.time().hour() > 11) {
                mAmPmPicker->setSelected(1);
            }
        } else if(mAmPmPicker->selected() == 1) {
            //is PM
            if(mDateTime.time().hour() < 12) {
                mAmPmPicker->setSelected(0);
            }
        }
    }

    emitTimeChange();
}

void HbDateTimePickerPrivate::_q_minutesChanged(int index)
{
    bool *bOk = false;
#ifdef HBDATETIMEPICKER_DEBUG
    qDebug() << "_q_minutesChanged:" << index;
    qDebug() << mLocale.toInt(mMinuteModel->index(mMinuteOffset+index,0).data().toString(),bOk, 10);
#endif
	QTime newTime(mDateTime.time().hour(),mLocale.toInt(mMinuteModel->index(index,0).data().toString(),bOk, 10),mDateTime.time().second());
    if(newTime.isValid()) {
        mDateTime.setTime(newTime);
    }
    else {
#ifdef HBDATETIMEPICKER_DEBUG
        qDebug() << "got invalid minute change:" << index;
        qDebug() << "got invalid minute change offset:" << mMinuteOffset;
#endif
        return;
    }

    int start,end;
    //check if seconds range changed
    if(mSecondPicker) {
        start=mSecondOffset;
        end=mSecondOffset+mSecondModel->rowCount()-1;
        if(isMinimumHour() && isMinimumMinute()) {
            start = mMinimumDate.time().second();
        } else {
            start = 0;
        }
        if(isMaximumHour() && isMaximumMinute()) {
            end = mMaximumDate.time().second();
        } else {
            end = 59;
        }

        //set if seconds range changed
        if((start != mSecondOffset)
                ||(end !=mSecondOffset+mSecondModel->rowCount()-1)) {
            setSecondRange(start,end);
        }
    }

    emitTimeChange();
}

void HbDateTimePickerPrivate::_q_secondsChanged(int index)
{
#ifdef HBDATETIMEPICKER_DEBUG
    qDebug() << "_q_secondsChanged:" << index;
#endif
    QTime newTime(mDateTime.time().hour(),mDateTime.time().minute(),mSecondOffset+index);
    if(newTime.isValid()) {
        mDateTime.setTime(newTime);
    }
    else {
#ifdef HBDATETIMEPICKER_DEBUG
        qDebug() << "got invalid second change:" << index;
        qDebug() << "got invalid second change offset:" << mSecondOffset;
#endif
        return;
    }


    emitTimeChange();
}

void HbDateTimePickerPrivate::_q_ampmChanged(int index)
{
    
#ifdef HBDATETIMEPICKER_DEBUG
    qDebug() << "_q_ampmChanged:" << index;
#endif
    if(index == 0) {
        //AM is chosen 
        QTime newTime(mDateTime.time().hour()-12,mDateTime.time().minute(),mDateTime.time().second());
        if(newTime.isValid()) {
            if(newTime >= mMinimumDate.time()) {
                mDateTime.setTime(newTime);
                if(mHourPicker) {
                    mHourPicker->setSelected(newTime.hour()-mHourOffset);
                }
                emitTimeChange();
            }else {
                mAmPmPicker->setSelected(1);//invalid so scrollback
            }

        }  
    } else if(index == 1) {
        //PM is chosen
        QTime newTime(mDateTime.time().hour()+12,mDateTime.time().minute(),mDateTime.time().second());
        if(newTime.isValid()) {
            if(newTime <= mMaximumDate.time()) {
                mDateTime.setTime(newTime);
                if(mHourPicker) {
                    mHourPicker->setSelected(newTime.hour()-mHourOffset);
                } 
                emitTimeChange();
            } else { 
                mAmPmPicker->setSelected(0);//invalid so scrollback
            }
        } 
    }
}
