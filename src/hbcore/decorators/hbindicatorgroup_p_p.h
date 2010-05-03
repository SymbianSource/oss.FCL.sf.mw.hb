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

#ifndef HBINDICATORGROUP_P_P_H
#define HBINDICATORGROUP_P_P_H

#include <hbwidget_p.h>
#include "hbindicatorgroup_p.h"

class HbIndicatorPrivate;

class HbIndicatorGroupPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbIndicatorGroup)

public:
    HbIndicatorGroupPrivate();
    virtual ~HbIndicatorGroupPrivate();

    void delayedConstruction();

    void init();
    int setIconName(HbStyleOptionIndicatorGroup &option, int index);

    void addIndicators(const QList<IndicatorClientInfo> &clientInfo);
    void updateIndicators(const QList<IndicatorClientInfo> &clientInfo);
    void removeIndicators(const QList<IndicatorClientInfo> &clientInfo);

    int findIndicator(const IndicatorClientInfo &indicator) const;
    void removeIndicator(const IndicatorClientInfo &indicator);

    bool canAddIndicator(const IndicatorClientInfo &indicator) const;

    void emitNotificationCount();

    void startAddingEffect();
    void startRemovingEffect();

    int findIndicatorIcon(const QString &iconPath) const;

private:
    HbIndicatorGroup::IndicatorType mIndicatorType;
    QList<QGraphicsItem*> mIcons;   
    HbIndicatorPrivate *mIndicatorPrivate;
    QList<IndicatorClientInfo> mIndicators;
    bool mProgressAdded;
    bool mIndicatorAdded;
    QList<QPointF> mOriginalPos;
    QStringList mRemovedIndicators;    
};

#endif // HBINDICATORGROUP_P_P_H

