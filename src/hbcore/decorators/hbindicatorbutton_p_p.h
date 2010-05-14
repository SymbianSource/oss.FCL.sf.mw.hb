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

#ifndef HBINDICATORBUTTON_P_P_H
#define HBINDICATORBUTTON_P_P_H

#include <hbtoolbutton_p.h>
#include "hbindicatorbutton_p.h"

class HbIndicatorButtonPrivate : public HbToolButtonPrivate
{
    Q_DECLARE_PUBLIC(HbIndicatorButton)

public:
    HbIndicatorButtonPrivate();
    virtual ~HbIndicatorButtonPrivate();

    void init();
    void showIndicatorMenu();

    void addIndicators(const QList<IndicatorClientInfo> &clientInfo);
    void removeIndicators(const QList<IndicatorClientInfo> &clientInfo);
    int findIndicator(const IndicatorClientInfo &indicator) const;
    void updateIcon();

private:

    QGraphicsItem *handleIcon;
    HbAction *defaultAction;
    HbAction *newEventAction;

    HbDeviceDialog *deviceDialog;

    QList<IndicatorClientInfo> mIndicators;
};

#endif // HBINDICATORBUTTON_P_P_H

