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

    ~HbNvgFitToViewBoxImpl();

public:
    void setViewBox(qreal vbX, qreal vbY, qreal vbW, qreal vbH)
    {
        mVbX = vbX;
        mVbY = vbY;
        mVbW = vbW;
        mVbH = vbH;
        mViewBoxDefined = true;
    }


    void setAllignment(HbNvgEngine::HbNvgAlignType alignStatus)
    {
        mAlign = alignStatus;
    }

    void setScaling(HbNvgEngine::HbNvgMeetType meetSlice)
    {
        mMeetSlice = meetSlice;
    }

    void setTransform(qreal m00, qreal m01, qreal m02, qreal m10, qreal m11, qreal m12)
    {
        mM00 = m00;
        mM01 = m01;
        mM02 = m02;
        mM10 = m10;
        mM11 = m11;
        mM12 = m12;
    }

    void translate(qreal tx, qreal ty)
    {
        qreal lTranslateMatrix[6] =  { 1, 0, tx, 0, 1, ty};
        concatenate(lTranslateMatrix);
    }

    void scale(qreal sx, qreal sy)
    {
        qreal lScaleMatrix[6] = { sx, 0, 0, 0, sy, 0};
        concatenate(lScaleMatrix);
    }

    void concatenate(qreal *matrix)
    {
        concatenate(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5]);
    }

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

    HbNvgEngine::HbNvgAlignType mAlign;

    HbNvgEngine::HbNvgMeetType  mMeetSlice;
};

#endif

