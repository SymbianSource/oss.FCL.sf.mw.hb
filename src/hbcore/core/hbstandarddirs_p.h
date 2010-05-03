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

#ifndef HBSTANDARDDIRS_P_H
#define HBSTANDARDDIRS_P_H

#include <QtCore/QString>
#include <QDir>
#include <hbnamespace.h>

// The theme root path is platform-dependent
#if defined(Q_OS_WIN)
const QString rootPathsFile = "c:/theme/themerootsdir.txt";
#elif defined(Q_OS_SYMBIAN)
const QString rootPathsFile = "c:/data/theme/themerootsdir.txt";
#elif defined(Q_OS_MAC)
const QString rootPathsFile = QDir::homePath() + QString( "Library/UI Extensions for Mobile/themes/themerootsdir.txt" );
#elif defined(Q_OS_UNIX)
const QString rootPathsFile = "/usr/local/hb/theme/themerootsdir.txt";
#endif

// Standard theme root dirs
extern const char *coreResourcesRootDir;
// WARNING: This API is at prototype level and shouldn't be used before
// the resource fetching with theming is fully implemented
class HbStandardDirs
{
public:
    static QString findResource(
        const QString &name,
        Hb::ResourceType resType );

    static void findResourceList(
        QMap<int,QString> &pathList,
        Hb::ResourceType resType, bool assumeAbsolutesExists = false);

    static QStringList findExistingFolderList(
        const QStringList &relativeFolderPaths,
        const QString &currentThemeName, Hb::ResourceType resType);

    static const QString &themesDir();
};

#endif
