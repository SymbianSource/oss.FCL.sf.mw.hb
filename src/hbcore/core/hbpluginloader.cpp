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

#include "hbpluginloader_p.h"
#include "hbpluginloader_p_p.h"
#include <QtDebug>

HbPluginLoaderPrivate::HbPluginLoaderPrivate()
{
}

HbPluginLoaderPrivate::~HbPluginLoaderPrivate()
{
}

/*
    Constructs a plugin loader with \a parent.
*/
HbPluginLoader::HbPluginLoader(QObject *parent) :
    QPluginLoader(parent), d_ptr(new HbPluginLoaderPrivate)
{
}

/*
    Constructs a plugin loader with \a pluginName and \a parent.
*/
HbPluginLoader::HbPluginLoader(const QString &pluginName, QObject *parent) :
    QPluginLoader(parent), d_ptr(new HbPluginLoaderPrivate)
{
    Q_D(HbPluginLoader);
    d->pluginName = pluginName;
}

/*
    Destructs the plugin loader.
*/
HbPluginLoader::~HbPluginLoader()
{
}

/*
    Returns the plugin name.

    NOTE: Plugin name is the first argument passed to Q_EXPORT_PLUGIN2(PluginName, ClassName).

    \sa setPluginName(), Q_EXPORT_PLUGIN2
*/
QString HbPluginLoader::pluginName() const
{
    Q_D(const HbPluginLoader);
    return d->pluginName;
}

/*
    Sets the plugin name.

    NOTE: Plugin name is the first argument passed to Q_EXPORT_PLUGIN2(PluginName, ClassName).

    \sa pluginName(), Q_EXPORT_PLUGIN2
*/
void HbPluginLoader::setPluginName(const QString &pluginName)
{
    Q_D(HbPluginLoader);
    d->pluginName = pluginName;
}

/*
    This convenience function resolves the actual file name of the plugin first and then calls QPluginLoader::instance().

    NOTE: this function does not override but overshadows QPluginLoader::instance()

    \sa resolveFileName(), QPluginLoader::instance()
*/
QObject *HbPluginLoader::instance()
{
    QObject *result = 0;
    if (resolveFileName()) {
        result = QPluginLoader::instance();
    }
    return result;
}

/*
    This convenience function resolves the actual file name of the plugin first and then calls QPluginLoader::load().

    NOTE: this function does not override but overshadows QPluginLoader::load()

    \sa resolveFileName(), QPluginLoader::load()
*/
bool HbPluginLoader::load()
{
    bool result = false;
    if (resolveFileName()) {
        result = QPluginLoader::load();
    }
    return result;
}

/*
    Resolves the actual file name of the plugin.

    \sa pluginName(), fileName()
*/
bool HbPluginLoader::resolveFileName()
{
    Q_D(HbPluginLoader);
    if (!d->library.isLoaded() && !d->pluginName.isEmpty()) {
        d->library.setFileName(d->pluginName);
        if (d->library.load()) {
            setFileName(d->library.fileName());
        } else {
            qWarning() << "HbPluginLoader::resolveFileName():" << d->library.errorString();
        }
    }
    return d->library.isLoaded();
}
