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

#ifndef HBICONSOURCE_P_H
#define HBICONSOURCE_P_H

#ifndef HB_BOOTSTRAPPED
#include "hbglobal.h"
#else
#define HB_CORE_PRIVATE_EXPORT
#endif // HB_BOOTSTRAPPED
#include <QString>
#include <QSizeF>
#include <QSize>

QT_BEGIN_NAMESPACE
class QSvgRenderer;
class QPicture;
class QPixmap;
class QImageReader;
class QByteArray;
QT_END_NAMESPACE

// Pixmaps larger than this amount of bytes are deleted after use from previously used icon sources list
// for not wasting system's memory too much.
static const int PIXMAP_SIZE_LIMIT = 150000;

class HB_CORE_PRIVATE_EXPORT HbIconSource
{
public:
    HbIconSource(const QString &filename);
    HbIconSource(const QString &filename, const QString &type);
    ~HbIconSource();

    QString filename() const;
    QString type();

    QSizeF defaultSize();

    QSvgRenderer *svgRenderer();
    void releaseSvgRenderer();
    void takeSvgRenderer();
    QPicture *picture();
    QImageReader *imageReader();
    void releaseImageReader();
    void takeImageReader();
    QPixmap *pixmap();
    QByteArray *byteArray();

    void deletePixmapIfLargerThan(int limitInBytes);

private:
    bool canKeepOpen() const;

    QString mFilename;
    QString mFullFilename;
    QString mType;

    // These are used to generate the raster pixmap of the icon and for fetching icon's default size
    QPicture   *mPicture;
    QPixmap    *mPixmap;
    QByteArray *mByteArray;

    // Stored default size
    QSize mDefaultSize;

    QSvgRenderer *mSvgRenderer;
    QImageReader *mImageReader;
};

#endif // HBICONSOURCE_P_H
