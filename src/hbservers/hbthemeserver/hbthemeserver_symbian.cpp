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
#include <QFileInfo>
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
#include <sgresource/sgerror.h>
#endif
// 16 MB GPU cache size
#define GPU_CACHE_SIZE 0x1000000

// 5 MB  CPU cache size
#define CPU_CACHE_SIZE 0x500000

static const TInt KThemeName = 0;

const QString KOperatorCPath = "C:/resource/hb/prioritytheme/icons/";
const QString KOperatorZPath = "Z:/resource/hb/prioritytheme/icons/";
const QString KBaseThemePath = "z:/resource/hb/";

bool HbThemeServerPrivate::gpuGoodMemoryState = true;

#define HB_SPLASH_FLICKER_WORKAROUND

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
    CleanupStack::Pop(self);
    return self;
}

#ifdef HB_SPLASH_FLICKER_WORKAROUND
_LIT_SECURITY_POLICY_PASS(KHbTfxPsRdPolicy); // all pass
_LIT_SECURITY_POLICY_S0(KHbTfxPsWrPolicy, KServerUid3.iUid); // pass themeserver only
static void checkTfxRenderStage()
{
    // Check wsini.ini for a line containing [tfxrenderstage]. When present,
    // publish 1 in the property that will be read by HbMainWindow.
    TUint32 key = 0x746678;
    RProperty::Define(KServerUid3, key, RProperty::EInt, KHbTfxPsRdPolicy, KHbTfxPsWrPolicy);
    RProperty::Set(KServerUid3, key, 0);
    QFile f(QLatin1String("z:/system/data/wsini.ini"));
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream input(&f);
        input.setCodec("UTF-16");
        while (!input.atEnd()) {
            QString line = input.readLine().trimmed();
            if (!line.isEmpty()) {
                if (!line.compare(QLatin1String("[tfxrenderstage]"), Qt::CaseInsensitive)) {
                    RProperty::Set(KServerUid3, key, 1);
                    break;
                }
            }
        }
        f.close();
    }
}
#else
static void checkTfxRenderStage()
{
}
#endif

/**
ConstructL
 */
void HbThemeServerPrivate::ConstructL()
{
    checkTfxRenderStage();

    TInt err = RProperty::Define(KServerUid3, KThemeName, RProperty::ELargeText);
    if ( err != KErrAlreadyExists ) {
        User::LeaveIfError( err );
    }
    TInt error = iThemeProperty.Attach(KServerUid3, KThemeName );
    User::LeaveIfError(error);

    QString currentTheme = HbThemeUtils::getThemeSetting(HbThemeUtils::CurrentThemeSetting);

    // Store the active theme name in a member string
    // and resolve the path of the current theme
    QDir path(currentTheme);
    iCurrentThemeName = path.dirName();
    iCurrentThemePath = path.absolutePath();

    cache = 0;

    QT_TRY {
        //Create the Icon cache
        cache = new HbIconDataCache();
    } QT_CATCH(const std::bad_alloc &badalloc) {
        delete cache;
        qt_symbian_exception2LeaveL(badalloc);
    }
    setMaxGpuCacheSize(GPU_CACHE_SIZE);
    setMaxCpuCacheSize(CPU_CACHE_SIZE);
#if defined(HB_SGIMAGE_ICON) || defined(HB_NVG_CS_ICON)
    renderMode = EHWRendering;
#else
    renderMode = ESWRendering;
#endif

    // Load theme indexes
    UpdateThemeIndexes();

    // Start the splash screen generator app.
    QProcess::startDetached("hbsplashgenerator.exe");
}

void HbThemeServerPrivate::UpdateThemeIndexes(bool updateBase)
{
    // Start watching the current theme.index for uninstallation
    if (!openCurrentIndexFile()) {
        // theme doesn't exist activate default theme
        QString defaultTheme = HbThemeUtils::getThemeSetting(HbThemeUtils::DefaultThemeSetting);
        QDir path(defaultTheme);
        iCurrentThemeName = path.dirName();
        iCurrentThemePath = path.absolutePath();
    }

    if (updateBase) {
        // Process base theme index, it is used as parent index also when the current theme is something else
        HbThemeServerUtils::createThemeIndex(HbThemeUtils::getThemeSetting(HbThemeUtils::BaseThemeSetting), BaseTheme);
    }
    // Process operator theme indexes
    QString operatorName = HbThemeUtils::getThemeSetting(HbThemeUtils::OperatorNameSetting);
    if (!operatorName.isEmpty()) {
        QString operatorPath;
        operatorPath.append(KOperatorCPath);
        operatorPath.append(operatorName);
        HbThemeServerUtils::createThemeIndex(operatorPath, OperatorC);
        if (updateBase) {
            // Process operator Drive Z theme index
            QString operatorROMPath;
            operatorROMPath.append(KOperatorZPath);
            operatorROMPath.append(operatorName);
            HbThemeServerUtils::createThemeIndex(operatorROMPath, OperatorROM);
        }
    }

    // Process current theme index
    HbThemeServerUtils::createThemeIndex(iCurrentThemePath, ActiveTheme);

    // Register theme system effects
    // TODO: fix parameter
    HbThemeSystemEffect::handleThemeChange(iCurrentThemeName);

    // Set the current theme also in the cenrep key that is used to notify clients.
    HbThemeUtils::setThemeSetting(HbThemeUtils::CurrentThemeSetting, iCurrentThemePath);
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
    : CServer2( aActiveObjectPriority ), iWatcher(0)
{
    // Set up the listener to listen for Publish events
    TRAPD(err, iListener = CHbThemeChangeNotificationListener::NewL(*this));
    if (err) {
        THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "CHbThemeChangeNotificationListener::NewL failed. Error code:" << err;
    } else {
        TRAPD(err, iListener->startListeningL());
        if (err) {
             THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "iListener->startListening failed. Error code:" << err;
         }
    }    
}

/**
Destructor
 */
HbThemeServerPrivate::~HbThemeServerPrivate()
{
    delete cache;
    cache = 0;      // so that HbThemeServerSession::~HbThemeServerSession can avoid using these pointers;
    TInt err = RProperty::Delete(KServerUid3, KNewThemeForThemeChanger);
    if(err != KErrNotFound) {
        User::LeaveIfError(err);
    }
    // Delete the listener for Publish/Subscribe
    delete iListener;
    // Delete file watcher
    if (iWatcher) {
        delete iWatcher;
    }
}

/*
 * Returns FALSE if file doesn't exist, TRUE otherwise
 */
bool HbThemeServerPrivate::openCurrentIndexFile()
{
    if (!iCurrentThemePath.isEmpty() && iCurrentThemePath[0] != 'z' && 
        iCurrentThemePath[0] != 'Z' && iCurrentThemePath[0] != ':') {
        QString indexFileName;
        indexFileName.append(iCurrentThemePath);
        indexFileName.append("/index.theme");
        
        QFile currentIndexfile(indexFileName);
        if(!currentIndexfile.open(QIODevice::ReadOnly)) {
            THEME_GENERIC_DEBUG()<< Q_FUNC_INFO << "No Index file found in the new theme.";
            return false;
        } else {
            currentIndexfile.close();
            if (!iWatcher) {
                // Set up the file watcher for active theme changes
                iWatcher = new HbThemeWatcher(*this);
            }
            // Start watching in case of mmc ejection or active theme uninstall
            if (iWatcher) {
                iWatcher->startWatching(indexFileName);
            }
        }
    }
    return true;
}

/**
Handles theme selection
*/
void HbThemeServerPrivate::HandleThemeSelection( const QString& themeName)
{
    QString cleanThemeName = themeName;
    if (!HbThemeUtils::isThemeValid(cleanThemeName)) {
        // check if the theme name is logical
        cleanThemeName = KBaseThemePath + HbThemeUtils::platformHierarchy + '/' +
                    HbThemeUtils::iconsResourceFolder + '/' + cleanThemeName;
    }

    QDir path(cleanThemeName);
    iCurrentThemeName = path.dirName();
    iCurrentThemePath = path.absolutePath();

    THEME_INDEX_DEBUG() << Q_FUNC_INFO << "Theme change request, new theme =" << cleanThemeName.toUtf8();

    // Clear cached icons and session data
    clearIconCache();
    iSessionIter.SetToFirst();
    while(iSessionIter) {
        HbThemeServerSession &session = reinterpret_cast<HbThemeServerSession &>(*iSessionIter);
        session.ClearSessionData();
        iSessionIter++;
    }

    // Update current theme index
    UpdateThemeIndexes(false);
}

/**
Creates a new session with the server.
*/
CSession2* HbThemeServerPrivate::NewSessionL(const TVersion& aVersion, const RMessage2& /*aMessage*/) const
{
    THEME_GENERIC_DEBUG() << "entering" << Q_FUNC_INFO;
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
 * HbThemeServerPrivate::iconCacheItem
 *
 * Retrieves a icon cache-item from the icon cache based on it's key.
 */
HbIconCacheItem * HbThemeServerPrivate::iconCacheItem(const HbIconKey &key , bool isMultiIconPiece)
{
    return(cache->getCacheItem(key, renderMode, isMultiIconPiece));
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

void HbThemeServerPrivate::FreeGpuRam(int bytes, bool useSwRendering )
{
    gpuGoodMemoryState = false;
    cache->freeGpuRam(bytes, useSwRendering);
}

void HbThemeServerPrivate::freeGpuRam()
{
    cache->freeGpuRam();
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

HbRenderingMode HbThemeServerPrivate::expectedRenderingMode(HbRenderingMode requsetedRenderingMode) const
{
    HbRenderingMode renderingMode = ESWRendering;

#if defined(HB_SGIMAGE_ICON) || defined(HB_NVG_CS_ICON)    
    if((requsetedRenderingMode == EHWRendering) &&
       (currentRenderingMode() == EHWRendering) &&
        HbThemeServerPrivate::gpuMemoryState()) {
        renderingMode = EHWRendering;
    }
#endif    
    
    return renderingMode;
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

unsigned long HbThemeServerPrivate::totalGPUMemory()
{
#ifdef HB_SGIMAGE_ICON
    qDebug() << "Inside  HbThemeServerSymbian::totalGPUMemory()  " ;
    return HbSgImageRenderer::global()->totalGPUMemory();
#else
    return 0;
#endif
}

int HbThemeServerPrivate::cachedSgImagesCount() const
{
    return cache->cachedSgImagesCount();
}

int HbThemeServerPrivate::totalSgImagesCost() const
{
    return cache->totalSgImagesCost();
}

int HbThemeServerPrivate::cachedPixmapCount() const
{
    return cache->cachedPixmapCount();
}

unsigned long HbThemeServerPrivate::freeGPUMemory()
{
#ifdef HB_SGIMAGE_ICON
#ifdef HB_ICON_CACHE_DEBUG
    qDebug() << "Inside  HbThemeServerSymbian::freeGPUMemory()  " ;
#endif    
    return HbSgImageRenderer::global()->freeGPUMemory();
#else
    return 0;
#endif
}


bool HbThemeServerPrivate::isItemCacheableinGpu(int itemCost, HbIconFormatType type)
{    
    return cache->isItemCachableInGpu(itemCost, type);
}

bool HbThemeServerPrivate::isItemCacheableinCpu(int itemCost, HbIconFormatType type)
{
    return cache->isItemCachableInCpu(itemCost, type);
}


//Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
int HbThemeServerPrivate ::cacheIconCount() const
{
    return cache->count();
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
#endif

int HbThemeServerPrivate::gpuLRUSize() const
{
    return cache->gpuLRUSize();
}

//*********************************
//HbThemeServerSession
//**********************************
/**
This class represents a session with the server.
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

    sessionData.clear();
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
    THEME_GENERIC_DEBUG() << "entered" << Q_FUNC_INFO << "with function:" << aMessage.Function();

    TRAPD(err, DispatchMessageL(aMessage));
    aMessage.Complete(err);
    THEME_GENERIC_DEBUG() << "completed" << Q_FUNC_INFO << "with error code" + err;
}

/**
Called by ServiceL()

It tests the function code and then delegates to
the appropriate function.
*/
void HbThemeServerSession::DispatchMessageL(const RMessage2& aMessage)
{
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

    case EMultiPieceIcon:
        GetSharedMultiIconInfoL(aMessage);
        break;

    case EMultiIcon:
        GetMultiIconInfoL(aMessage);
        break;

    case EUnloadIcon:
        unLoadIcon(aMessage);
        break;

    case EBatchUnloadIcon:
        batchUnLoadIcon(aMessage);
        break;

    case EUnloadMultiIcon:
        unloadMultiIcon(aMessage);
        break;

    case ENotifyForegroundLost:
#if defined(HB_SGIMAGE_ICON) || defined(HB_NVG_CS_ICON)
        freeIconResources();
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
    case EFreeGPUMem: {
        unsigned long freeMem = iServer->freeGPUMemory();
        TPckg<unsigned long> out(freeMem);
        aMessage.WriteL(1, out);
        break;
    }   
    case ECachedSgImages: {
        TInt cachedSgImages = iServer->cachedSgImagesCount();
        TPckg<TInt> out(cachedSgImages);
        aMessage.WriteL(1, out);
        break;
    }
    case ETotalSgImagesCost: {
        TInt cachedSgImagesCost = iServer->totalSgImagesCost();
        TPckg<TInt> out(cachedSgImagesCost);
        aMessage.WriteL(1, out);
        break;
    }
    case ECachedPixmapImages: {
        TInt cachedPixImages = iServer->cachedPixmapCount();
        TPckg<TInt> out(cachedPixImages);
        aMessage.WriteL(1, out);
        break;
    }
    case ECurrentRenderingMode: {
        TInt currentRenderingMode = iServer->currentRenderingMode();
        TPckg<TInt> out(currentRenderingMode);
        aMessage.WriteL(1, out);
        break;
    }
    case ETotalGPUMem: {
        unsigned long totalMem = iServer->totalGPUMemory();
        TPckg<unsigned long> out(totalMem);
        aMessage.WriteL(1, out);
        break;
    }
#ifdef HB_THEME_SERVER_MEMORY_REPORT
    case ECreateMemoryReport: {
        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
        static_cast<HbSharedMemoryManager *>(manager)->createReport();
        break;
    }
#endif
    case EGPULRUSize: {
        TInt gpuLRUSize = iServer->gpuLRUSize();
        TPckg<TInt> out(gpuLRUSize);
        aMessage.WriteL(1, out);
        break;
    }
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
        iServer->FreeGpuRam(params.bytesToFree, params.useSwRendering);
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
    case EMissedHbCssLookup: {
        HandleMissedHbCssLookupL(aMessage);
        break;
    }
    default:
        PanicClient(aMessage, EBadRequest);
        break;
    }
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
        THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "no filename given.";
        return;
    }

    TFileName fileName;
    aMessage.ReadL(0, fileName, 0);
    TBuf<256> layerPriorityBuf;
    aMessage.ReadL(1, layerPriorityBuf, 0);
    TLex lex(layerPriorityBuf);
    TInt priorityValue;
    lex.Val(priorityValue);

    HbLayeredStyleLoader::LayerPriority layerPriority = (HbLayeredStyleLoader::LayerPriority) priorityValue;

    QString cssFileName((QChar*)fileName.Ptr(), fileName.Length());
    HbSharedStyleSheetInfo offsetInfo;

    offsetInfo.offset = HbThemeServerUtils::getSharedStylesheet(cssFileName, 
            layerPriority, offsetInfo.fileExists);

    TPckg<HbSharedStyleSheetInfo> data(offsetInfo);
    aMessage.WriteL(2, data);
}

/**
 * HandleMissedHbCssLookupL
 */
void HbThemeServerSession::HandleMissedHbCssLookupL(const RMessage2& aMessage)
{
    HbSharedMissedHbCssInfo offsetInfo;
    offsetInfo.offset = HbThemeServerUtils::getMissedHbCssFilesOffset();
    TPckg<HbSharedMissedHbCssInfo> data(offsetInfo);
    aMessage.WriteL(0, data);
}

static const TInt KMaxLayoutName = 256;
static const TInt KMaxSectionName = 256;

/**
 * HandleWidgetMLLookUp
 */
void HbThemeServerSession::HandleWidgetMLLookupL(const RMessage2& aMessage)
{
    if (aMessage.GetDesLength(0) == 0) {
        return;
    }

    TFileName fileName;
    aMessage.ReadL(0, fileName, 0);
    TBuf<KMaxLayoutName> layoutName;
    aMessage.ReadL(1, layoutName, 0);
    TBuf<KMaxSectionName> sectionName;
    aMessage.ReadL(2, sectionName, 0);

    QString wmlFileName((QChar*)fileName.Ptr(), fileName.Length());
    QString layout((QChar*)layoutName.Ptr(), layoutName.Length());
    QString section((QChar*)sectionName.Ptr(), sectionName.Length());

    HbSharedWMLInfo offsetInfo;
    offsetInfo.offset = HbThemeServerUtils::getSharedLayoutDefinition(wmlFileName, layout, 
            section, offsetInfo.fileExists);
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

    bool insertKeyIntoSessionList = false;
    QString format = HbThemeServerUtils::formatFromPath(key.filename);
    HbIconFormatType iconType = getIconFormatType(format);

    HbIconCacheItem* cacheItem = getCachedIcon(key, format, false);    
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
            QScopedPointer <HbIconCacheItem> tempIconCacheItem;
            int gpuItemCost = HbThemeServerUtils::computeGpuCost(key, iconType, false);
            HbRenderingMode rm = iServer->expectedRenderingMode((HbRenderingMode)key.renderMode);
            // A hardware icon (SGImage) is only created when icon format is NVG,
            // when both client and ThemeServer is in HW rendering mode and
            // there is enough GPU space to create anew HW icon
            if (rm == EHWRendering && format == HbIconCacheItemCreator::KNvg &&
                iServer->isItemCacheableinGpu(gpuItemCost, iconType)) {
                tempIconCacheItem.reset(HbIconCacheItemCreator::createCacheItem(key,
                static_cast<HbIconLoader::IconLoaderOptions>(params.options),
                format, EHWRendering));    
            } else {
                int cpuItemCost = HbThemeServerUtils::computeCpuCost(key, iconType, false);
                if (iServer->isItemCacheableinCpu(cpuItemCost, iconType)){
                    tempIconCacheItem.reset(HbIconCacheItemCreator::createCacheItem(key,
                    static_cast<HbIconLoader::IconLoaderOptions>(params.options),
                    format, ESWRendering));    
                }
            }
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
                    // set the rendering mode according to the actual rendering mode
                    // in which the icon is created
                    key.renderMode = cacheItem->renderingMode();
                    
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
#ifdef HB_SGIMAGE_ICON
    HbSgImageRenderer *sgImg = HbSgImageRenderer::global();
    if (sgImg->lastError() == KErrNoGraphicsMemory ) {
       iServer->freeGpuRam();
    }
#endif
    // create dshared pixmap info from HbIconCacheItem
    TPckg<HbSharedIconInfo> pixdata(data);
    aMessage.WriteL(1, pixdata);
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

inline
QSize convert(const TSize &size)
{
    return QSize(size.iWidth, size.iHeight);
}
inline
QRect convert(const TRect &rect)
{
    return QRect(QPoint(rect.iTl.iX, rect.iTl.iY), QPoint(rect.iBr.iX, rect.iBr.iY));
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
    QString format = HbThemeServerUtils::formatFromPath(fullPath);
    int index = fullPath.lastIndexOf('/');
    fullPath = fullPath.left(index + 1);
    iconId.prepend(fullPath);
    HbIconKey finalIconKey(iconId,
                           params.size,
                           static_cast<Qt::AspectRatioMode>(params.aspectRatioMode),
                           static_cast<QIcon::Mode>(params.mode),
                           params.mirrored,
                           color,
                           static_cast<HbRenderingMode>(params.renderMode));

    if (!IconInfoFromSingleIcon(finalIconKey, stitchedData, format, false)) {
        HbMultiIconParams frameItemParams;
        int noOfPieces = 1;
        if (iconId.contains("_3PV", Qt::CaseInsensitive) || iconId.contains("_3PH", Qt::CaseInsensitive)) {
            noOfPieces = 3;
        } else if (iconId.contains("_9P", Qt::CaseInsensitive)) {
            noOfPieces = 9;
        }

        frameItemParams.multiPartIconId = iconId;
        frameItemParams.aspectRatioMode = params.aspectRatioMode;
        frameItemParams.colorflag = params.colorflag;
        frameItemParams.mirrored = params.mirrored;
        frameItemParams.options = params.options;
        frameItemParams.rgba = params.rgba;
        frameItemParams.mode = params.mode;
        frameItemParams.size = params.size;
        frameItemParams.color = color;
        frameItemParams.renderMode = params.renderMode;
        QT_TRY {
            for (int i = 0; i < noOfPieces; i++) {
                frameItemParams.multiPartIconData.pixmapSizes[i] = convert(params.pixmapSizes[i]);
                frameItemParams.multiPartIconData.targets[i] = convert(params.targets[i]);
                frameItemParams.multiPartIconData.sources[i] = convert(params.sources[i]);
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

HbIconCacheItem* HbThemeServerSession::getCachedIcon(HbIconKey & key, const QString & format,
                                                    bool isConsolidatedIcon)
{
    HbIconCacheItem* cacheItem = 0;
    
    // try to find the icon in other rendering mode,
    // if rendering will take place in another rendering mode
    HbRenderingMode rm = iServer->expectedRenderingMode((HbRenderingMode)key.renderMode);
    HbIconFormatType type = getIconFormatType(format);
    if (rm == key.renderMode) {
        cacheItem = iServer->iconCacheItem(key);
    } else if (rm != key.renderMode) {
        HbIconKey newKey(key);
        newKey.renderMode = rm;
        cacheItem = iServer->iconCacheItem(newKey);
        if (cacheItem) {
            key.renderMode = rm;
        }
    }
    
    if (!cacheItem) {
        // try search for icon in software mode if the icon is not of type nvg
        // currently only nvg type is used to create RSgImage.
        // If hardware icon is not found and there is not space to create a hardware
        // icon, search for the icon in software mode. 
        int itemGpuCost = HbThemeServerUtils::computeGpuCost(key, type, isConsolidatedIcon ); 
        if (format != HbIconCacheItemCreator::KNvg ||
            (rm == EHWRendering && !iServer->isItemCacheableinGpu(itemGpuCost, type)) ) {            
            HbIconKey newKey(key);
            newKey.renderMode = ESWRendering;
            cacheItem = iServer->iconCacheItem(newKey);
            if (cacheItem) {
                key.renderMode = ESWRendering;
            }
        }
        
    }
    
    return cacheItem;
}

/**
 * HbThemeServerPrivate::IconInfoFromSingleIcon
  Checks for the cacheItem for a given key, if found gets the data relevant of the cacheItem.
 */

bool HbThemeServerSession::IconInfoFromSingleIcon(HbIconKey key,
        HbSharedIconInfo &stitchedData, const QString & format, bool isMultiPiece)
{
    HbIconCacheItem* cacheItem = 0;
    
    // try to find the icon in other rendering mode,
    // if rendering will take place in another rendering mode
    HbRenderingMode rm = iServer->expectedRenderingMode((HbRenderingMode)key.renderMode);
    HbIconFormatType type = getIconFormatType(format);
    bool consolidatedIcon = !isMultiPiece;
    if (rm == key.renderMode) {
        cacheItem = iServer->iconCacheItem(key, true);
    } else if (rm != key.renderMode) {
        HbIconKey newKey(key);
        newKey.renderMode = rm;
        cacheItem = iServer->iconCacheItem(newKey, true);
        if (cacheItem) {
            key.renderMode = rm;
        }
    }
    
    if (!cacheItem) {
        if (!format.isEmpty()) {
            int gpuItemCost = 0;
            // only consolidated icon ican be cached in GPU
            if (consolidatedIcon) {
                gpuItemCost = HbThemeServerUtils::computeGpuCost(key, type, consolidatedIcon);
            }
            
            // try search for icon in software mode if the icon is not of type nvg
            // currently only nvg type is used to create RSgImage
            if ((format != HbIconCacheItemCreator::KNvg) ||
                (rm == EHWRendering && !iServer->isItemCacheableinGpu(gpuItemCost, type) &&
                !isMultiPiece)) {            
                HbIconKey newKey(key);
                newKey.renderMode = ESWRendering;
                cacheItem = iServer->iconCacheItem(newKey);
                if (cacheItem) {
                    key.renderMode = ESWRendering;
                }
            }
        }
    }
    
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
    HbIconFormatType type = getIconFormatType(format);
    HbIconCacheItem * cacheItemOfPiece = 0; 
    HbRenderingMode rm = iServer->expectedRenderingMode((HbRenderingMode)key.renderMode);
    int cpuItemCost = HbThemeServerUtils::computeFrameItemCpuCost(key, type, rm);
    
    if (iServer->isItemCacheableinCpu(cpuItemCost, type)) {
        cacheItemOfPiece = HbIconCacheItemCreator::createCacheItem(key,
                            (HbIconLoader::IconLoaderOptions)options,
                            format,
                            rm,
                            isMultiIcon);
        if (cacheItemOfPiece) {
            GetDataFromCacheItem(cacheItemOfPiece, data);
            if (data.type != INVALID_FORMAT) {
                // set the rendering mode according to the actual rendering mode
                // in which the icon is created 
                key.renderMode = cacheItemOfPiece->renderingMode();
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
    HbIconFormatType type = getIconFormatType(format);
    QScopedPointer <HbIconCacheItem> tempCacheItem;
    
    int gpuItemCost = HbThemeServerUtils::computeGpuCost(finalIconKey, type, true);
    
    HbRenderingMode rm = iServer->expectedRenderingMode((HbRenderingMode)finalIconKey.renderMode);
    
    // A consolidated hardware icon (SGImage) is only created when icon format is NVG,
    // when both client and ThemeServer is in HW rendering mode and
    // there is enough GPU space to create anew HW icon
    
    if (rm == EHWRendering && format == HbIconCacheItemCreator::KNvg &&
        iServer->isItemCacheableinGpu(gpuItemCost, type)) {
        tempCacheItem.reset(HbIconCacheItemCreator::createMultiPieceCacheItem(finalIconKey,
        (HbIconLoader::IconLoaderOptions)params.options, format, dataForParts, 
        params, allNvg, EHWRendering));    
    }
    else {
        int cpuItemCost = HbThemeServerUtils::computeCpuCost(finalIconKey, type, true);
        if (iServer->isItemCacheableinCpu(cpuItemCost, type)) {
            tempCacheItem.reset(HbIconCacheItemCreator::createMultiPieceCacheItem(finalIconKey,
            (HbIconLoader::IconLoaderOptions)params.options, format, dataForParts, 
            params, allNvg, ESWRendering));    
        }
    }
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
        finalIconKey.renderMode = cacheItem->renderingMode();
        insertKeyIntoSessionList = iServer->insertIconCacheItem(finalIconKey, cacheItem);
        if (!insertKeyIntoSessionList) {
            //if insertion failed free the memory
            FreeDataFromCacheItem(cacheItem);
            tempCacheItem.take();
            delete cacheItem; // do delete the item after gpu/cpu memory is freed
            stitchedData.type = INVALID_FORMAT;
        }
    }
#ifdef HB_SGIMAGE_ICON    
    HbSgImageRenderer *sgImg = HbSgImageRenderer::global();
    if (sgImg->lastError() == KErrNoGraphicsMemory ) {
        iServer->freeGpuRam();
    }
#endif    
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
            QString format = HbThemeServerUtils::formatFromPath(key.filename);
            insertKeyIntoSessionList = IconInfoFromSingleIcon(key, data, format, true );
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
        case SGIMAGE:
#ifdef HB_SGIMAGE_ICON
            HbSgImageRenderer::removeSgImageFromHash(cacheItem->rasterIconData.sgImageData.id);
#endif
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
            QString format = HbThemeServerUtils::formatFromPath(key.filename);
            insertKeyIntoSessionList = IconInfoFromSingleIcon(key, data, format, true );
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


void HbThemeServerSession::performUnload(const TIconParams &params)
{
    QString filename((QChar*) params.fileName.Ptr(), params.fileName.Length());
    HbIconKey key(filename,
                  QSizeF(params.width, params.height),
                  (Qt::AspectRatioMode) params.aspectRatioMode,
                  (QIcon::Mode) params.mode,
                  params.mirrored,
                  GetColorFromRgba(params.rgba, params.colorflag),
                  (HbRenderingMode) params.renderMode);
    iServer->CleanupSessionIconItem(key);
    sessionData.removeOne(key);
}

void HbThemeServerSession::unLoadIcon(const RMessage2& aMessage)
{
    TIconParams params = ReadMessageAndRetrieveParams(aMessage);
    performUnload(params);
}

void HbThemeServerSession::batchUnLoadIcon(const RMessage2& aMessage)
{
    typedef TIconParams Params[BATCH_SIZE_LIMIT];
    Params paramList;
    TPckg<Params> paramsPckg(paramList);
    aMessage.ReadL(0, paramsPckg);
    for (int i = 0; i < BATCH_SIZE_LIMIT; ++i) {
        const TIconParams &params(paramList[i]);
        if (params.fileName.Length()) {
            performUnload(params);
        }
    }
}

void HbThemeServerSession::unloadMultiIcon(const RMessage2& aMessage)
{
    TIconListParams params;
    TPckg<TIconListParams> paramPckg(params);
    // Copy the client's descriptor data into our buffer.
    aMessage.ReadL(0, paramPckg, 0);

    QColor color = GetColorFromRgba(params.rgba, params.colorflag);
    Qt::AspectRatioMode aspectRatioMode = static_cast<Qt::AspectRatioMode>(params.aspectRatioMode);
    QIcon::Mode mode = static_cast<QIcon::Mode>(params.mode);
    TBool mirrored = params.mirrored;

    TInt iconCount = params.iconCount;
    for (int i = 0; i < iconCount; i++) {
        QString filename((QChar*)params.iconList[i].Ptr(), params.iconList[i].Length());
        HbIconKey key(filename, params.sizeList[i], aspectRatioMode, mode, mirrored, color, 
                                static_cast<HbRenderingMode>(params.renderMode));
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
freeIconResources - unloads all the resources in the session

Iterates all the keys in sessionData and then CleanUpSessionIConItem is called
on that item, which will do the cleanup based on the reference count.
*/
void HbThemeServerSession::freeIconResources()
{
    QList<HbIconKey> tempSessionData(sessionData);
    QList<HbIconKey>::const_iterator itEnd( tempSessionData.constEnd() );
    for ( QList<HbIconKey>::const_iterator iter = tempSessionData.constBegin();
            iter != itEnd;
            ++iter ) {
                
        iServer->CleanupSessionIconItem(*iter);
        // remove the item in the session data.
        sessionData.removeOne(*iter);
               
    }
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

HbIconFormatType HbThemeServerSession::getIconFormatType(const QString & format)
{
    HbIconFormatType type = INVALID_FORMAT;
    if (format == HbIconCacheItemCreator::KNvg) {
        type = NVG;
    } else if (format == HbIconCacheItemCreator::KBlob) {
        type = BLOB;
    } else if (format == HbIconCacheItemCreator::KSvg) {
        type = SVG;
    } else if (format == HbIconCacheItemCreator::KPic) {
        type = PIC;
    }
    
    return type;
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

    HbIconCacheItem* cacheItem = cache->value(key);
    if(cacheItem)
        refCount =  cacheItem->refCount;
    else
        refCount = 0;
    return refCount;
}
#endif

#endif
