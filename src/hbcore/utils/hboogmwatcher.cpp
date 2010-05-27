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

#include "hboogmwatcher_p.h"
#ifdef Q_OS_SYMBIAN
#include "hboogmwatcher_sym_p.h"
#else
#include "hboogmwatcher_dummy_p.h"
#endif
#include <hbiconitem.h>
#include <hbiconitem_p.h>
#ifdef HB_EFFECTS_OPENVG
#include <hbvgeffect_p.h>
#endif
#include <QApplication>
#include "hbiconloader_p.h"

/*!
  \class HbOogmWatcher

  \brief Listens for out-of-graphics memory notifications and distributes them
  via a signal. Also tries to reduce the resource usage of the pixmap cache and
  non-visible HbIconItem instances, whenever the low-graphics-memory event is
  received.

  \internal
*/

/*!
  \fn void HbOogmWatcher::graphicsMemoryNeeded()

  This signal is emitted when the amount of free graphics memory goes below some
  critical limit. Note that this has nothing to do with the amount of free
  system memory (RAM). When this signal is emitted, all entities, that have
  anything to do with graphics memory, are expected to perform as much cleanup
  as possible in order to increase the amount of free graphics memory.
*/

static bool oogmWatcherDeleted = false;

/*!
    Returns the global HbOogmWatcher instance.
 */
HbOogmWatcher *HbOogmWatcher::instance()
{
    if (oogmWatcherDeleted) {
        return 0;
    }
    static HbOogmWatcher *watcher = new HbOogmWatcher(qApp);
    return watcher;
}
    
HbOogmWatcher::HbOogmWatcher(QObject *parent)
    : QObject(parent), d_ptr(new HbOogmWatcherPrivate)
{
    Q_D(HbOogmWatcher);
    d->q_ptr = this;
    d->mRenderMode = EHWRendering;
}

HbOogmWatcher::~HbOogmWatcher()
{
    delete d_ptr;
    oogmWatcherDeleted = true;
}

/*!
  Forces the graphics-memory-low notifications to be sent.
  Useful for forcing icons to perform some cleanup, for example.
 */
void HbOogmWatcher::notifyGraphicsMemoryLow()
{
    Q_D(HbOogmWatcher);
    d->graphicsMemoryLow();
}

/*!
  \internal
*/
void HbOogmWatcherPrivate::graphicsMemoryLow()
{
    qWarning("HbOogmWatcher::graphicsMemoryLow()");
    if (mRenderMode == EHWRendering) {        
        mRenderMode = ESWRendering;
        HbIconLoader::global()->switchRenderingMode(mRenderMode);
    }    
#ifdef HB_EFFECTS_OPENVG
    // Destroy the cached pixmaps of effects. This is also necessary
    // to make the OpenVG filter effect caching working properly. (if
    // Qt would free the pixmaps' underlying VGImage when graphics
    // memory is low then that would render the filter effects' cached
    // pixmaps invalid so we just drop them here)
    HbVgEffect::releaseCachedResources();
#endif
    // Drop the underlying pixmap data (if possible) for all HbIconItems that
    // are not currently visible.
    int n = 0;
    foreach (HbIconItem *iconItem, mIconItems) {
        if (!iconItem->isVisible()) {
            HbIconItemPrivate::d_ptr(iconItem)->clearStoredIconContent();
            ++n;
        }
    }
    emit q_ptr->iconCleanupDone(n);
    emit q_ptr->graphicsMemoryLow();
}

/*!
  \internal
*/
void HbOogmWatcherPrivate::graphicsMemoryGood()
{
    qWarning("HbOogmWatcher::graphicsMemoryGood()");
    if (mRenderMode == ESWRendering) {        
        mRenderMode = EHWRendering;
        HbIconLoader::global()->switchRenderingMode(mRenderMode);
    }     
    emit q_ptr->graphicsMemoryGood();
}

/*!  Registers an HbIconItem instance. Whenever graphics memory is
  low, the registered iconitems will be examined and the ones that are
  not visible will be asked to drop the underlying data of their
  HbIcons.
 */
void HbOogmWatcher::registerIconItem(HbIconItem *item)
{
    Q_D(HbOogmWatcher);
    if (!d->mIconItems.contains(item)) {
        d->mIconItems.append(item);
    }
}

/*!
  Unregisters an HbIconItem. This must be done when the item is deleted, for example.
 */
void HbOogmWatcher::unregisterIconItem(HbIconItem *item)
{
    Q_D(HbOogmWatcher);
    d->mIconItems.removeOne(item);
}
