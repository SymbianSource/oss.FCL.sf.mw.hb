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

#include "hbvgreflectioneffect_p.h"
#include "hbvgreflectioneffect_p_p.h"
#include "hbvgcolorizeeffect_p_p.h" // for getColorMatrix
#include "hbvgblureffect_p_p.h" // for identityLUT
#include <QPainter>
#include <QGraphicsItem>
#include <QDebug>

/*!
 * \class HbVgReflectionEffect
 *
 * \brief OpenVG-based reflection effect.
 * 
 * \internal
 */

HbVgReflectionEffectPrivate::HbVgReflectionEffectPrivate()
    : fade(0), fadeInited(false)
{    
}

HbVgReflectionEffectPrivate::~HbVgReflectionEffectPrivate()
{
#ifdef HB_EFFECTS_OPENVG
    if (fadeInited) {
        vgDestroyPaint(fadePaint);
    }
#endif
}

HbVgReflectionEffect::HbVgReflectionEffect(QObject *parent)
    : HbVgFrameEffect(*new HbVgReflectionEffectPrivate, parent)
{
}

HbVgReflectionEffect::HbVgReflectionEffect(HbVgReflectionEffectPrivate &dd, QObject *parent)
    : HbVgFrameEffect(dd, parent)
{
}

HbVgReflectionEffect::~HbVgReflectionEffect()
{
}

QPointF HbVgReflectionEffect::offset() const
{
    Q_D(const HbVgReflectionEffect);
    return d->offset;
}

void HbVgReflectionEffect::setOffset(const QPointF &offset)
{
    Q_D(HbVgReflectionEffect);
    if (offset == d->offset)
        return;
    d->offset = offset;
    updateEffectBoundingRect();
    emit offsetChanged(offset);
}

qreal HbVgReflectionEffect::fade() const
{
    Q_D(const HbVgReflectionEffect);
    return d->fade;
}

/*!
 * The fade parameter tells where the opacity in the gradual fade-out effect reaches zero,
 * i.e. total transparency, when moving upwards from the bottom of the reflection. The
 * default 0 is a special value, it disables the fade-out effect on the reflection
 * completely. Examples: 0.5 causes the lower half of the reflection to be completely
 * invisible (and the upper half linearly faded), 0.9 causes almost the entire reflection
 * to be invisible, 1 shows nothing from the reflection (it is completely faded out).
 */
void HbVgReflectionEffect::setFade(qreal fade)
{
    Q_D(HbVgReflectionEffect);
    if (fade == d->fade)
        return;
    d->fade = fade;
    updateEffect();
    emit fadeChanged(fade);
}

QColor HbVgReflectionEffect::color() const
{
    Q_D(const HbVgReflectionEffect);
    return d->color;
}

void HbVgReflectionEffect::setColor(const QColor &color)
{
    Q_D(HbVgReflectionEffect);
    if (color == d->color)
        return;
    d->color = color;
    updateEffect();
    emit colorChanged(color);
}

QRectF HbVgReflectionEffect::boundingRectFor(const QRectF &rect) const
{
    // Double the height of the rectangle but take also the offset into account.
    Q_D(const HbVgReflectionEffect);
    QRectF r(rect);
    QPointF mappedOffset = d->mapOffset(d->offset);
    qreal rotationAngle = d->mainWindowRotation();  
    
    if (rotationAngle == 0) 
        r.adjust(0, 0, 0, r.height());
    else if (rotationAngle == 90 || rotationAngle == -270)
        r.adjust(-r.width(), 0, 0, 0);
    else if (rotationAngle == -90 || rotationAngle == 270)
        r.adjust(0, 0, r.width(), 0);
      
    qreal x1 = qMin(r.left(), r.left() + mappedOffset.x());
    qreal y1 = qMin(r.top(), r.top() + mappedOffset.y());
    qreal x2 = qMax(r.right(), r.right() + mappedOffset.x());
    qreal y2 = qMax(r.bottom(), r.bottom() + mappedOffset.y());
        
    return QRectF(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
}

void HbVgReflectionEffect::performEffect(QPainter *painter,
                                         const QPointF &offset,
                                         const QVariant &vgImage,
                                         const QSize &vgImageSize)
{
#ifdef HB_EFFECTS_OPENVG
    Q_D(HbVgReflectionEffect);

    if (d->hints & ForceFrameHint) {
        painter->save();
        HbVgFrameEffect::performEffect(painter, offset, vgImage, vgImageSize);
        painter->restore();
    }

    QPaintDevice *pdev = painter->device();
    QRectF rectWithChildren = d->deviceRectForSource(
        HbVgFrameEffectPrivate::IncludeChildren,
        pdev);
    QRectF rectWithoutChildren = d->deviceRectForSource(
        HbVgFrameEffectPrivate::ExcludeChildren,
        pdev);
    VGImage srcImage = vgImage.value<VGImage>();
    VGImage dstImage = d->ensurePixmap(&d->dstPixmap, vgImageSize);

    // Draw the source pixmap using the painter, this will also set up the
    // IMAGE_USER_TO_SURFACE matrix.
    painter->drawPixmap(offset, d->srcPixmap);

    // Prepare the mirrored image.    
    qreal rotationAngle = d->mainWindowRotation();
    qreal absRotationAngle = qAbs(rotationAngle);
    
    VGfloat m[9];
    vgGetMatrix(m);
    vgLoadIdentity();
    if (absRotationAngle == 0)
        m[4] *= -1.0f;
    else if (absRotationAngle == 90 || absRotationAngle == 270)
        m[0] *= -1.0f;
    vgMultMatrix(m);
    
    // Must move the mirrored image to have it on top of the original and then down
    // again to have it below in portrait-mode. Rotation angles -90 or 270 causes image to be moved to right,
    // and in rotation angles -90 and 270, image is in correct place initially.
    // Try to take the exclude-children hint into account when performing the second move.
    
    VGfloat trans;
    if (absRotationAngle == 0) {        
        if (d->hints & ExcludeChildrenHint)
            trans = -rectWithChildren.height() - rectWithoutChildren.height();
        else
            trans = -2.0f * rectWithChildren.height();
        
        vgTranslate(0.0f, trans);
    }  
    else if (absRotationAngle == 90 || absRotationAngle == 270) {
        if (d->hints & ExcludeChildrenHint)
            trans = -rectWithChildren.width() - rectWithoutChildren.width();
        else
            trans = -2.0f * rectWithChildren.width();
        
        vgTranslate(trans, 0.0f);
    }
        
    // Apply the additional offset. Note: down = minus, right = plus.
    QPointF mappedOffset = d->mapOffset(d->offset);    
    VGfloat ox = (VGfloat) mappedOffset.x();
    VGfloat oy = (VGfloat) mappedOffset.y();

    if (rotationAngle == 0)   
        vgTranslate(ox, -oy); 
    else if (rotationAngle == 90 || rotationAngle == -270)
        vgTranslate(-ox, oy);   
    else if (rotationAngle == -90 || rotationAngle == 270)
        vgTranslate(-ox, oy); 
    
    // Apply the opacity and the color. When no color was set and the opacity is 1, the
    // source image will be used as it is. This is the only place where we can try to use
    // the pixmap cache.
    VGImage imgToDraw = srcImage;
    QPixmap cachedPm = cached(vgImageSize);
   if (cachedPm.isNull()) {
        VGImage tmpImage = VG_INVALID_HANDLE;
        if (d->color.isValid()) {
            // Perform a colorize effect (ignore the opacity here because it must be set for
            // the full image, not just the color overlay).
            tmpImage = d->ensurePixmap(&d->tmpPixmap, vgImageSize);
            VGfloat colorMatrix[20];
            HbVgColorizeEffectPrivate::getColorMatrix(colorMatrix, d->color, 1.0f);
            vgColorMatrix(tmpImage, srcImage, colorMatrix);
            imgToDraw = tmpImage;
        }
        qreal opacity = clamp(d->opacity, 0.0f, 1.0f);
        if (d->opacity < 1.0f - HBVG_EPSILON) {
            // Apply the opacity, i.e. modify the alpha channel.
            if (d->paramsChanged) {
                for (int i = 0; i < 256; ++i)
                    d->alphaLUT[i] = (VGubyte) (i * opacity);
            }
            vgLookup(dstImage, imgToDraw,
                     identityLUT, identityLUT, identityLUT, d->alphaLUT,
                     VG_TRUE, VG_FALSE);
            imgToDraw = dstImage;
        }
        // If colorize and/or opacity was used then try to cache the result.
        if (imgToDraw == tmpImage)
            tryCache(d->tmpPixmap);
        else if (imgToDraw == dstImage)
            tryCache(d->dstPixmap);
    } else {
        imgToDraw = qPixmapToVGImage(cachedPm);
    }

    // Fade out the lower part of the mirrored image. Skip this if 'fade' is 0, i.e. there
    // is no fade-out effect.
    VGfloat fade = (VGfloat) clamp(d->fade, 0.0f, 1.0f);
    if (fade > HBVG_EPSILON) {
        // Prepare the linear gradient used for fading if not yet done.
        if (!d->fadeInited) {
            d->fadePaint = vgCreatePaint();
            vgSetParameteri(d->fadePaint, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
            vgSetParameteri(d->fadePaint,
                            VG_PAINT_COLOR_RAMP_SPREAD_MODE, VG_COLOR_RAMP_SPREAD_REFLECT);
            d->fadeInited = true;
        }

        if (d->paramsChanged) {
            // Prepare the color ramp, take the value of 'fade' into account. (It tells
            // where the opacity reaches zero, i.e. total transparency.) The RGB values
            // are set to 1 because the mirrored image's RGB values must not be changed,
            // only the alpha channel needs a little manipulation (to get the gradual
            // fade-out effect).
            VGfloat stops[] = {
                0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                1.0f - fade, 1.0f, 1.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 1.0f, 1.0f, 0.0f
            };
            vgSetParameterfv(d->fadePaint, VG_PAINT_COLOR_RAMP_STOPS, 15, stops);
        }

        // Set up the linear gradient based on the (transformed) size of the source.
        VGfloat sw = (VGfloat) rectWithChildren.width();
        VGfloat sh = (VGfloat) rectWithChildren.height();
         // must be bottom-up to get the proper effect
        if (rotationAngle == 0)  {
            VGfloat grad[] = { sw / 2.0f, sh,
                               sw / 2.0f, 0.0f };            
            vgSetParameterfv(d->fadePaint, VG_PAINT_LINEAR_GRADIENT, 4, grad);
        }
        else if (rotationAngle == -90 || rotationAngle == 270){
            VGfloat grad[] = { sw, sh / 2.0f,
                               0.0f, sh / 2.0f };
            vgSetParameterfv(d->fadePaint, VG_PAINT_LINEAR_GRADIENT, 4, grad);
        }
        else if (rotationAngle == 90 || rotationAngle == -270){
            VGfloat grad[] = { 0.0f, sh / 2.0f,
                               sw, sh / 2.0f };
            vgSetParameterfv(d->fadePaint, VG_PAINT_LINEAR_GRADIENT, 4, grad);
        }        

        // Draw the mirrored image by using the paint to get a gradual fade-out effect.
        vgSeti(VG_MATRIX_MODE, VG_MATRIX_FILL_PAINT_TO_USER);
        vgLoadIdentity();
        vgSetPaint(d->fadePaint, VG_FILL_PATH);
        vgSeti(VG_IMAGE_MODE, VG_DRAW_IMAGE_MULTIPLY);
        vgDrawImage(imgToDraw);
        vgSeti(VG_IMAGE_MODE, VG_DRAW_IMAGE_NORMAL);
        vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
        vgSetPaint(VG_INVALID_HANDLE, VG_FILL_PATH);
    } else {
        // Just draw the mirrored image normally.
        vgDrawImage(imgToDraw);
    }

#else
    Q_UNUSED(painter);
    Q_UNUSED(offset);
    Q_UNUSED(vgImage);
    Q_UNUSED(vgImageSize);
#endif
}
