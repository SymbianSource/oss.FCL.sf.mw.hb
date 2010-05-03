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

#include "hbprogressslidercontrol_p.h"
#include "hbslidercontrol_p.h"
#include "hbslidercontrol_p_p.h"
#include "hbglobal_p.h"
//#include "hbstyleoptionslider.h"


class HbProgressSliderControlPrivate : public HbSliderControlPrivate
{
	Q_DECLARE_PUBLIC(HbProgressSliderControl)

	public:
		HbProgressSliderControlPrivate();
		~HbProgressSliderControlPrivate();

        QGraphicsItem *createGroove();
};

/*!
    private class  
*/
HbProgressSliderControlPrivate::HbProgressSliderControlPrivate() :
	HbSliderControlPrivate()
{
}

/*!
   
*/
HbProgressSliderControlPrivate::~HbProgressSliderControlPrivate()
{

}

QGraphicsItem *HbProgressSliderControlPrivate::createGroove()
{
    return NULL;
}

/*!
    \this is a deprecated class.
    \class HbProgressSliderControl
*/
/*!
    \deprecated HbProgressSliderControl::HbProgressSliderControl(QGraphicsItem *)
        is deprecated.

    Constructor for HbProgressSliderControl.
*/
HbProgressSliderControl::HbProgressSliderControl(QGraphicsItem *parent)
	:HbSliderControl(*new HbProgressSliderControlPrivate,parent)
{
    HB_DEPRECATED("HbProgressSliderControl::HbProgressSliderControl(QGraphicsItem *) is deprecated as part of HbProgressSliderControl class deprecation.");
	Q_D(HbProgressSliderControl);
	d->q_ptr = this;
	d->init();
}

/*!
    \deprecated HbProgressSliderControl::HbProgressSliderControl(Qt::Orientation, QGraphicsItem *)
        is deprecated.

    Constructor for HbProgressSliderControl.
*/
HbProgressSliderControl::HbProgressSliderControl(Qt::Orientation orientation, QGraphicsItem *parent)
	:HbSliderControl(*new HbProgressSliderControlPrivate,parent)
{
    HB_DEPRECATED("HbProgressSliderControl::HbProgressSliderControl(Qt::Orientation,QGraphicsItem *) is deprecated as part of HbProgressSliderControl class deprecation.");
	Q_D(HbProgressSliderControl);
	d->q_ptr = this;
	d->init();      
    setOrientation(orientation);
}

HbProgressSliderControl::~HbProgressSliderControl()
{
}

