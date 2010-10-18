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
    \brief HbIndicator can be used to activate, update and deactivate indicators.

    Indicators are used to indicate events to user. For example unread mail, missed calls, etc. 
    They appear as icon in status bar and/or indicator menu. In addition of an icon, indicator menu
    may also contains text for an indicator. From the menu, user can interact with an indicator
    by tapping it. 

    Indicators are implemented by plugins loaded by indicator service. Indicator implementations
    in plugins are identified by unique strings.

    Activating indicator adds it into status bar and/or menu. Deactivating removes it.

    Activating an already active indicator updates it with a new data set. The plugin implementation
    is called the same way as first time activate is called.  

    HbIndicator indicates data from the plugin implementation by HbIndicator::userActivated() signal.
    The pugin may trigger this signal for example when user interacts with the indicator from
    indicator menu. Data accompanying the signal originates from indicator plugin and is
    passed by indicator framework unmodified. All instances of HbIndicator that have activated
    the indicator, receive the signal.

    The code below activates and deactivates an indicator.
    \code
    static const char indicatorType[] = "com.nokia.hb.unittestfirstindicator0/1.0";
    // Activate with an integer parameter value 0
    mIndicator.activate(indicatorType, QVariant(0));
    mIndicator.deactivate(indicatorType);
    \endcode

    \sa HbIndicatorPluginInterface, HbIndicatorInterface

    \stable
    \hbcore
*/

/*!
    \enum HbIndicator::ErrorRange
    Defines indicator error ranges.
*/
/*!
    \var HbIndicator::ErrorRange HbIndicator::FrameworkErrors
    Start of an error range for errors originating from indicator framework (client or server).
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

    Signal indicates data from indicator plugin implentation. Indicator plugin may originate the signal
    for example when user interacts with the indicator from indicator menu.

    @param type Indicator type (identification).
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
    Constructs HbIndicator with \a parent.
*/
HbIndicator::HbIndicator(QObject *parent) :
    QObject(parent), d_ptr(new HbIndicatorPrivate)
{
    Q_D(HbIndicator);
    d->q_ptr = this;
    d->init();
}

/*!
    Constructs HbIndicator. Allows class derivation by shared d-pointer paradigm.
*/
HbIndicator::HbIndicator(HbIndicatorPrivate &dd, QObject *parent) :
    QObject(parent), d_ptr(&dd)
{
    Q_D(HbIndicator);
    d->q_ptr = this;
    d->init();
}

/*!
    Destructs HbIndicator.
*/
HbIndicator::~HbIndicator()
{
    delete d_ptr;
}

/*!
    Activates an indicator. If indicator was already active, updates it.

    \param indicatorType Indicator to activate.
    \param parameter Parameter to pass into indicator plugin implementation.

    \return Returns true if the indicator was activated, false if an error occurred.
    
    \sa deactivate()
 */
bool HbIndicator::activate(const QString &indicatorType, const QVariant &parameter)
{
    return d_func()->activate(indicatorType, parameter);
}

/*!
    Deactivates an indicator.

    \param indicatorType Indicator to deactivate.
    \param parameter Parameter to pass into indicator plugin implementation.

    \return Returns true if the indicator was deactivated or wasn't active,
    false if an error occurred.

    \sa activate()
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

