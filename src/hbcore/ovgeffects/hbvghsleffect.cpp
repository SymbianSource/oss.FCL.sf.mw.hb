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

#include "hbvghsleffect_p.h"
#include "hbvghsleffect_p_p.h"
#include <QPainter>
#include <qmath.h>

/*!
 * \class HbVgHslEffect
 *
 * \brief OpenVG-based hue-saturation-lightness filter effect.
 *
 * \internal
 */

HbVgHslEffectPrivate::HbVgHslEffectPrivate()
    : hue(0), saturation(1), lightness(0)
{
}

HbVgHslEffect::HbVgHslEffect(QObject *parent)
    : HbVgEffect(*new HbVgHslEffectPrivate, parent)
{
}

HbVgHslEffect::HbVgHslEffect(HbVgHslEffectPrivate &dd, QObject *parent)
    : HbVgEffect(dd, parent)
{
}

HbVgHslEffect::~HbVgHslEffect()
{
}

qreal HbVgHslEffect::hue() const
{
    Q_D(const HbVgHslEffect);
    return d->hue;
}

void HbVgHslEffect::setHue(qreal hue)
{
    Q_D(HbVgHslEffect);
    if (d->hue == hue)
        return;
    d->hue = hue;
    updateEffect();
    emit hueChanged(hue);
}

qreal HbVgHslEffect::saturation() const
{
    Q_D(const HbVgHslEffect);
    return d->saturation;
}

void HbVgHslEffect::setSaturation(qreal saturation)
{
    Q_D(HbVgHslEffect);
    if (d->saturation == saturation)
        return;
    d->saturation = saturation;
    updateEffect();
    emit saturationChanged(saturation);
}

qreal HbVgHslEffect::lightness() const
{
    Q_D(const HbVgHslEffect);
    return d->lightness;
}

void HbVgHslEffect::setLightness(qreal lightness)
{
    Q_D(HbVgHslEffect);
    if (d->lightness == lightness)
        return;
    d->lightness = lightness;
    updateEffect();
    emit lightnessChanged(lightness);
}

void HbVgHslEffect::desaturate()
{
    setHue(0);
    setSaturation(0);
    setLightness(0);
}

QRectF HbVgHslEffect::boundingRectFor(const QRectF &rect) const
{
    return rect;
}

#ifdef HB_EFFECTS_OPENVG

inline void getSaturationRotationMatrix(VGfloat *effectMatrix, VGfloat opacity,
                                        VGfloat saturation, VGfloat angle) 
{
    const VGfloat sa = saturation;            
    const VGfloat as = 1.0f - saturation;
    
    const VGfloat o = opacity;
    const VGfloat ao = 1.0f - o;
    
    const VGfloat c = qCos(angle);
    const VGfloat s = qSin(angle);
    
    effectMatrix[0] = o * ((-0.02473f*as+0.66667f*sa)*c+ (0.30450f*as*s+(0.33333f*as+0.33333f*sa))) + ao;
    effectMatrix[1] = o * ((-0.02473f*as-0.33333f*sa)*c+((0.30450f*as+0.57736f*sa)*s+(0.33333f*as+0.33333f*sa)));
    effectMatrix[2] = o * ((-0.02473f*as-0.33333f*sa)*c+((0.30450f*as-0.57736f*sa)*s+(0.33333f*as+0.33333f*sa)));
    effectMatrix[3] = 0.0f;
    effectMatrix[4] = o * ((0.27607f*as-0.33333f*sa)*c+((-0.13083f*as-0.57736f*sa)*s+(0.33333f*as+0.33333f*sa)));
    effectMatrix[5] = o * ((0.27607f*as+0.66667f*sa)*c+ (-0.13083f*as*s+(0.33333f*as+0.33333f*sa))) + ao;
    effectMatrix[6] = o * ((0.27607f*as-0.33333f*sa)*c+((-0.13083f*as+0.57736f*sa)*s+(0.33333f*as+0.33333f*sa)));
    effectMatrix[7] = 0.0f;
    effectMatrix[8] = o * ((-0.25134f*as-0.33333f*sa)*c+((-0.17367f*as+0.57736f*sa)*s+(0.33333f*as+0.33333f*sa)));
    effectMatrix[9] = o * ((-0.25134f*as-0.33333f*sa)*c+((-0.17367f*as-0.57736f*sa)*s+(0.33333f*as+0.33333f*sa)));
    effectMatrix[10] = o * ((-0.25134f*as+0.66667f*sa)*c+ (-0.17367f*as*s+(0.33333f*as+0.33333f*sa))) + ao;
    effectMatrix[11] = 0.0f;
    effectMatrix[12] = 0.0f;
    effectMatrix[13] = 0.0f;
    effectMatrix[14] = 0.0f;
    effectMatrix[15] = 1.0f;
}

const VGfloat Rw = 0.3086f;
const VGfloat Gw = 0.6094f;
const VGfloat Bw = 0.0820f;

inline void getSaturationMatrix(VGfloat *effectMatrix, VGfloat opacity, VGfloat saturation)
{
    const VGfloat sa = saturation;            
    const VGfloat as = 1.0f - saturation;
    
    const VGfloat o = opacity;
    const VGfloat ao = 1.0f - o;

    const VGfloat asRw = o * as * Rw;
    const VGfloat asGw = o * as * Gw;
    const VGfloat asBw = o * as * Bw;
    
    effectMatrix[0] = asRw + sa + ao;
    effectMatrix[1] = asRw;
    effectMatrix[2] = asRw;
    effectMatrix[3] = 0.0f;
    effectMatrix[4] = asGw;
    effectMatrix[5] = asGw + sa + ao;
    effectMatrix[6] = asGw;
    effectMatrix[7] = 0.0f;
    effectMatrix[8] = asBw;
    effectMatrix[9] = asBw;
    effectMatrix[10] = asBw + sa + ao;
    effectMatrix[11] = 0.0f;
    effectMatrix[12] = 0.0f;
    effectMatrix[13] = 0.0f;
    effectMatrix[14] = 0.0f;
    effectMatrix[15] = 1.0f;
}

inline void getRotationMatrix(VGfloat *effectMatrix, VGfloat opacity, VGfloat angle)
{
    const VGfloat o = opacity;
    const VGfloat ao = 1.0f - o;
    
    const VGfloat c = qCos(angle);
    const VGfloat s = qSin(angle);
    
    effectMatrix[0] = o * ( 0.66667f*c+0.33333f) + ao;
    effectMatrix[1] = o * (-0.33333f*c+(0.57736f*s+0.33333f));
    effectMatrix[2] = o * (-0.33333f*c+(-0.57736f*s+0.33333f));
    effectMatrix[3] =  0.0f;
    effectMatrix[4] = o * (-0.33333f*c+(-0.57736f*s+0.33333f));
    effectMatrix[5] = o * ( 0.66667f*c+0.33333f) + ao;
    effectMatrix[6] = o * (-0.33333f*c+(0.57736f*s+0.33333f));
    effectMatrix[7] =  0.0f;
    effectMatrix[8] = o * (-0.33333f*c+(0.57736f*s+0.33333f));
    effectMatrix[9] = o * (-0.33333f*c+(-0.57736f*s+0.33333f));
    effectMatrix[10] = o * (0.66667f*c+0.33333f) + ao;
    effectMatrix[11] = 0.0f;
    effectMatrix[12] = 0.0f;
    effectMatrix[13] = 0.0f;
    effectMatrix[14] = 0.0f;
    effectMatrix[15] = 1.0f;
}

inline void getIdentityMatrix(VGfloat *effectMatrix)
{
    effectMatrix[0] = 1.0f;
    effectMatrix[1] = 0.0f;
    effectMatrix[2] = 0.0f;
    effectMatrix[3] = 0.0f;
    effectMatrix[4] = 0.0f;
    effectMatrix[5] = 1.0f;
    effectMatrix[6] = 0.0f;
    effectMatrix[7] = 0.0f;
    effectMatrix[8] = 0.0f;
    effectMatrix[9] = 0.0f;
    effectMatrix[10] = 1.0f;
    effectMatrix[11] = 0.0f;
    effectMatrix[12] = 0.0f;
    effectMatrix[13] = 0.0f;
    effectMatrix[14] = 0.0f;
    effectMatrix[15] = 1.0f;
}

#endif // HB_EFFECTS_OPENVG

void HbVgHslEffect::performEffect(QPainter *painter,
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

    Q_D(HbVgHslEffect);
    VGImage srcImage = vgImage.value<VGImage>();
    VGImage dstImage = d->ensurePixmap(&d->dstPixmap, vgImageSize);
    qreal opacity = clamp(d->opacity, 0.0f, 1.0f);
    if (opacity > HBVG_EPSILON) {
        if (d->paramsChanged) {
            // a helpful constant
            const qreal radsPerDeg = 2.0f * (qreal) M_PI / 360.0f;

            // make sure parametres are in range
            const VGfloat o = (VGfloat) opacity;
            const VGfloat angle = (VGfloat) clamp(d->hue * radsPerDeg, 0.0f, 2.0f * (qreal) M_PI); // angle [0, 2*pi]
            const VGfloat saturation = (VGfloat) clamp(d->saturation, 0.0f, 100.0f); // saturation [0, N]
            const VGfloat lightness = (VGfloat) clamp(d->lightness, -1.0f, 1.0f); // lightness [-1, 1]
    
            // check parametres which precalculated matrix we have to use.
            // Note: lightness affects offset and not matrix so we don't bother optimising that.
            const bool enableSaturation  = (saturation < 1.0f - HBVG_EPSILON || saturation > 1.0f + HBVG_EPSILON);
            const bool enableHueRotation = (HBVG_EPSILON < angle && angle < (2.0f * (qreal) M_PI - HBVG_EPSILON));

            if (enableSaturation && enableHueRotation) {
                // contains SaturateT*PrerotationT*HuerotationT*PostrotationT*I*opacity+I*(1-opacity) matrices
                // --- ugly, but saves lot of operations in FPU.
                // note: there are plenty of redundancy in these calculations
                // --- let compiler optimise them.
                getSaturationRotationMatrix(&d->colorMatrix[0], o, saturation, angle);
            } else if (enableSaturation && !enableHueRotation) {
                // saturationT*I*opacity+I*(1 - opacity) matrix without hue rotation
                getSaturationMatrix(&d->colorMatrix[0], o, saturation);
            } else if(!enableSaturation && enableHueRotation) {
                // PrerotationT*HuerotationT*PostrotationT*I*opacity+I*(1-opacity) matrices without saturation matrix
                getRotationMatrix(&d->colorMatrix[0], o, angle);
            } else {
                // identity matrix
                getIdentityMatrix(&d->colorMatrix[0]);
            }

            // colour component offsets
            d->colorMatrix[16] = lightness * o;
            d->colorMatrix[17] = lightness * o;
            d->colorMatrix[18] = lightness * o;
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
