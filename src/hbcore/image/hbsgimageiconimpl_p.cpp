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

#include "hbsgimageiconimpl_p.h"
#include <EGL/egl.h>
#include <QDebug>
#include <QImage>
#include <QPaintEngine>
#include <QPaintDevice>
#include <QGlobal.h>
#include <QStyleOption>
#include <QApplication>

#include "hbmaskableiconimpl_p.h"
#include "hbeglstate_p.h"

struct HbSgImageMaskedIcon
{
    QPixmap    currentPixmap;
};

typedef EGLImageKHR( *pfnEglCreateImageKHR)(EGLDisplay, EGLContext,
        EGLenum, EGLClientBuffer, EGLint*);
typedef EGLBoolean( *pfnEglDestroyImageKHR)(EGLDisplay, EGLImageKHR);
typedef VGImage( *pfnVgCreateEGLImageTargetKHR)(VGeglImageKHR);

HbSgimageIconImpl::HbSgimageIconImpl(const HbSharedIconInfo &iconData,
                                     const QString& name,
                                     const QSizeF& keySize,
                                     Qt::AspectRatioMode aspectRatioMode,
                                     QIcon::Mode mode,
                                     bool mirrored):
        HbIconImpl(iconData,
                   name,
                   keySize,
                   aspectRatioMode,
                   mode,
                   mirrored),
        vgImage(VG_INVALID_HANDLE),
        readyToRender(false),
        specialCaseApplied(false),
        opacityPaint(VG_INVALID_HANDLE),
        lastOpacity(1.0)

{
    eglStates = HbEglStates::global();
    eglStates->ref();
    retrieveSgImageData();
}

HbSgimageIconImpl::~HbSgimageIconImpl()
{
    if (vgImage) {
        eglStates->removeVGImage(&vgImage);
    }

    eglStates->deref(eglStates);

    if (opacityPaint) {
        vgDestroyPaint(opacityPaint);
    }
}

QPixmap HbSgimageIconImpl::pixmap()
{
    if (!currentPixmap.isNull()) {
        return currentPixmap;
    }

    VGImage localVgImage = getVgImageFromSgImage();
    if (localVgImage == VG_INVALID_HANDLE) {
        return currentPixmap;
    }

    VGint imgWidth = contentSize.width();
    VGint imgHeight = contentSize.height();

    QImage image(imgWidth, imgHeight, QImage::Format_ARGB32_Premultiplied);

    vgGetImageSubData(localVgImage, image.bits(),
                      image.bytesPerLine(),
                      VG_sARGB_8888_PRE,
                      0,
                      0,
                      imgWidth,
                      imgHeight);

    currentPixmap = QPixmap::fromImage(image);

    vgDestroyImage(localVgImage);

    return currentPixmap;
}

QSize HbSgimageIconImpl::defaultSize() const
{
    return defaultIconSize;
}

void HbSgimageIconImpl::retrieveSgImageData()
{
    memcpy(&sgImageId.iId, &sharedIconData.sgImageData.id, sizeof(sgImageId.iId));
    defaultIconSize =
        QSize(sharedIconData.sgImageData.defaultWidth, sharedIconData.sgImageData.defaultHeight);
    contentSize = QSize(sharedIconData.sgImageData.width, sharedIconData.sgImageData.height);
}

void HbSgimageIconImpl::paint(QPainter *painter,
                              const QRectF &rect,
                              Qt::Alignment alignment,
                              HbMaskableIconImpl *maskIconData)
{
#ifdef HB_ICON_CACHE_DEBUG
    qDebug() << "HbSgimageIconImpl::paint()-->"<<this->fileName;
#endif

    QSizeF renderSize(contentSize);
    QPointF topLeft = setAlignment(rect, renderSize, alignment);
    bool maskApplied = false;
    if (maskIconData && maskIconData->maskChanged()) {
        maskApplied = true;
    }

    if (readyToRender && !maskApplied) {
        if (maskIconData) {
            HbSgImageMaskedIcon * mi = (HbSgImageMaskedIcon *) maskIconData->implData();
            if (mi) {
                painter->drawPixmap(topLeft, mi->currentPixmap, mi->currentPixmap.rect());
                return;
            }
        }
        painter->drawPixmap(topLeft, currentPixmap, currentPixmap.rect());
        return;
    }
    if ((iconColor.isValid()) || (mode != QIcon::Normal) ||
            (painter->paintEngine()->type() != QPaintEngine::OpenVG) ||
            (maskApplied)) {
        applySpecialCases(painter, topLeft, maskIconData);
        return;
    }
    if (vgImage == VG_INVALID_HANDLE) {
        vgImage = getVgImageFromSgImage();
        if (vgImage == VG_INVALID_HANDLE) {
            return;
        }
        eglStates->addVGImage(&vgImage);
        opacityPaint = VG_INVALID_HANDLE;
    }

    VGint imageMode      = vgGeti(VG_IMAGE_MODE);
    VGint matrixMode     = vgGeti(VG_MATRIX_MODE);
    VGPaint oldFillPaint = VG_INVALID_HANDLE;
    VGPaint oldStrkPaint = VG_INVALID_HANDLE;
    VGint   blendMode    = 0;

    updatePainterTransformation(painter, topLeft);

    qreal opacity = painter->opacity();

    if (opacity != lastOpacity) {
        lastOpacity = opacity;
        if (opacity != 1.0) {
            if (opacityPaint == VG_INVALID_HANDLE) {
                opacityPaint = vgCreatePaint();
            }
            VGfloat opaquePaint[] = {1.0f, 1.0f, 1.0f, opacity};
            if (opacityPaint != VG_INVALID_HANDLE) {
                vgSetParameteri(opacityPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
                vgSetParameterfv(opacityPaint, VG_PAINT_COLOR, 4, opaquePaint);
            }
        }
    }

    if (opacity != 1.0 && opacityPaint != VG_INVALID_HANDLE) {
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

#ifdef HB_ICON_TRACES
    qDebug() <<  "HbSgimageIconImpl::paint() " + this->fileName;
#endif
    vgDrawImage(vgImage);
    eglWaitClient();

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
}

void HbSgimageIconImpl::updatePainterTransformation(QPainter *painter, const QPointF &pos)
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

QPointF HbSgimageIconImpl::setAlignment(const QRectF &rect,
                                        QSizeF &renderSize,
                                        Qt::Alignment alignment)
{
    QPointF topLeft = rect.topLeft();
    if (alignment & Qt::AlignRight) {
        topLeft.setX(rect.right() - renderSize.width());
    } else if (alignment & Qt::AlignHCenter) {
        topLeft.setX(topLeft.x() + (rect.width() - renderSize.width()) / 2);
    }

    if (alignment & Qt::AlignBottom) {
        topLeft.setY(rect.bottom() - renderSize.height());
    } else if (alignment & Qt::AlignVCenter) {
        topLeft.setY(topLeft.y() + (rect.height() - renderSize.height()) / 2);
    }
    return topLeft;
}

VGImage HbSgimageIconImpl::getVgImageFromSgImage()
{
    // Open the image from the server process passed to us via sharedId.
    RSgImage sgImage;
    TInt err = sgImage.Open(sgImageId);

    if (err == KErrNotFound) {
#ifdef HB_ICON_TRACES
        qDebug() << "sgImage: Open: Not Found %d", err;
#endif
        return VG_INVALID_HANDLE;
    } else {
#ifdef HB_ICON_TRACES
        qDebug() << "sgImage: Open: Something Available %d", err;
#endif
    }

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    // Retrieve the extensions
    pfnEglCreateImageKHR eglCreateImageKHR = (pfnEglCreateImageKHR)
        eglGetProcAddress("eglCreateImageKHR");
    pfnEglDestroyImageKHR eglDestroyImageKHR = (pfnEglDestroyImageKHR)
        eglGetProcAddress("eglDestroyImageKHR");
    pfnVgCreateEGLImageTargetKHR vgCreateEGLImageTargetKHR = (pfnVgCreateEGLImageTargetKHR)
        eglGetProcAddress("vgCreateEGLImageTargetKHR");

    // Create an EGLImage based on the RSgImage via extensions, specifying the
    // EGL_IMAGE_PRESERVED_KHR attribute as EGL_TRUE to ensure its contents
    // are preserved.
    EGLint imgAttr[] = {
        EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
        EGL_NONE
    };

    EGLImageKHR eglImage = eglCreateImageKHR(display, EGL_NO_CONTEXT,
                           EGL_NATIVE_PIXMAP_KHR,
                           (EGLClientBuffer) & sgImage,
                           imgAttr);
    if (eglImage == EGL_NO_IMAGE_KHR) {
        return VG_INVALID_HANDLE;
    }

    VGImage vgImage = vgCreateEGLImageTargetKHR((VGeglImageKHR)eglImage);
    eglDestroyImageKHR(display, eglImage);
    return vgImage;
}

QSize HbSgimageIconImpl::size()
{
    return  contentSize;
}

void HbSgimageIconImpl::applySpecialCases(QPainter *painter,
        const QPointF &topLeft,
        HbMaskableIconImpl *maskIconData)
{
    painter->beginNativePainting();
    pixmap();
    painter->endNativePainting();

    if (!specialCaseApplied) {
        if ((iconColor.isValid()) && (mode != QIcon::Disabled)) {
            if (!currentPixmap.isNull()) {
                QPixmap mask = currentPixmap.alphaChannel();
                currentPixmap.fill(iconColor);
                currentPixmap.setAlphaChannel(mask);
            }
        }
        // Apply the mode
        if (mode != QIcon::Normal) {
            QStyleOption opt(0);
            opt.palette = QApplication::palette();
            currentPixmap = QApplication::style()->generatedIconPixmap(mode, currentPixmap, &opt);
        }
        specialCaseApplied = true;
    }

    if (maskIconData) {
        HbSgImageMaskedIcon * mi = (HbSgImageMaskedIcon *)maskIconData->implData();
        if (maskIconData->maskChanged()) {
            if (!mi) {
                mi = new HbSgImageMaskedIcon();
            }
            mi->currentPixmap = currentPixmap;
            mi->currentPixmap.setMask(maskIconData->mask());
            maskIconData->setImplData(mi);
        }
        if (mi) {
            painter->drawPixmap(topLeft, mi->currentPixmap, mi->currentPixmap.rect());
            readyToRender = true;
            return;
        }
    }
    painter->drawPixmap(topLeft, currentPixmap, currentPixmap.rect());
    readyToRender = true;
}

void HbSgimageIconImpl::destroyMaskedData(IconMaskedData data)
{
    delete((HbSgImageMaskedIcon *)data);
}

