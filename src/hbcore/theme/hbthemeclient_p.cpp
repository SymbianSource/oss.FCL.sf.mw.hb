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
#include "hbsharedmemorymanager_p.h"
#include "hbmemoryutils_p.h"

static const QLatin1String ResourceStylePath(":/themes/style/hbdefault/");

static HbThemeClient *clientInst = 0;

/**
 * Constructor
 */
HbThemeClient::HbThemeClient():d_ptr(new HbThemeClientPrivate)
{

}

/**
 * HbThemeClient::connectToServer()
 */
bool HbThemeClient::connectToServer()
{
    Q_D(HbThemeClient);
    return d->connectToServer();
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
    Q_D(HbThemeClient);
    return d->getSharedIconInfo(iconPath,
                                size,
                                aspectRatioMode,
                                mode,
                                mirrored,
                                options,
                                color,
                                renderMode);
}

/**
 * HbThemeClient::getSharedBlob()
 *
 * \a name
 */
QByteArray HbThemeClient::getSharedBlob(const QString &name)
{
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
    Q_D(HbThemeClient);
    return d->getMultiPartIconInfo(multiPartIconList, multiPartIconData, size,
                                   aspectRatioMode, mode, mirrored, options, color, renderMode);
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
    const QString filePathFixed = QDir::fromNativeSeparators(filePath);

    HbCss::StyleSheet *styleSheet = 0;
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
    const QString filePathFixed = QDir::fromNativeSeparators(filePath);

    HbWidgetLoader::LayoutDefinition *layoutDefinition = 0;
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
    return layoutDefinition;
}
/**
 * HbThemeClient::deviceProfiles()
 */
HbDeviceProfileList *HbThemeClient::deviceProfiles()
{
    Q_D(HbThemeClient);
    return d->deviceProfiles();
}

/**
 * HbThemeClient::typefaceInfo()
 */
HbTypefaceInfoVector *HbThemeClient::typefaceInfo()
{
    Q_D(HbThemeClient);
    return d->typefaceInfo();
}

/**
 * HbThemeClient::notifyForegroundLostToServer()
 *
 */
void HbThemeClient::notifyForegroundLostToServer()
{
    Q_D(HbThemeClient);
    d->notifyForegroundLostToServer();
}

/**
 * HbThemeClient::getSharedEffect()
 *
 * \a filePath. Only acceptable path separator is '/'.
 */
HbEffectFxmlData *HbThemeClient::getSharedEffect(const QString &filePath)
{
    const QString filePathFixed = QDir::fromNativeSeparators(filePath);

    int offset = sharedCacheItemOffset(HbSharedCache::Effect, filePathFixed);
    if ( -1 != offset ) {
       HbEffectFxmlData  *effectFxmlData =
               HbMemoryUtils::getAddress<HbEffectFxmlData>(HbMemoryManager::SharedMemory, offset);
       return effectFxmlData;
    }
    Q_D(HbThemeClient);
    return d->getSharedEffect(filePathFixed);
}

/**
 * HbThemeClient::addSharedEffect()
 *
 * \a filePath. Only acceptable path separator is '/'.
 */
bool HbThemeClient::addSharedEffect(const QString& filePath)
{
    const QString filePathFixed = QDir::fromNativeSeparators(filePath);

    int offset = sharedCacheItemOffset(HbSharedCache::Effect, filePathFixed);
    if ( -1 != offset ) {
        // effect already added.
        return true;
    }
    Q_D(HbThemeClient);
    return d->addSharedEffect(filePathFixed);
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
    Q_D(HbThemeClient);
    return d->unloadIcon(iconPath,
                         size,
                         aspectRatioMode,
                         mode,
                         mirrored,
                         color,
                         renderMode);
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
    Q_D(HbThemeClient);
    return d->unLoadMultiIcon(iconPathList,
                         sizeList,
                         aspectRatioMode,
                         mode,
                         mirrored,
                         color,
                         renderMode);
}

/**
 * HbThemeClient::~HbThemeClient()
 */
HbThemeClient::~HbThemeClient()
{
    Q_D(HbThemeClient);
    delete d;
}

/**
 * HbThemeClient::clientConnected()
 */
bool HbThemeClient::clientConnected() const
{
    Q_D(const HbThemeClient);
    return d->clientConnected;
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
    HbSharedCache *cache = HbSharedCache::instance();
    if (cache) {
        offset = cache->offset(type, key);
    }
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
    HbSharedCache *cache = HbSharedCache::instance();
    if (cache) {
        offset = cache->layoutDefinitionOffset(fileName, layout, section);
    }
    return offset;
}

#ifdef HB_THEME_SERVER_MEMORY_REPORT
/**
 * createMemoryReport  creates memory report from current shared memory
 *
 */
void HbThemeClient::createMemoryReport() const
{
    Q_D(const HbThemeClient);
    d->createMemoryReport();
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
    Q_D(HbThemeClient);
    return d->getMultiIconInfo(multiPartIconList, sizeList,aspectRatioMode, mode,
                               mirrored, options, color, renderMode);
}

/**
 * freeSharedMemory  gets free memory in shared chunk
 *
 */
int HbThemeClient::freeSharedMemory()
{
    Q_D(HbThemeClient);
    return d->freeSharedMemory();
}

/**
 * allocatedSharedMemory  gets allocated memory in shared chunk
 *
 */
int HbThemeClient::allocatedSharedMemory()
{
    Q_D(HbThemeClient);
    return d->allocatedSharedMemory();
}

/**
 * allocatedHeapMemory  gets free memory in shared chunk
 *
 */
int HbThemeClient::allocatedHeapMemory()
{
    Q_D(HbThemeClient);
    return d->allocatedHeapMemory();
}

/**
 * switchRenderingMode  IPC sent to themeserver
 *
 */
bool HbThemeClient::switchRenderingMode(HbRenderingMode renderMode)
{
    Q_D(HbThemeClient);    
    return d->switchRenderingMode(renderMode);    
}

void HbThemeClient::setTheme(const QString &theme)
{
    Q_D(HbThemeClient);
    d->setTheme(theme);
}
