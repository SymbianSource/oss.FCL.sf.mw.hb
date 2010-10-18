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

#ifndef HBTHEMESERVER_SYMBIAN_P_H
#define HBTHEMESERVER_SYMBIAN_P_H

#include <QFile>
#include <QColor>
#include <QHash>
#include <QList>
#include <QSizeF>
#include <QString>
#include <QChar>

#include "hbthemecommon_p.h"
#include "hbthemecommon_symbian_p.h"
#include "hbicondatacache_p.h"
#include "hbthemewatcher_symbian_p.h"
#include <e32property.h>
#include <e32base.h>
#include <f32file.h>

class HbThemeServerSession;
struct HbIconKey;
class HbIconSource;
class CHbThemeChangeNotificationListener;
class CHbThemeWatcher;

// reasons for server panic
enum TPixmapServPanic {
    EBadRequest = 1,
    EBadDescriptor,
    EMainSchedulerError,
    ESvrCreateServer,
    ESvrStartServer,
    ECreateTrapCleanup,
    ENotImplementedYet,
};

//**********************************
//HbThemeServerSymbian
//**********************************
/**
Our server class - an active object - and therefore derived ultimately from CActive.
It accepts requests from client threads and forwards
them to the client session to be dealt with. It also handles the creation
of the server-side client session.
*/
class HbThemeServerPrivate : public CServer2
{
public:
    // Creates a new session with the server; the function
    // implements the pure virtutal function
    // defined in class CServer2
    HbThemeServerPrivate(CActive::TPriority aActiveObjectPriority);
    HbIconFormatType IconTypeInCache( const HbIconKey &key ) const;
    static HbThemeServerPrivate * NewL(CActive::TPriority aActiveObjectPriority);
    ~HbThemeServerPrivate();
    CSession2 * NewSessionL(const TVersion& aVersion, const RMessage2& aMessage) const;

public :
    // Function to panic the server
    static void PanicServer(TPixmapServPanic aPanic);
    bool insertIconCacheItem(const HbIconKey &key,  HbIconCacheItem *item);
    HbIconCacheItem * iconCacheItem(const HbIconKey &key, bool isMultiPiece = false);
    void setMaxGpuCacheSize(int size);
    void setMaxCpuCacheSize(int size);
    void CleanupSessionIconItem(HbIconKey key);
    void clearIconCache();

    void MemoryGood();
    void FreeGpuRam(int bytes, bool useSwRendering);
    void freeGpuRam();
    void freeUnusedGpuResources();
    
    static bool gpuMemoryState();
    bool isItemCacheableinGpu(int itemCost, HbIconFormatType type);
    bool isItemCacheableinCpu(int itemCost, HbIconFormatType type);
   

    bool openCurrentIndexFile();
    HbRenderingMode currentRenderingMode() const;
    HbRenderingMode expectedRenderingMode(HbRenderingMode requsetedRenderingMode) const;
    void setCurrentRenderingMode(HbRenderingMode currentMode);
    void HandleThemeSelection( const QString& themeName);

    int freeSharedMemory();
    int allocatedSharedMemory();
    int allocatedHeapMemory();
    int gpuLRUSize() const;
    unsigned long freeGPUMemory();
    unsigned long totalGPUMemory();
    int cachedSgImagesCount() const;
    int totalSgImagesCost() const;
    int cachedPixmapCount() const;
//Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
    int cacheIconCount() const;
    int memorySize(const QString &mem);
    int freeVectorMemory();
    int freeRasterMemory();
    int lastAddedRefCount();
    int lastAddedItemMem();
    int lastRemovedItemMem();
    int lastRemovedItemRfCount();
    bool enableCache(bool cacheIt);
    int cacheHitCount();
    int cacheMissCount();
    int serverHeapSize();
    void cleanRasterLRUList();
    void cleanVectorLRUList();
    int rasterLruCount();
    int vectorLruCount();
#if defined(Q_OS_SYMBIAN)
    int iconRefCount(const RMessage2& aMessage);
#endif

#endif

public:
    RProperty iThemeProperty;
    QString iCurrentThemeName;
    QString iCurrentThemePath;

private:
    void ConstructL();
    void UpdateThemeIndexes(bool updateBase = true);
    HbIconDataCache * cache;

    static bool gpuGoodMemoryState;
    HbRenderingMode renderMode;
    CHbThemeChangeNotificationListener * iListener;
    HbThemeWatcher *iWatcher;
};

//**********************************
//HbThemeServerSession
//**********************************
/**
This class represents a session with the  server.
Functions are provided to respond appropriately to client messages.
*/
class HbThemeServerSession : public CSession2
{
public:
    HbThemeServerSession(HbThemeServerPrivate * aServer);
    ~HbThemeServerSession();
    void ServiceL(const RMessage2 & aMessage);
    void DispatchMessageL(const RMessage2 & aMessage);
    void GetSharedIconInfoL(const RMessage2 & aMessage);
    void HandleStyleSheetLookupL(const RMessage2 & aMessage);
    void HandleMissedHbCssLookupL(const RMessage2 &aMessage);
    void HandleWidgetMLLookupL(const RMessage2& aMessage);
    void HandleDeviceProfilesReqL(const RMessage2& aMessage);
    void HandleEffectAddAndFileLookupL(const RMessage2 &aMessage);
    QColor GetColorFromRgba(TUint32 aRgba, bool aColorFlag);
    HbIconCacheItem* getCachedIcon(HbIconKey & key, const QString & format,
                                    bool isConsolidatedIcon);
    void GetSharedMultiIconInfoL(const RMessage2& aMessage);
    void GetMultiIconInfoL(const RMessage2& aMessage);
    TMultiIconSymbParams ReadMessageAndRetrieveMultiIconParams(const RMessage2& aMessage);
    void GetDataFromCacheItem(HbIconCacheItem* cacheItem, HbSharedIconInfo &data) const;
    void FreeDataFromCacheItem(HbIconCacheItem* cacheItem);
    bool IconInfoFromSingleIcon(HbIconKey key, HbSharedIconInfo &stitchedData, 
                                const QString & format, bool isMultiPiece);
    bool CreateCacheItemData(HbIconKey key, int options, HbSharedIconInfo &data,
                             bool isMultiPiece = false);
    bool CreateStichedIconInfoOfParts(QVector<HbSharedIconInfo> dataForParts,
                                      HbMultiIconParams params,
                                      HbIconKey &finalIconKey,
                                      HbSharedIconInfo &stitchedData,
                                      bool allNvg);
    void IconInfoFromMultiParts(const HbMultiIconParams &paramsCommon,
                                const int noOfPieces,
                                HbIconKey &stichedKey,
                                HbSharedIconInfo &stitchedData);
    void IconInfoL(const TIconListParams &frameItemParams,
                   const int noOfPieces,
                   HbSharedIconInfoList &iconInfoList);
    void HandleTypefaceReqL(const RMessage2& aMessage);
    void unLoadIcon(const RMessage2& aMessage);
    void batchUnLoadIcon(const RMessage2& aMessage);
    void unloadMultiIcon(const RMessage2& aMessage);
    void freeClientGpuResources();
    void freeIconResources();
    void ClearSessionData();
    void freeGpuResources();
    void SwitchRenderingMode(HbRenderingMode aRenderMode);
    HbIconFormatType getIconFormatType(const QString & format);

protected:
    TIconParams ReadMessageAndRetrieveParams(const RMessage2 & aMessage);
    void PanicClient(const RMessage2 & aMessage, TInt aPanic) const;
    void performUnload(const TIconParams &params);

private:
    HbThemeServerPrivate *iServer;
    QList<HbIconKey> sessionData;
};

#endif // HBTHEMESERVER_SYMBIAN_P_H
