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

#ifndef HBTHEMESERVERUTILS_P_H
#define HBTHEMESERVERUTILS_P_H

#define OUT_OF_MEMORY_ERROR -2

#include <QIcon>

#include "hblayeredstyleloader_p.h"
#include "hbthemecommon_p.h"
#include "hbsharedcache_p.h"
#include "hbiconloader_p.h"
#include "hbcssparser_p.h"
#include "hbiconprocessor_p.h"

class QString;
class QSizeF;

class HbThemeServerUtils
{
public:
    static HbIconSource *getIconSource(const QString &filename);
    static QString formatFromPath(const QString &iconPath);
    static int getSharedStylesheet(const QString &fileName,
                                   HbLayeredStyleLoader::LayerPriority priority,
                                   bool &fileExists,
                                   bool *inSharedCache = 0);
    static int getMissedHbCssFilesOffset();
    static bool parseCssFile(HbCss::Parser &parser, 
                             const QString &fileName, 
                             int &cssOffset,
                             bool &fileExists);
    static int getSharedEffect(const QString &fileName);
    static int getSharedLayoutDefinition(const QString & fileName,
                                         const QString &layout,
                                         const QString &section,
                                         bool &fileExists);
    static void createDeviceProfileDatabase();
    static bool removeSharedEffect(const QString &fileName);
    static void clearSharedEffects();
    static int computeCpuCost(const HbIconKey & key, HbIconFormatType iconType,
                                bool isConsolidatedIcon);
    static int computeGpuCost(const HbIconKey & key, HbIconFormatType iconType,
                                bool isConsolidatedIcon);
    static int computeFrameItemCpuCost(const HbIconKey & key, 
                                                            HbIconFormatType iconType,
                                                            HbRenderingMode rm); 

    static void createThemeIndex(const QString &themePath, const HbThemeType &themetype);

private:
    static void initMissedHbCssFilesList();
};

#endif // HBTHEMESERVERUTILS_P_H
