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

#ifndef HBTHEMECOMMON_P_H
#define HBTHEMECOMMON_P_H

#include <QImage>
#include <QSize>
#include <QMap>
#include <QStringList>
#include <QColor>
#include <hbstring_p.h>
#include <hbvector_p.h>
#ifdef HB_SGIMAGE_ICON
#include <sgresource/sgimage.h>
#endif

#define THEME_SERVER_NAME "hbthemeserver"
#define BIN_CSS_APP "hbbincssmaker"
#define BIN_CSS_APP_SYMBIAN "hbbincssmaker_symbian"
#ifdef HB_BIN_CSS
#define HB_THEME_SHARED_PIXMAP_CHUNK "themeserver_tool_chunk"
#else
#define HB_THEME_SHARED_PIXMAP_CHUNK "themeserver_chunk"
#endif
#define ORGANIZATION "Nokia"
#define THEME_COMPONENT "Hb Themes"
#define CURRENT_THEME_KEY "CurrentTheme"

// To enable/disable debug messages for theme server functionality
// this is master trace switch that enables all theme server related traces
#undef THEME_SERVER_TRACES

// To enable/disable debug messages for theme index functionality
#undef THEME_INDEX_TRACES

#undef CSSBIN_TRACES

// To enable memory report creation (memory usage, fragmentation etc.)
#undef HB_THEME_SERVER_MEMORY_REPORT
// To enable full memory report including all allocs, frees and reallocs
#undef HB_THEME_SERVER_FULL_MEMORY_REPORT

#ifdef HB_THEME_SERVER_FULL_MEMORY_REPORT
#define HB_THEME_SERVER_MEMORY_REPORT
#endif

// If master trace macro is defined, define also other theme traces
#ifdef THEME_SERVER_TRACES
#define THEME_INDEX_TRACES
#define CSSBIN_TRACES
#endif

// To enable fute testing for cache
//#define HB_ICON_CACHE_DEBUG


enum HbThemeType {
    BaseTheme   = 0,
    OperatorC   = 1,
    OperatorROM = 2,
    ActiveTheme = 3
};

const quint32 INITIALIZED_CHUNK_IDENTIFIER = 0x54535256; //'TSRV'
struct HbSharedChunkHeader
{
    quint32 identifier;
    quint32 mainAllocatorOffset;
    quint32 subAllocatorOffset;
    quint32 sharedCacheOffset;	
    // Base theme offsets
    quint32 baseThemePathOffset;
    quint32 baseThemeNameOffset;
    quint32 baseThemeIndexOffset;
    // Operator theme in C-drive offsets
    quint32 operatorThemeDriveCPathOffset;
    quint32 operatorThemeDriveCNameOffset;
    quint32 operatorThemeDriveCIndexOffset;
    // Operator theme in ROM offsets
    quint32 operatorThemeRomPathOffset;
    quint32 operatorThemeRomNameOffset;
    quint32 operatorThemeRomIndexOffset;
    // Active theme offsets
    quint32 activeThemePathOffset;
    quint32 activeThemeNameOffset;
    quint32 activeThemeIndexOffset;

};

enum LayerPriority {
    Priority_Core,
    Priority_Theme,
    Priority_AppTheme,
    Priority_Application,
    Priority_Operator
};

// Assuming there is only one folder for single layer to keep it simple.
// for icon resources there may be multiple folders but for there is other
// approach being used for icons. Approaches need to be merged while refactoring
// complete lookup
typedef QMap<LayerPriority, QString> HbLayerFolderMap;
struct HbMultiPartSizeData
{
    // Indexing order is:

    // 'NinePieces'             -> tl, t, tr, l, c, r, bl, b, br
    // 'ThreePiecesHorizontal'  -> l, c, r
    // 'ThreePiecesVertical'    -> t, c, b
    QString multiPartIconId;
    QRect sources[9];       // rects used from the rasterized frame part pixmaps
    QRect targets[9];       // frame part target rects inside the bounding rectangle of the frame
    QSize pixmapSizes[9];   // frame part pixmaps are rasterized to these sizes
};


struct HbMultiIconParams
{
    QString multiPartIconId;
    QStringList multiPartIconList;
    HbMultiPartSizeData multiPartIconData;
    QSizeF size;
    int aspectRatioMode;
    int mode;
    int options;
    bool mirrored;
    QColor color;
    int rgba;
    bool colorflag;
    int renderMode;
};


enum HbIconFormatType {
    INVALID_FORMAT = -1,
    NVG,
    PIC,
    SVG,
    SGIMAGE,
    BLOB,
    OTHER_SUPPORTED_FORMATS
};

struct HbSharedPixmapInfo
{
    int offset;
    int width;
    int height;
    int defaultWidth;
    int defaultHeight;
    QImage::Format format;
};

struct HbSharedNVGInfo
{
    int offset;
    int dataSize;
    int width;
    int height;
    int defaultWidth;
    int defaultHeight;

};

struct HbSharedPICInfo
{
    int offset;
    int dataSize;
    int width;
    int height;
    int defaultWidth;
    int defaultHeight;
};

struct HbSharedSgimageInfo
{
    unsigned long long id;
    int width;
    int height;
    int defaultWidth;
    int defaultHeight;
};

struct HbSharedBLOBInfo
{
    int offset;
    int dataSize;
};

struct HbSharedIconInfo
{
    HbIconFormatType type;

    union
    {
        HbSharedPixmapInfo pixmapData;
        HbSharedNVGInfo nvgData;
        HbSharedPICInfo picData;
        HbSharedSgimageInfo sgImageData;
        HbSharedBLOBInfo blobData;
    };

    HbSharedIconInfo():type(INVALID_FORMAT){}

};

struct HbSharedIconInfoList
{
  HbSharedIconInfo icon[9];
};

struct HbSharedStyleSheetInfo
{
    int offset;
    int refCount;
    HbSharedStyleSheetInfo():
        offset(-1),
        refCount(0)
    {}
};

struct HbSharedEffectInfo
{
    int offset;
    HbSharedEffectInfo(): offset(-1){}
};

struct HbSharedWMLInfo
{
    int offset;
    HbSharedWMLInfo() : offset(-1) {}
};

struct HbDeviceProfileInfo
{
    int offset;
    HbDeviceProfileInfo() : offset(-1) {}
};
struct HbSecondaryCacheInfo
{
    int offset;
    HbSecondaryCacheInfo() : offset(-1) {}
};


struct HbTypefaceDataInfo
{
    int offset;
    HbTypefaceDataInfo() : offset(-1) {}
};

// Function codes (opcodes) used in message passing between client and server
enum HbThemeServerRequest
    {
     EInvalidServerRequest = 0,
     EIconLookup = 1,
     EIconDefaultSize,
     EStyleSheetLookup,
     EThemeSelection,
     EMultiPieceIcon,
     EMultiIcon,
     EWidgetMLLookup,
     EDeviceProfileOffset,
     ENotifyForegroundLost,
 #ifdef HB_ICON_CACHE_DEBUG
     EIconCleanUp,
     ECacheIconCount,
     ERasterMemLimit,
     EVectorMemLimit,
     EFreeRasterMem,
     EFreeVectorMem,
     ELastAddedItemMem,
     ELastAddedItemRefCount,
     ELastRemovedItemMem,
     ELastRemovedItemRefCount,
     EEnableCache,
     ECacheHit,
     ECacheMiss,
     ECleanRasterLRUList,
     ECleanVectorLRUList,
     EServerHeap,
     EGpuLruCount,
     ECpuLruCount,
     EServerStat,
     EServerHeapMarkStart,
     EServerHeapMarkEnd,
     EServerAllocFail,
     EServerAllocReset,
     EFreeGPUMem,
     ETotalGPUMem,
     EGPULRUSize,
     ERefCount,
#endif
     EThemeContentUpdate,
     EEffectLookupFilePath,
     EEffectAdd,
     EUnloadIcon,
     EUnloadMultiIcon,
     EMemoryGood,
     EFreeRam,
     ERenderModeSwitch,
     EThemeServerStop,
     EFreeSharedMem,
     EAllocatedSharedMem,
     EAllocatedHeapMem,
     ETypefaceOffset
#ifdef HB_THEME_SERVER_MEMORY_REPORT
     ,ECreateMemoryReport
#endif
    };
//Rendering Modes
enum HbRenderingMode
{
	ESWRendering,
	EHWRendering
};

struct HbFreeRamNotificationData
{
	int bytesToFree;
	bool useSwRendering;
};	

#endif /* HBTHEMECOMMON_P_H */
