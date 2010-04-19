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

#include "hbnvgiconimpl_p.h"

#include <EGL/egl.h>
#include <VG/openvg.h>
#include <QDebug>
#include <QPaintEngine>
#include <QPaintDevice>
#include <QBitmap>
#include <QStyleOption>
#include <QApplication>
#include <QScopedPointer>

#include "hbmemoryutils_p.h"
#include "hbnvgenginepool_p.h"
#include "hbeglstate_p.h"
#include "hbmaskableiconimpl_p.h"

struct HbNvgMaskedIcon
{
    QPixmap currentPixmap;
};

// Constants
static const int HB_BITS_PER_COLOR =    8;

HbNvgIconImpl::HbNvgIconImpl(const HbSharedIconInfo &iconData,
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
        readyToRender(false),
        specialCaseApplied(false),
        vgimage(0),
        opacityPaint(VG_INVALID_HANDLE),
        lastOpacity(1.0),
        nvgEngine(0),
        eglStates(HbEglStates::global())

{
    eglStates->ref();
    retrieveNvgData();
}

HbNvgIconImpl::~HbNvgIconImpl()
{
    delete nvgEngine;
    if (eglStates) {
        if (vgimage) {
            eglStates->removeVGImage(&vgimage);
        }
        if (opacityPaint) {
            vgDestroyPaint(opacityPaint);
        }
        eglStates->deref(eglStates);
    }
}

VGImage HbNvgIconImpl::createVGImageFromNVG(EGLDisplay display,
        EGLSurface currentReadSurface,
        EGLSurface currentWriteSurface,
        EGLContext eglContext,
        int width,
        int height,
        bool useGivenContext,
        HbNvgEngine * nvgEngine)
{
    EGLConfig config;

    if (!eglStates->eglConfig || !useGivenContext) {
        EGLint    numConfigs;

        const EGLint attribList[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT | EGL_VG_ALPHA_FORMAT_PRE_BIT,
            EGL_RED_SIZE, HB_BITS_PER_COLOR,
            EGL_GREEN_SIZE, HB_BITS_PER_COLOR,
            EGL_BLUE_SIZE, HB_BITS_PER_COLOR,
            EGL_ALPHA_SIZE, HB_BITS_PER_COLOR,
            EGL_NONE
        };

        if (eglChooseConfig(display, attribList, &config, 1, &numConfigs) == EGL_FALSE) {
            return VG_INVALID_HANDLE;
        }

        if (useGivenContext) {
            eglStates->eglConfig = config;
        }
    } else {
        config = eglStates->eglConfig;
    }

    HbEglStateRestorer prevEGLState(display,
                                    currentReadSurface,
                                    currentWriteSurface,
                                    eglContext);

    if (!useGivenContext) {
        if (eglMakeCurrent(display,
                           currentWriteSurface,
                           currentReadSurface,
                           eglContext) == EGL_FALSE) {
            return VG_INVALID_HANDLE;
        }
    }

    VGImage surfaceImage = vgCreateImage(VG_sARGB_8888_PRE,
                                         width, height, VG_IMAGE_QUALITY_FASTER);
    if (surfaceImage == VG_INVALID_HANDLE) {
        return VG_INVALID_HANDLE;
    }

    const VGfloat clearColor[4] = {1.0f, 1.0f, 1.0f, 0.0f};

    vgSetfv(VG_CLEAR_COLOR, 4, clearColor);
    vgClearImage(surfaceImage, 0, 0, width, height);

    EGLContext newContext;

    if (useGivenContext) {
        newContext = eglContext;
    } else {
        newContext = eglCreateContext(display, config, EGL_NO_CONTEXT, 0);
        if (newContext == EGL_NO_CONTEXT) {
            prevEGLState.restore();
            vgDestroyImage(surfaceImage);
            return VG_INVALID_HANDLE;
        }
    }

    const EGLint attribList2[] = {  EGL_NONE };
    EGLSurface pbufferSurface = eglCreatePbufferFromClientBuffer(display,
                                EGL_OPENVG_IMAGE,
                                static_cast<EGLClientBuffer>(surfaceImage),
                                config, attribList2);
    if (!pbufferSurface) {
        prevEGLState.restore();
        if (!useGivenContext) {
            eglDestroyContext(display, newContext);
        }
        vgDestroyImage(surfaceImage);
        return VG_INVALID_HANDLE;
    }

    if (eglMakeCurrent(display, pbufferSurface, pbufferSurface, newContext) == EGL_FALSE) {
        prevEGLState.restore();
        if (!useGivenContext) {
            eglDestroyContext(display, newContext);
        }
        eglDestroySurface(display, pbufferSurface);
        vgDestroyImage(surfaceImage);
        return VG_INVALID_HANDLE;
    }

    QSize iconSize(width, height);

    vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_BETTER);

    HbNvgEngine::NvgErrorType errorType = drawNVGIcon(iconSize, *nvgEngine);

    if (!useGivenContext) {
        vgFinish();
    }

    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(display, pbufferSurface);

    if (!useGivenContext) {
        eglDestroyContext(display, newContext);
    }

    return surfaceImage;
}

QPixmap HbNvgIconImpl::pixmap()
{
    if (!currentPixmap.isNull()) {
        return currentPixmap;
    }

    QSizeF renderSize = contentSize;

    int width = renderSize.width();
    int height = renderSize.height();

    if (vgimage != VG_INVALID_HANDLE) {
        QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
        vgGetImageSubData(vgimage,
                          image.bits(),
                          image.bytesPerLine(),
                          VG_sARGB_8888_PRE,
                          0,
                          0,
                          width,
                          height);

        currentPixmap = QPixmap::fromImage(image);
        return currentPixmap;
    }

    EGLDisplay display      = eglGetCurrentDisplay();
    EGLSurface currentReadSurface  = eglGetCurrentSurface(EGL_READ);
    EGLSurface currentWriteSurface  = eglGetCurrentSurface(EGL_DRAW);
    EGLContext eglContext   = eglGetCurrentContext();

    //save the original surface and context
    HbEglStateRestorer prevEGLState(display,
                                    currentReadSurface,
                                    currentWriteSurface,
                                    eglContext);

    if (display == EGL_NO_DISPLAY) {
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        eglInitialize(display, 0, 0);

        if (eglBindAPI(EGL_OPENVG_API) == EGL_FALSE) {
            return currentPixmap;
        }
    }

    EGLSurface dummySurface = EGL_NO_SURFACE;

    EGLConfig config;

    EGLint    numConfigs;

    const EGLint attribList[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT | EGL_VG_ALPHA_FORMAT_PRE_BIT,
        EGL_RED_SIZE, HB_BITS_PER_COLOR,
        EGL_GREEN_SIZE, HB_BITS_PER_COLOR,
        EGL_BLUE_SIZE, HB_BITS_PER_COLOR,
        EGL_ALPHA_SIZE, HB_BITS_PER_COLOR,
        EGL_NONE
    };

    if (eglChooseConfig(display, attribList, &config, 1, &numConfigs) == EGL_FALSE) {
        return currentPixmap;
    }

    if (currentWriteSurface == EGL_NO_SURFACE) {
         //pixmap is called without EGL being initialized

        const EGLint attribList2[] = {
            EGL_WIDTH, width,
            EGL_HEIGHT, height,
            EGL_NONE
        };

        dummySurface = eglCreatePbufferSurface(display, config, attribList2);
        if (dummySurface == EGL_NO_SURFACE) {
            return currentPixmap;
        }
    }

    //Bind the newly created context and surface
    VGImage surfaceImage;
    HbNvgEngine localNvgEngine;

    if (dummySurface) {
        surfaceImage = createVGImageFromNVG(display,
                                            dummySurface,
                                            dummySurface,
                                            eglContext,
                                            width,
                                            height,
                                            true,
                                            &localNvgEngine);
    } else {
        surfaceImage = createVGImageFromNVG(display,
                                            currentReadSurface,
                                            currentWriteSurface,
                                            eglContext,
                                            width,
                                            height,
                                            true,
                                            &localNvgEngine);
    }

    if (surfaceImage == VG_INVALID_HANDLE) {
        return currentPixmap;
    }

    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    if (dummySurface != EGL_NO_SURFACE) {
        eglDestroySurface(display, dummySurface);
    }

    prevEGLState.restore();

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    vgGetImageSubData(surfaceImage, image.bits(), image.bytesPerLine(),
                      VG_sARGB_8888_PRE, 0, 0, width, height);

    vgDestroyImage(surfaceImage);
    currentPixmap = QPixmap::fromImage(image);
    return currentPixmap;
}

HbNvgEngine::NvgErrorType HbNvgIconImpl::drawNVGIcon(const QSize & size, HbNvgEngine & nvgEngine)
{

    VGint                   mMatrixMode;
    VGfloat                 mImageMatrix[9];
    VGfloat                 mPathMatrix[9];

    mMatrixMode = vgGeti(VG_MATRIX_MODE);
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgGetMatrix(mPathMatrix);
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
    vgGetMatrix(mImageMatrix);
    vgSeti(VG_MATRIX_MODE, mMatrixMode);

    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgLoadIdentity();
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
    vgLoadIdentity();

    int scissoring = vgGeti(VG_SCISSORING);
    if (scissoring) {
        vgSeti(VG_SCISSORING, VG_FALSE);
    }

    NvgAspectRatioSettings settings = mapKeyAspectRatioToNvgAspectRatio(aspectRatioMode);
    nvgEngine.setPreserveAspectRatio(settings.nvgAlignStatusAndAspectRatio, settings.type);
    nvgEngine.setMirroringMode(mirrored);

    HbNvgEngine::NvgErrorType errorType = nvgEngine.drawNvg(nvgData, size);

    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgLoadMatrix(mPathMatrix);
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
    vgLoadMatrix(mImageMatrix);
    vgSeti(VG_MATRIX_MODE, mMatrixMode);
    if (scissoring) {
        vgSeti(VG_SCISSORING, VG_TRUE);
    }

    return errorType;
}

NvgAspectRatioSettings HbNvgIconImpl::mapKeyAspectRatioToNvgAspectRatio(
    Qt::AspectRatioMode aspectRatio)
{
    NvgAspectRatioSettings settings;
    switch (aspectRatio) {

    case Qt::IgnoreAspectRatio: {
        settings.nvgAlignStatusAndAspectRatio = HbNvgEngine::NvgPreserveAspectRatioNone;
        settings.type = HbNvgEngine::NvgMeet;
        break;
    }
    case Qt::KeepAspectRatio: {
        settings.nvgAlignStatusAndAspectRatio = HbNvgEngine::NvgPreserveAspectRatioXmidYmid;
        settings.type = HbNvgEngine::NvgMeet;
        break;
    }
    case Qt::KeepAspectRatioByExpanding: {
        settings.nvgAlignStatusAndAspectRatio = HbNvgEngine::NvgPreserveAspectRatioXmidYmid;
        settings.type = HbNvgEngine::NvgSlice;
        break;
    }
    default: {
        settings.nvgAlignStatusAndAspectRatio = HbNvgEngine::NvgPreserveAspectRatioXmidYmid;
        settings.type = HbNvgEngine::NvgMeet;
        break;
    }
    }
    return settings;
}

QSize HbNvgIconImpl::defaultSize() const
{
    return defaultIconSize;
}

void HbNvgIconImpl::retrieveNvgData()
{
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    nvgData  = QByteArray::fromRawData((char*)manager->base() + sharedIconData.nvgData.offset,
                                       sharedIconData.nvgData.dataSize);
    defaultIconSize =
        QSize(sharedIconData.nvgData.defaultWidth, sharedIconData.nvgData.defaultHeight);
    contentSize = QSize(sharedIconData.nvgData.width, sharedIconData.nvgData.height);

}

void HbNvgIconImpl::paint(QPainter* painter,
                          const QRectF& rect,
                          Qt::Alignment alignment,
                          HbMaskableIconImpl * maskIconData)
{
#ifdef HB_ICON_CACHE_DEBUG
    qDebug() << "HbNvgIconImpl::paint()-->"<<this->fileName;
#endif
    QSizeF renderSize = contentSize;

    NvgAspectRatioSettings settings = mapKeyAspectRatioToNvgAspectRatio(aspectRatioMode);

    QPointF topLeft = setAlignment(rect, renderSize, alignment);
    if (!nvgEngine) {
        nvgEngine = HbNVGEnginePool::instance()->getNVGEngine();
    }

    bool maskApplied = false;
    if (maskIconData && maskIconData->maskChanged()) {
        maskApplied = true;
    }

    if (readyToRender && !maskApplied) {
        if (maskIconData) {
            HbNvgMaskedIcon * mi = (HbNvgMaskedIcon *) maskIconData->implData();
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

    if (multiPieceIcon || painter->opacity() != 1.0) {
        if (drawRasterizedIcon(painter, topLeft, renderSize)) {
            return;
        }
    }

    drawNVGIcon(painter, topLeft, renderSize, settings);
}

void HbNvgIconImpl::applySpecialCases(QPainter * painter,
                                      const QPointF & topLeft,
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
        HbNvgMaskedIcon * mi = (HbNvgMaskedIcon *)maskIconData->implData();
        if (maskIconData->maskChanged()) {
            if (!mi) {
                mi = new HbNvgMaskedIcon();
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

void HbNvgIconImpl::drawNVGIcon(QPainter * painter,
                                const QPointF & topLeft,
                                const QSizeF & renderSize,
                                NvgAspectRatioSettings settings)
{
    nvgEngine->engine()->setPreserveAspectRatio(settings.nvgAlignStatusAndAspectRatio,
        settings.type);
    nvgEngine->engine()->setMirroringMode(mirrored);

    painter->beginNativePainting();
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgTranslate(topLeft.x(), topLeft.y());

#ifdef ROUND_TRANSLATE_POINT
    // nvg drawing is seems to be having some issues with
    // translation with decimal values
    // ceiling of the translation values are calculated and set.

    VGfloat pathMatrix[9];
    vgGetMatrix(pathMatrix);

    VGfloat tX = pathMatrix[6];
    VGfloat tY = pathMatrix[7];

    VGfloat sX = pathMatrix[0];
    VGfloat sY = pathMatrix[4];

    VGfloat shX = pathMatrix[3];
    VGfloat shY = pathMatrix[1];

    int itx = (int)tX;
    int ity = (int)tY;

    VGfloat frX = tX - (VGfloat)(itx);
    VGfloat frY = tY - (VGfloat)(ity);

    if (frX != 0.0) {
        frX = 1.0 - frX;
    }

    if (frY != 0.0) {
        frY = 1.0 - frY;
    }

    if (sY < 0) {
        frY = -frY;
    }

    if (sX < 0) {
        frX = -frX;
    }

    if (shX > 0 && shY > 0) {
        VGfloat temp = frX;
        frX = frY;
        frY = temp;
    } else if (shX < 0 && shY > 0) {
        VGfloat temp = frX;
        frX = frY;
        frY = temp;
        frY = -frY;
    } else if (shX > 0 && shY < 0) {
        VGfloat temp = frX;
        frX = frY;
        frY = temp;
        frX = -frX;
    } else if (shX < 0 && shY < 0) {
        VGfloat temp = frX;
        frX = frY;
        frY = temp;
        frX = -frX;
        frY = -frY;
    }

    vgTranslate(frX, frY);
#endif

    nvgEngine->engine()->drawNvg(nvgData, renderSize.toSize());

    painter->endNativePainting();
}

bool HbNvgIconImpl::drawRasterizedIcon(QPainter * painter,
                                       const QPointF & topLeft,
                                       const QSizeF & renderSize)
{
    // need to do a reset if EGL contexts changes/recreated
    if (!eglStates->initialized()) {
        eglStates->set(eglGetCurrentDisplay(),
                       eglGetCurrentSurface(EGL_READ),
                       eglGetCurrentSurface(EGL_DRAW),
                       eglGetCurrentContext());
    }

    if (vgimage == VG_INVALID_HANDLE) {
        painter->beginNativePainting();

        vgimage = createVGImageFromNVG(eglStates->display,
                                       eglStates->currentReadSurface,
                                       eglStates->currentWriteSurface,
                                       eglStates->eglContext,
                                       renderSize.width(),
                                       renderSize.height(),
                                       true,
                                       nvgEngine->engine());
        if (vgimage) {
            eglStates->addVGImage(&vgimage);
        }
        painter->endNativePainting();
        opacityPaint = VG_INVALID_HANDLE;
    }

    if (vgimage != VG_INVALID_HANDLE) {

        VGint imageMode      = vgGeti(VG_IMAGE_MODE);
        VGint matrixMode     = vgGeti(VG_MATRIX_MODE);
        VGPaint oldFillPaint = VG_INVALID_HANDLE;
        VGPaint oldStrkPaint = VG_INVALID_HANDLE;
        VGint   blendMode    = 0;

        qreal opacity = painter->opacity();

        if (opacity != lastOpacity) {
            lastOpacity = opacity;
            if (opacityPaint == VG_INVALID_HANDLE) {
                opacityPaint = vgCreatePaint();
            }
            if (opacity != 1.0) {
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

        VGfloat devh = painter->device()->height() - 1;
        QTransform viewport(1.0f, 0.0f, 0.0f,
                            0.0f, -1.0f, 0.0f,
                            0.5f, devh + 0.5f, 1.0f);
        QTransform imageTransform = painter->transform() * viewport;
        imageTransform.translate(topLeft.x(), topLeft.y());

        VGfloat mat[9];

        if (opacity == 1.0) {
            vgSeti(VG_IMAGE_MODE, VG_DRAW_IMAGE_NORMAL);
        } else {
            vgSeti(VG_IMAGE_MODE, VG_DRAW_IMAGE_MULTIPLY);
        }

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

        vgDrawImage(vgimage);
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
        return true;
    }
    return false;
}

QPointF HbNvgIconImpl::setAlignment(const QRectF& rect,
                                    QSizeF& renderSize,
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

QSize HbNvgIconImpl::size()
{
    return contentSize;
}

void HbNvgIconImpl::destroyMaskedData(IconMaskedData data)
{
    delete((HbNvgMaskedIcon *)data);
}

