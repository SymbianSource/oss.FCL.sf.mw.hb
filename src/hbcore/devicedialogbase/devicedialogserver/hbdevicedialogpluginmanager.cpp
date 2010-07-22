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

#include "hbdevicedialogpluginmanager_p.h"

#include <hbpopup.h>
#include <hbdevicedialogplugin.h>
#include <hbdevicedialog.h>
#include <hbdevicedialoginterface.h>
#include <hbdevicedialogerrors_p.h>
#include <hbdevicedialogtrace_p.h>

#include <QDir>
#include <QApplication>

#if defined(Q_OS_SYMBIAN)
#include <coemain.h>
#include <f32file.h>
#endif // Q_OS_SYMBIAN
/*
    \class HbDeviceDialogPluginManager
    \brief HbDeviceDialogPluginManager manages loading and unloading of device dialog plugins

    HbDeviceDialogPluginManager maintains a list of plugins loaded into memory. Plugin's
    reference count is increased by each loadPlugin() or createWidget() call. There are
    corresponding unloadPlugin() and freeWidget() calls which decrease reference count. When
    reference count becomes 0, the plugin is unloaded from memory. User of the class should
    match each loadPlugin() with a corresponding unloadPlugin(). Each widget created
    createWidget() should be freed by freeWidget();

    Plugins may also specify a keep loaded flag. These are kept loaded in memory after they
    have been loaded first time.
*/

// Created widget gets a property added containing the device dialog type.
static const char deviceDialogTypePropertyName[] = "HbDevDlgPlugManXX";

// Constructor
HbDeviceDialogPluginManager::HbDeviceDialogPluginManager(Flags flags, QObject *parent) :
    QObject(parent), mNameCache(pluginKeys)
{
    TRACE_ENTRY
    mFlags = flags;
    mAllWidgetsDeleted = false;
    mDeleteTimer.setSingleShot(true);
    connect(&mDeleteTimer, SIGNAL(timeout()), this, SLOT(deleteWidgets()));

    // Get list of plugin directories to watch/scan
    int readOnlyPaths;
    mPluginPathList = pluginPathList("/devicedialogs/", readOnlyPaths);

    // Scan only read-only drives + allow eclipsing at startup to ensure installed plugins cannot
    // affect device boot
    for(int i = 0; i < mPluginPathList.count(); i++) {
        updateCachePath(mPluginPathList.at(i), true);
    }
    TRACE_EXIT
}

// Destructor
HbDeviceDialogPluginManager::~HbDeviceDialogPluginManager()
{
    TRACE_ENTRY
    freeRecycleWidgets();
    deleteWidgets();
    TRACE_EXIT
}

/*
    Creates a device dialog widget. Plugin is loaded into memory if it's not already loaded.
    \a deviceDialogType contains device dialog type. \a parameters contains widget parameters.
    \a baseFileName limits search for certain file name. \a pluginFilePath contains a plugin
    file path, if empty a search is performed. \a error receives an error code if
    widget couldn't be created. Returns pointer to widget or null on error.
*/
HbDeviceDialogInterface *HbDeviceDialogPluginManager::createWidget(const QString &deviceDialogType,
    const QVariantMap &parameters, const QString &baseFileName, const QString &pluginFilePath,
    bool &recycled, int &error)
{
    TRACE_ENTRY
    error = HbDeviceDialogNoError;
    HbDeviceDialogInterface *widgetIf = 0;
    QString filePath;
    bool loaded;
    if (!pluginFilePath.isEmpty()) {
        incPluginRefCount(pluginFilePath);
        filePath = pluginFilePath;
        loaded = true;
    } else {
        loaded = loadPlugin(deviceDialogType, baseFileName, &filePath);
    }
    if (loaded) {
        PluginInfo &pluginInfo = mPlugins[findPlugin(filePath)];
        // Check if widget reuse is requested
        if (recycled) {
            pluginInfo.mFlags |= PluginInfo::RecycleWidget; // freeWidget() will keep the widget
            // Check if we have a widget to show again
            if (pluginInfo.mRecycledWidget) {
                widgetIf = pluginInfo.mRecycledWidget;
                pluginInfo.mRecycledWidget = 0;
                widgetIf->setDeviceDialogParameters(parameters);
                // Decrease plugin reference count increased by loadPlugin()
                unloadPlugin(deviceDialogType);
                return widgetIf;
            }
        }
        recycled = false; // widget wasn't recycled
        // Create widget
        QObject *pluginInstance = pluginInfo.mLoader->instance();
        if (pluginInstance) {
            HbDeviceDialogPluginInterface *pluginIf =
                qobject_cast<HbDeviceDialogPluginInterface*>(pluginInstance);
            widgetIf = pluginIf->createDeviceDialog(deviceDialogType, parameters);
            if (widgetIf) {
                pluginInfo.mWidgets.append(widgetIf);
                pluginInfo.mRefCount++;
            } else {
                error = static_cast<HbDeviceDialogPlugin *>(pluginInstance)->error();
            }
        }
        if (!widgetIf) {
            // Widget could not be created. Unload plugin.
            unloadPlugin(deviceDialogType);
            // Ensure an error code is returned even if plugin didn't return an error code
            if (error == HbDeviceDialogNoError) {
                error = HbDeviceDialogParameterError;
            }
        }
    } else {
        error = HbDeviceDialogNotFoundError;
    }
    TRACE_EXIT
    return widgetIf;
}

/*
    Frees a device dialog widget. Widgets created by createWidget() have to freed using this
    function. Plugin is unloaded from memory if reference count becomes 0. \a widget contains
    widget to free.
*/
void HbDeviceDialogPluginManager::freeWidget(HbDeviceDialogInterface *widget)
{
    TRACE_ENTRY
    if (widget) {
        // Check if widget should be reused
        int index = findPlugin(widget);
        Q_ASSERT(index >= 0);
        PluginInfo &pluginInfo = mPlugins[index];
        if (pluginInfo.mFlags & PluginInfo::RecycleWidget &&
            pluginInfo.mRecycledWidget == 0) {
            pluginInfo.mRecycledWidget = widget;
        } else {
            // Delete widget from a timer
            mDeleteWidgets.append(widget);
            // Delete immediately as widget should be ready to be deleted (close effect ended) when
            // devicedialogClosed() signal was emitted
            const int deleteDelay = 0;
            mDeleteTimer.start(deleteDelay);
        }
    }
    TRACE_EXIT
}

/*
    Loads a plugin into memory. \a deviceDialogType contains device dialog type of the plugin.
    \a baseFileName limits the search to a certain file name. \a pluginFilePath contains plugin
    file path on return. If plugin is already loaded, only reference count is increased.
    Returns true on success and false on failure.
*/
bool HbDeviceDialogPluginManager::loadPlugin(const QString &deviceDialogType, const QString &baseFileName,
    QString *pluginFilePath)
{
    TRACE_ENTRY_ARGS(deviceDialogType)
    QString baseNameWithExt(baseFileName);
    if (!baseNameWithExt.isEmpty()) {
        // Add extension to file name
        baseNameWithExt.append(fileNameExtension());
    }
    // Search name cache
    bool loaded = false;
    QString filePath = mNameCache.find(deviceDialogType, baseNameWithExt);
    if (!filePath.isEmpty()) {
        TRACE("cache hit")
        int index = findPlugin(filePath);
        if (index >= 0) {
            loaded = true;
            // Plugin is already loaded, increase reference count
            mPlugins[index].mRefCount++;
        } else {
            loaded = scanPlugin(&HbDeviceDialogPluginManager::loadPluginCallback, deviceDialogType,
                filePath);
        }
        // If plugin wasn't loaded, the cache has stale information. Rescan the directory.
        if (!loaded) {
            TRACE("cache stale")
            updateCachePath(filePath);
        }
    }
    if (!loaded) {
        TRACE("cache miss")
        // Plugin name wasn't in cache, try to find it
        filePath = scanPlugins(deviceDialogType, baseNameWithExt);
        if (!filePath.isEmpty()) {
            int index = findPlugin(filePath);
            if (index >= 0) {
                loaded = true;
                // Plugin is already loaded, increase reference count
                mPlugins[index].mRefCount++;
            } else {
                loaded = scanPlugin(&HbDeviceDialogPluginManager::loadPluginCallback, deviceDialogType,
                    filePath);
            }
        }
        if (loaded) {
            // Plugin was found, update plugin name cache by scanning the directory
            updateCachePath(filePath);
        }
    }
    TRACE("loaded" << loaded)

    if (loaded) {
        *pluginFilePath = filePath;
    }
    TRACE_EXIT
    return loaded;
}

/*
    Unloads plugin from memory. Each loadPlugin() should be matched by unloadPlugin(). Plugin is
    unloaded from memory if reference count becomes 0. \a pluginFilePath contains plugin file name
    and path. Returns true on success and false on failure.
*/
bool HbDeviceDialogPluginManager::unloadPlugin(const QString &pluginFilePath)
{
    TRACE_ENTRY_ARGS(pluginFilePath)
    bool removed = false;
    int index = findPlugin(pluginFilePath);
    if (index >= 0) {
        PluginInfo &pluginInfo = mPlugins[index];
        if (--pluginInfo.mRefCount == 0) {
            mPlugins.removeAt(index);
            removed = true;
        }
    }
    TRACE_EXIT
    return removed;
}

/*
    Returns pointer to a plugin. Assumes plugin has been loaded. \a deviceDialogType contains
    deviceDialog type of the plugin.
*/
const HbDeviceDialogPlugin &HbDeviceDialogPluginManager::plugin(
    const QString &pluginFilePath)
{
    TRACE_ENTRY
    // Plugin has to be loaded when this function is called
    int index = findPlugin(pluginFilePath);
    Q_ASSERT(index >= 0);

    const PluginInfo &pluginInfo = mPlugins[index];
    QObject *pluginInstance = pluginInfo.mLoader->instance();
    TRACE_EXIT
    return *qobject_cast<HbDeviceDialogPlugin*>(pluginInstance);
}

// Scan plugins in file system. Returns plugin file path.
QString HbDeviceDialogPluginManager::scanPlugins(const QString &deviceDialogType,
    const QString &baseFileName)
{
    TRACE_ENTRY
    const QString fileNameFilter = pluginFileNameFilter();
    QString pluginFileName(baseFileName);
    if (!pluginFileName.isEmpty()) {
        // Add extension to file name
        pluginFileName.append(fileNameExtension());
    }

    QString result;
    foreach(const QString &path, mPluginPathList) {
        QDir pluginDir(path, fileNameFilter, QDir::NoSort, QDir::Files | QDir::Readable);
        foreach(const QString &fileName, pluginDir.entryList()) {
            if (pluginFileName.isEmpty() || HbPluginNameCache::compare(pluginFileName, fileName) == 0) {
                const QString current(pluginDir.absoluteFilePath(fileName));
                if (scanPlugin(&HbDeviceDialogPluginManager::scanPluginCallback, deviceDialogType,
                    current)) {
                    result = current;
                    if (pluginFileName.isEmpty()) {
                        pluginFileName = fileName;
                    }
                }
            }
        }
    }
    TRACE_EXIT
    return result;
}

// Scan a plugin. Return true if plugin was loaded.
bool HbDeviceDialogPluginManager::scanPlugin(PluginScanCallback func,
    const QString &deviceDialogType, const QString &filePath)
{
    TRACE_ENTRY_ARGS(filePath)
    HbLockedPluginLoader *loader = new HbLockedPluginLoader(mNameCache, filePath);
    QObject *pluginInstance = loader->instance();

    if (pluginInstance) {
        HbDeviceDialogPlugin *plugin =
            qobject_cast<HbDeviceDialogPlugin*>(pluginInstance);
        // Device dialog plugin found. Call function handle it.
        if (plugin) {
            loader = (this->*func)(loader, deviceDialogType);
        }
    }
    bool loaded = !loader;
    if (loader) {
        loader->unload();
        delete loader;
        loader = 0;
    }
    return loaded;
}

// Callback for scanPlugins(). Load plugin for device dialog type.
HbLockedPluginLoader *HbDeviceDialogPluginManager::loadPluginCallback(HbLockedPluginLoader *loader,
    const QString &deviceDialogType)
{
    TRACE_ENTRY
    QObject *pluginInstance = loader->instance();
    HbDeviceDialogPlugin *plugin = qobject_cast<HbDeviceDialogPlugin*>(pluginInstance);

    QStringList types = plugin->deviceDialogTypes();
    if (types.contains(deviceDialogType)) {
        // Save plugin into a list
        PluginInfo pluginInfo;
        pluginInfo.mPluginFlags = plugin->pluginFlags();
        pluginInfo.mLoader = loader;
        loader = 0;
        pluginInfo.mRefCount++;
        bool keepLoaded = (pluginInfo.mPluginFlags & HbDeviceDialogPlugin::KeepPluginLoaded) &&
            (mFlags & HbDeviceDialogPluginManager::NoKeepLoadedFlag) == 0;
        pluginInfo.mRefCount += keepLoaded;
        // Reference count 2 keeps plugin loaded in memory
        pluginInfo.mRefCount =
            pluginInfo.mPluginFlags & HbDeviceDialogPlugin::KeepPluginLoaded ? 2 : 1;
        mPlugins.append(pluginInfo);
        pluginInfo.mLoader = 0; // ownership was transferred to the list
    }
    TRACE_EXIT
    return loader;
}

// Callback for scanPlugins(). Check whether plugin implements device dialog type.
HbLockedPluginLoader *HbDeviceDialogPluginManager::scanPluginCallback(HbLockedPluginLoader *loader,
    const QString &deviceDialogType)
{
    TRACE_ENTRY
    QObject *pluginInstance = loader->instance();
    HbDeviceDialogPlugin *plugin = qobject_cast<HbDeviceDialogPlugin*>(pluginInstance);

    QStringList types = plugin->deviceDialogTypes();
    if (types.contains(deviceDialogType)) {
        loader->unload();
        delete loader;
        loader = 0;
    }
    TRACE_EXIT
    return loader;
}

// Find index of a plugin
int HbDeviceDialogPluginManager::findPlugin(const QString &pluginFilePath) const
{
    TRACE_ENTRY
    int count = mPlugins.count();
    for(int i = 0; i < count; i++) {
        if (HbPluginNameCache::compare(mPlugins.at(i).mLoader->fileName(), pluginFilePath) == 0) {
            TRACE_EXIT_ARGS(i);
            return i;
        }
    }
    TRACE_EXIT_ARGS(-1);
    return -1;
}

// Find index of a plugin
int HbDeviceDialogPluginManager::findPlugin(HbDeviceDialogInterface* widget) const
{
    TRACE_ENTRY
    int count = mPlugins.count();
    for(int i = 0; i < count; i++) {
        if (mPlugins.at(i).mWidgets.contains(widget)) {
            TRACE_EXIT_ARGS(i);
            return i;
        }
    }
    TRACE_EXIT_ARGS(-1);
    return -1;
}

// Increase plugin reference count
void HbDeviceDialogPluginManager::incPluginRefCount(const QString &pluginFilePath)
{
    int index = findPlugin(pluginFilePath);
    Q_ASSERT(index >= 0);
    mPlugins[index].mRefCount++;
}

// Free widgets that are kept for re-use. Widget reuse is a performance optimization
// to get widgets to appear faster.
void HbDeviceDialogPluginManager::freeRecycleWidgets()
{
    QList<PluginInfo>::iterator i = mPlugins.begin();
    while (i != mPlugins.end()) {
        PluginInfo &pluginInfo = *i;
        if (pluginInfo.mRecycledWidget) {
            mDeleteWidgets.append(pluginInfo.mRecycledWidget);
            pluginInfo.mRecycledWidget = 0;
        }
        ++i;
    }
}

// Update plugin name cache watch/scan list
void HbDeviceDialogPluginManager::updateCachePath(const QString &path, bool firstScan)
{
    QString dirPath = HbPluginNameCache::directoryPath(path);
    QFileInfo fileInfo(dirPath);
    if (fileInfo.exists()) {
        // If directory is writable, watch it. Otherwise scan it only once.
        if (fileInfo.isWritable()) {
            HbPluginNameCache::ScanParameters::Options scanOptions =
                HbPluginNameCache::ScanParameters::NoOptions;
            Q_UNUSED(firstScan)
#if defined(Q_OS_SYMBIAN)
            if (firstScan) {
                scanOptions = HbPluginNameCache::ScanParameters::LimitToSet;
            }
#endif // defined(Q_OS_SYMBIAN)
            mNameCache.addWatchPath(HbPluginNameCache::ScanParameters(dirPath, scanOptions));
        } else {
            if (firstScan) {
                HbPluginNameCache::ScanParameters parameters(path, HbPluginNameCache::ScanParameters::AddToLimitSet);
                mNameCache.scanDirectory(parameters);
            }
        }
    } else {
        mNameCache.removeWatchPath(dirPath);
    }
}

//Generate directory path list to search plugins
QStringList HbDeviceDialogPluginManager::pluginPathList(const QString &subDir, int &readOnlyPaths)
{
    QStringList pluginPathList;

#if defined(Q_OS_SYMBIAN)
    QString pluginRelativePath("resource/plugins");
    pluginRelativePath.append(subDir);

    QFileInfoList driveInfoList = QDir::drives();

    foreach(const QFileInfo &driveInfo, driveInfoList) {
        const QString drive = driveInfo.absolutePath();
        QString path(drive + pluginRelativePath);
        pluginPathList << path;
    }
#elif defined(Q_OS_WIN32) || defined(Q_OS_UNIX)
    pluginPathList <<  HB_PLUGINS_DIR + subDir << qApp->applicationDirPath() + '/';
#endif
    readOnlyPaths = trimPluginPathList(pluginPathList);
    // Plugin name caching differentiates directory and file names by trailing slash in a name
    for (int i = 0; i < pluginPathList.length(); i++) {
        Q_ASSERT(pluginPathList.at(i).endsWith('/'));
    }

    TRACE_EXIT
    return pluginPathList;
}

// Generate plugin file name filter
QString HbDeviceDialogPluginManager::pluginFileNameFilter()
{
    QString filter("*");
    filter.append(fileNameExtension());
    return filter;
}

// Generate file name extension
QString HbDeviceDialogPluginManager::fileNameExtension()
{
#if defined(Q_OS_SYMBIAN)
    return QString(".qtplugin");
#elif defined(Q_OS_MAC)
    return QString(".dylib");
#elif defined(Q_OS_WIN32)
    return QString(".dll");
#else
    return QString(".so");
#endif
}

// Return keys (device dialog types) the plugin implements
QStringList HbDeviceDialogPluginManager::pluginKeys(QObject *pluginInstance)
{
    HbDeviceDialogPlugin *plugin = qobject_cast<HbDeviceDialogPlugin*>(pluginInstance);
    return plugin ? plugin->deviceDialogTypes() : QStringList();
}

// Delete plugin widgets in a delete list. Widget delete is delayed to allow time for
// the widget implementation to finish execution before it is deleted.
void HbDeviceDialogPluginManager::deleteWidgets()
{
    TRACE_ENTRY
    QList<HbDeviceDialogInterface*>::iterator i = mDeleteWidgets.begin();
    while (i != mDeleteWidgets.end()) {
        HbDeviceDialogInterface *&widgetIf = *i;
        int index = findPlugin(widgetIf);
        // IN Windows/Linux scene may get deleted before plugin manager and deletes all widgets
        if (!mAllWidgetsDeleted) {
            delete widgetIf;
        }
        Q_ASSERT(index >= 0);
        PluginInfo &pluginInfo = mPlugins[index];
        pluginInfo.mRefCount--;
        pluginInfo.mWidgets.removeAt(pluginInfo.mWidgets.indexOf(widgetIf));
        unloadPlugin(pluginInfo.mLoader->fileName());
        ++i;
    }
    mDeleteWidgets.clear();
    TRACE_EXIT
}

// Widgets have already been deleted by scene
void HbDeviceDialogPluginManager::allWidgetsDeleted()
{
    mAllWidgetsDeleted = true;
}

// Trim plugin search path list from all possible drives. Returns number of read only paths
// at head of path list.
int HbDeviceDialogPluginManager::trimPluginPathList(QStringList &pathList)
{
#if defined(Q_OS_SYMBIAN)
    // Get list of disk drives
    TDriveList driveList;
    RFs &fs = CCoeEnv::Static()->FsSession();
    int error = fs.DriveList(driveList);
    Q_ASSERT(error == KErrNone);

    // Put pathlist into search order. Rom-drives first, then internal and local removable drives.
    // Remote drives are not supported for plugins.
    int numRomDrives = 0;
    int numInternalDrives = 0;
    int numRemovableDrives = 0;
    int count = pathList.count();
    for(int i = 0; i < count; i++) {
        QChar c(pathList.at(i).at(0).toLower());
        int driveIndex = c.toAscii() - 'a';
        TDriveInfo driveInfo;
        error = fs.Drive(driveInfo, EDriveA + driveIndex);
        Q_ASSERT(error == KErrNone);
        if (driveInfo.iDriveAtt == (KDriveAttInternal|KDriveAttRom)) {
            if (i != numRomDrives) {
              const QString tmp(pathList.at(i));
              pathList.removeAt(i);
              pathList.insert(numRomDrives, tmp);
            }
            numRomDrives++;
        } else if (driveInfo.iDriveAtt == (KDriveAttInternal|KDriveAttLocal)) {
              int newPlace = numRomDrives + numInternalDrives;
              if (i != newPlace) {
                  const QString tmp(pathList.at(i));
                  pathList.removeAt(i);
                  pathList.insert(newPlace, tmp);
              }
              numInternalDrives++;
        } else if (driveInfo.iDriveAtt & (KDriveAttInternal|KDriveAttRemovable)) {
            int newPlace = numRomDrives + numInternalDrives + numRemovableDrives;
            if (i != newPlace) {
                const QString tmp(pathList.at(i));
                pathList.removeAt(i);
                pathList.insert(newPlace, tmp);
            }
            numRemovableDrives++;
        }
    }
    // Cut other drives off the list
    for(int i = numRomDrives + numInternalDrives + numRemovableDrives; i < count; i++) {
        pathList.removeLast();
    }

    // Ensure alphanumeric order
    if (numRomDrives > 1) {
        QStringList::iterator begin = pathList.begin();
        qSort(begin, begin + numRomDrives);
    }
    if (numInternalDrives > 1) {
        QStringList::iterator begin = pathList.begin() + numRomDrives;
        qSort(begin, begin + numInternalDrives);
    }
    if (numRemovableDrives > 1) {
        QStringList::iterator begin = pathList.begin() + numRomDrives + numInternalDrives;
        qSort(begin, begin + numRemovableDrives);
    }

    return numRomDrives;
#else // Q_OS_SYMBIAN
    Q_UNUSED(pathList)
    return 0;
#endif // Q_OS_SYMBIAN
}
