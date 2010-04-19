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

#include "hbframedrawerpool_p.h"
#include <QHash>
#include <QDebug>

//#define HB_FRAMEDRAWERPOOL_TRACE

/*!
  \class HbFrameDrawerPool

  \brief A global frame drawer pool that is used to reduce the number of
  HbFrameDrawer instances when the same multi-piece graphics is shown in several
  different primitives or widgets.

  \internal
 */

/*!
  \class HbFrameDrawerPoolKey

  \internal
 */
struct HbFrameDrawerPoolKey
{
public:
    HbFrameDrawerPoolKey(const QString &name,
                         HbFrameDrawer::FrameType type,
                         const QSizeF &size = QSizeF())
        : frameGraphicsName(name), frameGraphicsType(type), frameSize(size) { }
    QString frameGraphicsName;
    HbFrameDrawer::FrameType frameGraphicsType;
    QSizeF frameSize;
    bool operator==(const HbFrameDrawerPoolKey &other) const {
        return other.frameGraphicsName == frameGraphicsName
            && other.frameGraphicsType == frameGraphicsType
            && other.frameSize == frameSize;
    }
};

/*!
  \class HbFrameDrawerPoolValue

  \internal
 */
struct HbFrameDrawerPoolValue
{
    HbFrameDrawerPoolValue()
        : frameDrawer(0), refCount(0) { }
    HbFrameDrawerPoolValue(HbFrameDrawer *drawer)
        : frameDrawer(drawer), refCount(1) { }
    HbFrameDrawer *frameDrawer;
    int refCount;
};

uint qHash(const HbFrameDrawerPoolKey &key)
{
    return qHash(key.frameGraphicsName);
}

typedef QHash<HbFrameDrawerPoolKey, HbFrameDrawerPoolValue> HbFrameDrawerPoolData;
Q_GLOBAL_STATIC(HbFrameDrawerPoolData, poolData)

/*!  Returns a new or an existing HbFrameDrawer instance. Ownership of the
    pointer is NOT transfered to the caller. The returned pointer must never be
    destroyed with delete, use release() instead.

    \a frameSize is optional, if the default constructed QSizeF is
    used then it is ignored. Otherwise only framedrawers requested for
    the same size are shared (meaning that requesting a framedrawer
    for the same graphics with same type but different size will
    return a different framedrawer instance).
*/
HbFrameDrawer *HbFrameDrawerPool::get(const QString &frameGraphicsName,
                                      HbFrameDrawer::FrameType frameGraphicsType,
                                      const QSizeF &frameSize)
{
    HbFrameDrawer *frameDrawer = 0;
    HbFrameDrawerPoolKey key(frameGraphicsName, frameGraphicsType, frameSize);
    HbFrameDrawerPoolData *pool = poolData();
    if (pool->contains(key)) {
        HbFrameDrawerPoolValue value = pool->value(key);
        ++value.refCount;
        frameDrawer = value.frameDrawer;
        pool->insert(key, value);
    } else {
        frameDrawer = new HbFrameDrawer(frameGraphicsName, frameGraphicsType);
        pool->insert(key, HbFrameDrawerPoolValue(frameDrawer));
    }
#ifdef HB_FRAMEDRAWERPOOL_TRACE
    qDebug() << "HbFrameDrawerPool::get()" << key.frameGraphicsName << pool->value(key).refCount;
#endif
    return frameDrawer;
}

/*!  Releases the given framedrawer. Use this instead of delete when retrieving
  HbFrameDrawer pointers via get().

  If \a frameDrawer is not present in the global pool then it is destroyed by
  calling delete so this function can be used to destroy any kind of frame
  drawers.
 */
void HbFrameDrawerPool::release(HbFrameDrawer *frameDrawer)
{
    bool inPool = false;
    HbFrameDrawerPoolData *pool = poolData();
    foreach (const HbFrameDrawerPoolKey &key, pool->keys()) {
        HbFrameDrawerPoolValue value = pool->value(key);
        if (value.frameDrawer == frameDrawer) {
            if (!--value.refCount) {
                delete frameDrawer;
                pool->remove(key);
            } else {
                pool->insert(key, value);
            }
#ifdef HB_FRAMEDRAWERPOOL_TRACE
            qDebug() << "HbFrameDrawerPool::release()" << key.frameGraphicsName << value.refCount;
#endif
            inPool = true;
            break;
        }
    }
    if (!inPool) {
        // Was not found in pool, just delete it.
        delete frameDrawer;
    }
}
