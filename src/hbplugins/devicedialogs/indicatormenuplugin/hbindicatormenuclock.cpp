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
#include <QTime>
#include <hbtextitem.h>

#include "hbindicatormenuclock_p.h"

void HbIndicatorMenuClock::localeInfo(LocaleInfo *info)
{
    QString timeFormat(QLocale().timeFormat(QLocale::LongFormat));
#if defined(Q_OS_SYMBIAN)
    //use extendedlocale
    HbExtendedLocale loc;
    info->useSpace = loc.amPmSpace();
    info->symbolPos = loc.amPmSymbolPosition();
    
    //qt returns wrong timeformat in QLocale().timeFormat(), fix it using hbextendedlocale
    //todo: remove these, when it works
    timeFormat.clear();
    QChar sep(loc.timeSeparator(0));
    if (!sep.isNull()) {
        timeFormat.append(sep);
    }
    timeFormat.append('h').append(loc.timeSeparator(1))
              .append("mm").append(loc.timeSeparator(2))
              .append("ss");
    sep = loc.timeSeparator(3);
    if (!sep.isNull()) {
        timeFormat.append(sep);
    }
    
    if (loc.timeStyle() == HbExtendedLocale::Time12) {
        timeFormat.append(" ap");
    }
    //todo: remove end
#endif
    QString strippedTimeFormat(timeFormat); //removes am/pm-symbol from format string.
    strippedTimeFormat.remove(QChar('a'), Qt::CaseInsensitive)
                      .remove(QChar('p'), Qt::CaseInsensitive);

    QString trimmedTimeFormat(timeFormat.trimmed());
    int index = trimmedTimeFormat.indexOf(QChar('A'));
    if (index >= 0) {
        info->amPmFormat = "AP";
        info->useAmPmSymbol = true;
    } else {
        index = trimmedTimeFormat.indexOf(QChar('a'));
        if (index >= 0) {
            info->amPmFormat = "ap";
            info->useAmPmSymbol = true;
        }
    }
#if !defined(Q_OS_SYMBIAN)
    if (index == 0) {
        info->symbolPos = HbExtendedLocale::Before;

        //am/pm removed from strippedTimeFormat, only space is left at(0).
        info->useSpace = strippedTimeFormat.at(0).isSpace();
    } else {
        info->symbolPos = HbExtendedLocale::After;
        index--; //index to point to where space should be.
        if (index >= 0 && index < strippedTimeFormat.size()) {
            info->useSpace = strippedTimeFormat.at(index).isSpace();
        }
    }
#endif
    if (info->useAmPmSymbol && info->useSpace) {
        if (info->symbolPos == HbExtendedLocale::Before) {
            info->amPmFormat.append(" ");
        } else {
            info->amPmFormat.prepend(" ");
        }
    }
    info->timeFormat = strippedTimeFormat.trimmed() + info->amPmFormat;
}

HbIndicatorMenuClock::HbIndicatorMenuClock(QGraphicsItem *parent) :
        HbWidget(parent), mTickTimerId(0),
        mTimeItem(0), mAmPmSymbol(0)
{
    mTimeItem = new HbTextItem(this);
    HbStyle::setItemName(mTimeItem, "time");
    mTimeItem->setFontSpec(HbFontSpec(HbFontSpec::Primary));
    mTimeItem->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mTimeItem->setTextWrapping( Hb::TextNoWrap );

    localeInfo(&mInfo);
    if (mInfo.useAmPmSymbol) {
        mAmPmSymbol = new HbTextItem(this);
        mAmPmSymbol->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        mTimeItem->setTextWrapping(Hb::TextNoWrap);
        mAmPmSymbol->setFontSpec(HbFontSpec(HbFontSpec::Primary));
        HbStyle::setItemName(mAmPmSymbol, "ampmSymbol");
    }
}

HbIndicatorMenuClock::~HbIndicatorMenuClock()
{
    if (mTickTimerId != 0) {
        killTimer(mTickTimerId);
        mTickTimerId = 0;
    }
}

void HbIndicatorMenuClock::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mTickTimerId) {
        updateClock();
    }
}

void HbIndicatorMenuClock::updateClock()
{
    QTime current = QTime::currentTime();
    QString amPmSymbolText(current.toString(mInfo.amPmFormat));
    if (mAmPmSymbol) {
        mAmPmSymbol->setText(amPmSymbolText);
    }

    QString timeText(current.toString(mInfo.timeFormat));
    timeText = timeText.left(timeText.size()- amPmSymbolText.size());
    mTimeItem->setText(timeText);

    QDate date = QDate::currentDate();
    if (mDate != date) {
        mDate = date;
        emit dateChanged();
    }
}

void HbIndicatorMenuClock::polish(HbStyleParameters& params)
{
    setProperty("amPmSymbolPos", (int) mInfo.symbolPos);
    HbWidget::polish(params);
}

void HbIndicatorMenuClock::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    updateClock();
    // Start a timer.
    if (mTickTimerId == 0) {
        mTickTimerId = startTimer(1000);
    }
}

void HbIndicatorMenuClock::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)
    if (mTickTimerId != 0) {
        killTimer(mTickTimerId);
        mTickTimerId = 0;
    }
}

