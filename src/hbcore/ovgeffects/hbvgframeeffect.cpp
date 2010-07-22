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

#include "hbvgframeeffect_p.h"
#include "hbvgframeeffect_p_p.h"
#include <QPainter>
#include <QGraphicsItem>

/*!
 * \class HbVgFrameEffect
 *
 * \brief This effect makes the bounding rectangle of the source item visible by
 * drawing a frame around it. It also serves as a base class for similar
 * effects.
 *
 * \internal
 */

/*!
 * \enum HbVgFrameEffect::Hint
 * The hints that can be set via setHint().
 */

/*!
 * \var HbVgFrameEffect::Hint HbVgFrameEffect::ExcludeChildrenHint
 *
 * When this hint is set, certain calculations will ignore the source item's
 * child items and only take the source item's bounding rectangle into
 * account. This does not affect the content, i.e. the source will always be
 * drawn together with all its children, however the position and size of the
 * frame, the reflection, etc. can be different, based on this hint.
 */

/*!
 * \var HbVgFrameEffect::Hint HbVgFrameEffect::DrawSourceHint
 *
 * Used by HbVgFrameEffect only. Forces the source item to be drawn. By default
 * only the frame is shown, set this hint to draw also the content.
 */

/*!
 * \var HbVgFrameEffect::Hint HbVgFrameEffect::ForceFrameHint
 *
 * Used by derived classes only (not by HbVgFrameEffect itself). Forces the
 * frame to be drawn around the source item, just like a plain HbVgFrameEffect
 * would do.
 */

HbVgFrameEffectPrivate::HbVgFrameEffectPrivate()
    : color(Qt::black), hints(0)
{
}

/*!
 * Returns the device rect, i.e. maps the bounding rectangle to device
 * coordinates and performs clipping to the device viewport.
 *
 * \internal
 */
QRectF HbVgFrameEffectPrivate::deviceRectForSource(DeviceRectType type,
        QPaintDevice *pdev)
{
    Q_Q(HbVgFrameEffect);
    QRectF br = type == IncludeChildren
                ? q->sourceBoundingRectForRoot()
                : q->sourceItemForRoot()->boundingRect();
    QRect result = worldTransform.mapRect(br).toAlignedRect();
    if (pdev) {
        int left, top, right, bottom;
        result.getCoords(&left, &top, &right, &bottom);
        if (left < 0) {
            result.setX(0);
        }
        if (top < 0) {
            result.setY(0);
        }
        int deviceWidth = pdev->width();
        int deviceHeight = pdev->height();
        if (right > deviceWidth - 1) {
            result.setRight(deviceWidth - 1);
        }
        if (bottom > deviceHeight - 1) {
            result.setBottom(deviceHeight - 1);
        }
    }
    return result;
}

HbVgFrameEffect::HbVgFrameEffect(QObject *parent)
    : HbVgEffect(*new HbVgFrameEffectPrivate, parent)
{
}

HbVgFrameEffect::HbVgFrameEffect(HbVgFrameEffectPrivate &dd, QObject *parent)
    : HbVgEffect(dd, parent)
{
}

HbVgFrameEffect::~HbVgFrameEffect()
{
}

QColor HbVgFrameEffect::color() const
{
    Q_D(const HbVgFrameEffect);
    return d->color;
}

void HbVgFrameEffect::setColor(const QColor &color)
{
    Q_D(HbVgFrameEffect);
    if (d->color == color) {
        return;
    }
    d->color = color;
    updateEffect();
    emit colorChanged(color);
}

/*!
 * Enables the given \a hint. Hints are not guaranteed to be taken into account
 * in any way.
 */
void HbVgFrameEffect::setHint(Hints hint)
{
    Q_D(HbVgFrameEffect);
    d->hints |= hint;
}

QRectF HbVgFrameEffect::boundingRectFor(const QRectF &rect) const
{
    return rect;
}

void HbVgFrameEffect::performEffect(QPainter *painter,
                                    const QPointF &offset,
                                    const QVariant &vgImage,
                                    const QSize &vgImageSize)
{
    Q_UNUSED(offset);
    Q_UNUSED(vgImage);
    Q_UNUSED(vgImageSize);

    Q_D(HbVgFrameEffect);
    if (d->hints & DrawSourceHint) {
        painter->drawPixmap(offset, d->srcPixmap);
    }
    painter->setOpacity(d->opacity);
    painter->setPen(d->color);
    painter->drawRect(d->deviceRectForSource(
                          (d->hints & ExcludeChildrenHint)
                          ? HbVgFrameEffectPrivate::ExcludeChildren
                          : HbVgFrameEffectPrivate::IncludeChildren,
                          painter->device()));
}
