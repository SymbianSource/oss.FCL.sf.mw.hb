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

static HbThemeClient *clientInst=0;
static int sharedCacheOffset = -1;

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

QSizeF HbThemeClient::getSharedIconDefaultSize(const QString &iconPath)
{
    Q_D(HbThemeClient);
    return d->getSharedIconDefaultSize(iconPath);
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
                                                  const QColor &color)
{
    Q_D(HbThemeClient);
    return d->getSharedIconInfo(iconPath,
                                size,
                                aspectRatioMode, 
                                mode, 
                                mirrored, 
                                options,
                                color);
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
        QColor());
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
                        const QColor &color)
{
    Q_D(HbThemeClient);
    return d->getMultiPartIconInfo(multiPartIconList, multiPartIconData, size, aspectRatioMode, mode, mirrored, options, color);
}

void HbThemeClient::getThemeIndexTables(ThemeIndexTables &tables)
{
    Q_D(HbThemeClient);
    d->getThemeIndexTables(tables);
}

/**
 * HbThemeClient::getSharedStyleSheet()
 *
 * \a fielName  css filename
 * \a priority  layer priority
 */  
HbCss::StyleSheet *HbThemeClient::getSharedStyleSheet(const QString &fileName, HbLayeredStyleLoader::LayerPriority priority)
{
    int offset = -1;
    if( HbLayeredStyleLoader::Priority_Core == priority ) {
        offset = sharedCacheItemOffset(fileName);
    }
    if ( -1 != offset ) {
        HbCss::StyleSheet *styleSheet = HbMemoryUtils::getAddress<HbCss::StyleSheet>(HbMemoryManager::SharedMemory,offset);
        return styleSheet;
    }
    Q_D(HbThemeClient);
    return d->getSharedStyleSheet(fileName,priority);
}

/**
 * HbThemeClient::getSharedLayoutDefs()
 *
 * \a fileName
 * \a layout
 * \a section
 */  
LayoutDefinition *HbThemeClient::getSharedLayoutDefs(const QString &fileName,const QString &layout,const QString &section)
{
    int offset = sharedCacheItemOffset(fileName + layout + section);
    if ( -1 != offset ) {
       LayoutDefinition *layoutDefs = HbMemoryUtils::getAddress<LayoutDefinition>(HbMemoryManager::SharedMemory,offset);
       return layoutDefs;
    }
    Q_D(HbThemeClient);
    return d->getSharedLayoutDefs(fileName,layout,section);
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
 * HbThemeClient::globalCacheOffset()
 *
 */
int HbThemeClient::globalCacheOffset()
{
    if ( -1 == sharedCacheOffset ) {
        Q_D(HbThemeClient);
        sharedCacheOffset = d->globalCacheOffset();
    }
    return sharedCacheOffset;
}

/**
 * HbThemeClient::getSharedEffect()
 *
 * \a filePath
 */ 
HbEffectFxmlData *HbThemeClient::getSharedEffect(const QString &filePath)
{
    int offset = sharedCacheItemOffset(filePath);
    if ( -1 != offset ) {
       HbEffectFxmlData  *effectFxmlData = HbMemoryUtils::getAddress<HbEffectFxmlData>(HbMemoryManager::SharedMemory,offset);
       return effectFxmlData;
    }
    Q_D(HbThemeClient);
    return d->getSharedEffect(filePath);
}

/**
 * HbThemeClient::addSharedEffect()
 *
 * \a filePath
 */ 
bool HbThemeClient::addSharedEffect(const QString& filePath)
{
    int offset = sharedCacheItemOffset(filePath);
    if ( -1 != offset ) {
        // effect already added.
        return true;
    }
    Q_D(HbThemeClient);
    return d->addSharedEffect(filePath);
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
                               const QColor &color)
{
    Q_D(HbThemeClient);
    return d->unloadIcon(iconPath,
                         size,
                         aspectRatioMode, 
                         mode, 
                         mirrored, 
                         color);
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
                    const QColor &color)
{
    Q_D(HbThemeClient);
    return d->unLoadMultiIcon(iconPathList,
                         sizeList,
                         aspectRatioMode, 
                         mode, 
                         mirrored, 
                         color);
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
 * sharedCacheItemOffset  function returns the offset of the cache item
 * for the given key
 * \param key
 *
 */
int HbThemeClient::sharedCacheItemOffset(const QString & key)
{
    if ( -1 == sharedCacheOffset ) {
        // No IPC call happened so far for the globalCacheOffset
        globalCacheOffset();
    }
    if ( -1 != sharedCacheOffset ) {
        HbSharedCache *sharedCache = HbMemoryUtils::getAddress<HbSharedCache>(HbMemoryManager::SharedMemory,sharedCacheOffset);
        int count = sharedCache->count();
        for(int i = 0; i < count ; i++ ) {
            if (sharedCache->at(i).key == key) {
                return sharedCache->at(i).offset;
            }
        }
    }
    // item not found in the secondary cache.
    return -1;
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
                        const QColor &color)
{
    Q_D(HbThemeClient);
    return d->getMultiIconInfo(multiPartIconList, sizeList,aspectRatioMode, mode, mirrored, options, color);
}


