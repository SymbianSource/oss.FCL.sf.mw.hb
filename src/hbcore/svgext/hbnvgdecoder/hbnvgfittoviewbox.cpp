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

#include "hbnvgfittoviewbox_p.h"

const qreal Zero = 0.0f ;
const qreal One = 1.0f ;

HbNvgFitToViewBoxImpl::HbNvgFitToViewBoxImpl()
		: mM00(One),
        mM01(Zero),
        mM02(Zero),
        mM10(Zero),
        mM11(One),
        mM12(Zero),
        mViewBoxDefined(false),
        mAlign(HbNvgEngine::NvgPreserveAspectRatioXmidYmid),
        mMeetSlice(HbNvgEngine::NvgMeet)
{
    mVbX    = 0.0;
    mVbY    = 0.0;
    mVbW   = 0.0;
    mVbH    = 0.0;

}

HbNvgFitToViewBoxImpl::~HbNvgFitToViewBoxImpl()
{
}

void HbNvgFitToViewBoxImpl::setWindowViewportTrans(const QRect &viewPort, const QSize &size)
{

    //VIEWPORT NUMBERS
    qreal lViewPortX = viewPort.left();
    qreal lViewPortY = viewPort.top();
    qreal lViewPortWidth = viewPort.width();
    qreal lViewPortHeight = viewPort.height();

    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgTranslate(lViewPortX, lViewPortY);

    qreal lViewBoxXmin;
    qreal lViewBoxYmin;
    qreal lViewBoxWidth;
    qreal lViewBoxHeight;

    if (mViewBoxDefined) {
        lViewBoxXmin = mVbX;
        lViewBoxYmin = mVbY;
        lViewBoxWidth = mVbW;
        lViewBoxHeight = mVbH;
    } else {
        //this will default viewBox to <svg> element width and height
        lViewBoxXmin = 0;
        lViewBoxYmin = 0;
        lViewBoxWidth = size.width();
        lViewBoxHeight = size.height();
    }

    if (lViewBoxWidth == 0.0f || lViewBoxHeight == 0.0f) {
        return;
    }

    qreal sx = lViewPortWidth / lViewBoxWidth;
    qreal sy = lViewPortHeight / lViewBoxHeight;

    if (sx == 0.0f || sy == 0.0f) {
        return;
    }

    qreal xtrans = qreal(-1.0f) * lViewBoxXmin;
    qreal ytrans = qreal(-1.0f) * lViewBoxYmin;

    switch (mAlign) {
    case HbNvgEngine::NvgPreserveAspectRatioNone:
        /* Non uniform scaling */
        //none - Do not force uniform scaling.
        //Scale the graphic content of the given element
        //non-uniformly if necessary such that the element's
        //bounding box exactly matches the viewport rectangle.
        break;
    case HbNvgEngine::NvgPreserveAspectRatioXminYmin:
        //Align the <min-x> of the element's viewBox with the smallest X value of the viewport.
        //Align the <min-y> of the element's viewBox with the smallest Y value of the viewport.

        if (mMeetSlice == HbNvgEngine::NvgMeet) {
            if (sx > sy) {
                sx = sy;
                //no change for xtrans...default above
            } else { // (sx < sy)
                sy = sx;
                //no change for ytrans...default above
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
            } else { // (sx < sy)
                sx = sy;
            }
        }
        break;
    case HbNvgEngine::NvgPreserveAspectRatioXmidYmin:
        //Align the midpoint X value of the element's viewBox with the midpoint X value of the viewport.
        //Align the <min-y> of the element's viewBox with the smallest Y value of the viewport.
        //Align the <min-x> of the element's viewBox with the smallest X value of the viewport.
        //Align the <min-y> of the element's viewBox with the smallest Y value of the viewport.

        if (mMeetSlice == HbNvgEngine::NvgMeet) {
            if (sx > sy) {
                sx = sy;
                xtrans = ((lViewPortWidth - ((lViewBoxWidth / lViewBoxHeight) * lViewPortHeight)) * (.5f)) / sx - lViewBoxXmin;
            } else { 
                sy = sx;
                //no change for ytrans...default above
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
            } else { 
                sx = sy;
                xtrans = lViewPortWidth - sx * lViewBoxWidth;
                xtrans = xtrans / sx;
                xtrans = xtrans / qreal(2) - lViewBoxXmin;
            }
        }
        break;
    case HbNvgEngine::NvgPreserveAspectRatioXmaxYmin:
        //Align the <min-x>+<width> of the element's viewBox with the maximum X value of the viewport.
        //Align the <min-y> of the element's viewBox with the smallest Y value of the viewport.
        if (mMeetSlice == HbNvgEngine::NvgMeet) {
            if (sx > sy) {
                sx = sy;
                xtrans = ((lViewPortWidth - ((lViewBoxWidth / lViewBoxHeight) * lViewPortHeight))) / sx - lViewBoxXmin;
            } else { 
                sy = sx;
                //no change for ytrans...default above
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
                //no change for ytrans...default above
            } else { 
                sx = sy;
                xtrans = lViewPortWidth - sx * lViewBoxWidth;
                xtrans = xtrans / sx - lViewBoxXmin;
            }
        }
        break;
    case HbNvgEngine::NvgPreserveAspectRatioXminYmid:
        //Align the <min-x> of the element's viewBox with the smallest X value of the viewport.
        //Align the midpoint Y value of the element's viewBox with the midpoint Y value of the viewport.
        if (mMeetSlice == HbNvgEngine::NvgMeet) {
            if (sx > sy) {
                sx = sy;
                //no change for xtrans...default above
            } else { 
                sy = sx;
                ytrans = ((qreal)
                          (lViewPortHeight - ((qreal)(lViewBoxHeight / lViewBoxWidth) * lViewPortWidth)) * qreal(.5f)) / sy - lViewBoxYmin;
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
                ytrans = lViewPortHeight - sx * lViewBoxHeight;
                ytrans = ytrans / sx;
                ytrans = ytrans / qreal(2) - lViewBoxYmin;
            } else { 
                sx = sy;
            }
        }
        break;
    case HbNvgEngine::NvgPreserveAspectRatioXmidYmid:
        //(default) case
        //Align the midpoint X value of the element's viewBox with the midpoint X value of the viewport.
        //Align the midpoint Y value of the element's viewBox with the midpoint Y value of the viewport.
        if (mMeetSlice == HbNvgEngine::NvgMeet) {
            if (sx > sy) {
                sx = sy;
                xtrans = ((lViewPortWidth - ((lViewBoxWidth / lViewBoxHeight) * lViewPortHeight)) * (.5f)) / sx - lViewBoxXmin;

            } else if (sx < sy) {
                sy = sx;
                ytrans = ((lViewPortHeight - ((lViewBoxHeight / lViewBoxWidth) * lViewPortWidth)) * (.5f)) / sy - lViewBoxYmin;
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
                ytrans = lViewPortHeight - sx * lViewBoxHeight;
                ytrans = ytrans / sx;
                ytrans = ytrans / qreal(2) - lViewBoxYmin;
            } else { 
                sx = sy;
                xtrans = lViewPortWidth - sx * lViewBoxWidth;
                xtrans = xtrans / sx;
                xtrans = xtrans / qreal(2) - lViewBoxXmin;
            }
        }
        break;
    case HbNvgEngine::NvgPreserveAspectRatioXmaxYmid:
        //Align the <min-x>+<width> of the element's viewBox with the maximum X value of the viewport.
        //Align the midpoint Y value of the element's viewBox with the midpoint Y value of the viewport.
        if (mMeetSlice == HbNvgEngine::NvgMeet) {
            if (sx > sy) {
                sx = sy;
                xtrans = ((lViewPortWidth - ((lViewBoxWidth / lViewBoxHeight) * lViewPortHeight))) / sx - lViewBoxXmin;
            } else { 
                sy = sx;
                ytrans = ((lViewPortHeight - ((lViewBoxHeight / lViewBoxWidth) * lViewPortWidth)) * (.5f)) / sy - lViewBoxYmin;
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
                ytrans = lViewPortHeight - sx * lViewBoxHeight;
                ytrans = ytrans / sx;
                ytrans = ytrans / qreal(2) - lViewBoxYmin;
            } else { 
                sx = sy;
                xtrans = lViewPortWidth - sx * lViewBoxWidth;
                xtrans = xtrans / sx - lViewBoxXmin;
            }
        }
        break;
    case HbNvgEngine::NvgPreserveAspectRatioXminYmax:
        //Align the <min-x> of the element's viewBox with the smallest X value of the viewport.
        //Align the <min-y>+<height> of the element's viewBox with the maximum Y value of the viewport.
        if (mMeetSlice == HbNvgEngine::NvgMeet) {
            if (sx > sy) {
                sx = sy;
                //no change for xtrans...default above
            } else { 
                sy = sx;

                ytrans = ((lViewPortHeight - ((lViewBoxHeight / lViewBoxWidth) * lViewPortWidth))) / sy - lViewBoxYmin;
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
                ytrans = lViewPortHeight - sx * lViewBoxHeight;
                ytrans = ytrans / sx - lViewBoxYmin;
            } else {
                sx = sy;
            }
        }
        break;
    case HbNvgEngine::NvgPreserveAspectRatioXmidYmax:
        //Align the midpoint X value of the element's viewBox with the midpoint X value of the viewport.
        //Align the <min-y>+<height> of the element's viewBox with the maximum Y value of the viewport.
        if (mMeetSlice == HbNvgEngine::NvgMeet) {
            if (sx > sy) {
                sx = sy;
                xtrans = ((lViewPortWidth - ((lViewBoxWidth / lViewBoxHeight) * lViewPortHeight)) * qreal(.5f)) / sx - lViewBoxXmin;
            } else {
                sy = sx;
                ytrans = ((lViewPortHeight - ((lViewBoxHeight / lViewBoxWidth) * lViewPortWidth))) / sy - lViewBoxYmin;
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
                ytrans = lViewPortHeight - sx * lViewBoxHeight;
                ytrans = ytrans / sx - lViewBoxYmin;
            } else {
                sx = sy;
                xtrans = lViewPortWidth - sx * lViewBoxWidth;
                xtrans = xtrans / sx;
                xtrans = xtrans / qreal(2) - lViewBoxXmin;
            }
        }
        break;
    case HbNvgEngine::NvgPreserveAspectRatioXmaxYmax:
        //Align the <min-x>+<width> of the element's viewBox with the maximum X value of the viewport.
        //Align the <min-y>+<height> of the element's viewBox with the maximum Y value of the viewport.
        if (mMeetSlice == HbNvgEngine::NvgMeet) {
            if (sx > sy) {
                sx = sy;
                xtrans = ((lViewPortWidth - ((lViewBoxWidth / lViewBoxHeight) * lViewPortHeight))) / sx - lViewBoxXmin;
            } else {
                sy = sx;
                ytrans = ((lViewPortHeight - ((lViewBoxHeight / lViewBoxWidth) * lViewPortWidth))) / sy - lViewBoxYmin;
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
                ytrans = lViewPortHeight - sx * lViewBoxHeight;
                ytrans = ytrans / sx - lViewBoxYmin;
            } else {
                sx = sy;
                xtrans = lViewPortWidth - sx * lViewBoxWidth;
                xtrans = xtrans / sx - lViewBoxXmin;
            }
        }
        break;
    default:
        break;
    }

    vgScale(sx, sy);
    vgTranslate(xtrans, ytrans);
}

void HbNvgFitToViewBoxImpl::concatenate(qreal m00, qreal m01, qreal m02, qreal m10, qreal m11, qreal m12)
{
    qreal m0, m1;
    m0  = mM00;
    m1  = mM01;
    mM00 = m00 * m0 + m10 * m1;
    mM01 = m01 * m0 + m11 * m1;
    mM02 += m02 * m0 + m12 * m1;
    m0 = mM10;
    m1 = mM11;
    mM11 = m01 * m0 + m11 * m1;
    mM10 = m00 * m0 + m10 * m1;
    mM12 += m02 * m0 + m12 * m1;
}

//--------------------------------EndOfFile------------------------------------
