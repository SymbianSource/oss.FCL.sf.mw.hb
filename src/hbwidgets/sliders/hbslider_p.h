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

#ifndef HBSLIDER_P_H
#define HBSLIDER_P_H


#include "hbwidget_p.h"
#include <QHash>

class HbSliderControl;
class QGraphicsItem;

struct ItemPrimitive
{
    QGraphicsItem* item;
    QGraphicsItem *touchItem;
    HbStyle::Primitive type;
};

class HbSliderPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbSlider)

public:
    HbSliderPrivate();
    virtual ~HbSliderPrivate();
    void init();
    void setElements( QList<HbSlider::SliderElement> elementList);
    void elementWidget(HbSlider::SliderElement element);
    void updateElements();
    void startIncrementing();
    void startDecrementing();
    void stopRepeatAction();
    void setTickLabelPresentProperty();

#ifdef HB_EFFECTS
    void _q_startIconPressedEffect();
    void _q_startIconReleasedEffect();
    void _q_startTextClickEffect();
#endif

    //Data member

    HbSliderControl *sliderControl;
    Qt::Orientation orientation;
    QList<HbSlider::SliderElement> elements;
    QString sliderTextString;
    QHash<HbSlider::SliderElement,HbIcon> icons;
    QString thumbPath;   
    bool  pressOnIncrement;
    QMap<HbSlider::SliderElement , ItemPrimitive> elementItemMap;

};

#endif // HBSLIDER_P_H
