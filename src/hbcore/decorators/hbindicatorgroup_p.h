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

#ifndef HBINDICATORGROUP_P_H
#define HBINDICATORGROUP_P_H

#include "hbnamespace_p.h"

#include <hbwidget.h>
#ifdef HB_EFFECTS
#include <hbeffect.h>
#endif // HB_EFFECTS

class HbIndicatorGroupPrivate;
class HbStyleOptionIndicatorGroup;
struct IndicatorClientInfo;

class HB_CORE_PRIVATE_EXPORT HbIndicatorGroup : public HbWidget
{
    Q_OBJECT

public:

    enum { Type = HbPrivate::ItemType_IndicatorGroup };
    int type() const { return Type; }

    enum IndicatorType { 
        NotificationType,
        SettingsType
    };

    explicit HbIndicatorGroup(IndicatorType indicatorType, QGraphicsItem *parent = 0);
    virtual ~HbIndicatorGroup();

    void delayedConstruction();

public slots:
    virtual void createPrimitives();
    virtual void updatePrimitives();

    void activate(const QList<IndicatorClientInfo> &clientInfo);
    void update(const QList<IndicatorClientInfo> &clientInfo);
    void activateAll(const QList<IndicatorClientInfo> &clientInfo);
    void deactivate(const QList<IndicatorClientInfo> &clientInfo);

protected:
    virtual void initStyleOption(HbStyleOptionIndicatorGroup *option) const;

private slots:
#ifdef HB_EFFECTS
    void moveEffectFinnished(const HbEffect::EffectStatus &status);
    void disappearEffectFinnished(const HbEffect::EffectStatus &status);
#endif // HB_EFFECTS

private:  
    Q_DECLARE_PRIVATE_D(d_ptr, HbIndicatorGroup)
    Q_DISABLE_COPY(HbIndicatorGroup)
};

#endif // HBINDICATORGROUP_P_H
