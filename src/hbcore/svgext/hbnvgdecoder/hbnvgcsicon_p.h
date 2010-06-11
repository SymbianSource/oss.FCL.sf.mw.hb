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

#ifndef HB_NVGCSICON_P_H
#define HB_NVGCSICON_P_H

#include <QByteArray>
#include <QSize>
#include <QtGlobal>
#include <QPoint>

#include <VG/openvg.h>
#include "hbnvgicon_p.h"

/**
 *The multiplying factor to convert degrees to radians.
 */
const qreal DegToRad = 0.017453292519943296;

enum NvgBitEncoding  {
    NvgEightBitEncoding = 1,
    NvgSixteenBitEncoding ,
    NvgThirtyTwoBitEncoding,
    NvgFloatEncoding
};

class HbDereferencer;
class HbNvgEngine;
class HbOpenVgHandleStore;
class HbNvgFitToViewBoxImpl;

class HbNvgCsIcon :  public HbNvgIcon
{
private:
    enum NvgCsIconCommands  {
        NvgPath,
        NvgPathData,
        NvgPaint,
        NvgColorRamp,
        NvgTransform,
        NvgStrokeWidth,
        NvgStrokeMiterLimit,
        NvgStrokeLineJoinCap,
        NvgStrokePaint,
        NvgStrokeColorRamp
    };

public:
    HbNvgCsIcon();

    virtual ~HbNvgCsIcon();

    HbNvgEngine::HbNvgErrorType draw(const QSize &size)
    {
	   // TODO @ Deprecated, remove from base class
       Q_UNUSED(size);
       return HbNvgEngine::NvgErrNone;
    }

    void setViewBox(float x, float y, float w, float h)
    {
        mViewBoxX = x;
        mViewBoxY = y;
        mViewBoxW = w;
        mViewBoxH = h;
    }

    void setPreserveAspectRatio(HbNvgEngine::HbNvgAlignType preserveAspectSetting,
                                HbNvgEngine::HbNvgMeetType smilFitSetting);

    void rotate(float angle, float xValue, float yValue) ;

    void enableMirroring(bool mirroringMode);

    void directDraw(const QByteArray &buffer, const QSize &targetSize);

    void create(const QByteArray &buffer, const QSize& targetSize)
    {
	   // TODO @ Deprecated, remove from base class
        Q_UNUSED(buffer);
        Q_UNUSED(targetSize);
    }

private:

    HbNvgEngine::HbNvgErrorType initializeGc();

    void drawCommandSection(const QByteArray &buffer, const QSize &targetSize);

    HbNvgEngine::HbNvgErrorType createPathHandle(qint16 pathDataType, float scale, float bias);

    HbNvgEngine::HbNvgErrorType doDraw(const QSize &size);

    void setRotation();

    void setFillPaint(HbDereferencer *iconData);

    void setColorRamp(HbDereferencer * iconData);

    void drawPath(HbDereferencer * iconData);

    void setTransform(HbDereferencer * iconData, quint32 & counter, const VGfloat* currentMatrix);

    void setStrokePaint(HbDereferencer * iconData);

    void setStrokeWidth(const quint8* buffer);

    void setStrokeMiterLimit(const quint8* buffer);

    void resetNvgState();

    void applyViewboxToViewPortTransformation(const QSize& targetSize, float viewboxX, float viewboxY, float viewboxW, float viewboxH);


    void executeNvgCsCommandLoop(quint16 commandCount, HbDereferencer * iconData, HbDereferencer * offsetVector,
                                 HbDereferencer * commandSection, quint8 nvgVersion);

    HbDereferencer getCommandSection(quint16 offset, HbDereferencer * iconData, qint32 nvgVersion);

    template <class T>
    inline T Align4(T value) {
        return((T)((((quint32)value) + sizeof(quint32) - 1)&~(sizeof(quint32) - 1)));
    }

    template <class T>
    inline T Align2(T value) {
        return((T)((((quint32)value) + sizeof(quint16) - 1)&~(sizeof(quint16) - 1)));
    }

private:

    VGPaint                 mPaintFill;
    VGPaint                 mPaintStroke;
    VGPath                  mVgPath;
    qint16                  mLastPathDataType;
    VGboolean               mDoFill;
    VGboolean               mDoStroke;
    VGPaint                 mGradPaintFill;
    qint32                  mFillAlpha;
    qint32                  mStrokeAlpha;
    VGPaint                 mGradPaintStroke;
    float                   mViewBoxX;
    float                   mViewBoxY;
    float                   mViewBoxW;
    float                   mViewBoxH;
    float                   mRotationAngle;
    float                   mRotationX;
    float                   mRotationY;
    HbNvgEngine::HbNvgAlignType      mPreserveAspectSetting;
    HbNvgEngine::HbNvgMeetType      mSmilFitSetting;    
    bool                    mMirrored;
};
#endif
