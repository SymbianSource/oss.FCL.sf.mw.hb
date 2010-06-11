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

#include "hbpixmapiconimpl_p.h"

#include <QPixmap>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QBitmap>

#include "hbmemoryutils_p.h"
#include "hbmaskableiconimpl_p.h"
#include "hbpixmapiconrenderer_p.h"

struct HbPixmapMaskedIcon {
    QPixmap    currentPixmap;
};

HbPixmapIconImpl::HbPixmapIconImpl(HbSharedIconInfo iconData, QString &name,
                                   const QSizeF &keySize,
                                   Qt::AspectRatioMode aspectRatioMode,
                                   QIcon::Mode mode,
                                   bool mirrored,
                                   HbRenderingMode renderMode):
    HbIconImpl(iconData,
               name,
               keySize,
               aspectRatioMode,
               mode,
               mirrored,
               renderMode),
    pixmapIconRenderer(0)
{
    retrievePixmapData();
    pixmapIconRenderer = new HbPixmapIconRenderer(pixmapData, this);
}

HbPixmapIconImpl::HbPixmapIconImpl(const QPixmap &pixmap, const QString &name):
    pixmapData(pixmap),
    pixmapIconRenderer(0)
{
    pixmapIconRenderer = new HbPixmapIconRenderer(pixmapData, this);
    aspectRatioMode = Qt::KeepAspectRatio;
    mode = QIcon::Normal;
    mirrored = false;
    createdOnServer = false;
    fileName = name;
}

HbPixmapIconImpl::~HbPixmapIconImpl()
{
    if (pixmapIconRenderer) {
        delete pixmapIconRenderer;
    }
}

QPixmap HbPixmapIconImpl::pixmap()
{
    return pixmapData;
}

QSize HbPixmapIconImpl::defaultSize() const
{
    if (createdOnServer) {
        return defaultIconSize;
    } else {
        return pixmapData.size();
    }
}

void HbPixmapIconImpl::retrievePixmapData()
{
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    QImage image((const uchar *)
                 ((char *)manager->base() + sharedIconData.pixmapData.offset),
                 sharedIconData.pixmapData.width, sharedIconData.pixmapData.height,
                 sharedIconData.pixmapData.format);

    pixmapData = QPixmap::fromImage((image));
    defaultIconSize =
        QSize(sharedIconData.pixmapData.defaultWidth, sharedIconData.pixmapData.defaultHeight);

}

void HbPixmapIconImpl::paint(QPainter *painter,
                             const QRectF &rect,
                             Qt::Alignment alignment,
                             const QPainterPath &clipPath,
                             HbMaskableIconImpl *maskIconData)
{
#ifdef HB_ICON_CACHE_DEBUG
    qDebug() << "HbPixmapIconImpl::paint()-->" << this->fileName;
#endif
    QPointF topLeft = rect.topLeft();
    QSizeF pixmapSize = pixmapData.size();

    if (alignment & Qt::AlignRight) {
        topLeft.setX(rect.right() - pixmapSize.width());
    } else if (alignment & Qt::AlignHCenter) {
        topLeft.setX(topLeft.x() + (rect.width() - pixmapSize.width()) / 2);
    }

    if (alignment & Qt::AlignBottom) {
        topLeft.setY(rect.bottom() - pixmapSize.height());
    } else if (alignment & Qt::AlignVCenter) {
        topLeft.setY(topLeft.y() + (rect.height() - pixmapSize.height()) / 2);
    }

    pixmapIconRenderer->draw(painter, topLeft, clipPath, maskIconData);
}

QSize HbPixmapIconImpl::size()
{
    return pixmapData.size();

}

void HbPixmapIconImpl::destroyMaskedData(HbIconMaskedData *data)
{
    delete((HbPixmapMaskedIcon *) data);
}

