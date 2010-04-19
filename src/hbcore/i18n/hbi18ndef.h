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

#ifndef HB_I18N_DEF_H
#define HB_I18N_DEF_H

#include <QtGlobal>
#include <hbglobal.h>

enum DigitType {
    WesternDigit = 0x0030,
    ArabicIndicDigit = 0x0660,
    EasternArabicIndicDigit = 0x6F0,
    DevanagariDigit = 0x0966,
    ThaiDigit = 0x0E50
};

// 1:20, 1:20 am, 13:05, pm 1:05  // separators are locale dependent
#define r_qtn_time_usual "%-B%:0%*J%:1%T%:3%+B"

// 01:20, 1:20 am, 13:05, pm 1:05 // separators are locale dependent
#define r_qtn_time_usual_with_zero "%-B%:0%J%:1%T%:3%+B"

// 13:20:45,  1:20:45 pm, 1:02:05, 1:02:05 am  // separators are locale dependent
#define r_qtn_time_long "%-B%:0%*J%:1%T%:2%S%:3%+B"

// 13:20:45,  1:20:45 pm, 01:02:05, 1:02:05 am  // separators are locale dependent
#define r_qtn_time_long_with_zero "%-B%:0%J%:1%T%:2%S%:3%+B"

// 1.9.00, 00.9,1  // order and separators are locale dependent.
#define r_qtn_date_short "%*D%*M%*Y%/0%1%/1%2%/2%3%/3"

// 01.09.00, 00.09.01  // order and separators are locale dependent.
#define r_qtn_date_short_with_zero "%D%M%*Y%/0%1%/1%2%/2%3%/3"

// 1.9.2000, 2000.9,1  // order and separators are locale dependent.
#define r_qtn_date_usual "%*D%*M%Y%/0%1%/1%2%/2%3%/3"

// 01.09.2000, 2000.09.01  // order and separators are locale dependent.
#define r_qtn_date_usual_with_zero "%D%M%Y%/0%1%/1%2%/2%3%/3"
 
// 25.12  12/25  4.5  // order and separators are locale dependent.
#define r_qtn_date_without_year "%*D%*M%/0%4%/1%5"

// 25.12  12/25  04.05  // order and separators are locale dependent.
#define r_qtn_date_without_year_with_zero "%D%M%/0%4%/1%5"

// 13:15, 3:05 (hours and minutes, without seconds) // separators are locale dependent.
#define r_qtn_time_durat_short "%:0%*H%:1%T%:3"

// 13:15, 03:05 (hours and minutes, without seconds) // separators are locale dependent.
#define r_qtn_time_durat_short_with_zero "%:0%H%:1%T%:3"

// 13:55:23, 3:15:12  (hours, minutes and seconds) // separators are locale dependent
#define r_qtn_time_durat_long "%:0%*H%:1%T%:2%S%:3"

// 13:55:23, 03:15:12 (hours, minutes and seconds) // separators are locale dependent
#define r_qtn_time_durat_long_with_zero "%:0%H%:1%T%:2%S%:3"

// 13:15, 3:05 (minutes without leading zero and seconds) // separators are locale dependent.
#define r_qtn_time_durat_min_sec "%:0%*T%:2%S%:3"

// 13:15, 03:05 (minutes with leading zero and seconds) // separators are locale dependent.
#define r_qtn_time_durat_min_sec_with_zero "%:0%T%:2%S%:3"

#endif // HB_I18N_DEF_H
