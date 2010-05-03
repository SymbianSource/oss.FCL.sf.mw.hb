/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbServers module of the UI Extensions for Mobile.
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

#include "hbsgimageiconprocessor_p.h"
#include "hbiconsource_p.h"
#include "hbthemeserverutils_p.h"
#include <QtGlobal>
#include <QDebug>
#include <QFile>
#include <sgresource/sgimage.h>
#include <EGL/egl.h>
#include <VG/openvg.h>
#include <cmath>



static HbNvgEngine nvgEngine;
HbSgImageRenderer* HbSgimageIconProcessor::sgImageRenderer = HbSgImageRenderer::global();

struct HbSgImageClosure
{
    static inline void cleanup(RSgImage *sgImage)
    {
        if (sgImage) { 
            sgImage->Close();
        }
    }
};

const int PIXEL_DEPTH = 4;
const int SGIMAGE_PIXEL_OVERHEAD = 200; //200 bytes of overhead in VideoCore for irregular size images
const int TILE_SIZE_IN_PIXELS = 32 * 32;

/*!
    \fn HbSgimageIconProcessor::HbSgimageIconProcessor()
    Constructor
    \a key denotes the unique identifier for the cache item
    \a options indicate different ways of loading icons
    \a format indicates the icon format e.g. svg/nvg etc.
 */
HbSgimageIconProcessor::HbSgimageIconProcessor(const HbIconKey &key,
        const HbIconLoader::IconLoaderOptions &options,
        const QString &type)
{
    iconKey = key;
    iconOptions = options;
    iconType = type;
    defaultSize = QSize(0, 0);
    sgImageRenderer->initialize();
}

/*!
    \fn HbSgimageIconProcessor::~HbSgimageIconProcessor()
    Destructor
 */
HbSgimageIconProcessor::~HbSgimageIconProcessor()
{
}

/*!
    \fn HbSgimageIconProcessor::createIconData()
    ToDo
    \a iconPath the path to the icon file
 */
bool HbSgimageIconProcessor::createIconData(const QString &iconPath)
{
    if (!iconKey.size.isValid()) {
        return false;
    }
    bool isSgImageCreated = createSgimage(iconPath);
    return isSgImageCreated;
}

/*!
    \fn HbSgimageIconProcessor::createSgimage()
    ToDO
    \a iconPath the path to the icon file
 */
bool HbSgimageIconProcessor::createSgimage(const QString &iconPath)
{
    bool isDefaultSize =  iconKey.size.isNull();
    data.type = INVALID_FORMAT;

    HbIconSource *source = HbThemeServerUtils::getIconSource(iconPath);
    QByteArray *sourceByteArray = source->byteArray();
    if( !sourceByteArray ) {
        return false;
    }
    byteArray = *sourceByteArray;
    QSizeF size = source->defaultSize();
    defaultSize = size.toSize();
    QSizeF renderSize(defaultSize);
    data.sgImageData.defaultWidth = defaultSize.width();
    data.sgImageData.defaultHeight = defaultSize.height();
    if (!isDefaultSize) {
        renderSize.scale(iconKey.size, iconKey.aspectRatioMode);
    }

    data.sgImageData.width = renderSize.width();
    data.sgImageData.height = renderSize.height();

    //SgDriver is opened only once for the themserver process when the themserver is started.
    //Also eglInitialize is done only once for the theserver process
    //It is closed only when the themeserver is closed.

    // Create RSgImage, specifying ESgUsageBitOpenVgImage and ESgUsageBitOpenVgSurface
    // usage bits so it can be used as a target for OpenVG rendering inside the Theme
    // Server and as an OpenVG image inside the client applications.

    TSgImageInfo info;
    info.iSizeInPixels = TSize(data.sgImageData.width, data.sgImageData.height);
    info.iPixelFormat = ESgPixelFormatARGB_8888_PRE;
    info.iUsage = ESgUsageBitOpenVgImage | ESgUsageBitOpenVgSurface;

    QScopedPointer<RSgImage, HbSgImageClosure> tempSgImage(new  RSgImage);
    RSgImage *sgImage = tempSgImage.data();
    Q_CHECK_PTR(sgImage);

    TInt err = sgImage->Create(info, 0, 0);
    if (err != KErrNone) {
        return false;
    }

    TSgDrawableId sgImageId = sgImage->Id();

    memcpy(&data.sgImageData.id, &sgImageId.iId, sizeof(data.sgImageData.id));
    data.type = SGIMAGE;

    bool success = sgImageRenderer->beginRendering(sgImage);
    if (!success) {
        return false;
    }

    VGfloat color[4] = { 0.0f, 1.0f, 1.0f, 0.0f };  // TODO revert back to white color
    vgSetfv(VG_CLEAR_COLOR, 4, color);
    vgClear(0, 0, data.sgImageData.width, data.sgImageData.height);

    success = renderNvg(byteArray, QRect(0, 0, data.sgImageData.width, data.sgImageData.height),
                        iconKey.aspectRatioMode, iconKey.mirrored);
    if (!success) {
        return false;
    }
    vgFinish();
    // Once finished, release all handles to the image, and shut down EGL.
    // Make a null EGLSurface current to release the current surface before
    // destroying. The RSgImage contents will persist until the RSgImage is closed.
    sgImageRenderer->endRendering();
    tempSgImage.take();
    HbSgImageRenderer::addSgImageToHash(data.sgImageData.id, sgImage);
    return true;
}

/*!
    \fn HbSgimageIconProcessor::mapKeyAspectRatioToNvgAspectRatio()
    maps qt aspect ratio to nvg aspect ratio.
 */
NvgAspectRatioSettings HbSgimageIconProcessor::mapKeyAspectRatioToNvgAspectRatio(
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

/*!
    \fn HbSgimageIconProcessor::sharedIconDataCost()
    Returns the size of the Nvg icon data in bytes
 */
int HbSgimageIconProcessor::sharedIconDataCost() const
{
    // here depth is based on pixel format which in this case is EUidPixelFormatRGB_565 which is 2bytes.
    int xTiles = ceil(data.sgImageData.width / 32.0);
    int yTiles = ceil(data.sgImageData.height / 32.0);
    int sizeInPixels =  xTiles * yTiles * TILE_SIZE_IN_PIXELS;
    int sizeInBytes = sizeInPixels * PIXEL_DEPTH;
    return (sizeInBytes  + SGIMAGE_PIXEL_OVERHEAD);
}

/*!
    \fn HbSgimageIconProcessor::sharedIconData()
    Copies the Nvg vector data to the shared memory
 */
HbSharedIconInfo HbSgimageIconProcessor::sharedIconData()const
{
    return data;
}

bool HbSgimageIconProcessor::createMultiPieceIconData(const QVector<HbSharedIconInfo> &multiPieceIconInfo,
        HbMultiIconParams &multiPieceIconParams)
{
    bool mirrored = false;
    int consolidatedIconWidth = 0;
    TSgImageInfo info;
    info.iSizeInPixels = TSize(qRound(multiPieceIconParams.size.width()),
                               qRound(multiPieceIconParams.size.height()));
    info.iPixelFormat = ESgPixelFormatARGB_8888_PRE;
    info.iUsage = ESgUsageBitOpenVgImage | ESgUsageBitOpenVgSurface;

    QScopedPointer<RSgImage, HbSgImageClosure> tempSgImage(new  RSgImage);
    RSgImage *sgImage = tempSgImage.data();
    Q_CHECK_PTR(sgImage);

    TInt err = sgImage->Create(info, 0, 0);
    if (err != KErrNone) {
        return false;
    }

    data.type = SGIMAGE;
    data.sgImageData.width = qRound(multiPieceIconParams.size.width());
    data.sgImageData.height = qRound(multiPieceIconParams.size.height());
    data.sgImageData.defaultWidth = defaultSize.width();
    data.sgImageData.defaultHeight = defaultSize.height();
    //data.sgImageData.sgImage = sgImage;


    TSgDrawableId sgImageId = sgImage->Id();
    memcpy(&data.sgImageData.id, &sgImageId.iId, sizeof(data.sgImageData.id));

    bool success = sgImageRenderer->beginRendering(sgImage);
    if (!success) {
        return false;
    }

    //Clear
    VGfloat color[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
    vgSetfv(VG_CLEAR_COLOR, 4, color);
    vgClear(0, 0, data.sgImageData.width, data.sgImageData.height);

    if (multiPieceIconParams.mirrored) {
        mirrored = true;
        consolidatedIconWidth = multiPieceIconParams.size.toSize().width();
    }

    int iconCount = multiPieceIconParams.multiPartIconList.count();
    for (int i = 0; i < iconCount; i++) {
        HbIconFormatType type = multiPieceIconInfo[i].type;
        bool success = false;

        if (type == NVG) {
#ifdef HB_NVG_CS_ICON
            QPoint position = multiPieceIconParams.multiPartIconData.targets[i].topLeft();
            if (mirrored) {
                int  pieceTopRight = position.x() + multiPieceIconParams.multiPartIconData.targets[i].width();
                position.setX(consolidatedIconWidth - pieceTopRight);
            }


            HbIconSource *source = HbThemeServerUtils::getIconSource(multiPieceIconParams.multiPartIconList[i]);
            QByteArray *sourceByteArray = source->byteArray();
            if( !sourceByteArray ) {
                return false;
            }
            byteArray = *sourceByteArray;
            success = renderNvg(byteArray, QRect(position,
                                                 multiPieceIconParams.multiPartIconData.pixmapSizes[i]), (Qt::AspectRatioMode)multiPieceIconParams.aspectRatioMode,
                                mirrored);
#endif
        }

        if (!success) {
            sgImageRenderer->endRendering();
            return false;
        }
    }
    vgFinish();

    sgImageRenderer->endRendering();

    tempSgImage.take();
    HbSgImageRenderer::addSgImageToHash(data.sgImageData.id, sgImage);
    return true;
}

bool HbSgimageIconProcessor::renderNvg(const QByteArray& byteArray, const QRect& rect, Qt::AspectRatioMode aspectRatioMode, bool mirrored)
{
    QPoint topLeft = rect.topLeft();
    QSize size(rect.size());

    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgLoadIdentity();

    if (!topLeft.isNull()) {
        vgTranslate(topLeft.x(), topLeft.y());
    }

    NvgAspectRatioSettings settings = mapKeyAspectRatioToNvgAspectRatio(aspectRatioMode);
    nvgEngine.setPreserveAspectRatio(settings.nvgAlignStatusAndAspectRatio, settings.type);
    nvgEngine.enableMirroring(mirrored);

    HbNvgEngine::HbNvgErrorType errorType = nvgEngine.drawNvg(byteArray, size);
    return errorType == HbNvgEngine::NvgErrNone;
}

