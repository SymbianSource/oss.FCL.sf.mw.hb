/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbUtils module of the UI Extensions for Mobile.
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

#include "hbdocumentloader_p.h"
#include "hbdocumentloaderactions_p.h"
#include "hbdocumentloadersyntax_p.h"
#include "hbdocumentloader.h"

#include <QGraphicsWidget>

#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>
#include "hbdocumentloaderplugin.h"
#include <hbmainwindow.h>
#include <hbinstance.h>

/*
    \class HbDocumentLoaderPrivate
    \internal
    \proto
*/


HbDocumentLoaderPrivate::HbDocumentLoaderPrivate(const HbMainWindow *window)
: q_ptr(0)
{
    actions = new HbDocumentLoaderActions(this);
    syntax = new HbDocumentLoaderSyntax(actions, window);
    pluginManager = new HbDocumentLoaderPluginManager;
}

HbDocumentLoaderPrivate::~HbDocumentLoaderPrivate()
{
    delete pluginManager;
    delete syntax;
    delete actions;
}

bool HbDocumentLoaderPrivate::load( QIODevice *device, const QString &section )
{
    return syntax->load( device, section );
}

QList<QObject *> HbDocumentLoaderPrivate::takeAll()
{
    return actions->takeAll();
}

QGraphicsWidget* HbDocumentLoaderPrivate::findWidget(const QString &name) const
{
    return actions->findWidget( name );
}

QObject* HbDocumentLoaderPrivate::findObject(const QString &name) const
{
    return actions->findObject( name );
}


QObject *HbDocumentLoaderPrivate::lookUp(const QString& type, const QString &name, const QString &plugin)
{
    if ( plugin.isEmpty() ) {
        return q_ptr->createObject( type, name );
    } else {
        QObject *object = pluginManager->lookUp( type, name, plugin );
        if ( !object ) {
            // use factory loader for fallback
            object = q_ptr->createObject( type, name );
        }
        return object;
    }
}

void HbDocumentLoaderPrivate::reset()
{
    actions->reset();
}

bool HbDocumentLoaderPrivate::setObjectTree( QList<QObject *> roots )
{
    return actions->setObjectTree( roots );
}

QString HbDocumentLoaderPrivate::version()
{
    return HbDocumentLoaderSyntax::version();
}


HbDocumentLoaderPluginManager::HbDocumentLoaderPluginManager()
{
}

HbDocumentLoaderPluginManager::~HbDocumentLoaderPluginManager()
{
    while ( mPlugins.count() ) {
        QPluginLoader *loader = mPlugins.at(0);
        mPlugins.removeAt( 0 );
        //loader->unload();
        delete loader;
    }
}

QStringList HbDocumentLoaderPluginManager::pluginPathList() const
{
    return hbInstance->libraryPaths();
}

QString HbDocumentLoaderPluginManager::pluginFileNameFilter() const
{
#if defined(Q_OS_LINUX)
    return QString("*.so");
#elif defined(Q_OS_MAC)
    return QString("*.dylib");
#elif defined(Q_OS_WIN32)
    return QString("*plugin.dll");
#else
    return QString("*.qtplugin");
#endif
}

void HbDocumentLoaderPluginManager::scanPlugins()
{
    const QStringList pathList = pluginPathList();
    const QString fileNameFilter = pluginFileNameFilter();

    foreach (const QString &path, pathList) {
        QDir pluginDir(path, fileNameFilter, QDir::Unsorted, QDir::Files | QDir::Readable);

        foreach (const QString &fileName, pluginDir.entryList()) {
            QPluginLoader *loader = new QPluginLoader(pluginDir.absoluteFilePath(fileName));
            QObject *pluginInstance = loader->instance();

            if (pluginInstance) {
                    HbDocumentLoaderPlugin *plugin =
                        qobject_cast<HbDocumentLoaderPlugin*>(pluginInstance);
                // plugin found. Call function handle it.
                if (plugin) {
                    mPlugins.append( loader );
                    continue;
                }
                loader->unload();
            }
            delete loader;
        }
    }
}

QObject *HbDocumentLoaderPluginManager::lookUp(const QString& type, const QString &name, const QString &plugin)
{
    QObject *result = 0;
    QPluginLoader *loader = lookUpPlugin( plugin );
    if ( loader ) {
        result = qobject_cast<HbDocumentLoaderPlugin*>(loader->instance())->createObject( type, name );
    } else {
        HB_DOCUMENTLOADER_PRINT( QString( "Cannot find plug-in: " ) + plugin );
    }
    return result;
}

QPluginLoader *HbDocumentLoaderPluginManager::lookUpPlugin( const QString &plugin )
{
    // check the exising plug-ins
    foreach( QPluginLoader *loader, mPlugins ) {
        const QFileInfo fileInfo( loader->fileName() );
        QString compareName;
#ifdef Q_OS_LINUX
        compareName.append( "lib" );
#endif
        compareName.append( plugin );
        if ( fileInfo.baseName() == compareName ) {
            return loader;
        }
    }

    // not found -> try to find it.
    const QStringList pathList = pluginPathList();
    QString fileNameFilter;
#ifdef Q_OS_LINUX
    fileNameFilter.append( "lib" );
#endif
    fileNameFilter.append( plugin );
    fileNameFilter.append( "*" );

    foreach (const QString &path, pathList) {
        QDir pluginDir(path, fileNameFilter, QDir::Unsorted, QDir::Files | QDir::Readable);

        foreach (const QString &fileName, pluginDir.entryList()) {

            QPluginLoader *loader = new QPluginLoader(pluginDir.absoluteFilePath(fileName));
            QObject *pluginInstance = loader->instance();

            if (pluginInstance) {
                HbDocumentLoaderPlugin *plugin =
                    qobject_cast<HbDocumentLoaderPlugin*>(pluginInstance);
                if (plugin) {
                    mPlugins.append( loader );
                    return loader;
                } else {
                    loader->unload();
                }
            }
        }
    }
    return 0;
}

// end of file

