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
#include "hbvgimageiconrenderer_p.h"
#include "hbpixmapiconrenderer_p.h"

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
        nvgEngine(NULL),
        eglStates(HbEglStates::global()),
        vgImageRenderer(0),
        pixmapIconRenderer(0)
{
    eglStates->ref();
    retrieveNvgData();
}

HbNvgIconImpl::~HbNvgIconImpl()
{
    delete nvgEngine;
    if (eglStates) {
        eglStates->deref(eglStates);
    }
    if (vgImageRenderer) {
        delete vgImageRenderer;
    }
    if (pixmapIconRenderer) {
        delete pixmapIconRenderer;
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

    HbNvgEngine::HbNvgErrorType errorType = drawNVGIcon(iconSize, *nvgEngine);

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

HbNvgEngine::HbNvgErrorType HbNvgIconImpl::drawNVGIcon(const QSize & size, HbNvgEngine & nvgEngine)
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
    nvgEngine.enableMirroring(mirrored);

    HbNvgEngine::HbNvgErrorType errorType = nvgEngine.drawNvg(nvgData, size);

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

VGImage HbNvgIconImpl::getVgImage(HbIconImpl * impl, QPainter * painter)
{
    HbNvgIconImpl * nvgImpl = (HbNvgIconImpl*)impl;

    painter->beginNativePainting();

    VGImage vgimage = nvgImpl->createVGImageFromNVG(nvgImpl->eglStates->display,
                      nvgImpl->eglStates->currentReadSurface,
                      nvgImpl->eglStates->currentWriteSurface,
                      nvgImpl->eglStates->eglContext,
                      nvgImpl->contentSize.width(),
                      nvgImpl->contentSize.height(),
                      true,
                      nvgImpl->nvgEngine->engine());

    painter->endNativePainting();

    return vgimage;
}
void HbNvgIconImpl::paint(QPainter* painter,
                          const QRectF& rect,
                          Qt::Alignment alignment,
                          const QPainterPath &clipPath,
                          HbMaskableIconImpl * maskIconData)
{
#ifdef HB_ICON_CACHE_DEBUG
    qDebug() << "HbNvgIconImpl::paint()-->"<<this->fileName;
#endif
    QSizeF renderSize = contentSize;

    NvgAspectRatioSettings settings = mapKeyAspectRatioToNvgAspectRatio(aspectRatioMode);

    QPointF topLeft = setAlignment(rect, renderSize, alignment);
    if (!nvgEngine) {
        nvgEngine = HbNvgEnginePool::instance()->getNvgEngine();
    }

    bool maskApplied = false;
    if (maskIconData && (maskIconData->maskChanged() ||  maskIconData->implData())) {
        maskApplied = true;
    }

    if ((painter->paintEngine()->type() != QPaintEngine::OpenVG) || maskApplied) {

        painter->beginNativePainting();
        pixmap();
        painter->endNativePainting();

        if (!pixmapIconRenderer) {
            pixmapIconRenderer = new HbPixmapIconRenderer(currentPixmap, this);
            pixmapIconRenderer->setColor(iconColor);
            pixmapIconRenderer->setMode(mode);
        }

        pixmapIconRenderer->draw(painter, topLeft, clipPath, maskIconData);
        return;
    }

    QPainterPath intersect;
    if (!clipPath.isEmpty()) {
        QPainterPath piecePath;
        piecePath.addRect(rect);
        intersect = clipPath.intersected(piecePath);
        QRectF intRect = intersect.boundingRect();
        if (intersect.isEmpty()) {
            return;
        }
    }

    if ((iconColor.isValid()) || (mode != QIcon::Normal) ||
            multiPieceIcon || painter->opacity() != 1.0) {
        if (drawRasterizedIcon(painter, topLeft, renderSize, intersect)) {
            return;
        }
    }

    drawNVGIcon(painter, topLeft, renderSize, settings);
}

void HbNvgIconImpl::drawNVGIcon(QPainter * painter,
                                const QPointF & topLeft,
                                const QSizeF & renderSize,
                                NvgAspectRatioSettings settings)
{
    nvgEngine->engine()->setPreserveAspectRatio(settings.nvgAlignStatusAndAspectRatio,
        settings.type);
    nvgEngine->engine()->enableMirroring(mirrored);

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
                                       const QSizeF & renderSize,
                                       const QPainterPath &clipPath)
{
    bool ret = false;
    // need to do a reset if EGL contexts changes/recreated
    if (!eglStates->initialized()) {
        eglStates->set(eglGetCurrentDisplay(),
                       eglGetCurrentSurface(EGL_READ),
                       eglGetCurrentSurface(EGL_DRAW),
                       eglGetCurrentContext());
    }

    if (!vgImageRenderer) {
        painter->beginNativePainting();

        VGImage vgimage = createVGImageFromNVG(eglStates->display,
                                               eglStates->currentReadSurface,
                                               eglStates->currentWriteSurface,
                                               eglStates->eglContext,
                                               renderSize.width(),
                                               renderSize.height(),
                                               true,
                                               nvgEngine->engine());

        if (vgimage != VG_INVALID_HANDLE) {
            vgImageRenderer = new HbVgImageIconRenderer(vgimage, renderSize.toSize(), this);
            vgImageRenderer->setVgImageCreator(getVgImage);
            vgImageRenderer->setColor(iconColor);
            vgImageRenderer->setMode(mode);
        }
        painter->endNativePainting();
    }

    if (vgImageRenderer) {
        ret = vgImageRenderer->draw(painter, topLeft, clipPath);
    }
    return ret;
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

void HbNvgIconImpl::destroyMaskedData(HbIconMaskedData *data)
{
    pixmapIconRenderer->destroyMaskedData(data);
}

