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
 * With certain OpenVG implementations masking with vgMask() may be
 * problematic.  In such cases enable the pure QPainter implementation
 * with setForceSwMode().
 *
 * Currently the sw version only support masks set by setMask() or
 * provided via the callback.  Mask rectangles are not supported.
 *
 * When using this effect in a chain, sw mode must always be forced
 * via setForceSwMode().
 *
 * \internal
 */

HbVgMaskEffectPrivate::HbVgMaskEffectPrivate()
    : maskRectIsInDeviceCoords(false),
      maskCallback(0),
      maskCallbackParam(0),
      lastMainWindowRotationAngle(-1),
      includeSourceItemOnly(false)
{
}

HbVgMaskEffect::HbVgMaskEffect(QObject *parent)
    : HbVgFrameEffect(*new HbVgMaskEffectPrivate, parent)
{
}

HbVgMaskEffect::HbVgMaskEffect(HbVgMaskEffectPrivate &dd, QObject *parent)
    : HbVgFrameEffect(dd, parent)
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
 * setMask() call. Any other automatic transformation (e.g. rotation) of the
 * mask is not included in the returned pixmap.
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
 * anymore. If the graphics view is rotated (due to a transformed screen
 * orientation) then the mask will be rotated automatically too.
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
    if (d->maskCallback == callback) {
        return;
    }
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
    if (rect == d->maskRect && !d->maskRectIsInDeviceCoords) {
        return;
    }
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
 *
 * Even when the graphics view is transformed (e.g. when being in "landscape
 * orientation") the rectangle passed here is treated as being in device
 * coordinates, ignoring the rotation of the graphics view.
 */
void HbVgMaskEffect::setMaskDeviceRect(const QRectF &rect)
{
    Q_D(HbVgMaskEffect);
    if (rect == d->maskRect && d->maskRectIsInDeviceCoords) {
        return;
    }
    clear();
    d->maskRect = rect;
    d->maskRectIsInDeviceCoords = true;
    updateEffect();
    emit maskRectChanged(rect);
}

/*!
 * Returns the current setting for masking only the source graphics item. By
 * default this is disabled.
 */
bool HbVgMaskEffect::includeSourceItemOnly() const
{
    Q_D(const HbVgMaskEffect);
    return d->includeSourceItemOnly;
}

/*!
 * When enabled, only the source item (excluding its children) is masked.  This
 * is needed when shaping e.g. a pop-up or other widget that has a scroll area
 * in it. The real size, together with all the children, is usually much bigger
 * then the size of the widget itself (and it depends on the position in the
 * scroll area etc.).  To solve this issue, enable this setting when shaping
 * such widgets.
 */
void HbVgMaskEffect::setIncludeSourceItemOnly(bool b)
{
    Q_D(HbVgMaskEffect);
    clear();
    d->includeSourceItemOnly = b;
    updateEffect();
    emit includeSourceItemOnlyChanged(b);
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
 * \internal
 */
QRectF HbVgMaskEffectPrivate::mapRect(const QRectF &rect, const QSize &srcSize) const
{
    qreal rotationAngle = mainWindowRotation();
    qreal x1 = 0;
    qreal y1 = 0;

    QPointF mp = mapOffset(QPointF(rect.x(), rect.y()));
    QSizeF sz = mapSize(rect.size());

    if (rotationAngle == -90 || rotationAngle == 270) {
        x1 = mp.x();
        y1 = mp.y() + srcSize.height() - sz.height();
    } else if (rotationAngle == 90 || rotationAngle == -270) {
        x1 = mp.x() + srcSize.width() - sz.width();
        y1 = mp.y();
    } else {
        x1 = mp.x();
        y1 = mp.y();
    }

    return QRectF(x1, y1, sz.width(), sz.height());
}

/*!
 * \reimp
 *
 * OpenVG-based implementation. Supports all masking options.
 */
void HbVgMaskEffect::performEffect(QPainter *painter,
                                   const QPointF &offset,
                                   const QVariant &vgImage,
                                   const QSize &vgImageSize)
{
#ifdef HB_EFFECTS_OPENVG
    Q_UNUSED(vgImage);
    Q_D(HbVgMaskEffect);

    if (!painter->paintEngine()) {
        return;
    }

    // Find out the target size and position.
    QPaintDevice *pdev = painter->paintEngine()->paintDevice();
    QRectF srcDevRect(d->deviceRectForSource(HbVgFrameEffectPrivate::ExcludeChildren, pdev));
    QSize targetSize = d->includeSourceItemOnly ? srcDevRect.size().toSize() : vgImageSize;
    int posX = (int) offset.x();
    int posY = (int) offset.y();
    if (d->includeSourceItemOnly) {
        posX = (int) srcDevRect.x();
        posY = (int) srcDevRect.y();
    }

    // Initialize scaledMask if the mask has changed or the size of the source
    // is different than before.
    if (!d->mask.isNull()) {
        if (d->scaledMask.isNull()) {
            d->scaledMask = d->mask;
        }
        // Scale only when really needed, i.e. when the size is different than
        // before (or there is a new mask).
        if (d->scaledMask.size() != targetSize) {
            d->scaledMask = d->mask.scaled(targetSize);
            d->rotatedPixmap = QPixmap();
        }
    }

    vgSeti(VG_MASKING, VG_TRUE);
    // Set the mask for the entire surface to 1 (i.e. nothing is transparent).
    vgMask(VG_INVALID_HANDLE, VG_FILL_MASK,
           0, 0, pdev->width(), pdev->height());

    // If there is a pixmap then use that, otherwise use the rectangle. If none
    // of these is set then try the callback, if that is not set either then
    // just draw the source normally.
    QPixmap *maskPtr = 0;
    if (d->scaledMask.isNull() && !d->maskRect.isNull()) {
        QRectF mappedRect = d->maskRect;
        if (!d->maskRectIsInDeviceCoords) {
            mappedRect = d->mapRect(d->maskRect, targetSize);
            mappedRect.adjust(posX, posY, posX, posY);
        }
        int x1 = (int) mappedRect.x();
        int y1 = (int) mappedRect.y();
        int w = (int) mappedRect.width();
        int h = (int) mappedRect.height();
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
        if (d->callbackResult.isNull() || d->callbackResult.size() != targetSize) {
            d->callbackResult = d->maskCallback(targetSize, d->maskCallbackParam);
            d->rotatedPixmap = QPixmap();
        }
        maskPtr = &d->callbackResult;
    }

    if (maskPtr) {
        int w = targetSize.width();
        int h = targetSize.height();
        QPixmap pm;
        qreal rotationAngle = d->mainWindowRotation();
        if (rotationAngle != 0) {
            if (d->rotatedPixmap.isNull() || rotationAngle != d->lastMainWindowRotationAngle) {
                d->rotatedPixmap = maskPtr->transformed(d->rotationTransform()).scaled(targetSize);
                d->lastMainWindowRotationAngle = rotationAngle;
            }
            pm = d->rotatedPixmap;
        }
        QPixmap *finalMaskPtr = pm.isNull() ? maskPtr : &pm;
        // Will use the alpha channel from the image, alpha=0 => 0 in the mask
        // => transparent pixel, alpha=255 => 1 in the mask => opaque pixel.
        vgMask(qPixmapToVGImage(*finalMaskPtr), VG_SET_MASK,
               posX, toVgYH(posY, h, pdev),
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

/*!
 * \reimp
 *
 * QPainter-based implementation. Currently only supports masking via pixmaps.
 *
 * The behavior for partially visible items (clipped by the device rect) is
 * somewhat wrong, the mask is never clipped, it is just scaled down to match
 * the visible part of the item.
 */
void HbVgMaskEffect::performEffectSw(QPainter *devicePainter, QPixmap *result, QPointF *resultPos)
{
    Q_D(HbVgMaskEffect);

    if (!devicePainter->paintEngine()) {
        drawSource(devicePainter);
        return;
    }

    QPoint offset;
    QPixmap srcPixmap = sourcePixmapForRoot(Qt::DeviceCoordinates, &offset); // needs the original world transform
    if (srcPixmap.isNull()) {
        return;
    }

    QPaintDevice *pdev = devicePainter->paintEngine()->paintDevice();
    d->worldTransform = devicePainter->worldTransform(); // deviceRectForSource needs this
    // The full source rect (without child items) would be
    // d->worldTransform.mapRect(sourceItemForRoot()->boundingRect()).toRect()
    // but we only care about the visible part here so clipping must be applied.
    QRect srcDevRect(d->deviceRectForSource(HbVgFrameEffectPrivate::ExcludeChildren, pdev).toRect());
    QPoint pos = d->includeSourceItemOnly ? srcDevRect.topLeft() : offset;
    QSize size = d->includeSourceItemOnly ? srcDevRect.size() : srcPixmap.size();
    if (size.width() <= 0 || size.height() <= 0) {
        return;
    }

    if (resultPos) {
        *resultPos = pos;
    }

    QPixmap cachedPm = cached(size);
    if (!cachedPm.isNull()) {
        if (result) {
            *result = cachedPm;
        } else {
            devicePainter->setWorldTransform(QTransform());
            devicePainter->drawPixmap(pos, cachedPm);
            devicePainter->setWorldTransform(d->worldTransform);
        }
        return;
    }

    QPixmap maskPixmap;
    if (d->maskCallback) {
        if (d->callbackResult.isNull() || d->callbackResult.size() != size) {
            d->callbackResult = d->maskCallback(size, d->maskCallbackParam);
            d->rotatedPixmap = QPixmap();
        }
        maskPixmap = d->callbackResult;
    } else if (!d->mask.isNull()) {
        if (d->scaledMask.isNull()) {
            d->scaledMask = d->mask;
        }
        if (d->scaledMask.size() != size) {
            d->scaledMask = d->mask.scaled(size);
            d->rotatedPixmap = QPixmap();
        }
        maskPixmap = d->scaledMask;
    } else {
        // Masking via rectangles is not supported here.
        drawSource(devicePainter);
        return;
    }

    qreal rotationAngle = d->mainWindowRotation();
    if (rotationAngle != 0) {
        if (d->rotatedPixmap.isNull() || rotationAngle != d->lastMainWindowRotationAngle) {
            d->rotatedPixmap = maskPixmap.transformed(d->rotationTransform()).scaled(size);
            d->lastMainWindowRotationAngle = rotationAngle;
        }
        maskPixmap = d->rotatedPixmap;
    }

    if (d->includeSourceItemOnly) {
        // Take only the source item itself, excluding its children.
        srcPixmap = srcPixmap.copy(srcDevRect.adjusted(-offset.x(), -offset.y(), -offset.x(), -offset.y()));
    }

    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    QPainter p(&image);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(image.rect(), Qt::transparent);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.drawPixmap(0, 0, srcPixmap);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawPixmap(0, 0, maskPixmap);
    p.end();

    if (result) {
        *result = QPixmap(size);
        result->fill(Qt::transparent);
        QPainter p(result);
        p.drawImage(QPointF(0, 0), image);
        p.end();
        tryCache(*result);
    } else {
        devicePainter->setWorldTransform(QTransform());
        devicePainter->drawImage(pos, image);
        devicePainter->setWorldTransform(d->worldTransform);
        tryCache(QPixmap::fromImage(image));
    }
}

HbVgEffect::ChainBehavior HbVgMaskEffect::chainBehavior() const
{
    Q_D(const HbVgMaskEffect);
    // This is actually incorrect if sw mode is not forced and we are
    // rendering on a raster painter, but mask effects without sw mode
    // forcing should not be used in a chain at all.
    return d->forceSwMode ? ChainBehavAsSource : ChainBehavNormal;
}
