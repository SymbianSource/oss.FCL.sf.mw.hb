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

/*!
    \class HbIndicator
    \brief HbIndicator can be used to activate and deactivate indicators.

    HbIndicator sends a request for indicator activation and deactivation to
    server side. Indicators are identified by their type-string and there must be a
    server side indicator implementation for that type.

    Active indicators may appear in the status indicator area as an icon,
    and/or inside universal indicator popup showing icon and text.
    Depending on the indicator implementation, activated indicator may also show up with
    a discreet popup and some indicators can be interacted by the user in universal indicator popup.

    When deactivated, icons are removed from the status
    indicator area and in universal indicator popup.

    User can interact with indicator from the indicator menu. Client is notified about
    the user interaction via userActivated signal. Interaction notification and data 
    sent by the indicator is a contract between HbIndicator class and indicator.
    
    \sa HbIndicatorPluginInterface

    \stable
    \hbcore
*/

/*!
    \enum HbIndicator::ErrorRange
    Defines indicator error ranges.
*/
/*!
    \var HbIndicator::ErrorRange HbIndicator::FrameworkErrors
    Start of an error range for errors originating from device dialog framework (client or server).
    Error codes are defined in hbdevicedialogerrors.h
*/
/*!
    \var HbIndicator::ErrorRange HbIndicator::PluginErrors
    Start of an error range for errors originating from indicator plugins.
*/
/*!
    \var HbIndicator::ErrorRange HbIndicator::ErrorTypeMask
    Mask for error type part of the error code.
*/

/*!
    \fn void HbIndicator::userActivated(const QString &type, const QVariantMap &data)

    The class should emit this signal, when client needs to be notified of the
    user interaction.
    @param type Type of the indicator that user interacted with.
    @param data Data sent by indicator.
*/

#include "hbindicator.h"

#include <QtGlobal>
#include <QDataStream>

// indicators are implemented only for Symbian/S60 OS. All others use a stub which shows
// indicators in the calling process.
#if defined(Q_OS_SYMBIAN)
#include "hbindicatorsym_p.h"
#else
#include "hbindicatorwin32_p.h"
#endif // defined(Q_OS_SYMBIAN)

/*!
    Constructs an indicator object. \a parent is a pointer to the parent.
*/
HbIndicator::HbIndicator(QObject *parent) :
    QObject(parent), d_ptr(new HbIndicatorPrivate)
{
    Q_D(HbIndicator);
    d->q_ptr = this;
    d->init();
}

HbIndicator::HbIndicator(HbIndicatorPrivate &dd, QObject *parent) :
    QObject(parent), d_ptr(&dd)
{
    Q_D(HbIndicator);
    d->q_ptr = this;
    d->init();
}

HbIndicator::~HbIndicator()
{
    delete d_ptr;
}

/*!
    Activates an indicator of type \a indicatorType.
    An extra parameter can be passed along to the indicator plugin.
    Returns true, if the indicator is activated, false, if an error occurred.    

    \sa deactivate
 */
bool HbIndicator::activate(const QString &indicatorType, const QVariant &parameter)
{
    return d_func()->activate(indicatorType, parameter);
}

/*!
    Deactivates an indicator of type \a indicatorType.
    An extra parameter can be passed along to the indicator plugin.
    If indicator is not currently active, does nothing and returns true.
    Returns false, if error occurred.

    \sa activate
 */
bool HbIndicator::deactivate(const QString &indicatorType, const QVariant &parameter)
{
    return d_func()->deactivate(indicatorType, parameter);
}

/*!
    Returns the last error code. The code is cleared when any other API function than error() is called.
*/
int HbIndicator::error() const
{
    return d_func()->error();
}

