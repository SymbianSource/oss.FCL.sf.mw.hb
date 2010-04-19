/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
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

#ifndef HBEXTENDEDLOCALE_H
#define HBEXTENDEDLOCALE_H

#include <QString>
#include <QLocale>
#include <qobjectdefs.h>
#include <hbi18ndef.h>

class HB_CORE_EXPORT HbExtendedLocale : public QLocale
{
public:
    enum DateStyle {
        American,
        European,
        Japanese
    };

    enum TimeStyle {
        Time12,
        Time24
    };

    enum SymbolPos {
        Before,
        After
    };

    enum NegativeCurrencyFormat {
        LeadingMinusSign,
        InBrackets,
        TrailingMinusSign,
        InterveningMinusSign
    };

    enum WeekDay {
        Monday,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday,
        Sunday
    };
    
    enum DaylightSavingZone {
        HomeZone,
        EuropeanZone,
        NorthernZone,
        SouthernZone,
        None
    };
    

public: 
    QChar dateSeparator( int index ) const;
    bool setDateSeparator( const QChar ch, const int index );
    QChar timeSeparator( int index ) const;
    bool setTimeSeparator( const QChar ch, const int index );

    DateStyle dateStyle() const;
    bool setDateStyle( const DateStyle style );
    TimeStyle timeStyle() const;
    bool setTimeStyle( const TimeStyle style );

    bool amPmSpace() const;
    bool setAmPmSpace( const bool space );
    SymbolPos amPmSymbolPosition() const;
    bool setAmPmSymbolPosition( const SymbolPos position );

    QLocale::MeasurementSystem unitsDistanceLong() const;
    bool setUnitsDistanceLong( const QLocale::MeasurementSystem format );
    QLocale::MeasurementSystem unitsDistanceShort() const;
    bool setUnitsDistanceShort( const QLocale::MeasurementSystem format );
    bool setUnitsGeneral( const QLocale::MeasurementSystem format );

    NegativeCurrencyFormat negativeCurrencyFormat() const;
    bool setNegativeCurrencyFormat( const NegativeCurrencyFormat format );
    bool negativeCurrencySpace() const;
    bool setNegativeCurrencySpace( const bool space );
    bool negativeCurrencySymbolOpposite() const;
    bool setNegativeCurrencySymbolOpposite( const bool opposite );

    bool currencyTriadsAllowed() const;
    bool setCurrencyTriadsAllowed( const bool allowed );
    bool currencySpace() const;
    bool setCurrencySpace( const bool space );
    QString currencySymbol() const;
    bool setCurrencySymbol( const QString &symbol );
    SymbolPos currencySymbolPosition() const;
    bool setCurrencySymbolPosition( const SymbolPos position );
    int currencyDecimalPlaces() const;
    bool setCurrencyDecimalPlaces( const int places ); 

    QString formatCurrency( const qint64 amount );

    bool setDecimalPoint( const QChar ch );
    bool setGroupSeparator( const QChar ch );
    bool setZeroDigit( const DigitType type );

    static QString symbianLangToISO( const int code );
    
    QString symbianDateTimeToQt( const QString &sys_fmt );

    WeekDay startOfWeek() const;
    bool setStartOfWeek(WeekDay day);
    
    QString workDays() const;
    bool setWorkDays( const QString &days );

    bool homeHasDaylightSavingOn() const;
    DaylightSavingZone homeDaylightSavingZone() const;
    int universalTimeOffset() const;
    
    HbExtendedLocale();
    static HbExtendedLocale system();

    QString format( const QDate &date, const QString &dateFormat );
    QString format( const QTime &time, const QString &timeFormat );
};

#endif
