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

#include "hbvgbceffect_p.h"
#include "hbvgbceffect_p_p.h"
#include <QPainter>

/*!
 * \class HbVgBcEffect
 *
 * \brief OpenVG-based brightness-contrast filter effect.
 *
 * \internal
 */

HbVgBcEffectPrivate::HbVgBcEffectPrivate()
    : brightness(0), contrast(1)
{
}

HbVgBcEffect::HbVgBcEffect(QObject *parent)
    : HbVgEffect(*new HbVgBcEffectPrivate, parent)
{
}

HbVgBcEffect::HbVgBcEffect(HbVgBcEffectPrivate &dd, QObject *parent)
    : HbVgEffect(dd, parent)
{
}

HbVgBcEffect::~HbVgBcEffect()
{
}

qreal HbVgBcEffect::brightness() const
{
    Q_D(const HbVgBcEffect);
    return d->brightness;
}

void HbVgBcEffect::setBrightness(qreal brightness)
{
    Q_D(HbVgBcEffect);
    if (d->brightness == brightness) {
        return;
    }
    d->brightness = brightness;
    updateEffect();
    emit brightnessChanged(brightness);
}

qreal HbVgBcEffect::contrast() const
{
    Q_D(const HbVgBcEffect);
    return d->contrast;
}

void HbVgBcEffect::setContrast(qreal contrast)
{
    Q_D(HbVgBcEffect);
    if (d->contrast == contrast) {
        return;
    }
    d->contrast = contrast;
    updateEffect();
    emit contrastChanged(contrast);
}

QRectF HbVgBcEffect::boundingRectFor(const QRectF &rect) const
{
    return rect;
}

void HbVgBcEffect::performEffect(QPainter *painter,
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

    Q_D(HbVgBcEffect);
    VGImage srcImage = vgImage.value<VGImage>();
    VGImage dstImage = d->ensurePixmap(&d->dstPixmap, vgImageSize);
    qreal opacity = clamp(d->opacity, 0.0f, 1.0f);
    if (opacity > HBVG_EPSILON) {
        if (d->paramsChanged) {
            // brightness [-1, 1]
            const VGfloat offset_br = clamp(d->brightness, -1.0f, 1.0f);
            const VGfloat scale_br = 1.0f - 0.5f * ((offset_br < 0.0f) ? -offset_br : offset_br);

            // contrast [0, N]
            const VGfloat scale_con = clamp(d->contrast, 0.0f, 100.0f);
            const VGfloat offset_con = -0.5f * scale_con + 0.5f ;

            // combine the effects of brightness and contrast
            const VGfloat off = offset_br + offset_con;
            const VGfloat sc  = scale_br * scale_con;

            // take opacity into account
            const VGfloat o = (VGfloat) opacity;
            const VGfloat oOff = off * o;
            const VGfloat oSc  = (sc * o) + (1.0f - o);

            d->colorMatrix[0] = oSc;
            d->colorMatrix[1] = 0.0f;
            d->colorMatrix[2] = 0.0f;
            d->colorMatrix[3] = 0.0f;
            d->colorMatrix[4] = 0.0f;
            d->colorMatrix[5] = oSc;
            d->colorMatrix[6] = 0.0f;
            d->colorMatrix[7] = 0.0f;
            d->colorMatrix[8] = 0.0f;
            d->colorMatrix[9] = 0.0f;
            d->colorMatrix[10] = oSc;
            d->colorMatrix[11] = 0.0f;
            d->colorMatrix[12] = 0.0f;
            d->colorMatrix[13] = 0.0f;
            d->colorMatrix[14] = 0.0f;
            d->colorMatrix[15] = 1.0f;
            d->colorMatrix[16] = oOff;
            d->colorMatrix[17] = oOff;
            d->colorMatrix[18] = oOff;
            d->colorMatrix[19] = 0.0f;
        }
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
