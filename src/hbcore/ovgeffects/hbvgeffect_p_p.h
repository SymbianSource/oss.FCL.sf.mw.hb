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

#ifndef HBVGEFFECT_P_P_H
#define HBVGEFFECT_P_P_H

#include <hbglobal.h>
#include <QMetaType>
#include <QPixmap>
#include <QTransform>

#if defined(HB_EFFECTS_OPENVG)
#if defined(QT_LOWER_CASE_VG_INCLUDES)
#include <vg/openvg.h>
#else
#include <VG/openvg.h>
#endif
Q_DECLARE_METATYPE(VGImage)
Q_DECL_IMPORT VGImage qPixmapToVGImage(const QPixmap &pixmap); // see qpixmapdata_vg.cpp in qt/src/openvg
#endif

#include "hbvgeffect_p.h"
#include "hbvgeffecttraces_p.h"

// Watch out for the fact that qreal is float on arm and double on win32 or linux.
// Try sticking to float literals so that no doubles will be involved at all on arm.

const qreal HBVG_EPSILON = 0.001f;

// clamp() must support all combinations of float/double parameters.
template<class T1, class T2, class T3> inline T1 clamp(const T1 value, const T2 minimum, const T3 maximum)
{
    return qMax(qMin((T1) maximum, value), (T1) minimum);
}

class HbMainWindow;

class HbVgEffectPrivate
{
    Q_DECLARE_PUBLIC(HbVgEffect)

public:
    HbVgEffectPrivate();
    virtual ~HbVgEffectPrivate();

#ifdef HB_EFFECTS_OPENVG
    VGImage ensurePixmap(QPixmap *pixmap, const QSize &size);
#endif

    qreal mainWindowRotation() const;
    QTransform rotationTransform() const;
    QPointF mapOffset(const QPointF &point) const;
    QSizeF mapSize(const QSizeF &size) const;

    static HbVgEffectPrivate *d_ptr(HbVgEffect *effect) { return effect->d_func(); }

    void ensureCacheInvalidated();

    // Called whenever cacheInvalidated is changed to true. Derived classes can override
    // this function if they need to perform some additional operation right away whenever
    // the pixmap cache gets invalidated.
    virtual void notifyCacheInvalidated() { }

    HbVgEffect *q_ptr;

    // Flag to indicate that the effect parameters have changed. It is set to true on
    // construction and set to false after each call to performEffect().  Also, every
    // effect parameter setter should set it to true by calling updateEffectBoundingRect()
    // or updateEffect() so the effect implementation can decide to use some previous
    // cached value of various internal structures if this flag indicates that the
    // parameters are same as before.  Note that this has nothing to do with bounding rect
    // or apperance changes that are not caused by the effects themselves, this flag is
    // used to indicate parameter related changes only. Effects should never try to cache
    // data that depends on the appearance of the item by relying solely on this flag. For
    // such purposes use cacheInvalidated.
    bool paramsChanged;

    // Flag to indicate that any possible cache, that depends on the effect parameters or
    // the size, appearance, etc. of the item, needs to be cleared. Use this flag only
    // when the effect implementation caches some visual data that depends on the
    // appearance of the source item. For cached data that depends on the parameter values
    // only, use paramsChanged instead.
    bool cacheInvalidated;

    qreal opacity;

    bool caching;

    // Before each call to performEffect() the source pixmap is stored here.
    QPixmap srcPixmap;

    QPixmap dstPixmap;
    QPixmap tmpPixmap;
    
    // Root of the chain, null by default.  If non-null then updates are
    // delegated to this effect.  Note that the sourceXxxx() functions in
    // QGraphicsEffect are not valid for chained effects so call those functions
    // on rootEffect instead (when it is non-null).
    HbVgEffect *rootEffect;

    // Contains the painter's world transformation. When performEffect() is
    // invoked, the world transformation is cleared, however the effect can
    // still access it via this member.
    QTransform worldTransform;

    // The graphics item on which the effect is installed or null.
    QGraphicsItem *sourceGraphicsItem;

    mutable HbMainWindow *mainWindow;
    mutable qreal lastUsedRotation;
    mutable QTransform lastRotationTransform;
    mutable qreal lastRotationTransformAngle;
};

#endif
