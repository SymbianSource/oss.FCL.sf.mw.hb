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

#ifndef HBDEVICEDIALOGPLUGINMANAGER_P_H
#define HBDEVICEDIALOGPLUGINMANAGER_P_H

#include <QObject>
#include <QVariantMap>
#include <QList>
#include <QStringList>
#include <QTimer>

#include <hbdevicedialogplugin.h>
#include <hbdevicedialog.h>
#include <hbpluginnamecache_p.h>

#include <hbglobal.h>

class HbWidget;
class HbDeviceDialogPluginManagerPrivate;
class HbDeviceDialogPlugin;
class HbDeviceDialogInterface;

class HbDeviceDialogPluginManager : public QObject
{
    Q_OBJECT

public:
    friend class HbDeviceDialogPluginManagerPrivate;

    enum Flags {
        NoFlags = 0,
        NoPreloadFlag = 1, // no plugins are preloaded
        NoKeepLoadedFlag = 2 // no plugins are kept loaded
    };

    explicit HbDeviceDialogPluginManager(Flags flags = NoFlags, QObject *parent = 0);
    virtual ~HbDeviceDialogPluginManager();

    void preloadPlugins();
    HbDeviceDialogInterface *createWidget(const QString &deviceDialogType,
        const QVariantMap &parameters, bool &recycled, int &error);
    void freeWidget(HbDeviceDialogInterface *widget);
    bool loadPlugin(const QString &deviceDialogType);
    bool unloadPlugin(const QString &deviceDialogType);
    const HbDeviceDialogPlugin &plugin(const QString &deviceDialogType);

    static QStringList pluginPathList(const QString &subDir, int &readOnlyPaths);
    static QString pluginFileNameFilter();
    static int trimPluginPathList(QStringList &pathList);

private: // types
    struct PluginInfo {
        enum Flag {
            NoFlags = 0,
            RecycleWidget = 1 // re-use a widget the plugin creates
        };
        Q_DECLARE_FLAGS(Flags, Flag)

        QStringList mTypes; // device dialog types implemented by the plugin
        HbLockedPluginLoader *mLoader; // loaded plugin
        int mRefCount; // number of references into the plugin
        HbDeviceDialogPlugin::PluginFlags mPluginFlags;
        Flags mFlags;
        HbDeviceDialogInterface *mRecycledWidget;
        PluginInfo(){mRefCount = 0; mLoader = 0; mFlags = NoFlags; mRecycledWidget = 0;}
        ~PluginInfo(){if (mLoader) mLoader->unload(); delete mLoader;}
    };

    // Callback function for plugin scan
    typedef HbLockedPluginLoader *(HbDeviceDialogPluginManager::*PluginScanCallback)
        (HbLockedPluginLoader*, const QString&);

private: // functions
    void scanPlugins(PluginScanCallback func, const QString &deviceDialogType, bool stopIfFound = true);
    bool scanPlugin(PluginScanCallback func, const QString &deviceDialogType, const QString &filePath);
    HbLockedPluginLoader *preloadPluginCallback(HbLockedPluginLoader *loader, const QString& unused);
    HbLockedPluginLoader *loadPluginCallback(HbLockedPluginLoader *loader, const QString &deviceDialogType);

    int findPlugin(const QString &deviceDialogType) const;
    void freeRecycleWidgets();
    void updateCachePath(const QString &path, bool updateReadOnly = false);
    static QStringList pluginKeys(QObject *pluginInstance);

private slots:
    void deleteWidgets();
    void allWidgetsDeleted();

private: // data
    Q_DISABLE_COPY(HbDeviceDialogPluginManager)
    Flags mFlags;
    QList<PluginInfo> mPlugins;
    QList<HbDeviceDialogInterface*> mDeleteWidgets; // widgets to be deleted
    bool mAllWidgetsDeleted;
    QTimer mDeleteTimer; // timer used to delete widgets
    HbPluginNameCache mNameCache;
    QStringList mPluginPathList;
};

#endif // HBDEVICEDIALOGPLUGINMANAGER_P_H
