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

HbNvgFitToViewBoxImpl::HbNvgFitToViewBoxImpl()
        :mViewBoxDefined(false),
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
    qreal viewPortX = viewPort.left();
    qreal viewPortY = viewPort.top();
    qreal viewPortWidth = viewPort.width();
    qreal viewPortHeight = viewPort.height();

    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgTranslate(viewPortX, viewPortY);

    qreal viewBoxXmin;
    qreal viewBoxYmin;
    qreal viewBoxWidth;
    qreal viewBoxHeight;

    if (mViewBoxDefined) {
        viewBoxXmin = mVbX;
        viewBoxYmin = mVbY;
        viewBoxWidth = mVbW;
        viewBoxHeight = mVbH;
    } else {
        //this will default viewBox to <svg> element width and height
        viewBoxXmin = 0;
        viewBoxYmin = 0;
        viewBoxWidth = size.width();
        viewBoxHeight = size.height();
    }

    if (viewBoxWidth == 0.0f || viewBoxHeight == 0.0f) {
        return;
    }

    qreal sx = viewPortWidth / viewBoxWidth;
    qreal sy = viewPortHeight / viewBoxHeight;

    if (sx == 0.0f || sy == 0.0f) {
        return;
    }

    qreal xtrans = qreal(-1.0f) * viewBoxXmin;
    qreal ytrans = qreal(-1.0f) * viewBoxYmin;

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
                xtrans = ((viewPortWidth - ((viewBoxWidth / viewBoxHeight) * viewPortHeight)) * (.5f)) / sx - viewBoxXmin;
            } else {
                sy = sx;
                //no change for ytrans...default above
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
            } else {
                sx = sy;
                xtrans = viewPortWidth - sx * viewBoxWidth;
                xtrans = xtrans / sx;
                xtrans = xtrans / qreal(2) - viewBoxXmin;
            }
        }
        break;
    case HbNvgEngine::NvgPreserveAspectRatioXmaxYmin:
        //Align the <min-x>+<width> of the element's viewBox with the maximum X value of the viewport.
        //Align the <min-y> of the element's viewBox with the smallest Y value of the viewport.
        if (mMeetSlice == HbNvgEngine::NvgMeet) {
            if (sx > sy) {
                sx = sy;
                xtrans = ((viewPortWidth - ((viewBoxWidth / viewBoxHeight) * viewPortHeight))) / sx - viewBoxXmin;
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
                xtrans = viewPortWidth - sx * viewBoxWidth;
                xtrans = xtrans / sx - viewBoxXmin;
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
                          (viewPortHeight - ((qreal)(viewBoxHeight / viewBoxWidth) * viewPortWidth)) * qreal(.5f)) / sy - viewBoxYmin;
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
                ytrans = viewPortHeight - sx * viewBoxHeight;
                ytrans = ytrans / sx;
                ytrans = ytrans / qreal(2) - viewBoxYmin;
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
                xtrans = ((viewPortWidth - ((viewBoxWidth / viewBoxHeight) * viewPortHeight)) * (.5f)) / sx - viewBoxXmin;

            } else if (sx < sy) {
                sy = sx;
                ytrans = ((viewPortHeight - ((viewBoxHeight / viewBoxWidth) * viewPortWidth)) * (.5f)) / sy - viewBoxYmin;
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
                ytrans = viewPortHeight - sx * viewBoxHeight;
                ytrans = ytrans / sx;
                ytrans = ytrans / qreal(2) - viewBoxYmin;
            } else {
                sx = sy;
                xtrans = viewPortWidth - sx * viewBoxWidth;
                xtrans = xtrans / sx;
                xtrans = xtrans / qreal(2) - viewBoxXmin;
            }
        }
        break;
    case HbNvgEngine::NvgPreserveAspectRatioXmaxYmid:
        //Align the <min-x>+<width> of the element's viewBox with the maximum X value of the viewport.
        //Align the midpoint Y value of the element's viewBox with the midpoint Y value of the viewport.
        if (mMeetSlice == HbNvgEngine::NvgMeet) {
            if (sx > sy) {
                sx = sy;
                xtrans = ((viewPortWidth - ((viewBoxWidth / viewBoxHeight) * viewPortHeight))) / sx - viewBoxXmin;
            } else {
                sy = sx;
                ytrans = ((viewPortHeight - ((viewBoxHeight / viewBoxWidth) * viewPortWidth)) * (.5f)) / sy - viewBoxYmin;
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
                ytrans = viewPortHeight - sx * viewBoxHeight;
                ytrans = ytrans / sx;
                ytrans = ytrans / qreal(2) - viewBoxYmin;
            } else {
                sx = sy;
                xtrans = viewPortWidth - sx * viewBoxWidth;
                xtrans = xtrans / sx - viewBoxXmin;
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

                ytrans = ((viewPortHeight - ((viewBoxHeight / viewBoxWidth) * viewPortWidth))) / sy - viewBoxYmin;
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
                ytrans = viewPortHeight - sx * viewBoxHeight;
                ytrans = ytrans / sx - viewBoxYmin;
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
                xtrans = ((viewPortWidth - ((viewBoxWidth / viewBoxHeight) * viewPortHeight)) * qreal(.5f)) / sx - viewBoxXmin;
            } else {
                sy = sx;
                ytrans = ((viewPortHeight - ((viewBoxHeight / viewBoxWidth) * viewPortWidth))) / sy - viewBoxYmin;
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
                ytrans = viewPortHeight - sx * viewBoxHeight;
                ytrans = ytrans / sx - viewBoxYmin;
            } else {
                sx = sy;
                xtrans = viewPortWidth - sx * viewBoxWidth;
                xtrans = xtrans / sx;
                xtrans = xtrans / qreal(2) - viewBoxXmin;
            }
        }
        break;
    case HbNvgEngine::NvgPreserveAspectRatioXmaxYmax:
        //Align the <min-x>+<width> of the element's viewBox with the maximum X value of the viewport.
        //Align the <min-y>+<height> of the element's viewBox with the maximum Y value of the viewport.
        if (mMeetSlice == HbNvgEngine::NvgMeet) {
            if (sx > sy) {
                sx = sy;
                xtrans = ((viewPortWidth - ((viewBoxWidth / viewBoxHeight) * viewPortHeight))) / sx - viewBoxXmin;
            } else {
                sy = sx;
                ytrans = ((viewPortHeight - ((viewBoxHeight / viewBoxWidth) * viewPortWidth))) / sy - viewBoxYmin;
            }
        } else if (mMeetSlice == HbNvgEngine::NvgSlice) {
            if (sx > sy) {
                sy = sx;
                ytrans = viewPortHeight - sx * viewBoxHeight;
                ytrans = ytrans / sx - viewBoxYmin;
            } else {
                sx = sy;
                xtrans = viewPortWidth - sx * viewBoxWidth;
                xtrans = xtrans / sx - viewBoxXmin;
            }
        }
        break;
    default:
        break;
    }

    vgScale(sx, sy);
    vgTranslate(xtrans, ytrans);
}

