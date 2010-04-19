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

#ifndef HBPLUGINNAMECACHE_P_H
#define HBPLUGINNAMECACHE_P_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QMutex>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QPluginLoader>

#include <hbglobal.h>

class HbPluginNameCacheThread;
class HbLockedPluginLoader;

class HB_CORE_PRIVATE_EXPORT HbPluginNameCache : public QObject
{
    Q_OBJECT

    friend class HbPluginNameCacheThread;
    friend class HbLockedPluginLoader;

public:

    typedef QStringList (*GetPluginKeys)(QObject *pluginInstance);
    typedef QString (*PluginFileNameFilter)();

    explicit HbPluginNameCache(GetPluginKeys getPluginKeys,
        PluginFileNameFilter pluginFileNameFilter = 0, QObject *parent = 0);
    ~HbPluginNameCache();

    void addWatchPath(const QString &path);
    void removeWatchPath(const QString &path);
    void scanDirectory(const QString &path);
    QString find(const QString &key);
    static QString directoryPath(const QString &path);

    void print();
private:
    void insert(const QStringList &keys, const QString &filePath);
    void remove(const QString &key);
    void removePath(const QString &filePath);
    static QString pluginFileNameFilter();

private slots:
    void directoryChanged(const QString &path);

private: // data
    Q_DISABLE_COPY(HbPluginNameCache)
    QHash<QString, QString> mCache;
    HbPluginNameCacheThread *mThread;
    QFileSystemWatcher mWatcher;
};

class HbLockedPluginLoader
{
public:
    HbLockedPluginLoader(HbPluginNameCache &nameCache, const QString &fileName, QObject *parent = 0);
    HbLockedPluginLoader(QMutex &mutex, const QString &fileName, QObject *parent = 0);
    ~HbLockedPluginLoader();
    QObject *instance();
    bool load();
    bool unload();
    QString fileName() const {return mLoader->fileName();}

private:
    QMutex &mMutex;
    QPluginLoader *mLoader;
};

#endif // HBPLUGINNAMECACHE_P_H

