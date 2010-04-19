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

#ifndef HBTHEMECLIENTSYMBIAN_P_H
#define HBTHEMECLIENTSYMBIAN_P_H


#include <QIcon>

#include "hbthemecommon_p.h"
#include "hbiconloader_p.h"
#include "hblayeredstyleloader_p.h"
//ToDo: replace this with forward declaration
#include "hbcssparser_p.h"
#include "hbdeviceprofile_p.h"
#include "hbthemeindex_p.h"
#ifdef HB_SGIMAGE_ICON
#include <sgresource/sgimage.h>
#endif
#if !defined(__E32BASE_H__)
#include <e32base.h>
#endif

class CThemeListenerPrivate;
class QSizeF;
class HbEffectFxmlData;
struct LayoutDefinition;

class HB_AUTOTEST_EXPORT HbThemeClientPrivate : public RSessionBase
{
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
                                       const QColor &color);

    HbCss::StyleSheet *getSharedStyleSheet(const QString &filepath, HbLayeredStyleLoader::LayerPriority priority);

    LayoutDefinition *getSharedLayoutDefs(const QString &fileName,const QString &layout,const QString &section);

    HbDeviceProfileList *deviceProfiles();

    int globalCacheOffset();
 
    void unloadIcon(const QString& iconPath,
                    const QSizeF &size,
                    Qt::AspectRatioMode aspectRatioMode,
                    QIcon::Mode mode,
                    bool mirrored,
                    const QColor &color);
    
    void unLoadMultiIcon(const QStringList& iconPathList, 
                    const QVector<QSizeF> &sizeList,
                    Qt::AspectRatioMode aspectRatioMode,
                    QIcon::Mode mode,
                    bool mirrored,
                    const QColor &color);   
    
    HbEffectFxmlData *getSharedEffect(const QString &filePath);
    
    bool addSharedEffect(const QString &filePath);
    
    void getThemeIndexTables(ThemeIndexTables &tables);
	
    HbSharedIconInfo getMultiPartIconInfo(const QStringList &multiPartIconList, 
                                          const HbMultiPartSizeData &multiPartIconData ,
                                          const QSizeF &size,
                                          Qt::AspectRatioMode aspectRatioMode,
                                          QIcon::Mode mode,
                                          bool mirrored,
                                          HbIconLoader::IconLoaderOptions options,
                                          const QColor &color);
    
    HbSharedIconInfoList getMultiIconInfo(const QStringList &multiPartIconList, 
                                        const QVector<QSizeF>  &sizeList ,
                                        Qt::AspectRatioMode aspectRatioMode,
                                        QIcon::Mode mode,
                                        bool mirrored,
                                        HbIconLoader::IconLoaderOptions options,
                                        const QColor &color);
    
    ~HbThemeClientPrivate();
    
public:
    bool clientConnected;

private:
    TVersion Version() const;
    TInt StartServer();
    TInt CreateServerProcess();
    
private:
    CThemeListenerPrivate *themelistener;
#ifdef HB_SGIMAGE_ICON
    RSgDriver sgDriver;
#endif
};

#endif /* HBTHEMECLIENTSYMBIAN_P_H */
