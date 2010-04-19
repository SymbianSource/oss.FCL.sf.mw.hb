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

#include <qglobal.h>
#if defined(Q_OS_SYMBIAN)
#include <f32file.h>
#include <eikenv.h> 
#endif
#include <QString>
#include <QCoreApplication>
#include <QFileInfo>

#include <hbfindfile.h>

/*!
    @beta
    @hbcore
    \class HbFindFile
    \brief Checks from which drive a certain file is found.
    Scans drives through and adds drive information to \a str if file is found.

    \param str is file and path beginning with "/"
    \param defaultDrive is drive letter which should be checked first. Default value is null.
    \return true if file is found. Otherwise return false.
*/
bool HbFindFile::hbFindFile(QString &str, const QChar &defaultDrive)
{
    QString file = str;
#if defined(Q_OS_WIN32)
    file = "C:" + str;
#endif
#if !defined(Q_OS_SYMBIAN)
    QFileInfo info(file);
    if (info.exists()) {
        str = file;
        return true;
    }
    return false;
#endif

    if (!defaultDrive.isNull()) {
        file = defaultDrive + QString(":") + str;
        QFileInfo info(file);
        if (info.exists()) {
            str = file;
            return true;
        }
    }
    
    QString drives = availableDrives();
    for (int i = 0; i < drives.size(); i++) {
        if (drives.at(i) == defaultDrive) {
    	    continue;
        }
        file = drives.at(i) + QString(":") + str;
        QFileInfo info(file);
        if (info.exists()) {
            str = file;
            return true;
        }
    }
    return false;
}

/*!
    Returns available drives in device (Symbian). Empty for other platforms.
*/
QString HbFindFile::availableDrives()
{
    QString drives = "";
#if defined(Q_OS_SYMBIAN)
    RFs& fs = CCoeEnv::Static()->FsSession();
    TDriveList driveList;
    fs.DriveList(driveList);
    TChar driveLetter;
    for (TInt driveNumber = EDriveA; driveNumber <= EDriveZ; driveNumber++) {
        fs.DriveToChar(driveNumber, driveLetter);
        QChar c = static_cast<QChar>(driveLetter);
        drives.append(c);
    }
#endif
    return drives;
}
