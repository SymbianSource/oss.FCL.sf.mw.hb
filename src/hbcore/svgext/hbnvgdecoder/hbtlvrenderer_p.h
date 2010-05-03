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

#ifndef HB_TLVRENDERER_P_H
#define HB_TLVRENDERER_P_H


#include <VG/openvg.h>
#include <VG/vgu.h>

#include <QByteArray>
#include <QPoint>
#include <QSize>
#include <QtGlobal>
#include <QList>

#include "hbtlvcommon_p.h"
#include "hbnvgutil_p.h"
#include "hbnvg_p.h"

class HbNvgTlvIcon;
class HbNvgIconData;

class HbVgImageBinder;

class HbTlvRenderer
{

public:

    HbTlvRenderer(const QByteArray &ptr, qint32 width, qint32 height);

    virtual ~HbTlvRenderer();

    virtual void execute();

    void setVgImageBinder(HbVgImageBinder *imageBinder) {
        mVgImageBinder = imageBinder;
    }

    void initialize();

protected:

    void tlvVgSeti();
    void tlvVgSetf();
    void tlvVgSetParameteri();
    void tlvVgSetParameterf();
    void tlvVgSetParameterfv();
    void tlvVgSetColor();
    void tlvVgSetPaint();
    void tlvVgLoadMatrix();
    void tlvVgMultMatrix();
    void tlvVgLoadIdentity();
    void tlvVgScale();
    void tlvVgTranslate();
    void tlvVgAppendPathData();
    void tlvVgDrawPath();
    void tlvVgClearPath();
    void tlvVguRect();
    void tlvVguEllipse();
    void tlvVguRoundRect();
    void tlvVguLine();
    void tlvVgCreatePaint();
    void tlvVgSetiv();
    void tlvVgClear();
    void tlvVgSetfv();
    void tlvVgRotate();
    void tlvVgCreatePath();
    void tlvVgCreateImage();
    void tlvVgGetPixels();
    void tlvVgClearImage();
    void tlvVgImageSubData();
    void tlvVgDrawImage();
    void tlvVgDestroyImage();
    void tlvVgDestroyPaint();
    void tlvVgDestroyPath();
    void tlvVgPrepareToBindImage();
    void tlvVgBindImage();
    void tlvVgUnBindImage();
    void tlvVgFlush();

    virtual void execute(qint32 index);

#ifdef HB_NVG_DEBUG
    //Command specific logging methods
    void logVgSeti(VGParamType type, VGint fvalue);
    void logVgSetf(VGParamType type, VGfloat fvalue);
    void logvgSetXv(VGParamType type, VGint count, qint8 vtype, VGint * data);
    void logVgSetParameteri(VGHandle handle, VGint paramType, VGint pvalue, qint32 Lpvalue);
    void logVgSetPaint(VGPaint paint, VGbitfield paintModes, qint32 Lpvalue);
    void logVgDrawPath(VGuint path, VGbitfield paintModes);
    void logVgSetParameterfv(VGPaint handle, VGint paramtype, qint32 count, float* handlenum);
    void logvgImageSubData(VGImage imageHandle, const void * dataPtr, qint32 dataLength, VGint dataStride,
                           VGImageFormat dataFormat, VGint x, VGint y, VGint width, VGint height);
#endif

protected:

    typedef void (HbTlvRenderer::*VgApiCallBack)();
    VgApiCallBack vgapi[VgApiCount];

    HbNvgIconData *  mNvgIconData;
    qint32             mDataLength;
    QSize           mResizedCanvasSize;

    struct HbTlvVgHandlePair {
        enum HbHandleType {
            VgPath,
            VgPaint,
            VgImage
        };
        VGHandle      mVgHandle;
        qint32           mTlvHandle;
        HbHandleType  mHandleType;
        qint32           mCustomData;

        explicit HbTlvVgHandlePair(VGHandle vgHandle = 0, qint32 tlvHandle = 0,
                                   HbHandleType handleType = VgPaint, qint32 customData = 0)
                : mVgHandle(vgHandle),
                mTlvHandle(tlvHandle),
                mHandleType(handleType),
                mCustomData(customData) {
        }
    };

    VGHandle getHandle(qint32 index);
    VGHandle removeHandle(qint32 index);
    void getHandlePair(qint32 index, HbTlvVgHandlePair & pair);

    void setHandle(qint32 index, VGHandle handle);

    QPoint  getTranslatedPoint(VGfloat *matrix, const QPoint &point);
    VGfloat minVal4(VGfloat x1, VGfloat x2, VGfloat x3, VGfloat x4);
    VGfloat maxVal4(VGfloat x1, VGfloat x2, VGfloat x3, VGfloat x4);

    VGPath          mPathHandle;

    quint8          mNvgHeader[NvgHeaderSize];
    qint32            mTargetWidth;
    qint32            mTargetHeight;
    bool           mScaled;

    VGfloat             mUserMatrix[9];
    HbVgImageBinder *    mVgImageBinder;
    qint32               mPrepareToBindImage;
    QList<HbTlvVgHandlePair > mTlvVgHandleList;

    HbNvgTlvIcon *   mNvgTlvIcon;
    friend class HbNvgTlvIcon;

    DECLARE_HANDLECHECKER()
};

#endif

