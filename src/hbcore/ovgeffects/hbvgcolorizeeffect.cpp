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

#include "hbvgcolorizeeffect_p.h"
#include "hbvgcolorizeeffect_p_p.h"
#include <QPainter>

/*!
 * \class HbVgColorizeEffect
 *
 * \brief OpenVG-based colorize filter effect.
 *
 * \internal
 */

HbVgColorizeEffectPrivate::HbVgColorizeEffectPrivate()
    : color(Qt::white)
{
}

HbVgColorizeEffect::HbVgColorizeEffect(QObject *parent)
    : HbVgEffect(*new HbVgColorizeEffectPrivate, parent)
{
}

HbVgColorizeEffect::HbVgColorizeEffect(HbVgColorizeEffectPrivate &dd, QObject *parent)
    : HbVgEffect(dd, parent)
{
}

HbVgColorizeEffect::~HbVgColorizeEffect()
{
}

QColor HbVgColorizeEffect::color() const
{
    Q_D(const HbVgColorizeEffect);
    return d->color;
}

void HbVgColorizeEffect::setColor(const QColor &color)
{
    Q_D(HbVgColorizeEffect);
    if (d->color == color)
        return;
    d->color = color;
    updateEffect();
    emit colorChanged(color);
}

QRectF HbVgColorizeEffect::boundingRectFor(const QRectF &rect) const
{
    return rect;
}

#ifdef HB_EFFECTS_OPENVG

const VGfloat Rw = 0.3086f;
const VGfloat Gw = 0.6094f;
const VGfloat Bw = 0.0820f;

void HbVgColorizeEffectPrivate::getColorMatrix(VGfloat *colorMatrix,
                                               const QColor &color,
                                               qreal opacity)
{
    const VGfloat o = (VGfloat) opacity;
    const VGfloat ao = 1 - o;
    const VGfloat R = (o / 255.0f) * (VGfloat) color.red();
    const VGfloat G = (o / 255.0f) * (VGfloat) color.green();
    const VGfloat B = (o / 255.0f) * (VGfloat) color.blue();
    colorMatrix[0] = R*Rw + ao;
    colorMatrix[1] = G*Rw;
    colorMatrix[2] = B*Rw;
    colorMatrix[3] = 0.0f;
    colorMatrix[4] = R*Gw;
    colorMatrix[5] = G*Gw + ao;
    colorMatrix[6] = B*Gw;
    colorMatrix[7] = 0.0f;
    colorMatrix[8] = R*Bw;
    colorMatrix[9] = G*Bw;
    colorMatrix[10] = B*Bw + ao;
    colorMatrix[11] = 0.0f;
    colorMatrix[12] = 0.0f;
    colorMatrix[13] = 0.0f;
    colorMatrix[14] = 0.0f;
    colorMatrix[15] = 1.0f;
    colorMatrix[16] = 0.0f;
    colorMatrix[17] = 0.0f;
    colorMatrix[18] = 0.0f;
    colorMatrix[19] = 0.0f;    
}

#endif

void HbVgColorizeEffect::performEffect(QPainter *painter,
                                   const QPointF &offset,
                                   const QVariant &vgImage,
                                   const QSize &vgImageSize)
{
#ifdef HB_EFFECTS_OPENVG
    QPixmap cachedPm = cached(vgImageSize);
    if (!cachedPm.isNull()) {
        painter->drawPixmap(offset, cachedPm);
        return;
    }

    Q_D(HbVgColorizeEffect);
    VGImage srcImage = vgImage.value<VGImage>();
    VGImage dstImage = d->ensurePixmap(&d->dstPixmap, vgImageSize);
    qreal opacity = clamp(d->opacity, 0.0f, 1.0f);
    if (opacity > HBVG_EPSILON) {
        if (d->paramsChanged)
            HbVgColorizeEffectPrivate::getColorMatrix(d->colorMatrix, d->color, opacity);
        vgColorMatrix(dstImage, srcImage, d->colorMatrix);
        painter->drawPixmap(offset, d->dstPixmap);
        tryCache(d->dstPixmap);
    } else {
        painter->drawPixmap(offset, d->srcPixmap);
    }
#else
    Q_UNUSED(painter);
    Q_UNUSED(offset);
    Q_UNUSED(vgImage);
    Q_UNUSED(vgImageSize);
#endif
}
