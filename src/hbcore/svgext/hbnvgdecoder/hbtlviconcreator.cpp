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
const qint32 apiCommandLength[] = {
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

HbTlvIconCreator::HbTlvIconCreator(const QByteArray &ptr, qint32 width, qint32 height, HbNvgTlvIcon * nvgTlvIcon)
{
    vgapi[VgSeti]              = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgSetf]              = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgSetParameteri]     = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgSetParameterf]     = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgSetParameterfv]    = &HbTlvIconCreator::tlvVgSetParameterfv;
    vgapi[VgSetColor]          = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgSetPaint]          = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgLoadMatrix]        = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgMultMatrix]        = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgLoadIdentity]      = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgScale]             = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgTranslate]         = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgAppendPathData]    = &HbTlvIconCreator::tlvVgAppendPathData;
    vgapi[VgDrawPath]          = &HbTlvIconCreator::tlvVgDrawPath;
    vgapi[VgClearPath]         = &HbTlvIconCreator::tlvVgClearPath;
    vgapi[VguRect]             = &HbTlvIconCreator::tlvVguRect;
    vgapi[VguEllipse]          = &HbTlvIconCreator::tlvVguEllipse;
    vgapi[VguRoundRect]        = &HbTlvIconCreator::tlvVguRoundRect;
    vgapi[VguLine]             = &HbTlvIconCreator::tlvVguLine;
    vgapi[VgCreatePaint]       = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgSetiv]             = &HbTlvIconCreator::tlvVgSetiv;
    vgapi[VgClear]             = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgSetfv]             = &HbTlvIconCreator::tlvVgSetfv;
    vgapi[VgRotate]            = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgCreatePath]        = &HbTlvIconCreator::tlvVgCreatePath;
    vgapi[VgCreateImage]       = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgGetPixels]         = &HbTlvIconCreator::tlvVgGetPixels;
    vgapi[VgDrawImage]         = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgClearImage]        = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgImageSubData]      = &HbTlvIconCreator::tlvVgImageSubData;
    vgapi[VgDestroyImage]      = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgDestroyPaint]      = &HbTlvIconCreator::tlvVgDestroyPaint;
    vgapi[VgDestroyPath]       = &HbTlvIconCreator::tlvVgDestroyPath;
    vgapi[VgPrepareToBindImage] = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgBindImage]         = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgUnBindImage]       = &HbTlvIconCreator::tlvVgAddCommand;
    vgapi[VgFlush]             = &HbTlvIconCreator::tlvVgAddCommand;

    mTargetWidth        = width;
    mTargetHeight       = height;
    mPrepareToBindImage = 0;
    mNvgTlvIcon         = nvgTlvIcon;
    mLastVgPath         = 0;

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
    if (mLastVgPath) {
        vgDestroyPath(mLastVgPath);
    }

    delete mNvgIconData;
}

void HbTlvIconCreator::tlvVgAddCommand(qint32 length, qint32 pos)
{
    mNvgTlvIcon->addCommand(mCurrentCommand, mCommandBuffer + pos, length);
}

void HbTlvIconCreator::tlvVgAddCommand()
{
    mNvgTlvIcon->addCommand(mCurrentCommand, mCommandBuffer + mNvgIconData->readPos(),
                            apiCommandLength[mCurrentCommand]);
    mNvgIconData->skip(apiCommandLength[mCurrentCommand]);
}

void HbTlvIconCreator::tlvVgSetParameterfv()
{
    qint32 length = 0;
    qint32 cpos = mNvgIconData->readPos();

    mNvgIconData->readInt32();
    mNvgIconData->readInt16();
    quint32 countt = mNvgIconData->readInt32();
    length = length + sizeof(quint32) + sizeof(quint16) + sizeof(quint32) + countt * sizeof(float);
    mNvgIconData->skip(countt * sizeof(float));

    tlvVgAddCommand(length, cpos);
}

void HbTlvIconCreator::tlvVgSetiv()
{
    qint32 length   = 0;
    qint32 cpos     = mNvgIconData->readPos();
    mNvgIconData->readInt16();
    quint16 count = mNvgIconData->readInt16();

    length = sizeof(qint16) + sizeof(qint16);
    length += count * sizeof(qint32);

    mNvgIconData->skip(count * sizeof(qint32));

    tlvVgAddCommand(length, cpos);
}

VGPath  HbTlvIconCreator::createPath()
{
    const float scale = 1.0f / 65536.0f;

    VGPath pathH = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                                VG_PATH_DATATYPE_S_32, scale, 0.0f, 0, 0,
                                VG_PATH_CAPABILITY_APPEND_TO);

    if (pathH == VG_INVALID_HANDLE) {
        throw HbNvgException(openVgErrorToHbNvgError(vgGetError()));
    }
    return pathH;
}

void HbTlvIconCreator::tlvVgClearPath()
{
    mNvgIconData->readInt8();
    mNvgIconData->readInt16();
}

void HbTlvIconCreator::tlvVgAppendPathData()
{
    quint16         numSegments;
    quint16         coordinateCount;

    mNvgIconData->readInt32();
    numSegments = mNvgIconData->readInt16();

    VGubyte *pSegArry = new VGubyte[numSegments];
    Q_CHECK_PTR(pSegArry);
    QScopedArrayPointer<quint8> pathSegments(pSegArry);

    for (qint32 j = 0; j < numSegments; j++) {
        pathSegments[j] = mNvgIconData->readInt8();
    }

    coordinateCount = mNvgIconData->readInt16();

    qint32 *pDataArry = new qint32[coordinateCount];
    Q_CHECK_PTR(pDataArry);
    QScopedArrayPointer<qint32> pathData(pDataArry);

    for (qint32 i = 0; i < coordinateCount; i++) {
        pathData[i] = mNvgIconData->readInt32();
    }

    if (mLastVgPath) {
        vgDestroyPath(mLastVgPath);
        mLastVgPath = 0;
    }
    mLastVgPath = createPath();

    vgAppendPathData(mLastVgPath, numSegments, pathSegments.data(), pathData.data());
}

void HbTlvIconCreator::tlvVgDrawPath()
{
    mNvgIconData->readInt32();
    quint16 value = mNvgIconData->readInt16();

    mNvgTlvIcon->addDrawPathCommand(mLastVgPath, (VGPaintMode)value);
    mLastVgPath = 0;
}

void HbTlvIconCreator::tlvVguRect()
{
    mNvgIconData->readInt32();

    float x   = mNvgIconData->readReal32();
    float y   = mNvgIconData->readReal32();

    float width   = mNvgIconData->readReal32();
    float height  = mNvgIconData->readReal32();

    if (mLastVgPath) {
        vgDestroyPath(mLastVgPath);
        mLastVgPath = 0;
    }
    mLastVgPath = createPath();

    vguRect(mLastVgPath, x, y, width, height);

}

void HbTlvIconCreator::tlvVguEllipse()
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

    if (mLastVgPath) {
        vgDestroyPath(mLastVgPath);
        mLastVgPath = 0;
    }
    mLastVgPath = createPath();

    vguEllipse(mLastVgPath, cx, cy, width, height);
}

void HbTlvIconCreator::tlvVguRoundRect()
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

    if (mLastVgPath) {
        vgDestroyPath(mLastVgPath);
        mLastVgPath = 0;
    }
    mLastVgPath = createPath();

    vguRoundRect(mLastVgPath, x, y, width, height, arcWidth, arcHeight);
}

void HbTlvIconCreator::tlvVguLine()
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

    if (mLastVgPath) {
        vgDestroyPath(mLastVgPath);
        mLastVgPath = 0;
    }
    mLastVgPath = createPath();

    vguLine(mLastVgPath, x0, y0, x1, y1);
}

void HbTlvIconCreator::tlvVgSetfv()
{
    qint32 length = 0;
    quint16 type;
    quint16 count;
    qint32 cpos   = mNvgIconData->readPos();

    mNvgIconData->readInt16();
    
    count = mNvgIconData->readInt16();

    mNvgIconData->skip(sizeof(float) * count);

    length = length + 2 * sizeof(quint16) + count * sizeof(VGfloat);

    tlvVgAddCommand(length, cpos);
}

void HbTlvIconCreator::tlvVgCreatePath()
{
    mNvgIconData->skip(apiCommandLength[VgCreatePath]);
}

void HbTlvIconCreator::tlvVgImageSubData()
{
    qint32 cpos   = mNvgIconData->readPos();

    mNvgIconData->readInt32();
    qint32 dataLength         = 0;

    mNvgIconData->readInt32();
    mNvgIconData->readInt32();
    mNvgIconData->readInt32();
    mNvgIconData->readInt32();
    mNvgIconData->readInt32();
    mNvgIconData->readInt32();
    dataLength = mNvgIconData->readInt32();
    mNvgIconData->skip(dataLength);

    tlvVgAddCommand(mNvgIconData->readPos() - cpos, cpos);
}

void HbTlvIconCreator::tlvVgGetPixels()
{
}

void HbTlvIconCreator::tlvVgDestroyPaint()
{
}

void HbTlvIconCreator::tlvVgDestroyPath()
{
}

void HbTlvIconCreator::tlvVgFlush()
{
}

HbNvgEngine::HbNvgErrorType HbTlvIconCreator::execute(qint32 index)
{
    HbNvgEngine::HbNvgErrorType ret = HbNvgEngine::NvgErrNone;
    mCurrentCommand = index;
    if ((0 <= index) && (index < VgFlush + 1)) {
        (this->*(vgapi[index]))();
    } else {
        throw HbNvgException(HbNvgEngine::NvgErrCorrupt);
    }
    return ret;
}

HbNvgEngine::HbNvgErrorType HbTlvIconCreator::execute()
{
    HbNvgEngine::HbNvgErrorType error = HbNvgEngine::NvgErrNone;

    while (!mNvgIconData->eof()) {
        execute(mNvgIconData->readInt8());
    }

    return error;
}

