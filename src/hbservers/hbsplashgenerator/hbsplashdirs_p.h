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

#ifndef HBSPLASHDIRS_P_H
#define HBSPLASHDIRS_P_H

#include <QStringList>

#ifdef Q_OS_SYMBIAN
#include <driveinfo.h>
#endif

inline QString hbsplash_output_dir()
{
#if defined(Q_OS_SYMBIAN)
    QString path("c:/private/2002E68B");
/*
#ifndef __WINSCW__
    TChar driveLetter;
    if (DriveInfo::GetDefaultDrive(DriveInfo::EDefaultMassStorage, driveLetter) == KErrNone) {
        path[0] = driveLetter;
    }
#endif
*/
    return path;
#else
    return QString("splashscreens");
#endif
}

// Watch out for the order of directories. They will be searched in the order in
// which they are in the list. If there are files with the same name at these
// locations then only one of the files is processed (the one that is
// encountered first).

inline QStringList hbsplash_splashml_dirs()
{
    QStringList dirs;
#if defined(Q_OS_SYMBIAN)
    dirs.append("c:/resource/hb/splashml");
    dirs.append("e:/resource/hb/splashml");
    dirs.append("f:/resource/hb/splashml");
    dirs.append("z:/resource/hb/splashml");
#else
    dirs.append("splashml");
#endif
    return dirs;
}

inline QStringList hbsplash_translation_dirs()
{
    QStringList dirs;
#if defined(Q_OS_SYMBIAN)
    dirs.append("c:/resource/qt/translations");
    dirs.append("e:/resource/qt/translations");
    dirs.append("f:/resource/qt/translations");
    dirs.append("z:/resource/qt/translations");
#else
    dirs.append("splashml");
#endif
    return dirs;
}

#endif
