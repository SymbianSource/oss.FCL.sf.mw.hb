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

#ifndef HBSLIDERTICKMARKS_P_H
#define HBSLIDERTICKMARKS_P_H

#include "hbnamespace_p.h"
#include <hbwidget.h>
#include <hbnamespace.h>

class QStyleOption;
class HbSliderTickmarksPrivate;

class HB_AUTOTEST_EXPORT HbSliderTickmarks : public HbWidget
{
    Q_OBJECT
public:
    explicit HbSliderTickmarks( QGraphicsItem *parent = 0 );
    ~HbSliderTickmarks();

   /* virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, 
        QWidget *widget );*/
    virtual void resizeEvent(QGraphicsSceneResizeEvent *event);

    enum {Type = HbPrivate::ItemType_SliderTickmarks };
    int type() const {return Type;}

    void createIcons(bool create);
    void updateTicks( );
    void setTickPosition(Hb::SliderTickPositions position);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);    


protected:
    void polish( HbStyleParameters& params );

private:
    enum SliderTickType {
        ST_SliderMajorTick = 1,
        ST_SliderMinorTick
    };

    Q_DECLARE_PRIVATE_D(d_ptr, HbSliderTickmarks )
    Q_DISABLE_COPY( HbSliderTickmarks )
};

#endif // HbSliderTickmarks_P_H
