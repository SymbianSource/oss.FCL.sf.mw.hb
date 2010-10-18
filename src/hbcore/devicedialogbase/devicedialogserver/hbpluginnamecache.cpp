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

#include "hbpluginnamecache_p.h"
#include <hbdevicedialogplugin.h>
#include <hbdevicedialogtrace_p.h>

#include <QDir>
#include <QMutexLocker>
#include <QThread>
#include <QStringList>

/*
    \class HbPluginNameCache
    \brief HbPluginNameCache caches plugin file names by keys

    Searching for a correct plugin can be time consuming. This class maintains a cache of plugin
    file names and corresponding keys the pluging implements and speeds up the search. Keys are
    strings the plugin returns.

    The cache watches a set of directories for changes. On directory change, cache is cleared from
    all entries with that directory and the directory is re-scanned and cache updated.
    Directory is added into cache by addWatchPath() function and removed by removeWatchPath().
    A directory that does not change and need watching, eg. a directory on a rom drive, can be
    added into the cache by calling scanDirectory().

    Directory scans are performed by a worker thread.

    HbPluginNameCache constructor takes as parameter a function pointer which it uses to read keys
    from a plugin. Class user must implement this function for plugins to be cached. Optionally
    a pointer to plugin file name filter function can be provided. If none is provided, a default
    is used.
*/

// QPluginLoader is not re-entrant if two threads are accessing the same plugin. In Windows/Linux
// where indicator/device-dialog plugin managers run in application process cache thread cannot
// be enabled as there is a chance that application is using QPluginLoader. On Symbian, device
// dialog manager runs in separate process and we can ensure that indicator/device-dialog plugins
// are not accessed outside of the manager.
#if defined(Q_OS_SYMBIAN)
#define USE_NAME_CACHE_THREAD 1
#endif

// Uncomment to monitor plugin installation directories and updating plugin name cache on directory
// changes. Otherwise name cache is updated when cache miss occurs.
// Currently directory monitoring is disabled in order slightly improve security. Plugin is not
// executed automatically after installation to get type strings. Instead application must trigger
// a plugin scan and name cache miss to get plugin executed.
//#define MONITOR_INSTALLATION_DIRS 1

// Thread that scans directories and keeps the cache current
/// \cond
class HbPluginNameCacheThread : public QThread
{
public:
    typedef HbPluginNameCache::ScanParameters WorkItem;

    HbPluginNameCacheThread(HbPluginNameCache &nameCache,
        HbPluginNameCache::GetPluginKeys getPluginKeys,
        HbPluginNameCache::PluginFileNameFilter pluginFileNameFilter);
    void scanDirectory(const WorkItem &workItem);
    void cancelScan(const QString &path);
    void stop();
    QMutex &lock(){return *mMutex;}

private:
    void doDirectoryScan(const WorkItem &workItem);
    void run();

private: // data
    HbPluginNameCache::GetPluginKeys mGetPluginKeys; // function to get keys from a plugin
    HbPluginNameCache::PluginFileNameFilter mPluginFileNameFilter;
    bool mExit;
    QList<WorkItem> mWorkQueue; // queue for directories to be scanned
    WorkItem mCurrentScan;  // directory currently scanned
    QStringList mLimitSet; // names of plugin-files found from rom-drives
    HbPluginNameCache &mNameCache;
    static QMutex *mMutex;
};
/// \endcond

// Share lock between all instances of the cache.
QMutex *HbPluginNameCacheThread::mMutex = 0;

// For file name compare
#if defined(Q_OS_LINUX) && !defined(Q_OS_SYMBIAN)
    static const Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive;
#else
    static const Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive;
#endif // defined(Q_OS_LINUX) && !defined(Q_OS_SYMBIAN)

HbPluginNameCacheThread::HbPluginNameCacheThread(HbPluginNameCache &nameCache,
    HbPluginNameCache::GetPluginKeys getPluginKeys,
    HbPluginNameCache::PluginFileNameFilter pluginFileNameFilter) :
    mGetPluginKeys(getPluginKeys), mPluginFileNameFilter(pluginFileNameFilter),
    mExit(false), mNameCache(nameCache)
{
    if (!mMutex) {
        mMutex = new QMutex();
    }
}

// Add directory to be scanned
void HbPluginNameCacheThread::scanDirectory(const WorkItem &workItem)
{
    QMutexLocker lock(mMutex);
    if (!mWorkQueue.contains(workItem)) {
        mWorkQueue.append(workItem);
    }
#ifdef USE_NAME_CACHE_THREAD
    if (!isRunning()) {
        mExit = false;
        start(IdlePriority);
    }
#else
    mExit = false;
    lock.unlock();
    run();
#endif
}

// Cancel a directory scan request
void HbPluginNameCacheThread::cancelScan(const QString &path)
{
    QMutexLocker lock(mMutex);
    // If scan is waiting in a queue, remove it
    WorkItem workItem(path);
    int i = mWorkQueue.indexOf(workItem);
    if (i >= 0) {
        mWorkQueue.removeAt(i);
    }
#ifdef USE_NAME_CACHE_THREAD
    // If thread is currently scanning the path, stop and wait for it to exit
    if (isRunning() && mCurrentScan.mDirPath.compare(path, caseSensitivity) == 0) {
        mExit = true;
        lock.unlock();
        stop();
        // Restart scan thread if scan queue is not empty
        if (!mWorkQueue.isEmpty()) {
            const WorkItem &workItem =  mWorkQueue.first();
            scanDirectory(workItem);
        }
    }
#endif
}

// Stop thread and wait for it to exit
void HbPluginNameCacheThread::stop()
{
#ifdef USE_NAME_CACHE_THREAD
    QMutexLocker lock(mMutex);
    mExit = true;
    lock.unlock();
    const unsigned long time = 5000; // 5 s
    if (!wait(time)) {
        // Thread did not exit, terminate it
        TRACE("Thread didn't exit")
        terminate();
        wait();
    }
#endif
}

// Scan directory for plugins and keys they implement.
void HbPluginNameCacheThread::doDirectoryScan(const WorkItem &workItem)
{
    TRACE_ENTRY_ARGS(workItem.mDirPath)

    // Invalidate cache contents for the path
    mNameCache.insertPath(workItem.mDirPath);

    QString fileNameFilter = mPluginFileNameFilter();

    QDir pluginDir(workItem.mDirPath, fileNameFilter, QDir::NoSort, QDir::Files | QDir::Readable);
    foreach(const QString &fileName, pluginDir.entryList()) {
        if (mExit) {
            break;
        }
        if (workItem.mOptions == HbPluginNameCache::ScanParameters::LimitToSet) {
            // Scan is limited to a set of files
            if (!mLimitSet.contains(fileName, caseSensitivity)) {
                continue;
            }
        }
        const QString absolutePath = pluginDir.absoluteFilePath(fileName);
        HbLockedPluginLoader *loader = new HbLockedPluginLoader(*mMutex, absolutePath);
        QObject *pluginInstance = loader->instance();

        if (pluginInstance) {
            // If plugin type is correct, plugin file name and keys are saved into a cache
            QStringList keys = mGetPluginKeys(pluginInstance);
            if (!keys.isEmpty()) {
                mNameCache.insert(keys, workItem.mDirPath, fileName);
                if (workItem.mOptions == HbPluginNameCache::ScanParameters::AddToLimitSet) {
                    // Add file name to limit set
                    if (!mLimitSet.contains(fileName, caseSensitivity)) {
                        mLimitSet.append(fileName);
                    }
                }
            }
        }
        loader->unload();
        delete loader;
        loader = 0;
    }
    TRACE_EXIT
}

// Plugin scan thread function
void HbPluginNameCacheThread::run()
{
    TRACE_ENTRY

    while(true) {
        mMutex->lock();
        if (mExit || mWorkQueue.isEmpty()) {
            break;
        }
        mCurrentScan = mWorkQueue.takeFirst();
        mMutex->unlock();
        doDirectoryScan(mCurrentScan);
        mCurrentScan.clear();
    }
    mMutex->unlock();
}

// Constructor
HbPluginNameCache::HbPluginNameCache(GetPluginKeys getPluginKeys,
    PluginFileNameFilter pluginFileNameFilter, QObject *parent) :
    QObject(parent)
{
    mThread = new HbPluginNameCacheThread(*this, getPluginKeys,
        pluginFileNameFilter ? pluginFileNameFilter:&HbPluginNameCache::pluginFileNameFilter);
    connect(&mWatcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(directoryChanged(const QString&)));
}

// Destructor
HbPluginNameCache::~HbPluginNameCache()
{
    mThread->stop(); // ask thread to stop and wait
    delete mThread;
    mThread = 0;
}

// Add directory watch path
void HbPluginNameCache::addWatchPath(const ScanParameters &parameters)
{
    TRACE_ENTRY_ARGS(parameters.mDirPath)
    QString dirPath = directoryPath(parameters.mDirPath);
    TRACE(dirPath)
    if (!dirPath.isEmpty()) {
#ifdef MONITOR_INSTALLATION_DIRS
        mWatcher.addPath(dirPath); // start watching
#endif // MONITOR_INSTALLATION_DIRS
        mThread->scanDirectory(ScanParameters(dirPath, parameters.mOptions));
    }
}

// Remove directory watch path
void HbPluginNameCache::removeWatchPath(const QString &path)
{
    TRACE_ENTRY_ARGS(path)
    QString dirPath = directoryPath(path);
    TRACE(dirPath)
    if (!dirPath.isEmpty()) {
#ifdef MONITOR_INSTALLATION_DIRS
        if (mWatcher.directories().contains(dirPath, caseSensitivity)) {
            mWatcher.removePath(dirPath);
#else // MONITOR_INSTALLATION_DIRS
        {
#endif // MONITOR_INSTALLATION_DIRS
            mThread->cancelScan(dirPath);
            removePath(path);
        }
    }
}

// Scan directory directory for plugins
void HbPluginNameCache::scanDirectory(const ScanParameters &parameters)
{
    TRACE_ENTRY_ARGS(parameters.mDirPath)
    QString dirPath = directoryPath(parameters.mDirPath);
    TRACE(dirPath)
    if (!dirPath.isEmpty()) {
        mThread->scanDirectory(ScanParameters(dirPath, parameters.mOptions));
    }
}

// Find a plugin by a key. Returns plugin file path or an empty string. Search may be limited
// to a certain file name.
QString HbPluginNameCache::find(const QString &key, const QString &fileName)
{
    TRACE_ENTRY_ARGS(key)
    QMutexLocker(&mThread->lock());
    const DirItem *resultDir = 0;
    const FileItem *resultFile = 0;
    int count = mCache.count();
    for(int i = 0; i < count; i++) {
        const DirItem &item = mCache.at(i);
        int files = item.mFiles.count();
        for(int j = 0; j < files; j++) {
            const FileItem &fileItem = item.mFiles.at(j);
            // Search limited to a certain file name
            if (!fileName.isEmpty() && compare(fileName, fileItem.mFile)) {
                continue;
            }
            // Allow eclipsing only if file names are the same
            if (resultFile && compare(resultFile->mFile, fileItem.mFile) != 0) {
                continue;
            }
            if (!fileItem.mKeys.contains(key)) {
                continue;
            }
            resultDir = &item;
            resultFile = &fileItem;
        }
    }
    return resultFile ? resultDir->mPath + resultFile->mFile : QString();
}

// Get directory part of a file name
QString HbPluginNameCache::directoryPath(const QString &path)
{
    // Use QString instead of QFileInfo to handle path to ensure that file system is not
    // accessed (performance). Assume path is a directory if it ends with '/', otherwise
    // it's a file.
    QChar slash('/');
    if (path.isEmpty() || path.endsWith(slash)) {
        return path;
    } else {
        int i = path.lastIndexOf(slash);
        return i != -1 ? path.left(i + 1) : QString();
    }
}

// Case sensitive/insensitive compare depending on os
int HbPluginNameCache::compare(const QString &s1, const QString &s2)
{
    return s1.compare(s2, caseSensitivity);
}

// Print cache contents
void HbPluginNameCache::print()
{
    TRACE_ENTRY
    QMutexLocker(&mThread->lock());
    TRACE("Name cache")

    int count = mCache.count();
    for(int i = 0; i < count; i++) {
        const DirItem &current = mCache.at(i);
        TRACE("Dir:" << current.mPath)
        int fileCount = current.mFiles.count();
        for(int j = 0; j < fileCount; j++) {
            const FileItem &fileitem = current.mFiles.at(j);
            TRACE("File:" << fileitem.mFile)
            int keyCount = fileitem.mKeys.count();
            for(int k = 0; k < keyCount; k++) {
                TRACE(fileitem.mKeys.at(k))
            }
        }
    }
}

// Create cache entry for directory path
void HbPluginNameCache::insertPath(const QString &dirPath)
{
    TRACE_ENTRY_ARGS(dirPath)
    QMutexLocker(&mThread->lock());
    DirItem searchItem;
    searchItem.mPath = dirPath;
    int index = mCache.indexOf(searchItem);
    if (index < 0) {
        mCache.append(searchItem);
    } else {
        mCache[index].mFiles.clear();
    }
}

// Update cache with \a keys , \a firPath and \a fileName
void HbPluginNameCache::insert(const QStringList &keys, const QString &dirPath,
    const QString &fileName)
{
    TRACE_ENTRY_ARGS("keys" << keys << "filePath" << dirPath)
    QMutexLocker(&mThread->lock());
    // Find directory path
    DirItem searchItem;
    searchItem.mPath = dirPath;
    int dirIndex = mCache.indexOf(searchItem);
    if (dirIndex >= 0) {
        DirItem &dirItem = mCache[dirIndex];
        FileItem searchItem;
        searchItem.mFile = fileName;
        int fileIndex = dirItem.mFiles.indexOf(searchItem);
        if (fileIndex >= 0) {
            FileItem &fileItem = dirItem.mFiles[fileIndex];
            fileItem.mKeys = keys;
        } else {
            FileItem fileItem;
            fileItem.mFile = fileName;
            fileItem.mKeys = keys;
            dirItem.mFiles.append(fileItem);
        }
    }
}

// Remove all keys with a \a filePath
void HbPluginNameCache::removePath(const QString &filePath)
{
    TRACE_ENTRY_ARGS(filePath)
    QMutexLocker(&mThread->lock());
    DirItem searchItem;
    searchItem.mPath = directoryPath(filePath);
    int index = mCache.indexOf(searchItem);
    if (index >= 0) {
        mCache[index].mFiles.clear();
    }
}

// Generate filter for plugin file names
QString HbPluginNameCache::pluginFileNameFilter()
{
#if defined(Q_OS_SYMBIAN)
    return QString("*.qtplugin");
#elif defined(Q_OS_MAC)
    return QString("*.dylib");
#elif defined(Q_OS_WIN32)
    return QString("*.dll");
#else
    return QString("*.so");
#endif
}

// Watched directory has changed
void HbPluginNameCache::directoryChanged(const QString &path)
{
    TRACE_ENTRY_ARGS(path)
    mThread->scanDirectory(ScanParameters(path));
    TRACE_EXIT
}

// Compare scan parameters
bool HbPluginNameCache::ScanParameters::operator ==(const ScanParameters &other) const
{
    return compare(mDirPath, other.mDirPath) == 0;
}

// Compare cache items
bool HbPluginNameCache::FileItem::operator ==(const FileItem &other) const
{
    return compare(mFile, other.mFile) == 0;
}

// Compare cache items
bool HbPluginNameCache::DirItem::operator ==(const DirItem &other) const
{
    return compare(mPath, other.mPath) == 0;
}

// Constructor
HbLockedPluginLoader::HbLockedPluginLoader(HbPluginNameCache &nameCache, const QString &fileName,
    QObject *parent) : mMutex(nameCache.mThread->lock())
{
    QMutexLocker locker(&mMutex);
    mLoader = new QPluginLoader(fileName, parent);
}


// Constructor
HbLockedPluginLoader::HbLockedPluginLoader(QMutex &mutex, const QString &fileName,
    QObject *parent) : mMutex(mutex)
{
    QMutexLocker locker(&mMutex);
    mLoader = new QPluginLoader(fileName, parent);
}

// Destructor
HbLockedPluginLoader::~HbLockedPluginLoader()
{
    QMutexLocker locker(&mMutex);
    delete mLoader;
}

// Locked instance
// QPluginLoader is not re-entrant if two loaders are accessing the same plugin
QObject *HbLockedPluginLoader::instance()
{
    QMutexLocker locker(&mMutex);
    return mLoader->instance();
}

// Locked load
bool HbLockedPluginLoader::load()
{
    QMutexLocker locker(&mMutex);
    return mLoader->load();
}

// Locked unload
bool HbLockedPluginLoader::unload()
{
    QMutexLocker locker(&mMutex);
    return mLoader->unload();
}
