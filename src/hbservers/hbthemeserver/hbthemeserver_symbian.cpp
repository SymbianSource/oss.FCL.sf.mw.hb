/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbServers module of the UI Extensions for Mobile.
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

#include "hbthemeserver_symbian_p_p.h"
#include "hbthemeserverutils_p.h"
#include "hbmemorymanager_p.h"
#include "hbiconsource_p.h"
#include "hbthemeindex_p.h"
#include "hbthemeutils_p.h"

#include <QHash>
#include <QImage>
#include <QSettings>
#include <QSharedMemory>
#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QPainter>
#include <QSizeF>
#include <e32property.h>
#include <e32base.h>
#include <e32svr.h>
#include "hbpixmapiconprocessor_p.h"
#include "hbpixmapiconimpl_p.h"
#include "hblayeredstyleloader_p.h"
#include "hbmemoryutils_p.h"
#include "hbdeviceprofiledatabase_p.h"
#ifdef HB_SGIMAGE_ICON
#include "hbsgimagerenderer_p.h"
#endif
// 5 MB GPU cache size
#define GPU_CACHE_SIZE 0x500000

// 5 MB  CPU cache size
#define CPU_CACHE_SIZE 0x500000

const TInt KThemeName = 0;



static HbThemeServerPrivate *TheServer = 0;
bool HbThemeServerPrivate::gpuGoodMemoryState = true;
// This is used as parent theme always regardless of the active theme
static const char *baseThemeName = "hbdefault";
const int themeIndexFailed = -2; // error code to indicate failure in processing theme index

//**********************************
//HbThemeServerPrivate
//**********************************
/**
Our server class - an active object - and therefore derived ultimately from CActive.
It accepts requests from client threads and forwards
them to the client session to be dealt with. It also handles the creation
of the server-side client session.
*/

/**
NewL
 */
HbThemeServerPrivate * HbThemeServerPrivate::NewL(CActive::TPriority aActiveObjectPriority)
{
    HbThemeServerPrivate* self = new(ELeave) HbThemeServerPrivate(aActiveObjectPriority);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(); // self
    return self;
}

/**
ConstructL
 */
void HbThemeServerPrivate::ConstructL()
{
    TInt err = RProperty::Define(KServerUid3, KThemeName, RProperty::ELargeText);
    if ( err != KErrAlreadyExists ) {
        User::LeaveIfError( err );
    }
    TInt error = iThemeProperty.Attach(KServerUid3, KThemeName );
    User::LeaveIfError(error);
    
    QSettings settings(QLatin1String(ORGANIZATION), QLatin1String(THEME_COMPONENT));
    // Store the active theme name in a member string
    iCurrentThemeName = settings.value("currenttheme").toString();
    
    // HACK
    if (iCurrentThemeName.isEmpty()) {
        iCurrentThemeName = "sfblacktheme";
    }
    
    // TODO: Get default theme name if current theme setting does not return theme name

/*
    if (iCurrentThemeName.isEmpty()) {
        iCurrentThemeName = HbThemeUtils::defaultTheme();
        settings.setValue("currenttheme", iCurrentThemeName); 
        settings.sync();
        TPtrC name(reinterpret_cast<const TUint16 *>(iCurrentThemeName.constData()));
        iThemeProperty.Set(name);
    }
*/

    cache = 0;
    cssCache = 0;

    QT_TRY {
        //Create the Icon cache
        cache = new HbIconDataCache();
        //Create the CSS cache
        cssCache = new HbCache();
    } QT_CATCH(const std::bad_alloc &badalloc) {
        delete cache;
        delete cssCache;
        qt_symbian_exception2LeaveL(badalloc);
    }
    setMaxGpuCacheSize(GPU_CACHE_SIZE);
    setMaxCpuCacheSize(CPU_CACHE_SIZE);

    // Process base theme index, it is used as parent index also when the current theme is something else
    processThemeIndex(baseThemeName, QChar('Z'));
    baseThemeIndexKey = themeIndexKey(baseThemeName, QChar('Z'));	
	
    // Temporary hack for pre-loading app. background graphics in server startup to give more realistic
    // results in performance tests. (Normally these graphics get loaded anyway when the first hb app is started.)
#ifndef HB_NVG_CS_ICON
    QProcess::startDetached("hbiconpreloader.exe");
#endif

    // Start the splash screen generator app.
    //QProcess::startDetached("hbsplashgenerator.exe");
}

/**
Constructor takes the server priority value.

The server is an active object, and the priority value is the priority
of this active object.

It passes the priority value to the base class in the Ctor list.
By default, the session is not sharable, which is what we want here
so no second parameter is passed to the CServer2 constructor.
*/
HbThemeServerPrivate::HbThemeServerPrivate(CActive::TPriority aActiveObjectPriority)
    : CServer2( aActiveObjectPriority ),
      currentThemeDrive('Z') // Assume default theme is in ROM
{
    // Set server pointer in static variable
    TheServer = this;
}

/**
Destructor
 */
HbThemeServerPrivate::~HbThemeServerPrivate()
{
    delete cache;
    delete cssCache;
    cache = 0;      // so that HbThemeServerSession::~HbThemeServerSession can avoid using these pointers;
    cssCache = 0;   // it may be called inside HbThemeServerPrivate::~HbThemeServerPrivate
}

HbThemeServerPrivate *HbThemeServerPrivate::Instance()
{
    return TheServer;
}

void HbThemeServerPrivate::openCurrentIndexFile()
{
    // Open index file to prevent uninstallation of the active theme
    if (!iCurrentThemeName.isEmpty() && currentThemeDrive != 'Z') {
        QString indexFileName;
        indexFileName.append(currentThemeDrive);
        indexFileName.append(":\\resource\\hb\\themes\\icons\\");
        indexFileName.append(iCurrentThemeName);
        indexFileName.append("\\index.theme");

        currentIndexfile.setFileName(indexFileName);
        if(!currentIndexfile.open(QIODevice::ReadOnly)) {
            qWarning()<< "HbSymbianThemeServer: No Index file found in the new theme, How did this happen ??";
        }
    }
}

void HbThemeServerPrivate::resolveCurrentThemeDrive()
{
    if (!iCurrentThemeName.isEmpty()) {
        // Check for the theme's icon directory in different drives.
        // ROM is checked first and then phone memory and memory card drives.

        QString filename = "Z:\\resource\\hb\\themes\\icons\\";
        filename.append(iCurrentThemeName);

        if (QFile::exists(filename)) {
            currentThemeDrive = 'Z';
            return;
        }
    
        filename[0] = 'C';
        if (QFile::exists(filename)) {
            currentThemeDrive = 'C';
            return;
        }

        filename[0] = 'E';
        if (QFile::exists(filename)) {
            currentThemeDrive = 'E';
            return;
        }
        
        // Default to 'Z' if not found in any drive
        currentThemeDrive = 'Z';
    }
}

/**
Creates a new session with the server.
*/
CSession2* HbThemeServerPrivate::NewSessionL(const TVersion& aVersion, const RMessage2& /*aMessage*/) const
{
#ifdef THEME_SERVER_TRACES
    qDebug() << "HbThemeServerPrivate::NewSessionL: entered";
#endif
    // Check that the version is OK
    TVersion v(KThemeServerMajorVersionNumber, KThemeServerMinorVersionNumber, KThemeServerBuildVersionNumber);
    if (!User::QueryVersionSupported(v, aVersion))
        User::Leave(KErrNotSupported);
    // Create the session.
    return new(ELeave) HbThemeServerSession(const_cast<HbThemeServerPrivate*>(this));
}

/**
A utility function to panic the server.
*/
void HbThemeServerPrivate::PanicServer(TPixmapServPanic aPanic)
{
    _LIT(KTxtServerPanic, "Pixmap server panic");
    User::Panic(KTxtServerPanic, aPanic);
}

/**
 * HbThemeServerPrivate::insertIconCacheItem
 *
 * Inserts an icon-cache item along with its key into the icon-cache.
 */
bool HbThemeServerPrivate::insertIconCacheItem(const HbIconKey &key,  HbIconCacheItem* item)
{
    return (cache->insert(key, item));

}

/**
 * HbThemeServerPrivate::insertCssCacheItem
 *
 * Inserts a css-cache item along with its key into the css-cache.
 */
bool HbThemeServerPrivate::insertCssCacheItem(const QString& key,  HbCacheItem* item)
{
    return (cssCache->insert(key, item));
}

/**
 * HbThemeServerPrivate::iconCacheItem
 *
 * Retrieves a icon cache-item from the icon cache based on it's key.
 */
HbIconCacheItem * HbThemeServerPrivate::iconCacheItem(const HbIconKey &key , bool isMultiIconPiece)
{
    return(cache->getCacheItem(key, isMultiIconPiece));
}

/**
 * HbThemeServerPrivate::cssCacheItem
 *
 * Retrieves a css-cache item from the css cache based on it's key.
 */
HbCacheItem * HbThemeServerPrivate::cssCacheItem(const QString &key)
{
    return(cssCache->cacheItem(key));
}

void HbThemeServerPrivate::insertIconDefaultSizeCacheItem(const QString &key, const QSizeF &item)
{
    iconDefaultSizes.insert(key, item);
}

QSizeF HbThemeServerPrivate::iconDefaultSizeCacheItem(const QString &key)
{
    return iconDefaultSizes.value(key);
}

/**
 * HbThemeServerPrivate::clearIconCache
 *
 * Clears icon cache.
 */
void HbThemeServerPrivate::clearIconCache()
{
    cache->clear();
}

/**
 * HbThemeServerPrivate::clearCssCache
 *
 * Clears css cache.
 */
void HbThemeServerPrivate::clearCssCache()
{
    cssCache->clear();
}

/**
 * HbThemeServerPrivate::CleanupSessionIconItem
 *
 * Removes an icon cache-item from icon-cache based on it's key.
 */
void HbThemeServerPrivate::CleanupSessionIconItem(HbIconKey key)
{
    /*
        Don't call any HbIconDataCache functions if HbThemeServerPrivate has already deleted it,
         which happens when ThemeServer is closed before the client(s).
    */
    if (cache)
        cache->remove(key);
}

/**
 * HbThemeServerPrivate::CleanupSessionCssItem
 *
 * Removes a css cache-item from css-cache based on it's key.
 */
void HbThemeServerPrivate::CleanupSessionCssItem(QString key)
{
    /*
        Don't call any HbCache ( CssCache )  functions if HbThemeServerPrivate has already deleted it,
         which happens when ThemeServer is closed before the client(s).
    */
    if (cssCache)
        cssCache->remove(key);
}

/**
HbThemeServerPrivate::setMaxGpuCacheSize
 */
void HbThemeServerPrivate::setMaxGpuCacheSize(int size)
{
    cache->setMaxGpuCacheSize(size);
}

/**
HbThemeServerPrivate::setMaxGpuCacheSize
 */
void HbThemeServerPrivate::setMaxCpuCacheSize(int size)
{
    cache->setMaxCpuCacheSize(size);
}

void HbThemeServerPrivate::MemoryGood()
{
    gpuGoodMemoryState = true;
    cache->memoryGood();
}

void HbThemeServerPrivate::FreeGpuRam(int bytes)
{
    gpuGoodMemoryState = false;
    cache->freeGpuRam(bytes);
}

bool HbThemeServerPrivate::gpuMemoryState()
{
    return gpuGoodMemoryState;
}


//Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
int HbThemeServerPrivate ::cacheIconCount() const
{
    return cache->count();
}

unsigned long HbThemeServerPrivate::totalGPUMemory()
{
#ifdef HB_SGIMAGE_ICON
    qDebug() << "Inside  HbThemeServerSymbian::totalGPUMemory()  " ;
    return HbSgImageRenderer::instance()->totalGPUMemory();
#else
    return 0;
#endif
}

unsigned long HbThemeServerPrivate::freeGPUMemory()
{
#ifdef HB_SGIMAGE_ICON
    qDebug() << "Inside  HbThemeServerSymbian::freeGPUMemory()  " ;
    return HbSgImageRenderer::instance()->freeGPUMemory();
#else
    return 0;
#endif
}
int HbThemeServerPrivate::freeVectorMemory()
{
    return cache->freeVectorMemory();
}

int HbThemeServerPrivate::freeRasterMemory()
{
    return cache->freeRasterMemory();
}

int HbThemeServerPrivate::lastAddedRefCount()
{
    return cache->lastAddedRefCount();
}

int HbThemeServerPrivate::lastAddedItemMem()
{
    return cache->lastAddedItemMem();
}

int HbThemeServerPrivate::lastRemovedItemMem()
{
    return cache->lastRemovedItemMem();
}

int HbThemeServerPrivate::lastRemovedItemRfCount()
{
    return cache->lastRemovedItemRfCount();
}

bool HbThemeServerPrivate::enableCache(bool cacheIt)
{
    return cache->enableCache(cacheIt);
}

int HbThemeServerPrivate::cacheHitCount()
{
    return cache->cacheHitCount();
}

int HbThemeServerPrivate::cacheMissCount()
{
    return cache->cacheMissCount();
}

int HbThemeServerPrivate::serverHeapSize()
{
    TInt heapSize = 0;
    User::AllocSize(heapSize);
    return heapSize;
}

void HbThemeServerPrivate::cleanVectorLRUList()
{
    cache->cleanVectorLRUList();
}

void HbThemeServerPrivate::cleanRasterLRUList()
{
    cache->cleanRasterLRUList();
}

int HbThemeServerPrivate::rasterLruCount()
{
    return cache->rasterLruCount();
}

int HbThemeServerPrivate::vectorLruCount()
{
    return cache->vectorLruCount();
}

int HbThemeServerPrivate::gpuLRUSize() const
{
    return cache->gpuLRUSize();
}
#endif

/**
 * HbThemeServerPrivate::doCleanup()
 *
 * This function releases shared memory occupied by css-resources whose reference count is zero,
 * so that subsequent css-requests could be fulfilled by the server. Those css-files whose reference
 * count are zero, are already appended to the LRU list maintained by the css-cache. Since these resources
 * are not being referred to by any application, they can be removed from the cache and corresponding
 * shared memory can be freed up.
 */
void HbThemeServerPrivate::doCleanup()
{
    HbThemeServerUtils::cleanupUnusedCss(cssCache);
}

QString HbThemeServerPrivate::themeIndexKey(const QString &theme, const QChar drive)
{
    QString key(drive);
    key.append(':');
    key.append(theme);
    return key;
}

void HbThemeServerPrivate::processThemeIndex(const QString &theme, const QChar drive)
{
    QString filename(drive);
    filename.append(":\\resource\\hb\\themes\\");
    filename.append(theme);
    filename.append(".themeindex");
    
    QFile indexFile(filename);

    bool indexOK = false;

    if (indexFile.open(QIODevice::ReadOnly)) {

        indexOK = true;

        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);

        qint64 byteSize = indexFile.size();
        
        int offset = manager->alloc(byteSize);
        if (offset >= 0) {        
            // Read the theme index in the shared chunk
            char *address = HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, offset);

            #ifdef THEME_INDEX_TRACES
            qDebug() <<  "ThemeIndex: Reading themeindex for theme" << theme.toUtf8();
            #endif

            indexFile.read(address, byteSize);
            indexFile.close();

            #ifdef THEME_INDEX_TRACES
            qDebug() <<  "ThemeIndex: Reading themeindex for theme" << theme.toUtf8() << "... Done!";
            #endif

            // Verify theme index contents if it is not located in ROM,
            // so that it does not have over-indexing offsets which might
            // crash all the clients trying to read from it.
            
            if (drive != 'Z') {
                #ifdef THEME_INDEX_TRACES
                qDebug() <<  "ThemeIndex: Validating themeindex for theme" << theme.toUtf8();
                #endif

                HbThemeIndex index(address);
                int count = index.itemCount();
                if (count * sizeof(HbThemeIndexItem) >= byteSize) {
                    indexOK = false;
                } else {
                    const HbThemeIndexItem *itemArray = index.itemArray();
                    int stringAreaStart = reinterpret_cast<int>(index.stringAreaStart()) - reinterpret_cast<int>(address);

                    for (int i = 0; i<count; i++) {
                        const HbThemeIndexItem *item = itemArray++;
                        if (item->iconnameOffset < stringAreaStart || item->iconnameOffset >= byteSize ||
                            item->folderOffset < stringAreaStart || item->folderOffset >= byteSize ||
                            item->extOffset < stringAreaStart || item->extOffset >= byteSize ||
                            item->mirroredExtOffset != -1 &&
                            (item->mirroredExtOffset < stringAreaStart || item->mirroredExtOffset >= byteSize)) {
                            
                            indexOK = false;
                            break;
                        }
                    }
                    
                }

                #ifdef THEME_INDEX_TRACES
                qDebug() <<  "ThemeIndex: Validating themeindex for theme" << theme.toUtf8() << "... Done!";
                #endif
            }

            if (indexOK) {
                // Clear some old indexes so the chunk does not get filled completely if
                // the user switches themes a lot.
                removeOldThemeIndexes();            
                // Store offset to the index in chunk
                lastThemeIndexKey = themeIndexKey(theme, drive);
                themeIndexes.insert(lastThemeIndexKey, offset);                
            } else {
                // If the index contents were not OK, remove the index from the chunk
                manager->free(offset);
            }
        }
    }


    if (!indexOK) {
        // If there was a problem with the index, store error code so the index loading is not retried.
        themeIndexes.insert(themeIndexKey(theme, drive), themeIndexFailed);
    }
}

void HbThemeServerPrivate::removeOldThemeIndexes()
{
    // This function removes the previously used theme indexes from the shared chunk.
    // The last index is not removed, because some clients might be still using it.
    // Also the base theme index is never removed.
  
/*
    QMap<QString, int>::iterator i = themeIndexes.begin();

    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);

    while (i != themeIndexes.end()) {
        if (i.value() != themeIndexFailed && i.key() != baseThemeIndexKey && i.key() != lastThemeIndexKey) {
            // Remove the theme index table from the shared chunk and offset map
            manager->free(i.value());        
            i = themeIndexes.erase(i); // Moves iterator to the next item in the map
        } else {
            i++;
        }
    }
*/

}

void HbThemeServerPrivate::getThemeIndexTables(ThemeIndexTables &tables)
{
    #ifdef THEME_INDEX_TRACES
    qDebug() << "ThemeIndex: Server request for theme index tables. Current theme =" << iCurrentThemeName.toUtf8();
    #endif

    QString key = themeIndexKey(iCurrentThemeName, currentThemeDrive);
    int offset = themeIndexes.value(key, -1);

    // If the index has failed earlier, do not retry
    if (offset == themeIndexFailed) {
        return;
    }

    if (offset == -1) {
        processThemeIndex(iCurrentThemeName, currentThemeDrive);
    }
    
    tables.tables[0] = themeIndexes.value(key, -1);
    tables.drives[0] = currentThemeDrive;

    // Set base theme table as parent table if the current theme is not the base theme
    if (tables.tables[0] >= 0 && iCurrentThemeName != baseThemeName) {
        QString base(baseThemeName);
        tables.tables[1] = themeIndexes.value(themeIndexKey(base, 'Z'), -1);
        tables.drives[1] = 'Z';
    }    
}


//**********************************
//HbThemeServerSession
//**********************************
/**
This class represents a session with the  server.
Functions are provided to respond appropriately to client messages.
*/

/**
Constructor
*/
HbThemeServerSession::HbThemeServerSession(HbThemeServerPrivate *aServer):
        iServer(aServer)
{

}

/**
Destructor
*/
HbThemeServerSession::~HbThemeServerSession()
{
    //Clean up the icon related session-specific info
    QList<HbIconKey>::const_iterator itEnd(sessionData.constEnd());
    for (QList<HbIconKey>::const_iterator iter = sessionData.constBegin();
            iter != itEnd;
            ++iter) {
        iServer->CleanupSessionIconItem(*iter);
    }
    //clean up css related session-specific info
    QList<QString>::const_iterator iterEnd(sessionCssData.constEnd());
    for (QList<QString>::const_iterator iter = sessionCssData.constBegin();
            iter != iterEnd;
            ++iter) {
        iServer->CleanupSessionCssItem(*iter);
    }

    sessionData.clear();
    sessionCssData.clear();
}

TIconParams HbThemeServerSession::ReadMessageAndRetrieveParams(const RMessage2& aMessage)
{
    TInt deslen = aMessage.GetDesLength(0);
    TIconParams params;
    TPckg<TIconParams> paramPckg(params);
    // Copy the client's descriptor data into our buffer.
    aMessage.ReadL(0, paramPckg, 0);
    return params;
}

/**
Services a client request.
*/
void HbThemeServerSession::ServiceL(const RMessage2& aMessage)
{
#ifdef THEME_SERVER_TRACES
    qDebug() << "Just entered HbThemeServerSession::ServiceL";
#endif

    TRAPD(err, DispatchMessageL(aMessage));
    aMessage.Complete(err);
    QString er;
    er.setNum(err);

#ifdef THEME_SERVER_TRACES
    qDebug() << "completed DispatchMessageL error code is " + er;
#endif

}

/**
Called by ServiceL()

It tests the function code and then delegates to
the appropriate function.
*/
void HbThemeServerSession::DispatchMessageL(const RMessage2& aMessage)
{
#ifdef THEME_SERVER_TRACES
    qDebug() << "Just entered HbThemeServerSession::DispatchMessageL";
#endif

    switch (aMessage.Function()) {
    case EStyleSheetLookup:
        HandleStyleSheetLookupL(aMessage);
        break;
    case EWidgetMLLookup:
        HandleWidgetMLLookupL(aMessage);
        break;
    case EDeviceProfileOffset:
        HandleDeviceProfilesReqL(aMessage);
        break;
    case ESecondaryCacheOffset:
        HandleSecondaryCacheOffsetReqL(aMessage);
        break;
    case EEffectAdd: // FALLTHROUGH
    case EEffectLookupFilePath:
        HandleEffectAddAndFileLookupL(aMessage);
        break;

    case EIconLookup:
        GetSharedIconInfoL(aMessage);
        break;

    case EIconDefaultSize:
        GetSharedIconDefaultSizeInfoL(aMessage);
        break;

    case EThemeSelection:
        HandleThemeSelectionL(aMessage);
        break;

    case EMultiPieceIcon:
        GetSharedMultiIconInfoL(aMessage);
        break;

    case EMultiIcon:
        GetMultiIconInfoL(aMessage);
        break;

    case EUnloadIcon:
        unLoadIcon(aMessage);
        break;

    case EUnloadMultiIcon:
        unloadMultiIcon(aMessage);
        break;

        //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
    case ECacheIconCount: {
        TInt count = iServer->cacheIconCount();
        TPckg<TInt> out(count);
        aMessage.WriteL(1, out);
        break;
    }
    case ERasterMemLimit: {
        TInt params = 0;
        TPckg<TInt> paramPckg(params);
        aMessage.ReadL(0, paramPckg, 0);

        iServer->setMaxGpuCacheSize(params);
        break;
    }
    case EVectorMemLimit: {
        TInt params = 0;
        TPckg<TInt> paramPckg(params);
        aMessage.ReadL(0, paramPckg, 0);
        iServer->setMaxCpuCacheSize(params);
        break;
    }
    case EFreeRasterMem: {
        TInt freeRastMem = iServer->freeRasterMemory();
        TPckg<TInt> out(freeRastMem);
        aMessage.WriteL(1, out);
        break;
    }
    case EFreeVectorMem: {
        TInt freeVectMem = iServer->freeVectorMemory();
        TPckg<TInt> out(freeVectMem);
        aMessage.WriteL(1, out);
        break;
    }

    case EFreeGPUMem: {
        unsigned long freeMem = iServer->freeGPUMemory();
        TPckg<unsigned long> out(freeMem);
        aMessage.WriteL(1, out);
        break;
    }
    case ETotalGPUMem: {
        unsigned long totalMem = iServer->totalGPUMemory();
        TPckg<unsigned long> out(totalMem);
        aMessage.WriteL(1, out);
        break;
    }
    case ELastAddedItemMem: {
        TInt lAddItemMem = iServer->lastAddedItemMem();
        TPckg<TInt> out(lAddItemMem);
        aMessage.WriteL(1, out);
        break;
    }
    case ELastRemovedItemMem: {
        TInt lRemItemMem = iServer->lastRemovedItemMem();
        TPckg<TInt> out(lRemItemMem);
        aMessage.WriteL(1, out);
        break;
    }
    case ELastAddedItemRefCount: {
        TInt lAddItemRfCnt = iServer->lastAddedRefCount();
        TPckg<TInt> out(lAddItemRfCnt);
        aMessage.WriteL(1, out);
        break;
    }
    case EEnableCache: {
        TBool params = 0;
        TPckg<TBool> paramPckg(params);
        aMessage.ReadL(0, paramPckg, 0);
        TBool success = iServer->enableCache(params);
        TPckg<TBool> out(success);
        aMessage.WriteL(1, out);
        break;
    }
    case ECacheHit: {
        int cacheHitCnt = iServer->cacheHitCount();
        TPckg<TInt> out(cacheHitCnt);
        aMessage.WriteL(1, out);
        break;
    }
    case ECacheMiss: {
        int cacheMissCnt = iServer->cacheMissCount();
        TPckg<TInt> out(cacheMissCnt);
        aMessage.WriteL(1, out);
        break;
    }
    case ECleanRasterLRUList: {
        iServer->cleanRasterLRUList();
        break;
    }
    case ECleanVectorLRUList: {
        iServer->cleanVectorLRUList();
        break;
    }
    case EGpuLruCount: {
        TInt rasterMissCount = iServer->rasterLruCount();
        TPckg<TInt> out(rasterMissCount);
        aMessage.WriteL(1, out);
        break;
    }
    case ECpuLruCount: {
        TInt vectorMissCount = iServer->vectorLruCount();
        TPckg<TInt> out(vectorMissCount);
        aMessage.WriteL(1, out);
        break;
    }
    case EGPULRUSize: {
        TInt gpuLRUSize = iServer->gpuLRUSize();
        TPckg<TInt> out(gpuLRUSize);
        aMessage.WriteL(1, out);
        break;
    }
    case ERefCount: {
        TInt refCount = iServer->iconRefCount(aMessage);
        TPckg<TInt> out(refCount);
        aMessage.WriteL(1, out);
        break;
    }
    case EServerHeap: {
        TInt heapSize = iServer->serverHeapSize();
        TPckg<TInt> out(heapSize);
        aMessage.WriteL(1, out);
        break;
    }

    case ELastRemovedItemRefCount: {
        TInt lastItemRemovedRefCount = iServer->lastRemovedItemRfCount();
        TPckg<TInt> out(lastItemRemovedRefCount);
        aMessage.WriteL(1, out);
        break;
    }

    case EServerHeapMarkStart: {
        __UHEAP_MARK;
        break;
    }

    case EServerHeapMarkEnd: {
        __UHEAP_MARKEND;
        break;
    }

    case EServerAllocFail: {
        //TODO
        break;
    }

    case EServerAllocReset: {
        //TODO
        break;
    }
#endif
    case EMemoryGood: {

        iServer->MemoryGood();
        break;
    }
    case EFreeRam: {
        TInt params = 0;
        TPckg<TInt> paramPckg(params);
        aMessage.ReadL(0, paramPckg, 0);

        iServer->FreeGpuRam(params);
        break;
    }

        case EThemeIndex: {
            // Using leave here because this function does not return a value
            ThemeIndexTables tables;
            iServer->getThemeIndexTables(tables);
            TPckg<ThemeIndexTables> out(tables);
            aMessage.WriteL(0, out);
            break;
        }

    // This is an example of a request that we know about, but don't support.
    // We cause KErrNotSupported to be returned to the client.
    default:
        PanicClient(aMessage, EBadRequest);
        break;
    }
#ifdef THEME_SERVER_TRACES
    qDebug() << "Leave HbThemeServerSession::DispatchMessageL";
#endif

}

/**
 * HandleStyleSheetLookupL
 */
void HbThemeServerSession::HandleStyleSheetLookupL(const RMessage2& aMessage)
{
    if (aMessage.GetDesLength(0) == 0) {
#ifdef THEME_SERVER_TRACES
        qDebug() << "Empty Filename";
#endif
        return;
    }

    TBuf<256> fileName;
    aMessage.ReadL(0, fileName, 0);
    TBuf<256> layerPriorityBuf;
    aMessage.ReadL(1, layerPriorityBuf, 0);
    TLex lex(layerPriorityBuf);
    TInt priorityValue;
    lex.Val(priorityValue);

    HbLayeredStyleLoader::LayerPriority layerPriority = (HbLayeredStyleLoader::LayerPriority) priorityValue;

    QString cssFileName((QChar*)fileName.Ptr(), fileName.Length());
    HbSharedStyleSheetInfo offsetInfo;
    HbCacheItem* cssCacheItem = iServer->cssCacheItem(cssFileName);
    bool insertKeyIntoSessionList = false;
    if (cssCacheItem) {
        //The item was found in the cache and reference count was incremented
        insertKeyIntoSessionList = true;
        offsetInfo.offset = cssCacheItem->offset;
    } else {
        bool tryAgain = false;
        do {
            offsetInfo.offset = HbThemeServerUtils::getSharedStylesheet(cssFileName, layerPriority);
            if (offsetInfo.offset >= 0) {
                HbCacheItem *cssItem =  new HbCacheItem(offsetInfo.offset, 0, cssFileName);
                insertKeyIntoSessionList = iServer->insertCssCacheItem(cssFileName, cssItem);
                if (layerPriority == HbLayeredStyleLoader::Priority_Core && cssItem->refCount == 1) {
                    // This will make sure the requested stylesheet will always remain
                    // in the primary and secondary cache.
                    cssItem->incrementRefCount();
                }
                break;
            } else if (offsetInfo.offset == OUT_OF_MEMORY_ERROR && tryAgain == false) {
                iServer->doCleanup();
                tryAgain = true;
            } else if (offsetInfo.offset == OUT_OF_MEMORY_ERROR && tryAgain == true) {
                //try only once to free up memory, else offset remains -2
                tryAgain = false;
            }
        } while (tryAgain);
    }
    if (insertKeyIntoSessionList) {
        //The session will only keep track of cssFiles that were either successfully found or were
        //successfully inserted in the cache.
        sessionCssData.append(cssFileName);
    }
    TPckg<HbSharedStyleSheetInfo> data(offsetInfo);
    aMessage.WriteL(2, data);
}

/**
 * HandleWidgetMLLookUp
 */
void HbThemeServerSession::HandleWidgetMLLookupL(const RMessage2& aMessage)
{
    if (aMessage.GetDesLength(0) == 0) {
        return;
    }

    TBuf<256> fileName;
    aMessage.ReadL(0, fileName, 0);
    TBuf<256> layoutName;
    aMessage.ReadL(1, layoutName, 0);
    TBuf<256> sectionName;
    aMessage.ReadL(2, sectionName, 0);

    QString wmlFileName((QChar*)fileName.Ptr(), fileName.Length());
    QString layout((QChar*)layoutName.Ptr(), layoutName.Length());
    QString section((QChar*)sectionName.Ptr(), sectionName.Length());

    HbSharedWMLInfo offsetInfo;
    offsetInfo.offset = HbThemeServerUtils::getSharedLayoutDefinition(wmlFileName, layout, section);
    TPckg<HbSharedWMLInfo> data(offsetInfo);
    aMessage.WriteL(3, data);
}


/**
 * Handle DeviceProfiles Request.
 */
void HbThemeServerSession::HandleDeviceProfilesReqL(const RMessage2& aMessage)
{
    HbDeviceProfileInfo offsetInfo;
    HbDeviceProfileDatabase *deviceProfileDatabase = HbDeviceProfileDatabase::instance(HbMemoryManager::SharedMemory);
    if (deviceProfileDatabase) {
        offsetInfo.offset = deviceProfileDatabase->deviceProfilesOffset();
    } else {
        offsetInfo.offset = -1;
    }
    TPckg<HbDeviceProfileInfo> data(offsetInfo);
    aMessage.WriteL(0, data);
}

/**
 * Handle SecondaryCacheOffset Request.
 */
void HbThemeServerSession::HandleSecondaryCacheOffsetReqL(const RMessage2& aMessage)
{
    HbSecondaryCacheInfo offsetInfo;
    offsetInfo.offset = HbThemeServerUtils::sharedCacheOffset();
    TPckg<HbSecondaryCacheInfo> data(offsetInfo);
    aMessage.WriteL(0, data);
}

/**
 * HandleEffectAddAndFileLookupL
 */
void HbThemeServerSession::HandleEffectAddAndFileLookupL(const RMessage2& aMessage)
{
    TInt fileNameLength = aMessage.GetDesLength(0);
    if (fileNameLength == 0) {
        return;
    }

    TFileName fileName;
    aMessage.ReadL(0, fileName, 0);
    QString effFileName((QChar*)fileName.Ptr(), fileName.Length());
    HbSharedEffectInfo offsetInfo;
    offsetInfo.offset = HbThemeServerUtils::getSharedEffect(effFileName);
    TPckg<HbSharedEffectInfo> data(offsetInfo);
    aMessage.WriteL(1, data);
}

void HbThemeServerSession::GetSharedIconDefaultSizeInfoL(const RMessage2 &aMessage)
{
    TIconParams params = ReadMessageAndRetrieveParams(aMessage);

    // Need to be allocated from heap or the leave in the end causes a crash
    QScopedPointer<QString> filenamePtr(new QString((QChar*)params.fileName.Ptr(), params.fileName.Length()));
    
    // See if the icon's default size has been queried already earlier and
    // can be found stored in the hash.

    QSizeF defSize = iServer->iconDefaultSizeCacheItem(*filenamePtr.data());
                    
    if (!defSize.isValid()) {
        defSize = RetrieveIconDefaultSize(*filenamePtr.data());

        // If the default size was retrieved, insert it in the hash for further lookups
        if (defSize.isValid()) {
            iServer->insertIconDefaultSizeCacheItem(*filenamePtr.data(), defSize);
        }
    }
    
    // Return the default size back to the client if it was resolved
    if (defSize.isValid()) {    
        TPckg<QSizeF> returnData(defSize);
        aMessage.WriteL(1, returnData);
#ifdef THEME_SERVER_TRACES 
        qDebug() << "Completed aMessage.WriteL";
#endif
    }
    // Otherwise leave with error code
    else {
        User::Leave(KErrNotFound);
    }
}

QSizeF HbThemeServerSession::RetrieveIconDefaultSize(const QString &filename)
{
    QSizeF ret;

    // Get icon source, previous icons sources are cached so if accessed again,
    // they don't need to be loaded and parsed from a file always.
    HbIconSource *source = HbThemeServerUtils::getIconSource(filename);
    if (source) {
        ret = source->defaultSize();
    }

    return ret;
}

/**
 * GetSharedIconInfoL
 */
void HbThemeServerSession::GetSharedIconInfoL(const RMessage2& aMessage)
{
    HbSharedIconInfo data;
    TIconParams params = ReadMessageAndRetrieveParams(aMessage);

    QString filename((QChar*)params.fileName.Ptr(), params.fileName.Length());
    QColor color = GetColorFromRgba(params.rgba, params.colorflag);
    HbIconKey key(filename, QSizeF(params.width, params.height),
                  (Qt::AspectRatioMode)params.aspectRatioMode,
                  (QIcon::Mode)params.mode, params.mirrored, color);

    HbIconCacheItem* cacheItem = iServer->iconCacheItem(key);
    bool insertKeyIntoSessionList = false;
    if (cacheItem) {
        insertKeyIntoSessionList = true; //The item was found in the cache and ref count was incremented
        if (cacheItem->rasterIconData.type != INVALID_FORMAT) {
            data = cacheItem->rasterIconData;
        } else if (cacheItem->vectorIconData.type != INVALID_FORMAT) {
            data = cacheItem->vectorIconData;
        } else if (cacheItem->blobIconData.type != INVALID_FORMAT) {
            data = cacheItem->blobIconData;
        } else {
            data.type = INVALID_FORMAT;
        }
    } else {
        QT_TRY {
            QString format = HbThemeServerUtils::formatFromPath(key.filename);
            QScopedPointer <HbIconCacheItem> tempIconCacheItem(HbIconCacheItemCreator::createCacheItem(key, (HbIconLoader::IconLoaderOptions)params.options, format));
            cacheItem = tempIconCacheItem.data();
            if (cacheItem) {
                if (cacheItem->rasterIconData.type != INVALID_FORMAT) {
                    data = cacheItem->rasterIconData;
                } else if (cacheItem->vectorIconData.type != INVALID_FORMAT) {
                    data = cacheItem->vectorIconData;
                } else if (cacheItem->blobIconData.type != INVALID_FORMAT) {
                    data = cacheItem->blobIconData;
                } else {
                    data.type = INVALID_FORMAT;
                }
                if (data.type != INVALID_FORMAT) {
                    insertKeyIntoSessionList = iServer->insertIconCacheItem(key, cacheItem);
                    if (!insertKeyIntoSessionList) {
                        FreeDataFromCacheItem(cacheItem);
                        tempIconCacheItem.take();
                        delete cacheItem; // do delete the item after gpu/cpu memory is freed
                        data.type = INVALID_FORMAT;
                    }
                }
            }
            if (tempIconCacheItem.data()) {
                tempIconCacheItem.take();
            }
        } QT_CATCH(const std::bad_alloc &) {
            data.type = INVALID_FORMAT;
            FreeDataFromCacheItem(cacheItem);
        }
    }
    if (insertKeyIntoSessionList) {
        //The session will only keep track of icons that were either successfully found or were
        //successfully inserted in the cache.
        sessionData.append(key);
    }
    // create dshared pixmap info from HbIconCacheItem
    TPckg<HbSharedIconInfo> pixdata(data);
    aMessage.WriteL(1, pixdata);

#ifdef THEME_SERVER_TRACES
    qDebug() << "Completed  aMessage.WriteL";
#endif
}

/**
 * handleThemeSelectionL
 */
void HbThemeServerSession::HandleThemeSelectionL(const RMessage2& aMessage)
{
    TInt deslen = aMessage.GetDesLength(0);
    RBuf buffer;
    buffer.CreateL(deslen);
    buffer.CleanupClosePushL();
    aMessage.ReadL(0, buffer, 0);
    if (buffer.Length() == 0) {
        User::Leave(ENonNumericString);
    }
    QString newTheme((QChar*)buffer.Ptr(), buffer.Length());
    CleanupStack::PopAndDestroy(); // close the buffer

    QString cleanThemeName = newTheme.trimmed();    
    
    if (cleanThemeName == iServer->iCurrentThemeName) {
        // Theme did not change, return.
        return;
    }
    // ToDo: Validate the theme name somehow

    #ifdef THEME_INDEX_TRACES
    qDebug() << "ThemeIndex: theme change request, new theme =" << cleanThemeName.toUtf8();
    #endif

    iServer->iCurrentThemeName = cleanThemeName;
    
    // Resolve the drive letter of the current theme
    iServer->resolveCurrentThemeDrive();

    iServer->currentIndexfile.close();
    // Open index file to prevent uninstallation of the active theme
    iServer->openCurrentIndexFile();

    QSettings settings(QLatin1String(ORGANIZATION), QLatin1String(THEME_COMPONENT));    
    settings.setValue("currenttheme", cleanThemeName); 
    settings.sync();
    TPtrC name(reinterpret_cast<const TUint16 *>(cleanThemeName.constData()));
    TInt err = iServer->iThemeProperty.Set(name);
    User::LeaveIfError(err);
}

/**
 * Panics the client
 */
void HbThemeServerSession::PanicClient(const RMessage2& aMessage, TInt aPanic) const
{
    _LIT(KTxtServer, "Theme server");
    aMessage.Panic(KTxtServer, aPanic);
}

QColor HbThemeServerSession::GetColorFromRgba(TUint32 aRgba, bool aColorFlag)
{
    QColor color;
    if (aColorFlag) {
        color.setRgba((QRgb)aRgba);
    }
    return color;
}

/**
 * HbThemeServerPrivate::GetSharedMultiIconInfoL
  Creates a consolidated icon of the frame item pieces ,
  if failed to do so creates a icons of the pieces provided
  there is enough space in the cache and shared memory.
 */
void HbThemeServerSession::GetSharedMultiIconInfoL(const RMessage2& aMessage)
{
    HbSharedIconInfo stitchedData;
    stitchedData.type = INVALID_FORMAT;
    QVector<HbIconKey> insertedKeys;
    TMultiIconSymbParams params = ReadMessageAndRetrieveMultiIconParams(aMessage);

    QColor color = GetColorFromRgba(params.rgba, params.colorflag);
    QString iconId((QChar*)params.multiPartIconId.Ptr(), params.multiPartIconId.Length());
    QString fullPath((QChar*)params.multiPartIconList[0].Ptr(), params.multiPartIconList[0].Length());
    int index = fullPath.lastIndexOf("/");
    fullPath = fullPath.left(index + 1);
    iconId.prepend(fullPath);
    HbIconKey finalIconKey(iconId,
                           (QSizeF)params.size,
                           (Qt::AspectRatioMode)params.aspectRatioMode,
                           (QIcon::Mode)params.mode,
                           (bool)params.mirrored,
                           color);

    if (!IconInfoFromSingleIcon(finalIconKey, stitchedData)) {
        HbMultiIconParams frameItemParams;
        int noOfPieces = 1;
        if (iconId.contains("_3PV", Qt::CaseInsensitive) || iconId.contains("_3PH", Qt::CaseInsensitive)) {
            noOfPieces = 3;
        } else if (iconId.contains("_9P", Qt::CaseInsensitive)) {
            noOfPieces = 9;
        }

        frameItemParams.multiPartIconId = iconId;
        frameItemParams.aspectRatioMode = (Qt::AspectRatioMode)params.aspectRatioMode;
        frameItemParams.colorflag = params.colorflag;
        frameItemParams.mirrored = (bool)params.mirrored;
        frameItemParams.options = params.options;
        frameItemParams.rgba = params.rgba;
        frameItemParams.mode = (QIcon::Mode)params.mode;
        frameItemParams.size = (QSizeF)params.size;
        frameItemParams.color = color;
        QT_TRY {
            for (int i = 0; i < noOfPieces; i++) {
                frameItemParams.multiPartIconData.pixmapSizes[i] = (QSize &)params.pixmapSizes[i];
                frameItemParams.multiPartIconData.targets[i] = (QRect &)params.targets[i];
                frameItemParams.multiPartIconData.sources[i] = (QRect &)params.sources[i];
                QString pieceName((QChar*)params.multiPartIconList[i].Ptr(), params.multiPartIconList[i].Length());
                frameItemParams.multiPartIconList.append(pieceName);
            }

            IconInfoFromMultiParts(frameItemParams, noOfPieces, finalIconKey, stitchedData);
        } QT_CATCH(const std::bad_alloc &) {
            stitchedData.type = INVALID_FORMAT;
        }
    }

    // create dshared pixmap info from HbIconCacheItem
    TPckg<HbSharedIconInfo> pixdata(stitchedData);
    aMessage.WriteL(1, pixdata);
}

/**
 * HbThemeServerPrivate::IconInfoFromSingleIcon
  Checks for the cacheItem for a given key, if found gets the data relevant of the cacheItem.
 */

bool HbThemeServerSession::IconInfoFromSingleIcon(HbIconKey key,
        HbSharedIconInfo &stitchedData)
{
    stitchedData.type = INVALID_FORMAT;
    HbIconCacheItem * cacheItem = iServer->iconCacheItem(key, true);
    if (cacheItem) {
        GetDataFromCacheItem(cacheItem, stitchedData);
        return true; //The item was found in the cache and ref count was incremented
    }
    return false;
}

/**
 * HbThemeServerPrivate::createCacheItemData
  Creates a cacheItem of the given key and insert the item in to the list
  else free the data allocated for the cache.
 */

bool HbThemeServerSession::CreateCacheItemData(HbIconKey key, int options , HbSharedIconInfo &data, bool isMultiIcon)
{
    bool insertKeyIntoSessionList = false;
    data.type = INVALID_FORMAT;
    QString format = HbThemeServerUtils::formatFromPath(key.filename);
    HbIconCacheItem * cacheItemOfPiece = iServer->iconCacheItem(key, isMultiIcon);
    if (cacheItemOfPiece) {
        GetDataFromCacheItem(cacheItemOfPiece, data);
        insertKeyIntoSessionList = true;
    } else {
        cacheItemOfPiece = HbIconCacheItemCreator::createCacheItem(key,
                           (HbIconLoader::IconLoaderOptions)options,
                           format,
                           isMultiIcon);
        if (cacheItemOfPiece) {
            GetDataFromCacheItem(cacheItemOfPiece, data);
            if (data.type != INVALID_FORMAT) {
                insertKeyIntoSessionList = iServer->insertIconCacheItem(key, cacheItemOfPiece);
                if (!insertKeyIntoSessionList) {
                    //if insertion failed free the memory
                    FreeDataFromCacheItem(cacheItemOfPiece);
                    delete cacheItemOfPiece; // do delete the item after gpu/cpu memory is freed
                    data.type = INVALID_FORMAT;
                }
            }
        }
    }
    return insertKeyIntoSessionList;
}

/**
 * HbThemeServerPrivate::CreateStichedIconInfoOfParts
  Creates a consolidated icon of the available piece iconInfo.
 */

bool HbThemeServerSession::CreateStichedIconInfoOfParts(QVector<HbSharedIconInfo> dataForParts,
        HbMultiIconParams params,
        HbIconKey &finalIconKey,
        HbSharedIconInfo &stitchedData,
        bool allNvg)
{
    bool insertKeyIntoSessionList = false;
    stitchedData.type = INVALID_FORMAT;
    QString format = HbThemeServerUtils::formatFromPath(params.multiPartIconList[0]);

    QScopedPointer<HbIconCacheItem> tempCacheItem(HbIconCacheItemCreator::createMultiPieceCacheItem(finalIconKey, (HbIconLoader::IconLoaderOptions)params.options, format, dataForParts, params, allNvg));
    HbIconCacheItem * cacheItem = tempCacheItem.data();
    if (cacheItem) {
        if (cacheItem->rasterIconData.type == INVALID_FORMAT) {
            return false;
        }
    }

    stitchedData = cacheItem->rasterIconData;
    if (stitchedData.type != INVALID_FORMAT) {
        insertKeyIntoSessionList = iServer->insertIconCacheItem(finalIconKey, cacheItem);
        if (!insertKeyIntoSessionList) {
            //if insertion failed free the memory
            FreeDataFromCacheItem(cacheItem);
            tempCacheItem.take();
            delete cacheItem; // do delete the item after gpu/cpu memory is freed
            stitchedData.type = INVALID_FORMAT;
        }
    }
    if (tempCacheItem.data()) {
        tempCacheItem.take();
    }
    return insertKeyIntoSessionList;
}

/**
 * HbThemeServerPrivate::iconInfoFromMultiParts
  Creates a shared IconInfo of the piece files of a frame item and
  tries to create a stiched icon of the same.
 */

void HbThemeServerSession::IconInfoFromMultiParts(const HbMultiIconParams &frameItemParams,
        const int noOfPieces,
        HbIconKey &stichedKey,
        HbSharedIconInfo &stitchedData)
{
    QVector<HbIconKey> keysInserted;
    QVector<HbSharedIconInfo> dataForParts;
    bool insertKeyIntoSessionList = false;
    bool failedToCreateParts = false;
    QString format;

    int nvgPiecesCount = 0;

    for (int i = 0; i < noOfPieces; i++) {
        HbSharedIconInfo data;
        bool iconPieceMirrored = false;
        HbIconKey key(frameItemParams.multiPartIconList.at(i), frameItemParams.multiPartIconData.pixmapSizes[i], (Qt::AspectRatioMode)stichedKey.aspectRatioMode, (QIcon::Mode)stichedKey.mode, iconPieceMirrored, stichedKey.color);
        insertKeyIntoSessionList = IconInfoFromSingleIcon(key, data);
        if (!insertKeyIntoSessionList) {
            insertKeyIntoSessionList = CreateCacheItemData(key, frameItemParams.options, data, true);
        }
        if ((data.type == INVALID_FORMAT) || (!insertKeyIntoSessionList)) {
            failedToCreateParts = true;
            break;
        } else {
            if (data.type == NVG) {
                ++nvgPiecesCount;
            }

            //The session will only keep track of icons that were either successfully found or were
            //successfully inserted in the cache.
            keysInserted.append(key);
            dataForParts.append(data);
        }
    }//end of for
    bool allNvg = false;

    if (nvgPiecesCount == noOfPieces) {
#ifdef HB_SGIMAGE_ICON
        // all individual icons are NVGs, can create sgImage
        allNvg = true;
#else
        // All pieces are NVGs but HB_SGIMAGE_ICON not defined so don't
        // consolidate the icon.
        failedToCreateParts = true;
#endif

    } else if (nvgPiecesCount == 0) {
        // for pixmap case, sgImage is not required at all
        allNvg = false;
    } else {
        // Not all individual icons are of the same format, sgImage can't be created
        failedToCreateParts =  true;
    }

    int dataPartCount = dataForParts.count();
    if ((failedToCreateParts) || (dataPartCount != noOfPieces) || (!insertKeyIntoSessionList)) {
        //atLeast one of the icon did'nt get constructed , so move the cached piece icons to unused state and return
        QVector<HbIconKey>::const_iterator itEnd(keysInserted.constEnd());
        for (QVector<HbIconKey>::const_iterator iter = keysInserted.constBegin();
                iter != itEnd;
                ++iter) {
            iServer->CleanupSessionIconItem(*iter);
        }
        stitchedData.type = INVALID_FORMAT;
        return;
    }
// Create a stitched icon of the available piece shared iconinfos
    if ((dataPartCount == noOfPieces) && (!failedToCreateParts) &&
            (CreateStichedIconInfoOfParts(dataForParts, frameItemParams, stichedKey, stitchedData, allNvg))) {
        sessionData.append(stichedKey);
    }

    QVector<HbIconKey>::const_iterator itEnd(keysInserted.constEnd());
    for (QVector<HbIconKey>::const_iterator iter = keysInserted.constBegin();
            iter != itEnd;
            ++iter) {
        iServer->CleanupSessionIconItem(*iter);
    }
}

/**
 * HbThemeServerPrivate::FreeDataFromCacheItem
 *Frees data from the cached item when insertion to the list fails.
 */
void HbThemeServerSession::FreeDataFromCacheItem(HbIconCacheItem* cacheItem)
{
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory)
    if (cacheItem->rasterIconData.type != INVALID_FORMAT) {
        switch (cacheItem->rasterIconData.type) {
        case PIC :
            manager->free(cacheItem->rasterIconData.picData.offset);
            break;
        case NVG :
            manager->free(cacheItem->rasterIconData.nvgData.offset);
            break;
        case OTHER_SUPPORTED_FORMATS :
            manager->free(cacheItem->rasterIconData.pixmapData.offset);
            break;
        default:
            break;
        }
    }
    if (cacheItem->vectorIconData.type != INVALID_FORMAT) {
        switch (cacheItem->vectorIconData.type) {
        case PIC :
            manager->free(cacheItem->vectorIconData.picData.offset);
            break;
        case NVG :
            manager->free(cacheItem->vectorIconData.nvgData.offset);
            break;
        case OTHER_SUPPORTED_FORMATS :
            manager->free(cacheItem->vectorIconData.pixmapData.offset);
            break;
        default:
            break;
        }
    }
    if (cacheItem->blobIconData.type == BLOB) {
        manager->free(cacheItem->blobIconData.blobData.offset);
    }
}

/**
 * HbThemeServerPrivate::GetDataFromCacheItem
 * Gets data from the cache Item.
 */

// Code to get data from the cached item
void HbThemeServerSession::GetDataFromCacheItem(HbIconCacheItem* cacheItem, HbSharedIconInfo &data) const
{
    if (cacheItem) {
        if (cacheItem->rasterIconData.type != INVALID_FORMAT) {
            data = cacheItem->rasterIconData;
        } else if (cacheItem->vectorIconData.type != INVALID_FORMAT) {
            data = cacheItem->vectorIconData;
        } else if (cacheItem->blobIconData.type != INVALID_FORMAT) {
            data = cacheItem->blobIconData;
        } else {
            data.type = INVALID_FORMAT;
        }
    }
}

/**
 * HbThemeServerPrivate::ReadMessageAndRetrieveMultiIconParams
 *
 * Reads all the information from client.
 */

TMultiIconSymbParams HbThemeServerSession::ReadMessageAndRetrieveMultiIconParams(const RMessage2& aMessage)
{
    TInt deslen = aMessage.GetDesLength(0);
    TMultiIconSymbParams params;
    TPckg<TMultiIconSymbParams> paramPckg(params);
    // Copy the client's descriptor data into our buffer.
    aMessage.ReadL(0, paramPckg, 0);
    return params;
}

/**
 * HbThemeServerSymbian::GetMultiIconInfoL
 *
 * Creates a individual icons of the frame item pieces,provided
 * there is enough space in the cache and shared memory..
 */
void HbThemeServerSession::GetMultiIconInfoL(const RMessage2& aMessage)
{
    HbSharedIconInfoList iconInfoList;

    TInt deslen = aMessage.GetDesLength(0);
    TIconListParams params;
    TPckg<TIconListParams> paramPckg(params);
    // Copy the client's descriptor data into our buffer.
    aMessage.ReadL(0, paramPckg, 0);

    //Populate the iconInfoList with the list of sharedinfo for all the frame parts.
    IconInfoL(params, params.iconCount, iconInfoList);

    // create dshared pixmap info from HbIconCacheItem
    TPckg<HbSharedIconInfoList> pixdata(iconInfoList);
    aMessage.WriteL(1, pixdata);
}

/**
 * HbThemeServerSymbian::IconInfoL()
 */
void HbThemeServerSession::IconInfoL(const TIconListParams &frameItemParams,
                                     const int noOfPieces,
                                     HbSharedIconInfoList &iconInfoList)
{
    QVector<HbIconKey> keysInserted;

    bool insertKeyIntoSessionList = false;
    QString format;
    QColor color = GetColorFromRgba(frameItemParams.rgba, frameItemParams.colorflag);
    for (int i = 0; i < noOfPieces; i++) {
        HbSharedIconInfo data;
        QString pieceName((QChar*)frameItemParams.iconList[i].Ptr(), frameItemParams.iconList[i].Length());
        HbIconKey key(pieceName, frameItemParams.sizeList[i],
                      (Qt::AspectRatioMode)frameItemParams.aspectRatioMode, (QIcon::Mode)frameItemParams.mode,
                      (bool)frameItemParams.mirrored, color);
        insertKeyIntoSessionList = IconInfoFromSingleIcon(key, data);
        if (!insertKeyIntoSessionList) {
            insertKeyIntoSessionList = CreateCacheItemData(key, 0, data, false);
        }
        if ((data.type != INVALID_FORMAT) || (insertKeyIntoSessionList)) {
            //The session will only keep track of icons that were either successfully found or were
            //successfully inserted in the cache.
            keysInserted.append(key);
            iconInfoList.icon[i] = data;
        }
    }//end of for
}

void HbThemeServerSession::unLoadIcon(const RMessage2& aMessage)
{
    TIconParams params = ReadMessageAndRetrieveParams(aMessage);
    QString filename((QChar*)params.fileName.Ptr(), params.fileName.Length());
    QColor color = GetColorFromRgba(params.rgba, params.colorflag);
    HbIconKey key(filename, QSizeF(params.width, params.height),
                  (Qt::AspectRatioMode)params.aspectRatioMode,
                  (QIcon::Mode)params.mode, params.mirrored, color);
    iServer->CleanupSessionIconItem(key);
    sessionData.removeOne(key);
}

void HbThemeServerSession::unloadMultiIcon(const RMessage2& aMessage)
{
    TIconListParams params;

    TInt deslen = aMessage.GetDesLength(0);
    TPckg<TIconListParams> paramPckg(params);
    // Copy the client's descriptor data into our buffer.
    aMessage.ReadL(0, paramPckg, 0);

    QColor color = GetColorFromRgba(params.rgba, params.colorflag);
    Qt::AspectRatioMode aspectRatioMode = (Qt::AspectRatioMode)params.aspectRatioMode;
    QIcon::Mode mode = (QIcon::Mode)params.mode;
    TBool mirrored = params.mirrored;

    TInt iconCount = params.iconCount;
    for (int i = 0; i < iconCount; i++) {
        QString filename((QChar*)params.iconList[i].Ptr(), params.iconList[i].Length());
        HbIconKey key(filename, params.sizeList[i], aspectRatioMode, mode, mirrored, color);
        iServer->CleanupSessionIconItem(key);
        sessionData.removeOne(key);
    }
}

#ifdef HB_ICON_CACHE_DEBUG
/**
 * iconRefCount
 */
#if defined (Q_OS_SYMBIAN)
int HbThemeServerPrivate::iconRefCount(const RMessage2& aMessage)
{
    HbSharedIconInfo data;
    int refCount;
    TIconParams params;
    TPckg<TIconParams> paramPckg(params);
    // Copy the client's descriptor data into our buffer.
    aMessage.ReadL(0, paramPckg, 0);

    QString filename((QChar*)params.fileName.Ptr(), params.fileName.Length());
    QColor color;
    if (params.colorflag) {
        color.setRgba((QRgb)params.rgba);
    }
    
    HbIconKey key(filename, QSizeF(params.width, params.height),
                  (Qt::AspectRatioMode)params.aspectRatioMode,
                  (QIcon::Mode)params.mode, params.mirrored, color);

    HbIconCacheItem* cacheItem = cache->value(key);//iconCacheItem(key);
    if(cacheItem)
        refCount =  cacheItem->refCount;
    else
        refCount = 0;
    return refCount;
}
#endif

#endif
