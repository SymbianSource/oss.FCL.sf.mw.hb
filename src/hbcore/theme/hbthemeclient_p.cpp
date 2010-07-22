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

#include "hbthemeclient_p.h"
#include "hbthemeclient_p_p.h"
#include "hbthemeutils_p.h"
#include "hbsharedmemorymanager_p.h"
#include "hbmemoryutils_p.h"

static const QLatin1String ResourceStylePath(":/themes/style/hbdefault/");

static HbThemeClient *clientInst = 0;

/**
 * Constructor
 */
HbThemeClient::HbThemeClient() :
        d_ptr(new HbThemeClientPrivate)
{
}

/**
 * HbThemeClient::connectToServer()
 */
bool HbThemeClient::connectToServer()
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbThemeClient);
    return d->connectToServer();
#else
    return false;
#endif
}

/**
 * HbThemeClient::getSharedIconInfo()
 *
 * Returns HbIconInfo structure to the clients. This basically contains information about the shared
 * icon resources on server's shared memory
 *
 * \a iconPath
 * \a size
 * \a aspectRatioMode
 * \a mode
 * \a mirrored
 * \a options
 * \a color
 *

 */
HbSharedIconInfo HbThemeClient::getSharedIconInfo(const QString& iconPath ,
                                                  const QSizeF &size,
                                                  Qt::AspectRatioMode aspectRatioMode,
                                                  QIcon::Mode mode,
                                                  bool mirrored,
                                                  HbIconLoader::IconLoaderOptions options,
                                                  const QColor &color,
                                                  HbRenderingMode renderMode)
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbThemeClient);
    return d->getSharedIconInfo(iconPath,
                                size,
                                aspectRatioMode,
                                mode,
                                mirrored,
                                options,
                                color,
                                renderMode);
#else
    Q_UNUSED(iconPath);
    Q_UNUSED(size);
    Q_UNUSED(aspectRatioMode);
    Q_UNUSED(mode);
    Q_UNUSED(mirrored);
    Q_UNUSED(options);
    Q_UNUSED(color);
    Q_UNUSED(renderMode);

    return HbSharedIconInfo();
#endif
}

/**
 * HbThemeClient::getSharedBlob()
 *
 * \a name
 */
QByteArray HbThemeClient::getSharedBlob(const QString &name)
{
#ifdef Q_OS_SYMBIAN
    HbSharedIconInfo info = getSharedIconInfo(
        name,
        QSizeF(),
        Qt::KeepAspectRatio,
        QIcon::Normal,
        false,
        HbIconLoader::NoOptions,
        QColor(),
        ESWRendering);
    return info.type == BLOB
        ? QByteArray::fromRawData(HbMemoryUtils::getAddress<char>(
                                      HbMemoryManager::SharedMemory,
                                      info.blobData.offset),
                                  info.blobData.dataSize)
        : QByteArray();
#else
    Q_UNUSED(name);
    return QByteArray();
#endif
}

HbSharedIconInfo HbThemeClient::getMultiPartIconInfo(const QStringList &multiPartIconList,
                        const HbMultiPartSizeData  &multiPartIconData ,
                        const QSizeF &size,
                        Qt::AspectRatioMode aspectRatioMode,
                        QIcon::Mode mode,
                        bool mirrored,
                        HbIconLoader::IconLoaderOptions options,
                        const QColor &color,
                        HbRenderingMode renderMode)
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbThemeClient);
    return d->getMultiPartIconInfo(multiPartIconList, multiPartIconData, size,
                                   aspectRatioMode, mode, mirrored, options, color, renderMode);
#else
    Q_UNUSED(multiPartIconList);
    Q_UNUSED(multiPartIconData);
    Q_UNUSED(size);
    Q_UNUSED(aspectRatioMode);
    Q_UNUSED(mode);
    Q_UNUSED(mirrored);
    Q_UNUSED(options);
    Q_UNUSED(color);
    Q_UNUSED(renderMode);

    return HbSharedIconInfo();
#endif
}

/**
 * HbThemeClient::getSharedStyleSheet()
 *
 * \a filePath  css filepath. Only acceptable path separator is '/'.
 * \a priority  layer priority
 */
HbCss::StyleSheet *HbThemeClient::getSharedStyleSheet(const QString &filePath,
                                                      HbLayeredStyleLoader::LayerPriority priority)
{
    HbCss::StyleSheet *styleSheet = 0;
#ifdef Q_OS_SYMBIAN
    const QString filePathFixed = QDir::fromNativeSeparators(filePath);

    bool requestFromServer = true;
    if (filePathFixed.startsWith(QLatin1Char(':')) && !filePathFixed.startsWith(ResourceStylePath)) {
        //filePathFixed is located in application resource, parse it on client side.
        requestFromServer = false;
    }
    if (requestFromServer) {
        int offset = -1;
        if(HbLayeredStyleLoader::Priority_Core == priority) {
            offset = sharedCacheItemOffset(HbSharedCache::Stylesheet, filePathFixed);
        }
        if ( -1 != offset ) {
            styleSheet =
                    HbMemoryUtils::getAddress<HbCss::StyleSheet>(HbMemoryManager::SharedMemory,
                                                                 offset);
        } else {
            Q_D(HbThemeClient);
            styleSheet = d->getSharedStyleSheet(filePathFixed, priority);
        }
    }
#else
    Q_UNUSED(filePath);
    Q_UNUSED(priority);
#endif
    return styleSheet;
}

/**
 * HbThemeClient::getSharedLayoutDefs()
 * \a filePath  layout definition filepath. Only acceptable path separator is '/'.
 * \a layout  layout name
 * \a section section name
 */
HbWidgetLoader::LayoutDefinition *HbThemeClient::getSharedLayoutDefs(const QString &filePath,
                                                                     const QString &layout,
                                                                     const QString &section)
{
    HbWidgetLoader::LayoutDefinition *layoutDefinition = 0;
#ifdef Q_OS_SYMBIAN
    const QString filePathFixed = QDir::fromNativeSeparators(filePath);

    bool requestFromServer = true;
    if (filePathFixed.startsWith(QLatin1Char(':')) && !filePathFixed.startsWith(ResourceStylePath)) {
        //filePathFixed is located in application resource, parse it on client side.
        requestFromServer = false;
    }
    if (requestFromServer) {
        int offset = sharedCacheLayoutDefinitionOffset(filePathFixed, layout, section);
        if (offset != -1) {
           layoutDefinition =
               HbMemoryUtils::getAddress<HbWidgetLoader::LayoutDefinition>(
                       HbMemoryManager::SharedMemory, offset);
        } else {
            Q_D(HbThemeClient);
            layoutDefinition = d->getSharedLayoutDefs(filePathFixed, layout, section);
        }
    }
#else
    Q_UNUSED(filePath);
    Q_UNUSED(layout);
    Q_UNUSED(section);
#endif
    return layoutDefinition;
}
/**
 * HbThemeClient::deviceProfiles()
 */
HbDeviceProfileList *HbThemeClient::deviceProfiles()
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbThemeClient);
    return d->deviceProfiles();
#else
    return 0;
#endif
}

/**
 * HbThemeClient::typefaceInfo()
 */
HbTypefaceInfoVector *HbThemeClient::typefaceInfo()
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbThemeClient);
    return d->typefaceInfo();
#else
    return 0;
#endif
}

/**
 * HbThemeClient::notifyForegroundLostToServer()
 *
 */
void HbThemeClient::notifyForegroundLostToServer()
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbThemeClient);
    d->notifyForegroundLostToServer();
#endif
}

/**
 * HbThemeClient::getSharedEffect()
 *
 * \a filePath. Only acceptable path separator is '/'.
 */
HbEffectFxmlData *HbThemeClient::getSharedEffect(const QString &filePath)
{
#ifdef Q_OS_SYMBIAN
    const QString filePathFixed = QDir::fromNativeSeparators(filePath);

    int offset = sharedCacheItemOffset(HbSharedCache::Effect, filePathFixed);
    if ( -1 != offset ) {
       HbEffectFxmlData  *effectFxmlData =
               HbMemoryUtils::getAddress<HbEffectFxmlData>(HbMemoryManager::SharedMemory, offset);
       return effectFxmlData;
    }
    Q_D(HbThemeClient);
    return d->getSharedEffect(filePathFixed);
#else
    Q_UNUSED(filePath);
    return 0;
#endif
}

/**
 * HbThemeClient::addSharedEffect()
 *
 * \a filePath. Only acceptable path separator is '/'.
 */
bool HbThemeClient::addSharedEffect(const QString& filePath)
{
#ifdef Q_OS_SYMBIAN
    const QString filePathFixed = QDir::fromNativeSeparators(filePath);

    int offset = sharedCacheItemOffset(HbSharedCache::Effect, filePathFixed);
    if ( -1 != offset ) {
        // effect already added.
        return true;
    }
    Q_D(HbThemeClient);
    return d->addSharedEffect(filePathFixed);
#else
    Q_UNUSED(filePath);
    return false;
#endif
}

/**
 * HbThemeClient::unloadIcon()
 *
 * \a iconPath
 * \a size
 * \a aspectRatioMode
 * \a mode
 * \a mirrored
 * \a options
 * \a color

 */
void HbThemeClient::unloadIcon(const QString& iconPath ,
                               const QSizeF &size,
                               Qt::AspectRatioMode aspectRatioMode,
                               QIcon::Mode mode,
                               bool mirrored,
                               const QColor &color,
                               HbRenderingMode renderMode)
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbThemeClient);
    return d->unloadIcon(iconPath,
                         size,
                         aspectRatioMode,
                         mode,
                         mirrored,
                         color,
                         renderMode);
#else
    Q_UNUSED(iconPath);
    Q_UNUSED(size);
    Q_UNUSED(aspectRatioMode);
    Q_UNUSED(mode);
    Q_UNUSED(mirrored);
    Q_UNUSED(color);
    Q_UNUSED(renderMode);
#endif
}

/**
 * HbThemeClient::unLoadMultiIcon()
 *
 * \a iconPathList
 * \a sizeList
 * \a aspectRatioMode
 * \a mode
 * \a mirrored
 * \a color
 */
void HbThemeClient::unLoadMultiIcon(const QStringList& iconPathList,
                    const QVector<QSizeF> &sizeList,
                    Qt::AspectRatioMode aspectRatioMode,
                    QIcon::Mode mode,
                    bool mirrored,
                    const QColor &color,
                    HbRenderingMode renderMode)
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbThemeClient);
    return d->unLoadMultiIcon(iconPathList,
                         sizeList,
                         aspectRatioMode,
                         mode,
                         mirrored,
                         color,
                         renderMode);
#else
    Q_UNUSED(iconPathList);
    Q_UNUSED(sizeList);
    Q_UNUSED(aspectRatioMode);
    Q_UNUSED(mode);
    Q_UNUSED(mirrored);
    Q_UNUSED(color);
    Q_UNUSED(renderMode);
#endif
}

/**
 * HbThemeClient::~HbThemeClient()
 */
HbThemeClient::~HbThemeClient()
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbThemeClient);
    delete d;
#endif
}

/**
 * HbThemeClient::clientConnected()
 */
bool HbThemeClient::clientConnected() const
{
#ifdef Q_OS_SYMBIAN
    Q_D(const HbThemeClient);
    return d->clientConnected;
#else
    return false;
#endif
}

/**
 * HbThemeClient::global()
 */
HbThemeClient *HbThemeClient::global()
{
    if ( !clientInst ) {
        clientInst = new HbThemeClient;
    }

    return clientInst;
}

/**
 * HbThemeClient::releaseInstance()
 */
void HbThemeClient::releaseInstance()
{
    delete clientInst;
    clientInst = 0;
}

/**
 * returns the offset of the cache item for the given key
 * \param key
 *
 */
int HbThemeClient::sharedCacheItemOffset(HbSharedCache::ItemType type, const QString &key)
{
    int offset = -1;
#ifdef Q_OS_SYMBIAN
    HbSharedCache *cache = HbSharedCache::instance();
    if (cache) {
        offset = cache->offset(type, key);
    }
#else
    Q_UNUSED(type);
    Q_UNUSED(key);
#endif
    return offset;
}

/**
 * returns the offset of the layout definition for the given file, layout and section.
 */
int HbThemeClient::sharedCacheLayoutDefinitionOffset(const QString &fileName,
                                         const QString &layout,
                                         const QString &section)
{
    int offset = -1;
#ifdef Q_OS_SYMBIAN
    HbSharedCache *cache = HbSharedCache::instance();
    if (cache) {
        offset = cache->layoutDefinitionOffset(fileName, layout, section);
    }
#else
    Q_UNUSED(fileName);
    Q_UNUSED(layout);
    Q_UNUSED(section);
#endif
    return offset;
}

#ifdef HB_THEME_SERVER_MEMORY_REPORT
/**
 * createMemoryReport  creates memory report from current shared memory
 *
 */
void HbThemeClient::createMemoryReport() const
{
#ifdef Q_OS_SYMBIAN
    Q_D(const HbThemeClient);
    d->createMemoryReport();
#endif
}
#endif

/**
 * getMultiIconInfo  function returns a list of HbSharedIconInfo
 * for the given list of frameitems.
 */
HbSharedIconInfoList HbThemeClient::getMultiIconInfo(const QStringList &multiPartIconList,
                        const QVector<QSizeF>  &sizeList ,
                        Qt::AspectRatioMode aspectRatioMode,
                        QIcon::Mode mode,
                        bool mirrored,
                        HbIconLoader::IconLoaderOptions options,
                        const QColor &color,
                        HbRenderingMode renderMode)
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbThemeClient);
    return d->getMultiIconInfo(multiPartIconList, sizeList,aspectRatioMode, mode,
                               mirrored, options, color, renderMode);
#else
    Q_UNUSED(multiPartIconList);
    Q_UNUSED(sizeList);
    Q_UNUSED(aspectRatioMode);
    Q_UNUSED(mode);
    Q_UNUSED(mirrored);
    Q_UNUSED(options);
    Q_UNUSED(color);
    Q_UNUSED(renderMode);

    return HbSharedIconInfoList();
#endif
}

/**
 * freeSharedMemory  gets free memory in shared chunk
 *
 */
int HbThemeClient::freeSharedMemory()
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbThemeClient);
    return d->freeSharedMemory();
#else
     return 0;
#endif
}

/**
 * allocatedSharedMemory  gets allocated memory in shared chunk
 *
 */
int HbThemeClient::allocatedSharedMemory()
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbThemeClient);
    return d->allocatedSharedMemory();
#else
    return 0;
#endif
}

/**
 * allocatedHeapMemory  gets free memory in shared chunk
 *
 */
int HbThemeClient::allocatedHeapMemory()
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbThemeClient);
    return d->allocatedHeapMemory();
#else
    return 0;
#endif
}

/**
 * switchRenderingMode  IPC sent to themeserver
 *
 */
bool HbThemeClient::switchRenderingMode(HbRenderingMode renderMode)
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbThemeClient);    
    return d->switchRenderingMode(renderMode);    
#else
    Q_UNUSED(renderMode);
    return true;
#endif
}

void HbThemeClient::setTheme(const QString &theme)
{
    Q_D(HbThemeClient);
    d->setTheme(theme);
}
