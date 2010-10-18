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

// HB_VERSION_STR="M.N.P-B" (M=major, N=minor, P=patch, B=build=[dev|tag])
#define HB_VERSION_STR "0.5.0-2010wk41"
// HB_VERSION=0xMMNNPP (MM=major, NN=minor, PP=patch)
#define HB_VERSION 0x000500

#ifndef HB_DOXYGEN

#define HB_DECL_DEPRECATED Q_DECL_DEPRECATED
#define HB_DECL_VARIABLE_DEPRECATED Q_DECL_VARIABLE_DEPRECATED
#define HB_DECL_CONSTRUCTOR_DEPRECATED Q_DECL_CONSTRUCTOR_DEPRECATED

#if defined(HB_STATIC) || defined(HB_BOOTSTRAPPED)

#  define HB_CORE_EXPORT
#  define HB_CORE_RESTRICTED_EXPORT
#  define HB_CORE_PRIVATE_EXPORT
#  define HB_WIDGETS_EXPORT
#  define HB_WIDGETS_RESTRICTED_EXPORT
#  define HB_WIDGETS_PRIVATE_EXPORT
#  define HB_UTILS_EXPORT
#  define HB_UTILS_RESTRICTED_EXPORT
#  define HB_UTILS_PRIVATE_EXPORT
#  define HB_INPUT_EXPORT
#  define HB_INPUT_RESTRICTED_EXPORT
#  define HB_INPUT_PRIVATE_EXPORT
#  define HB_FEEDBACK_EXPORT
#  define HB_FEEDBACK_RESTRICTED_EXPORT
#  define HB_FEEDBACK_PRIVATE_EXPORT
#  define HB_AUTOTEST_EXPORT

# else

#  ifdef BUILD_HB_CORE
#    define HB_CORE_EXPORT Q_DECL_EXPORT
#    define HB_CORE_RESTRICTED_EXPORT Q_DECL_EXPORT
#    define HB_CORE_PRIVATE_EXPORT Q_DECL_EXPORT
#  else
#    define HB_CORE_EXPORT Q_DECL_IMPORT
#    define HB_CORE_RESTRICTED_EXPORT Q_DECL_IMPORT
#    define HB_CORE_PRIVATE_EXPORT Q_DECL_IMPORT
#  endif // BUILD_HB_CORE

#  ifdef BUILD_HB_WIDGETS
#    define HB_WIDGETS_EXPORT Q_DECL_EXPORT
#    define HB_WIDGETS_RESTRICTED_EXPORT Q_DECL_EXPORT
#    define HB_WIDGETS_PRIVATE_EXPORT Q_DECL_EXPORT
#  else
#    define HB_WIDGETS_EXPORT Q_DECL_IMPORT
#    define HB_WIDGETS_RESTRICTED_EXPORT Q_DECL_IMPORT
#    define HB_WIDGETS_PRIVATE_EXPORT Q_DECL_IMPORT
#  endif // BUILD_HB_WIDGETS

#  ifdef BUILD_HB_UTILS
#    define HB_UTILS_EXPORT Q_DECL_EXPORT
#    define HB_UTILS_RESTRICTED_EXPORT Q_DECL_EXPORT
#    define HB_UTILS_PRIVATE_EXPORT Q_DECL_EXPORT
#  else
#    define HB_UTILS_EXPORT Q_DECL_IMPORT
#    define HB_UTILS_RESTRICTED_EXPORT Q_DECL_IMPORT
#    define HB_UTILS_PRIVATE_EXPORT Q_DECL_IMPORT
#  endif // BUILD_HB_UTILS

#  ifdef BUILD_HB_INPUT
#    define HB_INPUT_EXPORT Q_DECL_EXPORT
#    define HB_INPUT_RESTRICTED_EXPORT Q_DECL_EXPORT
#    define HB_INPUT_PRIVATE_EXPORT Q_DECL_EXPORT
#  else
#    define HB_INPUT_EXPORT Q_DECL_IMPORT
#    define HB_INPUT_RESTRICTED_EXPORT Q_DECL_IMPORT
#    define HB_INPUT_PRIVATE_EXPORT Q_DECL_IMPORT
#  endif // BUILD_HB_INPUT

#  ifdef BUILD_HB_FEEDBACK
#    define HB_FEEDBACK_EXPORT Q_DECL_EXPORT
#    define HB_FEEDBACK_RESTRICTED_EXPORT Q_DECL_EXPORT
#    define HB_FEEDBACK_PRIVATE_EXPORT Q_DECL_EXPORT
#  else
#    define HB_FEEDBACK_EXPORT Q_DECL_IMPORT
#    define HB_FEEDBACK_RESTRICTED_EXPORT Q_DECL_IMPORT
#    define HB_FEEDBACK_PRIVATE_EXPORT Q_DECL_IMPORT
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

#endif // !HB_STATIC && !HB_BOOTSTRAPPED

/*
Flag enables text measurements for localization.
*/
#ifdef HB_TEXT_MEASUREMENT_UTILITY

#define LOC_TEST_START 0x91
#define LOC_TEST_END 0x92

#endif // HB_TEXT_MEASUREMENT_UTILITY

#if defined(HB_NO_DEBUG_OUTPUT) || (!defined(HB_DEBUG_OUTPUT) && defined(Q_CC_RVCT) && defined(QT_NO_DEBUG))
#define hbDebug QT_NO_QDEBUG_MACRO
#else
#define hbDebug qDebug
#endif

#if defined(HB_NO_WARNING_OUTPUT) || (!defined(HB_WARNING_OUTPUT) && defined(Q_CC_RVCT) && defined(QT_NO_DEBUG))
#define hbWarning QT_NO_QWARNING_MACRO
#else
#define hbWarning qWarning
#endif


#ifndef HB_ALWAYS_INLINE
#if defined(Q_CC_GNU) || defined(Q_CC_RVCT) // GCC & RVCT both support forced inlining
#define HB_ALWAYS_INLINE __attribute__((always_inline))
#else
#define HB_ALWAYS_INLINE
#endif
#endif

#endif // HB_DOXYGEN

HB_CORE_EXPORT uint hbVersion();
HB_CORE_EXPORT const char *hbVersionString();
HB_CORE_EXPORT QString hbTrId(const char *id, int n = -1);

#endif // HBGLOBAL_H
