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

#ifndef HBBATTERYINDICATOR_H
#define HBBATTERYINDICATOR_H

#include <QBasicTimer>

#include "hbnamespace_p.h"

#include <hbwidget.h>
#include <hbnamespace.h>

class HbBatteryIndicatorPrivate;
class HbStyleOptionBatteryIndicator;

class HB_CORE_PRIVATE_EXPORT HbBatteryIndicator : public HbWidget
{
    Q_OBJECT

public:
    enum { Type = HbPrivate::ItemType_BatteryIndicator };

    explicit HbBatteryIndicator( QGraphicsItem *parent = 0 );
    virtual ~HbBatteryIndicator();

    void delayedConstruction();

    inline int type() const { return Type; }
    int level() const;
    bool isCharging() const;
    void chargingEvent(bool start);

public slots:
    virtual void createPrimitives();
    virtual void updatePrimitives();

    void setLevel(int levelPercent);

signals:
    void levelChanged();

protected:
    void timerEvent(QTimerEvent *event);
    void initStyleOption(HbStyleOptionBatteryIndicator *option) const;
    bool event(QEvent *event);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbBatteryIndicator)
    Q_DISABLE_COPY(HbBatteryIndicator)

#ifdef HB_HAVE_QT_MOBILITY
    Q_PRIVATE_SLOT(d_func(), void _q_setPowerState(QSystemDeviceInfo::PowerState))
#endif // HB_HAVE_QT_MOBILITY
};

#endif // HBBATTERYINDICATOR_H
