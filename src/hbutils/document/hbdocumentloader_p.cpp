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

#include <hbxmlloaderbinaryactions_p.h>
#include <hbxmlloaderbinarysyntax_p.h>

#include <QDebug>

#ifndef HB_BOOTSTRAPPED

#include <QGraphicsWidget>
#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>
#include "hbdocumentloaderplugin.h"
#include <hbmainwindow.h>
#include <hbinstance.h>
#include <QBuffer>

#endif // HB_BOOTSTRAPPED

/*
    \class HbDocumentLoaderPrivate
    \internal
    \proto
*/

//#define DEBUG_TIMES

#ifdef DEBUG_TIMES
#include <QTime>
#include <QDebug>
static QTime debugTime;

#ifdef Q_OS_SYMBIAN
#include <e32debug.h>
#endif

void doDebugPrintX(const char* text)
{
#ifdef Q_OS_SYMBIAN
    RDebug::Printf(text);
#else
    qDebug() << text;
#endif
}

// Takes standard c-format.
void debugPrintX(const char* cformat, ...)
{
    va_list ap;
    va_start(ap, cformat);
    QString str = QString().vsprintf(cformat, ap);
    va_end(ap);
    doDebugPrintX(str.toAscii().constData());
}

#endif // DEBUG_TIMES


HbDocumentLoaderPrivate::HbDocumentLoaderPrivate(const HbMainWindow *window)
: q_ptr(0)
{
#ifdef HB_BOOTSTRAPPED
    Q_UNUSED(window);
#else
    actions = new HbDocumentLoaderActions(this, window);
    binarysyntax = new HbXmlLoaderBinarySyntax(actions);
    pluginManager = new HbDocumentLoaderPluginManager;
#endif
    binaryactions = new HbXmlLoaderBinaryActions();
    syntax = new HbDocumentLoaderSyntax(binaryactions);
}

HbDocumentLoaderPrivate::~HbDocumentLoaderPrivate()
{
    delete binaryactions;
    delete syntax;
#ifndef HB_BOOTSTRAPPED
    delete pluginManager;
    delete actions;
    delete binarysyntax;
#endif
}

bool HbDocumentLoaderPrivate::createBinary( QIODevice *srcDevice, QIODevice *dstDevice )
{
    qDebug() << "createBinary, 1";
    bool result = true;
    syntax->setActions( binaryactions );
#ifdef DEBUG_TIMES
    debugTime.restart();
    debugPrintX("MYTRACE: DocML create binary, start");
#endif
    binaryactions->setOutputDevice( dstDevice );
    // TODO: Has to process all sections!
    QList<QString> sectionsList;
    QHash< QString, qint64 > sectionsPositionList;
    qint64 startPos = srcDevice->pos();
    qDebug() << "createBinary, 2";
    if( syntax->scanForSections( srcDevice, sectionsList ) ) {
        qDebug() << "createBinary, 3";
        srcDevice->seek( startPos );
        qDebug() << "createBinary, 4";
        result = syntax->load( srcDevice, "" );
        if( !sectionsList.isEmpty() ) {
            for( int i = 0; i < sectionsList.size(); i++ ) {
                sectionsPositionList[ sectionsList.at( i ) ] = dstDevice->pos();
                srcDevice->seek( startPos );
                qDebug() << "createBinary, 5, " << i;
                result &= syntax->load( srcDevice, sectionsList.at( i ) );
                qDebug() << "createBinary, 6, " << i;
            }
        }
    } else {
        result = false;
    }
    qDebug() << "createBinary, 7";
    qint64 sectionsMetaDataPos = dstDevice->pos();
    QDataStream stream( dstDevice );
    stream << sectionsPositionList;
    stream << sectionsMetaDataPos;

    qDebug() << "createBinary, 8";

#ifdef DEBUG_TIMES
    debugPrintX("MYTRACE: DocML create binary, end: %d", debugTime.elapsed());
#endif
    return result;
}

bool HbDocumentLoaderPrivate::load( QIODevice *device, const QString &section )
{
#ifdef HB_BOOTSTRAPPED
    Q_UNUSED(device);
    Q_UNUSED(section);
    return false;
#else
    bool result(true);

    if (binarysyntax->isBinary(device)) {
        binarysyntax->setActions(actions);
#ifdef DEBUG_TIMES
        debugTime.restart();
        debugPrintX("MYTRACE: DocML load binary, start");
#endif
        result = binarysyntax->load( device, section );
#ifdef DEBUG_TIMES
        debugPrintX("MYTRACE: DocML load binary, end: %d", debugTime.elapsed());
#endif
    } else {
        syntax->setActions(actions);
#ifdef DEBUG_TIMES
        debugTime.restart();
        debugPrintX("MYTRACE: DocML load plain text, start");
#endif
        result = syntax->load( device, section );
#ifdef DEBUG_TIMES
        debugPrintX("MYTRACE: DocML load plain text, end: %d", debugTime.elapsed());
#endif
    }
    return result;
#endif
}

QList<QObject *> HbDocumentLoaderPrivate::takeAll()
{
#ifdef HB_BOOTSTRAPPED
    QList<QObject *> list;
    return list;
#else
    return actions->takeAll();
#endif
}

QGraphicsWidget* HbDocumentLoaderPrivate::findWidget(const QString &name) const
{
#ifdef HB_BOOTSTRAPPED
    Q_UNUSED(name);
    return 0;
#else
    return actions->findWidget( name );
#endif
}

QObject* HbDocumentLoaderPrivate::findObject(const QString &name) const
{
#ifdef HB_BOOTSTRAPPED
    Q_UNUSED(name);
    return 0;
#else
    return actions->findObject( name );
#endif
}


QObject *HbDocumentLoaderPrivate::lookUp(const QString& type, const QString &name, const QString &plugin)
{
#ifdef HB_BOOTSTRAPPED
    Q_UNUSED(type);
    Q_UNUSED(name);
    Q_UNUSED(plugin);
    return 0;
#else
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
#endif
}

void HbDocumentLoaderPrivate::reset()
{
#ifndef HB_BOOTSTRAPPED
    actions->reset();
#endif
}

bool HbDocumentLoaderPrivate::setObjectTree( QList<QObject *> roots )
{
#ifdef HB_BOOTSTRAPPED
    Q_UNUSED(roots);
    return false;
#else
    return actions->setObjectTree( roots );
#endif
}

QString HbDocumentLoaderPrivate::version()
{
#ifdef HB_BOOTSTRAPPED
    return QString();
#else
    return HbDocumentLoaderSyntax::version();
#endif
}


#ifndef HB_BOOTSTRAPPED
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
#if defined(Q_OS_MAC)
    return QString("*.dylib");
#elif defined(Q_OS_WIN32)
    return QString("*plugin.dll");
#elif defined(Q_OS_SYMBIAN)
    return QString("*.qtplugin");
#else
    return QString("*.so");
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
#if defined(Q_OS_UNIX) && !defined(Q_OS_SYMBIAN)
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
#if defined(Q_OS_UNIX) && !defined(Q_OS_SYMBIAN)
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

#endif // HB_BOOTSTRAPPED

// end of file

