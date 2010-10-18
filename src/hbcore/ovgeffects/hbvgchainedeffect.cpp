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

#include "hbvgchainedeffect_p.h"
#include "hbvgchainedeffect_p_p.h"
#include "hbinstance_p.h"
#include <QPainter>

/*!
 * \class HbVgChainedEffect
 *
 * \brief A container effect that delegates the drawing to a number of
 * other effects.
 *
 * \internal
 *
 * A chained effect contains one or more other effects. When drawing,
 * all of these effects will be executed one by one, using the
 * original source pixmap as the input, in the order in which they
 * were added to the chained effect.
 *
 * Due to the fact that all the effects operate on the original source
 * many effect combinations do not make sense as the result of the
 * earlier ones may not be visible at all. However a chained effect
 * can be very useful when combining an outline or drop shadow with
 * something else.
 *
 * For the sake of simplicity the opacity and caching properties of
 * ChainedEffect instances is ignored, use the property of the
 * individual effects instead.
 *
 * Software-based mask effects in the chain are handled specially. If
 * such an effect is present at any position, it will be applied first
 * and the other effects will get its output as their source pixmap.
 * (this works only in hw mode)
 */

/*!
 * Constructs a new chained effect instance.
 */
HbVgChainedEffect::HbVgChainedEffect(QObject *parent)
    : HbVgEffect(*new HbVgChainedEffectPrivate, parent)
{
}

HbVgChainedEffect::HbVgChainedEffect(HbVgChainedEffectPrivate &dd, QObject *parent)
    : HbVgEffect(dd, parent)
{
}

HbVgChainedEffect::~HbVgChainedEffect()
{
    Q_D(HbVgChainedEffect);
    qDeleteAll(d->effects);
}

/*!
 * Adds a new effect to the end of the chain.
 *
 * If \a effect has any parent set then it is removed because the
 * ChainedEffect instance will take ownership.
 */
void HbVgChainedEffect::add(HbVgEffect *effect)
{
    Q_D(HbVgChainedEffect);
    d->effects.append(effect);
    effect->setParent(0);
    effect->setChainRoot(this);
    updateBoundingRect();
    emit effectAdded(effect);
}

/*!
 * Removes the given \a effect from the chain. As the chain owns the
 * effect, it is deleted upon a successful removal.
 */
void HbVgChainedEffect::remove(HbVgEffect *effect)
{
    Q_D(HbVgChainedEffect);
    int idx = d->effects.indexOf(effect);
    if (idx != -1) {
        d->effects.remove(idx);
        updateBoundingRect();
        emit effectRemoved(effect);
        delete effect;
    }
}

/*!
 * Checks if the given \a effect is present in the chain.
 */
bool HbVgChainedEffect::contains(HbVgEffect *effect) const
{
    Q_D(const HbVgChainedEffect);
    return d->effects.contains(effect);
}

/*!
 * Returns the number of effects in the chain.
 */
int HbVgChainedEffect::count() const
{
    Q_D(const HbVgChainedEffect);
    return d->effects.count();
}

/*!
 * Returns the effect at the given \a index.
 */
const HbVgEffect *HbVgChainedEffect::at(int index) const
{
    Q_D(const HbVgChainedEffect);
    return d->effects.at(index);
}

/*!
 * Returns the effect at the given \a index.
 */
HbVgEffect *HbVgChainedEffect::at(int index)
{
    Q_D(HbVgChainedEffect);
    return d->effects.at(index);
}

/*!
 * Returns all effects in a list and removes them from the chain
 * without destroying them.
 */
QList<HbVgEffect *> HbVgChainedEffect::takeAll()
{
    Q_D(HbVgChainedEffect);
    QList<HbVgEffect *> result = d->effects.toList();
    d->effects.clear();
    return result;
}

/*!
 * \reimp
 */
QRectF HbVgChainedEffect::boundingRectFor(const QRectF &rect) const
{
    Q_D(const HbVgChainedEffect);
    if (d->effects.empty()) {
        return rect;
    }
    // find out the bounding rect that covers all the effects
    QRectF result;
    foreach(HbVgEffect * effect, d->effects) {
        QRectF br = effect->boundingRectFor(rect);
        if (result.isNull()) {
            result = br;
        } else {
            if (br.left() < result.left()) {
                result.setLeft(br.left());
            }
            if (br.top() < result.top()) {
                result.setTop(br.top());
            }
            if (br.right() > result.right()) {
                result.setRight(br.right());
            }
            if (br.bottom() > result.bottom()) {
                result.setBottom(br.bottom());
            }
        }
    }
    return result;
}

/*!
 * \reimp
 */
void HbVgChainedEffect::performEffect(QPainter *painter,
                                      const QPointF &offset,
                                      const QVariant &vgImage,
                                      const QSize &vgImageSize)
{
#ifdef HB_EFFECTS_OPENVG
    Q_D(HbVgChainedEffect);
    // If some effect wants to be the source then have it rendered
    // into a pixmap first (works only for sw-based effects).
    QPixmap src = d->srcPixmap;
    QVariant srcVgImage = vgImage;
    foreach(HbVgEffect * effect, d->effects) {
        if (effect->chainBehavior() == ChainBehavAsSource) {
            // Restore the world transform temporarily because the sw
            // version has slightly different semantics.
            painter->setWorldTransform(d->worldTransform);
            QPixmap modSrc = src;
            QPointF modOffset;
            // This can be nothing but a mask effect and it can produce smaller output if
            // the source is based on a scroll area (or anything that clips its children,
            // as long as the source pixmaps are not restricted to obey the clipping). So
            // have its output in a temporary pixmap and copy it into the original source
            // pixmap to the (hopefully) appropriate position afterwards.
            effect->performEffectSw(painter, &modSrc, &modOffset);
            qreal dx = modOffset.x() - offset.x();
            qreal dy = modOffset.y() - offset.y(); 
            painter->setWorldTransform(QTransform());
            if (dx >= 0 && dy >= 0) {
                src.fill(Qt::transparent);
                QPainter p(&src);
                p.drawPixmap(QPointF(dx, dy), modSrc);
                p.end();
                srcVgImage = QVariant::fromValue<VGImage>(qPixmapToVGImage(src));
            }
            break;
        }
    }
    bool hadNormalEffects = false;
    foreach(HbVgEffect * effect, d->effects) {
        if (effect->chainBehavior() == ChainBehavAsSource) {
            continue;
        }
        // Set up srcPixmap and others for the individual effects
        // because the base class does it only for us, not for the
        // contained ones.
        HbVgEffectPrivate *effD = HbVgEffectPrivate::d_ptr(effect);
        effD->srcPixmap = src;
        effD->worldTransform = d->worldTransform;
        // Draw.
        effect->performEffect(painter, offset, srcVgImage, vgImageSize);
        // The flags must be cleared manually for the contained effects.
        effD->paramsChanged = effD->cacheInvalidated = false;
        if (effD->alwaysClearPixmaps || HbInstancePrivate::d_ptr()->mDropHiddenIconData) {
            effD->clearPixmaps();
        }
        hadNormalEffects = true;
    }
    // If there are no effects in the chain then just draw the source.
    if (d->effects.isEmpty() || !hadNormalEffects) {
        painter->drawPixmap(offset, src);
    }

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
 * Sw-mode for a chained effect does not make much sense and will
 * usually not have any good results because typically the only
 * sw-based effect is the mask effect and that would need special
 * handling which is impossible to provide here.
 */
void HbVgChainedEffect::performEffectSw(QPainter *devicePainter,
                                        QPixmap *result,
                                        QPointF *resultPos)
{
    Q_D(HbVgChainedEffect);
    foreach(HbVgEffect * effect, d->effects) {
        effect->performEffectSw(devicePainter, result, resultPos);
        HbVgEffectPrivate *effD = HbVgEffectPrivate::d_ptr(effect);
        effD->paramsChanged = effD->cacheInvalidated = false;
    }
    if (d->effects.isEmpty()) {
        drawSource(devicePainter);
    }
}

/*!
  \reimp
*/
void HbVgChainedEffectPrivate::notifyCacheInvalidated()
{
    // Distribute the new value of cacheInvalidated to all the
    // contained effects.
    foreach(HbVgEffect * effect, effects) {
        HbVgEffectPrivate *effD = HbVgEffectPrivate::d_ptr(effect);
        effD->cacheInvalidated = true;
        effD->notifyCacheInvalidated();
    }
}
