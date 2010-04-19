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
#ifndef HB_NVGFITTOVIEWBOX_P_H
#define HB_NVGFITTOVIEWBOX_P_H


#include <QtGlobal>
#include <QRect>
#include <QSize>
#include <VG/openvg.h>
#include "hbnvg_p.h"

class HbNvgFitToViewBoxImpl
{
public:
    HbNvgFitToViewBoxImpl();

    virtual ~HbNvgFitToViewBoxImpl();

public:
    inline void setViewBox(qreal vbX, qreal vbY, qreal vbW, qreal vbH);

    inline void setAlign(HbNvgEngine::NvgAlignStatusType alignStatus);

    inline void setScaling(HbNvgEngine::NvgMeetOrSliceType  meetSlice);

    inline void setTransform(qreal m00, qreal m01, qreal m02, qreal m10, qreal m11, qreal m12);


    inline void translate(qreal xValue, qreal yValue);

    inline void scale(qreal xValue, qreal yValue);
    
    inline void concatenate(qreal *matrix);

    void setWindowViewportTrans(const QRect &viewPort, const QSize &size);

    void concatenate(qreal m00, qreal m01, qreal m02, qreal m10, qreal m11, qreal m12);

private:
    
    qreal mM00;
    qreal mM01;
    qreal mM02;
    qreal mM10;
    qreal mM11;
    qreal mM12;

    qreal   mVbX;
    qreal   mVbY;
    qreal   mVbW;
    qreal   mVbH;

    bool mViewBoxDefined;

    HbNvgEngine::NvgAlignStatusType mAlign;

    HbNvgEngine::NvgMeetOrSliceType  mMeetSlice;
};

#include "hbnvgfittoviewbox_p.inl"
#endif

