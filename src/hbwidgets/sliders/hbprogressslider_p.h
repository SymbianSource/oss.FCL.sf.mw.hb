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

#ifndef HBPROGRESSSLIDER_P_H
#define HBPROGRESSSLIDER_P_H

#include "hbprogressbar_p.h"
#include "hbprogresssliderhandle_p.h"

class HB_AUTOTEST_EXPORT HbProgressSliderPrivate :public HbProgressBarPrivate,public HbHandleParent
{
    Q_DECLARE_PUBLIC( HbProgressSlider )
public:
    HbProgressSliderPrivate();
    ~HbProgressSliderPrivate();
    void init();
    int pointToValue(QPointF pt);
    HbProgressSliderHandle *handle;

    //inherited from HbHandleParent
    void emitSliderPressed();
    void emitSliderReleased();
    void setProgressValue(int value);
    void emitSliderMoved(int newValue);
	   void setEnableFlag(bool flag);
    QRectF boundingRect() const;
    HbStyle* style() const;
    QGraphicsItem* parentGraphicsItem() const;
    HbWidget* parentGraphicsWidget() const;
    int progressValue()const;
    int maximum() const ;
    int minimum() const ;
    bool invertedAppearance() const ;
    QString toolTipText() const;
    Qt::Alignment textAlignment() const;
    bool textVisible()const{ return (mTooltipText=="")? false:true ;};

    Qt::Orientation orientation();
    void setRange(int minimum, int maximum);
private:
    QGraphicsItem *mSliderGraphicItem;
    int mSliderValue;
    bool mDownState;
    QString mTooltipText;
    QString mHandlePath;
};

#endif //HBPROGRESSSLIDER_P_H

