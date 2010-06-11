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
#ifndef HB_BIN_CSS
#define HB_USETHEMESERVER
#endif

#include "hbwidgetloader_p.h"
#include "hbwidgetloadersyntax_p.h"
#include "hbwidgetloaderactions_p.h"

#include "hbinstance.h"
#include "hbtheme_p.h"
#ifdef HB_USETHEMESERVER
#include "hbthemeclient_p.h"
#endif

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

typedef QHash<QString,HbWidgetLoader::LayoutDefinition*> ClientHashForLayoutDefs;
Q_GLOBAL_STATIC(ClientHashForLayoutDefs, clientLayoutDefsCache)

// List of files that doesn't exist.
// This reduces the check QFile::exists() at client side as well as the server side.
// also no unnecessary IPC calls.
Q_GLOBAL_STATIC(QStringList, filesNotPresent)

// Layout caching
static HbWidgetLoader::LayoutDefinition *staticCacheLayout = 0;
static QString staticCacheFileName;
static QString staticCacheName;
static QString staticCacheSection;
static QDateTime staticCacheModified;

class HbWidgetLoaderPrivate
{
    Q_DECLARE_PUBLIC(HbWidgetLoader)    
        
public:

    HbWidgetLoaderPrivate();
    virtual ~HbWidgetLoaderPrivate();
    
    void setWidget( HbWidget* widget );
    
    bool getSharedLayoutDefinition(
        const QString &fileName,
        const QString &name,
        const QString &section,
        HbWidgetLoader::LayoutDefinition *&layoutDef );

    bool getCachedLayoutDefinition(
        const QString &fileName,
        const QString &name,
        const QString &section,
        HbWidgetLoader::LayoutDefinition *&layoutDef );
    
private:
    Q_DISABLE_COPY(HbWidgetLoaderPrivate)

public:
    HbWidgetLoader* q_ptr;
    
    HbWidgetLoaderActions* mActions;
    HbWidgetLoaderSyntax* mSyntax;
    HbWidgetLoaderMemoryActions* mMemActions;
    HbWidgetLoaderMemorySyntax* mMemSyntax;
};


/*
    \class HbWidgetLoader
    \internal
    \proto
*/

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
    Prints current version of widget loader and minimum version of supported WidgetML in brackets
    For example "3.2 (1.4)" means that current version is 3.2 and WidgetML versions from 1.4 to 3.2 are supported   
*/
QString HbWidgetLoader::version()
{
    return HbWidgetLoaderSyntax::version();       
}

/*!
    Loads and processes a WidgetML file.

    If the widget already has a layout assumes it's HbMeshLayout.
    If the widget doesn't have a layout creates HbMeshLayout and sets it to widget.
    Creates the anchor edge attachments based on WidgetML.

    \param fileName file to be processed.
    \param name the name of the layout to be loaded.
    \param section space separated route to section, that you want to load.
    \param storage specifies where to store the mesh items.
    \return true if file was loaded and processed successfully.
*/
bool HbWidgetLoader::load(
    HbWidget* widget,
    const QString &fileName,
    const QString &name,
    const QString &section,
    HbMemoryManager::MemoryType storage)
{
    Q_D(HbWidgetLoader);
    bool result(true);

    d->setWidget(widget);

    LayoutDefinition* layoutDef(0);

    if (storage == HbMemoryManager::SharedMemory) {
        result = d->getSharedLayoutDefinition(fileName, name, section, layoutDef);
    }
    if (result) {
        if (!layoutDef) {
            //fall back
            result = d->getCachedLayoutDefinition(fileName, name, section, layoutDef);
        }
        if (result){
            Q_ASSERT(layoutDef);
            result = d->mMemSyntax->load(layoutDef);
        }
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
bool HbWidgetLoader::load(
    HbWidget* widget,
    QIODevice *device,
    const QString &name,
    const QString &section)
{
    Q_D(HbWidgetLoader);
    d->setWidget(widget);
    d->mSyntax->setActions(d->mActions);
    return d->mSyntax->load(device, name, section);
}

/*!
    Loads WidgetML to given memory structure.
*/
bool HbWidgetLoader::loadLayoutDefinition(
    LayoutDefinition *targetLayoutDef,
    QIODevice *device,
    const QString &name,
    const QString &section )
{
    Q_D(HbWidgetLoader);
    d->mMemActions->mLayoutDef = targetLayoutDef;
    d->mSyntax->setActions(d->mMemActions);
    return d->mSyntax->load(device, name, section);
}

/*!
    \internal
*/
HbWidgetLoaderPrivate::HbWidgetLoaderPrivate() : q_ptr(0)
{
    mActions = new HbWidgetLoaderActions();
    mMemActions = new HbWidgetLoaderMemoryActions();
    mSyntax = new HbWidgetLoaderSyntax(mMemActions);
    mMemSyntax = new HbWidgetLoaderMemorySyntax(mActions);
}

/*!
    \internal
*/
HbWidgetLoaderPrivate::~HbWidgetLoaderPrivate()
{
    delete mMemSyntax;
    delete mSyntax;
    delete mMemActions;
    delete mActions;
}

#ifdef HB_BIN_CSS
void HbWidgetLoaderPrivate::setWidget( HbWidget* widget )
{
    Q_UNUSED(widget)
}
#else
/*!
    \internal
*/
void HbWidgetLoaderPrivate::setWidget( HbWidget* widget )
{
    mActions->reset();       
    mActions->mWidget = widget;
    mActions->mCurrentProfile = HbDeviceProfile::profile(widget);
}
#endif

/*!
    \internal
*/
bool HbWidgetLoaderPrivate::getSharedLayoutDefinition(
    const QString &fileName,
    const QString &name,
    const QString &section,
    HbWidgetLoader::LayoutDefinition *&layoutDef )
{
    // check in the client side cache if the vector of meshitems is present.
    QString key (fileName + name + section);
    if (clientLayoutDefsCache()->contains(key)){
        // present in the client cache.
        layoutDef = clientLayoutDefsCache()->value(key);
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
#ifdef HB_USETHEMESERVER
    layoutDef = HbThemeClient::global()->getSharedLayoutDefs(fileName, name, section);
    if (layoutDef) {
        clientLayoutDefsCache()->insert(key, layoutDef);
    }
#endif
    return true;
}

/*!
    \internal
*/
bool HbWidgetLoaderPrivate::getCachedLayoutDefinition(
    const QString &fileName,
    const QString &name,
    const QString &section,
    HbWidgetLoader::LayoutDefinition *&layoutDef )
{
    QFileInfo info(fileName);
    
#ifdef HB_WIDGETLOADER_DEBUG
    qDebug() << "Cached layout currently contains" << HbWidgetLoaderActions::mCacheLayout.count() << "items";
#endif
    bool cacheHit = (name == staticCacheName
		&& section == staticCacheSection
		&& fileName == staticCacheFileName 
		&& info.lastModified() == staticCacheModified);
		
    if (cacheHit){
#ifdef HB_WIDGETLOADER_DEBUG
        qDebug() << "Cache hit.";
#endif
        layoutDef = staticCacheLayout;
    	return true;
    }
    
#ifdef HB_WIDGETLOADER_DEBUG
	qDebug() << "Cache miss, reloading cache data";
#endif
		
    // Not found in the client cache.
    if (filesNotPresent()->contains(fileName)){
        return false;
    } 
	QFile file(fileName);
	if ( !file.open( QFile::ReadOnly | QFile::Text ) ) {
	    qWarning( "Unable to open file ");
        filesNotPresent()->append(fileName);
	    return false;
	}

    if (!staticCacheLayout) {
        staticCacheLayout = new HbWidgetLoader::LayoutDefinition(HbMemoryManager::HeapMemory);
    } else {
        Q_ASSERT(staticCacheLayout->type == HbMemoryManager::HeapMemory);
    }

    mMemActions->mLayoutDef = staticCacheLayout;
    mSyntax->setActions(mMemActions);
    bool result = mSyntax->load(&file, name, section);
    if (result){
        layoutDef = staticCacheLayout;
    	staticCacheName = name;
        staticCacheSection = section;
        staticCacheFileName = fileName;
        staticCacheModified = info.lastModified();
    }
    
    return result;
}


