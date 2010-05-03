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

#ifndef HBSIGNALINDICATOR_H
#define HBSIGNALINDICATOR_H

#include "hbnamespace_p.h"

#include <hbwidget.h>
#include <hbnamespace.h>

class HbSignalIndicatorPrivate;
class HbStyleOptionSignalIndicator;

class HB_CORE_PRIVATE_EXPORT HbSignalIndicator : public HbWidget
{
    Q_OBJECT

public:
    enum { Type = HbPrivate::ItemType_SignalIndicator };

    explicit HbSignalIndicator(QGraphicsItem *parent = 0);
    virtual ~HbSignalIndicator();

    void delayedConstruction();

    inline int type() const { return Type; }
    void setLevel(int levelPercent);
    int level() const;

public slots:
    virtual void createPrimitives();
    virtual void updatePrimitives();

protected:
    void initStyleOption(HbStyleOptionSignalIndicator *option) const;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbSignalIndicator)
    Q_DISABLE_COPY(HbSignalIndicator)

    Q_PRIVATE_SLOT(d_func(), void _q_setNetworkSignalStrength(HbSystemNetworkInfo::NetworkMode, int))
    Q_PRIVATE_SLOT(d_func(), void _q_setNetworkMode(HbSystemNetworkInfo::NetworkMode))
};

#endif // HBSIGNALINDICATOR_H
