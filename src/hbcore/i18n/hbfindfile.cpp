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
#include <qbasicatomic.h>
#include <QDir>

/*!
    @beta
    @hbcore
    \class HbFindFile
    \brief Checks from which drive a certain file is found.
    Scans drives through and adds drive information to \a str if file is found.
    
    \attention Cross-Platform API

    \param str is file and path beginning with "/"
    \param defaultDrive is drive letter which should be checked first. Default value is null.
    
    \return True if file is found. Otherwise return false.
*/
bool HbFindFile::hbFindFile(QString &str, const QChar &defaultDrive)
{
#ifdef Q_OS_SYMBIAN
    RFs& fs = CCoeEnv::Static()->FsSession();
    TFindFile ff(fs);
    TPtrC fName((ushort*)(str.constData()),str.length());
    QString dNameString;
    
    if (!defaultDrive.isNull()) {
        dNameString.append(defaultDrive);
        dNameString += QString(":");
    }    
    dNameString += QString("\\");    
    TPtrC dName((ushort*)(dNameString.constData()),dNameString.length());
    TInt err=ff.FindByDir(fName, dName);
    if (err==KErrNone) {
        TParse p;
        p.Set(ff.File(), 0,0);
        TPtrC ptrC = p.Drive();
        QString str2 = QString::fromRawData((QChar*)(ushort*)ptrC.Ptr(),ptrC.Length()); 
        str.prepend(str2);
        return true;
    }    
    else {
        return false;
    }
#else
    QString file = str;
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
#endif
    
}

class AvailableDrives : public QString
{
public:
    AvailableDrives();    
};

AvailableDrives::AvailableDrives() {
#ifdef Q_OS_SYMBIAN    
    RFs& fs = CCoeEnv::Static()->FsSession();
    TDriveList driveList;
    fs.DriveList(driveList);
    TChar driveLetter;
    
    // add first C and then Y..A and then Z.
    TInt driveNumber;
    if (driveList[EDriveC]) {        
        driveNumber = EDriveC;
        fs.DriveToChar(driveNumber, driveLetter);
        QChar cC = static_cast<QChar>(driveLetter);    
        this->append(cC);
    }
    for (driveNumber = EDriveY; driveNumber >= EDriveA; driveNumber--) {
        if (driveNumber == EDriveC) {
            continue;
        }
        else {
            if (driveList[driveNumber]) {
                fs.DriveToChar(driveNumber, driveLetter);
                QChar c = static_cast<QChar>(driveLetter);
                this->append(c);
            }    
        }    
    }
    if (driveList[EDriveZ]) {    
        driveNumber = EDriveZ;
        fs.DriveToChar(driveNumber, driveLetter);
        QChar cZ = static_cast<QChar>(driveLetter);
        this->append(cZ);
    }

    
#else            
     QFileInfoList fil = QDir::drives();
     for (int j=0; j< fil.length(); j++) {
         QString fp = fil.at(j).filePath();
         
         if ((!fp.isEmpty()) && (fp[0] != '/') && (fp[0] != '\\')) {                
         this->append(fp[0]);
         }        
     }
#endif     
}

Q_GLOBAL_STATIC(AvailableDrives, gs_AvailableDrives)

/*!
    \attention Cross-Platform API
    
    \returns Available drive(s) if platform supports (Eg. Symbian, Windows...). 
    If platform doesn't support drive(s) (Eg. Linux) then empty QString is returned. 
*/
QString HbFindFile::availableDrives()
{
     QString *str = gs_AvailableDrives();
     if (str) {
         return *str;         
     }
     else {
         return QString(); 
     }
}
