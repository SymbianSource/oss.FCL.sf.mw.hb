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

#include "hbthemeserver_generic_p_p.h"

#include <QLabel>
#include <QLocalServer>
#include <QLocalSocket>
#include <QSharedMemory>
#include <QSettings>
#include <QDebug>
#include <QList>
#include <QIcon>
#include <QFile>
#include <QCoreApplication>
#include <QPainter>
#include <QStringList>
#include <QPixmap>

#include <hbinstance.h>

#include "hbthemecommon_p.h"
#include "hbmemoryutils_p.h"
#include "hbthemeserverutils_p.h"
#include "hbiconcacheitemcreator_p.h"
#include "hbiconloader_p.h"
#include "hbcache_p.h"
#include "hbdeviceprofiledatabase_p.h"
#include "hbpixmapiconimpl_p.h"
#include "hbpixmapiconprocessor_p.h"
#include "hblayeredstyleloader_p.h"
#include "hbthemesystemeffect_p.h"
#include "hbsharedmemorymanager_p.h"

/*!
  @hbserver
  \class HbThemeServerPrivate
  \brief HbThemeServerPrivate implements the theme server
*/

#ifdef THEME_SERVER_TRACES
extern QLabel *testLabel;
#endif

// 5 MB GPU & CPU cache size
#define GPU_CACHE_SIZE 0x500000
#define CPU_CACHE_SIZE 0x500000

/*!
  \fn HbThemeServerPrivate::HbThemeServerPrivate()
  Constructor
  \a parent
*/
#ifdef QT_DEBUG
HbThemeServerPrivate::HbThemeServerPrivate(QWidget *parent): QMainWindow(parent), server(new QLocalServer(this))
#else
HbThemeServerPrivate::HbThemeServerPrivate(): server(new QLocalServer(this))
#endif
{
    iThemeSelectionClient = 0;
    sessionList.clear();
#ifdef QT_DEBUG
    setWindowTitle("Theme Server");
#endif
    // Using QScopedPointer so that it deallocates memory
    // when std::badalloc exception occurs.
    QScopedPointer <HbIconDataCache> tempIconCache(new HbIconDataCache());
    QScopedPointer <HbCache> tempCssCache(new HbCache());
    iconCache = tempIconCache.take();
    cssCache = tempCssCache.take();
    setMaxGpuCacheSize(GPU_CACHE_SIZE);
    setMaxCpuCacheSize(CPU_CACHE_SIZE);
}

/*!
  \fn HbThemeServerPrivate::~HbThemeServerPrivate()
  Destructor
*/
HbThemeServerPrivate::~HbThemeServerPrivate()
{
    server->close();
    delete server; // Order of Deletion needs to be maintain ,as the QLocalServer should delete first before deleting Server data so All sessions will be cleanup first.
    delete iconCache;
    delete cssCache;
}

/*!
  \fn HbThemeServerPrivate::start()
  start the themeserver
*/
bool HbThemeServerPrivate::start()
{
    bool success = false;
    if (!server->listen(THEME_SERVER_NAME)) {
        qWarning("Unable to start the server: %s.", server->errorString().toLatin1().data());
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        if (server->serverError() == QAbstractSocket::AddressInUseError) {
            qDebug() << "Trying to delete temporary file hbthemeserver";
            QFile::remove("/tmp/hbthemeserver");
            success = start();
        }
#endif
        return success;
    }
#ifdef QT_DEBUG
    statusLabel.setText("Theme Server Started");
    setCentralWidget(&statusLabel);
#endif
    connect(server, SIGNAL(newConnection()), this, SLOT(newClientConnected()));
    success = true;
    return success;
}

/*!
  \fn HbThemeServerPrivate::insertIconCacheItem()
  Insert item into the icon cache
  \a key denotes the unique identifier for the cache item that is to be inserted into the cache.
  \a item denotes the cache item that is to be inserted
*/
bool HbThemeServerPrivate::insertIconCacheItem(const HbIconKey &key,  HbIconCacheItem *item)
{
    return (iconCache->insert(key, item));
}

/*!
  \fn HbThemeServerPrivate::insertCssCacheItem()
  Insert item into the css cache
  \a key denotes the unique identifier for the cache item that is to be inserted into the cache.
  \a item denotes the cache item that is to be inserted
*/
bool HbThemeServerPrivate::insertCssCacheItem(const QString &key,  HbCacheItem *item)
{
    return (cssCache->insert(key, item));
}

/*!
  \fn HbThemeServerPrivate::iconCacheItem()
  Find an item in the icon cache
  \a key denotes the unique identifier for the cache item that is to be found in the cache.
*/
HbIconCacheItem *HbThemeServerPrivate::iconCacheItem(const HbIconKey &key)
{
    return(iconCache->getCacheItem(key, false));
}

/*!
  \fn HbThemeServerPrivate::cssCacheItem()
  Find an item in the css cache
  \a key denotes the unique identifier for the cache item that is to be found in the cache.
*/
HbCacheItem *HbThemeServerPrivate::cssCacheItem(const QString &key)
{
    return(cssCache->cacheItem(key));
}

/*!
  \fn HbThemeServerPrivate::setMaxGpuCacheSize()
  Provides a mechanism for setting the Gpu cache limit
  \a size denotes the cache limit in bytes e.g. size = 0x500000
*/
void HbThemeServerPrivate::setMaxGpuCacheSize(int size)
{
    iconCache->setMaxGpuCacheSize(size);
}

/*!
  \fn HbThemeServerPrivate::setMaxCpuCacheSize()
  Provides a mechanism for setting the Cpu cache limit
  \a size denotes the cache limit in bytes e.g. size = 0x500000
*/
void HbThemeServerPrivate::setMaxCpuCacheSize(int size)
{
    iconCache->setMaxCpuCacheSize(size);
}

/*!
  \fn HbThemeServerPrivate::removeIconCacheItem()
  Remove an item from iconcache corresponding to key.
  \a key denotes the unique identifier for the cache item that is to be removed from the cache.
*/
void HbThemeServerPrivate::removeIconCacheItem(const HbIconKey &key)
{
    iconCache->remove(key);
}

/*!
  \fn HbThemeServerPrivate::removeCssCacheItem()
  Remove item from css cache corresponding to key.
  \a key denotes the unique identifier for the cache item that is to be removed from the cache.
*/
void HbThemeServerPrivate::removeCssCacheItem(const QString &key)
{
    cssCache->remove(key);
}

/*!
  \fn HbThemeServerPrivate::clearIconCache()
  Clears the icon cache
*/
void HbThemeServerPrivate::clearIconCache()
{
    iconCache->clear();
}

/*!
  \fn HbThemeServerPrivate::clearCssCache()
  Clears the css cache
*/
void HbThemeServerPrivate::clearCssCache()
{
    cssCache->clear();
}


/*!
  data.type = INVALID_FORMAT;
  \fn HbThemeServerPrivate::handleThemeSelection()
  Handle change of theme
  \a newTheme
*/
void HbThemeServerPrivate::handleThemeSelection(const QString &newTheme)
{
#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO << "  theme=" << newTheme;
#endif
    // Modify the QSettings to store the applied theme
    QSettings settings(QLatin1String(ORGANIZATION), QLatin1String(THEME_COMPONENT));
    QString prevTheme = settings.value(CURRENT_THEME_KEY).toString();
    if (prevTheme == newTheme) {
        // Theme did not change, return.
        return;
    }
    // Clear cached icons and session data
    clearIconCache();
    HbThemeServerSession *session;
    foreach(session, sessionList) {
        session->clearSessionIconData();
    }

    QString cleanThemeName = newTheme.trimmed();
    settings.remove("currenttheme"); //temporary
    settings.setValue(CURRENT_THEME_KEY, cleanThemeName);
    settings.sync();

    // Register new system effects
    HbThemeSystemEffect::handleThemeChange(cleanThemeName);

    HbThemeServerRequest requestType;
    requestType = EThemeSelection;
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (int)requestType;
    out << cleanThemeName;
    writeToClients(block);
}

void HbThemeServerPrivate::handleContentUpdate(const QStringList &fileNames)
{
    // If list is empty clear all themed content
    if (!fileNames.count()) {
        // Clear icons
        HbThemeServerSession *session;
        foreach(session, sessionList) {
            session->clearSessionIconData();
        }
        iconCache->clear();

        // Clear effects
        HbThemeServerUtils::clearSharedEffects();

        // Clear stylesheets
        QHash<QString, HbCacheItem*>::const_iterator itEnd(themePriorityItems.constEnd());
        for (QHash<QString, HbCacheItem*>::const_iterator iter = themePriorityItems.constBegin();
                iter != itEnd;
                ++iter) {
            cssCache->cacheHandle().remove(iter.key());
            HbThemeServerSession *session;
            foreach(session, sessionList) {
                session->removeSessionCssItem(iter.key());
            }
        }
        themePriorityItems.clear();

        return;
    }

    // Else delete only specified files
    for (int i=0; i<fileNames.count();i++) {
        QString filename = fileNames.at(i);

        // Stylesheet
        if (themePriorityItems.contains(filename)) {
            cssCache->cacheHandle().remove(filename);
            themePriorityItems.remove(filename);
            HbThemeServerSession *session;
            foreach(session, sessionList) {
                session->removeSessionCssItem(filename);
            }
            break;
        }
        // Effect
        if (HbThemeServerUtils::removeSharedEffect(filename)) {
            break;
        }

        // Icon
        QVector<const HbIconKey *> keys = iconCache->getKeys(filename);
        for (int j = 0; j<keys.count();j++) {
            HbThemeServerSession *session;
            foreach(session, sessionList) {
                session->removeSessionIconItem(*keys.at(j));
            }
            iconCache->remove(*keys.at(j),false);
        }
    }
}

/*!
  \fn HbThemeServerPrivate::writeToClients()
  Write to all clients
  \a block
*/
void HbThemeServerPrivate::writeToClients(QByteArray &block)
{
    HbThemeServerSession *session;
#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO << "socketlist count: " << sessionList.count();
#endif
    foreach(session, sessionList) {
        QLocalSocket * curSocket = session->clientConnection();
        if (iThemeSelectionClient != curSocket) {
            curSocket->write(block);
        }
    }
}

/*!
  \fn HbThemeServerPrivate::setThemeSelectionClient()
  Set the theme selection client
  \a socket
*/
void HbThemeServerPrivate::setThemeSelectionClient(QLocalSocket *socket)
{
    iThemeSelectionClient = socket;
}

/*!
  \fn HbThemeServerPrivate::newClientConnected()
  Creates a new session with the server.
*/
void HbThemeServerPrivate::newClientConnected()
{
    QLocalSocket * newClient = server->nextPendingConnection();
    if (newClient) {
        HbThemeServerSession * newSession =
            new HbThemeServerSession(newClient, this);
        // Store list of client connected to server
        sessionList.append(newSession);
#ifdef THEME_SERVER_TRACES
        qDebug() << "Total No of Connection after addition = " << sessionList.count();
#endif
        connect(newClient, SIGNAL(disconnected()), this, SLOT(removeFromList()));
    }
}

/*!
  \fn HbThemeServerPrivate::removeFromList()
  Remove a session from list
*/
void HbThemeServerPrivate::removeFromList()
{
    QList<HbThemeServerSession*>::const_iterator itEnd(sessionList.constEnd());

    for (QList<HbThemeServerSession*>::const_iterator iter = sessionList.constBegin();
            iter != itEnd;
            ++iter) {
        if ((*iter)->clientConnection() == (QLocalSocket *)sender()) {
            sessionList.removeOne((*iter));
            delete(*iter);
            break;
        }
    }
    qDebug() << "Total No of Connection after deletion = " << sessionList.count();
}

//Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
int HbThemeServerPrivate ::cacheIconCount() const
{
    return iconCache->count();
}

int HbThemeServerPrivate::freeVectorMemory()
{
    return iconCache->freeVectorMemory();
}

int HbThemeServerPrivate::freeRasterMemory()
{
    return iconCache->freeRasterMemory();
}

int HbThemeServerPrivate::lastAddedRefCount()
{
    return iconCache->lastAddedRefCount();
}

int HbThemeServerPrivate::lastAddedItemMem()
{
    return iconCache->lastAddedItemMem();
}

int HbThemeServerPrivate::lastRemovedItemMem()
{
    return iconCache->lastRemovedItemMem();
}

int HbThemeServerPrivate::lastRemovedItemRfCount()
{
    return iconCache->lastRemovedItemRfCount();
}

bool HbThemeServerPrivate::enableCache(bool cacheIt)
{
    return iconCache->enableCache(cacheIt);
}

int HbThemeServerPrivate::cacheHitCount()
{
    return iconCache->cacheHitCount();
}

int HbThemeServerPrivate::cacheMissCount()
{
    return iconCache->cacheMissCount();
}

int HbThemeServerPrivate::serverHeapSize()
{
    return 0;
}

void HbThemeServerPrivate::cleanVectorLRUList()
{
    iconCache->cleanVectorLRUList();
}

void HbThemeServerPrivate::cleanRasterLRUList()
{
    iconCache->cleanRasterLRUList();
}

int HbThemeServerPrivate::rasterLruCount()
{
    return iconCache->rasterLruCount();
}

int HbThemeServerPrivate::vectorLruCount()
{
    return iconCache->vectorLruCount();
}
#endif

//***********************************
//HbThemeServerSession - implementations
//***********************************
/*!
  @hbserver
  \class HbThemeServerSession
  \brief HbThemeServerSession implements the sessions associated with the server.
  The Theme server maintains a list of sessions, each session corresponding to a client.
  Each session in turn is responsible for keeping track of client specific resources.
  In case of Icon caching, a session maintains a list of Icon keys.
  A  key is added to the list only if the corresponding cache item has been successfully inserted into cache
  or if a previously cached icon is requested and successfully found in the cache.
  In case of client crash or graceful exit of client, the server goes through its list of sessions,
  finds the session corresponding to the client, removes the session from its session list and deletes the session.
  In the session destructor, the session specific resources are cleaned up.
  In case of icon caching, the list of icon keys is traversed and a remove operation is performed on the cache
  items corresponding to these keys. On removal, the items reference count is decremented and in case the
  reference count becomes 0, the cache item instance gets added to the back of the LRU list.

*/

/*!
  \fn HbThemeServerSession::HbThemeServerSession()
  Constructor
  \a aClientConnection indicates the local socket that is contained within this session
  \a aServer denotes the handle to the theme server
*/
HbThemeServerSession::HbThemeServerSession(QLocalSocket *aClientConnection, HbThemeServerPrivate *aServer)
{
    iServer = aServer;
    iClientConnection = aClientConnection;
    sessionIconData.clear();
    sessionCssData.clear();
    connect(iClientConnection, SIGNAL(readyRead()), this, SLOT(readDataFromClient()));
    connect(iClientConnection, SIGNAL(disconnected()), iClientConnection, SLOT(deleteLater()));
}

/*!
  \fn HbThemeServerSession::~HbThemeServerSession()
  Destructor
*/
HbThemeServerSession::~HbThemeServerSession()
{
    //Remove icon related-session-specific data
    QList<HbIconKey>::const_iterator itEnd(sessionIconData.constEnd());
    for (QList<HbIconKey>::const_iterator iter = sessionIconData.constBegin();
            iter != itEnd;
            ++iter) {
        iServer->removeIconCacheItem(*iter);
    }

    //Remove css related-session-specific data
    QList<QString>::const_iterator iterEnd(sessionCssData.constEnd());
    for (QList<QString>::const_iterator iter = sessionCssData.constBegin();
            iter != iterEnd;
            ++iter) {
        iServer->removeCssCacheItem(*iter);
    }
    sessionIconData.clear();
    sessionCssData.clear();
}

/*!
  \fn HbThemeServerSession::clientConnection()
  Returns a handle to the local socket contained by the session
*/
QLocalSocket * HbThemeServerSession::clientConnection()
{
    return iClientConnection;
}

/*!
  \fn HbThemeServerSession::readDataFromClient()
  Slot that is reponsible for reading data from the client
  Is responsible for performing operations such as creating cache items and storing into cache
  and writing back data to the client
*/
void HbThemeServerSession::readDataFromClient()
{
#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO;
#endif
    HbSharedIconInfo data;
    data.type = INVALID_FORMAT;
    HbThemeServerRequest requestType;
    QByteArray inputByteArray = ((QLocalSocket *)sender())->readAll();
    if (inputByteArray.size() > 0) {
        QDataStream inputDataStream(inputByteArray);
        int clue;
        inputDataStream >> clue;
        requestType = (HbThemeServerRequest)clue;

#ifdef THEME_SERVER_TRACES
        qDebug() << Q_FUNC_INFO << "recognizer: " << requestType;
#endif

//Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
        QByteArray outputByteArray;
        QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
        outputDataStream << (int)requestType;
#endif
        switch (requestType) {
        case EStyleSheetLookup: {
            QString fileName;
            int priority;
            HbLayeredStyleLoader::LayerPriority layerdPriority;
            inputDataStream >> fileName;
            inputDataStream >> priority;
            layerdPriority = (HbLayeredStyleLoader::LayerPriority) priority;
            QByteArray output = handleStyleSheetLookup((int) requestType, fileName, layerdPriority);
            ((QLocalSocket *)sender())->write(output);
            break;
        }
        case EWidgetMLLookup: {
            QString filename;
            QString section;
            QString layout;

            inputDataStream >> filename;
            inputDataStream >> layout;
            inputDataStream >> section;
            // handle the shared Widgetml look up.
            QByteArray output = iServer->handleSharedWidgetMLLookup(filename, layout, section);
            ((QLocalSocket *)sender())->write(output);
            break;
        }
        case EDeviceProfileOffset: {
            int offset = -1;
            HbDeviceProfileDatabase *deviceProfileDatabase =
                HbDeviceProfileDatabase::instance(HbMemoryManager::SharedMemory);

            if (deviceProfileDatabase) {
                offset = deviceProfileDatabase->deviceProfilesOffset();
            }
            // offset will be -1 if the deviceProfileDatabase is NULL.
            QByteArray outputByteArray;
            QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
            outputDataStream << requestType;
            outputDataStream << offset;
            ((QLocalSocket *)sender())->write(outputByteArray);
            break;
        }
        case ESecondaryCacheOffset: {
            int offset = HbThemeServerUtils::sharedCacheOffset();
            QByteArray outputByteArray;
            QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
            outputDataStream << requestType;
            outputDataStream << offset;
            ((QLocalSocket *)sender())->write(outputByteArray);
            break;
        }
        case EEffectLookupFilePath:
        case EEffectAdd: {
            QString fileName;
            inputDataStream >> fileName;
            QByteArray output = iServer->handleSharedEffectAddAndFileLookup((int) requestType, fileName);
            ((QLocalSocket *)sender())->write(output);
            break;
        }
        case EThemeSelection: {
            QString themename;
            QLocalSocket *themeSelectionClient = (QLocalSocket *)sender();
            inputDataStream >> themename;
            iServer->setThemeSelectionClient(themeSelectionClient);
            iServer->handleThemeSelection(themename);
            break;
        }
        case EThemeContentUpdate: {
            QStringList themedItems;
            inputDataStream >> themedItems;
            iServer->handleContentUpdate(themedItems);
            HbThemeServerRequest requestType = EThemeContentUpdate;
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out << (int)requestType;
            out << themedItems;
            iServer->writeToClients(block);
            break;
        }
        case EThemeServerStop: {
            //This segment is used by HbThemeApiWrapperUi.
            //When no HbApplication is open and HbThemeserver has only HbThemeApiWrapperUi as client, making sessionListCount =1
            //HbThemeserver is closed otherwise warning is shown.
            //QCoreApplication::quit() removes the UI of HbThemeServer. Destructor of HbThemeServer will be called where it was instantiated.
            if (iServer->sessionListCount() == 1) {
#ifdef QT_DEBUG
                iServer->close();
#endif
                ((QLocalSocket *)sender())->disconnectFromServer();
                QCoreApplication::quit();
            } else {
                qWarning() << "Close all HbApplications before closing hbthemeserver!!";
            }
            break;
        }
        case EIconLookup: {
            QString filename;
            QSizeF size;
            int aspectRatioMode;
            int mode;
            bool mirrored;
            int options;
            QColor color;
            inputDataStream >> filename;
            inputDataStream >> size;
            inputDataStream >> aspectRatioMode;
            inputDataStream >> mode;
            inputDataStream >> mirrored;
            inputDataStream >> options;
            inputDataStream >> color;

#ifdef THEME_SERVER_TRACES
            qDebug() << "image req at server: " << filename;
#endif
            HbIconKey key(filename, size, (Qt::AspectRatioMode)aspectRatioMode, (QIcon::Mode)mode, mirrored, color);
            QByteArray output = handleIconLookup(key, data, options);
            ((QLocalSocket *)sender())->write(output);
            break;
        }
        case EIconDefaultSize:
            break; //todo
        case EMultiPieceIcon: {
            HbMultiIconParams frameItemParams;
            inputDataStream >> frameItemParams.multiPartIconList;
            inputDataStream >> frameItemParams.multiPartIconData.multiPartIconId;

            int noOfPieces = 1;
            if (frameItemParams.multiPartIconData.multiPartIconId.contains("_3PV", Qt::CaseInsensitive) ||
                    frameItemParams.multiPartIconData.multiPartIconId.contains("_3PH", Qt::CaseInsensitive)) {
                noOfPieces = 3;
            } else if (frameItemParams.multiPartIconData.multiPartIconId.contains("_9P", Qt::CaseInsensitive)) {
                noOfPieces = 9;
            }

            for (int i = 0; i < noOfPieces; i++) {
                inputDataStream >> frameItemParams.multiPartIconData.sources[i];
            }

            for (int i = 0; i < noOfPieces; i++) {
                inputDataStream >> frameItemParams.multiPartIconData.targets[i];
            }

            for (int i = 0; i < noOfPieces; i++) {
                inputDataStream >> frameItemParams.multiPartIconData.pixmapSizes[i];
            }

            inputDataStream >> frameItemParams.size;
            inputDataStream >> frameItemParams.aspectRatioMode;
            inputDataStream >> frameItemParams.mode;
            inputDataStream >> frameItemParams.mirrored;
            inputDataStream >> frameItemParams.options;
            inputDataStream >> frameItemParams.color;

#ifdef THEME_SERVER_TRACES
            qDebug() << "image req at server: " << frameItemParams.multiPartIconList;
#endif

            int index = frameItemParams.multiPartIconList[0].lastIndexOf("/");
            QString iconId = frameItemParams.multiPartIconList[0].left(index + 1);
            HbSharedIconInfo stitchedData;
            QT_TRY {
                iconId.append(frameItemParams.multiPartIconData.multiPartIconId);
                HbIconKey finalIconKey(iconId, frameItemParams.size,
                                       (Qt::AspectRatioMode)frameItemParams.aspectRatioMode,
                                       (QIcon::Mode)frameItemParams.mode, frameItemParams.mirrored,
                                       frameItemParams.color);

                stitchedData.type = INVALID_FORMAT;

                if (!iconInfoFromSingleIcon(finalIconKey, stitchedData)) {
                    iconInfoFromMultiParts(frameItemParams, noOfPieces, finalIconKey, stitchedData);
                }
            } QT_CATCH(const std::bad_alloc &) {
                stitchedData.type = INVALID_FORMAT;
            }

            QByteArray outputByteArray;
            QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
            HbThemeServerRequest request;
            request = EMultiPieceIcon;
            fillOutPutDataStream(outputDataStream, stitchedData, request);
#ifdef THEME_SERVER_TRACES
            qDebug() << Q_FUNC_INFO << " offset= " << stitchedData.pixmapData.offset << " format= " << stitchedData.pixmapData.format;
            testLabel->setPixmap(QPixmap::fromImage(
                                     QImage(
                                         HbMemoryUtils::getAddress<uchar>(HbMemoryManager::SharedMemory, stitchedData.pixmapData.offset),
                                         stitchedData.pixmapData.width,
                                         stitchedData.pixmapData.height,
                                         stitchedData.pixmapData.format)));
#endif
            ((QLocalSocket *)sender())->write(outputByteArray);
            break;
        }
        case EMultiIcon: {
            QStringList fileList;
            QVector<QSizeF> sizeList;
            QSizeF size;
            int aspectRatioMode;
            int mode;
            bool mirrored;
            int options;
            QColor color;
            inputDataStream >> fileList;
            for (int i = 0; i < fileList.count(); i++) {
                inputDataStream >> size;
                sizeList << size;
            }
            inputDataStream >> aspectRatioMode;
            inputDataStream >> mode;
            inputDataStream >> mirrored;
            inputDataStream >> options;
            inputDataStream >> color;

#ifdef THEME_SERVER_TRACES
            qDebug() << "image req at server: " << fileList;
#endif
            QByteArray output;
            for (int i = 0; i < fileList.count(); i++) {
                HbIconKey key(fileList[i], sizeList[i],
                              static_cast<Qt::AspectRatioMode>(aspectRatioMode),
                              static_cast<QIcon::Mode>(mode), mirrored, color);
                output.append(handleIconLookup(key, data, options));
            }

            ((QLocalSocket *)sender())->write(output);

            break;
        }
        case ENotifyForegroundLost: {
            //Nothing to do here when the app notifies it's foreground lost event
            break;
        }
        //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
        case ECacheIconCount: {
            int count = iServer->cacheIconCount();
            outputDataStream << count;
            ((QLocalSocket *)sender())->write(outputByteArray);
            break;
        }
        case ERasterMemLimit: {
            int limit;
            inputDataStream >> limit;
            iServer->setMaxGpuCacheSize(limit);
            break;
        }
        case EVectorMemLimit: {
            int limit;
            inputDataStream >> limit;
            iServer->setMaxCpuCacheSize(limit);
            break;
        }
        case EFreeRasterMem: {
            int freeRastMem = iServer->freeRasterMemory();
            outputDataStream << freeRastMem;
            ((QLocalSocket *)sender())->write(outputByteArray);
            break;
        }
        case EFreeVectorMem: {
            int freeVectMem = iServer->freeVectorMemory();
            outputDataStream << freeVectMem;
            ((QLocalSocket *)sender())->write(outputByteArray);
            break;
        }
        case ELastAddedItemMem {
                int lAddItemMem = iServer->lastAddedItemMem();
                outputDataStream << lAddItemMem;
                ((QLocalSocket *)sender())->write(outputByteArray);
                break;
            }
        case ELastRemovedItemMem {
                int lRemItemMem = iServer->lastRemovedItemMem();
                outputDataStream << lRemItemMem;
                ((QLocalSocket *)sender())->write(outputByteArray);
                break;
            }
        case ELastRemovedItemRefCount: {
                int lRemItemRfCnt = iServer->lastRemovedItemRfCount();
                outputDataStream << lRemItemRfCnt;
                ((QLocalSocket *)sender())->write(outputByteArray);
                break;
            }
        case ELastAddedItemRefCount: {
                int lAddItemRfCnt = iServer->lastAddedRefCount();
                outputDataStream << lAddItemRfCnt;
                ((QLocalSocket *)sender())->write(outputByteArray);
                break;
            }
        case EEnableCache: {
                int enable ;
                inputDataStream >> enable;
                bool success = iServer->enableCache(enable);
                outputDataStream << (int)success;
                ((QLocalSocket *)sender())->write(outputByteArray);
                break;
            }
        case ECacheHit: {
                int cacheHitCnt = iServer->cacheHitCount();
                outputDataStream << cacheHitCnt;
                ((QLocalSocket *)sender())->write(outputByteArray);
                break;
            }
        case ECacheMiss: {
                int cacheMissCnt = iServer->cacheMissCount();
                outputDataStream << cacheMissCnt;
                ((QLocalSocket *)sender())->write(outputByteArray);
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
                int rasterLruCount = iServer->rasterLruCount();
                outputDataStream << rasterLruCount;
                ((QLocalSocket *)sender())->write(outputByteArray);
                break;
            }
        case ECpuLruCount: {
                int vectorLruCount = iServer->vectorLruCount();
                outputDataStream << vectorLruCount;
                ((QLocalSocket *)sender())->write(outputByteArray);
                break;
            }
        case EServerHeap: {
            }
#endif
        case EUnloadIcon: {
                QString filename;
                QSizeF size;
                int aspectRatioMode;
                int mode;
                bool mirrored;
                int options;
                QColor color;
                inputDataStream >> filename;
                inputDataStream >> size;
                inputDataStream >> aspectRatioMode;
                inputDataStream >> mode;
                inputDataStream >> mirrored;
                inputDataStream >> options;
                inputDataStream >> color;

                HbIconKey key(filename, size, (Qt::AspectRatioMode)aspectRatioMode,
                              (QIcon::Mode)mode, mirrored, color);
                iServer->removeIconCacheItem(key);
                sessionIconData.removeOne(key);
                QByteArray outputByteArray;
                QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
                outputDataStream << (int)requestType;
                ((QLocalSocket *)sender())->write(outputByteArray);
                break;
            }
        case EFreeSharedMem: {
            int freeSharedMem = iServer->freeSharedMemory();
            QByteArray outputByteArray;
            QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
            outputDataStream << requestType;
            outputDataStream << freeSharedMem;
            ((QLocalSocket *)sender())->write(outputByteArray);
            break;
        }
        case EAllocatedSharedMem: {
            int allocatedSharedMem = iServer->allocatedSharedMemory();
            QByteArray outputByteArray;
            QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
            outputDataStream << requestType;
            outputDataStream << allocatedSharedMem;
            ((QLocalSocket *)sender())->write(outputByteArray);
            break;
        }

        default:
            break;
        }
    }
}

/*!
  \fn HbThemeServerSession::iconInfoFromSingleIcon()
  Checks for the cacheItem for a given key, if found gets the data relevant of the cacheItem.
*/

bool HbThemeServerSession::iconInfoFromSingleIcon(HbIconKey key,
        HbSharedIconInfo &stitchedData)
{
    stitchedData.type = INVALID_FORMAT;
    HbIconCacheItem * cacheItem = iServer->iconCacheItem(key);
    if (cacheItem) {
        getDataFromCacheItem(cacheItem, stitchedData);
        return true;
    }
    return false;
}

/*!
  \fn HbThemeServerSession::createCacheItemData()
  Creates a cacheItem of the given key and insert the item in to the list
  else free the data allocated for the cache..
*/
bool HbThemeServerSession::createCacheItemData(HbIconKey key, int options , HbSharedIconInfo &data)
{
    HbIconCacheItem * cacheItemOfPiece = iServer->iconCacheItem(key);
    if (cacheItemOfPiece) {
        return true;
    }

    QScopedPointer <HbIconCacheItem> tempIconCacheItem;
    bool insertKeyIntoSessionList = false;
    data.type = INVALID_FORMAT;
    QString format = HbThemeServerUtils::formatFromPath(key.filename);

    tempIconCacheItem.reset(HbIconCacheItemCreator::createCacheItem(key,
                            (HbIconLoader::IconLoaderOptions)options,
                            format,
                            false));
    cacheItemOfPiece = tempIconCacheItem.data();
    if (cacheItemOfPiece) {
        getDataFromCacheItem(cacheItemOfPiece, data);
        if (data.type != INVALID_FORMAT) {
            insertKeyIntoSessionList = iServer->insertIconCacheItem(key, cacheItemOfPiece);
            if (!insertKeyIntoSessionList) {
                //if insertion failed free the memory
                freeDataFromCacheItem(cacheItemOfPiece);
                data.type = INVALID_FORMAT;
            }
        }
    }
    tempIconCacheItem.take();
    return insertKeyIntoSessionList;
}

/*!
  \fn HbThemeServerSession::createStichedIconInfoOfParts()
  Creates a consolidated icon of the available piece iconInfo.
*/
bool HbThemeServerSession::createStichedIconInfoOfParts(QVector<HbSharedIconInfo> dataForParts, HbMultiIconParams params,
        HbIconKey &finalIconKey, HbSharedIconInfo &stitchedData)
{
    HbIconCacheItem * cacheItem = iServer->iconCacheItem(finalIconKey);
    if (cacheItem) {
        return true;
    }
    bool insertKeyIntoSessionList = false;
    stitchedData.type = INVALID_FORMAT;
    QString format = HbThemeServerUtils::formatFromPath(params.multiPartIconList[0]);

    QScopedPointer <HbPixmapIconProcessor> tempIconProcessor(new HbPixmapIconProcessor(finalIconKey,
                                                static_cast<HbIconLoader::IconLoaderOptions>(params.options), format));
    HbPixmapIconProcessor * rasterIcon = tempIconProcessor.data();
    rasterIcon->createMultiPieceIconData(dataForParts, params);

    QScopedPointer <HbIconCacheItem> tempIconCacheItem;
    tempIconCacheItem.reset(HbIconCacheItemCreator::createCacheItem(finalIconKey,
                                    static_cast<HbIconLoader::IconLoaderOptions>(params.options), format, false));
    cacheItem = tempIconCacheItem.data();

    cacheItem->rasterIconData = rasterIcon->sharedIconData();
    cacheItem->rasterIconDataCost = rasterIcon->sharedIconDataCost();
    stitchedData = cacheItem->rasterIconData;
    if (stitchedData.type != INVALID_FORMAT) {
        insertKeyIntoSessionList = iServer->insertIconCacheItem(finalIconKey, cacheItem);
        if (!insertKeyIntoSessionList) {
            //if insertion failed free the memory
            freeDataFromCacheItem(cacheItem);
            stitchedData.type = INVALID_FORMAT;
        }
    }
    tempIconCacheItem.take();
    return insertKeyIntoSessionList;
}


/*!
  \fn HbThemeServerSession::iconInfoFromMultiParts()
  Creates a shared IconInfo of the piece files of a frame item and
  tries to create a stiched icon of the same.
*/
void HbThemeServerSession::iconInfoFromMultiParts(HbMultiIconParams params,
        int noOfPieces,
        HbIconKey &stichedKey,
        HbSharedIconInfo &stitchedData)
{
    QVector<HbIconKey> keysInserted;
    QVector<HbSharedIconInfo>  dataForParts;
    bool insertKeyIntoSessionList = false;
    bool failedToCreateParts = false;
    QString format;
    try {
        for (int i = 0; i < noOfPieces; i++) {
            HbSharedIconInfo data;
            bool iconPieceMirrored = false;
            HbIconKey key(params.multiPartIconList.at(i), params.multiPartIconData.pixmapSizes[i],
                          static_cast<Qt::AspectRatioMode>(stichedKey.aspectRatioMode),
                          static_cast<QIcon::Mode>(stichedKey.mode), iconPieceMirrored, stichedKey.color);
            insertKeyIntoSessionList = iconInfoFromSingleIcon(key, data);
            if (!insertKeyIntoSessionList) {
                insertKeyIntoSessionList = createCacheItemData(key, params.options, data);
            }
            if ((data.type == INVALID_FORMAT) || (!insertKeyIntoSessionList)) {
                failedToCreateParts = true;
                break;
            } else {
                //The session will only keep track of icons that were either successfully found or were
                //successfully inserted in the cache.
                keysInserted.append(key);
                dataForParts.append(data);
                sessionIconData.append(key);
            }
        }//end of for
    } catch(std::exception &) {
        failedToCreateParts = true;
    }

    if ((failedToCreateParts) || (dataForParts.count() != noOfPieces) || (!insertKeyIntoSessionList)) {
        //atLeast one of the icon did'nt get constructed , so move the cached piece icons to unused state and return
        for (int i = 0; i < keysInserted.count(); i++) {
            sessionIconData.removeOne(keysInserted.at(i));
        }
        dataForParts.clear();
        stitchedData.type = INVALID_FORMAT;
        return;
    }
// Create a stitched icon of the available piece shared iconinfos
    if ((dataForParts.count() == noOfPieces) && (!failedToCreateParts)) {
        try {
            if (createStichedIconInfoOfParts(dataForParts, params, stichedKey, stitchedData)) {
                sessionIconData.append(stichedKey);
            }
        } catch(std::exception &) {
        }
    }
// Move the keys created for pieces to unused state*/
    for (int i = 0; i < keysInserted.count(); i++) {
        sessionIconData.removeOne(keysInserted.at(i));
    }
}

/*!
  \fn HbThemeServerSession::clearSessionIconData()
  Clears the session data of the icons found in the cache.
*/
void HbThemeServerSession::clearSessionIconData()
{
    sessionIconData.clear();
}

void HbThemeServerSession::removeSessionIconItem(const HbIconKey &key)
{
    sessionIconData.removeAll(key);
}

void HbThemeServerSession::removeSessionCssItem(const QString &key)
{
    sessionCssData.removeAll(key);
}

/*!
  \fn HbThemeServerSession::freeDataFromCacheItem()
  Frees data from the cached item when insertion to the list fails.
*/
void HbThemeServerSession::freeDataFromCacheItem(HbIconCacheItem* cacheItem)
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

/*!
  \fn HbThemeServerSession::fillOutPutDataStream()
  Fills the Output data stream with the sharedIconInfo data.
*/
void HbThemeServerSession::fillOutPutDataStream(QDataStream &outputDataStream, HbSharedIconInfo &data, HbThemeServerRequest request)
{
    outputDataStream << (int)request;
    outputDataStream << (int)data.type;

    switch (data.type) {
    case OTHER_SUPPORTED_FORMATS:
        outputDataStream << data.pixmapData.offset;
        outputDataStream << data.pixmapData.width;
        outputDataStream << data.pixmapData.height;
        outputDataStream << data.pixmapData.defaultWidth;
        outputDataStream << data.pixmapData.defaultHeight;
        outputDataStream << (int)data.pixmapData.format;
        break;
        /*case SVG:*/
    case PIC: {
        outputDataStream << data.picData.offset;
        outputDataStream << data.picData.dataSize;
        outputDataStream << data.picData.defaultWidth;
        outputDataStream << data.picData.defaultHeight;
        break;
    }
    case NVG: {
        outputDataStream << data.nvgData.offset;
        outputDataStream << data.nvgData.dataSize;
        break;
    }
    case BLOB: {
        outputDataStream << data.blobData.offset;
        outputDataStream << data.blobData.dataSize;
        break;
    }
    default:
        break;
    }
}

/*!
  \fn HbThemeServerSession::getDataFromCacheItem()
  Gets data from the cache Item.
*/
void HbThemeServerSession::getDataFromCacheItem(HbIconCacheItem* cacheItem, HbSharedIconInfo &data) const
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
 * HbThemeServerPrivate::handleSharedWidgetMLLookup()
 */
QByteArray HbThemeServerPrivate::handleSharedWidgetMLLookup(const QString &fileName, const QString &layout, const QString &section)
{
    int offset = HbThemeServerUtils::getSharedLayoutDefinition(fileName, layout, section);
    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest request;
    request = EWidgetMLLookup;
    outputDataStream << (int)request;
    outputDataStream << offset;
    return outputByteArray;
}

/**
 * HbThemeServerPrivate::handleSharedEffectAddAndFileLookup()
 */
QByteArray HbThemeServerPrivate::handleSharedEffectAddAndFileLookup(int request, const QString &fileName)
{
    int offset = HbThemeServerUtils::getSharedEffect(fileName);
    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    outputDataStream << request;
    outputDataStream << offset;
    return outputByteArray;
}

/**
 * HbThemeServerSession::handleStyleSheetLookup()
 */
QByteArray HbThemeServerSession::handleStyleSheetLookup(int request, const QString &fileName, HbLayeredStyleLoader::LayerPriority priority)
{
    int offset = -1;
    HbCacheItem* cssItem = iServer->cssCacheItem(fileName);
    bool insertKeyIntoSessionList = false;
    if (cssItem) {
        offset = cssItem->offset;
        insertKeyIntoSessionList = true;
    } else {
        bool tryAgain = false;
        do {
            offset = HbThemeServerUtils::getSharedStylesheet(fileName, priority);
            if (offset >= 0) {
                HbCacheItem *cssItem = new HbCacheItem(offset, 0, fileName);
                insertKeyIntoSessionList = iServer->insertCssCacheItem(fileName, cssItem);
                if (priority == HbLayeredStyleLoader::Priority_Core && cssItem->refCount == 1) {
                    // This will make sure the requested stylesheet will always remain
                    // in the primary and secondary cache.
                    cssItem->incrementRefCount();
                }
                if (priority == HbLayeredStyleLoader::Priority_Theme && cssItem->refCount == 1) {
                    iServer->themePriorityItems.insert(fileName,cssItem);
                }
                break;
            } else if (offset == OUT_OF_MEMORY_ERROR && tryAgain == false) {
                iServer->doCleanup();
                tryAgain = true;
            } else if (offset == OUT_OF_MEMORY_ERROR && tryAgain == true) {
                //try only once to free up memory, else offset remains -2
                tryAgain = false;
            }
        } while (tryAgain);
    }

    if (insertKeyIntoSessionList) {
        //The session will only keep track of cssFiles that were either successfully found or were
        //successfully inserted in the cache.
        sessionCssData.append(fileName);
    }

    //write to client socket the offset of required stylesheet
    //it can be -1 in case of error.
    QByteArray output;
    QDataStream outputDataStream(&output, QIODevice::WriteOnly);

    outputDataStream << request;
    outputDataStream << offset;

    return output;
}

/**
 * HbThemeServerSession::handleIconLookup()
 */
QByteArray HbThemeServerSession::handleIconLookup(const HbIconKey &key, HbSharedIconInfo &data, int options)
{
    bool insertKeyIntoSessionList = false;
    HbIconCacheItem * cacheItem = iServer->iconCacheItem(key);
    QScopedPointer <HbIconCacheItem> tempIconCacheItem;

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
        QString format = HbThemeServerUtils::formatFromPath(key.filename);
        QT_TRY {
            tempIconCacheItem.reset(HbIconCacheItemCreator::createCacheItem(key,
                (HbIconLoader::IconLoaderOptions)options,
                format,
                false));
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
                }
            }
        } QT_CATCH(const std::bad_alloc &) {
            data.type = INVALID_FORMAT;
        }
    }
    if (insertKeyIntoSessionList) {
        //The session will only keep track of icons that were either successfully found or were
        //successfully inserted in the cache.
        sessionIconData.append(key);
    }

    tempIconCacheItem.take();

    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest request;
    request = EIconLookup;
    outputDataStream << (int)request;
    outputDataStream << (int)data.type;

    if (data.type == OTHER_SUPPORTED_FORMATS) {
        outputDataStream << data.pixmapData.offset;
        outputDataStream << data.pixmapData.width;
        outputDataStream << data.pixmapData.height;
        outputDataStream << data.pixmapData.defaultWidth;
        outputDataStream << data.pixmapData.defaultHeight;
        outputDataStream << (int)data.pixmapData.format;
    } else if (/*data.type == SVG || */data.type == PIC) {
        outputDataStream << data.picData.offset;
        outputDataStream << data.picData.dataSize;
        outputDataStream << data.picData.defaultWidth;
        outputDataStream << data.picData.defaultHeight;
    } else if (data.type == NVG) {
        outputDataStream << data.nvgData.offset;
        outputDataStream << data.nvgData.dataSize;
    } else if (data.type == BLOB) {
        outputDataStream << data.blobData.offset;
        outputDataStream << data.blobData.dataSize;
    }
#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO << " offset= " << data.pixmapData.offset << " format= " << data.pixmapData.format;
    testLabel->setPixmap(QPixmap::fromImage(
                             QImage(
                                 HbMemoryUtils::getAddress<uchar>(HbMemoryManager::SharedMemory, data.pixmapData.offset),
                                 data.pixmapData.width,
                                 data.pixmapData.height,
                                 data.pixmapData.format)));
#endif

    return outputByteArray;
}

/**
 * HbThemeServerPrivate::doCleanup()
 *
 * This function releases shared memory occupied by css-resources whose reference count is zero,
 * so that subsequent css-requests could be fulfilled by the server. Those css-files whose reference
 * count are zero, are already appended to the list maintained by the css-cache. Since these resources
 * are not being referred to by any application, they can be removed from the cache and corresponding
 * shared memory can be freed up.
 */
void HbThemeServerPrivate::doCleanup()
{
    HbThemeServerUtils::cleanupUnusedCss(cssCache);
}

/**
 * HbThemeServerPrivate::sessionListCount()
 * Gives the list count of Sessions.
 */
int HbThemeServerPrivate::sessionListCount() const
{
    return sessionList.count();
}

/**
 * HbThemeServerPrivate::freeSharedMemory()
 * Gives the free shared memory.
 */
int HbThemeServerPrivate::freeSharedMemory() const
{
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    return static_cast<HbSharedMemoryManager *>(manager)->freeSharedMemory();
}

/**
 * HbThemeServerPrivate::allocatedSharedMemory()
 * Gives the allocated shared memory.
 */
int HbThemeServerPrivate::allocatedSharedMemory() const
{
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    return static_cast<HbSharedMemoryManager *>(manager)->allocatedSharedMemory();
}
