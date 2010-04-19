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

#include "hbwidgetloader_p.h"
#include "hbwidgetloadersyntax_p.h"
#include "hbwidgetloaderactions_p.h"

#include "hbinstance.h"
#include "hbtheme_p.h"

#include <QtDebug>
#include <QFile>
#include <QStringList>
#include <hbwidget.h>
#include <QFileInfo>

class HbThemeClient;

// Uncomment the following in order to get additional debug prints
//#define HB_WIDGETLOADER_DEBUG
 
#ifndef HB_WIDGETLOADER_DEBUG
#define HB_WIDGETLOADER_PRINT(a) 
#else
#include <QDebug>
#define HB_WIDGETLOADER_PRINT(a) qDebug() << QString(a);
#endif // HB_WIDGETLOADER_DEBUG

#define hbInstance (HbInstance::instance())

// cache at the client side to store the mesh items.
// key used here is the filename+layoutname+sectionname.

typedef QHash<QString,LayoutDefinition*> ClientHashForLayoutDefs;
Q_GLOBAL_STATIC(ClientHashForLayoutDefs,clientLayoutDefsCache)

// List of files that doesn't exist.
// This reduces the check QFile::exists() at client side as well as the server side.
// also no unnecessary IPC calls.
Q_GLOBAL_STATIC(QStringList,filesNotPresent)

class HbWidgetLoaderPrivate
{
    Q_DECLARE_PUBLIC(HbWidgetLoader)    
        
public:

    HbWidgetLoaderPrivate();
    virtual ~HbWidgetLoaderPrivate();
    
    void setWidget( HbWidget* widget );
    
    bool updateCacheIfNeeded(const QString &fileName, const QString &name, const QString &section);
    
    static QString version();
private:
    Q_DISABLE_COPY(HbWidgetLoaderPrivate)

public:
    HbWidgetLoader* q_ptr;
    
    HbWidgetLoaderActions* mActions;
    HbWidgetLoaderSyntax* mSyntax;    
};


/*
    \class HbWidgetLoader
    \internal
    \proto
*/
LayoutDefinition *HbWidgetLoaderActions::mCacheLayout = NULL;
QString HbWidgetLoaderActions::mCacheFileName = QString();
QString HbWidgetLoaderActions::mCacheName = QString();
QString HbWidgetLoaderActions::mCacheSection = QString();
QDateTime HbWidgetLoaderActions::mCacheModified = QDateTime();

/*!
    Constructor.
 */
HbWidgetLoader::HbWidgetLoader()
: d_ptr(new HbWidgetLoaderPrivate)
{
    Q_D(HbWidgetLoader);
    d->q_ptr = this;
}

/*!
    Destructor.
*/
HbWidgetLoader::~HbWidgetLoader()
{
    delete d_ptr;
}

/*!
    Set widget. Temporary solution for time being.
    \param widget 
*/
void HbWidgetLoader::setWidget( HbWidget* widget )
{
    Q_D(HbWidgetLoader);
    
    d->mActions->reset();       
    d->mActions->mWidget = widget;
}

/*!
    Prints current version of widget loader and minimum version of supported WidgetML in brackets
    For example "3.2 (1.4)" means that current version is 3.2 and WidgetML versions from 1.4 to 3.2 are supported   
*/
QString HbWidgetLoader::version()
{
    return HbWidgetLoaderPrivate::version();       
}

/*!
    Loads and processes a WidgetML file.

    Proto:
    - Assumes that a widget is set with setWidget.
    - If the widget already has a layout assumes it's HbMeshLayout.
    - If the widget doesn't have a layout creates HbMeshLayout and sets it to widget.
    - Creates the anchor edge attachments for existing child items of the widget.

    \param fileName file to be processed.
    \param name the name of the layout to be loaded.
    \param section space separated route to section, that you want to load.
    \param storage specifies where to store the mesh items.
    \return true if file was loaded and processed successfully.
*/
bool HbWidgetLoader::load( const QString &fileName, const QString &name, const QString &section,const HbMemoryManager::MemoryType storage )
{
    Q_D(HbWidgetLoader);
    bool result = false;

    LayoutDefinition* sharedLayoutDef = NULL;

    // if the storage is SharedMemory
    if (storage == HbMemoryManager::SharedMemory) {
        // check in the client side cache if the vector of meshitems is present.
        QString key (fileName + name + section);
        if(clientLayoutDefsCache()->contains(key)){
            // present in the client cache.
            sharedLayoutDef = clientLayoutDefsCache()->value(key);
            //update the widget layout.
            d->mActions->updateWidget(sharedLayoutDef);
            return true;
        }

        // Not found in the client cache.
        if (filesNotPresent()->contains(fileName)){
            return false;
        } 
        // Check for the availability of the file, as QFile::Exists takes more time this 
        // method is used
        QFile file(fileName);        
        bool fileExists = file.open(QIODevice::ReadOnly);
        file.close();
        if (!fileExists) {
            // file doesn't exist save the info in the filesNotPresent list.
            filesNotPresent()->append(fileName);
            return false;
        }

        // get the shared layout definition address.
        sharedLayoutDef = HbThemeClient::global()->getSharedLayoutDefs(fileName, name, section);

        if (sharedLayoutDef){
            // This will update the widgets layout anchors.
            d->mActions->updateWidget(sharedLayoutDef);

            // Insert in to the client side cache.
            clientLayoutDefsCache()->insert(key, sharedLayoutDef);
            return true;
        }
    }
    //fall back
    result = d->updateCacheIfNeeded(fileName, name, section);
    if (result){
        d->mActions->updateWidget(0);
    }
    return result;
}

/*!
    This is an overloaded member function, provided for convenience.
    \param device IO device to be processed.
    \param name the name of the layout to be loaded.
    \param section space separated route to section, that you want to load.
    \return true if input was loaded and processed successfully.
*/
bool HbWidgetLoader::load( QIODevice *device, const QString &name, const QString &section,const HbMemoryManager::MemoryType storage  )
{
    Q_D(HbWidgetLoader);    
    Q_UNUSED(storage);
	
    bool result = d->mSyntax->load(device, name, section);
    if (result){
    	HbWidgetLoaderActions::mCacheName = name;
        HbWidgetLoaderActions::mCacheSection = section;
        HbWidgetLoaderActions::mCacheFileName = QString();
        HbWidgetLoaderActions::mCacheModified = QDateTime();
		d->mActions->updateWidget(0);
    }
    
    return result;
}


HbWidgetLoaderPrivate::HbWidgetLoaderPrivate() : q_ptr(0)
{
    mActions = new HbWidgetLoaderActions();
    mSyntax = new HbWidgetLoaderSyntax(mActions);
}

HbWidgetLoaderPrivate::~HbWidgetLoaderPrivate()
{
    delete mActions;
    delete mSyntax;
}

bool HbWidgetLoaderPrivate::updateCacheIfNeeded(const QString &fileName, const QString &name, const QString &section)
{
    QFileInfo info(fileName);
    
#ifdef HB_WIDGETLOADER_DEBUG
    qDebug() << "Cached layout currently contains" << HbWidgetLoaderActions::mCacheLayout.count() << "items";
#endif
    bool cacheHit = (name == HbWidgetLoaderActions::mCacheName
		&& section == HbWidgetLoaderActions::mCacheSection
		&& fileName == HbWidgetLoaderActions::mCacheFileName 
		&& info.lastModified() == HbWidgetLoaderActions::mCacheModified);
		
    if(cacheHit){
#ifdef HB_WIDGETLOADER_DEBUG
        qDebug() << "Cache hit.";
#endif
    	return true;
    }
    
    
#ifdef HB_WIDGETLOADER_DEBUG
	qDebug() << "Cache miss, reloading cache data";
#endif
		
	QFile file(fileName);
	if( !file.open( QFile::ReadOnly | QFile::Text ) ) {
	    qWarning( "Unable to open file ");
	    return false;
	}
	    
    bool result = mSyntax->load(&file, name, section);
    if(result){
    	HbWidgetLoaderActions::mCacheName = name;
        HbWidgetLoaderActions::mCacheSection = section;
        HbWidgetLoaderActions::mCacheFileName = fileName;
        HbWidgetLoaderActions::mCacheModified = info.lastModified();
    }
    
    return result;
}

QString HbWidgetLoaderPrivate::version()
{
    return HbWidgetLoaderSyntax::version();
}

