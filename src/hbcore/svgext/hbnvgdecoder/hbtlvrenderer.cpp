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

#include "hbtlvrenderer_p.h"
#include "hbnvgimagebinder_p.h"
#include "hbnvgutil_p.h"
#include "hbnvgicondata_p.h"
#include "hbnvgtlvicon_p.h"
#include "hbnvgexception_p.h"

#include <QScopedArrayPointer>

#include <QtDebug>

HbTlvRenderer::HbTlvRenderer(const QByteArray &buffer, int width, int height)
        : mVgImageBinder(0)
{
    vgapi[VgSeti]              = &HbTlvRenderer::dVgSeti;
    vgapi[VgSetf]              = &HbTlvRenderer::dVgSetf;
    vgapi[VgSetParameteri]     = &HbTlvRenderer::dVgSetParameteri;
    vgapi[VgSetParameterf]     = &HbTlvRenderer::dVgSetParameterf;
    vgapi[VgSetParameterfv]    = &HbTlvRenderer::dVgSetParameterfv;
    vgapi[VgSetColor]          = &HbTlvRenderer::dVgSetColor;
    vgapi[VgSetPaint]          = &HbTlvRenderer::dVgSetPaint;
    vgapi[VgLoadMatrix]        = &HbTlvRenderer::dVgLoadMatrix;
    vgapi[VgMultMatrix]        = &HbTlvRenderer::dVgMultMatrix;
    vgapi[VgLoadIdentity]      = &HbTlvRenderer::dVgLoadIdentity;
    vgapi[VgScale]             = &HbTlvRenderer::dVgScale;
    vgapi[VgTranslate]         = &HbTlvRenderer::dVgTranslate;
    vgapi[VgAppendPathData]    = &HbTlvRenderer::dVgAppendPathData;
    vgapi[VgDrawPath]          = &HbTlvRenderer::dVgDrawPath;
    vgapi[VgClearPath]         = &HbTlvRenderer::dVgClearPath;
    vgapi[VguRect]             = &HbTlvRenderer::dVguRect;
    vgapi[VguEllipse]          = &HbTlvRenderer::dVguEllipse;
    vgapi[VguRoundRect]        = &HbTlvRenderer::dVguRoundRect;
    vgapi[VguLine]             = &HbTlvRenderer::dVguLine;
    vgapi[VgCreatePaint]       = &HbTlvRenderer::dVgCreatePaint;
    vgapi[VgSetiv]             = &HbTlvRenderer::dVgSetiv;
    vgapi[VgClear]             = &HbTlvRenderer::dVgClear;
    vgapi[VgSetfv]             = &HbTlvRenderer::dVgSetfv;
    vgapi[VgRotate]            = &HbTlvRenderer::dVgRotate;
    vgapi[VgCreatePath]        = &HbTlvRenderer::dVgCreatePath;
    vgapi[VgCreateImage]       = &HbTlvRenderer::dVgCreateImage;
    vgapi[VgGetPixels]         = &HbTlvRenderer::dVgGetPixels;
    vgapi[VgDrawImage]         = &HbTlvRenderer::dVgDrawImage;
    vgapi[VgClearImage]        = &HbTlvRenderer::dVgClearImage;
    vgapi[VgImageSubData]      = &HbTlvRenderer::dVgImageSubData;
    vgapi[VgDestroyImage]      = &HbTlvRenderer::dVgDestroyImage;
    vgapi[VgDestroyPaint]      = &HbTlvRenderer::dVgDestroyPaint;
    vgapi[VgDestroyPath]       = &HbTlvRenderer::dVgDestroyPath;
    vgapi[VgPrepareToBindImage] = &HbTlvRenderer::dVgPrepareToBindImage;
    vgapi[VgBindImage]         = &HbTlvRenderer::dVgBindImage;
    vgapi[VgUnBindImage]       = &HbTlvRenderer::dVgUnBindImage;
    vgapi[VgFlush]             = &HbTlvRenderer::dVgFlush;

    mTargetWidth        = width;
    mTargetHeight       = height;
    mPrepareToBindImage = 0;
    mScaled = false;

    vgGetMatrix(mUserMatrix);

    mNvgIconData = new HbNvgIconData(buffer);
    Q_CHECK_PTR(mNvgIconData);
    
    mDataLength  = buffer.size();

	INIT_HANDLECHECKER()
}

void HbTlvRenderer::initialize()
{    
    mNvgIconData->beginRead();
    mNvgIconData->read(mNvgHeader, sizeof(mNvgHeader));
}

HbTlvRenderer::~HbTlvRenderer()
{
	if (mPathHandle != VG_INVALID_HANDLE){
        VGDESTROYPATH(mPathHandle);
    }

    int handleListCount    = mTlvVgHandleList.count();

    if (handleListCount > 0)
        {
        vgSetPaint(VG_INVALID_HANDLE, VG_FILL_PATH);
        vgSetPaint(VG_INVALID_HANDLE, VG_STROKE_PATH);
        }
        
    for (int i = 0; i < handleListCount; i++) {
        if (mTlvVgHandleList[i].mVgHandle) {
            switch (mTlvVgHandleList[i].mHandleType) {
            case HbTlvVgHandlePair::VgPath:
                VGDESTROYPATH(mTlvVgHandleList[i].mVgHandle);
                break;
            case HbTlvVgHandlePair::VgPaint:
                VGDESTROYPAINT(mTlvVgHandleList[i].mVgHandle);
                break;
            case HbTlvVgHandlePair::VgImage:
                VGDESTROYIMAGE(mTlvVgHandleList[i].mVgHandle);
                break;
            }
        }
    }

	mTlvVgHandleList.clear();
	if (mNvgIconData){
		mNvgIconData->endRead();
    	delete mNvgIconData;
    }
        ASSERT_HANDLE_COUNT()
}

void HbTlvRenderer::dVgSeti()
{
    quint16 type;
    quint16 value;
    type = mNvgIconData->readInt16();
    value = mNvgIconData->readInt16();

    vgSeti((VGParamType)type, value);

    NVG_DEBUGSTMT(logVgSeti((VGParamType)type, value));
}

void HbTlvRenderer::dVgSetf()
{
    quint16 type   = mNvgIconData->readInt16();
    VGfloat value     = mNvgIconData->readReal32();

    vgSetf((VGParamType)type, value);

    NVG_DEBUGSTMT(logVgSetf((VGParamType)type, value));
}

void HbTlvRenderer::dVgSetParameteri()
{
    quint8  handle;
    quint16 type;
    quint16 value;

    handle = mNvgIconData->readInt32();
    type = mNvgIconData->readInt16();
    value = mNvgIconData->readInt16();

    vgSetParameteri(getHandle(handle), type, value);

    NVG_DEBUGSTMT(logVgSetParameteri(getHandle(handle), type, value, handle));
}

void HbTlvRenderer::dVgSetParameterf()
{
    quint32 handle = mNvgIconData->readInt32();
    quint16 type = mNvgIconData->readInt16();
    quint32 value = mNvgIconData->readReal32();

    vgSetParameterf(getHandle(handle), type, value);

    NVG_DEBUGP4("vgSetParameterf(%d, %d, %d)\n", handle, type, value);
}

void HbTlvRenderer::dVgSetParameterfv()
{
    quint32 handle = mNvgIconData->readInt32();
    quint16 type = mNvgIconData->readInt16();
    quint32 countt = mNvgIconData->readInt32();

    float *fArr = new float[countt];
    Q_CHECK_PTR(fArr);
    QScopedArrayPointer<float> value(fArr);

    for (uint i = 0; i < countt; i++ ) {
        value[i] = mNvgIconData->readReal32();
    }

    vgSetParameterfv(getHandle(handle), type, countt, value.data());
    
    NVG_DEBUGSTMT(logVgSetParameterfv(getHandle(handle), type, countt, value.data()));
}

void HbTlvRenderer::dVgSetColor()
{
    quint32 paintvalue = mNvgIconData->readInt32();
    quint32 rgba = mNvgIconData->readInt32();

    vgSetColor(getHandle(paintvalue), rgba);

	NVG_DEBUGP6("vgSetColor(%d, ((%d << 24) | (%d << 16) | (%d << 8) | %d));\n", getHandle(paintvalue),
	 (rgba & 0xFF000000) >> 24, (rgba & 0x00FF0000) >> 16,(rgba & 0x0000FF00) >> 8, (rgba & 0x000000FF));
}

void HbTlvRenderer::dVgSetPaint()
{
    quint32 paintvalue = mNvgIconData->readInt32();
    quint8 value = mNvgIconData->readInt8();

    vgSetPaint(getHandle(paintvalue), value);

    NVG_DEBUGSTMT(logVgSetPaint(getHandle(paintvalue), value, paintvalue));
}

QPoint HbTlvRenderer::getTranslatedPoint(VGfloat *trMatrix, const QPoint &point)
{
    QPoint trPoint;

    trPoint.setX(trMatrix[0] * point.x() + trMatrix[3] * point.y() + trMatrix[6]);
    trPoint.setY(trMatrix[1] * point.x() + trMatrix[4] * point.y() + trMatrix[7]);

    return trPoint;
}

VGfloat HbTlvRenderer::minVal4(VGfloat x1, VGfloat x2, VGfloat x3, VGfloat x4)
{
    VGfloat min = x1;

    if (min > x2) {
        min = x2;
    }
    if (min > x3) {
        min = x3;
    }
    if (min > x4) {
        min = x4;
    }

    return min;
}

VGfloat HbTlvRenderer::maxVal4(VGfloat x1, VGfloat x2, VGfloat x3, VGfloat x4)
{
    VGfloat max = x1;

    if (max < x2) {
        max = x2;
    }
    if (max < x3) {
        max = x3;
    }
    if (max < x4) {
        max = x4;
    }

    return max;
}

void HbTlvRenderer::dVgSetiv()
{
    quint16 type = mNvgIconData->readInt16();
    quint16 count = mNvgIconData->readInt16();
    
    VGint *intArr = new VGint[count];
    Q_CHECK_PTR(intArr);
    QScopedArrayPointer<VGint> value(intArr);

    for (int i = 0; i < count; i++) {
        value[i] = mNvgIconData->readInt32();
    }

    if (type == VG_SCISSOR_RECTS) {
        VGfloat * matrix = mUserMatrix;

        // calculate the rectangle with respect to the transformation applied
        for (int i = 0; i < count; i += 4) {
            int sx = i + 0;
            int sy = i + 1;
            int sw = i + 2;
            int sh = i + 3;

            QPoint leftBottom  = getTranslatedPoint(matrix,
                                                    QPoint(value[sx], value[sy]));
            QPoint leftTop     = getTranslatedPoint(matrix,
                                                    QPoint(value[sx], value[sy] + value[sh]));
            QPoint rightBottom = getTranslatedPoint(matrix,
                                                    QPoint(value[sx] + value[sw], value[sy]));
            QPoint rightTop    = getTranslatedPoint(matrix,
                                                    QPoint(value[sx] + value[sw], value[sy] + value[sh]));


            VGfloat minX = leftBottom.x();
            VGfloat minY = leftBottom.y();
            VGfloat maxX = leftBottom.x();
            VGfloat maxY = leftBottom.y();

            minX = minVal4(leftBottom.x(), leftTop.x(), rightBottom.x(), rightTop.x());
            minY = minVal4(leftBottom.y(), leftTop.y(), rightBottom.y(), rightTop.y());

            maxX = maxVal4(leftBottom.x(), leftTop.x(), rightBottom.x(), rightTop.x());
            maxY = maxVal4(leftBottom.y(), leftTop.y(), rightBottom.y(), rightTop.y());

            VGfloat newW = maxX - minX;
            VGfloat newH = maxY - minY;

            value[sx] = minX;
            value[sy] = minY;
            value[sw] = newW;
            value[sh] = newH;
        }
    }

    vgSetiv((VGParamType)type, count, value.data());

    NVG_DEBUGSTMT(logvgSetXv((VGParamType)type, count, 'i', value.data()));
}

void HbTlvRenderer::dVgLoadMatrix()
{
    VGfloat matrix[9];
    for (int i = 0; i < 9; i++) {
        matrix[i] = mNvgIconData->readReal32();
    }
    
    // keep the caller's matrix as base
    int matrixMode = vgGeti(VG_MATRIX_MODE);

    if (matrixMode == VG_MATRIX_PATH_USER_TO_SURFACE ||
            matrixMode == VG_MATRIX_IMAGE_USER_TO_SURFACE /* need to check?*/) {
        vgLoadMatrix(mUserMatrix);
        vgMultMatrix(matrix);
    } else {
        vgLoadMatrix(matrix);
    }

#ifdef HB_NVG_DEBUG
	int matrixCount = clock() % 1000;
	NVG_DEBUGP2("VGfloat ldMatrix%d[] = {", matrixCount);
    
    NVG_DEBUGP4("    %f, %f, %f, ", matrix[0], matrix[1], matrix[2]);
    NVG_DEBUGP4("    %f, %f, %f, ", matrix[3], matrix[4], matrix[5]);
    NVG_DEBUGP4("    %f, %f, %f)", matrix[6], matrix[7], matrix[8]);
    
	NVG_DEBUGP2("vgLoadMatrix(ldMatrix%d);\n", matrixCount);
#endif
}

void HbTlvRenderer::dVgMultMatrix()
{
    VGfloat matrix[9];
    for (int i = 0; i < 9; i++) {
        matrix[i] = mNvgIconData->readReal32();
    }

    vgMultMatrix(matrix);
    
#ifdef HB_NVG_DEBUG
    int matrixCount = clock() % 1000;
    NVG_DEBUGP2("VGfloat mlMatrix%d[] = {", ++matrixCount);

    NVG_DEBUGP4("    %f, %f, %f, ", matrix[0], matrix[1], matrix[2]);
    NVG_DEBUGP4("    %f, %f, %f, ", matrix[3], matrix[4], matrix[5]);
    NVG_DEBUGP4("    %f, %f, %f)", matrix[6], matrix[7], matrix[8]);
	NVG_DEBUGP2("vgMultMatrix(mlMatrix%d);\n", matrixCount);
#endif
}

void HbTlvRenderer::dVgLoadIdentity()
{
    if (vgGeti(VG_MATRIX_MODE) == (VGint)VG_MATRIX_PATH_USER_TO_SURFACE) {
        vgLoadMatrix(mUserMatrix);     
    }else{
        vgLoadIdentity();
    }
    NVG_DEBUGP1("vgLoadIdentity();\n");
}

void HbTlvRenderer::dVgScale()
{
    VGfloat sx = mNvgIconData->readReal32();
    VGfloat sy = mNvgIconData->readReal32();
    vgScale(sx, sy);

    NVG_DEBUGP3("vgScale(%f, %f)\n", sx, sy);
}

void HbTlvRenderer::dVgTranslate()
{
    VGfloat tx = mNvgIconData->readReal32();
    VGfloat ty = mNvgIconData->readReal32();

    vgTranslate(tx, ty);

    NVG_DEBUGP3("vgTranslate(%f, %f)\n", tx, ty);
}

void HbTlvRenderer::dVgAppendPathData()
{
    quint16         numSegments;
    quint16         coordinatecount;
    
    mNvgIconData->readInt32();
    numSegments = mNvgIconData->readInt16();
    
    VGubyte *pSegArr = new VGubyte[numSegments];
    Q_CHECK_PTR(pSegArr);
    QScopedArrayPointer<VGubyte>pathSegments(pSegArr);

    for (int j = 0; j < numSegments; j++) {
        pathSegments[j] = mNvgIconData->readInt8();
    }

    coordinatecount = mNvgIconData->readInt16();
    
    qint32 *pDataArr = new qint32[coordinatecount];
    Q_CHECK_PTR(pDataArr);
    QScopedArrayPointer<qint32>pathData(pDataArr );
    
    for (int i = 0; i < coordinatecount; i++ ) {
        pathData[i] = mNvgIconData->readInt32();
    }

    vgAppendPathData(mPathHandle, numSegments, pathSegments.data(), pathData.data());

    NVG_DEBUGP3("vgappendPathData((VGPath)%d, (SENGMENTS)%d)\n", mPathHandle, numSegments);
}

void HbTlvRenderer::dVgDrawPath()
{
    quint16 value;

    mNvgIconData->readInt32();
    value = mNvgIconData->readInt16();

    vgDrawPath(mPathHandle, value);
    
    NVG_DEBUGSTMT(logVgDrawPath(mPathHandle, value));
}

void HbTlvRenderer::dVgClearPath()
{
    quint16 value;

    mNvgIconData->readInt32();
    value = mNvgIconData->readInt16();

    vgClearPath(mPathHandle, value);
    NVG_DEBUGP1("vgClearPath()\n");
}

void HbTlvRenderer::dVgCreatePaint()
{
    quint32  encodedHandle = mNvgIconData->readInt32();
    
    VGPaint paintH = VGCREATEPAINT();
    if (paintH == VG_INVALID_HANDLE) {
        throw HbNvgException(openVgErrorToHbNvgError(vgGetError()));
        }

    mTlvVgHandleList.append(HbTlvVgHandlePair(paintH, encodedHandle, HbTlvVgHandlePair::VgPaint));

    NVG_DEBUGP2("%d = vgCreatePaint()\n", getHandle(encodedHandle));
}

void HbTlvRenderer::dVguRect()
{
    float x;
    float y;
    float width;
    float height;

    mNvgIconData->readInt32();

    x   = mNvgIconData->readReal32();
    y   = mNvgIconData->readReal32();

    width   = mNvgIconData->readReal32();
    height  = mNvgIconData->readReal32();

    vguRect(mPathHandle, x, y, width, height);

    NVG_DEBUGP6("vguRect(%d, %f, %f, %f, %f)\n",mPathHandle, x, y, width, height);
}

void HbTlvRenderer::dVguEllipse()
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

    vguEllipse(mPathHandle, cx, cy, width, height);

    NVG_DEBUGP6("vguEllipse( %d ,%f, %f, %f, %f)\n", mPathHandle, cx, cy, width, height);
}

void HbTlvRenderer::dVguRoundRect()
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
    
    //uncomment in qt
    vguRoundRect(mPathHandle, x, y, width, height, arcWidth, arcHeight);

    NVG_DEBUGP8("vguRoundRect(%d %f, %f, %f, %f, %f, %f )\n", mPathHandle, x, y, width, height, arcWidth, arcHeight);   
}

void HbTlvRenderer::dVguLine()
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

    vguLine(mPathHandle, x0, y0, x1, y1);

    NVG_DEBUGP6("vguLine(%d, %f, %f, %f, %f)\n",mPathHandle, x0, y0, x1, y1);
}

void HbTlvRenderer::dVgClear()
{
    float width;
    float height;

    mNvgIconData->readReal32();
    mNvgIconData->readReal32();
    width  = mNvgIconData->readReal32();
    height = mNvgIconData->readReal32();

    if (!mScaled) {
        VGfloat scaleX = mTargetWidth  / width;
        VGfloat scaleY = mTargetHeight / height;
        vgLoadMatrix(mUserMatrix);
        vgTranslate(mTargetWidth / 2, mTargetHeight / 2);
        vgScale(scaleX, scaleY);
        vgTranslate(- width / 2, - height / 2);

        vgGetMatrix(mUserMatrix);
        mScaled = true;
    }

    NVG_DEBUGP1("vgClear()\n");
}

void HbTlvRenderer::dVgSetfv()
{
    int innerpos = 1;
    quint16 type;
    quint16 count;

    type  = mNvgIconData->readInt16();
    count = mNvgIconData->readInt16();

    VGfloat * vgfArr = new VGfloat[count];
    Q_CHECK_PTR(vgfArr);
    QScopedArrayPointer<VGfloat> value(vgfArr);

    for (int i = 0; i < count; i++ ) {
        value[i] = mNvgIconData->readReal32();
    }
    
    innerpos = innerpos + 2 * sizeof(quint16) + count * sizeof(VGfloat);

    vgSetfv((VGParamType)type, count, value.data());
    
    NVG_DEBUGSTMT(logvgSetXv((VGParamType)type, count, 'f', (VGint *)value.data()));
}

void HbTlvRenderer::dVgRotate()
{
    float angle;

    angle = mNvgIconData->readReal32();

    vgRotate(angle);

    NVG_DEBUGP2("vgRotate(%f)\n", angle);
}

void HbTlvRenderer::dVgCreatePath()
{
    quint32 pathFormat;
    quint8  datatype;
    float scale;
    float bias;
    quint32 segmentCapacityHint;
    quint32 coordCapacityHint;
    quint32 capabilities;

    pathFormat = mNvgIconData->readInt32();
    datatype = mNvgIconData->readInt8();

    scale = mNvgIconData->readReal32();
    bias  = mNvgIconData->readReal32();

    segmentCapacityHint = mNvgIconData->readInt32();
    coordCapacityHint = mNvgIconData->readInt32();
    capabilities  = mNvgIconData->readInt32();
    mNvgIconData->readInt32();

    mPathHandle = VGCREATEPATH(pathFormat, (VGPathDatatype)datatype, scale, bias, segmentCapacityHint, coordCapacityHint,
                               (VGbitfield)capabilities);
    if (mPathHandle == VG_INVALID_HANDLE){
        throw HbNvgException(openVgErrorToHbNvgError(vgGetError()));
    	}
    NVG_DEBUGP9("%d = vgCreatePath(%d, (VGPathDatatype)%d, %f, %f, %d, %d,(VGPathDatatype)%d)\n",
    	 mPathHandle, pathFormat, datatype, scale, bias, segmentCapacityHint, coordCapacityHint, capabilities);
}

void HbTlvRenderer::dVgCreateImage()
{
    qint32 format   = (VGImageFormat)mNvgIconData->readInt32();
    int width      = (VGImageFormat)mNvgIconData->readInt32();
    int height     = (VGImageFormat)mNvgIconData->readInt32();
    quint8 aq       = (VGImageFormat)mNvgIconData->readInt8();

    int    encodedHandle = mNvgIconData->readInt32();
    VGImage image = 0;

    if (mPrepareToBindImage) {
        QPoint leftBottom   = getTranslatedPoint(mUserMatrix, QPoint(0, 0));
        QPoint rightBottom  = getTranslatedPoint(mUserMatrix, QPoint(width, 0));
        QPoint leftTop      = getTranslatedPoint(mUserMatrix, QPoint(0, height));
        QPoint rightTop     = getTranslatedPoint(mUserMatrix, QPoint(width, height));

        VGfloat maxX = leftBottom.x();
        VGfloat maxY = leftBottom.y();

        maxX = maxVal4(leftBottom.x(), leftTop.x(), rightBottom.x(), rightTop.x());
        maxY = maxVal4(leftBottom.y(), leftTop.y(), rightBottom.y(), rightTop.y());

        width  = maxX - 0;
        height = maxY - 0;

        mPrepareToBindImage = 0;
    }

    image = VGCREATEIMAGE((VGImageFormat)format,
                          width, height, aq);
	if (image == VG_INVALID_HANDLE) {
            throw HbNvgException(openVgErrorToHbNvgError(vgGetError()));
        }

    mTlvVgHandleList.append(HbTlvVgHandlePair(image, encodedHandle, HbTlvVgHandlePair::VgImage, aq));

    NVG_DEBUGP6("%d = vgCreateImage(%d, %d, %d, %d)\n",getHandle(encodedHandle), format, width, height, (quint32)aq);
}

void HbTlvRenderer::dVgClearImage()
{
    int encodedImageHandle = mNvgIconData->readInt32();
    VGImage imageHandle     = getHandle(encodedImageHandle);

    int x      = mNvgIconData->readInt32();
    int y      = mNvgIconData->readInt32();
    int width  = mNvgIconData->readInt32();
    int height = mNvgIconData->readInt32();
    vgClearImage(imageHandle, x, y, width, height);

    NVG_DEBUGP6("vgClearImage(%d, %d, %d, %d, %d)\n", imageHandle, x, y, width, height);
}

void HbTlvRenderer::dVgDrawImage()
{
    int encodedImageHandle = mNvgIconData->readInt32();
    VGImage imageHandle     = getHandle(encodedImageHandle);

    vgDrawImage(imageHandle);
    NVG_DEBUGP2("vgDrawImage(%d)\n", imageHandle);
}

void HbTlvRenderer::dVgImageSubData()
{
    int encodedImageHandle = mNvgIconData->readInt32();
    VGImage imageHandle     = getHandle(encodedImageHandle);
    int dataLength         = 0;

    int dataStride = mNvgIconData->readInt32();
    int dataFormat = mNvgIconData->readInt32();
    int x          = mNvgIconData->readInt32();
    int y          = mNvgIconData->readInt32();
    int width      = mNvgIconData->readInt32();
    int height     = mNvgIconData->readInt32();

	dataLength      = mNvgIconData->readInt32();
    if (dataLength) {
        quint8* uIntArry = new quint8[dataLength];
        Q_CHECK_PTR(uIntArry);
        QScopedArrayPointer<quint8> data(uIntArry);
       	mNvgIconData->read(data.data(), dataLength);
		quint8 * dataPtr;
        if (dataStride < 0) {
            dataPtr = data.data() + (height - 1) * (-dataStride);
        } else {
            dataPtr = data.data();
        }
        vgImageSubData(imageHandle, dataPtr, dataStride, (VGImageFormat)dataFormat, x, y, width, height);

		NVG_DEBUGSTMT(logvgImageSubData(imageHandle, dataPtr, dataLength, dataStride,
		        (VGImageFormat)dataFormat, x, y, width, height));
        }
    else {
        throw HbNvgException(HbNvgEngine::NvgErrCorrupt);
    }
}

void HbTlvRenderer::dVgGetPixels()
{
    NVG_DEBUGP1("vgGetPixels()\n");
}

void HbTlvRenderer::dVgDestroyImage()
{
    VGImage imageHandle = removeHandle(mNvgIconData->readInt32());
    VGDESTROYIMAGE(imageHandle);
    NVG_DEBUGP2("vgDestroyImage(%d)\n", imageHandle);
}

void HbTlvRenderer::dVgDestroyPaint()
{
    int paint = mNvgIconData->readInt32();
	VGDESTROYPAINT(removeHandle(paint));
    NVG_DEBUGP1("vgDestroyPaint()\n");
}

void HbTlvRenderer::dVgDestroyPath()
{
    // only one path handle is maintained, which is a member variable
    NVG_DEBUGP2("vgDestroyPath(%d);\n", mPathHandle);
}

void HbTlvRenderer::dVgFlush()
{
    //no need to implement?
    NVG_DEBUGP1("vgFlush()\n");
}

void HbTlvRenderer::execute(int index)
{
    if (0 <= index && index < VgFlush + 1) {
        (this->*(vgapi[index]))();
    } else {
        throw HbNvgException(HbNvgEngine::NvgErrCorrupt);
    }
}

void HbTlvRenderer::execute()
{
    NVG_DEBUGP1("TLV rendering starts");
    while (!mNvgIconData->eof()) {
        execute(mNvgIconData->readInt8());
    }

    NVG_DEBUGP1("TLV rendering ends");
}

void HbTlvRenderer::getHandlePair(int index, HbTlvVgHandlePair & pair)
{
    int handleListCount    = mTlvVgHandleList.count();

    for (int i = 0; i < handleListCount; i++) {
        if (mTlvVgHandleList[i].mTlvHandle == index) {
            pair = mTlvVgHandleList[i];
            break;
        }
    }
}

VGHandle HbTlvRenderer::removeHandle(int index)
{
	int handleListCount    = mTlvVgHandleList.count();
	VGHandle handle         = 0;
	
	for (int i = 0; i < handleListCount; i++)
	    {
	    if (mTlvVgHandleList[i].mTlvHandle == index)
	        {
	        handle = mTlvVgHandleList[i].mVgHandle;
	        mTlvVgHandleList.removeAt(i);
	        break;
	        }
	    }
	
	if (!handle) {
        throw HbNvgException(HbNvgEngine::NvgErrCorrupt);
	    }
	return handle;
}

VGHandle HbTlvRenderer::getHandle(int index)
{
    int handleListCount    = mTlvVgHandleList.count();
    VGHandle handle = 0;

    for (int i = 0; i < handleListCount; i++) {
        if (mTlvVgHandleList[i].mTlvHandle == index) {
            handle = mTlvVgHandleList[i].mVgHandle;
            break;
        }
    }
    if (!handle) {
        throw HbNvgException(HbNvgEngine::NvgErrCorrupt);
        }
    return handle;
}

void HbTlvRenderer::setHandle(int index, VGHandle handle)
{
    int handleListCount    = mTlvVgHandleList.count();

    for (int i = 0; i < handleListCount; i++) {
        if (mTlvVgHandleList[i].mTlvHandle == index) {
            mTlvVgHandleList[i].mVgHandle = handle;
            break;
        }
    }
}

void HbTlvRenderer::dVgPrepareToBindImage()
{
    mPrepareToBindImage = 1;
    NVG_DEBUGP1("vgPrepareToBindImage()\n");
}

void HbTlvRenderer::dVgBindImage()
{
    mPrepareToBindImage = 0;

    int imageHandle = mNvgIconData->readInt32();

    if (mVgImageBinder) {
		mVgImageBinder->bindClientBuffer((VGHandle)getHandle(imageHandle));
		}
    NVG_DEBUGP2("vgBindImage(%d)\n", imageHandle);
}

void HbTlvRenderer::dVgUnBindImage()
{
    if (mVgImageBinder) {
        mVgImageBinder->unBindClientBuffer();
    }
    NVG_DEBUGP1("vgUnBindImage()\n");
}

#ifdef HB_NVG_DEBUG
void HbTlvRenderer::logVgSetf(VGParamType type, VGfloat value)
{

    QByteArray logptr;

    logptr.append("vgSetf(");
    switch (type) {
    case VG_STROKE_LINE_WIDTH: {
        logptr.append("VG_STROKE_LINE_WIDTH");
    }
    break;
    case VG_STROKE_MITER_LIMIT: {
        logptr.append("VG_STROKE_MITER_LIMIT");
    }
    break;
    case VG_STROKE_DASH_PHASE: {
        logptr.append("VG_STROKE_DASH_PHASE");
    }
    break;

    case VG_MATRIX_MODE: {
        logptr.append("VG_MATRIX_MODE");
    }
    break;
    case VG_FILL_RULE: {
        logptr.append("VG_FILL_RULE");
    }
    break;
    case VG_IMAGE_QUALITY: {
        logptr.append("VG_IMAGE_QUALITY");
    }
    break;
    case VG_IMAGE_MODE: {
        logptr.append("VG_IMAGE_MODE");
    }
    break;
    case VG_RENDERING_QUALITY: {
        logptr.append("VG_RENDERING_QUALITY");
    }
    break;
    case VG_BLEND_MODE: {
        logptr.append("VG_BLEND_MODE");
    }
    break;
    case VG_MASKING: {
        logptr.append("VG_MASKING");
    }
    break;
    case VG_SCISSORING: {
        logptr.append("VG_SCISSORING");
    }
    break;
    case VG_PIXEL_LAYOUT: {
        logptr.append("VG_PIXEL_LAYOUT");
    }
    break;
    case VG_FILTER_FORMAT_LINEAR: {
        logptr.append("VG_FILTER_FORMAT_LINEAR");
    }
    break;
    case VG_FILTER_FORMAT_PREMULTIPLIED: {
        logptr.append("VG_FILTER_FORMAT_PREMULTIPLIED");
    }
    break;
    case VG_FILTER_CHANNEL_MASK: {
        logptr.append("VG_FILTER_CHANNEL_MASK");
    }
    break;
    case VG_STROKE_CAP_STYLE: {
        logptr.append("VG_STROKE_CAP_STYLE");
    }
    break;
    case VG_STROKE_JOIN_STYLE: {
        logptr.append("VG_STROKE_JOIN_STYLE");
    }
    break;
    case VG_STROKE_DASH_PHASE_RESET: {
        logptr.append("VG_STROKE_DASH_PHASE_RESET");
    }
    break;
    /* Implementation limits (read-only) */
    case VG_SCREEN_LAYOUT: {
        logptr.append("VG_SCREEN_LAYOUT");
    }
    break;
    case VG_MAX_SCISSOR_RECTS: {
        logptr.append("VG_MAX_SCISSOR_RECTS");
    }
    break;
    case VG_MAX_DASH_COUNT: {
        logptr.append("VG_MAX_DASH_COUNT");
    }
    break;
    case VG_MAX_KERNEL_SIZE: {
        logptr.append("VG_MAX_KERNEL_SIZE");
    }
    break;
    case VG_MAX_SEPARABLE_KERNEL_SIZE: {
        logptr.append("VG_MAX_SEPARABLE_KERNEL_SIZE");
    }
    break;
    case VG_MAX_COLOR_RAMP_STOPS: {
        logptr.append("VG_MAX_COLOR_RAMP_STOPS");
    }
    break;
    case VG_MAX_IMAGE_WIDTH: {
        logptr.append("VG_MAX_IMAGE_WIDTH");
    }
    break;
    case VG_MAX_IMAGE_HEIGHT: {
        logptr.append("VG_MAX_IMAGE_HEIGHT");
    }
    break;
    case VG_MAX_IMAGE_PIXELS: {
        logptr.append("VG_MAX_IMAGE_PIXELS");
    }
    break;
    case VG_MAX_IMAGE_BYTES: {
        logptr.append("VG_MAX_IMAGE_BYTES");
    }
    break;
    case VG_MAX_FLOAT: {
        logptr.append("VG_MAX_FLOAT");
    }
    break;
    case VG_MAX_GAUSSIAN_STD_DEVIATION: {
        logptr.append("VG_MAX_GAUSSIAN_STD_DEVIATION");
    }
    break;

    default: {
        logptr.append("INVALID PARAMTYPE");
    }
    break;
    };

    logptr.append(", ");
    logptr.setNum((int)value,10);
    logptr.append(")");
    qDebug() << logptr.data();
    return;
}

void HbTlvRenderer::logVgSeti(VGParamType type, VGint value)
{
    QByteArray  logptr;
    logptr.append("vgSeti(");

    switch (type) {
    case VG_STROKE_LINE_WIDTH: {
        logptr.append("VG_STROKE_LINE_WIDTH");
    }
    break;
    case VG_STROKE_MITER_LIMIT: {
        logptr.append("VG_STROKE_MITER_LIMIT");
    }
    break;
    case VG_STROKE_DASH_PHASE: {
        logptr.append("VG_STROKE_DASH_PHASE");
    }
    break;

    case VG_MATRIX_MODE: {
        logptr.append("VG_MATRIX_MODE");
    }
    break;
    case VG_FILL_RULE: {
        logptr.append("VG_FILL_RULE");
    }
    break;
    case VG_IMAGE_QUALITY: {
        logptr.append("VG_IMAGE_QUALITY");
    }
    break;
    case VG_IMAGE_MODE: {
        logptr.append("VG_IMAGE_MODE");
    }
    break;
    case VG_RENDERING_QUALITY: {
        logptr.append("VG_RENDERING_QUALITY");
    }
    break;
    case VG_BLEND_MODE: {
        logptr.append("VG_BLEND_MODE");
    }
    break;
    case VG_MASKING: {
        logptr.append("VG_MASKING");
    }
    break;
    case VG_SCISSORING: {
        logptr.append("VG_SCISSORING");
    }
    break;
    case VG_PIXEL_LAYOUT: {
        logptr.append("VG_PIXEL_LAYOUT");
    }
    break;
    case VG_FILTER_FORMAT_LINEAR: {
        logptr.append("VG_FILTER_FORMAT_LINEAR");
    }
    break;
    case VG_FILTER_FORMAT_PREMULTIPLIED: {
        logptr.append("VG_FILTER_FORMAT_PREMULTIPLIED");
    }
    break;
    case VG_FILTER_CHANNEL_MASK: {
        logptr.append("VG_FILTER_CHANNEL_MASK");
    }
    break;
    case VG_STROKE_CAP_STYLE: {
        logptr.append("VG_STROKE_CAP_STYLE");
    }
    break;
    case VG_STROKE_JOIN_STYLE: {
        logptr.append("VG_STROKE_JOIN_STYLE");
    }
    break;
    case VG_STROKE_DASH_PHASE_RESET: {
        logptr.append("VG_STROKE_DASH_PHASE_RESET");
    }
    break;
    /* Implementation limits (read-only) */
    case VG_SCREEN_LAYOUT: {
        logptr.append("VG_SCREEN_LAYOUT");
    }
    break;
    case VG_MAX_SCISSOR_RECTS: {
        logptr.append("VG_MAX_SCISSOR_RECTS");
    }
    break;
    case VG_MAX_DASH_COUNT: {
        logptr.append("VG_MAX_DASH_COUNT");
    }
    break;
    case VG_MAX_KERNEL_SIZE: {
        logptr.append("VG_MAX_KERNEL_SIZE");
    }
    break;
    case VG_MAX_SEPARABLE_KERNEL_SIZE: {
        logptr.append("VG_MAX_SEPARABLE_KERNEL_SIZE");
    }
    break;
    case VG_MAX_COLOR_RAMP_STOPS: {
        logptr.append("VG_MAX_COLOR_RAMP_STOPS");
    }
    break;
    case VG_MAX_IMAGE_WIDTH: {
        logptr.append("VG_MAX_IMAGE_WIDTH");
    }
    break;
    case VG_MAX_IMAGE_HEIGHT: {
        logptr.append("VG_MAX_IMAGE_HEIGHT");
    }
    break;
    case VG_MAX_IMAGE_PIXELS: {
        logptr.append("VG_MAX_IMAGE_PIXELS");
    }
    break;
    case VG_MAX_IMAGE_BYTES: {
        logptr.append("VG_MAX_IMAGE_BYTES");
    }
    break;
    case VG_MAX_FLOAT: {
        logptr.append("VG_MAX_FLOAT");
    }
    break;
    case VG_MAX_GAUSSIAN_STD_DEVIATION: {
        logptr.append("VG_MAX_GAUSSIAN_STD_DEVIATION");
    }
    break;

    default: {
        logptr.append("INVALID PARAMTYPE");
    }
    break;
    };
    
    logptr.append(", ");
    switch (value) {
    case VG_RENDERING_QUALITY_NONANTIALIASED: {
        logptr.append("VG_RENDERING_QUALITY_NONANTIALIASED");
    }
    break;
    case VG_RENDERING_QUALITY_FASTER: {
        logptr.append("VG_RENDERING_QUALITY_FASTER");
    }
    break;
    case VG_RENDERING_QUALITY_BETTER: {
        logptr.append("VG_RENDERING_QUALITY_BETTER");
    }
    break;
    case VG_MATRIX_PATH_USER_TO_SURFACE: {
        logptr.append("VG_MATRIX_PATH_USER_TO_SURFACE");
    }
    break;
    case VG_MATRIX_IMAGE_USER_TO_SURFACE: {
        logptr.append("VG_MATRIX_IMAGE_USER_TO_SURFACE");
    }
    break;
    case VG_MATRIX_FILL_PAINT_TO_USER : {
        logptr.append("VG_MATRIX_FILL_PAINT_TO_USER");
    }
    break;
    case VG_MATRIX_STROKE_PAINT_TO_USER: {
        logptr.append("VG_MATRIX_STROKE_PAINT_TO_USER");
    }
    break;
    case VG_CAP_BUTT: {
        logptr.append("VG_CAP_BUTT");
    }
    break;
    case VG_CAP_ROUND: {
        logptr.append("VG_CAP_ROUND");
    }
    break;
    case VG_CAP_SQUARE: {
        logptr.append("VG_CAP_SQUARE");
    }
    break;
    case VG_BLEND_SRC: {
        logptr.append("VG_BLEND_SRC");
    }
    break;
    case VG_BLEND_SRC_OVER: {
        logptr.append("VG_BLEND_SRC_OVER");
    }
    break;
    case VG_BLEND_DST_OVER: {
        logptr.append("VG_BLEND_DST_OVER");
    }
    break;
    case VG_BLEND_SRC_IN: {
        logptr.append("VG_BLEND_SRC_IN");
    }
    break;
    case VG_BLEND_DST_IN: {
        logptr.append("VG_BLEND_DST_IN");
    }
    break;
    case VG_BLEND_MULTIPLY: {
        logptr.append("VG_BLEND_MULTIPLY");
    }
    break;
    case VG_BLEND_SCREEN: {
        logptr.append("VG_BLEND_SCREEN");
    }
    break;
    case VG_BLEND_DARKEN: {
        logptr.append("VG_BLEND_DARKEN");
    }
    break;
    case VG_BLEND_LIGHTEN: {
        logptr.append("VG_BLEND_LIGHTEN");
    }
    break;
    case VG_BLEND_ADDITIVE: {
        logptr.append("VG_BLEND_ADDITIVE");
    }
    break;
    case VG_IMAGE_QUALITY_NONANTIALIASED: {
        logptr.append("1");
    }
    break;
    case VG_IMAGE_QUALITY_FASTER: {
        logptr.append("VG_IMAGE_QUALITY_FASTER");
    }
    break;
    case VG_IMAGE_QUALITY_BETTER: {
        logptr.append("VG_IMAGE_QUALITY_BETTER");
    }
    break;
    case VG_FALSE: {
        logptr.append("VG_FALSE");
    }
    break;
    case VG_RED: {
        logptr.append("VG_RED");
    }
    break;

    case VG_DRAW_IMAGE_NORMAL: {
        logptr.append("VG_DRAW_IMAGE_NORMAL");
    }
    break;
    case VG_DRAW_IMAGE_MULTIPLY: {
        logptr.append("VG_DRAW_IMAGE_MULTIPLY");
    }
    break;
    case VG_DRAW_IMAGE_STENCIL: {
        logptr.append("VG_DRAW_IMAGE_STENCIL");
    }
    break;
    case VG_JOIN_MITER: {
        logptr.append("VG_JOIN_MITER");
    }
    break;
    case VG_JOIN_ROUND: {
        logptr.append("VG_JOIN_ROUND");
    }
    break;
    case VG_JOIN_BEVEL: {
        logptr.append("VG_JOIN_BEVEL");
    }
    case VG_EVEN_ODD:
        logptr.append("VG_EVEN_ODD");
        break;
    case VG_NON_ZERO:
        logptr.append("VG_NON_ZERO");
        break;
    default: {
        logptr.append(value);
    }
    break;
    };

    logptr.append(")");
    qDebug() << logptr.data();
    return;
}

void HbTlvRenderer::logvgSetXv(VGParamType type, VGint count, qint8 vtype, VGint* /*data*/)
{
    QByteArray logptr;
    logptr.append("vgSet");
    logptr.append(vtype);
    logptr.append("v(");

    switch (type) {
    case VG_STROKE_DASH_PATTERN:
        logptr.append("VG_STROKE_DASH_PATTERN");
        break;
    case VG_SCISSOR_RECTS:
        logptr.append("VG_SCISSOR_RECTS");
        break;
    case VG_CLEAR_COLOR:
        logptr.append("VG_CLEAR_COLOR");
        break;
    case VG_STROKE_LINE_WIDTH: {
        logptr.append("VG_STROKE_LINE_WIDTH");
    }
    break;
    case VG_STROKE_MITER_LIMIT: {
        logptr.append("VG_STROKE_MITER_LIMIT");
    }
    break;
    case VG_STROKE_DASH_PHASE: {
        logptr.append("VG_STROKE_DASH_PHASE");
    }
    break;
    case VG_MATRIX_MODE: {
        logptr.append("VG_MATRIX_MODE");
    }
    break;
    case VG_FILL_RULE: {
        logptr.append("VG_FILL_RULE");
    }
    break;
    case VG_IMAGE_QUALITY: {
        logptr.append("VG_IMAGE_QUALITY");
    }
    break;
    case VG_IMAGE_MODE: {
        logptr.append("VG_IMAGE_MODE");
    }
    break;
    case VG_RENDERING_QUALITY: {
        logptr.append("VG_RENDERING_QUALITY");
    }
    break;
    case VG_BLEND_MODE: {
        logptr.append("VG_BLEND_MODE");
    }
    break;
    case VG_MASKING: {
        logptr.append("VG_MASKING");
    }
    break;
    case VG_SCISSORING: {
        logptr.append("VG_SCISSORING");
    }
    break;
    case VG_PIXEL_LAYOUT: {
        logptr.append("VG_PIXEL_LAYOUT");
    }
    break;
    case VG_FILTER_FORMAT_LINEAR: {
        logptr.append("VG_FILTER_FORMAT_LINEAR");
    }
    break;
    case VG_FILTER_FORMAT_PREMULTIPLIED: {
        logptr.append("VG_FILTER_FORMAT_PREMULTIPLIED");
    }
    break;
    case VG_FILTER_CHANNEL_MASK: {
        logptr.append("VG_FILTER_CHANNEL_MASK");
    }
    break;
    case VG_STROKE_CAP_STYLE: {
        logptr.append("VG_STROKE_CAP_STYLE");
    }
    break;
    case VG_STROKE_JOIN_STYLE: {
        logptr.append("VG_STROKE_JOIN_STYLE");
    }
    break;
    case VG_STROKE_DASH_PHASE_RESET: {
        logptr.append("VG_STROKE_DASH_PHASE_RESET");
    }
    break;
    case VG_SCREEN_LAYOUT: {
        logptr.append("VG_SCREEN_LAYOUT");
    }
    break;
    case VG_MAX_SCISSOR_RECTS: {
        logptr.append("VG_MAX_SCISSOR_RECTS");
    }
    break;
    case VG_MAX_DASH_COUNT: {
        logptr.append("VG_MAX_DASH_COUNT");
    }
    break;
    case VG_MAX_KERNEL_SIZE: {
        logptr.append("VG_MAX_KERNEL_SIZE");
    }
    break;
    case VG_MAX_SEPARABLE_KERNEL_SIZE: {
        logptr.append("VG_MAX_SEPARABLE_KERNEL_SIZE");
    }
    break;
    case VG_MAX_COLOR_RAMP_STOPS: {
        logptr.append("VG_MAX_COLOR_RAMP_STOPS");
    }
    break;
    case VG_MAX_IMAGE_WIDTH: {
        logptr.append("VG_MAX_IMAGE_WIDTH");
    }
    break;
    case VG_MAX_IMAGE_HEIGHT: {
        logptr.append("VG_MAX_IMAGE_HEIGHT");
    }
    break;
    case VG_MAX_IMAGE_PIXELS: {
        logptr.append("VG_MAX_IMAGE_PIXELS");
    }
    break;
    case VG_MAX_IMAGE_BYTES: {
        logptr.append("VG_MAX_IMAGE_BYTES");
    }
    break;
    case VG_MAX_FLOAT: {
        logptr.append("VG_MAX_FLOAT");
    }
    break;
    case VG_MAX_GAUSSIAN_STD_DEVIATION: {
        logptr.append("VG_MAX_GAUSSIAN_STD_DEVIATION");
    }
    break;
    default: {
        logptr.append("INVALID PARAMTYPE");
    }
    break;
    };

    logptr.append(", ");
    logptr.setNum((int)count,(int)10);
    logptr.append(")");
    qDebug() << logptr.data();
    return;
}

void HbTlvRenderer::logVgSetParameteri(VGHandle handle, VGint paramType, VGint value, int /*lpvalue*/)
{
    QByteArray logptr;
    logptr.append("vgsetparameteri(");
    logptr.append(handle);
    logptr.append(", ");

    switch (paramType) {
    case VG_PAINT_TYPE: {
        logptr.append("VG_STROKE_LINE_WIDTH");
    }
    break;
    case VG_PAINT_COLOR: {
        logptr.append("VG_STROKE_LINE_WIDTH");
    }
    break;
    case VG_PAINT_COLOR_RAMP_SPREAD_MODE: {
        logptr.append("VG_STROKE_LINE_WIDTH");
    }
    break;
    case VG_PAINT_COLOR_RAMP_PREMULTIPLIED: {
        logptr.append("VG_STROKE_LINE_WIDTH");
    }
    break;
    case VG_PAINT_COLOR_RAMP_STOPS: {
        logptr.append("VG_STROKE_LINE_WIDTH");
    }
    break;

    /* Linear gradient paint parameters */
    case VG_PAINT_LINEAR_GRADIENT: {
        logptr.append("VG_STROKE_LINE_WIDTH");
    }
    break;
    /* Radial gradient paint parameters */
    case VG_PAINT_RADIAL_GRADIENT: {
        logptr.append("VG_STROKE_LINE_WIDTH");
    }
    break;
    /* Pattern paint parameters */
    case VG_PAINT_PATTERN_TILING_MODE: {
        logptr.append("VG_STROKE_LINE_WIDTH");
    }
    break;
    default: {
        logptr.append(paramType);
    }
    break;
    };
    logptr.append(", ");

    switch (value) {
    case VG_PAINT_TYPE_COLOR: {
        logptr.append("VG_STROKE_LINE_WIDTH");
    }
    break;
    case VG_PAINT_TYPE_LINEAR_GRADIENT: {
        logptr.append("VG_STROKE_LINE_WIDTH");
    }
    break;
    case VG_PAINT_TYPE_RADIAL_GRADIENT: {
        logptr.append("VG_STROKE_LINE_WIDTH");
    }
    break;
    case VG_PAINT_TYPE_PATTERN: {
        logptr.append("VG_STROKE_LINE_WIDTH");
    }
    break;
    default: {
        logptr.setNum((int)value,10);
    }
    break;
    };
    logptr.append(")");
    qDebug() << logptr.data();
    return;
}

void HbTlvRenderer::logVgSetPaint(VGPaint paint, VGbitfield paintModes, int /*Lpvalue*/)
{
    QByteArray logptr;
    logptr.append("vgSetPaint(");
    logptr.append(paint);
    logptr.append(", ");

    switch (paintModes) {
    case VG_STROKE_PATH: {
        logptr.append("VG_STROKE_PATH");
    }
    break;
    case VG_FILL_PATH: {
        logptr.append("VG_FILL_PATH");
    }
    break;
    default: {
        logptr.setNum((int)paintModes,10);
    }
    break;
    };
    logptr.append(")");
    qDebug() << logptr.data();
    return;
}

void HbTlvRenderer::logVgDrawPath(VGuint path, VGbitfield paintModes)
{
    QByteArray logptr;
    logptr.append("vgDrawPath(");
    logptr.setNum((int)path,10);
    logptr.append(", ");
    switch (paintModes) {
    case VG_STROKE_PATH: {
        logptr.append("VG_STROKE_PATH");
    }
    break;
    case VG_FILL_PATH: {
        logptr.append("VG_FILL_PATH");
    }
    break;
    case VG_FILL_PATH |  VG_STROKE_PATH: {
        logptr.append("FILL & STROKE");
    }
    break;
    default: {
        logptr.setNum((int)paintModes,10);
    }
    break;
    };
    logptr.append(")");
    qDebug() << logptr.data();
    return;
}

void HbTlvRenderer::logVgSetParameterfv(VGPaint handle, VGint paramtype, int count, float* handlenum)
{
    QByteArray logptr;
    logptr.append("vgSetParameterfv(");
    logptr.setNum((int)handle,10);
    logptr.append(", ");
    logptr.setNum((int)handlenum,10);
    logptr.append(", ");
    switch (paramtype) {
    case VG_PAINT_TYPE: {
        logptr.append("VG_PAINT_TYPE");
    }
    break;
    case VG_PAINT_COLOR: {
        logptr.append("VG_PAINT_COLOR");
    }
    break;
    case VG_PAINT_COLOR_RAMP_SPREAD_MODE: {
        logptr.append("VG_PAINT_COLOR_RAMP_SPREAD_MODE");
    }
    break;
    case VG_PAINT_COLOR_RAMP_PREMULTIPLIED: {
        logptr.append("VG_PAINT_COLOR_RAMP_PREMULTIPLIED");
    }
    break;
    case VG_PAINT_COLOR_RAMP_STOPS: {
        logptr.append("VG_PAINT_COLOR_RAMP_STOPS");
    }
    break;
    case VG_PAINT_LINEAR_GRADIENT: {
        logptr.append("VG_PAINT_LINEAR_GRADIENT");
    }
    break;
    case VG_PAINT_RADIAL_GRADIENT: {
        logptr.append("VG_PAINT_RADIAL_GRADIENT");
    }
    break;
    case VG_PAINT_PATTERN_TILING_MODE: {
        logptr.append("VG_PAINT_PATTERN_TILING_MODE");
    }
    break;
    default: {
        logptr.setNum((int)paramtype,10);
    }
    break;
    };
    logptr.append(", ");
    logptr.setNum((int)count,10);
    logptr.append(")");
    qDebug() << logptr.data();
    return;
}

void HbTlvRenderer::logvgImageSubData(VGImage imageHandle, const void* /* dataPtr*/, int dataLength, VGint dataStride,
                VGImageFormat dataFormat, VGint x, VGint y, VGint width, VGint height)
    {
    QByteArray logPtr;
    logPtr.append("vgImageSubData(");
    logPtr.append(imageHandle);
    logPtr.append(",");
    logPtr.append(dataLength);
    logPtr.append(dataStride);
    logPtr.append(dataFormat);
    logPtr.append(x);
    logPtr.append(y);
    logPtr.append(width);
    logPtr.append(height);
    logPtr.append(")");
    qDebug() << logPtr.data();
    }
#endif
    


