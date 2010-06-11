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

#ifndef HBTHEMESERVER_GENERIC_P_H
#define HBTHEMESERVER_GENERIC_P_H

#include <QMainWindow>
#include <QLabel>

#include "hbicondatacache_p.h"
#include "hbcache_p.h"
#include "hblayeredstyleloader_p.h"

class QLocalServer;
class QLocalSocket;
class HbThemeServerSession;
struct HbIconKey;

#ifdef QT_DEBUG
class HbThemeServerPrivate : public QMainWindow
#else
class HbThemeServerPrivate : public QObject
#endif
{
    Q_OBJECT

public:
#ifdef QT_DEBUG
    HbThemeServerPrivate(QWidget *parent = 0);
#else
    HbThemeServerPrivate();
#endif

    ~HbThemeServerPrivate();
    bool start();
    void stop();

    bool insertIconCacheItem(const HbIconKey &key, HbIconCacheItem *item);
    HbIconCacheItem *iconCacheItem(const HbIconKey &key);
    void setMaxGpuCacheSize(int size);
    void setMaxCpuCacheSize(int size);
    void removeIconCacheItem(const HbIconKey &key);
    void clearIconCache();
    void handleThemeSelection(const QString &newtheme);
    void handleContentUpdate(const QStringList &fileNames);

    QByteArray handleSharedEffectAddAndFileLookup(int request, const QString &fileName);
    QByteArray handleSharedWidgetMLLookup(const QString &fileName,
                                          const QString &layout, const QString &section);
    void writeToClients(QByteArray &block);

    bool insertCssCacheItem(const QString &key,  HbCacheItem *item);
    HbCacheItem *cssCacheItem(const QString &key);
    void removeCssCacheItem(const QString &key);
    void clearCssCache();

    void doCleanup();
    int sessionListCount() const;

    int freeSharedMemory() const;
    int allocatedSharedMemory() const;

	HbRenderingMode currentRenderingMode() const;
    void setCurrentRenderingMode(HbRenderingMode currentMode);

    bool resolveThemePath(const QString &themeName, QString &themePath);

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
#endif

private slots:
    void newClientConnected();
    void clienDisconnected();
    void removeFromList();
    bool listen();

public:
    QHash<QString, HbCacheItem*> themePriorityItems;

private:
#ifdef QT_DEBUG
    QLabel statusLabel;
#endif
    QLocalServer *server;
    QList<HbThemeServerSession *> sessionList;
    HbIconDataCache *iconCache;
    HbCache *cssCache;
	HbRenderingMode renderMode;

    QString iCurrentThemeName;
    QString iCurrentThemePath;
};

class HbThemeServerSession : public QObject
{
    Q_OBJECT
public:
    HbThemeServerSession(QLocalSocket *clientConnection, HbThemeServerPrivate *server);
    ~HbThemeServerSession();
    QLocalSocket *clientConnection();
    void fillOutPutDataStream(QDataStream &outputDataStream, HbSharedIconInfo &data,
                              HbThemeServerRequest request);
    void getDataFromCacheItem(HbIconCacheItem* cacheItem, HbSharedIconInfo &data) const;
    void freeDataFromCacheItem(HbIconCacheItem* cacheItem);
    QByteArray handleStyleSheetLookup(int request, const QString &fileName,
                                      HbLayeredStyleLoader::LayerPriority priority);
    QByteArray handleIconLookup(const HbIconKey &key, HbSharedIconInfo &data, int options);
    bool iconInfoFromSingleIcon(HbIconKey key, HbSharedIconInfo &stitchedData);
    bool createCacheItemData(HbIconKey key, int options, HbSharedIconInfo &data);
    bool createStichedIconInfoOfParts(QVector<HbSharedIconInfo> dataForParts,
                                      HbMultiIconParams params,
                                      HbIconKey &finalIconKey,
                                      HbSharedIconInfo &stitchedData);
    void iconInfoFromMultiParts(HbMultiIconParams params,
                                int noOfPieces,
                                HbIconKey &stichedKey,
                                HbSharedIconInfo &stitchedData);
    void clearSessionIconData();
    void removeSessionIconItem(const HbIconKey &key);
    void removeSessionCssItem(const QString &key);

public slots:
    void readDataFromClient();

private:
    HbThemeServerPrivate *iServer;
    QLocalSocket *iClientConnection;
    QList<HbIconKey> sessionIconData;
    QList<QString> sessionCssData;
};

#endif // HBTHEMESERVER_GENERIC_P_H

