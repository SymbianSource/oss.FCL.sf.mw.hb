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

// Macro HB_BOOTSTRAPPED builds a stripped version of HbIconSource that is
// used by ThemeIndexer build tool, to avoid unwanted dependencies.

#include "hbiconsource_p.h"
#ifndef HB_BOOTSTRAPPED
#include "hbiconloader_p.h"
#endif // HB_BOOTSTRAPPED
#include <QtSvg/QSvgRenderer>
#include <QPicture>
#include <QScopedPointer>
#include <QImageReader>
#include <QPixmap>
#include <QDir>
#include <QFile>
#include <QByteArray>
#ifndef HB_BOOTSTRAPPED
#ifdef HB_NVG_CS_ICON
#include "hbnvg_p.h"
#endif // HB_NVG_CS_ICON
#endif // HB_BOOTSTRAPPED

#ifdef HB_BOOTSTRAPPED

// Copied from HbIconLoader::formatFromPath,
// to avoid IconLoader dependency in ThemeIndexer, which needs this source file.
QString iconTypeFromFilename(const QString &iconPath)
{
    QString loweredIconPath = iconPath.toLower();

    if (loweredIconPath.endsWith(".svgz") ) {
        return "SVG";
    }
    if (loweredIconPath.endsWith(".qpic") ) {
        return "PIC";
    }

    if (loweredIconPath.endsWith(".svg")
        || loweredIconPath.endsWith(".png")
        || loweredIconPath.endsWith(".mng")
        || loweredIconPath.endsWith(".gif")
        || loweredIconPath.endsWith(".xpm")
        || loweredIconPath.endsWith(".jpg")
        || loweredIconPath.endsWith(".nvg")) {
        return iconPath.right(3).toUpper();
    }

    if (loweredIconPath.endsWith(".xml")
        || loweredIconPath.endsWith(".axml")
        || loweredIconPath.endsWith(".fxml"))
    {
        return "BLOB";
    }

    return iconPath.section('.', -1);
}

static const qint32 NVG_VIEWBOX_WIDTH_OFS   = 44;
static const qint32 NVG_VIEWBOX_HEIGHT_OFS  = 48;

// Copied from HbNvgEnginePrivate::contentDimensions,
// to avoid NVG dependency in ThemeIndexer
QSize nvgContentDimensions(const QByteArray &buffer)
{
    QSize ret(0,0);
     if (buffer.length() < static_cast<qint32>(NVG_VIEWBOX_HEIGHT_OFS + sizeof (float))){
        ret = QSize(0, 0);
        } 
     
    const quint8* lBuf = (quint8*) buffer.data();
    if((buffer.length() > NVG_VIEWBOX_WIDTH_OFS) && (buffer.length() > NVG_VIEWBOX_HEIGHT_OFS)) {	
	    float lViewboxWidth = * (float*)(lBuf + NVG_VIEWBOX_WIDTH_OFS);
	    float lViewboxHeight = * (float*)(lBuf + NVG_VIEWBOX_HEIGHT_OFS);

	    if (lViewboxWidth > 0 && lViewboxHeight > 0) {
	        ret = QSize((int)lViewboxWidth, (int)lViewboxHeight);
	    } else {
	        ret = QSize(0, 0);
	    }
    }
    return ret;
}


#endif // HB_BOOTSTRAPPED

/*!
  \class HbIconSource
  
  \brief Encapsulates access (size and pixel data reading) to image files.

  \internal
*/

HbIconSource::HbIconSource(const QString &filename) :
    mFilename(filename),
    mPicture(0),
    mPixmap(0),
    mByteArray(0),
    mSvgRenderer(0),
    mImageReader(0)
{
    mFullFilename = QDir(mFilename).absolutePath();
}

HbIconSource::HbIconSource(const QString &filename, const QString &type) :
    mFilename(filename),
    mType(type),
    mPicture(0),
    mPixmap(0),
    mByteArray(0),
    mSvgRenderer(0),
    mImageReader(0)
{
    mFullFilename = QDir(mFilename).absolutePath();
}

HbIconSource::~HbIconSource()
{
    delete mImageReader;
    delete mSvgRenderer;
    delete mPicture;
    delete mPixmap;
    delete mByteArray;
}

QString HbIconSource::filename() const
{
    return mFilename;
}

QString HbIconSource::type()
{
#ifndef HB_BOOTSTRAPPED
    if (mType.isEmpty()) {
        mType = HbIconLoader::formatFromPath(mFilename);
    }
#else
    mType = iconTypeFromFilename(mFilename);
#endif // HB_BOOTSTRAPPED
    return mType;
}

QSizeF HbIconSource::defaultSize()
{
    // If the default size has not been fetched yet, do it now.
    if (!mDefaultSize.isValid()) {
        type(); // make sure type is initialized
        if (mType == "NVG") {
#ifndef HB_BOOTSTRAPPED
#ifdef HB_NVG_CS_ICON
        if(!mByteArray){
            QFile file(mFilename);
            if (!file.open(QIODevice::NotOpen | QIODevice::ReadOnly)) {
                return QSizeF();
            }                
            mByteArray = new QByteArray(file.readAll());
        }
        HbNvgEngine nvgEngine; 
        mDefaultSize = nvgEngine.contentDimensions(*mByteArray);
#endif // HB_NVG_CS_ICON
#else // HB_BOOTSTRAPPED

        if(!mByteArray){
            QFile file(mFilename);
            if (!file.open(QIODevice::NotOpen | QIODevice::ReadOnly)) {
                return QSizeF();
            }                
            mByteArray = new QByteArray (file.readAll());
        }

        mDefaultSize = nvgContentDimensions(*mByteArray);
#endif
        }
        else if (mType == "SVG") {
            QSvgRenderer *renderer = svgRenderer();
            if (renderer) { // isValid() is already checked in svgRenderer()
                mDefaultSize = renderer->defaultSize();
            }
            releaseSvgRenderer();
        }
        else if (mType == "PIC") {
            if (!mPicture) {
                mPicture = new QPicture;
                mPicture->load(mFilename);
            }
            mDefaultSize = mPicture->boundingRect().size();
        }
        // Image reader supports getting the default size without rasterizing the image so
        // using it with the formats that it supports.
        else if (mType == "MNG" || mType == "GIF" || mType == "JPG" || mType == "PNG") {
            // Note that QImageReader::canRead() results in opening the file and so the
            // file will be locked until the QImageReader instance is
            // destroyed. Therefore the image reader instance must be destroyed as soon
            // as possible and must not be kept for later use.
            // Exception: Files on Z drive on Symbian. See canKeepOpen() and releaseImageReader().
            QImageReader *reader = imageReader();
            if (reader) { // canRead() is already checked in imageReader()
                if (mType != "MNG") {
                    mDefaultSize = reader->size();
                } else {
                    // MNG handler does not support size query so have to render it to get the size
                    QImage img = reader->read();
                    mDefaultSize = img.size();
                }
            }
            releaseImageReader();
        }
        else if (mType != "BLOB") {
            if (!mPixmap) {
                mPixmap = new QPixmap(mFilename);
            }
            mDefaultSize = mPixmap->size();
        }
    }

    return QSizeF(mDefaultSize);
}

/*!  Returns a QSvgRenderer instance. Ownership is not transferred to the
  caller. However make sure to call releaseSvgRenderer() as soon as possible.
 */
QSvgRenderer *HbIconSource::svgRenderer()
{
    if (!mSvgRenderer) {
        mSvgRenderer = new QSvgRenderer(mFilename);
    }
    return mSvgRenderer && mSvgRenderer->isValid() ? mSvgRenderer : 0;
}

/*!
    Must be called after the pointer returned by svgRenderer() is not needed by the client anymore.
*/
void HbIconSource::releaseSvgRenderer()
{
    if (mSvgRenderer && !canKeepOpen()) {
        delete mSvgRenderer;
        mSvgRenderer = 0;
    }
}

/*!
    Transfers the ownership of the pointer returned by svgRenderer() to the client.
*/
void HbIconSource::takeSvgRenderer()
{
    mSvgRenderer = 0;
}
    
QByteArray* HbIconSource::byteArray()
{
    if (!mByteArray) {
    #ifdef HB_NVG_CS_ICON
        QFile file(mFilename);
        if (!file.open(QIODevice::NotOpen | QIODevice::ReadOnly)) {
            return 0;
        }
        mByteArray = new QByteArray (file.readAll());
    #endif//nvg 
    }
    
    if (!mByteArray->isEmpty()) {
        return mByteArray;
    } else {
        return 0;
    }
}

QPicture *HbIconSource::picture()
{
    if (!mPicture) {
        mPicture = new QPicture;
        mPicture->load(mFilename);
    }

    return mPicture;
}

/*!  Returns a QImageReader instance. Ownership is not transferred to the
  caller. However make sure to call releaseImageReader() as soon as possible.
 */
QImageReader *HbIconSource::imageReader()
{
    if (!mImageReader) {
        mImageReader = new QImageReader(mFilename, mType.toLatin1());
    }
    return mImageReader && mImageReader->canRead() ? mImageReader : 0;
}

/*!
    Must be called after the pointer returned by imageReader() is not needed by the client anymore.
*/
void HbIconSource::releaseImageReader()
{
    if (mImageReader && !canKeepOpen()) {
        delete mImageReader;
        mImageReader = 0;
    }
}

/*!
    Transfers the ownership of the pointer returned by imageReader() to the client.
*/
void HbIconSource::takeImageReader()
{
    mImageReader = 0;
}

QPixmap *HbIconSource::pixmap()
{
    if (!mPixmap) {
        mPixmap = new QPixmap(mFilename);
    }

    return mPixmap;
}

void HbIconSource::deletePixmapIfLargerThan(int limitInBytes)
{
    if (mPixmap) {
        QSize size = mPixmap->size();
        int sizeInBytes = size.width() * size.height() * mPixmap->depth();
        sizeInBytes /= 8; // depth is in bits, we want bytes

        if (sizeInBytes > limitInBytes) {
            delete mPixmap;
            mPixmap = 0;
        }
    }
}

bool HbIconSource::canKeepOpen() const
{
#ifdef Q_OS_SYMBIAN
    if (mFullFilename.isEmpty()) {
        return false;
    } else {
        QChar driveLetter = mFullFilename.at(0);
        return driveLetter.toUpper() == 'Z';
    }
#else
    return false;
#endif
}
