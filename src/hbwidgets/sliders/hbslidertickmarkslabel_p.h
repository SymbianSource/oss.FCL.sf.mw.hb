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

#ifndef HBSLIDERTICKMARKSLABEL_P_H
#define HBSLIDERTICKMARKSLABEL_P_H

#include "hbnamespace_p.h"
#include <hbwidget.h>
#include <hbnamespace.h>
#include "hbstyleoptionslider_p.h"

class QStyleOption;
class HbSliderTickmarksLabelPrivate;

class HB_AUTOTEST_EXPORT HbSliderTickmarksLabel : public HbWidget
{
    Q_OBJECT

public:
    explicit HbSliderTickmarksLabel( QGraphicsItem *parent = 0 );
    ~HbSliderTickmarksLabel();

   /* virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, 
        QWidget *widget );*/
    virtual void resizeEvent(QGraphicsSceneResizeEvent *event);

    enum {Type = HbPrivate::ItemType_SliderTickmarksLabel };
    int type() const {return Type;}

    void updateTickLabels( );
    void setTickPosition(Hb::SliderTickPositions position);
    void setLabelSize();
    void createText(bool create);


protected:
    void polish( HbStyleParameters& params );
    void initStyleOption(HbStyleOptionSlider *option) const;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);    



private:
    enum SliderTickType {
        ST_SliderMajorTick = 1,
        ST_SliderMinorTick
    };

    Q_DECLARE_PRIVATE_D(d_ptr, HbSliderTickmarksLabel )
    Q_DISABLE_COPY( HbSliderTickmarksLabel )
};

#endif // HBSLIDERTICKMARKSITEM_P_H
