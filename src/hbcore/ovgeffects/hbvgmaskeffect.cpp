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

#include "hbvgmaskeffect_p.h"
#include "hbvgmaskeffect_p_p.h"
#include <QPainter>
#include <QPaintEngine>
#include <QPaintDevice>

/*!
 * \class HbVgMaskEffect
 *
 * \brief OpenVG-based mask effect.
 *
 * The mask effect can be used to make a rectangular area or an area
 * defined by a pixmap's alpha channel transparent. The opacity effect
 * parameter is ignored.
 *
 * \internal
 */

HbVgMaskEffectPrivate::HbVgMaskEffectPrivate()
    : maskRectIsInDeviceCoords(false), maskCallback(0)
{
}

HbVgMaskEffect::HbVgMaskEffect(QObject *parent)
    : HbVgEffect(*new HbVgMaskEffectPrivate, parent)
{
}

HbVgMaskEffect::HbVgMaskEffect(HbVgMaskEffectPrivate &dd, QObject *parent)
    : HbVgEffect(dd, parent)
{
}

HbVgMaskEffect::~HbVgMaskEffect()
{
}

/*!
 * Turns off masking. Any previously set mask pixmap or rectangle will not be
 * effective anymore.
 */
void HbVgMaskEffect::clear()
{
    Q_D(HbVgMaskEffect);
    d->mask = d->scaledMask = d->callbackResult = QPixmap();
    d->maskRect = QRectF();
    d->maskRectIsInDeviceCoords = false;
    d->maskCallback = 0;
    d->maskCallbackParam = 0;
}

/*!
 * Returns the mask pixmap set by the previous setMask() call.
 */
QPixmap HbVgMaskEffect::mask() const
{
    Q_D(const HbVgMaskEffect);
    return d->mask;
}

/*!
 * Returns the scaled version of the mask that was used during the previous
 * paint. Will return a null pixmap if no painting took place since the last
 * setMask() call.
 */
QPixmap HbVgMaskEffect::scaledMask() const
{
    Q_D(const HbVgMaskEffect);
    return d->scaledMask;
}

/*!
 * Sets the mask pixmap. Only the alpha channel is relevant. Pixels where alpha
 * is 0 will be set transparent. The pixmap is subject to scaling and therefore
 * distortion may occur. If this is not acceptable then use the callback
 * version. Any previously set mask pixmap or rectangle will not be effective
 * anymore.
 */
void HbVgMaskEffect::setMask(const QPixmap &mask)
{
    Q_D(HbVgMaskEffect);
    clear();
    d->mask = mask;
    updateEffect();
    emit maskChanged(mask);
}

/*!
 * Returns the currently set mask callback pointer.
 */
HbVgMaskEffect::MaskCallback HbVgMaskEffect::maskCallback() const
{
    Q_D(const HbVgMaskEffect);
    return d->maskCallback;
}

/*!
 * Returns the currently set custom parameter that will be passed to the
 * callback.
 */
void *HbVgMaskEffect::maskCallbackParam() const
{
    Q_D(const HbVgMaskEffect);
    return d->maskCallbackParam;
}

/*!
 * If the user of the API is able to generate a mask pixmap for a given size,
 * use this function instead of setMask(). The callback will be invoked during
 * painting to get the mask pixmap. It should return a pixmap that has a size
 * that is same as the size that has been passed to the callback. The effect
 * tries to reduce the number of calls by calling only when a new callback is
 * set or when the size of the source item is different than before. Any
 * previously set mask pixmap or rectangle will not be effective anymore.
 */
void HbVgMaskEffect::setMaskCallback(MaskCallback callback, void *param)
{
    Q_D(HbVgMaskEffect);
    if (d->maskCallback == callback)
        return;
    clear();
    d->maskCallback = callback;
    d->maskCallbackParam = param;
    updateEffect();
    emit maskCallbackChanged(callback);
}

/*!
 * Returns the mask rectangle set by the previous setMaskRect() call.
 */
QRectF HbVgMaskEffect::maskRect() const
{
    Q_D(const HbVgMaskEffect);
    return d->maskRectIsInDeviceCoords ? QRectF() : d->maskRect;
}

/*!
 * Returns the mask rectangle set by the previous setMaskDeviceRect() call.
 */
QRectF HbVgMaskEffect::maskDeviceRect() const
{
    Q_D(const HbVgMaskEffect);
    return d->maskRectIsInDeviceCoords ? d->maskRect : QRectF();
}

/*!
 * Sets the mask rectangle. The area defined by the rectangle will be made
 * transparent inside the source item. Any previously set mask pixmap or
 * rectangle will not be effective anymore.
 */
void HbVgMaskEffect::setMaskRect(const QRectF &rect)
{
    Q_D(HbVgMaskEffect);
    if (rect == d->maskRect && !d->maskRectIsInDeviceCoords)
        return;
    clear();
    d->maskRect = rect;
    d->maskRectIsInDeviceCoords = false;
    updateEffect();
    emit maskRectChanged(rect);
}

/*!
 * Similar to setMask() but the rectangle is assumed to be in device coordinates
 * (i.e. relative to the entire screen instead of the source item), meaning that
 * the source item will be clipped where it intersects with \a rect.
 */
void HbVgMaskEffect::setMaskDeviceRect(const QRectF &rect)
{
    Q_D(HbVgMaskEffect);
    if (rect == d->maskRect && d->maskRectIsInDeviceCoords)
        return;
    clear();
    d->maskRect = rect;
    d->maskRectIsInDeviceCoords = true;
    updateEffect();
    emit maskRectChanged(rect);
}

/*!
 * \reimp
 */
QRectF HbVgMaskEffect::boundingRectFor(const QRectF &rect) const
{
    return rect;
}

/*!
 * Returns the OpenVG Y coordinate for a rectangle with the given Qt Y
 * coordinate and height.
 *
 * \internal
 */
inline int toVgYH(int y, int h, QPaintDevice *pdev)
{
    return pdev->height() - 1 - y - h;
}

/*!
 * \reimp
 */
void HbVgMaskEffect::performEffect(QPainter *painter,
                                   const QPointF &offset,
                                   const QVariant &vgImage,
                                   const QSize &vgImageSize)
{
#ifdef HB_EFFECTS_OPENVG
    Q_UNUSED(vgImage);
    Q_D(HbVgMaskEffect);

    // Initialize scaledMask if the mask has changed or the size of the source
    // is different than before.
    if (!d->mask.isNull()) {
        if (d->scaledMask.isNull())
            d->scaledMask = d->mask;
        // Scale only when really needed, i.e. when the size is different than
        // before (or there is a new mask).
        if (d->scaledMask.size() != vgImageSize)
            d->scaledMask = d->mask.scaled(vgImageSize);
    }

    vgSeti(VG_MASKING, VG_TRUE);
    QPaintDevice *pdev = painter->paintEngine()->paintDevice();
    // Set the mask for the entire surface to 1 (i.e. nothing is transparent).
    vgMask(VG_INVALID_HANDLE, VG_FILL_MASK,
           0, 0, pdev->width(), pdev->height());

    // If there is a pixmap then use that, otherwise use the rectangle. If none
    // of these is set then try the callback, if that is not set either then
    // just draw the source normally.
    QPixmap *maskPtr = 0;
    int ox = (int) offset.x();
    int oy = (int) offset.y();
    if (d->scaledMask.isNull() && !d->maskRect.isNull()) {
        int x1 = (int) d->maskRect.x();
        int y1 = (int) d->maskRect.y();
        int w = (int) d->maskRect.width();
        int h = (int) d->maskRect.height();
        if (!d->maskRectIsInDeviceCoords) {
            x1 += ox;
            y1 += oy;
        }
        // Make the area defined by the rectangle transparent. Passing
        // VG_CLEAR_MASK results in writing 0 to the mask which results in
        // transparent pixels at that position.
        vgMask(VG_INVALID_HANDLE, VG_CLEAR_MASK,
               x1, toVgYH(y1, h, pdev),
               w, h);
    } else if (!d->scaledMask.isNull()) {
        maskPtr = &d->scaledMask;
    } else if (d->maskCallback) {
        // Invoke the callback but only if it has just been set or the size of
        // the source is different than before.
        if (d->callbackResult.isNull() || d->callbackResult.size() != vgImageSize)
            d->callbackResult = d->maskCallback(vgImageSize, d->maskCallbackParam);
        maskPtr = &d->callbackResult;
    }

    if (maskPtr) {
        int w = vgImageSize.width();
        int h = vgImageSize.height();
        // Will use the alpha channel from the image, alpha=0 => 0 in the mask
        // => transparent pixel, alpha=255 => 1 in the mask => opaque pixel.
        vgMask(qPixmapToVGImage(*maskPtr), VG_SET_MASK,
               ox, toVgYH(oy, h, pdev),
               w, h);
    }

    // Draw the source item with masking enabled.
    painter->drawPixmap(offset, d->srcPixmap);
    vgSeti(VG_MASKING, VG_FALSE);

#else
    Q_UNUSED(painter);
    Q_UNUSED(offset);
    Q_UNUSED(vgImage);
    Q_UNUSED(vgImageSize);
#endif
}
