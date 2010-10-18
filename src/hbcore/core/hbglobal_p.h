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

#ifndef HBGLOBAL_P_H
#define HBGLOBAL_P_H

#include <hbglobal.h>

#ifdef Q_OS_SYMBIAN
#define __HB_UHEAP_MARK __UHEAP_MARK
#else
#define __HB_UHEAP_MARK
#endif

#ifdef Q_OS_SYMBIAN
#define __HB_UHEAP_MARKEND __UHEAP_MARKEND
#else
#define __HB_UHEAP_MARKEND
#endif


// Macros for aiding deprecation
struct HbRecallFlag {
    HbRecallFlag(): oldValue(flag) {}
    ~HbRecallFlag() {
        flag = oldValue;
    }
    bool oldValue;
    HB_CORE_EXPORT static bool flag;
};

// Use this macro to display a warning msg when deprecated method is used.
// This macro ensures that if a deprecated method calls another deprecated
// method only one warning msg is shown.
#define HB_DEPRECATED(msg)\
    HbRecallFlag recallFlag;\
    if (HbRecallFlag::flag){\
        HbRecallFlag::flag = false;\
        static bool localFlag = true;\
        if(localFlag) {\
            qWarning(msg);\
            localFlag = false;\
        }\
    }

// Turns off the deprecation msg
#define HB_DEPRECATED_MSG_OFF HbRecallFlag recallFlag; HbRecallFlag::flag = false;

// Enable the following to make Device Dialog Server's window surface permanently allocated.
//#define HB_DDS_KEEPS_SURFACE

#endif // HBGLOBAL_P_H
