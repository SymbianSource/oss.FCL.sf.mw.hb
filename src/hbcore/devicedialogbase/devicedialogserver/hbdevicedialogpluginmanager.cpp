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

/*
    \class HbDeviceDialogPluginManager
    \brief HbDeviceDialogPluginManager manages loading and unloading of device dialog plugins

    HbDeviceDialogPluginManager maintains a list of plugins loaded into memory. Plugin's
    reference count is increased by each loadPlugin() or createWidget() call. There are
    corresponding unloadPlugin() and freeWidget() calls which decrease reference count. When
    reference count becomes 0, the plugin is unloaded from memory. User of the class should
    match each loadPlugin() with a corresponding unloadPlugin(). Each widget created
    createWidget() should be freed by freeWidget();

    HbDeviceDialogPluginManager can preload plugins into memory by a preloadPlugins() function.
    Plugins in file system are scanned and those returning preload flag are loaded into memory.
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
    const QStringList pathList = pluginPathList();
    for (int i = 0; i < pathList.length(); i++) {
        updateCachePath(pathList.at(i));
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
    Preloads plugins into memory.
*/
void HbDeviceDialogPluginManager::preloadPlugins()
{
    TRACE_ENTRY
    // Check if preloading is disabled
    if (mFlags & HbDeviceDialogPluginManager::NoPreloadFlag) {
        return;
    }

    QString unused;
    // Scan plugins and load those that request preloading
    scanPlugins(&HbDeviceDialogPluginManager::preloadPluginCallback, unused, false);
    TRACE_EXIT
}

/*
    Creates a device dialog widget. Plugin is loaded into memory if it's not already loaded.
    \a deviceDialogType contains device dialog type. \a parameters contains widget parameters.
    \a error receives an error code if widget couldn't be created. Returns pointer to widget
    or null on error.
*/
HbDeviceDialogInterface *HbDeviceDialogPluginManager::createWidget(const QString &deviceDialogType,
    const QVariantMap &parameters, bool &recycled, int &error)
{
    TRACE_ENTRY
    error = HbDeviceDialogNoError;
    HbDeviceDialogInterface *widgetIf = 0;
    if (loadPlugin(deviceDialogType)) {
        PluginInfo &pluginInfo = mPlugins[findPlugin(deviceDialogType)];
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
                // Add a dynamic property to be able to find plugin from a widget
                widgetIf->deviceDialogWidget()->setProperty(deviceDialogTypePropertyName,
                    QVariant(deviceDialogType));
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
        QObject *obj = widget->deviceDialogWidget();
        QString deviceDialogType = obj->property(deviceDialogTypePropertyName).toString();
        int index = findPlugin(deviceDialogType);
        Q_ASSERT(index >= 0);
        PluginInfo &pluginInfo = mPlugins[index];
        // Get signal sender for the widget
        QObject *sender = widget->signalSender();
        if (!sender) {
            sender = obj;
        }
        if (pluginInfo.mFlags & PluginInfo::RecycleWidget &&
            pluginInfo.mRecycledWidget == 0) {
            pluginInfo.mRecycledWidget = widget;
            sender->disconnect(); // disconnect all signals from receivers
        } else {
            // Delete widget from a timer as deviceDialogClosed() signal may
            // come before device dialog is fully closed.
            sender->disconnect(); // disconnect all signals
            mDeleteWidgets.append(widget);
#if defined(Q_OS_SYMBIAN)
            const int deleteDelay = 2000; // 2s
#else
            const int deleteDelay = 500; // 0.5s
#endif
            mDeleteTimer.start(deleteDelay);
        }
    }
    TRACE_EXIT
}

/*
    Loads a plugin into memory. \a deviceDialogType contains device dialog type of the plugin.
    If plugin is already loaded, only reference count is increased. Returns true on success
    and false on failure.
*/
bool HbDeviceDialogPluginManager::loadPlugin(const QString &deviceDialogType)
{
    TRACE_ENTRY_ARGS(deviceDialogType)
    // If plugin is not loaded, try to load it
    int index = findPlugin(deviceDialogType);
    if (index < 0) {
        // Check if plugin file name is in cache
        bool loaded = false;
        const QString filePath = mNameCache.find(deviceDialogType);
        if (!filePath.isEmpty()) {
            TRACE("cache hit")
            loaded = scanPlugin(&HbDeviceDialogPluginManager::loadPluginCallback, deviceDialogType,
                filePath);
            // If plugin wasn't loaded, the cache has stale information. Rescan the directory.
            if (!loaded) {
                TRACE("cache stale")
                updateCachePath(filePath);
            }
        }
        if (!loaded) {
            TRACE("cache miss")
            // Plugin name wasn't in cache, try to find it
            scanPlugins(&HbDeviceDialogPluginManager::loadPluginCallback, deviceDialogType);
            int i = findPlugin(deviceDialogType);
            if (i >= 0) {
                // Plugin was found, update plugin name cache by scanning the directory
                updateCachePath(mPlugins[i].mLoader->fileName());
            }
        }
    } else {
        // Plugin is already loaded, increase reference count
        mPlugins[index].mRefCount++;
    }
    TRACE_EXIT
    // Return true if plugin is loaded
    return findPlugin(deviceDialogType) >= 0;
}

/*
    Unloads plugin from memory. Each loadPlugin() should be matched by unloadPlugin(). Plugin is
    unloaded from memory if reference count becomes 0. \a deviceDialogType contains device dialog
    type of the plugin. Returns true on success and false on failure.
*/
bool HbDeviceDialogPluginManager::unloadPlugin(const QString &deviceDialogType)
{
    TRACE_ENTRY_ARGS(deviceDialogType)
    bool removed = false;
    int index = findPlugin(deviceDialogType);
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
    const QString &deviceDialogType)
{
    TRACE_ENTRY
    // Plugin has to be loaded when this function is called
    int index = findPlugin(deviceDialogType);
    Q_ASSERT(index >= 0);

    const PluginInfo &pluginInfo = mPlugins[index];
    QObject *pluginInstance = pluginInfo.mLoader->instance();
    TRACE_EXIT
    return *qobject_cast<HbDeviceDialogPlugin*>(pluginInstance);
}

// Scan plugins in file system
void HbDeviceDialogPluginManager::scanPlugins(PluginScanCallback func, const QString &deviceDialogType, bool stopIfFound)
{
    TRACE_ENTRY
    const QStringList pathList = pluginPathList();
    const QString fileNameFilter = pluginFileNameFilter();

    foreach (const QString &path, pathList) {
        QDir pluginDir(path, fileNameFilter, QDir::NoSort, QDir::Files | QDir::Readable);
        foreach (const QString &fileName, pluginDir.entryList()) {
            if (scanPlugin(func, deviceDialogType, pluginDir.absoluteFilePath(fileName)) &&
                stopIfFound) {
                break;
            }
        }
    }
    TRACE_EXIT
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

// Callback for scanPlugins(). Load plugin if it has a preload flag.
HbLockedPluginLoader *HbDeviceDialogPluginManager::preloadPluginCallback(HbLockedPluginLoader *loader,
    const QString &unused)
{
    TRACE_ENTRY
    Q_UNUSED(unused);

    QObject *pluginInstance = loader->instance();
    HbDeviceDialogPlugin *plugin = qobject_cast<HbDeviceDialogPlugin*>(pluginInstance);

    HbDeviceDialogPlugin::PluginFlags flags = plugin->pluginFlags();
    if (flags & HbDeviceDialogPlugin::PreloadPlugin) {
        // Save preloaded plugin into a list
        PluginInfo pluginInfo;
        pluginInfo.mTypes = plugin->deviceDialogTypes();
        pluginInfo.mPluginFlags = flags;
        pluginInfo.mLoader = loader;
        loader = 0;
        pluginInfo.mRefCount++; // this keeps plugin loaded in memory
        mPlugins.append(pluginInfo);
        pluginInfo.mLoader = 0; // ownership was transferred to the list
    }
    TRACE_EXIT
    return loader;
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
        pluginInfo.mTypes = types;
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

// Find index of a plugin
int HbDeviceDialogPluginManager::findPlugin(const QString &deviceDialogType) const
{
    TRACE_ENTRY
    int count = mPlugins.count();
    for(int i = 0; i < count; i++) {
        if (mPlugins.at(i).mTypes.contains(deviceDialogType)) {
            TRACE_EXIT_ARGS(i);
            return i;
        }
    }
    TRACE_EXIT_ARGS(-1);
    return -1;
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
void HbDeviceDialogPluginManager::updateCachePath(const QString &path)
{
    QString dirPath = HbPluginNameCache::directoryPath(path);
    QFileInfo fileInfo(dirPath);
    if (fileInfo.exists()) {
        // If directory is writable, watch it. Otherwise scan it only once.
        if (fileInfo.isWritable()) {
            mNameCache.addWatchPath(dirPath);
        } else {
            mNameCache.scanDirectory(dirPath);
        }
    } else {
        mNameCache.removeWatchPath(dirPath);
    }
}

//Generate directory path list to search plugins
QStringList HbDeviceDialogPluginManager::pluginPathList()
{
    QStringList pluginPathList;

#if defined(Q_OS_SYMBIAN)
    const QString pluginRelativePath("resource/plugins/devicedialogs/");

    QFileInfoList driveInfoList = QDir::drives();

    foreach (const QFileInfo &driveInfo, driveInfoList) {
        const QString drive = driveInfo.absolutePath();
        if (drive.startsWith("z:", Qt::CaseInsensitive) ||
            drive.startsWith("c:", Qt::CaseInsensitive)) {
            QString path(drive + pluginRelativePath);
            pluginPathList << path;
        }
    }
#elif defined(Q_OS_WIN32) || defined(Q_OS_UNIX)
    pluginPathList << qApp->applicationDirPath() + '/' << HB_PLUGINS_DIR"/devicedialogs/";
#endif

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
#if defined(Q_OS_LINUX)
    return QString("*.so");
#elif defined(Q_OS_MAC)
    return QString("*.dylib");
#elif defined(Q_OS_WIN32)
    return QString("*.dll");
#else
    return QString("*.qtplugin");
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
        QString deviceDialogType = widgetIf->deviceDialogWidget()->
            property(deviceDialogTypePropertyName).toString();
        // IN Windows/Linux scene may get deleted before plugin manager and deletes all widgets
        if (!mAllWidgetsDeleted) {
            delete widgetIf;
        }
        int index = findPlugin(deviceDialogType);
        Q_ASSERT(index >= 0);
        PluginInfo &pluginInfo = mPlugins[index];
        pluginInfo.mRefCount--;
        unloadPlugin(deviceDialogType);
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
