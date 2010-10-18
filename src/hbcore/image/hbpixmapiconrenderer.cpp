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

#include "hbpixmapiconrenderer_p.h"
#include "hbmaskableiconimpl_p.h"

#include <QStyleOption>
#include <QApplication>

struct HbPixmapIconMaskedData {
    QPixmap    currentPixmap;
};

HbPixmapIconRenderer::HbPixmapIconRenderer(const QPixmap &pixmap,
                                           HbIconImpl *impl,
                                           bool propertiesApplied)
    : iconMode(QIcon::Normal),
      iconPropertiesApplied(propertiesApplied),
      pixmapData(pixmap),
      iconImpl(impl)
{
}

HbPixmapIconRenderer::~HbPixmapIconRenderer()
{
}

void HbPixmapIconRenderer::draw(QPainter *painter,
                                const QPointF &topLeft,
                                const QPainterPath &clipPath,
                                HbMaskableIconImpl *maskIconData)
{
    if (!iconPropertiesApplied) {
        applyIconProperties();
    }

    QPixmap pixmapToDraw = pixmapData;

    if (maskIconData) {
        pixmapToDraw = getMaskedPixmap(maskIconData);
        if (pixmapToDraw.isNull()) {
            pixmapToDraw = pixmapData;
        }
    }

    doDraw(painter, topLeft, pixmapToDraw, clipPath);
}

void HbPixmapIconRenderer::doDraw(QPainter *painter,
                                  const QPointF &topLeft,
                                  const QPixmap &finalPixmap,
                                  const QPainterPath &clipPath)
{
    if (!clipPath.isEmpty()) {
        QPainterPath oldPath;
        bool wasClipped = painter->hasClipping();

        if (!wasClipped) {
            painter->setClipping(true);
        }

        QRectF cliprect = clipPath.boundingRect();
        QPainterPath intersect(clipPath);
        oldPath = painter->clipPath();
        if (wasClipped) {
            QRectF oldrect = oldPath.boundingRect();
            intersect =  oldPath.intersected(clipPath);
            QRectF interrect = intersect.boundingRect();
        }

        painter->setClipPath(intersect, Qt::ReplaceClip);
        painter->drawPixmap(topLeft, finalPixmap);

        painter->setClipPath(oldPath);
        painter->setClipping(wasClipped);
    }  else {
        painter->drawPixmap(topLeft, finalPixmap);
    }
}

void HbPixmapIconRenderer::applyIconProperties()
{
    if (iconMode != QIcon::Normal) {
        QStyleOption opt(0);
        opt.palette = QApplication::palette();
        pixmapData = QApplication::style()->generatedIconPixmap(iconMode, pixmapData, &opt);
    }
    if (iconColor.isValid()) {
        if (!pixmapData.isNull()) {
            QPixmap mask = pixmapData.alphaChannel();
            pixmapData.fill(iconColor);
            pixmapData.setAlphaChannel(mask);
        }
    }
    iconPropertiesApplied = true;
}

QPixmap HbPixmapIconRenderer::getMaskedPixmap(HbMaskableIconImpl *maskIconData)
{
    QPixmap maskedPixmap;

    HbPixmapIconMaskedData *mi = (HbPixmapIconMaskedData *)maskIconData->implData();
    if (maskIconData->maskChanged()) {
        if (!mi) {
            mi = new HbPixmapIconMaskedData();
        }

        mi->currentPixmap = pixmapData;
        mi->currentPixmap.setMask(maskIconData->mask());
        maskIconData->setImplData(mi);
    }

    if (mi) {
        maskedPixmap = mi->currentPixmap;
    }

    return maskedPixmap;
}

void HbPixmapIconRenderer::destroyMaskedData(HbIconMaskedData *data)
{
    delete((HbPixmapIconMaskedData *)data);
}

