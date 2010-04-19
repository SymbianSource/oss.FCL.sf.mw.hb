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

#include "hbtlviconcreator_p.h"
#include "hbnvgimagebinder_p.h"
#include "hbnvgutil_p.h"
#include "hbnvgicondata_p.h"
#include "hbnvgtlvicon_p.h"

#include <QScopedArrayPointer>

/*!
 Length of each encoded OpenVG API in bytes. -1 if not pre known.
 The order here and in the enum TOpenVGAPI should be same.
 */
const int apiCommandLength[] = {
    16, //VgClear
    4, //VgSeti
    6, //VgSetf
    -1, //VgSetfv
    5, //VgSetParameteri
    7, //VgSetParameterf
    -1, //VgSetParameterfv
    5, //VgSetColor
    2, //VgSetPaint
    36, //VgLoadMatrix
    36, //VgMultMatrix
    0, //VgLoadIdentity
    8, //VgScale
    4, //VgRotate
    8, //VgTranslate
    -1, //VgAppendPathData
    3, //VgDrawPath
    3, //VgClearPath
    17, //VguRect
    17, //VguEllipse
    25, //VguRoundRect
    17, //VguLine
    1, //VgCreatePaint
    26, //VgCreatePath
    0, //VgGetPixels
    1, //VgDestroyImage
    0, //VgDestroyPaint
    0, //VgDestroyPath
    -1, //VgSetiv
    14, //VgCreateImage
    1, //VgDrawImage
    17, //VgClearImage
    -1, //VgImageSubData
    0, //VgPrepareToBindImage
    4, //VgBindImage
    0, //VgUnBindImage
    0  //VgFlush
};

HbTlvIconCreator::HbTlvIconCreator(const QByteArray &ptr, int width, int height, HbNvgTlvIcon * nvgTlvIcon)
{
    vgapi[VgSeti]              = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgSetf]              = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgSetParameteri]     = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgSetParameterf]     = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgSetParameterfv]    = &HbTlvIconCreator::dVgSetParameterfv;
    vgapi[VgSetColor]          = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgSetPaint]          = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgLoadMatrix]        = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgMultMatrix]        = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgLoadIdentity]      = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgScale]             = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgTranslate]         = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgAppendPathData]    = &HbTlvIconCreator::dVgAppendPathData;
    vgapi[VgDrawPath]          = &HbTlvIconCreator::dVgDrawPath;
    vgapi[VgClearPath]         = &HbTlvIconCreator::dVgClearPath;
    vgapi[VguRect]             = &HbTlvIconCreator::dVguRect;
    vgapi[VguEllipse]          = &HbTlvIconCreator::dVguEllipse;
    vgapi[VguRoundRect]        = &HbTlvIconCreator::dVguRoundRect;
    vgapi[VguLine]             = &HbTlvIconCreator::dVguLine;
    vgapi[VgCreatePaint]       = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgSetiv]             = &HbTlvIconCreator::dVgSetiv;
    vgapi[VgClear]             = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgSetfv]             = &HbTlvIconCreator::dVgSetfv;
    vgapi[VgRotate]            = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgCreatePath]        = &HbTlvIconCreator::dVgCreatePath;
    vgapi[VgCreateImage]       = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgGetPixels]         = &HbTlvIconCreator::dVgGetPixels;
    vgapi[VgDrawImage]         = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgClearImage]        = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgImageSubData]      = &HbTlvIconCreator::dVgImageSubData;
    vgapi[VgDestroyImage]      = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgDestroyPaint]      = &HbTlvIconCreator::dVgDestroyPaint;
    vgapi[VgDestroyPath]       = &HbTlvIconCreator::dVgDestroyPath;
    vgapi[VgPrepareToBindImage] = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgBindImage]         = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgUnBindImage]       = &HbTlvIconCreator::dVgAddCommand;
    vgapi[VgFlush]             = &HbTlvIconCreator::dVgAddCommand;

    mTargetWidth        = width;
    mTargetHeight       = height;
    mPrepareToBindImage = 0;
    mNvgTlvIcon         = nvgTlvIcon;
    mLastVGPath         = 0;

    vgGetMatrix(mUserMatrix);

    mNvgIconData = new HbNvgIconData(ptr);
    Q_CHECK_PTR(mNvgIconData);
    mDataLength  = ptr.size();

    mCommandBuffer = (quint8 *)ptr.data();
}

void HbTlvIconCreator::initialize()
{
    mNvgIconData->beginRead();
    mNvgIconData->read(mNvgHeader, sizeof(mNvgHeader));
    mNvgTlvIcon->addCommand(mNvgHeader, sizeof(mNvgHeader));
}

HbTlvIconCreator::~HbTlvIconCreator()
{
    if (mLastVGPath) {
        vgDestroyPath(mLastVGPath);
    }

    delete mNvgIconData;
}

void HbTlvIconCreator::dVgAddCommand(int length, int pos)
{
    mNvgTlvIcon->addCommand(mCurrentCommand, mCommandBuffer + pos, length);
}

void HbTlvIconCreator::dVgAddCommand()
{
    mNvgTlvIcon->addCommand(mCurrentCommand, mCommandBuffer + mNvgIconData->readPos(),
                            apiCommandLength[mCurrentCommand]);
    mNvgIconData->skip(apiCommandLength[mCurrentCommand]);
}

void HbTlvIconCreator::dVgSetParameterfv()
{
	int length = 0;
    int cpos = mNvgIconData->readPos();

    mNvgIconData->readInt32();
    mNvgIconData->readInt16();
    quint32 countt = mNvgIconData->readInt32();
	length = length + sizeof(quint32) + sizeof(quint16) + sizeof(quint32) + countt * sizeof(float);
    mNvgIconData->skip(countt * sizeof(float));

    dVgAddCommand(length, cpos);
}

void HbTlvIconCreator::dVgSetiv()
{
	int length   = 0;
    int cpos     = mNvgIconData->readPos();
    mNvgIconData->readInt16();
    quint16 count = mNvgIconData->readInt16();
	
	length = sizeof(qint16) + sizeof(qint16);
    length += count * sizeof(qint32);
    
    mNvgIconData->skip(count * sizeof(qint32));

    dVgAddCommand(length, cpos);
}

VGPath  HbTlvIconCreator::createPath()
{
    const float scale = 1.0f / 65536.0f;

    VGPath pathH = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                                VG_PATH_DATATYPE_S_32, scale, 0.0f, 0, 0,
                                VG_PATH_CAPABILITY_APPEND_TO);
	
    if (pathH == VG_INVALID_HANDLE)
        {
        throw HbNvgException(openVgErrorToHbNvgError(vgGetError()));
        }
    return pathH;
}

void HbTlvIconCreator::dVgClearPath()
{
    mNvgIconData->readInt8();
    mNvgIconData->readInt16();
}

void HbTlvIconCreator::dVgAppendPathData()
{
    quint16         numSegments;
    quint16         coordinatecount;
    
	mNvgIconData->readInt32();
    numSegments = mNvgIconData->readInt16();

    VGubyte *pSegArry = new VGubyte[numSegments];
    Q_CHECK_PTR(pSegArry);
    QScopedArrayPointer<quint8> pathSegments(pSegArry);

    for (int j = 0; j < numSegments; j++ ) {
        pathSegments[j] = mNvgIconData->readInt8();
    }

    coordinatecount = mNvgIconData->readInt16();

    qint32 *pDataArry = new qint32[coordinatecount];
    Q_CHECK_PTR(pDataArry);
    QScopedArrayPointer<qint32> pathData(pDataArry);
    
    for (int i = 0; i < coordinatecount; i++) {
        pathData[i] = mNvgIconData->readInt32();
    }

    if (mLastVGPath) {
        vgDestroyPath(mLastVGPath);
        mLastVGPath = 0;
    }
    mLastVGPath = createPath();
    
    vgAppendPathData(mLastVGPath, numSegments, pathSegments.data(), pathData.data());    
}

void HbTlvIconCreator::dVgDrawPath()
{
    mNvgIconData->readInt32();
    quint16 value = mNvgIconData->readInt16();

    mNvgTlvIcon->addDrawPathCommand(mLastVGPath, (VGPaintMode)value);
    mLastVGPath = 0;
}

void HbTlvIconCreator::dVguRect()
{
    mNvgIconData->readInt32();

    float x   = mNvgIconData->readReal32();
    float y   = mNvgIconData->readReal32();

    float width   = mNvgIconData->readReal32();
    float height  = mNvgIconData->readReal32();

    if (mLastVGPath) {
        vgDestroyPath(mLastVGPath);
        mLastVGPath = 0;
    }
    mLastVGPath = createPath();

    vguRect(mLastVGPath, x, y, width, height);

}

void HbTlvIconCreator::dVguEllipse()
{
    float cx;
    float cy;
    float width;
    float height;

    mNvgIconData->readInt32();

    cx = mNvgIconData->readReal32();
    cy = mNvgIconData->readReal32();

    width  = mNvgIconData->readReal32();
    height = mNvgIconData->readReal32();

    if (mLastVGPath) {
        vgDestroyPath(mLastVGPath);
        mLastVGPath = 0;
    }
    mLastVGPath = createPath();

    vguEllipse(mLastVGPath, cx, cy, width, height);
}

void HbTlvIconCreator::dVguRoundRect()
{
    float x;
    float y;
    float width;
    float height;
    float arcWidth;
    float arcHeight;

    mNvgIconData->readInt32();

    x = mNvgIconData->readReal32();
    y = mNvgIconData->readReal32();

    width = mNvgIconData->readReal32();
    height = mNvgIconData->readReal32();

    arcWidth = mNvgIconData->readReal32();
    arcHeight = mNvgIconData->readReal32();

    if (mLastVGPath) {
        vgDestroyPath(mLastVGPath);
        mLastVGPath = 0;
    }
    mLastVGPath = createPath();

    vguRoundRect(mLastVGPath, x, y, width, height, arcWidth, arcHeight);
}

void HbTlvIconCreator::dVguLine()
{
    float x0;
    float y0;
    float x1;
    float y1;

    mNvgIconData->readInt32();

    x0 = mNvgIconData->readReal32();
    y0 = mNvgIconData->readReal32();

    x1 = mNvgIconData->readReal32();
    y1 = mNvgIconData->readReal32();

    if (mLastVGPath) {
        vgDestroyPath(mLastVGPath);
        mLastVGPath = 0;
    }
    mLastVGPath = createPath();

    vguLine(mLastVGPath, x0, y0, x1, y1);
 }

void HbTlvIconCreator::dVgSetfv()
{
	int length = 0;
    quint16 type;
    quint16 count;
    int cpos   = mNvgIconData->readPos();

    type  = mNvgIconData->readInt16();
    Q_UNUSED(type);
    count = mNvgIconData->readInt16();

    mNvgIconData->skip(sizeof(float) * count);
    
    length = length + 2 * sizeof(quint16) + count * sizeof(VGfloat);
    
    dVgAddCommand(length, cpos);
    }

void HbTlvIconCreator::dVgCreatePath()
{
    mNvgIconData->skip(apiCommandLength[VgCreatePath]);
}

void HbTlvIconCreator::dVgImageSubData()
{
    int cpos   = mNvgIconData->readPos();
	
	mNvgIconData->readInt32();
    int dataLength         = 0;

    mNvgIconData->readInt32();
    mNvgIconData->readInt32();
    mNvgIconData->readInt32();
    mNvgIconData->readInt32();
    mNvgIconData->readInt32();
    mNvgIconData->readInt32();
    dataLength = mNvgIconData->readInt32();
    mNvgIconData->skip(dataLength);

    dVgAddCommand(mNvgIconData->readPos() - cpos, cpos);
}

void HbTlvIconCreator::dVgGetPixels()
{
}

void HbTlvIconCreator::dVgDestroyPaint()
{
}

void HbTlvIconCreator::dVgDestroyPath()
{
}

void HbTlvIconCreator::dVgFlush()
{
}

HbNvgEngine::NvgErrorType HbTlvIconCreator::execute(int index)
{
    HbNvgEngine::NvgErrorType ret = HbNvgEngine::NvgErrNone;
    mCurrentCommand = index;
    if (0 <= index && index < VgFlush + 1) {
        (this->*(vgapi[index]))();
    } else {
        throw HbNvgException(HbNvgEngine::NvgErrCorrupt);
    }
    return ret;
}

HbNvgEngine::NvgErrorType HbTlvIconCreator::execute()
{
HbNvgEngine::NvgErrorType error = HbNvgEngine::NvgErrNone;

    while (!mNvgIconData->eof()) {
    	execute(mNvgIconData->readInt8());
    }
    
    return error;
}

