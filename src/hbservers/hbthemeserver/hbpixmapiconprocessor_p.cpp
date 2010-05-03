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

#include "hbpixmapiconprocessor_p.h"

#include <QStyleOption>
#include <QApplication>
#include <QSvgRenderer>
#include <QPainter>
#include <QPicture>
#include <QFile>
#include "hbpixmapiconimpl_p.h"
#include "hbmemoryutils_p.h"
#include "hbiconsource_p.h"
#include "hbthemeserverutils_p.h"

#if defined (Q_OS_SYMBIAN)
#include <VG/openvg.h>
#include <VG/vgcontext_symbian.h>
#endif //Q_OS_SYMBIAN

/*!
    @hbserver
    \class HbPixmapIconProcessor
    \brief HbPixmapIconProcessor is responsible for retrieving the pixmap raster data from the icon file,
    and copy the icon data to the cpu shared memory.

*/

// Constants
static const int HB_BITS_PER_BYTE = 8;

/*!
    \fn HbPixmapIconProcessor::HbPixmapIconProcessor()
    Constructor
    \a key denotes the unique identifier for the cache item
    \a options indicate different ways of loading icons
    \a format indicates the icon format e.g. svg/nvg etc.
 */
HbPixmapIconProcessor::HbPixmapIconProcessor(
    const HbIconKey &key,
    const HbIconLoader::IconLoaderOptions &options,
    const QString &type)
{
    iconKey = key;
    iconOptions = options;
    iconType = type;
    defaultSize = QSize(0, 0);
}

/*!
    \fn HbPixmapIconProcessor::~HbPixmapIconProcessor()
    Destructor
 */
HbPixmapIconProcessor::~HbPixmapIconProcessor()
{
}

/*!
    \fn HbPixmapIconProcessor::sharedIconDataCost()
    Returns the size of the pixel data in bytes.
 */
int HbPixmapIconProcessor::sharedIconDataCost() const
{
    int width = pixmap.size().width();
    int height = pixmap.size().height();
    int depth = pixmap.depth() / HB_BITS_PER_BYTE; // depth in bytes
    return(width * height * depth);

}
/*!
    \fn HbPixmapIconProcessor::sharedIconData()
    Copies the pixmap raster data to the shared memory
 */
HbSharedIconInfo HbPixmapIconProcessor::sharedIconData()const
{
    HbSharedIconInfo data;
    data.type = INVALID_FORMAT;
    const QImage image = pixmap.toImage();

    if (!image.isNull()) {
        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
        try {
            //alloc function of memory manager can throw
            data.pixmapData.offset =  manager->alloc(image.numBytes());
            data.type = OTHER_SUPPORTED_FORMATS;
            data.pixmapData.format = image.format();
            data.pixmapData.width = image.size().width();
            data.pixmapData.height = image.size().height();
            data.pixmapData.defaultWidth = defaultSize.width();
            data.pixmapData.defaultHeight = defaultSize.height();
            memcpy((char *)manager->base() + data.pixmapData.offset, image.bits(), image.numBytes());
        } catch (std::bad_alloc &badAlloc) {
            Q_UNUSED(badAlloc)
        }
    }
    return data;
}

/*!
    \fn HbPixmapIconProcessor::createPixmapData()
    \a iconpath
 */
bool HbPixmapIconProcessor::createIconData(const QString& iconPath)
{
    bool isIconCreated = false;
    if (iconType == "SVG") {
        isIconCreated = renderSvgToPixmap(iconPath);
    } else if (iconType == "PIC") {
        isIconCreated = renderPicToPixmap(iconPath);
    } else if (iconType == "NVG") {
#if defined (Q_OS_SYMBIAN)    
		isIconCreated = renderNvgToPixmap(iconPath);
#endif //Q_OS_SYMBIAN
    } else {
        isIconCreated = renderOtherFormatsToPixmap(iconPath);
    }
    if (isIconCreated) {
        // Apply mirroring if required
        if (iconKey.mirrored) {
            QTransform t;
            t.scale(-1, 1);
            pixmap = pixmap.transformed(t);
        }

        // Apply mode
        if (iconKey.mode != QIcon::Normal) {
            QStyleOption opt(0);
            opt.palette = QApplication::palette();
            pixmap = QApplication::style()->generatedIconPixmap(iconKey.mode, pixmap, &opt);
        }
        /* Apply color if valid*/

        if (iconKey.color.isValid() && (iconKey.mode != QIcon::Disabled)) {
            if (!pixmap.isNull()) {
                QPixmap mask = pixmap.alphaChannel();
                pixmap.fill(iconKey.color);
                pixmap.setAlphaChannel(mask);
            }
        }
    }

    return isIconCreated;
}

/*!
    \fn HbPixmapIconProcessor::renderSvgToPixmap()
    \a iconpath
 */
bool HbPixmapIconProcessor::renderSvgToPixmap(const QString& iconPath)
{
    bool isIconCreated = false;
    if (!iconKey.size.isValid()) {
        return isIconCreated;
    }

    bool isDefaultSize =  iconKey.size.isNull();

    HbIconSource *source = HbThemeServerUtils::getIconSource(iconPath);
    QSvgRenderer *svgRenderer = source->svgRenderer();

    if (svgRenderer && svgRenderer->isValid()) {
        QSizeF renderSize = QSizeF(svgRenderer->defaultSize());
        //Save the default size
        defaultSize = renderSize.toSize();
        if (!isDefaultSize) {
            renderSize.scale(iconKey.size, iconKey.aspectRatioMode);
        } else if (iconOptions.testFlag(HbIconLoader::ResolutionCorrected)) {
            applyResolutionCorrection(renderSize);
        }
        pixmap = QPixmap(renderSize.toSize());
        pixmap.fill(Qt::transparent);

        // creating raster data
        QPainter painter;
        painter.begin(&pixmap);
        svgRenderer->render(&painter, QRectF(QPointF(), renderSize.toSize()));
        painter.end();
        isIconCreated = true;
    }
    
    source->releaseSvgRenderer();
    return isIconCreated;
}

/*!
    \fn HbPixmapIconProcessor::renderPicToPixmap()
    \a iconpath
 */
bool HbPixmapIconProcessor::renderPicToPixmap(const QString& iconPath)

{
    bool isIconCreated = false;
    if (!iconKey.size.isValid()) {
        return isIconCreated;
    }
    bool isDefaultSize =  iconKey.size.isNull();

    HbIconSource *source = HbThemeServerUtils::getIconSource(iconPath);
    QPicture *picture = source->picture();

    QSizeF picSize = QSizeF(picture->boundingRect().size());
    if (picSize.isNull()) {
        return isIconCreated;
    }
    QSizeF renderSize(picSize);

    qreal sx = 1.0;
    qreal sy = 1.0;
    bool scale = false;

    //Save the default size
    defaultSize = picSize.toSize();
    if (!isDefaultSize) {
        scale = true;
        renderSize.scale(iconKey.size, iconKey.aspectRatioMode);
    } else if (iconOptions.testFlag(HbIconLoader::ResolutionCorrected)) {
        applyResolutionCorrection(renderSize);
    }

    if (scale) {
        // Determine scale factor as QPicture doesn't allow for scaling
        sx = renderSize.width() / picSize.width();
        sy = renderSize.height() / picSize.height();
    }

    pixmap = QPixmap(renderSize.toSize());
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    if (scale) {
        painter.scale(sx, sy);
    }
    painter.drawPicture(QPointF(0.0, 0.0), *picture);
    painter.end();
    isIconCreated = true;
    return isIconCreated;
}

/*!
    \fn HbPixmapIconProcessor::renderOtherFormatsToPixmap()
    \a iconpath
 */
bool HbPixmapIconProcessor::renderOtherFormatsToPixmap(const QString& iconPath)
{
    bool isIconCreated = false;
    if (!iconKey.size.isValid()) {
        return isIconCreated;
    }

    HbIconSource *source = HbThemeServerUtils::getIconSource(iconPath);
    pixmap = *source->pixmap();

    if (pixmap.isNull()) {
        return isIconCreated;
    }

    bool isDefaultSize =  iconKey.size.isNull();
    //Save the default size
    defaultSize = pixmap.size();
    if (!isDefaultSize) {

#ifdef ENABLE_EXPERIMENTAL_RESIZE_BOOST__

        // This test implementation improves resize speed up to 5 times..
        // Smooth scaling is very expensive (size^2). Therefore we reduce the size
        // to 1.5 of the destination size and using fast transformation.
        // Therefore we speed up but don't loose quality..
        if (pixmap.size().width() > (4 * iconKey.size.width())) {
            // Improve scaling speed by add an intermediate fast transformation..
            // qDebug() << "Smooth scaling now from size:" << canvasPixmap.size() << "to size:" << size.toSize();
            QSize intermediate_size = QSize(iconKey.size.width() * 2, iconKey.size.height() * 2);
            pixmap = pixmap.scaled(intermediate_size,
                                   aspectRatioMode,
                                   Qt::FastTransformation);  // Cheap operation!
        }
#endif // ENABLE_EXPERIMENTAL_RESIZE_BOOST__
        QPixmap newcanvasPixmap = pixmap.scaled(iconKey.size.toSize(),
                                                iconKey.aspectRatioMode,
                                                Qt::SmoothTransformation);  // Expensive operation!

        if (newcanvasPixmap.size().isEmpty()) {
            pixmap = pixmap.scaled(iconKey.size.toSize(),
                                   iconKey.aspectRatioMode,
                                   Qt::SmoothTransformation);  // Expensive operation!
        } else {
            pixmap = newcanvasPixmap;
        }
    }
    isIconCreated = true;

    // Delete original pixmap if its size is large
    source->deletePixmapIfLargerThan(PIXMAP_SIZE_LIMIT);

    return isIconCreated;
}


/*!
    \fn HbPixmapIconProcessor::setPixmap()
    \a pixmap
 */
void HbPixmapIconProcessor::setPixmap(const QPixmap & pixmap)
{
    this->pixmap = pixmap;
}

/*!
    \fn HbPixmapIconProcessor::createMultiPieceIconData()
    \a multiPieceIconInfo
    \a multiPieceIconParams
 */
bool HbPixmapIconProcessor::createMultiPieceIconData(const QVector<HbSharedIconInfo> &multiPieceIconInfo,
        HbMultiIconParams &multiPieceIconParams)
{
    QPixmap finalPixmap((int)multiPieceIconParams.size.width(), (int)multiPieceIconParams.size.height());
    finalPixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&finalPixmap);


    for (int i = 0; i < multiPieceIconParams.multiPartIconList.count(); i++) {

        if ((multiPieceIconInfo[i].type != INVALID_FORMAT) && multiPieceIconInfo[i].type == OTHER_SUPPORTED_FORMATS) {
            GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
            QImage image((const uchar*)
                         ((char *)manager->base() + multiPieceIconInfo[i].pixmapData.offset),
                         multiPieceIconParams.multiPartIconData.pixmapSizes[i].width(), 
                         multiPieceIconParams.multiPartIconData.pixmapSizes[i].height(),
                         multiPieceIconInfo[i].pixmapData.format);

            QPixmap pixmap = QPixmap::fromImage((image));            
            painter.drawPixmap(multiPieceIconParams.multiPartIconData.targets[i].topLeft(), pixmap, pixmap.rect());
        }
    }

    painter.end();
    if (multiPieceIconParams.mirrored) {
        QTransform t;
        t.scale(-1, 1);
        finalPixmap = finalPixmap.transformed(t);
    }
    setPixmap(finalPixmap);
    return true;
}

#if defined (Q_OS_SYMBIAN)
/**
 * HbNvgIconProcessor::renderNvgToPixmap()
 * This is used to render NVG data to a pixmap using the Software OpenVG
 * \a iconPath
 */
bool HbPixmapIconProcessor::renderNvgToPixmap(const QString& iconPath)
{
    bool isIconCreated = false;
    bool isDefaultSize =  iconKey.size.isNull();
    HbIconSource *source = HbThemeServerUtils::getIconSource(iconPath);
    QByteArray *sourceByteArray = source->byteArray();
    if( !sourceByteArray ) {
        return false;
    }
    QByteArray byteArray = *sourceByteArray;
    QSizeF renderSize = source->defaultSize();
    defaultSize = renderSize.toSize();
    if (!isDefaultSize) {
        renderSize.scale(iconKey.size,iconKey.aspectRatioMode);
    }
    size = renderSize.toSize();          
    TSize surfaceSize(TSize(size.width(), size.height()));

    QScopedPointer<CFbsBitmap> bitmapData(new CFbsBitmap());    
    
    TInt err = bitmapData.data()->Create(surfaceSize, EColor16MA);    
    if(err != KErrNone) {      
        return isIconCreated;
    }
    
    //Reset the surface incase already present
    VGISymbianTerminate();
    
    // Surface creation
    err =  VGISymbianInitialize( surfaceSize, VGI_COLORSPACE_SRGB );
    if( err != KErrNone) {
        return isIconCreated;
    }
    
    QScopedPointer<CNvgEngine> nvgEngine(CNvgEngine::NewL());
    //CNvgEngine* nvgEngine = CNvgEngine::NewL();
    HbNvgAspectRatioSettings settings = mapKeyAspectRatioToNvgAspectRatio(iconKey.aspectRatioMode);
    nvgEngine.data()->SetPreserveAspectRatio(settings.nvgAlignStatusAndAspectRatio, settings.type);
    // Rendering onto active surface
    TPtr8 data ((unsigned char*)byteArray.data(), byteArray.length(), byteArray.length());
    err = nvgEngine.data()->DrawNvg(data, surfaceSize, bitmapData.data(), 0);    
    if(err !=KErrNone) {  
        return isIconCreated;
    }
    
    //Copy the data from the surface
    err = VGISymbianCopyToBitmap(bitmapData.data(), 0, VGI_COPY_TRANSPARENT_PIXELS);
    if(err !=KErrNone) { 
        return isIconCreated;
    }
    //Get Pixmap from the Symbian Native format.
    pixmap = QPixmap::fromSymbianCFbsBitmap(bitmapData.data());
    isIconCreated = true;    
        
    //Clean Up
    VGISymbianTerminate();
    return isIconCreated;
}

/*!
    \fn HbPixmapIconProcessor::mapKeyAspectRatioToNvgAspectRatio()
    \a aspectRatio
 */
HbNvgAspectRatioSettings HbPixmapIconProcessor::mapKeyAspectRatioToNvgAspectRatio(
                                                Qt::AspectRatioMode aspectRatio) const
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
#endif //Q_OS_SYMBIAN

