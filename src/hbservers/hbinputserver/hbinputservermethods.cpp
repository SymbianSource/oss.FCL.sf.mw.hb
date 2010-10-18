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

#include "hbinputservermethods_p.h"

#include <QDateTime>
#include <QBuffer>
#include <QFileSystemWatcher>
#include <QSharedMemory>
#include <hbinputsettingproxy.h>
#include <hbinputmodecache_p_p.h>

HbInputServerMethods::HbInputServerMethods(HbInputServer *server)
    : mServer(server),
      mSharedMethods(0),
      mSharedMethodsModTime(0),
      mMethods(0),
      mMethodsSerialized(new QByteArray()),
      mMethodPathWatcher(0)
{
}

HbInputServerMethods::~HbInputServerMethods()
{
    delete mMethodsSerialized;
    delete mSharedMethods;
    delete mSharedMethodsModTime;
    delete mMethods;
    delete mMethodPathWatcher;
}

/*
Initializes input method sharing.

Creates shared memories for input method list and its latest modification time.
Initializes the list based on available input method plugins and updates the modification time.

Returns true on success, false if not able to create shared memories.
*/
bool HbInputServerMethods::initialize()
{
    mServer->debugPrint("Initializing input mode cache");
    // Create shared memory block with last modification time
    mSharedMethodsModTime = new QSharedMemory(HbInputMethodListModTimeKey);
    if (!mSharedMethodsModTime->create(sizeof(uint))) {
        mServer->debugPrint("Unable to create method list mod time shared memory block!");
        return false;
    }
    // Read mode data from disk to local structure
    updateLocalInputMethodList();
    // Create shared memory object for mode cache
    mSharedMethods = new QSharedMemory(HbInputMethodListKey);

    updateSharedMethodList();

    startMonitoringMethodListChanges();

    mServer->debugPrint("Input mode cache initialized");

    return true;
}

/*
Updates shared input methods list based on local list, updates modification time.
*/
void HbInputServerMethods::updateSharedMethodList()
{
    mServer->debugPrint("Updating shared memory for input methods");
    if (!mSharedMethods->isAttached()) {
        // If shared memory block is not attached, it should be created first
        mServer->debugPrint("Creating shared memory for input methods");
        mSharedMethods->create(mMethodsSerialized->size()+sizeof(int));
    } else if (mSharedMethods->size() < static_cast<int>(mMethodsSerialized->size()+sizeof(int))) {
        // If the shared memory block is too small, detach from it and recreate it
        mServer->debugPrint("Re-creating shared memory for input methods");
        mSharedMethods->detach();
        bool success = mSharedMethods->create(mMethodsSerialized->size()+sizeof(int));
        if (!success) {
            mServer->debugPrint("Re-creating shared memory for input methods failed, error: "
                                + mSharedMethods->errorString());
        }
    }

    // Externalize data from local structure to shared memory
    // Data size needs to be saved, since QSharedMemory is reserved in bigger blocks
    // and we can't make a block of exactly the required size
    mSharedMethods->lock();
    *static_cast<int*>(mSharedMethods->data()) = mMethodsSerialized->size();
    memcpy(static_cast<char*>(mSharedMethods->data())+sizeof(int), mMethodsSerialized->constData(), mMethodsSerialized->size());
    mSharedMethods->unlock();

    // Update modification time
    mSharedMethodsModTime->lock();
    *static_cast<uint*>(mSharedMethodsModTime->data()) = QDateTime::currentDateTime().toTime_t();
    mSharedMethodsModTime->unlock();
    mServer->debugPrint("Shared memory for input methods updated");
}

/*
Updates local copy of input methods list based on available input method plugins.
If path is given, updates only the methods in that directory.
*/
void HbInputServerMethods::updateLocalInputMethodList(const QString &path)
{
    mServer->debugPrint("Updating local copy of methods list");
    if (!mMethods) {
        mMethods = new QList<HbInputMethodListItem>();
    }
    if (!path.isEmpty()) {
        HbInputModeCachePrivate::readInputMethodDataFromDisk(mMethods, QDir(path));
    } else  {
        HbInputModeCachePrivate::readInputMethodDataFromDisk(mMethods);
    }

    // Clear deleted items from the list
    for (int i = mMethods->size()-1; i >= 0 ; --i) {
        if (mMethods->at(i).toBeRemoved) {
            mMethods->removeAt(i);
        }
    }

    mServer->debugPrint("Number of methods: " + QString::number(mMethods->count()));

    QBuffer buffer(mMethodsSerialized);
    buffer.open(QIODevice::WriteOnly);

    QDataStream out(&buffer);
    out << *mMethods;
    mServer->debugPrint("Method list updated, new size: " + QString::number(mMethodsSerialized->size()));
}

/*
Sets up directory watchers etc. to monitor changes in input method plugins.
*/
void HbInputServerMethods::startMonitoringMethodListChanges()
{
    mServer->debugPrint("Setting up change monitoring for input methods");
    if (mMethodPathWatcher) {
        mServer->debugPrint("Directory watcher already exists, deleting");
        delete mMethodPathWatcher;
    }

    mMethodPathWatcher = new QFileSystemWatcher();

    // Get the list of plugin paths, remove ROM drive from the paths if it's there
    QStringList directoriesToWatch = HbInputSettingProxy::inputMethodPluginPaths();
    foreach(const QString& path, directoriesToWatch) {
        if (path.length() > 0 && path.at(0) == 'z') {
            directoriesToWatch.removeOne(path);
        }
    }

    // TODO: Create directories to watch on non-removable drives (c, e on symbian) if they don't exist
    mMethodPathWatcher->addPaths(directoriesToWatch);
    connect(mMethodPathWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(changeInMethodDirectory(QString)));
    mServer->debugPrint("Input method path watcher setup done");

    // TODO: Set up MMC handling
}

/*
Slot directory watchers call when something changes.
*/
void HbInputServerMethods::changeInMethodDirectory(const QString &path)
{
    mServer->debugPrint("Change in method path detected, path: " + path);
    updateLocalInputMethodList(path);
    updateSharedMethodList();
}
