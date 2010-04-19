/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
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

#ifndef HBABSTRACTSLIDERCONTROL_P_H
#define HBABSTRACTSLIDERCONTROL_P_H

#include "hbabstractslidercontrol.h"
#include "hbwidget_p.h"
#include <QStyle>
#include <QBasicTimer>
#include <QApplication>

class HbAbstractSliderControlPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbAbstractSliderControl)

public:
    HbAbstractSliderControlPrivate();
    ~HbAbstractSliderControlPrivate();

    void setSteps(int single, int page);
    inline int bound(int val) const { return qMax(minimum, qMin(maximum, val)); }

    inline int overflowSafeAdd(int add) const
    {
        int newValue = value + add;
        if (add > 0 && newValue < value)
            newValue = maximum;
        else if (add < 0 && newValue > value)
            newValue = minimum;
        return newValue;
    }

    inline void adjustSliderPosition(int step)
    {
        Q_Q(HbAbstractSliderControl);
        // NOTE: the widget parameter is not used by
        // QStyle::styleHint(QStyle::SH_Slider_StopMouseOverSlider)
		if (qApp->style()->styleHint(QStyle::SH_Slider_StopMouseOverSlider, 0, 0)) {
            if (qAbs(position - pressValue) < step) {
                repeatAction = HbAbstractSliderControl::SliderNoAction;
                q->setSliderPosition(pressValue);
                return;
            }
        }
        q->triggerAction(repeatAction);
    }
    //data member
    int minimum;
    int maximum;
    int singleStep;
    int pageStep;
    int value;
    int position;
    int pressValue;
    uint tracking : 1;
    uint blocktracking :1;
    uint pressed : 1;
    uint invertedAppearance : 1;
    uint invertedControls : 1;
    Qt::Orientation orientation;

    QBasicTimer repeatActionTimer;
    int repeatActionTime;
    HbAbstractSliderControl::SliderAction repeatAction;

    int origValue;

};

#endif // HBABSTRACTSLIDERCONTROL_P_H
