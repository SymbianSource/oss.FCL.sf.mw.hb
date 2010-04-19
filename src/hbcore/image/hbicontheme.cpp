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

#include "hbicontheme_p.h"
#include "hbthemeutils_p.h"
#include <hbinstance.h>
#include "hbtheme.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QSettings>
#include <hbstandarddirs_p.h>
#include <hbiniparser_p.h>

static HbIconTheme *globalInst = 0;

class HbIconThemePrivate
{
public:
    HbIconThemePrivate();
    ~HbIconThemePrivate();

    QString m_dir;
    QStringList m_dirList;
    QString m_description;

    bool parseDesktopEntryFile(const QString &dir);
    bool initialise(const QString &dir);
    bool addDefaultPath();

};


bool HbIconThemePrivate::initialise(const QString &dir)
{
    bool success = false;

    QMap<int,QString> maplist = HbThemeUtils::constructHierarchyListWithPathInfo(
        QString(), dir, Hb::IconResource);
        
    // Cleanup old dirlist
    m_dirList.clear();
    QList<QString> list = maplist.values();
    for (int i = list.count() - 1; i >= 0; --i) {
        if (parseDesktopEntryFile(list.at(i))) {
            success = true;
        }
    }

    if (!addDefaultPath()) {
        qDebug() << "Can't find default theme";
    }
    return success;

}

HbIconThemePrivate::HbIconThemePrivate()
{
}

HbIconThemePrivate::~HbIconThemePrivate()
{
}

bool HbIconThemePrivate::parseDesktopEntryFile(const QString &dir)
{
    bool success = true;
    HbIniParser iniParser;
    QString themePath;
    QString indextheme("index.theme");

    themePath = HbStandardDirs::findResource( dir + indextheme, Hb::IconResource);
    if (themePath.isEmpty()) {
        return false;
    }

    QFile themeFile( themePath );

    if (!themeFile.open(QIODevice::ReadOnly) || !iniParser.read(&themeFile)) {
        qDebug() << "Can't access file : " << themePath;
        return false;
    }
    m_description = iniParser.value("Icon Theme", "Comment");

#ifdef Q_OS_SYMBIAN
    m_description = m_description.left(m_description.indexOf("\n", 0));
#endif

    QString directories = iniParser.value("Icon Theme", "Directories");

    QStringList dirList = directories.split( ',', QString::SkipEmptyParts );

    themePath.chop(indextheme.length());

    foreach (const QString &str, dirList) {
        m_dirList.append(QString(themePath + str + '/'));
    }

    return success;
}

bool HbIconThemePrivate::addDefaultPath()
{
    HbIniParser iniParser;
    QString defaultThemeName = HbThemeUtils::defaultTheme();
    QString defaultLocaleThemePath = HbStandardDirs::findResource("themes/icons/" + defaultThemeName 
                + "/locale/" + QLocale().name() + "/index.theme", Hb::IconResource);

    QString defaultThemePath = HbStandardDirs::findResource("themes/icons/" + defaultThemeName + "/index.theme", Hb::IconResource);
    QStringList defaultlist;
    if (!defaultLocaleThemePath.isEmpty()) {
        defaultlist << defaultLocaleThemePath;
    }
    if (!defaultThemePath.isEmpty()) {
        defaultlist << defaultThemePath;
    }
    foreach (QString defaultThemePath, defaultlist) {
        // Parse it
        QFile defaultThemeFile(defaultThemePath);
        if (!defaultThemeFile.open(QIODevice::ReadOnly) || !iniParser.read(&defaultThemeFile)) {
            qDebug() << "Can't access file";
            return false;
        }
        //Read parameters
        QString directories = iniParser.value("Icon Theme", "Directories");
        QStringList dirList = directories.split(',', QString::SkipEmptyParts);
        defaultThemePath.chop(sizeof("index.theme") - 1);
        // Save paths
        foreach (const QString &str, dirList) {
            m_dirList.append(QString(defaultThemePath + str + '/'));
        }
    }
    return true;
}


/*!
    \class HbIconTheme
    \brief HbIconTheme gives access to icon themes and stores icon theme properties 
           according to the Freedesktop Icon Theme Specification
*/
HbIconTheme::HbIconTheme()
    : d(new HbIconThemePrivate())
{
}

HbIconTheme::~HbIconTheme()
{
    delete d;
}

/**
 * Returns a pointer to the Singleton object.
 */
HbIconTheme *HbIconTheme::global()
{
    if (!globalInst) {
        globalInst = new HbIconTheme;    
    }
    return globalInst;
}

/**
 * Returns the name id and directory string of the current theme.
 */
QString HbIconTheme::currentTheme() const
{
    return d->m_dir;
}


/**
 * Allows HbTheme to set the current theme.
 */
void HbIconTheme::setCurrentTheme(const QString& dir)
{
    bool success = d->initialise(dir);

    if (!success) {
        qDebug() << "Theme not found. We stay with the current theme.";
        d->initialise(d->m_dir);
        return;
    }

    d->m_dir = dir;
}

/**
 * Name identifier / directory prefix
 */
QString HbIconTheme::dir() const
{
    return d->m_dir;
}

/**
 * List of valid subdirectories of a theme
 */
QStringList HbIconTheme::dirList() const
{ 
    if (!d->m_dirList.empty()) {
        return d->m_dirList;
    } else {
        d->initialise(d->m_dir);
        return d->m_dirList; 
    }
}

QString HbIconTheme::description() const
{
    return d->m_description;
}

void HbIconTheme::clearDirList()
{
	d->m_dirList.clear();
}
