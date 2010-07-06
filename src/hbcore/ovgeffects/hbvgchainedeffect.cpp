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
    foreach(HbVgEffect * effect, d->effects) {
        // Set up srcPixmap and others for the individual effects
        // because the base class does it only for us, not for the
        // contained ones.
        HbVgEffectPrivate *effD = HbVgEffectPrivate::d_ptr(effect);
        effD->srcPixmap = d->srcPixmap;
        effD->worldTransform = d->worldTransform;
        // Draw.
        effect->performEffect(painter, offset, vgImage, vgImageSize);
        // The flags must be cleared manually for the contained effects.
        effD->paramsChanged = effD->cacheInvalidated = false;
    }
    // If there are no effects in the chain then just draw the source.
    if (d->effects.isEmpty()) {
        painter->drawPixmap(offset, d->srcPixmap);
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
 */
void HbVgChainedEffect::performEffectSw(QPainter *painter)
{
    Q_D(HbVgChainedEffect);
    foreach(HbVgEffect * effect, d->effects) {
        effect->performEffectSw(painter);
        HbVgEffectPrivate *effD = HbVgEffectPrivate::d_ptr(effect);
        effD->paramsChanged = effD->cacheInvalidated = false;
    }
    if (d->effects.isEmpty()) {
        drawSource(painter);
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
