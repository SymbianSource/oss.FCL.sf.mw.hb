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


#ifndef HBPROGRESSBARPRIVATE_H
#define HBPROGRESSBARPRIVATE_H

#include "hbwidget_p.h"
#include "hbprogressbar.h"

#ifdef HB_EFFECTS
#include "hbeffect.h"
#endif

class QGraphicsItem;

class HbProgressBarPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC( HbProgressBar )
public:    
    HbProgressBarPrivate();
    ~HbProgressBarPrivate();

    void init();
    void createTextPrimitives();
    virtual void setRange(int minimum, int maximum);
    virtual void setOrientation(Qt::Orientation orientation);
	virtual void setProgressValue(int value);
	virtual void setEnableFlag(bool flag);

#ifdef HB_EFFECTS
    void _q_delayedHide(HbEffect::EffectStatus status);
    void _q_delayedShow(HbEffect::EffectStatus status);
#endif

public:
    QGraphicsItem *mFrame;
    QGraphicsItem *mTrack;
    QGraphicsItem *mWaitTrack;
    QGraphicsItem *mMinTextItem;
    QGraphicsItem *mMaxTextItem;
    QGraphicsItem *mTouchAreaItem;


    uint mMinMaxTextVisible : 1;
    int mMaximum;
    int mMinimum;
    int mProgressValue;
    uint mInvertedAppearance : 1;
    uint mTextVisible : 1;
    Qt::Alignment mMinMaxTextAlignment;

    QString mMinText;
    QString mMaxText;
    Qt::Orientation mOrientation;

    uint mDelayHideInProgress : 1;
    uint mShowEffectInProgress : 1;
};

#endif  //HBPROGRESSBARPRIVATE_H

