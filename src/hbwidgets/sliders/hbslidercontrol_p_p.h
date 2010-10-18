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

#ifndef HBSLIDERCONTROL_P_P_H
#define HBSLIDERCONTROL_P_P_H

#include "hbslidercontrol_p.h"
#include "hbabstractslidercontrol_p.h"
#include "hbslidertickmarks_p.h"

#include <QString>

class HbSliderHandle;
class HbToolTip;
class QGraphicsItem;
class HbSliderTickmarksLabel;
class HbTouchArea;

class HbSliderControlPrivate : public HbAbstractSliderControlPrivate
{
    Q_DECLARE_PUBLIC(HbSliderControl)

public:
    HbSliderControlPrivate();
    virtual ~HbSliderControlPrivate();

    void init();
    void adjustHandle();
    bool onHandle(QPointF pos);
    int getNearbyTick();
    virtual HbSliderHandle *createHandle();
    virtual QGraphicsItem *createGroove();
    virtual QGraphicsItem *createProgressGroove();
    int calculateSliderPosition( QPointF position );

   
    HbSliderHandle *handle;
    Hb::SliderTickPositions tickPosition;
    HbSliderControl::SnappingMode  snappingMode;
    int majorTickInterval;
    int minorTickInterval;
    QGraphicsItem *groove;
    QGraphicsItem *progressGroove;

 
    bool displayCurrValueToolTip;
    QPointF oldPos;
    Qt::Alignment toolTipAlignment;
    QString handleIcon;
    bool groovePressed;
    // These two variable are volume slider specific
    // in volume slider if volume is mute then on track press 
    // slider should go to previousValue
    bool setDefault;
    int previousValue;
    bool trackHandlingEnable;
    bool handleMoving;
    QStringList majorLabel;
    QStringList minorLabel;
    QGraphicsItem *grooveTouchArea;
    bool enableProgressTrack;
    bool userDefinedTooltipAlign;
    HbTouchArea *tooltipArea;
    qreal tooltipBorderHeight;

private:
    // not to pollute HbSliderControl (which may become public in the future)
    // with friend class declarations
    static HbSliderControlPrivate *d_ptr(HbSliderControl *sliderControl) {
        Q_ASSERT(sliderControl);
        return sliderControl->d_func();
    }
    friend class HbSlider;

};
#endif // HBSLIDERCONTROL_P_P_H

