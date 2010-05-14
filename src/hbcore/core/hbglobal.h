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

#ifndef HBGLOBAL_H
#define HBGLOBAL_H

#include <QtGlobal>

//#pragma hb_header(HbGlobal)

#define HB_EXPORT \
HB_EXPORT_macro_is_obsolete_Please_use_HB_YOURMODULE_EXPORT_instead \
{ &HB_EXPORT_macro_is_obsolete_Please_use_HB_YOURMODULE_EXPORT_instead; }; class

#define HB_DECL_DEPRECATED Q_DECL_DEPRECATED
#define HB_DECL_VARIABLE_DEPRECATED Q_DECL_VARIABLE_DEPRECATED
#define HB_DECL_CONSTRUCTOR_DEPRECATED Q_DECL_CONSTRUCTOR_DEPRECATED

#ifndef HB_BOOTSTRAPPED

#  ifdef BUILD_HB_CORE
#    define HB_CORE_EXPORT Q_DECL_EXPORT
#    define HB_CORE_PRIVATE_EXPORT Q_DECL_EXPORT
#  else
#    define HB_CORE_EXPORT Q_DECL_IMPORT
#    define HB_CORE_PRIVATE_EXPORT Q_DECL_IMPORT
#  endif // BUILD_HB_CORE

#  ifdef BUILD_HB_WIDGETS
#    define HB_WIDGETS_EXPORT Q_DECL_EXPORT
#    define HB_WIDGETS_PRIVATE_EXPORT Q_DECL_EXPORT
#  else
#    define HB_WIDGETS_EXPORT Q_DECL_IMPORT
#    define HB_WIDGETS_PRIVATE_EXPORT Q_DECL_IMPORT
#  endif // BUILD_HB_WIDGETS

#  ifdef BUILD_HB_TOOLS
#    define HB_TOOLS_EXPORT Q_DECL_EXPORT
#  else
#    define HB_TOOLS_EXPORT Q_DECL_IMPORT
#  endif // BUILD_HB_TOOLS

#  ifdef BUILD_HB_INPUT
#    define HB_INPUT_EXPORT Q_DECL_EXPORT
#    define HB_INPUT_PRIVATE_EXPORT Q_DECL_EXPORT
#  else
#    define HB_INPUT_EXPORT Q_DECL_IMPORT
#    define HB_INPUT_PRIVATE_EXPORT Q_DECL_IMPORT
#  endif // BUILD_HB_INPUT

#  ifdef BUILD_HB_FEEDBACK
#    define HB_FEEDBACK_EXPORT Q_DECL_EXPORT
#  else
#    define HB_FEEDBACK_EXPORT Q_DECL_IMPORT
#  endif // BUILD_HB_FEEDBACK

#  if defined(HB_DEVELOPER) || defined (Q_OS_SYMBIAN)
#    if defined(BUILD_HB_CORE) || defined(BUILD_HB_WIDGETS) || defined(BUILD_HB_FEEDBACK)
#      define HB_AUTOTEST_EXPORT Q_DECL_EXPORT
#    else
#      define HB_AUTOTEST_EXPORT Q_DECL_IMPORT
#    endif
#  else
#    define HB_AUTOTEST_EXPORT
#  endif

#else

#  define HB_CORE_EXPORT
#  define HB_CORE_PRIVATE_EXPORT
#  define HB_WIDGETS_EXPORT
#  define HB_WIDGETS_PRIVATE_EXPORT
#  define HB_TOOLS_EXPORT
#  define HB_INPUT_EXPORT
#  define HB_INPUT_PRIVATE_EXPORT
#  define HB_FEEDBACK_EXPORT
#  define HB_AUTOTEST_EXPORT

#endif // HB_BOOTSTRAPPED

/*
This flag enables QGraphicsEffect based effects that are available currently
only in kinetic development branch and will be released as a part of Qt 4.6. This
flag can be enabled with suitable environment to develop and test so called
filter-effects.
*/
//#define HB_FILTER_EFFECTS

/*
This flag enables interfaces for testing
TODO: Under flag for now, needs to be redesigned to use a more proper system(cenrep?)
to enable runtime loading once security etc... issues are clear
*/
#define HB_TESTABILITY

/*
This flag enables to turn Effect API off. Only for testing purpose.
*/
//#define HB_EFFECT_API_OFF

/*
Flag enables text measurements for localization.
*/
#ifdef HB_TEXT_MEASUREMENT_UTILITY

#define LOC_TEST_START 0x91
#define LOC_TEST_END 0x92

#endif // HB_TEXT_MEASUREMENT_UTILITY

HB_CORE_EXPORT QString hbTrId(const char *id, int n = -1);


#endif // HBGLOBAL_H
