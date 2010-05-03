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

#ifndef HBDOCUMENTLOADER_P_H
#define HBDOCUMENTLOADER_P_H

#include <hbglobal.h>

#include <QHash>
#include <QList>
#include <QXmlStreamReader>
#include <QPointer>

class HbDocumentLoader;
class HbDocumentLoaderSyntax;
class HbDocumentLoaderActions;
class HbXmlLoaderBinaryActions;
class HbXmlLoaderBinarySyntax;
class HbXmlLoaderAbstractActions;

class QPluginLoader;
class HbDocumentLoaderPluginManager;
class HbMainWindow;

QT_BEGIN_NAMESPACE
class QGraphicsWidget;
QT_END_NAMESPACE


class HbDocumentLoaderPrivate
{
    Q_DECLARE_PUBLIC(HbDocumentLoader)    
        
public:

    HbDocumentLoaderPrivate(const HbMainWindow *window);
    virtual ~HbDocumentLoaderPrivate();
    
    bool load( QIODevice *device, const QString &section );
    bool createBinary( QIODevice *srcDevice, QIODevice *dstDevice );
    QList<QObject *> takeAll();

    QGraphicsWidget *findWidget(const QString &name) const;
    QObject *findObject(const QString &name) const;
    
    QObject *lookUp(const QString& type, const QString &name, const QString &plugin);
    
    bool setObjectTree( QList<QObject *> roots );
    
    void reset();
    
    static QString version();

private:
    Q_DISABLE_COPY(HbDocumentLoaderPrivate)

public:
    HbDocumentLoader *q_ptr;
    HbDocumentLoaderActions *actions;
    HbDocumentLoaderSyntax *syntax;
    HbXmlLoaderBinaryActions *binaryactions;
    HbXmlLoaderBinarySyntax *binarysyntax;
    HbDocumentLoaderPluginManager *pluginManager;
};



#ifndef HB_BOOTSTRAPPED

class HbDocumentLoaderPluginManager
{
public:
    HbDocumentLoaderPluginManager();
    ~HbDocumentLoaderPluginManager();

    QObject *lookUp(const QString& type, const QString &name, const QString &plugin);

private:
    void scanPlugins();
    QPluginLoader *lookUpPlugin( const QString &plugin );
    QStringList pluginPathList() const;
    QString pluginFileNameFilter() const;

private:
    QList<QPluginLoader*> mPlugins;
};

#endif //HB_BOOTSTRAPPED

#endif // HBDOCUMENTLOADER_P_H
