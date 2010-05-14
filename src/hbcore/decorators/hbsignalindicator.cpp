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

#include <hbinstance.h>

#include "hbsignalindicator_p.h"
#include "hbsignalindicator_p_p.h"
#include "hbstyleoptionsignalindicator_p.h"

/*
    \class HbSignalIndicator
    \brief HbSignalIndicator represents a signal indicator item.

    The signal indicator shows approximately the signal strength of the mobile network.
    It is created and managed by the HbStatusBar which in turn is part of the HbMainWindow.
*/

// 0-33% for low, 34-66% for medium and 67-100% for high
static const int signalThreshold[] = {33, 66, 100};

HbSignalIndicatorPrivate::HbSignalIndicatorPrivate() :
    mLevelPercent(-1),
    mSignalBackgroundIcon(0),
    mSignalLevelIcon(0),
    mSignalIcon(0)
#ifdef HB_HAVE_QT_MOBILITY
    ,mSystemNetworkInfo(new HbSystemInfo(0, false))
    ,mNetworkMode(QSystemNetworkInfo::UnknownMode)
#endif // HB_HAVE_QT_MOBILITY
{
}

HbSignalIndicatorPrivate::~HbSignalIndicatorPrivate()
{
#ifdef HB_HAVE_QT_MOBILITY
    delete mSystemNetworkInfo;
#endif // HB_HAVE_QT_MOBILITY
}

#ifdef HB_HAVE_QT_MOBILITY
void HbSignalIndicatorPrivate::_q_setNetworkSignalStrength(QSystemNetworkInfo::NetworkMode mode, int strength)
{
    Q_Q(HbSignalIndicator);
    if (mode != mNetworkMode) {
        mNetworkMode = mode;
        _q_setNetworkMode(mNetworkMode);
    }
    q->setLevel(strength);
}

void HbSignalIndicatorPrivate::_q_setNetworkMode(QSystemNetworkInfo::NetworkMode mode)
{
    Q_Q(HbSignalIndicator);
    mNetworkMode = mode;
    q->updatePrimitives();
}
#endif // HB_HAVE_QT_MOBILITY

// ======== MEMBER FUNCTIONS ========

/*
    Constructs a signal indicator with \a parent.
*/
HbSignalIndicator::HbSignalIndicator(QGraphicsItem *parent)
    : HbWidget(*new HbSignalIndicatorPrivate, parent)
{
    createPrimitives();
    updatePrimitives();
#ifdef HB_HAVE_QT_MOBILITY
    Q_D(HbSignalIndicator);
    connect(d->mSystemNetworkInfo, SIGNAL(networkSignalStrengthChanged(QSystemNetworkInfo::NetworkMode, int)), 
        this, SLOT(_q_setNetworkSignalStrength(QSystemNetworkInfo::NetworkMode, int)));
    connect(d->mSystemNetworkInfo, SIGNAL(networkModeChanged(QSystemNetworkInfo::NetworkMode)), 
        this, SLOT(_q_setNetworkMode(QSystemNetworkInfo::NetworkMode)));
#endif // HB_HAVE_QT_MOBILITY
}

/*
    Destructor
 */
HbSignalIndicator::~HbSignalIndicator()
{

}

void HbSignalIndicator::delayedConstruction()
{
}

/*
    set signal strength level. \a level value should be >= HB_SIGNAL_STRENGTH_LEVELS_MIN
    and <= HB_SIGNAL_STRENGTH_LEVELS_MAX. \a level value is ignored if out of range.
*/
void HbSignalIndicator::setLevel(int levelPercent)
{
    Q_D(HbSignalIndicator);
    if (levelPercent < 0 || levelPercent > 100) {
        return;
    }

    if (d->mLevelPercent != levelPercent) {
        d->mLevelPercent = levelPercent;
        updatePrimitives();
    }
}

int HbSignalIndicator::level() const
{ 
     Q_D(const HbSignalIndicator);
    return d->mLevelPercent;
 }

void HbSignalIndicator::createPrimitives()
{
    Q_D(HbSignalIndicator);
    d->mSignalIcon = style()->createPrimitive(HbStyle::P_SignalIndicator_icon, this);
    d->mSignalBackgroundIcon = style()->createPrimitive(HbStyle::P_SignalLevel_background, this);
    d->mSignalLevelIcon = style()->createPrimitive(HbStyle::P_SignalLevel_icon, this);
}

void HbSignalIndicator::updatePrimitives()
{
    Q_D(HbSignalIndicator);
    HbStyleOptionSignalIndicator option;
    initStyleOption(&option);
    style()->updatePrimitive(d->mSignalIcon, HbStyle::P_SignalIndicator_icon, &option);
    style()->updatePrimitive(d->mSignalBackgroundIcon, HbStyle::P_SignalLevel_background, &option);
    style()->updatePrimitive(d->mSignalLevelIcon, HbStyle::P_SignalLevel_icon, &option);
}

void HbSignalIndicator::initStyleOption(HbStyleOptionSignalIndicator *option) const
{
    HbWidget::initStyleOption(option);

    Q_D(const HbSignalIndicator);

    // style option should default to unknown mode if not set
#ifdef HB_HAVE_QT_MOBILITY    
    option->networkMode = d->mNetworkMode;
#endif // HB_HAVE_QT_MOBILITY    

    //signal level setting
    if (d->mLevelPercent >= 0 && d->mLevelPercent <= signalThreshold[0]) { // low
        option->signalLevel = HbStyleOptionSignalIndicator::Low;
    } else if (d->mLevelPercent >= signalThreshold[0] &&
               d->mLevelPercent <= signalThreshold[1]) { // medium
        option->signalLevel = HbStyleOptionSignalIndicator::Medium;
    } else if (d->mLevelPercent >= signalThreshold[1] &&
               d->mLevelPercent <= signalThreshold[2]) { // high
        option->signalLevel = HbStyleOptionSignalIndicator::Full;
    } else {
        option->signalLevel = HbStyleOptionSignalIndicator::Zero;
    }
    option->signalValue = d->mLevelPercent;
}

#include "moc_hbsignalindicator_p.cpp"
