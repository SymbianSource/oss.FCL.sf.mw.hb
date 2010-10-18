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

#include "hbvgoutlineeffect_p.h"
#include "hbvgoutlineeffect_p_p.h"
#include <QPainter>

/*!
 * \class HbVgOutlineEffect
 *
 * \brief OpenVG-based outline filter effect.
 *
 * \internal
 */

HbVgOutlineEffectPrivate::HbVgOutlineEffectPrivate()
    : outline(1, 1), color(Qt::black), steepness(20), offset(0, 0)
{
}

HbVgOutlineEffect::HbVgOutlineEffect(QObject *parent)
    : HbVgEffect(*new HbVgOutlineEffectPrivate, parent)
{
}

HbVgOutlineEffect::HbVgOutlineEffect(HbVgOutlineEffectPrivate &dd, QObject *parent)
    : HbVgEffect(dd, parent)
{
}

HbVgOutlineEffect::~HbVgOutlineEffect()
{
}

QPointF HbVgOutlineEffect::outline() const
{
    Q_D(const HbVgOutlineEffect);
    return d->outline;
}

void HbVgOutlineEffect::setOutline(const QPointF &outline)
{
    Q_D(HbVgOutlineEffect);
    if (d->outline == outline) {
        return;
    }
    d->outline = outline;
    updateEffectBoundingRect();
    emit outlineChanged(outline);
}

QColor HbVgOutlineEffect::color() const
{
    Q_D(const HbVgOutlineEffect);
    return d->color;
}

void HbVgOutlineEffect::setColor(const QColor &color)
{
    Q_D(HbVgOutlineEffect);
    if (d->color == color) {
        return;
    }
    d->color = color;
    updateEffect();
    emit colorChanged(color);
}

qreal HbVgOutlineEffect::steepness() const
{
    Q_D(const HbVgOutlineEffect);
    return d->steepness;
}

void HbVgOutlineEffect::setSteepness(qreal steepness)
{
    Q_D(HbVgOutlineEffect);
    if (d->steepness == steepness) {
        return;
    }
    d->steepness = steepness;
    updateEffect();
    emit steepnessChanged(steepness);
}

QPointF HbVgOutlineEffect::offset() const
{
    Q_D(const HbVgOutlineEffect);
    return d->offset;
}

void HbVgOutlineEffect::setOffset(const QPointF &offset)
{
    Q_D(HbVgOutlineEffect);
    if (d->offset == offset) {
        return;
    }
    d->offset = offset;
    updateEffectBoundingRect();
    emit offsetChanged(offset);
}

QRectF HbVgOutlineEffect::boundingRectFor(const QRectF &rect) const
{
    Q_D(const HbVgOutlineEffect);
    QSizeF mappedOutline = d->mapSize(QSizeF(d->outline.x() + 2, d->outline.y() + 2));
    QPointF mappedOffset = d->mapOffset(d->offset);

    qreal ox1 = rect.left() - mappedOutline.width() + mappedOffset.x();
    qreal oy1 = rect.top() - mappedOutline.height() + mappedOffset.y();
    qreal ox2 = rect.right() + mappedOutline.width() + mappedOffset.x();
    qreal oy2 = rect.bottom() + mappedOutline.height() + mappedOffset.y();

    qreal x1 = qMin(rect.left(), ox1);
    qreal y1 = qMin(rect.top(), oy1);
    qreal x2 = qMax(rect.right(), ox2);
    qreal y2 = qMax(rect.bottom(), oy2);

    return QRectF(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
}

QPixmap HbVgOutlineEffect::makeOutline(const QVariant &vgImage, const QSize &vgImageSize)
{
#ifdef HB_EFFECTS_OPENVG
    QPixmap cachedPm = cached(vgImageSize);
    if (!cachedPm.isNull()) {
        return cachedPm;
    }

    Q_D(HbVgOutlineEffect);
    VGImage srcImage = vgImage.value<VGImage>();
    VGImage dstImage = d->ensurePixmap(&d->dstPixmap, vgImageSize);

    if (d->paramsChanged) {
        VGubyte stpc = (VGubyte) clamp(d->steepness, 0.0f, 255.0f);
        VGubyte unnormalisedOpacity = (VGubyte)(clamp(d->opacity, 0.0f, 1.0f) * 255.0f);
        for (int i = 0; i < 256; ++i) {
            VGubyte alpha = (i * stpc > unnormalisedOpacity) ? unnormalisedOpacity : i * stpc;
            d->lut[i] = 0x00000000
                        | (d->color.red() << 24)
                        | (d->color.green() << 16)
                        | (d->color.blue() << 8)
                        | alpha;
        }
    }

    QSizeF mappedOutline = d->mapSize(QSizeF(d->outline.x(), d->outline.y()));
    VGfloat outline_x = (VGfloat) clamp(mappedOutline.width(), HBVG_EPSILON, 16.0f);
    VGfloat outline_y = (VGfloat) clamp(mappedOutline.height(), HBVG_EPSILON, 16.0f);
    if (outline_x > HBVG_EPSILON && outline_y > HBVG_EPSILON) {
        VGImage tmpImage = d->ensurePixmap(&d->tmpPixmap, vgImageSize);
        vgGaussianBlur(tmpImage, srcImage, outline_x, outline_y, VG_TILE_PAD);
        vgLookupSingle(dstImage, tmpImage, d->lut, VG_ALPHA, VG_TRUE, VG_FALSE);
    } else {
        vgCopyImage(dstImage, 0, 0, srcImage, 0, 0,
                    vgImageSize.width(), vgImageSize.height(), VG_FALSE);
    }

    tryCache(d->dstPixmap);

    return d->dstPixmap;
#else
    Q_UNUSED(vgImage);
    Q_UNUSED(vgImageSize);
    return QPixmap();
#endif
}

void HbVgOutlineEffect::performEffect(QPainter *painter,
                                      const QPointF &offset,
                                      const QVariant &vgImage,
                                      const QSize &vgImageSize)
{
#ifdef HB_EFFECTS_OPENVG
    Q_D(HbVgOutlineEffect);
    QPixmap outlinePm = makeOutline(vgImage, vgImageSize);
    painter->drawPixmap(offset, outlinePm);
    painter->drawPixmap(offset, d->srcPixmap);
#else
    Q_UNUSED(painter);
    Q_UNUSED(offset);
    Q_UNUSED(vgImage);
    Q_UNUSED(vgImageSize);
#endif
}
