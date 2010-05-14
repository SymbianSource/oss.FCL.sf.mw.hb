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
#include "hbsharedmemorymanager_p.h"
#include "hbtypefaceinfodatabase_p.h"


#include <QHash>
#include <QImage>
#include <QSharedMemory>
#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QPainter>
#include <QSettings>
#include <QSizeF>
#include <e32property.h>
#include <e32base.h>
#include <e32svr.h>
#include <centralrepository.h>
#include "hbpixmapiconprocessor_p.h"
#include "hbpixmapiconimpl_p.h"
#include "hblayeredstyleloader_p.h"
#include "hbmemoryutils_p.h"
#include "hbdeviceprofiledatabase_p.h"
#include "hbthemesystemeffect_p.h"
#ifdef HB_SGIMAGE_ICON
#include "hbsgimagerenderer_p.h"
#endif
// 5 MB GPU cache size
#define GPU_CACHE_SIZE 0x500000

// 5 MB  CPU cache size
#define CPU_CACHE_SIZE 0x500000

static const TInt KThemeName = 0;

// Publish/Subscribe themeRequestProp specific
static _LIT_SECURITY_POLICY_PASS(KAllowAllPolicy);
static _LIT_SECURITY_POLICY_C1(KThemeChangerPolicy,ECapabilityWriteDeviceData);


const QString operatorCPath = "C:/resource/hb/operatorTheme/icons/";
const QString operatorZPath = "Z:/resource/hb/operatorTheme/icons/";

static HbThemeServerPrivate *TheServer = 0;
bool HbThemeServerPrivate::gpuGoodMemoryState = true;
// This is used as parent theme always regardless of the active theme

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

    // Store the active theme name in a member string
    iCurrentThemeName = HbThemeUtils::getThemeSetting(HbThemeUtils::CurrentThemeSetting);    

    if (iCurrentThemeName.isEmpty()) {
        iCurrentThemeName = HbThemeUtils::defaultTheme().name;
        // Set the current theme also in the cenrep key that is used to notify clients.
        HbThemeUtils::setThemeSetting(HbThemeUtils::CurrentThemeSetting, iCurrentThemeName);
    }

    // Cache ROM theme(name)s
    QString romPath = "Z:\\resource\\hb\\themes\\icons\\";
    QDir dir(romPath);
    romThemeNames = dir.entryList(QDir::Dirs);
    
    // Resolve the path of the current theme
    resolveThemePath(iCurrentThemeName, iCurrentThemePath);
    
    // Register theme system effects in construction
    // TODO: fix parameter
    HbThemeSystemEffect::handleThemeChange(iCurrentThemeName);

    // Open index file to prevent uninstallation of the active theme
    openCurrentIndexFile();
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
#if defined(HB_SGIMAGE_ICON) || defined(HB_NVG_CS_ICON)    
    renderMode = EHWRendering;
#else
    renderMode = ESWRendering;
#endif
    
    // Process base theme index, it is used as parent index also when the current theme is something else
    QString basePath;
    resolveThemePath(HbThemeUtils::getThemeSetting(HbThemeUtils::BaseThemeSetting), basePath);
    createThemeIndex(basePath, BaseTheme);
    // Process operator Drive C theme index
    QString operatorName = HbThemeUtils::getThemeSetting(HbThemeUtils::OperatorNameSetting);
    if (!operatorName.isEmpty()) {
        QString operatorPath;
        operatorPath.append(operatorCPath);
        operatorPath.append(operatorName);
        createThemeIndex(operatorPath, OperatorC);
        // Process operator Drive Z theme index
        QString operatorROMPath;
        operatorROMPath.append(operatorZPath);    
        operatorROMPath.append(operatorName);
        createThemeIndex(operatorROMPath, OperatorROM);
    }
    // Process current theme index
    createThemeIndex(iCurrentThemePath, ActiveTheme);
	
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
    : CServer2( aActiveObjectPriority )
{
    // Set server pointer in static variable
    TheServer = this;

    // Set up the listener to listen for Publish events
    TRAPD(err, iListener = CHbThemeChangeNotificationListener::NewL(*this));
    if (err) {
        qWarning( "HbThemeServerPrivate::HbThemeServerPrivate: CHbThemeChangeNotificationListener::NewL failed = %d", err );
    } else {
        TRAPD( err, iListener->startListening());
        if (err) {
             qWarning( "HbThemeServerPrivate::HbThemeServerPrivate: iListener->startListening failed = %d", err );
         }
    }
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
    TInt err = RProperty::Delete(KServerUid3, KNewThemeForThemeChanger);
    if(err != KErrNotFound) {
        User::LeaveIfError(err);
    }
    // Delete the listener for Publish/Subscribe
    delete iListener;
}

HbThemeServerPrivate *HbThemeServerPrivate::Instance()
{
    return TheServer;
}

void HbThemeServerPrivate::openCurrentIndexFile()
{
    // Open index file to prevent uninstallation of the active theme
    if (!iCurrentThemePath.isEmpty() && iCurrentThemePath[0] != 'Z') {
        QString indexFileName;
        indexFileName.append(iCurrentThemePath);
        indexFileName.append("\\index.theme");

        currentIndexfile.setFileName(indexFileName);
        if(!currentIndexfile.open(QIODevice::ReadOnly)) {
            qWarning()<< "HbSymbianThemeServer: No Index file found in the new theme, How did this happen ??";
        }
    }
}

bool HbThemeServerPrivate::resolveThemePath(const QString &themeName, QString &themePath)
{
    if(themeName == "hbdefault") {
        themePath = ":\\themes\\icons\\hbdefault";
        return true;
    }
    
    if (!themeName.isEmpty()) {
        // Check for the theme's icon directory in different drives.
        // ROM is checked first and then phone memory and memory card drives.

        QString themeLookupPath = "Z:\\resource\\hb\\themes\\icons\\";
    
        if (romThemeNames.contains(themeName)) {
            themeLookupPath.append(themeName);    
            themePath = themeLookupPath;
            return true;
        }

        themeLookupPath.append(themeName);
        QString filename(themeLookupPath);
        filename.append("\\index.theme");
        QFile file;

        filename[0] = 'C';
        file.setFileName(filename);
        if (file.open(QIODevice::ReadOnly)) {
            themeLookupPath[0] = 'C';
            themePath = themeLookupPath;
            file.close();
            return true;
        }

        filename[0] = 'E';
        file.setFileName(filename);
        if (file.open(QIODevice::ReadOnly)) {
            themeLookupPath[0] = 'E';
            themePath = themeLookupPath;
            file.close();
            return true;
        }

        filename[0] = 'F';
        file.setFileName(filename);
        if (file.open(QIODevice::ReadOnly)) {
            themeLookupPath[0] = 'F';
            themePath = themeLookupPath;
            file.close();
            return true;
        }        
        
    }
    return false;
}

/**
Handles theme selection
*/
void HbThemeServerPrivate::HandleThemeSelection( const QString& themeName)
{
    //Make a copy for ourselves
	
	
    QString cleanThemeName = themeName.trimmed();

    iCurrentThemeName = cleanThemeName;    

    QDir path(cleanThemeName);
    if (!path.isAbsolute()) {
        // Resolve the path of the current theme
        resolveThemePath(iCurrentThemeName, iCurrentThemePath);
    }
    
    #ifdef THEME_INDEX_TRACES
    qDebug() << "ThemeIndex: theme change request, new theme =" << cleanThemeName.toUtf8();
    #endif

    
    currentIndexfile.close();
    // Open index file to prevent uninstallation of the active theme
    openCurrentIndexFile();

    // Process operator Drive C theme index
    QString operatorName = HbThemeUtils::getThemeSetting(HbThemeUtils::OperatorNameSetting);
    if (!operatorName.isEmpty()) {
        QString operatorPath;
        operatorPath.append(operatorCPath);
        operatorPath.append(operatorName);
        createThemeIndex(operatorPath, OperatorC);
    }
    // Process current theme index
    createThemeIndex(iCurrentThemePath, ActiveTheme);

    // Clear cached icons and session data
    clearIconCache();
    iSessionIter.SetToFirst();
    while(iSessionIter != NULL) {
        HbThemeServerSession &session = reinterpret_cast<HbThemeServerSession &>(*iSessionIter);
        session.ClearSessionData();
        iSessionIter++;
    }
	
    // TODO: fix parameter
    HbThemeSystemEffect::handleThemeChange(cleanThemeName);
 
    // Update settings and notify clients
    HbThemeUtils::setThemeSetting(HbThemeUtils::CurrentThemeSetting,iCurrentThemeName);
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
    return(cache->getCacheItem(key, renderMode, isMultiIconPiece));
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
    if (cache) {
        cache->remove(key);
    }
}

/**
 * HbThemeServerPrivate::IconTypeInCache
 *
 * Returns the Icon Format Type i.e SGIMAGE,NVG,SVG etc...if it is present in cache
 * else returns INVALID_FORMAT
 */
 HbIconFormatType HbThemeServerPrivate::IconTypeInCache( const HbIconKey &key ) const
{
    if ( key.filename.isEmpty() || !cache->contains(key) ) {
         return INVALID_FORMAT;
    }
    HbIconCacheItem* item = cache->value(key);
    if ( item->rasterIconData.type != INVALID_FORMAT ) {
        return item->rasterIconData.type;
    } else {
        return item->vectorIconData.type;
    }
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
    if (cssCache) {
        cssCache->remove(key);
    }
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
#ifdef HB_SGIMAGE_ICON
    HbSgImageRenderer::global()->terminate();
#endif
}

/**
 *  HbThemeServerPrivate::freeUnusedGpuResources
 *  This function frees all unused SgImage icons
 *
 */

void HbThemeServerPrivate::freeUnusedGpuResources()
{
#if defined(HB_SGIMAGE_ICON) || defined(HB_NVG_CS_ICON)
#ifdef HB_ICON_CACHE_DEBUG
        HbSgImageRenderer *sgImg = HbSgImageRenderer::global();
        qDebug() << "Free GPU Memory Before clearing GPU LRU list  = " << sgImg->freeGPUMemory();
#endif    
    cache->freeUnusedGpuResources();
#ifdef HB_ICON_CACHE_DEBUG
        qDebug() << "Free GPU Memory After clearing GPU LRU list  = " << sgImg->freeGPUMemory();
#endif        
#endif
}
bool HbThemeServerPrivate::gpuMemoryState()
{
    return gpuGoodMemoryState;
}

int HbThemeServerPrivate::freeSharedMemory()
{
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    return static_cast<HbSharedMemoryManager *>(manager)->freeSharedMemory();
}

int HbThemeServerPrivate::allocatedSharedMemory()
{
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    return static_cast<HbSharedMemoryManager *>(manager)->allocatedSharedMemory();
}

int HbThemeServerPrivate::allocatedHeapMemory()
{
    TInt heapSize = 0;
    User::AllocSize(heapSize);
    return heapSize;
}
/**
 *  HbThemeServerPrivate::currentRenderingMode
 *  This function returns ThemeServer's current rendering mode
 *
 */
HbRenderingMode HbThemeServerPrivate::currentRenderingMode() const
{
    return renderMode;
}

/**
 *  HbThemeServerPrivate::setCurrentRenderingMode
 *  This function sets  ThemeServer's current rendering mode
 *
 */
void HbThemeServerPrivate::setCurrentRenderingMode(HbRenderingMode currentMode)
{
    renderMode = currentMode;
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
    return HbSgImageRenderer::global()->totalGPUMemory();
#else
    return 0;
#endif
}

unsigned long HbThemeServerPrivate::freeGPUMemory()
{
#ifdef HB_SGIMAGE_ICON
    qDebug() << "Inside  HbThemeServerSymbian::freeGPUMemory()  " ;
    return HbSgImageRenderer::global()->freeGPUMemory();
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

void HbThemeServerPrivate::createThemeIndex(const QString &themePath, const HbThemeType &themetype)
{
    #ifdef THEME_INDEX_TRACES
    qDebug() <<  "ThemeIndex: createThemeIndex" << themePath.toUtf8();
    #endif
    QDir themeBasePath(themePath);
    // Path is like "C:/resource/hb/themes/icons/sfwhitetheme"
    QString themeName = themeBasePath.dirName();
    
    QString filename(themeBasePath.absolutePath());
    int cutindex = filename.lastIndexOf("/",filename.lastIndexOf("/")-1)+1;
    filename = filename.left(cutindex);
    themeBasePath.setPath(filename);
    filename.append(themeName);
    filename.append(".themeindex");

    if (themePath == ":\\themes\\icons\\hbdefault") {
        themeBasePath.setPath(":/themes");
        filename = ":/themes/hbdefault.themeindex";
        themeName = "hbdefault";
    }
    
    QFile indexFile(filename);

    bool indexOK = false;

    if (indexFile.open(QIODevice::ReadOnly)) {

        indexOK = true;

        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);

        HbSharedChunkHeader *chunkHeader = (HbSharedChunkHeader*) manager->base();
        
        qint64 byteSize = indexFile.size();

        #ifdef THEME_INDEX_TRACES
        qDebug() <<  "ThemeIndex: " << theme.toUtf8() << " index file size:" << byteSize;
        #endif

        int offset = manager->alloc(byteSize);
        if (offset >= 0) {
            #ifdef THEME_INDEX_TRACES
            qDebug() <<  "ThemeIndex: memory allocated for theme: " << theme.toUtf8();
            #endif

            // Read the theme index in the shared chunk
            char *address = HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, offset);

            indexFile.read(address, byteSize);
            indexFile.close();

            #ifdef THEME_INDEX_TRACES
            qDebug() <<  "ThemeIndex: Reading themeindex for theme" << theme.toUtf8() << "... Done!";
            #endif

            // Verify theme index contents if it is not located in ROM,
            // so that it does not have over-indexing offsets which might
            // crash all the clients trying to read from it.

            if (themePath[0] != 'Z') {
                #ifdef THEME_INDEX_TRACES
                qDebug() <<  "ThemeIndex: Validating themeindex for theme" << theme.toUtf8();
                #endif

                HbThemeIndex index(address);
                indexOK = index.validateItems(byteSize);

                #ifdef THEME_INDEX_TRACES
                qDebug() <<  "ThemeIndex: Validating themeindex for theme" << theme.toUtf8() << " done! Result: " << indexOK;
                #endif
            }

            if (indexOK) {
                // Allocate theme path string from shared memory
                QByteArray themePathArray = themeBasePath.absolutePath().toLatin1();
                quint32 themePathOffset = manager->alloc(themePathArray.size()+1);
                memcpy(HbMemoryUtils::getAddress<char>(
                           HbMemoryManager::SharedMemory,
                           themePathOffset),
                           themePathArray.data(),
                           themePathArray.size()+1); // +1 for '/0'

                // Allocate theme name string from shared memory
                QByteArray themeNameArray = themeName.toLatin1();
                quint32 themeNameOffset = manager->alloc(themeNameArray.size()+1);
                memcpy(HbMemoryUtils::getAddress<char>(
                           HbMemoryManager::SharedMemory,
                           themeNameOffset),
                           themeNameArray.data(),
                           themeNameArray.size()+1); // +1 for '/0'

                // Store offset to the index in chunk
                switch (themetype) {
                case BaseTheme:
                    // Release previously allocated content
                    manager->free(chunkHeader->baseThemePathOffset);
                    manager->free(chunkHeader->baseThemeNameOffset);
                    manager->free(chunkHeader->baseThemeIndexOffset);
                    // Base theme offsets
                    chunkHeader->baseThemePathOffset = themePathOffset;
                    chunkHeader->baseThemeNameOffset = themeNameOffset;
                    chunkHeader->baseThemeIndexOffset = offset;
                    break;
                case OperatorC:
                    // Release previously allocated content
                    manager->free(chunkHeader->operatorThemeDriveCPathOffset);
                    manager->free(chunkHeader->operatorThemeDriveCNameOffset);
                    manager->free(chunkHeader->operatorThemeDriveCIndexOffset);
                    // Operator theme in C-drive offsets
                    chunkHeader->operatorThemeDriveCPathOffset = themePathOffset;
                    chunkHeader->operatorThemeDriveCNameOffset = themeNameOffset;
                    chunkHeader->operatorThemeDriveCIndexOffset = offset;
                    break;
                case OperatorROM:
                    // Release previously allocated content
                    manager->free(chunkHeader->operatorThemeRomPathOffset);
                    manager->free(chunkHeader->operatorThemeRomNameOffset);
                    manager->free(chunkHeader->operatorThemeRomIndexOffset);
                    // Operator theme in ROM offsets
                    chunkHeader->operatorThemeRomPathOffset = themePathOffset;
                    chunkHeader->operatorThemeRomNameOffset = themeNameOffset;
                    chunkHeader->operatorThemeRomIndexOffset = offset;
                    break;
                case ActiveTheme:
                    // Release previously allocated content
                    manager->free(chunkHeader->activeThemePathOffset);
                    manager->free(chunkHeader->activeThemeNameOffset);
                    manager->free(chunkHeader->activeThemeIndexOffset);
                    // Active theme offsets
                    chunkHeader->activeThemePathOffset = themePathOffset;
                    chunkHeader->activeThemeNameOffset = themeNameOffset;
                    chunkHeader->activeThemeIndexOffset = offset;
                    break;
                default:
                    break;
                }
            } else {
                // If the index contents were not OK, remove the index from the chunk
                manager->free(offset);
            }
        }
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
    case ENotifyForegroundLost:
#if defined(HB_SGIMAGE_ICON) || defined(HB_NVG_CS_ICON)
        freeClientGpuResources();
#endif
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
#ifdef HB_THEME_SERVER_MEMORY_REPORT
    case ECreateMemoryReport: {
        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
        static_cast<HbSharedMemoryManager *>(manager)->createReport();
        break;
    }
#endif
    case EMemoryGood: {
        if(iServer->currentRenderingMode() == ESWRendering){
            iServer->setCurrentRenderingMode(EHWRendering);
        }
        iServer->MemoryGood();
        break;
    }
    case EFreeRam: {
        HbFreeRamNotificationData params;
        TPckg<HbFreeRamNotificationData> paramPckg(params);
        aMessage.ReadL(0, paramPckg, 0);
        if(params.useSwRendering){
            iServer->setCurrentRenderingMode(ESWRendering );
        }
        iServer->FreeGpuRam(params.bytesToFree);
        break;
    }

    case ERenderModeSwitch: {
        TInt mode = 0;
        TPckg<TInt> paramPckg(mode);      
        aMessage.ReadL(0, paramPckg, 0);
        SwitchRenderingMode((HbRenderingMode)mode);
        break;
    }
    case EFreeSharedMem: {
        int freeSharedMem = iServer->freeSharedMemory();
        TPckg<int> out(freeSharedMem);
        aMessage.WriteL(1, out);
        break;
    }
    case EAllocatedSharedMem: {
        int allocatedSharedMem = iServer->allocatedSharedMemory();
        TPckg<int> out(allocatedSharedMem);
        aMessage.WriteL(1, out);
        break;
    }
    case EAllocatedHeapMem: {
        int allocatedHeapMem = iServer->allocatedHeapMemory();
        TPckg<int> out(allocatedHeapMem);
        aMessage.WriteL(1, out);
        break;
    }

    case ETypefaceOffset: {
        HandleTypefaceReqL(aMessage);
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

void HbThemeServerSession::SwitchRenderingMode(HbRenderingMode aRenderMode)
{
#if defined(HB_SGIMAGE_ICON) || defined(HB_NVG_CS_ICON)
    if(ESWRendering == aRenderMode) {
    //hw to sw    
#ifdef HB_ICON_CACHE_DEBUG
        HbSgImageRenderer *sgImg = HbSgImageRenderer::global();
        qDebug()<< "HbThemeServerSession::SwitchRenderingMode:GPU Memory Before Switching to Software:" << sgImg->freeGPUMemory();
#endif //HB_ICON_CACHE_DEBUG
        freeGpuResources();
#ifdef HB_ICON_CACHE_DEBUG
        qDebug()<< "HbThemeServerSession::SwitchRenderingMode:GPU Memory After Switching to Software:" << sgImg->freeGPUMemory();
#endif //HB_ICON_CACHE_DEBUG            
    } 
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
        if (!sessionCssData.contains(cssFileName)) {
            sessionCssData.append(cssFileName);
        }
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
                  (QIcon::Mode)params.mode, params.mirrored, color,
                  (HbRenderingMode)params.renderMode);

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
	        QScopedPointer <HbIconCacheItem> tempIconCacheItem(HbIconCacheItemCreator::createCacheItem( key,
                                                                    (HbIconLoader::IconLoaderOptions)params.options,                                                                    
                                                                    format,
                                                                    iServer->currentRenderingMode()));
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
            if (cacheItem) {
                FreeDataFromCacheItem(cacheItem);
            }
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

    if (cleanThemeName != iServer->iCurrentThemeName) {
        iServer->HandleThemeSelection(cleanThemeName);
        sessionData.clear();
    }
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
                           color,
                           (HbRenderingMode)params.renderMode);

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
        frameItemParams.renderMode = params.renderMode;
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
                           iServer->currentRenderingMode(),
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

    QScopedPointer<HbIconCacheItem> tempCacheItem(HbIconCacheItemCreator::createMultiPieceCacheItem(finalIconKey,
                                                  (HbIconLoader::IconLoaderOptions)params.options, 
                                                  format, dataForParts, params, allNvg,
                                                  iServer->currentRenderingMode()));
    HbIconCacheItem * cacheItem = tempCacheItem.data();
    if (cacheItem) {
        if (cacheItem->rasterIconData.type == INVALID_FORMAT) {
            return false;
        }
    } else {
        return false;
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

    try {
        for (int i = 0; i < noOfPieces; i++) {
            HbSharedIconInfo data;
            bool iconPieceMirrored = false;
        HbIconKey key(frameItemParams.multiPartIconList.at(i), frameItemParams.multiPartIconData.pixmapSizes[i], 
                      (Qt::AspectRatioMode)stichedKey.aspectRatioMode, (QIcon::Mode)stichedKey.mode, iconPieceMirrored, 
                      stichedKey.color,(HbRenderingMode)frameItemParams.renderMode);
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
    } catch (std::exception &) {
        failedToCreateParts = true;
    }
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
    if ((dataPartCount == noOfPieces) && (!failedToCreateParts)) {
        try {
            if (CreateStichedIconInfoOfParts(dataForParts, frameItemParams, stichedKey, stitchedData, allNvg)) {
                sessionData.append(stichedKey);
            }
        } catch(std::exception &) {

        }
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
    try {
        for (int i = 0; i < noOfPieces; i++) {
            HbSharedIconInfo data;
            QString pieceName((QChar*)frameItemParams.iconList[i].Ptr(), frameItemParams.iconList[i].Length());
            HbIconKey key(pieceName, frameItemParams.sizeList[i],
                          (Qt::AspectRatioMode)frameItemParams.aspectRatioMode, (QIcon::Mode)frameItemParams.mode,
                      (bool)frameItemParams.mirrored, color, (HbRenderingMode)frameItemParams.renderMode);
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
    } catch (std::exception &) {
        for (int i = 0; i < noOfPieces; i++) {
            iconInfoList.icon[i].type = INVALID_FORMAT;
        }
        QVector<HbIconKey>::const_iterator itEnd(keysInserted.constEnd());
        for (QVector<HbIconKey>::const_iterator iter = keysInserted.constBegin();iter != itEnd;++iter) {
            iServer->CleanupSessionIconItem(*iter);
        }
    }
}


/**
 * Handle Typeface data Request.
 */
void HbThemeServerSession::HandleTypefaceReqL(const RMessage2& aMessage)
{
    HbTypefaceDataInfo offsetInfo;
    HbTypefaceInfoDatabase *typefaceDatabase = HbTypefaceInfoDatabase::instance(HbMemoryManager::SharedMemory);
    if (typefaceDatabase) {
        offsetInfo.offset = typefaceDatabase->typefaceInfoVectorOffset();
    } else {
        offsetInfo.offset = -1;
    }
    TPckg<HbTypefaceDataInfo> data(offsetInfo);
    aMessage.WriteL(0, data);
}


void HbThemeServerSession::unLoadIcon(const RMessage2& aMessage)
{
    TIconParams params = ReadMessageAndRetrieveParams(aMessage);
    QString filename((QChar*)params.fileName.Ptr(), params.fileName.Length());
    QColor color = GetColorFromRgba(params.rgba, params.colorflag);
    HbIconKey key(filename, QSizeF(params.width, params.height),
                  (Qt::AspectRatioMode)params.aspectRatioMode,
                  (QIcon::Mode)params.mode, params.mirrored, color, (HbRenderingMode)params.renderMode);
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
        HbIconKey key(filename, params.sizeList[i], aspectRatioMode, mode, mirrored, color, 
                                (HbRenderingMode)params.renderMode);
        iServer->CleanupSessionIconItem(key);
        sessionData.removeOne(key);
    }
}

/**
freeClientGpuResources - unloads all the GPU resources in the session

Iterates all the keys in sessionData and if the Icon Format Type is SGIMage
or NVG then CleanUpSessionIConItem is called on that item, which will do the
cleanup based on the reference count.
*/
void HbThemeServerSession::freeClientGpuResources()
{
#if defined(HB_SGIMAGE_ICON) || defined(HB_NVG_CS_ICON)
	QList<HbIconKey> tempSessionData(sessionData);
    QList<HbIconKey>::const_iterator itEnd( tempSessionData.constEnd() );
    for ( QList<HbIconKey>::const_iterator iter = tempSessionData.constBegin();
            iter != itEnd;
            ++iter ) {
                if ( iServer->IconTypeInCache(*iter) == SGIMAGE
                    || iServer->IconTypeInCache(*iter) == NVG ) {
                    iServer->CleanupSessionIconItem(*iter);
                    // remove the item in the session data.
                    sessionData.removeOne(*iter);
                }
    }
#endif
}

/**
 * HbThemeServerSession::ClearSessionData
 *
 * Clears the session data list. Items in the server should be deleted before clearing.
 */
void HbThemeServerSession::ClearSessionData()
{
    sessionData.clear();
}

/**
freeGpuResources - unloads all the GPU resources in the session

Iterates all the keys in sessionData and if the Icon Format Type is SGIMage 
or NVG then CleanUpSessionIConItem is called on that item, which will do the 
cleanup based on the reference count. Also closes the SgDriver 
*/
void HbThemeServerSession::freeGpuResources()
{
#if defined(HB_SGIMAGE_ICON) || defined(HB_NVG_CS_ICON)
    freeClientGpuResources();
#if defined(HB_SGIMAGE_ICON)
    iServer->freeUnusedGpuResources();
#endif
#endif
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
                  (QIcon::Mode)params.mode, params.mirrored, color, (HbRenderingMode)params.renderMode);

    HbIconCacheItem* cacheItem = cache->value(key);//iconCacheItem(key);
    if(cacheItem)
        refCount =  cacheItem->refCount;
    else
        refCount = 0;
    return refCount;
}
#endif

#endif

CHbThemeChangeNotificationListener* CHbThemeChangeNotificationListener::NewL(HbThemeServerPrivate& aObserver)
{
    CHbThemeChangeNotificationListener* self = new (ELeave) CHbThemeChangeNotificationListener(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
}

CHbThemeChangeNotificationListener::CHbThemeChangeNotificationListener(HbThemeServerPrivate& aObserver)
    :CActive(EPriorityStandard),iObserver(aObserver)
{

}

void CHbThemeChangeNotificationListener::ConstructL()
{
    TInt err = RProperty::Define( KServerUid3, KNewThemeForThemeChanger, RProperty::ELargeText, KAllowAllPolicy, KThemeChangerPolicy );
     if ( err != KErrAlreadyExists ) {
         User::LeaveIfError( err );
     }
    err = themeRequestProp.Attach(KServerUid3, KNewThemeForThemeChanger );
    User::LeaveIfError(err);

    CActiveScheduler::Add(this);
}

CHbThemeChangeNotificationListener::~CHbThemeChangeNotificationListener()
{
    stopListening();
}

void CHbThemeChangeNotificationListener::startListening()
{
    if (IsActive()) {
         return; //do nothing if allready listening
    }

    User::LeaveIfError(themeRequestProp.Attach(KServerUid3,KNewThemeForThemeChanger));
    //Subscribe for updates
    themeRequestProp.Subscribe(iStatus);

    SetActive();

}

void CHbThemeChangeNotificationListener::stopListening()
{
     Cancel(); // cancel
     if(IsActive()) { // only if already listening
        themeRequestProp.Close(); // Close the handle since it is not needed anymore
   }
}

/*
 * Returns TRUE if parsing succeeded, FALSE otherwise
 */
bool CHbThemeChangeNotificationListener::parseData( TDesC& requestData, HbThemeServerRequest& etype, TDes& data)
{
    TInt result = 0;
    const TChar delimiter = ':';
    // initialize return value as failed
    bool bSuccess = false;

    result = requestData.Locate( delimiter );
    if( KErrNotFound != result ) {
        TInt len = requestData.Length();
        const TDesC& typestr = requestData.Mid( 0, result);
        TLex atype ( typestr );
        TInt iType;
        atype.Val( iType );
        etype = static_cast<HbThemeServerRequest>(iType);
        data.Copy( requestData.Mid( result + 1, len - result - 1 ) );
        bSuccess = true;
    } else {
        bSuccess = false;
    }

    return bSuccess;
}

void CHbThemeChangeNotificationListener::RunL()
{
    // Subscribe first to make sure we don't miss any
    // when handling this one.
    themeRequestProp.Subscribe(iStatus);

    SetActive();

    TBuf<256> requestData;
    TInt ret = themeRequestProp.Get(requestData);
    switch (ret) {
        case KErrNone:
            {
                QString qrequestData((QChar*)requestData.Ptr(),requestData.Length());
                HbThemeServerRequest etype = EInvalidServerRequest;
                TBuf<256> data;
                ///Parse the data from the Publisher
                bool bSuccess = parseData( requestData, etype, data);
                if( bSuccess && EThemeSelection == etype) {
                    QString str((QChar*)data.Ptr(),data.Length());
                    str = str.trimmed();
                    iObserver.HandleThemeSelection( str );
                }
            }
            break;
        case KErrPermissionDenied:
            qDebug() << "KErrPermissionDenied";
            break;
        case KErrNotFound:
            qDebug() << "KErrNotFound";
            break;
        case KErrArgument:
            qDebug() << "KErrArgument";
            break;
        case KErrOverflow:
            qDebug() << "KErrOverflow";
            break;
    }
}

void CHbThemeChangeNotificationListener::DoCancel()
{
    themeRequestProp.Cancel();
}



