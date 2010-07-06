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

#include <hbglobal.h>
#include "hbglobal_p.h"
#include "hbfeaturemanager_r.h"
#include <QString>
#include <QCoreApplication>

/*!
    @stable
    @hbcore
    \class HbGlobal
    \brief HbGlobal, Hb framework global definitions
*/

/*!
    \macro HB_VERSION
    \relates <HbGlobal>

    This macro expands a numeric value of the form 0xMMNNPP (MM =
    major, NN = minor, PP = patch) that specifies Hb's version
    number. For example, if you compile your application against Hb
    1.2.3, the HB_VERSION macro will expand to 0x010203.

    You can use HB_VERSION to use the latest Hb features where
    available.

    \sa hbVersion(), HB_VERSION_STR
*/

/*!
    \macro HB_VERSION_STR
    \relates <HbGlobal>

    This macro expands to a string that specifies Hb's version number
    (for example, "1.2.3"). This is the version against which the
    application is compiled.

    \sa hbVersionString(), HB_VERSION
*/

/*!
    \macro HB_CORE_EXPORT
    \relates <HbGlobal>

    Defined as Q_DECL_EXPORT when building HbCore
    and as Q_DECL_IMPORT when using HbCore.
 */

/*!
    \macro HB_WIDGETS_EXPORT
    \relates <HbGlobal>

    Defined as Q_DECL_EXPORT when building HbWidgets
    and as Q_DECL_IMPORT when using HbWidgets.
 */

/*!
   \macro HB_AUTOTEST_EXPORT
   \relates <HbGlobal>

   Used for internal exports for testing.
*/

/*!
    \relates <HbGlobal>

    Returns the version number of Hb at run-time (for example, 0x010203).
    This may be a different version than the version the application was
    compiled against.

    \sa HB_VERSION, hbVersionString()
*/
uint hbVersion()
{
    return HB_VERSION;
}

/*!
    \relates <HbGlobal>

    Returns the version number of Hb at run-time as a string (for
    example, "1.2.3"). This may be a different version than the
    version the application was compiled against.

    \sa HB_VERSION_STR, hbVersion()
*/
const char *hbVersionString()
{
    return HB_VERSION_STR;
}

/*!
    Returns the translation text from QM file.
    
    \param id Text ID identifier for translation. Example: txt_common_button_back
    \param n Defines numeric argument in case of plural strings.
    Note! As second parameter is only for plural strings in normal cases you shouldn't use it.
    For non-plural strings use QString::arg() function.
    Example: QString text = hbTrId("txt_with_value").arg(value);
    
    \return Translation if operation was successful, otherwise given \a id.

    \sa QCoreApplication::translate, QString::arg
*/
QString hbTrId(const char *id, int n)
{
    QString loc = qtTrId(id, n);
#ifdef HB_TEXT_MEASUREMENT_UTILITY
    if (HbFeatureManager::instance()->featureStatus(HbFeatureManager::TextMeasurement)) {
        loc.append(QChar(LOC_TEST_START));
        loc.append(id);
        loc.append(QChar(LOC_TEST_END));
    }
#endif
    return loc;
}

bool HbRecallFlag::flag = true;

