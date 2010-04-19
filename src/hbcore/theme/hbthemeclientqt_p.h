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

#ifndef HBTHEMECLIENTQT_P_H
#define HBTHEMECLIENTQT_P_H

#include <QIcon>
#include "hbthemecommon_p.h"
#include "hbiconloader_p.h"
#include "hbdeviceprofiledatabase_p.h"
#include "hblayeredstyleloader_p.h"
//ToDo: replace this with forward declaration
#include "hbcssparser_p.h"
#include "hbthemeindex_p.h"

class QString;
class QSizeF;
class QLocalSocket;
class HbEffectFxmlData;
struct LayoutDefinition;

class HB_AUTOTEST_EXPORT HbThemeClientPrivate : public QObject
{
    Q_OBJECT

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
   
    LayoutDefinition *getSharedLayoutDefs(const QString &fileName, const QString &layout, const QString &section);
   
    HbCss::StyleSheet *getSharedStyleSheet(const QString &filepath, HbLayeredStyleLoader::LayerPriority priority);

    HbEffectFxmlData *getSharedEffect(const QString &filePath);
    
    HbDeviceProfileList *deviceProfiles();

    int globalCacheOffset();

    bool addSharedEffect(const QString& filePath);
    
    void unloadIcon(const QString& iconPath , 
                        const QSizeF &size,
                        Qt::AspectRatioMode aspectRatioMode,
                        QIcon::Mode mode,
                        bool mirrored,
                        const QColor &color);

    HbSharedIconInfo getMultiPartIconInfo(const QStringList &multiPartIconList, 
                        const HbMultiPartSizeData &multiPartIconData,
                        const QSizeF &size,
                        Qt::AspectRatioMode aspectRatioMode,
                        QIcon::Mode mode,
                        bool mirrored,
                        HbIconLoader::IconLoaderOptions options,
                        const QColor &color);
    bool event(QEvent *e);

    void getThemeIndexTables(ThemeIndexTables &tables);

    ~HbThemeClientPrivate();
    
public slots:
    void changeTheme();

public:
    bool clientConnected;

private:
    void readIconInfo(QDataStream &dataStream, HbSharedIconInfo &iconInfo);
    void handleThemeChangeRequest(QDataStream &dataStream);

private:
    QLocalSocket* localSocket;
};

#endif // HBTHEMECLIENTQT_P_H
