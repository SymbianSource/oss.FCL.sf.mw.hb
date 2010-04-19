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
#include "hbfeaturemanager_p.h"
#include <QString>
#include <QCoreApplication>

/*!
    @stable
    @hbcore
    \class HbGlobal
    \brief HbGlobal, Hb framework global definitions
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
    Returns the translation text.
    \sa QCoreApplication::translate
*/
QString hbTrId(const char *id, int n)
{
    QString loc = qtTrId(id, n);
#ifdef HB_TEXT_MEASUREMENT_UTILITY
    if ( HbFeatureManager::instance()->featureStatus( HbFeatureManager::TextMeasurement ) ) {
        loc.append(QChar(LOC_TEST_START));
        loc.append(id);
        loc.append(QChar(LOC_TEST_END));
    }
#endif
    return loc;
}

bool HbRecallFlag::flag = true;

