/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbUtils module of the UI Extensions for Mobile.
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



/*!
    @proto
    @hbutils
    \class HbThemeServices
    \brief HbThemeServices class is used to set and get current theme.

    HbThemeServices class has static functions to change and query current theme. 
*/ 

#include "hbthemeservices_r.h"
#include "hbthemeutils_p.h"

#ifdef Q_OS_SYMBIAN
#include <e32property.h>
#include "hbthemecommon_symbian_p.h"
#else
#include "hbthemeclient_p.h"
#endif

#include <QSettings>

/*!
    Sets the active theme that is used with the Hb applications. HbTheme changed() signal will be emitted if theme change is
    applied succesfully. In addition to the active theme content loading also the underlying priority themes will be updated
    during the theme change.

    Depending on the platform setTheme functionality might by restricted.
    
    \param themePath absolute path to the folder where themes index.theme file is located.
*/
void HbThemeServices::setTheme(const QString &themePath)
{
#ifdef Q_OS_SYMBIAN
    RProperty themeRequestProp;
    
    User::LeaveIfError( themeRequestProp.Attach( KServerUid3, KNewThemeForThemeChanger ) );
    
    TBuf<256> newThemenameChangeRequest;
    _LIT(KThemeRequestFormatter, "%d:%S");
    TBuf<256> newThemename(themePath.utf16());
    newThemenameChangeRequest.Format(KThemeRequestFormatter, EThemeSelection, &newThemename);
    themeRequestProp.Set(newThemenameChangeRequest);
    themeRequestProp.Close();
#else
    HbThemeClient::global()->setTheme(themePath);
#endif  
}
    
/*!
    Returns the absolute path to the active theme.

    \return absolute path to the folder where the index.theme file of the active theme is located.        
*/
const QString HbThemeServices::themePath()
{
    QString path("");
    HbThemeIndexInfo info = HbThemeUtils::getThemeIndexInfo(ActiveTheme);
    if (info.address) {
        path.append(info.path);
        path.append("/icons/");
        path.append(info.name);
    }
    return path;
}

/*!
    Returns the list of available themes.

    \return list of themes. First item of QPair is the unlocalized theme name
    and the second item is the absolute path to the theme.
*/
const QList<QPair<QString, QString> > HbThemeServices::availableThemes()
{
    QList<QPair<QString, QString> > themes;

    QStringList rootDirs;
#ifdef Q_OS_SYMBIAN
    QFileInfoList driveList = QDir::drives();
    QString themesPath = "resource/hb/themes";
    foreach(const QFileInfo & drive, driveList) {
        QDir themeFolder(drive.absolutePath() + themesPath);
        if (themeFolder.exists()) {
            rootDirs << themeFolder.absolutePath();
        }
    }
#else
    QString envDir = QDir::fromNativeSeparators(qgetenv("HB_THEMES_DIR"));
    if (!envDir.isEmpty())
        rootDirs << envDir + "/themes";
#endif

    foreach(const QString &rootDir, rootDirs) {
        QDir root = rootDir;
        QDir dir = rootDir+"/icons";
        QStringList themeNamesList = dir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot,QDir::Name);
        foreach(QString themeName, themeNamesList) {
            QDir iconThemePath(dir.path()+'/'+themeName);
            QFile themeIndexFile(root.path()+'/'+themeName+".themeindex");
            if(themeIndexFile.exists() && iconThemePath.exists("index.theme")) {
                QSettings iniSetting(iconThemePath.path()+"/index.theme",QSettings::IniFormat);
                iniSetting.beginGroup("Icon Theme");
                bool hidden = iniSetting.value("Hidden",true).toBool();
                QString name = iniSetting.value("Name").toString();
                iniSetting.endGroup();
                if(!hidden && !name.isEmpty()) {
                    themes.append(QPair<QString,QString>(name, iconThemePath.absolutePath()));
                }
            }
        }
    }

    return themes;
}
