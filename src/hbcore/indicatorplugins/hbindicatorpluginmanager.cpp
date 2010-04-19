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

#include <QDir>
#include <QApplication>

#include <hbindicatorpluginmanager_p.h>
#include <hbindicatorplugininterface.h>
#include <hbindicatorinterface.h>
#include <hbdevicedialogtrace_p.h>
#include <hbdevicedialogerrors_p.h>

/*
    \class HbIndicatorPluginManager
    \brief HbIndicatorPluginManager manages loading and unloading of indicator plugins

    HbIndicatorPluginManager maintains a list of indicators which are created from
    indicator plugins
*/

// Constructor
HbIndicatorPluginManager::HbIndicatorPluginManager(QObject *parent) :
    QObject(parent), mNameCache(pluginKeys)
{
    // Get list of plugin directories to watch/scan
    const QStringList pathList = pluginPathList();
    for (int i = 0; i < pathList.length(); i++) {
        updateCachePath(pathList.at(i));
    }
}

// Destructor
HbIndicatorPluginManager::~HbIndicatorPluginManager()
{
}

/*
    Searches for plugin, that implements the indicator of type \a indicatorType.
    Creates and stores the indicator.
    Does nothing, if indicator is already added.
    Returns true on success and false on failure.
*/
HbIndicatorInterface *HbIndicatorPluginManager::addIndicator(const QString &indicatorType,
    const QVariantMap &securityCredentials, int *errorCode)
{
    TRACE_ENTRY
    int error = HbDeviceDialogNoError;
    if (!errorCode) {
        errorCode = &error;
    }
    HbIndicatorInterface *indicator = 0;
    bool loaded = false;
    bool added = false;
    *errorCode = HbDeviceDialogNoError;

    //search indicator 1. among already loaded plugins.
    int index = findPlugin(indicatorType);
    if (index < 0) {
        //2. search and load the correct plugin.
        index = loadPlugin(indicatorType);
        loaded = true;
    }
    // Allow plugin to do access control
    if (index >= 0) {
        if (!checkAccess(index, indicatorType, securityCredentials)) {
            if (loaded) {
                mPlugins.removeAt(index);
            }
            *errorCode = HbDeviceDialogAccessDeniedError;
            index = -1;
        }
    }
    if (index >= 0) {
        PluginInfo &pluginInfo = mPlugins[index];
        if (!loaded) {
            //check is the indicator is already created.
            foreach(const IndicatorInfo &indicatorInfo,
                     pluginInfo.mAddedIndicators) {
                if (indicatorInfo.indicator->indicatorType() ==
                        indicatorType) {
                    indicator = indicatorInfo.indicator;
                    added = true;
                    break;
                }
            }
        }
        if (!added) {
            //create and store the indicator.
            HbIndicatorPluginInterface *plugin = pluginInfo.plugin();
            if (plugin) {
                indicator = plugin->createIndicator(indicatorType);
            }
            if (indicator) {
                pluginInfo.mAddedIndicators.append(indicator);
                added = true;
            } else {
                if (pluginInfo.mAddedIndicators.count() == 0) {
                    mPlugins.removeAt(index);
                }
                *errorCode = plugin->error();
                TRACE("creating indicator: "
                      << indicatorType << " from plugin failed");
            }
        }
    }

    if (!added && error == HbDeviceDialogNoError) {
        *errorCode = HbDeviceDialogUnknownIndicatorError;
    }

    TRACE_EXIT
    return indicator;
}

/*
   Activates the indicator.
   Assumes indicator-instance is already created (addIndicator called).
*/
bool HbIndicatorPluginManager::activateIndicator(const QString &indicatorType,
    const QVariant &parameter, const QVariantMap &securityCredentials)
{
    bool success = false;
    IndicatorInfo *indicatorInfo = 0;
    int index = findPlugin(indicatorType, &indicatorInfo);
    if (index >= 0 && checkAccess(index, indicatorType, securityCredentials)) {
        HbIndicatorInterface *indicator = indicatorInfo->indicator;

        indicator->disconnect(this, SLOT(deactivateIndicator()));
        connect(indicator, SIGNAL(deactivate()), SLOT(deactivateIndicator()), Qt::QueuedConnection);

        indicator->processClientRequest(
                HbIndicatorInterface::RequestActivate, parameter);
        //in case indicator deactivated itself, find the indicator again.
        index = findPlugin(indicatorType, &indicatorInfo);

        if (index >= 0 && !indicatorInfo->activated) {
            indicatorInfo->activated = true;
            emit indicatorActivated(indicator);
            indicatorInfo->statusAreaIconPath = statusAreaIconPath(indicator);
            emit indicatorActivated(IndicatorClientInfo(
                indicator->indicatorType(),
                indicatorInfo->statusAreaIconPath,
                indicator->category()));

            connect(indicator, SIGNAL(dataChanged()), SLOT(indicatorDataChanged()));
        }
        success = true;
    }
    return success;
}

/*
   removes indicator from memory.
   Returns true, if indicator was removed, false otherwise.
*/
bool HbIndicatorPluginManager::removeIndicator(const QString &indicatorType)
{
    TRACE_ENTRY
    bool removed = false;
    int infoIndex = 0;
    IndicatorInfo *indicatorInfo = 0;

    int index = findPlugin(indicatorType, &indicatorInfo, &infoIndex);
    if (index >= 0) {
        PluginInfo &pluginInfo = mPlugins[index];
        HbIndicatorInterface *indicator = indicatorInfo->indicator;
        QString statusAreaPath(indicatorInfo->statusAreaIconPath);

        pluginInfo.mAddedIndicators.remove(infoIndex);
        indicatorInfo = 0; //destroyed
        emit indicatorRemoved(indicator);

        emit indicatorRemoved(IndicatorClientInfo(
            indicator->indicatorType(),
            statusAreaPath,
            indicator->category()));

        //plugin-interface may also be HbIndicatorInterface.
        //in that case, don't delete.
        HbIndicatorInterface *testPlugin =
                dynamic_cast<HbIndicatorInterface*>(pluginInfo.plugin());
        if ( testPlugin != indicator) {
            //indicator deletion must occur before idle-timer-cleanup.
            delete indicator;
            indicator = 0;
        }

        // delete plugin interface
        int count = mPlugins.count();
        for(int i = count-1; i >= 0; --i) {
            PluginInfo &pluginInfo = mPlugins[i];
            if (pluginInfo.mAddedIndicators.count() == 0) {
               mPlugins.removeAt(i);               
            }
        }
        removed = true;
    }
    TRACE_EXIT
    return removed;
}

//returns the indicator information needed in client side.
QList<IndicatorClientInfo>
        HbIndicatorPluginManager::indicatorClientInfoList() const
{
    QList<IndicatorClientInfo> clientInfoList;

    //add icons from all the activated indicators of group priority 1 and 2.
    foreach (const PluginInfo &pluginInfo, mPlugins) {
        foreach(const IndicatorInfo &indicatorInfo,
                pluginInfo.mAddedIndicators) {

            HbIndicatorInterface *indicator = indicatorInfo.indicator;
            HbIndicatorInterface::Category category = indicator->category();

            QString path(indicatorInfo.statusAreaIconPath);

            if (!path.isEmpty()) {
                IndicatorClientInfo clientInfo(
                        indicator->indicatorType(), path, category);
                clientInfoList.append(clientInfo);
            }
        }
    }
    return clientInfoList;
}

QList<HbIndicatorInterface*>
    HbIndicatorPluginManager::indicators() const
{
    QList<HbIndicatorInterface*> indicatorList;
    foreach (const PluginInfo &pluginInfo, mPlugins) {
        foreach(const IndicatorInfo &indicatorInfo,
                pluginInfo.mAddedIndicators) {
            if (indicatorInfo.activated) {
                indicatorList.append(indicatorInfo.indicator);
            }
        }
    }
    return indicatorList;
}

void HbIndicatorPluginManager::connectTo(QObject *widget)
{
    widget->disconnect(this);

   //connect pluginmanager signals to indicatormenu slots, so that
   //popup will get indicator updates.
    widget->connect(this,
                   SIGNAL(indicatorActivated(HbIndicatorInterface*)),
                   SLOT(indicatorActivated(HbIndicatorInterface*)));
    widget->connect(this,
                   SIGNAL(indicatorRemoved(HbIndicatorInterface*)),
                   SLOT(indicatorRemoved(HbIndicatorInterface*)));
    QList<HbIndicatorInterface*> addedIndicators = indicators();

    if (addedIndicators.count() > 0) {
        //initialize popup with added indicators.
        QMetaObject::invokeMethod(widget, "indicatorsActivated",
            Qt::DirectConnection,
            Q_ARG(QList<HbIndicatorInterface*>, addedIndicators));
    }
}

bool HbIndicatorPluginManager::deactivateIndicator(const QString &indicatorType,
    const QVariant &parameter, const QVariantMap &securityCredentials)
{
    TRACE_ENTRY
    bool success = false;
    IndicatorInfo *info = 0;
    int index = findPlugin(indicatorType, &info);
    if (index >= 0 && checkAccess(index, indicatorType, securityCredentials)) {
        HbIndicatorInterface *indicator = info->indicator;
        if (indicator->indicatorType() == indicatorType && info->activated) {
            indicator->processClientRequest(
                    HbIndicatorInterface::RequestDeactivate, parameter);
        }
        success = true;
    }
    TRACE_EXIT
    return success;
}

void HbIndicatorPluginManager::deactivateIndicator()
{
    HbIndicatorInterface* indicator =
            qobject_cast<HbIndicatorInterface*>(sender());
    if (indicator) {
        indicator->disconnect(this);
        removeIndicator(indicator->indicatorType());
    }
}

void HbIndicatorPluginManager::indicatorDataChanged()
{
     const HbIndicatorInterface* indicator =
            qobject_cast<const HbIndicatorInterface*>(sender());
    if (!indicator) {
        return;
    }

    IndicatorInfo *indicatorInfo = 0;
    int index = findPlugin(indicator->indicatorType(), &indicatorInfo);
    if (index >= 0) {
        indicator = indicatorInfo->indicator;
        QString statusAreaPath(statusAreaIconPath(indicator));

        if (indicatorInfo->statusAreaIconPath != statusAreaPath) {
            indicatorInfo->statusAreaIconPath = statusAreaPath;

            emit indicatorUpdated(IndicatorClientInfo(
                indicator->indicatorType(),
                indicatorInfo->statusAreaIconPath,
                indicator->category()));
        }
    }
}

bool HbIndicatorPluginManager::checkAccess(int index, const QString &indicatorType,
    const QVariantMap &securityCredentials)
{
    return mPlugins.at(index).plugin()->accessAllowed(indicatorType, securityCredentials);
}

// Scan plugins in file system
int HbIndicatorPluginManager::loadPlugin(const QString &indicatorType)
{
    TRACE_ENTRY

    // Check if plugin file name is in cache
    int index = -1;
    const QString filePath = mNameCache.find(indicatorType);
    if (!filePath.isEmpty()) {
        TRACE("cache hit")
        index = loadPlugin(indicatorType, filePath);
        // If plugin wasn't loaded, the cache has stale information. Rescan the directory.
        if (index < 0) {
            TRACE("cache stale")
            updateCachePath(filePath);
        }
    }
    if (index < 0) {
        TRACE("cache miss")
        // Plugin name wasn't in cache, try to find it
        index = scanPlugins(indicatorType);
        if (index >= 0) {
            // Plugin was found, update plugin name cache by scanning the directory
            updateCachePath(mPlugins[index].mLoader->fileName());
        }
    }
    TRACE_EXIT
    return index;
}

// Scan for and load plugin in file system
int HbIndicatorPluginManager::scanPlugins(const QString &indicatorType)
{
    TRACE_ENTRY
    const QStringList pathList = pluginPathList();
    const QString fileNameFilter = pluginFileNameFilter();
    int index = -1;

    foreach (const QString &path, pathList) {
        QDir pluginDir(path, fileNameFilter, QDir::NoSort, QDir::Files | QDir::Readable);
        foreach (const QString &fileName, pluginDir.entryList()) {
            index = loadPlugin(indicatorType, pluginDir.absoluteFilePath(fileName));
            if (index >= 0) {
                break;
            }
        }
    }
    TRACE_EXIT
    return index;
}

// Load plugin from file path name
int HbIndicatorPluginManager::loadPlugin(const QString &indicatorType, const QString &filePath)
{
    TRACE_ENTRY
    int index = -1;

    HbLockedPluginLoader *loader = new HbLockedPluginLoader(mNameCache, filePath);
    QObject *pluginInstance = loader->instance();

    if (pluginInstance) {
        HbIndicatorPluginInterface *plugin =
            qobject_cast<HbIndicatorPluginInterface*>(pluginInstance);
        if (plugin) {
            QStringList types = plugin->indicatorTypes();
            if (types.contains(indicatorType)) {
                // Save plugin into a list
                PluginInfo pluginInfo;
                pluginInfo.mTypes = types;
                pluginInfo.mLoader = loader;
                loader = 0;
                mPlugins.append(pluginInfo);
                pluginInfo.mLoader = 0; // ownership was transferred to the list
                index = mPlugins.count() - 1;
            }
        }
    }
    if (loader) {
        loader->unload();
        delete loader;
        loader = 0;
    }
    TRACE_EXIT_ARGS(index)
    return index;
}

// Find index of a plugin
int HbIndicatorPluginManager::findPlugin(
        const QString &indicatorType) const
{
    TRACE_ENTRY
    int count = mPlugins.count();
    for(int i = 0; i < count; i++) {
        if (mPlugins.at(i).mTypes.contains(indicatorType)) {
            TRACE_EXIT
            return i;
        }
    }
    TRACE_EXIT
    return -1;
}

int HbIndicatorPluginManager::findPlugin(const QString &indicatorType,
    IndicatorInfo **indicatorInfo, int *infoIndex)
{
    int count = mPlugins.count();
    for(int i = 0; i < count; i++) {
        PluginInfo &pluginInfo = mPlugins[i];
        const int indicatorCount = pluginInfo.mAddedIndicators.count();
        for(int j = 0; j < indicatorCount; ++j) {
            IndicatorInfo *info = &pluginInfo.mAddedIndicators[j];
            if (info->indicator->indicatorType() == indicatorType) {
                *indicatorInfo = info;
                if (infoIndex) {
                    *infoIndex = j;
                }
                return i;
            }
        }
    }
    return -1;
}

QString HbIndicatorPluginManager::statusAreaIconPath(
        const HbIndicatorInterface *indicator) const
{
    //Use MonoDecorationNameRole-role first, if empty,
    //try with DecorationRole.
    QString path(indicator->indicatorData(
        HbIndicatorInterface::MonoDecorationNameRole).toString());

    if (path.isEmpty()) {
        path = indicator->indicatorData(
            HbIndicatorInterface::DecorationNameRole).toString();
    }

    return path;
}

// Update plugin name cache watch/scan list
void HbIndicatorPluginManager::updateCachePath(const QString &path)
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

// Generate directory path list to search plugins
QStringList HbIndicatorPluginManager::pluginPathList()
{
    QStringList pluginPathList;

#if defined(Q_OS_SYMBIAN)
    const QString pluginRelativePath("resource/plugins/indicators/");

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
    pluginPathList << qApp->applicationDirPath() + '/' << HB_PLUGINS_DIR"/indicators/";
#endif

    // Plugin name caching differentiates directory and file names by trailing slash in a name
    for (int i = 0; i < pluginPathList.length(); i++) {
        Q_ASSERT(pluginPathList.at(i).endsWith('/'));
    }

    return pluginPathList;
}

// Generate plugin file name filter
QString HbIndicatorPluginManager::pluginFileNameFilter()
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

// Return keys (indicator types) the plugin implements
QStringList HbIndicatorPluginManager::pluginKeys(QObject *pluginInstance)
{
    HbIndicatorPluginInterface *plugin =
        qobject_cast<HbIndicatorPluginInterface*>(pluginInstance);
    return plugin ? plugin->indicatorTypes() : QStringList();
}
