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

	HbTlvRenderer(const QByteArray &ptr, int width, int height);

    virtual ~HbTlvRenderer();

    virtual void execute();

    void setVgImageBinder(HbVgImageBinder *imageBinder) 
    {
        mVgImageBinder = imageBinder;
    }

    void initialize();
    
protected:

    void dVgSeti();
    void dVgSetf();
    void dVgSetParameteri();
    void dVgSetParameterf();
    void dVgSetParameterfv();
    void dVgSetColor();
    void dVgSetPaint();
    void dVgLoadMatrix();
    void dVgMultMatrix();
    void dVgLoadIdentity();
    void dVgScale();
    void dVgTranslate();
    void dVgAppendPathData();
    void dVgDrawPath();
    void dVgClearPath();
    void dVguRect();
    void dVguEllipse();
    void dVguRoundRect();
    void dVguLine();
    void dVgCreatePaint();
    void dVgSetiv();
    void dVgClear();
    void dVgSetfv();
    void dVgRotate();
    void dVgCreatePath();
    void dVgCreateImage();
    void dVgGetPixels();
    void dVgClearImage();
    void dVgImageSubData();
    void dVgDrawImage();
    void dVgDestroyImage();
    void dVgDestroyPaint();
    void dVgDestroyPath();
    void dVgPrepareToBindImage();
    void dVgBindImage();
    void dVgUnBindImage();
    void dVgFlush();

    virtual void execute(int index);

#ifdef HB_NVG_DEBUG
    //Command specific logging methods
    void logVgSeti(VGParamType type, VGint fvalue);
    void logVgSetf(VGParamType type, VGfloat fvalue);
    void logvgSetXv(VGParamType type, VGint count, qint8 vtype, VGint * data); 
    void logVgSetParameteri(VGHandle handle, VGint paramType, VGint pvalue, int Lpvalue);
    void logVgSetPaint(VGPaint paint, VGbitfield paintModes, int Lpvalue);
    void logVgDrawPath(VGuint path, VGbitfield paintModes);
    void logVgSetParameterfv(VGPaint handle, VGint paramtype, int count, float* handlenum);
    void logvgImageSubData(VGImage imageHandle, const void * dataPtr, int dataLength, VGint dataStride,
                    VGImageFormat dataFormat, VGint x, VGint y, VGint width, VGint height);
#endif

protected:

    typedef void (HbTlvRenderer::*pvgapi)();
    pvgapi vgapi[VgAPICount];

    HbNvgIconData *  mNvgIconData;
    int             mDataLength;
    QSize           mResizedCanvasSize;

    struct HbTlvVgHandlePair {
        enum HandleType {
            VgPath,
            VgPaint,
            VgImage
        };
        VGHandle    mVgHandle;
        int         mTlvHandle;
        HandleType  mHandleType;
        int         mCustomData;

        explicit HbTlvVgHandlePair(VGHandle vgHandle = 0, int tlvHandle = 0,
                                   HandleType handleType = VgPaint, int customData = 0)
                : mVgHandle(vgHandle),
                mTlvHandle(tlvHandle),
                mHandleType(handleType),
                mCustomData(customData) {
        }
    };

    VGHandle getHandle(int index);
    VGHandle removeHandle(int index);
    void getHandlePair(int index, HbTlvVgHandlePair & pair);

    void setHandle(int index, VGHandle handle);

    QPoint  getTranslatedPoint(VGfloat *matrix, const QPoint &point);
    VGfloat minVal4(VGfloat x1, VGfloat x2, VGfloat x3, VGfloat x4);
    VGfloat maxVal4(VGfloat x1, VGfloat x2, VGfloat x3, VGfloat x4);

    VGPath          mPathHandle;

    quint8          mNvgHeader[NvgHeaderSize];
    int            mTargetWidth;
    int            mTargetHeight;
    bool           mScaled;

    VGfloat             mUserMatrix[9];
    HbVgImageBinder *    mVgImageBinder;
    int               mPrepareToBindImage;
    QList<HbTlvVgHandlePair > mTlvVgHandleList;

    HbNvgTlvIcon *   mNvgTlvIcon;
    friend class HbNvgTlvIcon;
    
    DECLARE_HANDLECHECKER()
};

#endif

