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

#include "hbnvgrasterizer_p.h"

#if defined (HB_NVG_CS_ICON)
#include <nvg.h>
// Note: Cases of the following two directory names intentionally differ to
//       match the cases of the corresponding directories in Symbian 4.
#include <VG/openvg.h>
#include <vg/vgcontext_symbian.h>

struct HbNvgAspectRatioSettings
{
    TNvgAlignStatusType nvgAlignStatusAndAspectRatio;
    TNvgMeetOrSliceType type;
};
#endif //HB_NVG_CS_ICON

/*!
    \fn mapKeyAspectRatioToNvgAspectRatio()
    \a aspectRatio
 */
HbNvgAspectRatioSettings mapKeyAspectRatioToNvgAspectRatio(
                                                Qt::AspectRatioMode aspectRatio)
{
    HbNvgAspectRatioSettings settings;
    switch(aspectRatio) {

    case Qt::IgnoreAspectRatio: {
        settings.nvgAlignStatusAndAspectRatio = ENvgPreserveAspectRatio_None;
        settings.type = ENvgMeet;
        break;
    }
    case Qt::KeepAspectRatio: {
        settings.nvgAlignStatusAndAspectRatio = ENvgPreserveAspectRatio_XmidYmid;
        settings.type = ENvgMeet;
        break;
        }
    case Qt::KeepAspectRatioByExpanding: {
        settings.nvgAlignStatusAndAspectRatio = ENvgPreserveAspectRatio_XmidYmid;
        settings.type = ENvgSlice;
        break;
        }
    default: {
        settings.nvgAlignStatusAndAspectRatio = ENvgPreserveAspectRatio_XmidYmid;
        settings.type = ENvgMeet;
        break;
        }
    }
    return settings;
}

/*!
    \fn mapToVgiDisplayFormat()
    \a imageFormat
 */
VGIColorBufferFormat mapToVgiDisplayFormat(QImage::Format imageFormat)
{
    VGIColorBufferFormat format = VGI_COLOR_BUFFER_FORMAT_ARGB8888_PRE;
    switch(imageFormat)
    {
    case QImage::Format_Mono:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
            format = VGI_COLOR_BUFFER_FORMAT_ARGB8888;
            break;
    case QImage::Format_ARGB32_Premultiplied:
            format = VGI_COLOR_BUFFER_FORMAT_ARGB8888_PRE;
            break;
    case QImage::Format_RGB16:
    case QImage::Format_ARGB8565_Premultiplied:
    case QImage::Format_RGB666:
    case QImage::Format_ARGB6666_Premultiplied:
    case QImage::Format_RGB555:
    case QImage::Format_ARGB8555_Premultiplied:
            break;
    case QImage::Format_RGB888:
            format = VGI_COLOR_BUFFER_FORMAT_RGB888;
            break;
    case QImage::Format_RGB444:
    case QImage::Format_ARGB4444_Premultiplied:
    case QImage::Format_Invalid:
        break;
    }
    return format;
}

Q_GLOBAL_STATIC(HbNvgRasterizer, globalNvgRasterizer);

HbNvgRasterizer* HbNvgRasterizer::global()
{
    return globalNvgRasterizer();
}

HbNvgRasterizer::HbNvgRasterizer() : vgiSymbianInit(false), nvgEngine(0)
{
}

HbNvgRasterizer::~HbNvgRasterizer()
{
    terminate();
}

bool HbNvgRasterizer::initialize(int width, int height)
{
    vgiSymbianInit = false;

    TInt err =  VGISymbianInitialize( TSize(width, height), VGI_COLORSPACE_SRGB );
    if( err == KErrAlreadyExists || err == KErrNone) {
        vgiSymbianInit = true;
    }

    if (vgiSymbianInit) {
        TRAP(err, nvgEngine = CNvgEngine::NewL());
        if (err != KErrNone) {
            return (vgiSymbianInit = false);
        }
    }

    return vgiSymbianInit;
}

bool HbNvgRasterizer::terminate()
{
    if (vgiSymbianInit) {
        delete nvgEngine;
        nvgEngine = 0;
        VGISymbianTerminate();
        vgiSymbianInit = false;
    }

    return true;
}

bool HbNvgRasterizer::rasterize(const QByteArray &nvgData,
                   const QSizeF &renderSize,
                   Qt::AspectRatioMode aspectRatioMode,
                   QImage & destination,
                   QImage::Format imageFormat)
{
    if (destination.isNull()) {
        destination = QImage(renderSize.toSize(), imageFormat);
    }

    TUint8 * imageData = destination.bits();
    TInt stride = destination.bytesPerLine();

    return rasterize(nvgData, renderSize, aspectRatioMode, imageData, stride, imageFormat);
}

bool HbNvgRasterizer::rasterize(const QByteArray &nvgData,
               const QSizeF &renderSize,
               Qt::AspectRatioMode aspectRatioMode,
               void * destination, int stride,
               QImage::Format imageFormat)
{
    bool isIconCreated = false;
    TSize surfaceSize(TSize(renderSize.width(), renderSize.height()));

    if (!vgiSymbianInit) {
        if (!initialize(surfaceSize.iWidth, surfaceSize.iHeight)) {
            return isIconCreated;
        }
    }

    TInt err = VGISymbianResize(surfaceSize);
    if(err != KErrNone) {
        return isIconCreated;
    }

    HbNvgAspectRatioSettings settings = mapKeyAspectRatioToNvgAspectRatio(aspectRatioMode);
    nvgEngine->SetPreserveAspectRatio(settings.nvgAlignStatusAndAspectRatio, settings.type);


    nvgEngine->SetBackgroundColor(0xFFFFFF00);

    TPtr8 data((unsigned char*)nvgData.data(), nvgData.length(), nvgData.length());
    err = nvgEngine->DrawNvg(data, surfaceSize, 0, 0);
    if(err !=KErrNone) {
        return isIconCreated;
    }

    //copy the data from the surface
    VGIColorBufferFormat format = (VGIColorBufferFormat)mapToVgiDisplayFormat(imageFormat);

    err = VGICopyToTarget(format, stride, destination, 0, NULL, VGI_COPY_TRANSPARENT_PIXELS);
    if (err == VGI_OK) {
        isIconCreated = true;
    }

    return isIconCreated;
}

