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

#ifndef HBTHEMECLIENT_P_P_H
#define HBTHEMECLIENT_P_P_H

#include <QIcon>
#include <QQueue>
#include "hbthemecommon_p.h"
#include "hbthemeclient_p.h"
#include "hbiconloader_p.h"
#include "hbwidgetloader_p.h"
#include "hblayeredstyleloader_p.h"
#include "hbdeviceprofile_p.h"
#include "hbthemeindex_p.h"
#include "hbtypefaceinfodatabase_p.h"

#ifdef Q_OS_SYMBIAN
#ifdef HB_SGIMAGE_ICON
#include <sgresource/sgimage.h>
#endif
#include <e32base.h>
#endif // Q_OS_SYMBIAN

QT_BEGIN_NAMESPACE
class QFileSystemWatcher;
class QSizeF;
QT_END_NAMESPACE

class CHbThemeListenerPrivate;
class HbEffectFxmlData;
class QueueEntry;

class HB_AUTOTEST_EXPORT HbThemeClientPrivate :
#ifdef Q_OS_SYMBIAN
public RSessionBase
#else
public QObject
#endif

{
#ifndef Q_OS_SYMBIAN
    Q_OBJECT
#endif

public:
    HbThemeClientPrivate();

#ifdef Q_OS_SYMBIAN
    bool connectToServer();

    HbSharedIconInfo getSharedIconInfo(const HbThemeClient::IconReqInfo &reqInfo);

    void getSharedIconInfo(const HbThemeClient::IconReqInfo &reqInfo,
                           HbAsyncIconInfoCallback callback,
                           void *callbackParam);

    void cancelGetSharedIconInfo(HbAsyncIconInfoCallback callback,
                                 void *callbackParam);

    HbWidgetLoader::LayoutDefinition *getSharedLayoutDefs(const QString &fileName,
                                                          const QString &layout,
                                                          const QString &section,
                                                          bool &fileExists);

    HbCss::StyleSheet *getSharedStyleSheet(const QString &filePath,
                                           HbLayeredStyleLoader::LayerPriority priority,
                                           bool &fileExists);

    HbVector<uint> *getSharedMissedHbCss();

    HbEffectFxmlData *getSharedEffect(const QString &filePath);

    HbDeviceProfileList *deviceProfiles();

    HbTypefaceInfoVector *typefaceInfo();

    bool addSharedEffect(const QString& filePath);

    void unloadIcon(const HbThemeClient::IconReqInfo &reqInfo);

    void batchUnloadIcon(const QVector<HbThemeClient::IconReqInfo> &reqInfos);

    HbSharedIconInfo getMultiPartIconInfo(const QStringList &multiPartIconList,
                                          const HbMultiPartSizeData &multiPartIconData,
                                          const QSizeF &size,
                                          Qt::AspectRatioMode aspectRatioMode,
                                          QIcon::Mode mode,
                                          bool mirrored,
                                          HbIconLoader::IconLoaderOptions options,
                                          const QColor &color,
                                          HbRenderingMode renderMode);
    
    HbSharedIconInfoList getMultiIconInfo(const QStringList &multiPartIconList,
                                          const QVector<QSizeF>  &sizeList ,
                                          Qt::AspectRatioMode aspectRatioMode,
                                          QIcon::Mode mode,
                                          bool mirrored,
                                          HbIconLoader::IconLoaderOptions options,
                                          const QColor &color,
                                          HbRenderingMode renderMode);

    void unLoadMultiIcon(const QStringList& iconPathList,
                         const QVector<QSizeF> &sizeList,
                         Qt::AspectRatioMode aspectRatioMode,
                         QIcon::Mode mode,
                         bool mirrored,
                         const QColor &color,
                         HbRenderingMode renderMode);   

    void scheduleQueueCheck();
    void notifyForegroundLostToServer();
    bool switchRenderingMode(HbRenderingMode renderMode);
    int freeSharedMemory();
    int allocatedSharedMemory();
    int allocatedHeapMemory();
#ifdef HB_THEME_SERVER_MEMORY_REPORT
    void createMemoryReport() const;
#endif

    CHbThemeListenerPrivate *themelistener;

    CIdle *queueCheckInvoker;
    typedef QQueue<QueueEntry *> QueueType;
    QueueType reqQueue;

#ifdef HB_SGIMAGE_ICON
    RSgDriver sgDriver;
    bool sgDriverInit;
#endif

private:
    TVersion Version() const;
    TInt StartServer();
    TInt CreateServerProcess();
    void handleThemeChange(const QString &themeName);

#else

    QFileSystemWatcher *iniFileWatcher;

public slots:
    void iniFileChanged(QString iniFile);

#endif  // Q_OS_SYMBIAN

public:
    bool clientConnected;

    void setTheme(const QString &theme);

    ~HbThemeClientPrivate();

    friend class QueueEntry;
    friend class CHbThemeListenerPrivate;
};

#endif // HBTHEMECLIENT_P_P_H
