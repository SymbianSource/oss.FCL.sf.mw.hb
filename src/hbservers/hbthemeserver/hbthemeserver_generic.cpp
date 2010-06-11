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
#include "hbthemeserverapplication_p.h"

#include <QLabel>
#include <QLocalServer>
#include <QLocalSocket>
#include <QSharedMemory>
#include <QDebug>
#include <QList>
#include <QIcon>
#include <QFile>
#include <QCoreApplication>
#include <QPainter>
#include <QStringList>
#include <QPixmap>
#include <QTimer>

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
#include "hbtypefaceinfodatabase_p.h"
#include "hbthemeutils_p.h"

static const int CLOSE_TIMEOUT = 3000;

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
HbThemeServerPrivate::HbThemeServerPrivate(QWidget *parent)
    : QMainWindow(parent), server(new QLocalServer(this))
#else
HbThemeServerPrivate::HbThemeServerPrivate(): server(new QLocalServer(this))
#endif
{
    sessionList.clear();
#ifdef QT_DEBUG
    setWindowTitle("Theme Server");
    setCentralWidget(&statusLabel);
#endif
    renderMode = ESWRendering;
    QScopedPointer <HbIconDataCache> tempIconCache(new HbIconDataCache());
    QScopedPointer <HbCache> tempCssCache(new HbCache());
    iconCache = tempIconCache.take();
    cssCache = tempCssCache.take();
    setMaxGpuCacheSize(GPU_CACHE_SIZE);
    setMaxCpuCacheSize(CPU_CACHE_SIZE);

    // Store the active theme name in a member string
    iCurrentThemeName = HbThemeUtils::getThemeSetting(HbThemeUtils::CurrentThemeSetting);    

    if (iCurrentThemeName.isEmpty()) {
        iCurrentThemeName = HbThemeUtils::defaultTheme().name;
    }
    
    // Resolve the path of the current theme
    QDir path(iCurrentThemeName);
    if (!path.isAbsolute()) {
        // Resolve the path of the current theme
        resolveThemePath(iCurrentThemeName, iCurrentThemePath);
    } else {
        iCurrentThemeName = path.dirName();
        iCurrentThemePath = path.absolutePath();
    }          
    // Process base theme index, it is used as parent index also when the current theme is something else
    QString basePath;
    resolveThemePath(HbThemeUtils::getThemeSetting(HbThemeUtils::BaseThemeSetting), basePath);
    HbThemeServerUtils::createThemeIndex(basePath, BaseTheme);
    // Process operator theme indexes
    QString operatorName = HbThemeUtils::getThemeSetting(HbThemeUtils::OperatorNameSetting);
    if (!operatorName.isEmpty()) {
        QString operatorPath( HbThemeUtils::baseTheme().rootDir + '/' +
            QLatin1String(HbThemeUtils::operatorHierarchy) + '/' +
            QLatin1String(HbThemeUtils::iconsResourceFolder) + '/' +
            operatorName );
        
        HbThemeServerUtils::createThemeIndex(operatorPath, OperatorC);
        // Operator Z not used in generic themeserver.
    }
    // Process current theme index
    HbThemeServerUtils::createThemeIndex(iCurrentThemePath, ActiveTheme);

    // Register theme system effects in construction
    // TODO: fix parameter
    HbThemeSystemEffect::handleThemeChange(iCurrentThemeName);

    connect(server, SIGNAL(newConnection()), this, SLOT(newClientConnected()));
    HbThemeUtils::setThemeSetting(HbThemeUtils::CurrentThemeSetting, iCurrentThemePath);    
}

/*!
  \fn HbThemeServerPrivate::~HbThemeServerPrivate()
  Destructor
*/
HbThemeServerPrivate::~HbThemeServerPrivate()
{
    server->close();
    // QLocalServer should be deleted first before deleting Server data
    // so all sessions will be cleaned up first.
    delete server;
    delete iconCache;
    delete cssCache;
}

/*!
  \fn HbThemeServerPrivate::start()
  start the themeserver
*/
bool HbThemeServerPrivate::start()
{
    // try max 2 times
    bool success = listen() || listen();
    if (!success) {
        qWarning() << "HbThemeServer: unable to start the server.";
    } else {
#ifdef QT_DEBUG
        statusLabel.setText("Theme Server Started");
#endif
    }
    return success;
}

bool HbThemeServerPrivate::listen()
{
    bool success = server->listen(THEME_SERVER_NAME);
    if (!success) {
        qWarning() << "HbThemeServer:" << server->errorString();
        if (server->serverError() == QAbstractSocket::AddressInUseError) {
            stop();
        }
    }
    return success;
}

/*!
  \fn HbThemeServerPrivate::stop()
  stop the themeserver
*/
void HbThemeServerPrivate::stop()
{
    if (server) {
        server->close();
    }
    QLocalServer::removeServer(THEME_SERVER_NAME);
#ifdef QT_DEBUG
    statusLabel.setText("Theme Server Stopped");
#endif
}

/*!
  \fn HbThemeServerPrivate::insertIconCacheItem()
  Insert item into the icon cache
  \a key denotes the unique identifier for the cache item that is to be inserted into the cache.
  \a item denotes the cache item that is to be inserted
*/
bool HbThemeServerPrivate::insertIconCacheItem(const HbIconKey &key, HbIconCacheItem *item)
{
    return (iconCache->insert(key, item));
}

/*!
  \fn HbThemeServerPrivate::insertCssCacheItem()
  Insert item into the css cache
  \a key denotes the unique identifier for the cache item that is to be inserted into the cache.
  \a item denotes the cache item that is to be inserted
*/
bool HbThemeServerPrivate::insertCssCacheItem(const QString &key, HbCacheItem *item)
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
    return(iconCache->getCacheItem(key, renderMode, false));
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

    // Clear cached icons and session data
    clearIconCache();
    HbThemeServerSession *session;
    foreach(session, sessionList) {
        session->clearSessionIconData();
    }

    iCurrentThemeName = newTheme.trimmed();

    // Resolve the path of the current theme
    QDir path(iCurrentThemeName);
    if (!path.isAbsolute()) {
        // Resolve the path of the current theme
        resolveThemePath(iCurrentThemeName, iCurrentThemePath);
    } else {
        iCurrentThemeName = path.dirName();
        iCurrentThemePath = path.absolutePath();
    }          

    // Set the new theme in theme settings
    HbThemeUtils::setThemeSetting(HbThemeUtils::CurrentThemeSetting, iCurrentThemeName);    

    // Process operator Drive C theme index
    QString operatorName = HbThemeUtils::getThemeSetting(HbThemeUtils::OperatorNameSetting);
    if (!operatorName.isEmpty()) {
        QString operatorPath( HbThemeUtils::baseTheme().rootDir + '/' +
            QLatin1String(HbThemeUtils::operatorHierarchy) + '/' +
            QLatin1String(HbThemeUtils::iconsResourceFolder) + '/' +
            operatorName );
        
        HbThemeServerUtils::createThemeIndex(operatorPath, OperatorC);
    }
    // Process current theme index
    HbThemeServerUtils::createThemeIndex(iCurrentThemePath, ActiveTheme);

    // Register new system effects
    HbThemeSystemEffect::handleThemeChange(iCurrentThemeName);

    HbThemeServerRequest requestType;
    requestType = EThemeSelection;
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (int)requestType;
    out << iCurrentThemeName;
    writeToClients(block);
}

bool HbThemeServerPrivate::resolveThemePath(const QString &themeName, QString &themePath)
{
    static QString mainThemesDir = QDir::fromNativeSeparators(qgetenv("HB_THEMES_DIR"));

    if (themeName == "hbdefault") {
        themePath = ":/themes/icons/hbdefault";
        return true;
    } else {
        QString themeLookupPath = mainThemesDir + "/themes/icons/" + themeName;
        if (QFile::exists(themeLookupPath + "/index.theme")) {
            themePath = themeLookupPath;
            return true;
        }
    }

    return false;
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
    for (int i = 0; i < fileNames.count(); i++) {
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
        for (int j = 0; j < keys.count(); j++) {
            HbThemeServerSession *session;
            foreach(session, sessionList) {
                session->removeSessionIconItem(*keys.at(j));
            }
            iconCache->remove(*keys.at(j), false);
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
        curSocket->write(block);
    }
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

    QTimer::singleShot(CLOSE_TIMEOUT, this, SLOT(clienDisconnected()));
}

void HbThemeServerPrivate::clienDisconnected()
{
#ifdef THEME_SERVER_TRACES
    qDebug() << "Total No of Connection after deletion = " << sessionList.count();
#endif

    // Quit the server if no more clients connected
    if (!HbThemeServerApplication::Options::persistent && sessionList.isEmpty()) {
#ifdef THEME_SERVER_TRACES
        qDebug() << "HbThemeServerQt: last application quit, so do we!";
#endif
        stop();
        QCoreApplication::quit();
    }
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
  A  key is added to the list only if the corresponding cache item
  has been successfully inserted into cache or if a previously cached icon
  is requested and successfully found in the cache.
  In case of client crash or graceful exit of client, the server goes through its list of sessions,
  finds the session corresponding to the client, removes the session from its session list and
  deletes the session.
  In the session destructor, the session specific resources are cleaned up.
  In case of icon caching, the list of icon keys is traversed and a remove operation is
  performed on the cache items corresponding to these keys.
  On removal, the items reference count is decremented and in case the
  reference count becomes 0, the cache item instance gets added to the back of the LRU list.

*/

/*!
  \fn HbThemeServerSession::HbThemeServerSession()
  Constructor
  \a aClientConnection indicates the local socket that is contained within this session
  \a aServer denotes the handle to the theme server
*/
HbThemeServerSession::HbThemeServerSession(QLocalSocket *aClientConnection,
                                           HbThemeServerPrivate *aServer)
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
    QByteArray inputByteArray = static_cast<QLocalSocket *>(sender())->readAll();
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
        outputDataStream << int(requestType);
#endif
        switch (requestType) {
        case EStyleSheetLookup: {
            QString fileName;
            int priority;
            HbLayeredStyleLoader::LayerPriority layerPriority;
            inputDataStream >> fileName;
            inputDataStream >> priority;
            layerPriority = static_cast<HbLayeredStyleLoader::LayerPriority>(priority);
            QByteArray output = handleStyleSheetLookup(int(requestType), fileName, layerPriority);
            static_cast<QLocalSocket *>(sender())->write(output);
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
            static_cast<QLocalSocket *>(sender())->write(output);
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
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
            break;
        }
        case EEffectLookupFilePath:
        case EEffectAdd: {
            QString fileName;
            inputDataStream >> fileName;
            QByteArray output = iServer->handleSharedEffectAddAndFileLookup((int) requestType, fileName);
            static_cast<QLocalSocket *>(sender())->write(output);
            break;
        }
        case EThemeSelection: {
            QString themename;
            inputDataStream >> themename;
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
            out << int(requestType);
            out << themedItems;
            iServer->writeToClients(block);
            break;
        }
        case EThemeServerStop: {
            //This segment is used by HbThemeApiWrapperUi.
            //When no HbApplication is open and
            // HbThemeserver has only HbThemeApiWrapperUi as client, making sessionListCount =1
            //HbThemeserver is closed otherwise warning is shown.
            //QCoreApplication::quit() removes the UI of HbThemeServer.
            // Destructor of HbThemeServer will be called where it was instantiated.
            if (iServer->sessionListCount() == 1) {
#ifdef QT_DEBUG
                iServer->close();
#endif
                static_cast<QLocalSocket *>(sender())->disconnectFromServer();
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
            HbIconKey key(filename, size, static_cast<Qt::AspectRatioMode>(aspectRatioMode),
                          static_cast<QIcon::Mode>(mode), mirrored, color,
                          iServer->currentRenderingMode());
            QByteArray output = handleIconLookup(key, data, options);
            static_cast<QLocalSocket *>(sender())->write(output);
            break;
        }
        case EMultiPieceIcon: {
            HbMultiIconParams frameItemParams;
            inputDataStream >> frameItemParams.multiPartIconList;
            inputDataStream >> frameItemParams.multiPartIconData.multiPartIconId;

            int noOfPieces = 1;
            if (frameItemParams.multiPartIconData.multiPartIconId.contains("_3PV", Qt::CaseInsensitive)
                || frameItemParams.multiPartIconData.multiPartIconId.contains("_3PH", Qt::CaseInsensitive)) {
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
            int index = frameItemParams.multiPartIconList[0].lastIndexOf('/');
            QString iconId = frameItemParams.multiPartIconList[0].left(index + 1);
            HbSharedIconInfo stitchedData;
            QT_TRY {
                iconId.append(frameItemParams.multiPartIconData.multiPartIconId);
                HbIconKey finalIconKey(iconId, frameItemParams.size,
                    static_cast<Qt::AspectRatioMode>(frameItemParams.aspectRatioMode),
                    static_cast<QIcon::Mode>(frameItemParams.mode),
                    frameItemParams.mirrored,
                    frameItemParams.color,
                    static_cast<HbRenderingMode>(frameItemParams.renderMode));

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
            qDebug() << Q_FUNC_INFO << " offset= " << stitchedData.pixmapData.offset
                     << " format= " << stitchedData.pixmapData.format;
            testLabel->setPixmap(QPixmap::fromImage(
                QImage(HbMemoryUtils::getAddress<uchar>(HbMemoryManager::SharedMemory,
                                                        stitchedData.pixmapData.offset),
                       stitchedData.pixmapData.width,
                       stitchedData.pixmapData.height,
                       stitchedData.pixmapData.format)));
#endif
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
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
            inputDataStream >> sizeList;
            inputDataStream >> aspectRatioMode;
            inputDataStream >> mode;
            inputDataStream >> mirrored;
            inputDataStream >> options;
            inputDataStream >> color;

#ifdef THEME_SERVER_TRACES
            qDebug() << "image req at server: " << fileList;
#endif
            QByteArray output;
            QDataStream outputDataStream(&output, QIODevice::WriteOnly);
            outputDataStream << (int)requestType; // Put EMultiIcon request type in the beginning
            for (int i = 0; i < fileList.count(); i++) {
                HbIconKey key(fileList[i], sizeList[i],
                              static_cast<Qt::AspectRatioMode>(aspectRatioMode),
                              static_cast<QIcon::Mode>(mode), mirrored, color,
                              iServer->currentRenderingMode());
                output.append(handleIconLookup(key, data, options));
            }

            static_cast<QLocalSocket *>(sender())->write(output);
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
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
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
            static_cast<QLocalSocket *>sender())->write(outputByteArray);
            break;
        }
        case EFreeVectorMem: {
            int freeVectMem = iServer->freeVectorMemory();
            outputDataStream << freeVectMem;
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
            break;
        }
        case ELastAddedItemMem {
            int lAddItemMem = iServer->lastAddedItemMem();
            outputDataStream << lAddItemMem;
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
            break;
        }
        case ELastRemovedItemMem {
            int lRemItemMem = iServer->lastRemovedItemMem();
            outputDataStream << lRemItemMem;
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
            break;
        }
        case ELastRemovedItemRefCount: {
            int lRemItemRfCnt = iServer->lastRemovedItemRfCount();
            outputDataStream << lRemItemRfCnt;
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
            break;
        }
        case ELastAddedItemRefCount: {
            int lAddItemRfCnt = iServer->lastAddedRefCount();
            outputDataStream << lAddItemRfCnt;
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
            break;
        }
        case EEnableCache: {
            int enable ;
            inputDataStream >> enable;
            bool success = iServer->enableCache(enable);
            outputDataStream << (int)success;
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
            break;
        }
        case ECacheHit: {
            int cacheHitCnt = iServer->cacheHitCount();
            outputDataStream << cacheHitCnt;
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
            break;
        }
        case ECacheMiss: {
            int cacheMissCnt = iServer->cacheMissCount();
            outputDataStream << cacheMissCnt;
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
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
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
            break;
        }
        case ECpuLruCount: {
            int vectorLruCount = iServer->vectorLruCount();
            outputDataStream << vectorLruCount;
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
            break;
        }
        case EServerHeap: {
        }
#endif
#ifdef HB_THEME_SERVER_MEMORY_REPORT
        case ECreateMemoryReport: {
            GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
            static_cast<HbSharedMemoryManager *>(manager)->createReport();
            break;
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

            HbIconKey key(filename, size,
                          static_cast<Qt::AspectRatioMode>(aspectRatioMode),
                          static_cast<QIcon::Mode>(mode), mirrored, color,
                          iServer->currentRenderingMode());
            iServer->removeIconCacheItem(key);
            sessionIconData.removeOne(key);
            QByteArray outputByteArray;
            QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
            outputDataStream << int(requestType);
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
            break;
        }
        case EFreeSharedMem: {
            int freeSharedMem = iServer->freeSharedMemory();
            QByteArray outputByteArray;
            QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
            outputDataStream << requestType;
            outputDataStream << freeSharedMem;
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
            break;
        }
        case EAllocatedSharedMem: {
            int allocatedSharedMem = iServer->allocatedSharedMemory();
            QByteArray outputByteArray;
            QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
            outputDataStream << requestType;
            outputDataStream << allocatedSharedMem;
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
            break;
        }
        case ETypefaceOffset: {
            int offset = -1;
            HbTypefaceInfoDatabase *typefaceDatabase =
                HbTypefaceInfoDatabase::instance(HbMemoryManager::SharedMemory);

            if (typefaceDatabase) {
                offset = typefaceDatabase->typefaceInfoVectorOffset();
            }
            // offset will be -1 if the typefaceDatabase is NULL.
            QByteArray outputByteArray;
            QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
            outputDataStream << requestType;
            outputDataStream << offset;
            static_cast<QLocalSocket *>(sender())->write(outputByteArray);
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
    HbIconCacheItem *cacheItem = iServer->iconCacheItem(key);
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
bool HbThemeServerSession::createCacheItemData(HbIconKey key, int options, HbSharedIconInfo &data)
{
    HbIconCacheItem *cacheItemOfPiece = iServer->iconCacheItem(key);
    if (cacheItemOfPiece) {
        return true;
    }

    QScopedPointer<HbIconCacheItem> tempIconCacheItem;
    bool insertKeyIntoSessionList = false;
    data.type = INVALID_FORMAT;
    QString format = HbThemeServerUtils::formatFromPath(key.filename);

    tempIconCacheItem.reset(HbIconCacheItemCreator::createCacheItem(key,
                            static_cast<HbIconLoader::IconLoaderOptions>(options),
                            format, iServer->currentRenderingMode(), false));
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
    HbIconCacheItem *cacheItem = iServer->iconCacheItem(finalIconKey);
    if (cacheItem) {
        return true;
    }
    bool insertKeyIntoSessionList = false;
    stitchedData.type = INVALID_FORMAT;
    QString format = HbThemeServerUtils::formatFromPath(params.multiPartIconList[0]);

    QScopedPointer<HbPixmapIconProcessor> tempIconProcessor(new HbPixmapIconProcessor(finalIconKey,
                                                static_cast<HbIconLoader::IconLoaderOptions>(params.options), format));
    HbPixmapIconProcessor *rasterIcon = tempIconProcessor.data();
    rasterIcon->createMultiPieceIconData(dataForParts, params);

    QScopedPointer <HbIconCacheItem> tempIconCacheItem;
    tempIconCacheItem.reset(HbIconCacheItemCreator::createCacheItem(finalIconKey,
                                    static_cast<HbIconLoader::IconLoaderOptions>(params.options), format, iServer->currentRenderingMode(),false));
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
    try {
        for (int i = 0; i < noOfPieces; i++) {
            HbSharedIconInfo data;
            bool iconPieceMirrored = false;
            HbIconKey key(params.multiPartIconList.at(i), params.multiPartIconData.pixmapSizes[i],
                          static_cast<Qt::AspectRatioMode>(stichedKey.aspectRatioMode),
                          static_cast<QIcon::Mode>(stichedKey.mode),
                          iconPieceMirrored, stichedKey.color, stichedKey.renderMode);
            insertKeyIntoSessionList = iconInfoFromSingleIcon(key, data);
            if (!insertKeyIntoSessionList) {
                insertKeyIntoSessionList = createCacheItemData(key, params.options, data);
            }
            if ((data.type == INVALID_FORMAT) || !insertKeyIntoSessionList) {
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

    if (failedToCreateParts || (dataForParts.count() != noOfPieces)
        || !insertKeyIntoSessionList) {
        //atLeast one of the icon did'nt get constructed ,
        //so move the cached piece icons to unused state and return
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
void HbThemeServerSession::fillOutPutDataStream(QDataStream &outputDataStream,
                                                HbSharedIconInfo &data,
                                                HbThemeServerRequest request)
{
    outputDataStream << int(request);
    outputDataStream << int(data.type);

    switch (data.type) {
    case OTHER_SUPPORTED_FORMATS:
        outputDataStream << data.pixmapData.offset;
        outputDataStream << data.pixmapData.width;
        outputDataStream << data.pixmapData.height;
        outputDataStream << data.pixmapData.defaultWidth;
        outputDataStream << data.pixmapData.defaultHeight;
        outputDataStream << int(data.pixmapData.format);
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
void HbThemeServerSession::getDataFromCacheItem(HbIconCacheItem* cacheItem,
                                                HbSharedIconInfo &data) const
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
QByteArray HbThemeServerPrivate::handleSharedWidgetMLLookup(const QString &fileName,
                                                            const QString &layout,
                                                            const QString &section)
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
QByteArray HbThemeServerPrivate::handleSharedEffectAddAndFileLookup(int request,
                                                                    const QString &fileName)
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
QByteArray HbThemeServerSession::handleStyleSheetLookup(int request,
    const QString &fileName, HbLayeredStyleLoader::LayerPriority priority)
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
            bool inSharedCache = false;
            offset = HbThemeServerUtils::getSharedStylesheet(fileName, priority, &inSharedCache);
            if (!inSharedCache) {
                if (offset >= 0) {
                    HbCacheItem *cssItem = new HbCacheItem(offset, 0, fileName);
                    insertKeyIntoSessionList = iServer->insertCssCacheItem(fileName, cssItem);
                    if (priority == HbLayeredStyleLoader::Priority_Core
                        && cssItem->refCount == 1) {
                        // This will make sure the requested stylesheet will always remain
                        // in the primary and secondary cache.
                        cssItem->incrementRefCount();
                    }
                    if (priority == HbLayeredStyleLoader::Priority_Theme
                        && cssItem->refCount == 1) {
                        iServer->themePriorityItems.insert(fileName, cssItem);
                    }
                    break;
                } else if (offset == OUT_OF_MEMORY_ERROR && tryAgain == false) {
                    iServer->doCleanup();
                    tryAgain = true;
                } else if (offset == OUT_OF_MEMORY_ERROR && tryAgain == true) {
                    //try only once to free up memory.
                    tryAgain = false;
                }
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
QByteArray HbThemeServerSession::handleIconLookup(const HbIconKey &key, HbSharedIconInfo &data,
                                                  int options)
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
				iServer->currentRenderingMode(),
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
