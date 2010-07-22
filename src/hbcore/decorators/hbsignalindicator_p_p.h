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

#ifndef HBSIGNALINDICATOR_P_P_H
#define HBSIGNALINDICATOR_P_P_H

#include <hbwidget_p.h>
#include "hbsignalindicator_p.h"

#ifdef HB_HAVE_QT_MOBILITY
#include "hbsysteminfo_p.h"
#endif // HB_HAVE_QT_MOBILITY

class HbSignalIndicatorPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbSignalIndicator)

public:
    HbSignalIndicatorPrivate();
    virtual ~HbSignalIndicatorPrivate();

#ifdef HB_HAVE_QT_MOBILITY
    void _q_setNetworkSignalStrength(QSystemNetworkInfo::NetworkMode mode, int strength);
    void _q_setNetworkMode(QSystemNetworkInfo::NetworkMode mode, QSystemNetworkInfo::NetworkStatus status);
#endif // HB_HAVE_QT_MOBILITY

private:
    int mLevelPercent;
    Qt::AspectRatioMode mAspectRatioMode;
    QGraphicsItem *mSignalBackgroundIcon;
    QGraphicsItem *mSignalLevelIcon;
    QGraphicsItem *mSignalIcon;

#ifdef HB_HAVE_QT_MOBILITY
    HbSystemInfo *mSystemNetworkInfo;
    QSystemNetworkInfo::NetworkMode mNetworkMode;
    QSystemNetworkInfo::NetworkStatus mNetworkStatus;
#endif // HB_HAVE_QT_MOBILITY
};

#endif // HBSIGNALINDICATOR_P_P_H

