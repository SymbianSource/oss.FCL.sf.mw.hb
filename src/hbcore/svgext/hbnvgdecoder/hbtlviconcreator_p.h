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

#ifndef HB_TLVICONCREATER_P_H
#define HB_TLVICONCREATER_P_H

#include <VG/openvg.h>
#include <VG/vgu.h>
#include "hbtlvcommon_p.h"
#include "hbnvg_p.h"
#include <QSize>

class HbNvgTlvIcon;
class HbNvgIconData;

class HbTlvIconCreator
{
public:

    HbTlvIconCreator(const QByteArray &ptr, qint32 width, qint32 height, HbNvgTlvIcon * nvgTlvIcon);

    virtual ~HbTlvIconCreator();

    HbNvgEngine::HbNvgErrorType execute();

    void setTlvIcon(HbNvgTlvIcon * nvgTlvIcon) {
        mNvgTlvIcon = nvgTlvIcon;
    }

    void initialize();

private:

    void tlvVgAddCommand(qint32 length, qint32 pos);
    void tlvVgAddCommand();
    void tlvVgSetParameterfv();
    void tlvVgSetPaint();
    void tlvVgAppendPathData();
    void tlvVgDrawPath();
    void tlvVgClearPath();
    void tlvVgCreatePaint();
    void tlvVgSetiv();
    void tlvVgClear();
    void tlvVgSetfv();
    void tlvVgCreatePath();
    void tlvVgCreateImage();
    void tlvVgGetPixels();
    void tlvVgClearImage();
    void tlvVgImageSubData();
    void tlvVgDrawImage();
    void tlvVgDestroyImage();
    void tlvVgDestroyPaint();
    void tlvVgDestroyPath();
    void tlvVguRect();
    void tlvVguEllipse();
    void tlvVguRoundRect();
    void tlvVguLine();
    void tlvVgPrepareToBindImage();
    void tlvVgBindImage();
    void tlvVgUnBindImage();
    void tlvVgFlush();
    HbNvgEngine::HbNvgErrorType execute(qint32 index);

#ifdef HB_NVG_DEBUG
    //Command specific logging methods
    void logVgSeti(VGParamType type, VGint fvalue, qint32 cmdsize);
    void logVgSetf(VGParamType type, VGfloat fvalue, qint32 cmdsize);
    void logVgSetParameteri(VGHandle handle, VGint paramType, VGint pvalue, qint32 cmdsize, qint32 lpvalue);
    void logVgSetPaint(VGPaint paint, VGbitfield paintModes, qint32 cmdsize, qint32 lpvalue);
    void logVgDrawPath(VGbitfield paintModes, qint32 cmdsize);
    void logVgSetParameterfv(VGPaint handle, VGint paramtype, qint32 count, qint32 handlenum);
#endif

private:

    typedef void (HbTlvIconCreator::*VgApiCallBack)();

    VGPath  createPath();

    VgApiCallBack vgapi[VgApiCount];

    HbNvgIconData *  mNvgIconData;
    qint32              mDataLength;
    QSize            mResizedCanvasSize;

    quint8          mNvgHeader[NvgHeaderSize];
    qint32             mTargetWidth;
    qint32             mTargetHeight;
    bool            mScaled;

    VGfloat         mUserMatrix[9];
    quint32         mPrepareToBindImage;

    HbNvgTlvIcon *   mNvgTlvIcon;

    qint8           mCurrentCommand;
    quint8 *        mCommandBuffer;
    VGPath          mLastVgPath;

    friend class HbNvgTlvIcon;
};

#endif
