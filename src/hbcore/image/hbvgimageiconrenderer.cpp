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

#include "hbvgimageiconrenderer_p.h"
#include "hbeglstate_p.h"
#include "hbiconimpl_p.h"

#include <QtCore/qvarlengtharray.h>

HbVgImageIconRenderer::HbVgImageIconRenderer(VGImage img, const QSize & size, HbIconImpl * impl)
        : vgImage(img),
        iconMode(QIcon::Normal),
        specialCaseApplied(false),
        rendersize(size),
        addedToStates(false),
        opacityPaint(VG_INVALID_HANDLE),
        lastOpacity(1.0),
        iconImpl(impl)
{
    eglStates = HbEglStates::global();
    eglStates->ref();
}

HbVgImageIconRenderer::~HbVgImageIconRenderer()
{
    if (vgImage) {
        eglStates->removeVGImage(&vgImage);
    }
    eglStates->deref(eglStates);
}

void HbVgImageIconRenderer::applyIconProperties()
{
    if (!specialCaseApplied) {
        if (iconMode == QIcon::Disabled) {
            VGImage modifiedVgImage = vgCreateImage((VGImageFormat)  VG_sARGB_8888_PRE,
                                                    rendersize.width(), rendersize.height(), VG_IMAGE_QUALITY_NONANTIALIASED);

            VGImageFormat imageFormat =
                (VGImageFormat)vgGetParameteri(vgImage, VG_IMAGE_FORMAT);

            VGfloat matrix[20] = {0.1700, 0.1700, 0.1700, 0,
                                  0.5721, 0.5721, 0.5721, 0,
                                  0.0577, 0.0577, 0.0577, 0,
                                  0, 0, 0, 1,
                                  0, 0, 0, 0
                                 };

            vgColorMatrix(modifiedVgImage, vgImage, matrix);
            vgDestroyImage(vgImage);
            vgImage = modifiedVgImage;
        }

        if (iconColor.isValid() && iconMode != QIcon::Disabled) {
            VGImage resultImage = vgCreateImage((VGImageFormat)  VG_sARGB_8888_PRE,
                                                rendersize.width(), rendersize.height(), VG_IMAGE_QUALITY_NONANTIALIASED);

            VGfloat matrix[20] = {0, 0, 0, 0,
                                  0, 0, 0, 0,
                                  0, 0, 0, 0,
                                  0, 0, 0, 1,
                                  (VGfloat)iconColor.red() / 255.0, (VGfloat)iconColor.green() / 255.0, (VGfloat)iconColor.blue() / 255.0, 0
                                 };
            vgColorMatrix(resultImage, vgImage, matrix);
            vgDestroyImage(vgImage);
            vgImage = resultImage;
        }
        specialCaseApplied = true;
    }
}

bool HbVgImageIconRenderer::draw(QPainter * painter, const QPointF & topLeft, const QPainterPath & clipPath)
{
    if ((iconColor.isValid()) || (iconMode != QIcon::Normal)) {
        applyIconProperties();
    }

    if (vgImage == VG_INVALID_HANDLE) {
        vgImage = vgImageCreator(iconImpl , painter);
        addedToStates = false;
    }

    if (!addedToStates && vgImage != VG_INVALID_HANDLE) {
        eglStates->addVGImage(&vgImage);
        addedToStates = true;
    }

    if (vgImage != VG_INVALID_HANDLE) {

        if (!clipPath.isEmpty()) {
            painter->beginNativePainting();
        }

        VGint imageMode      = vgGeti(VG_IMAGE_MODE);
        VGint matrixMode     = vgGeti(VG_MATRIX_MODE);
        VGPaint oldFillPaint = VG_INVALID_HANDLE;
        VGPaint oldStrkPaint = VG_INVALID_HANDLE;
        VGint   blendMode    = 0;

        updatePainterTransformation(painter, topLeft);

        qreal opacity = painter->opacity();

        if (opacity != lastOpacity || iconMode == QIcon::Selected) {
            lastOpacity = opacity;
            if (opacityPaint == VG_INVALID_HANDLE) {
                opacityPaint = vgCreatePaint();
            }
            if (opacity != 1.0) {
                VGfloat opaquePaint[] = {1.0f, 1.0f, 1.0f, opacity};

                if (iconMode == QIcon::Selected) {
                    opaquePaint[0] = 0.54901f;
                    opaquePaint[1] = 0.54901f;
                    opaquePaint[2] = 0.72156f;
                    opaquePaint[3] = 0.81f * opaquePaint[3];
                }

                if (opacityPaint != VG_INVALID_HANDLE) {
                    vgSetParameteri(opacityPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
                    vgSetParameterfv(opacityPaint, VG_PAINT_COLOR, 4, opaquePaint);
                }
            }
        }

        if ((iconMode == QIcon::Selected || opacity != 1.0) && opacityPaint != VG_INVALID_HANDLE) {
            oldFillPaint = vgGetPaint(VG_FILL_PATH);
            oldStrkPaint = vgGetPaint(VG_STROKE_PATH);
            blendMode = vgGeti(VG_BLEND_MODE);
            vgSeti(VG_BLEND_MODE, VG_BLEND_SRC_OVER);
            vgSetPaint(opacityPaint, VG_FILL_PATH | VG_STROKE_PATH);
        }

        if (opacity == 1.0) {
            vgSeti(VG_IMAGE_MODE, VG_DRAW_IMAGE_NORMAL);
        } else {
            vgSeti(VG_IMAGE_MODE, VG_DRAW_IMAGE_MULTIPLY);
        }

        VGint prevMask = 0;
        VGPath vgpath = VG_INVALID_HANDLE;

        if (!clipPath.isEmpty()) {

            QPaintDevice *pdev = painter->paintEngine()->paintDevice();
            int width = pdev->width();
            int height = pdev->height();

            vgMask(VG_INVALID_HANDLE, VG_CLEAR_MASK, 0, 0, width, height);

            vgpath = painterPathToVGPath(clipPath);
            vgRenderToMask(vgpath, VG_FILL_PATH, VG_UNION_MASK);

            prevMask = vgGeti(VG_MASKING);
            vgSeti(VG_MASKING, VG_TRUE);
        }

        vgDrawImage(vgImage);

        if (!clipPath.isEmpty()) {
            vgSeti(VG_MASKING, VG_FALSE);
        }

        vgSeti(VG_MATRIX_MODE, matrixMode);
        vgSeti(VG_IMAGE_MODE, imageMode);

        if (oldFillPaint) {
            vgSetPaint(oldFillPaint, VG_FILL_PATH);
        }
        if (oldStrkPaint) {
            vgSetPaint(oldStrkPaint, VG_STROKE_PATH);
        }

        if (blendMode) {
            vgSeti(VG_BLEND_MODE, blendMode);
        }

        if (prevMask) {
            vgSeti(VG_MASKING, prevMask);
        }

        if (!clipPath.isEmpty()) {
            vgDestroyPath(vgpath);
            vgpath = VG_INVALID_HANDLE;
            painter->endNativePainting();
        }
        return true;
    }

    return false;
}


void HbVgImageIconRenderer::updatePainterTransformation(QPainter * painter, const QPointF & pos)
{
    VGfloat devh = painter->device()->height() - 1;
    QTransform viewport(1.0f, 0.0f, 0.0f,
                        0.0f, -1.0f, 0.0f,
                        0.5f, devh + 0.5f, 1.0f);
    QTransform imageTransform = painter->transform() * viewport;
    imageTransform.translate(pos.x(), pos.y());

    VGfloat mat[9];
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
    mat[0] = imageTransform.m11();
    mat[1] = imageTransform.m12();
    mat[2] = imageTransform.m13();
    mat[3] = imageTransform.m21();
    mat[4] = imageTransform.m22();
    mat[5] = imageTransform.m23();
    mat[6] = imageTransform.m31();
    mat[7] = imageTransform.m32();
    mat[8] = imageTransform.m33();
    vgLoadMatrix(mat);
}

VGPath HbVgImageIconRenderer::painterPathToVGPath(const QPainterPath& path)
{
    int count = path.elementCount();

    // Creating vgpath is needed else clipling is failing,
    // vgpaintengine also doing the same way,
    // optimising by storing the path as member was tried but not giving expected output
    VGPath vgpath = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                                 VG_PATH_DATATYPE_F,
                                 1.0f,        // scale
                                 0.0f,        // bias
                                 0,   // segmentCapacityHint
                                 0,   // coordCapacityHint
                                 VG_PATH_CAPABILITY_APPEND_TO);

    if (count == 0) {
        return vgpath;
    }

    const QPainterPath::Element *elements = &(path.elementAt(0));

    // Sizes chosen so that drawRoundedRect() paths fit in these arrays.
    QVarLengthArray<VGfloat, 48> coords;
    QVarLengthArray<VGubyte, 20> segments;

    int curvePos = 0;
    QPointF temp;

    // Keep track of the start and end of each sub-path.  QPainterPath
    // does not have an "implicit close" flag like QVectorPath does.
    // We therefore have to detect closed paths by looking for a LineTo
    // element that connects back to the initial MoveTo element.
    qreal startx = 0.0;
    qreal starty = 0.0;
    qreal endx = 0.0;
    qreal endy = 0.0;
    bool haveStart = false;
    bool haveEnd = false;

    // Convert the members of the element array.
    for (int i = 0; i < count; ++i) {
        switch (elements[i].type) {

        case QPainterPath::MoveToElement: {
            if (haveStart && haveEnd && startx == endx && starty == endy) {
                // Implicitly close the previous sub-path.
                segments.append(VG_CLOSE_PATH);
            }
            startx = elements[i].x;
            starty = elements[i].y;
            coords.append(startx);
            coords.append(starty);
            haveStart = true;
            haveEnd = false;
            segments.append(VG_MOVE_TO_ABS);
        }
        break;

        case QPainterPath::LineToElement: {
            endx = elements[i].x;
            endy = elements[i].y;
            coords.append(endx);
            coords.append(endy);
            haveEnd = true;
            segments.append(VG_LINE_TO_ABS);
        }
        break;

        case QPainterPath::CurveToElement: {
            coords.append(elements[i].x);
            coords.append(elements[i].y);
            haveEnd = false;
            curvePos = 2;
        }
        break;

        case QPainterPath::CurveToDataElement: {
            coords.append(elements[i].x);
            coords.append(elements[i].y);
            haveEnd = false;
            curvePos += 2;
            if (curvePos == 6) {
                curvePos = 0;
                segments.append(VG_CUBIC_TO_ABS);
            }
        }
        break;

        }
    }

    if ( haveStart && haveEnd && (startx == endx) && (starty == endy) ) {
        // Implicitly close the last sub-path.
        segments.append(VG_CLOSE_PATH);
    }

    vgAppendPathData(vgpath, segments.count(),
                     segments.constData(), coords.constData());

    return vgpath;
}

