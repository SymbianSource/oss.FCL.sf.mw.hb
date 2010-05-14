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
#include "hbthemecommon_p.h"
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
#endif

class CHbThemeListenerPrivate;
class QSizeF;
class HbEffectFxmlData;
#ifndef Q_OS_SYMBIAN
class QLocalSocket;
#endif

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
    bool connectToServer();

    QSizeF getSharedIconDefaultSize(const QString& iconPath);

    HbSharedIconInfo getSharedIconInfo(const QString& iconPath ,
                                       const QSizeF &size,
                                       Qt::AspectRatioMode aspectRatioMode,
                                       QIcon::Mode mode,
                                       bool mirrored,
                                       HbIconLoader::IconLoaderOptions options,
                                       const QColor &color,
                                       HbRenderingMode renderMode);						

    HbWidgetLoader::LayoutDefinition *getSharedLayoutDefs(const QString &fileName, const QString &layout, const QString &section);

    HbCss::StyleSheet *getSharedStyleSheet(const QString &filepath, HbLayeredStyleLoader::LayerPriority priority);

    HbEffectFxmlData *getSharedEffect(const QString &filePath);

    HbDeviceProfileList *deviceProfiles();

    HbTypefaceInfoVector *typefaceInfo();

    bool addSharedEffect(const QString& filePath);

    void unloadIcon(const QString& iconPath ,
                        const QSizeF &size,
                        Qt::AspectRatioMode aspectRatioMode,
                        QIcon::Mode mode,
                        bool mirrored,
                        const QColor &color,
                        HbRenderingMode renderMode);

    void unLoadMultiIcon(const QStringList& iconPathList,
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
                                        const QVector<QSizeF>  &sizeList ,
                                        Qt::AspectRatioMode aspectRatioMode,
                                        QIcon::Mode mode,
                                        bool mirrored,
                                        HbIconLoader::IconLoaderOptions options,
                                        const QColor &color,
                                        HbRenderingMode renderMode);
    void notifyForegroundLostToServer();
    bool switchRenderingMode(HbRenderingMode renderMode);
    int freeSharedMemory();
    int allocatedSharedMemory();
    int allocatedHeapMemory();

    ~HbThemeClientPrivate();
    bool event(QEvent *e);

#ifdef HB_THEME_SERVER_MEMORY_REPORT
    void createMemoryReport() const;
#endif

#ifndef Q_OS_SYMBIAN
public slots:
    void changeTheme();
#endif

public:
    bool clientConnected;
private:
    void handleThemeChange(const QString &themeName);
#ifdef Q_OS_SYMBIAN
    TVersion Version() const;
    TInt StartServer();
    TInt CreateServerProcess();
#else
    void readIconInfo(QDataStream &dataStream, HbSharedIconInfo &iconInfo);
#endif

private:
#ifdef Q_OS_SYMBIAN
    CHbThemeListenerPrivate *themelistener;
    friend class CHbThemeListenerPrivate;
#ifdef HB_SGIMAGE_ICON
    RSgDriver sgDriver;
    bool sgDriverInit;
#endif

#else
    QLocalSocket* localSocket;
#endif
};

#endif // HBTHEMECLIENT_P_P_H
