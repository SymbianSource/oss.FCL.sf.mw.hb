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

// internal
#include "hbfeedbackplugingroup.h"
#include "hbfeedbackmanager.h"
#include "hbfeedbackplugin.h"

// Qt related
#include <QtDebug>
#include <QPluginLoader>
#include <QDir>

class HbFeedbackPluginGroupPrivate
{

public:
    HbFeedbackPluginGroupPrivate(HbFeedbackManager& manager);
    ~HbFeedbackPluginGroupPrivate();

public:
    HbFeedbackManager& manager;
    QList< QPointer< HbFeedbackPlugin> > feedbackPlugins;
};

HbFeedbackPluginGroupPrivate::HbFeedbackPluginGroupPrivate(HbFeedbackManager& manager)
        : manager(manager)
{
}

HbFeedbackPluginGroupPrivate::~HbFeedbackPluginGroupPrivate()
{
}

void HbFeedbackPluginGroup::removePlugins()
{
    foreach(HbFeedbackPlugin* plugin, d->feedbackPlugins) {
        // we cannot trust that dynamically
        // loaded plugins haven't been destroyed somewhere else
        if (plugin) {
            delete plugin;
        }
    }

    d->feedbackPlugins.clear();
}

/*!
    @beta
    @hbcore

    \class HbFeedbackPluginGroup

    \brief Container class that loads and manages all feedback plugins.

    Feedback plugins are used for implementing various feedback features.

    HbFeedbackPluginGroup handles feedback plugins
    \li loadPlugins() loads plugins
    \li plugins() returns the list of plugins
    \li supportedFeatures() lists supported features
    \li isFeatureSupported() returns \c true if the feature is supported
    \li enableFeature() enables feedback feature
    \li disableFeature() disables feedback feature
    \li isFeatureEnabled() returns \c true if the features is enabled

    See \ref arcFeedback "Feedback Manager Framework" for more information about the plugins.

    \sa feedbackPluginInterface
*/


/*!
    Constructor.
*/
HbFeedbackPluginGroup::HbFeedbackPluginGroup(HbFeedbackManager& manager) : 
    d(new HbFeedbackPluginGroupPrivate(manager))
{
}

/*!
    Destructor.
*/
HbFeedbackPluginGroup::~HbFeedbackPluginGroup()
{
    removePlugins();
    delete d;
}


/*!
    Loads platform's featureName plugins.
*/
void HbFeedbackPluginGroup::loadPlugins()
{
    QStringList pluginPathList;
#if defined(Q_OS_SYMBIAN)
    const QString pluginRelativePath("resource/plugins/feedback/");

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
    pluginPathList << HB_PLUGINS_DIR"/feedback/";
#endif

  loadPlugins(pluginPathList);
}

/*!
    Loads plugins from specified directories.

    \param pluginPathList list of directories in which the plugins are located
*/
void HbFeedbackPluginGroup::loadPlugins(QStringList pluginPathList)
{
    // remove old feedbackPlugins before loading new ones
    removePlugins();
    QString nameFilter;

    foreach (const QString &path, pluginPathList) {
        QDir pluginDir(path, nameFilter, QDir::NoSort, QDir::Files | QDir::Readable);
        foreach(const QString& fileName, pluginDir.entryList(QDir::Files)) {
            QPluginLoader loader(pluginDir.absoluteFilePath(fileName));
            QObject *plugin = loader.instance();
            if (plugin) {
                if (HbFeedbackPlugin *feedbackPlugin = qobject_cast<HbFeedbackPlugin *>(plugin)) {
                    addPlugin(feedbackPlugin);
                }
            }
        }
    }
}

//! Adds a plugin to the plugin group.
/*!
  You don't need to call this. Plugins are loaded by the loadPlugins() method.

  \param feedbackPlugin the plugin to be added
*/
void HbFeedbackPluginGroup::addPlugin(HbFeedbackPlugin* feedbackPlugin)
{
    if (feedbackPlugin
        && !d->feedbackPlugins.contains(feedbackPlugin)) {
        d->feedbackPlugins.append(feedbackPlugin);
        feedbackPlugin->engine().setManager(&d->manager);
    }
}

/*!
    List supported feedback features
*/
QStringList HbFeedbackPluginGroup::supportedFeatures() const
{
    QStringList supportedFeatures;
    foreach(HbFeedbackPlugin *plugin, d->feedbackPlugins) {
        // sadly we cannot trust that dynamically
        // loaded plugins aren't destroyed somewhere else
        if (plugin) {
            if (!supportedFeatures.contains(plugin->featureName())) {
                supportedFeatures.append(plugin->featureName());
            }
        }
    }
    return supportedFeatures;
}

/*!
    To check whether feedback feature is supported or not.

    \param featureName the name of the feature
*/
bool HbFeedbackPluginGroup::isFeatureSupported(QString featureName)
{
    if (featureName.isEmpty()) {
        qWarning("HbFeedbackPluginGroup::isFeatureSupported: Attempt to give empty string \
                 as the parameter");
    }

    bool supported(false);
    foreach(HbFeedbackPlugin *plugin, d->feedbackPlugins) {
        // There's a small risk, that dynamically
        // loaded plugins aren't destroyed somewhere else
        if (plugin) {
            if (plugin->featureName() == featureName) {
                supported = true;
                break;
            }
        }
    }
    return supported;
}

/*!
    To check whether \a featureName is enabled or not.

    \param featureName the name of the feature
*/
bool HbFeedbackPluginGroup::isFeatureEnabled(QString featureName)
{
    if (featureName.isEmpty()) {
        qWarning("HbFeedbackPluginGroup::isFeatureEnabled: Attempt to give empty string \
                 as the parameter");
    }

    bool enabled(false);
    foreach(HbFeedbackPlugin *plugin, d->feedbackPlugins) {
        // There's a small risk, that dynamically
        // loaded plugins aren't destroyed somewhere else
        if (plugin) {
            if (plugin->featureName() == featureName) {
                enabled = plugin->engine().receivesInteractions();
                break;
            }
        }
    }
    return enabled;
}

/*!
    Enables feedback feature

    \param featureName the name of the feature
*/
bool HbFeedbackPluginGroup::enableFeature(QString featureName)
{
    if (featureName.isEmpty()) {
        qWarning("HbFeedbackPluginGroup::enableFeature: Attempt to give empty string \
                 as the parameter");
    }
    bool succeeded(false);
    foreach(HbFeedbackPlugin *plugin, d->feedbackPlugins) {
        // There's a small risk, that dynamically
        // loaded plugins aren't destroyed somewhere else
        if (plugin) {
            if (plugin->featureName() == featureName) {
                plugin->engine().setReceivesInteractions(true);
                succeeded = plugin->engine().receivesInteractions();
                if (!succeeded) {
                    break;
                }
            }
        }
    }
    return succeeded;
}

/*!
    Disables feedback feature

    \param featureName the name of the feature
*/
bool HbFeedbackPluginGroup::disableFeature(QString featureName)
{
    if (featureName.isEmpty()) {
        qWarning("HbFeedbackPluginGroup::disableFeature: Attempt to give empty string \
                 as the parameter");
    }

    bool succeeded(false);
    foreach(HbFeedbackPlugin *plugin, d->feedbackPlugins) {
        if (plugin) {
            if (plugin->featureName() == featureName) {
                plugin->engine().setReceivesInteractions(false);
                succeeded = !plugin->engine().receivesInteractions();
                if (!succeeded) {
                    break;
                }
            }
        }
    }
    return succeeded;
}

/*!
  Returns the list of plugins

  \return Reference to the plugin list.
*/
QList< QPointer< HbFeedbackPlugin> >& HbFeedbackPluginGroup::plugins() const
{
    return d->feedbackPlugins;
}
