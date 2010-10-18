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

#include "hbvgeffect_p.h"
#include "hbvgeffect_p_p.h"
#include <QPainter>
#include <QPaintEngine>
#include <QPixmapCache>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QSet>
#include <hbmainwindow.h>
#include <hbmainwindow_p.h>
#include <hbinstance_p.h>

/*!
 * \class HbVgEffect
 *
 * \brief Abstract base class for OpenVG effects.
 *
 * \internal
 *
 * Brief guide for creating new effects:
 * <ul>
 *
 *    <li>Derive from HbVgEffect.
 *
 *    <li>Add getter/setter/property/signal for new effect parameters.
 *
 *    <li>In the setter make sure to call either updateEffectBoundingRect() or
 *    updateEffect(), depending on the parameter (some parameters affect the
 *    bounding rectangle, some not).
 *
 *    <li>Reimplement boundingRectFor().
 *
 *    <li>Implement performEffect(). This function will typically generate
 *    VGImages containing the filtered source image (or just the extra visuals
 *    that appear due to the effect) and draw them via the given painter's
 *    drawPixmap() (or vgDrawImage). Do not forget to draw the source image too,
 *    if needed.  The VGImage argument is passed in a variant to keep things
 *    building even without OpenVG headers/libraries.  Always use an appropriate
 *    ifdef for code that uses OpenVG types or functions.
 *
 * </ul>
 *
 * Note that these effects must not be installed using QGraphicsItem::setGraphicsEffect(),
 * use install() instead.
 *
 * \sa install()
 */

/*!
 * \fn void HbVgEffect::performEffect(QPainter *painter, const QPointF &offset,
 * const QVariant &vgImage, const QSize &vgImageSize)
 *
 * This function must be implemented in derived classes, providing the actual
 * implementation for the effect generation.
 *
 * \a painter is the painter to be used (it is guaranteed to be an OpenVG-based
 * one), \a offset is the target position, the variant \a vgImage contains a
 * VGImage handle for the source image, and \a vgImageSize is the size of that
 * image. The handle in \a vgImage is actually the underlying VGImage of the
 * pixmap stored in d->srcPixmap.
 *
 * Note that the size of this pixmap (and thus vgImageSize) already includes the
 * extra margins added by the effect's boundingRectFor, so drawing the entire
 * pixmap at any other position than \a offset will lead to painting outside the
 * item's effective bounding rectangle which may cause artifacts on the screen.
 *
 * \sa QGraphicsEffect::boundingRectFor()
 */

Q_GLOBAL_STATIC(QSet<QString>, cacheKeys)

HbVgEffectPrivate::HbVgEffectPrivate()
    : q_ptr(0),
      paramsChanged(true),
      cacheInvalidated(true),
      opacity(1),
      caching(false),
      rootEffect(0),
      sourceGraphicsItem(0),
      mainWindow(0),
      lastUsedRotation(0),
      lastRotationTransformAngle(0),
      forceSwMode(false),
      alwaysClearPixmaps(true) // Enable it always for now, due to limited gpu memory.
{
}

/*!
 * \internal
 */
HbVgEffectPrivate::~HbVgEffectPrivate()
{
}

#ifdef HB_EFFECTS_OPENVG
/*!
 * Makes sure that the given pixmap has the given \a size.  Returns
 * the underlying VGImage handle.  Typically used with dstPixmap and
 * tmpPixmap.
 *
 * \internal
 */
VGImage HbVgEffectPrivate::ensurePixmap(QPixmap *pixmap, const QSize &size)
{
    if (pixmap->size() != size) {
        *pixmap = QPixmap(size);
    }
    return qPixmapToVGImage(*pixmap);
}
#endif

/*!
 * Clears all the pixmaps that live as member variables. This is not mandatory,
 * but helps reducing graphics memory usage.
 *
 * The downside is decreased performance due to new pixmap creation in the next
 * ensurePixmap() call.
 *
 * \internal
 */
void HbVgEffectPrivate::clearPixmaps()
{
    // Note: If the effect used tryCache() then the underlying pixmap data for
    // dstPixmap will not really be destroyed here due to implicit sharing.
    srcPixmap = dstPixmap = tmpPixmap = QPixmap();
}

/*!
 * Invalidates the cache but only if it has not been done already.
 *
 * \internal
 */
void HbVgEffectPrivate::ensureCacheInvalidated()
{
    if (!cacheInvalidated) {
#ifdef HBVG_TRACES
        Q_Q(HbVgEffect);
        qDebug("HbVgEffect [%x]: invalidating cache", (int) q);
#endif
        cacheInvalidated = true;
        notifyCacheInvalidated();
    }
}

/*!
 * Returns the rotation of the graphics view in degrees.
 *
 * For some effects it is important to know if the graphics view has been
 * transformed or not (e.g. a reflection effect may need to draw at a different
 * position when the graphics view is rotated).
 *
 * \internal
 */
qreal HbVgEffectPrivate::mainWindowRotation() const
{
    if (!mainWindow) {
        Q_Q(const HbVgEffect);
        const QGraphicsItem *srcItem = q->sourceItemForRoot();
        if (srcItem) {
            QGraphicsScene *scene = srcItem->scene();
            if (scene) {
                QList<QGraphicsView *> views = scene->views();
                foreach(QGraphicsView * view, views) {
                    mainWindow = qobject_cast<HbMainWindow *>(view);
                    if (mainWindow) {
                        break;
                    }
                }
            }
        }
    }
    return mainWindow ? HbMainWindowPrivate::d_ptr(mainWindow)->rotation() : 0;
}

/*!
 * Returns a transformation that defines the graphics view's current rotation.
 *
 * \internal
 */
QTransform HbVgEffectPrivate::rotationTransform() const
{
    int rotation = (int) mainWindowRotation();
    if (rotation != lastRotationTransformAngle) {
        lastRotationTransformAngle = rotation;
        lastRotationTransform = QTransform().rotate(rotation);
    }
    return lastRotationTransform;
}

/*!
 * Maps the given translation offset to an offset that is based on the current
 * rotation of the graphics view. Effects are drawing directly, in device
 * coordinates, however they must take into account the rotation of the graphics
 * view too. Therefore any translation they use must be mapped using this
 * function. (NB. this does not apply to the offset argument of
 * performEffect()).
 *
 * For example if a dropshadow effect gets (x, y) as the offset of the shadow,
 * it must call this function instead of using the offset as it is, because the
 * correct device coordinates for the offset may be something different,
 * e.g. (y, -x) if the graphics view is rotated -90 degrees.
 *
 * \internal
 */
QPointF HbVgEffectPrivate::mapOffset(const QPointF &offset) const
{
    return rotationTransform().map(offset);
}

/*!
 * Returns a size that has the width and height swapped if the current rotation
 * of the graphics view implies that. Typically used with size-like values,
 * e.g. the radius for a blur effect.
 *
 * \internal
 */
QSizeF HbVgEffectPrivate::mapSize(const QSizeF &size) const
{
    QPointF p = mapOffset(QPointF(size.width(), size.height()));
    return QSizeF(qAbs(p.x()), qAbs(p.y()));
}

/*!
 * \internal
 *
 * Returns a cache key that is unique to this effect instance (but only for the
 * lifetime of the instance).
 */
inline QString cacheKey(const HbVgEffect *e)
{
    return QString::number((qptrdiff) e);
}

HbVgEffect::HbVgEffect(QObject *parent)
    : QGraphicsEffect(parent), d_ptr(new HbVgEffectPrivate)
{
    Q_D(HbVgEffect);
    d->q_ptr = this;
}

HbVgEffect::HbVgEffect(HbVgEffectPrivate &dd, QObject *parent)
    : QGraphicsEffect(parent), d_ptr(&dd)
{
    Q_D(HbVgEffect);
    d->q_ptr = this;
}

HbVgEffect::~HbVgEffect()
{
    // Get rid of the cached pixmap if it is still present.
    QString key = cacheKey(this);
    QPixmapCache::remove(key);
    cacheKeys()->remove(key);
    delete d_ptr;
}

/*!
 * Updates the source item (and thus the effect). Also sets the paramsChanged
 * and cacheInvalidated flags. This function should be called from the effect
 * parameter setters only.
 *
 * Note that this is not sufficient if the results of boundingRectFor() are also
 * changing due to an updated parameter value. In that case call
 * updateEffectBoundingRect() instead.
 *
 * \sa updateEffectBoundingRect()
 */
void HbVgEffect::updateEffect()
{
    Q_D(HbVgEffect);
    d->paramsChanged = true;
    d->ensureCacheInvalidated();
    if (d->rootEffect) {
        d->rootEffect->updateEffect();
    } else {
        update();
    }
}

/*!
 * Notifies that the bounding rectangle for the effect (i.e. the return value of
 * boundingRectFor()) is possibly different than before. Also updates the source
 * item (and thus the effect) and sets the paramsChanged and cacheInvalidated
 * flags. This function should be called from the effect parameter setters only.
 *
 * When changing effect parameters that have no effect on the bounding
 * rectangle, use updateEffect() instead because that might be more efficient.
 *
 * \sa updateEffect()
 */
void HbVgEffect::updateEffectBoundingRect()
{
    Q_D(HbVgEffect);
    d->paramsChanged = true;
    d->ensureCacheInvalidated();
    if (d->rootEffect) {
        d->rootEffect->updateBoundingRect();
    } else {
        updateBoundingRect();
    }
}

/*!
 * Enables delegation of update requests to the given \a effect.
 *
 * This is needed when there are several effects in a chain and thus all update
 * requests should be routed from the individual elements to the container
 * effect.
 */
void HbVgEffect::setChainRoot(HbVgEffect *effect)
{
    Q_D(HbVgEffect);
    d->rootEffect = effect;
}

/*!
 * Returns the current root of the chain or null if this effect is not part of
 * any delegation chain. Do not assume that the returned effect is a
 * HbVgChainedEffect, it may also be something else (in theory), so use
 * qobject_cast if the return value needs to be casted to a more specific type.
 */
HbVgEffect *HbVgEffect::chainRoot() const
{
    Q_D(const HbVgEffect);
    return d->rootEffect;
}

/*!
 * Returns the current opacity parameter of the effect.
 *
 * \sa setOpacity()
 */
qreal HbVgEffect::opacity() const
{
    Q_D(const HbVgEffect);
    return d->opacity;
}

/*!
 * Sets the \a opacity parameter of the effect.
 * The value must fall between 0.0 and 1.0.
 *
 * \sa opacity()
 */
void HbVgEffect::setOpacity(qreal opacity)
{
    Q_D(HbVgEffect);
    if (d->opacity == opacity) {
        return;
    }
    d->opacity = opacity;
    updateEffect();
    emit opacityChanged(opacity);
}

/*!
 * Returns the current setting for sw mode forcing. By default this is
 * disabled (false).
 */
bool HbVgEffect::forceSwMode() const
{
    Q_D(const HbVgEffect);
    return d->forceSwMode;
}

/*!
 * When enabled the sw implementation is used always.
 *
 * Many effects have no software implementation so enabling this will
 * lead to the same result as trying to paint the hw accelerated
 * effects on a non-hw paint engine: Only the source is painted,
 * without any effects.
 */
void HbVgEffect::setForceSwMode(bool b)
{
    Q_D(HbVgEffect);
    if (d->forceSwMode == b) {
        return;
    }
    d->forceSwMode = b;
    updateEffect();
    emit forceSwModeChanged(b);
}

/*!
 * Called when using a non-OpenVG paint engine. The default
 * implementation simply calls drawSource(), i.e. paints the source
 * item without any effect.
 *
 * Note that the source pixmap is not requested and the painter's
 * world transform is not reset before calling this, in contrast to
 * performEffect(). (this means that neither d->srcPixmap nor
 * d->worldTransform is available).
 *
 * If the effect is able to render itself into a pixmap then it should do so
 * when \a result and \a resultPos are not 0. In this case the effect should not
 * paint anything onto \a devicePainter but all its output should go into a
 * newly created pixmap starting at position (0, 0). This pixmap is then
 * returned to the caller in \a result. The offset that will be needed when
 * painting the pixmap onto the devicePainter is passed back in \a resultPos.
 */
void HbVgEffect::performEffectSw(QPainter *devicePainter, QPixmap *result, QPointF *resultPos)
{
    Q_UNUSED(result);
    Q_UNUSED(resultPos);
    drawSource(devicePainter);
}

/*!
 * \reimp
 *
 * Implementation of QGraphicsEffect's draw(). After setting up the environment
 * for direct OpenVG painting, the work is delegated to the performEffect()
 * implementation of the derived classes.
 */
void HbVgEffect::draw(QPainter *painter)
{
    Q_D(HbVgEffect);

    // Invalidate the cache if the rotation of the graphics view has changed.
    qreal rotation = d->mainWindowRotation();
    if (rotation != d->lastUsedRotation) {
        d->lastUsedRotation = rotation;
        d->ensureCacheInvalidated();
    }

    // Just draw the source without effects if the painter's paint engine
    // is not using OpenVG.
    QPaintEngine *paintEngine = painter->paintEngine();
    if (d->forceSwMode || !paintEngine || paintEngine->type() != QPaintEngine::OpenVG) {
        // No sourcePixmap() and world transform change here because
        // in most cases we will just call drawSource().
        performEffectSw(painter, 0, 0);
        d->paramsChanged = d->cacheInvalidated = false;
        return;
    }

#ifdef HB_EFFECTS_OPENVG
    // Render the source into a pixmap. Note that the effect's modified bounding
    // rectangle is already taken into account in the size of the pixmap created
    // here, so there is no need to waste time with boundingRectFor() calls
    // later in performEffect() when creating temporary buffers.
    QPoint offset;
    // Don't bother with optimizations based on sourceIsPixmap() because we will
    // rarely be used on QGraphicsPixmapItems anyway.
    d->srcPixmap = sourcePixmap(Qt::DeviceCoordinates, &offset);

    // Pull out the underlying VGImage from the pixmap.
    VGImage vgImage = qPixmapToVGImage(d->srcPixmap);
    // If something went wrong then just draw the source without effects.
    if (vgImage == VG_INVALID_HANDLE) {
        drawSource(painter);
        return;
    }

    // Save the world transformation and then reset it.
    d->worldTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());

    // Enter raw VG mode.
    painter->beginNativePainting();
    // Draw something.
    performEffect(painter, offset, QVariant::fromValue<VGImage>(vgImage), d->srcPixmap.size());
    // Clear the param/cache flags as all data should be up-to-date now.
    d->paramsChanged = d->cacheInvalidated = false;
    // Leave raw VG mode.
    painter->endNativePainting();

    // Restore the painter's previously set transformations.
    painter->setWorldTransform(d->worldTransform);

    // Make sure we don't keep any unused pixmaps alive in case the application
    // wants to minimize gpu memory usage (on the expense of performance).
    if (d->alwaysClearPixmaps || HbInstancePrivate::d_ptr()->mDropHiddenIconData) {
        d->clearPixmaps();
    }

#else
    // OpenVG code disabled => effect is not shown (but have the source drawn still).
    drawSource(painter);
#endif
}

/*!
 *  \reimp
 */
void HbVgEffect::sourceChanged(ChangeFlags flags)
{
    // If the bounding rectangle or appearance of the source has changed then
    // set our flag to indicate that any cached visual data will have to be
    // dropped.
    if (flags & SourceBoundingRectChanged || flags & SourceInvalidated) {
        Q_D(HbVgEffect);
        d->ensureCacheInvalidated();
    }
}

/*!
 * Returns the current status of caching.
 *
 * \sa setCaching()
 */
bool HbVgEffect::caching() const
{
    Q_D(const HbVgEffect);
    return d->caching;
}

/*!
 * Enables or disables caching of the output of the effects.
 *
 * By default caching is disabled.
 *
 * Note that caching may not cause any increase in performance if the size or
 * apperance of the source item is changing rapidly, it is only useful for items
 * that are static enough.
 *
 * Enabling this setting may have no effect because some subclasses may not be
 * able to support it, it is treated as a hint only.
 *
 * \sa caching()
 */
void HbVgEffect::setCaching(bool caching)
{
    Q_D(HbVgEffect);
    if (d->caching == caching) {
        return;
    }
    d->caching = caching;
    emit cachingChanged(caching);
}

/*!
 * Returns the cached pixmap that is associated with this effect instance. Will
 * return a null pixmap if caching is disabled or there was no tryCache() call
 * for this instance before or the cache has been invalidated (e.g. because the
 * source item's appearance has changed).
 *
 * \sa tryCache()
 */
QPixmap HbVgEffect::cached(const QSize &size) const
{
    Q_D(const HbVgEffect);
    if (d->caching) {
        QString key = cacheKey(this);
        if (d->cacheInvalidated) {
            QPixmapCache::remove(key);
            cacheKeys()->remove(key);
        } else {
            QPixmap cachedPm;
            // Check if we have a pixmap in the cache, then check the size
            // too. The size check is important because the effects will
            // typically render their source using device coordinates which
            // causes a clipping to the device viewport, therefore the cached
            // pixmap for an item that was/is clipped should not be used.
            if (QPixmapCache::find(key, &cachedPm)
                    && (size.isNull() || cachedPm.size() == size)) {
#ifdef HBVG_TRACES
                qDebug("HbVgEffect [%x]: cache hit", (int) this);
#endif
                return cachedPm;
            }
        }
    }
    return QPixmap();
}

/*!
 * Inserts/replaces the given pixmap into/in the cache if caching is enabled.
 * The pixmap will be associated with this effect instance and can be retrieved
 * later by calling cached().
 *
 * Note that the OpenVG effects rely on "out-of-sync" pixmaps, i.e. pixmaps
 * where the underlying QImage and VGImage are not synchronized. Therefore the
 * QPixmapCache must be cleared whenever there is a risk that some other entity
 * (Qt) would destroy the underlying VGImages. This is currently handled in
 * HbForegroundWatcher and HbOogmWatcher by calling releaseCachedResources().
 *
 * \sa cached()
 */
void HbVgEffect::tryCache(const QPixmap &pm)
{
    Q_D(HbVgEffect);
    if (d->caching) {
        QString key = cacheKey(this);
        QPixmapCache::insert(key, pm);
        cacheKeys()->insert(key);
    }
}

/*!
 * Drops all cached data used by any effect in the application. Other pixmaps
 * (not related to effects) are left intact in the global pixmap cache.
 */
void HbVgEffect::releaseCachedResources()
{
    QSet<QString> *keys = cacheKeys();
    foreach(const QString & key, *keys) {
        QPixmapCache::remove(key);
    }
    keys->clear();
}

/*!
 * Returns the source graphics item for this effect or null if it is not
 * available for some reason. Also works for chained effects (i.e. it uses the
 * root effect if needed).
 *
 * NOTE: If the effect has not been attached to a graphics item using install()
 * then this function returns null (even if setGraphicsEffect() was used). So
 * use install().
 */
const QGraphicsItem *HbVgEffect::sourceItemForRoot() const
{
    Q_D(const HbVgEffect);
    const HbVgEffectPrivate *p = d->rootEffect ? HbVgEffectPrivate::d_ptr(d->rootEffect) : d;
    return p->sourceGraphicsItem;
}

/*!
 * Returns the bounding rectangle of the source graphics item. Also works for
 * chained effects (it will use the root effect if needed).
 */
QRectF HbVgEffect::sourceBoundingRectForRoot() const
{
    Q_D(const HbVgEffect);
    return d->rootEffect ? d->rootEffect->sourceBoundingRect() : sourceBoundingRect();
}

/*!
 * Calls sourcePixmap() for this effect, or, in case of chained effects, for the
 * root of the chain.
 */
QPixmap HbVgEffect::sourcePixmapForRoot(Qt::CoordinateSystem system, QPoint *offset)
{
    Q_D(HbVgEffect);
    return d->rootEffect
        ? d->rootEffect->sourcePixmapForRoot(system, offset)
        : sourcePixmap(system, offset);
}

/*!
 * Installs the effect on a given graphics item. This is merely a wrapper to
 * QGraphicsItem::setGraphicsEffect(). To uninstall the effect use
 * setGraphicsEffect in the usual way (i.e. pass null). This function will do
 * nothing if \a item is null.
 *
 * Note that installing effects derived from HbVgEffect using
 * QGraphicsItem::setGraphicsEffect() will not work and will lead to undefined
 * behaviour. Use this function instead. You have been warned.
 */
void HbVgEffect::install(QGraphicsItem *item)
{
    if (item) {
        Q_D(HbVgEffect);
        d->sourceGraphicsItem = item;
        item->setGraphicsEffect(this);
    }
}

/*!
 * A sw-based mask effect should return ChainBehavAsSource. Anything else should
 * stick to the default ChainBehavNormal. There can only be 0 or 1 effect with
 * ChainBehavAsSource in a chain.
 */
HbVgEffect::ChainBehavior HbVgEffect::chainBehavior() const
{
    return ChainBehavNormal;
}
