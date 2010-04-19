/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbServers module of the UI Extensions for Mobile.
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

#ifndef HBCACHE_H
#define HBCACHE_H

#include <hbglobal.h>
#include <QHash>
#include <QList>

struct HbCacheItem
{
    int offset;
    int refCount;
    QString fileName;

    HbCacheItem(int off = -1, int count = 0, const QString &fName = QString())
            : offset(off),
            refCount(count),
            fileName(fName)
    {

    }
    void incrementRefCount()
    {
        refCount++;
    }
    void decrementRefCount()
    {
        refCount--;
    }
};

class HbCache
{
public:
    HbCache();
    ~HbCache();
    void clear();
    HbCacheItem* value(const QString &key) const;
    HbCacheItem* cacheItem(const QString &key);
    bool insert(const QString &key, HbCacheItem *item);
    bool remove(const QString &key);
    bool contains(const QString &key) const;
    QList<HbCacheItem*> &lruList();
    QHash<QString, HbCacheItem*> &cacheHandle();

private:
    QHash<QString, HbCacheItem*> cache;
    QList<HbCacheItem*> unusedResources;
};


#endif /* HBCACHE_H */
