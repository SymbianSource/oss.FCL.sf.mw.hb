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

	HbTlvIconCreator(const QByteArray &ptr, int width, int height, HbNvgTlvIcon * nvgTlvIcon);
	
    virtual ~HbTlvIconCreator();

    HbNvgEngine::NvgErrorType execute();

    void setTlvIcon(HbNvgTlvIcon * nvgTlvIcon) {
        mNvgTlvIcon = nvgTlvIcon;
    }
    
    void initialize();

private:

    void dVgAddCommand(int length, int pos);
    void dVgAddCommand();
    void dVgSetParameterfv();
    void dVgSetPaint();
    void dVgAppendPathData();
    void dVgDrawPath();
    void dVgClearPath();
    void dVgCreatePaint();
    void dVgSetiv();
    void dVgClear();
    void dVgSetfv();
    void dVgCreatePath();
    void dVgCreateImage();
    void dVgGetPixels();
    void dVgClearImage();
    void dVgImageSubData();
    void dVgDrawImage();
    void dVgDestroyImage();
    void dVgDestroyPaint();
    void dVgDestroyPath();
    void dVguRect();
    void dVguEllipse();
    void dVguRoundRect();
    void dVguLine();
    void dVgPrepareToBindImage();
    void dVgBindImage();
    void dVgUnBindImage();
    void dVgFlush();
    HbNvgEngine::NvgErrorType execute(int index);

#ifdef HB_NVG_DEBUG
    //Command specific logging methods
    void logVgSeti(VGParamType type, VGint fvalue, int cmdsize);
    void logVgSetf(VGParamType type, VGfloat fvalue, int cmdsize);
    void logVgSetParameteri(VGHandle handle, VGint paramType, VGint pvalue, int cmdsize, int lpvalue);
    void logVgSetPaint(VGPaint paint, VGbitfield paintModes, int cmdsize, int lpvalue);
    void logVgDrawPath(VGbitfield paintModes, int cmdsize);
    void logVgSetParameterfv(VGPaint handle, VGint paramtype, int count, int handlenum);
#endif

private:

    typedef void (HbTlvIconCreator::*pvgapi)();

    VGPath  createPath();

    pvgapi vgapi[VgAPICount];

    HbNvgIconData *  mNvgIconData;
    int              mDataLength;
    QSize            mResizedCanvasSize;

    quint8          mNvgHeader[NvgHeaderSize];
    int             mTargetWidth;
    int             mTargetHeight;
    bool            mScaled;

    VGfloat         mUserMatrix[9];
    quint32         mPrepareToBindImage;

    HbNvgTlvIcon *   mNvgTlvIcon;

    qint8           mCurrentCommand;
    quint8 *        mCommandBuffer;
    VGPath          mLastVGPath;

    friend class HbNvgTlvIcon;
};

#endif
