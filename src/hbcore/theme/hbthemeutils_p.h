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

#ifndef HBTHEMEUTILS_P_H
#define HBTHEMEUTILS_P_H

#include <QList>
#include <hbglobal.h>
#include <hbnamespace.h>
#include <hblayeredstyleloader_p.h>
#include <QPair>

#undef USE_APPTHEMES

struct HbHierarchy
{
    HbHierarchy() {}
    HbHierarchy(QString name,
                HbLayeredStyleLoader::LayerPriority layerPriority) : name(name),
                                                                     layerPriority(layerPriority) {}
    QString name;
    HbLayeredStyleLoader::LayerPriority layerPriority;
};


struct HbThemeInfo
{
    HbThemeInfo()
    {
    }
    HbThemeInfo(const QString &themeName, const QString &dir):name(themeName),rootDir(dir)
    {
    }
    QString name;
    QString rootDir;

};


class HB_CORE_PRIVATE_EXPORT HbThemeUtils
{
public:
    static QVector<HbHierarchy> hierarchies();
    static void initSettings();

//following methods for unittests only
    static int addHierarchy(const QString& newHierarchy, int priorityOrder);
    static bool removeHierarchy(const QString &hierarchy);
    static QString operatorBasePath();
//unittest functions end.
    static QMap<int, QString> constructHierarchyListWithPathInfo(
                                        const QString &fileName,
                                        const QString &currentTheme,
                                        const Hb::ResourceType resType );

    enum Setting
    {
        CurrentThemeSetting = 1,
        DefaultThemeSetting = 2,
        DefaultThemeRootDirSetting = 3,
        BaseThemeSetting = 4,
        OperatorBasePathSetting = 5
    };

    static QString getThemeSetting(Setting setting);
    static void setThemeSetting(Setting setting, const QString &value);
    static void updateThemeSetting(Setting setting, const QString &value);
    

    static const HbThemeInfo &baseTheme();
    static HbThemeInfo defaultTheme();
    static bool isThemeValid(const HbThemeInfo &themeInfo);

    // Standard folder names
    static const char *iconsResourceFolder;
    static const char *effectsResourceFolder;
    static const char *styleResourceFolder;
    static const char *themeResourceFolder;
    static const char *operatorHierarchy;
    static const char *appHierarchy;
    static const char *platformHierarchy;

private:
    static HbThemeInfo getBaseThemeFromFile(const QString &rootDir);
};

#endif //HBTHEMEUTILS_P_H

