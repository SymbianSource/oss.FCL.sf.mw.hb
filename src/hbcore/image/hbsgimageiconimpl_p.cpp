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
#include "hbvgimageiconrenderer_p.h"
#include "hbeglstate_p.h"
#include "hbpixmapiconrenderer_p.h"

typedef EGLImageKHR(*pfnEglCreateImageKHR)(EGLDisplay, EGLContext,
        EGLenum, EGLClientBuffer, EGLint*);
typedef EGLBoolean(*pfnEglDestroyImageKHR)(EGLDisplay, EGLImageKHR);
typedef VGImage(*pfnVgCreateEGLImageTargetKHR)(VGeglImageKHR);

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
        vgImageRenderer(0),
        pixmapIconRenderer(0)
{
    retrieveSgImageData();
}

HbSgimageIconImpl::~HbSgimageIconImpl()
{
    if (vgImageRenderer) {
        delete vgImageRenderer;
    }
    if (pixmapIconRenderer) {
        delete pixmapIconRenderer;
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

VGImage HbSgimageIconImpl::getVgImage(HbIconImpl * impl, QPainter *)
{
    return ((HbSgimageIconImpl *)impl)->getVgImageFromSgImage();
}

void HbSgimageIconImpl::paint(QPainter* painter,
                              const QRectF& rect,
                              Qt::Alignment alignment,
                              const QPainterPath &clipPath,
                              HbMaskableIconImpl * maskIconData)
{
#ifdef HB_ICON_CACHE_DEBUG
    qDebug() << "HbSgimageIconImpl::paint()-->" << this->fileName;
#endif

    QSizeF renderSize(contentSize);

    QPointF topLeft = setAlignment(rect, renderSize, alignment);

    bool maskApplied = false;
    if (maskIconData && (maskIconData->maskChanged() ||  maskIconData->implData())) {
        maskApplied = true;
    }

    if ((painter->paintEngine()->type() != QPaintEngine::OpenVG) ||
         maskApplied || pixmapIconRenderer) {
        // going to pixmap, vgimage may not be required any more
        if (vgImageRenderer) {
            delete vgImageRenderer;
            vgImageRenderer = 0;
        }
        
        if (!pixmapIconRenderer) {
            painter->beginNativePainting();
            pixmap();
            painter->endNativePainting();
            
            pixmapIconRenderer = new HbPixmapIconRenderer(currentPixmap, this);
            pixmapIconRenderer->setColor(iconColor);
            pixmapIconRenderer->setMode(mode);
        }

        pixmapIconRenderer->draw(painter, topLeft, clipPath, maskIconData);
        return;
    }

    if (!vgImageRenderer) {
        VGImage vgImage = VG_INVALID_HANDLE;
        vgImage = getVgImageFromSgImage();
        if (vgImage != VG_INVALID_HANDLE) {
            vgImageRenderer = new HbVgImageIconRenderer(vgImage, renderSize.toSize(), this);
            vgImageRenderer->setVgImageCreator(getVgImage);
            vgImageRenderer->setColor(iconColor);
            vgImageRenderer->setMode(mode);
        }
    }

    if (vgImageRenderer) {
        vgImageRenderer->draw(painter, topLeft, clipPath);
        eglWaitClient();
        return;
    }
}

QPointF HbSgimageIconImpl::setAlignment(const QRectF& rect,
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
    sgImage.Close();
    return vgImage;
}

QSize HbSgimageIconImpl::size()
{
    return  contentSize;
}


void HbSgimageIconImpl::destroyMaskedData(HbIconMaskedData *data)
{
    pixmapIconRenderer->destroyMaskedData(data);
}

