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
#ifndef HBTHEMECLIENT_P_H
#define HBTHEMECLIENT_P_H

#include <qglobal.h>
#include "hbiconloader_p.h"
#include "hbthemecommon_p.h"
#include "hbwidgetloader_p.h"
#include "hbcssparser_p.h"
#include "hbdeviceprofiledatabase_p.h"
#include "hbtypefaceinfodatabase_p.h"
#include "hblayeredstyleloader_p.h"
#include "hbthemeindex_p.h"
#include "hbsharedcache_p.h"
#include <QByteArray>

class HbThemeClientPrivate;
class HbEffectFxmlData;

class HB_CORE_PRIVATE_EXPORT HbThemeClient
{
public:
    bool connectToServer();

    HbSharedIconInfo getSharedIconInfo(const QString &iconPath,
                        const QSizeF &size,
                        Qt::AspectRatioMode aspectRatioMode,
                        QIcon::Mode mode,
                        bool mirrored,
                        HbIconLoader::IconLoaderOptions options,
                        const QColor &color,
                        HbRenderingMode renderMode );

    QByteArray getSharedBlob(const QString &name);

    HbWidgetLoader::LayoutDefinition *getSharedLayoutDefs(const QString &fileName,
                                                          const QString &layout,
                                                          const QString &section);
    HbCss::StyleSheet *getSharedStyleSheet(
            const QString &filePath,
            HbLayeredStyleLoader::LayerPriority priority);

    HbEffectFxmlData *getSharedEffect(const QString &filePath);

    bool addSharedEffect(const QString &filePath);

    HbDeviceProfileList *deviceProfiles();
    HbTypefaceInfoVector *typefaceInfo();

    void notifyForegroundLostToServer();

    void unloadIcon(const QString &iconPath,
                        const QSizeF &size,
                        Qt::AspectRatioMode aspectRatioMode,
                        QIcon::Mode mode,
                        bool mirrored,
                        const QColor &color,
                        HbRenderingMode renderMode);

    void unLoadMultiIcon(const QStringList &iconPathList,
                        const QVector<QSizeF> &sizeList,
                        Qt::AspectRatioMode aspectRatioMode,
                        QIcon::Mode mode,
                        bool mirrored,
                        const QColor &color,
                        HbRenderingMode renderMode);

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
                            const QVector<QSizeF>  &sizeList,
                            Qt::AspectRatioMode aspectRatioMode,
                            QIcon::Mode mode,
                            bool mirrored,
                            HbIconLoader::IconLoaderOptions options,
                            const QColor &color,
                            HbRenderingMode renderMode);

    bool switchRenderingMode(HbRenderingMode renderMode);

    void setTheme(const QString &theme);
    
    bool clientConnected() const;

#ifdef HB_THEME_SERVER_MEMORY_REPORT
    void createMemoryReport() const;
#endif

    int freeSharedMemory();
    int allocatedSharedMemory();
    int allocatedHeapMemory();

public:
    static HbThemeClient *global();
    static void releaseInstance();
    ~HbThemeClient();
private:
    HbThemeClient();
    int sharedCacheItemOffset(HbSharedCache::ItemType type, const QString &key);
    int sharedCacheLayoutDefinitionOffset(const QString &filePath,
                                          const QString &layout,
                                          const QString &section);
    HbThemeClientPrivate *d_ptr;
    Q_DECLARE_PRIVATE_D(d_ptr, HbThemeClient)
};

#endif /* HBTHEMECLIENT_P_H */
