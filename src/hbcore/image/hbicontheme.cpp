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
#include <hbstandarddirs_p.h>
#include <hbiniparser_p.h>

#define THEME_INDEX_FILE "index.theme"

class HbIconThemePrivate
{
public:
    HbIconThemePrivate();
    ~HbIconThemePrivate();

    QString m_theme;
    QStringList m_dirList;
    QString m_description;
    bool loaded;
    void loadThemeDescriptionFile(const QString &themePath, int priority);
    void loadThemeDescriptionFiles(const QString &theme);
    bool addBaseThemePath();
};

void HbIconThemePrivate::loadThemeDescriptionFiles(const QString &theme)
{
    const QString indextheme(THEME_INDEX_FILE);
    QString pathToTheme;
    QMap<int, QString> maplist = HbThemeUtils::constructHierarchyListWithPathInfo(
        QString(), theme, Hb::IconResource);
    QMapIterator<int, QString> i(maplist);
     i.toBack();
     while (i.hasPrevious()) {
         i.previous();
         pathToTheme = HbStandardDirs::findResource(i.value() + indextheme, Hb::IconResource);
         if (!pathToTheme.isEmpty()) {
             loadThemeDescriptionFile(pathToTheme, i.key());
         }
     }
    if (!addBaseThemePath()) {
         qDebug() << "Can't find base theme";
     }
     loaded = true;
}

HbIconThemePrivate::HbIconThemePrivate() : loaded(false)
{
}

HbIconThemePrivate::~HbIconThemePrivate()
{
}

void HbIconThemePrivate::loadThemeDescriptionFile(const QString &themePath, int priority)
{
    HbIniParser iniParser;
    QFile themeFile(themePath);

    if (!themeFile.open(QIODevice::ReadOnly) || !iniParser.read(&themeFile)) {
        qDebug() << "Can't access file : " << themePath;
        return;
    }
    if (priority == HbLayeredStyleLoader::Priority_Theme) {
        m_description = iniParser.value("Icon Theme", "Comment");
#ifdef Q_OS_SYMBIAN
        m_description = m_description.left(m_description.indexOf("\n", 0));
#endif
    }

    QString directories = iniParser.value("Icon Theme", "Directories");
    QStringList dirList = directories.split( ',', QString::SkipEmptyParts );
    QString indexThemeDir(themePath);
    indexThemeDir.chop(sizeof(THEME_INDEX_FILE) - 1);

    foreach (const QString &str, dirList) {
        m_dirList.append(QString(indexThemeDir + str + '/'));
    }
}

bool HbIconThemePrivate::addBaseThemePath()
{
    HbIniParser iniParser;
    const HbThemeInfo &baseThemeInfo = HbThemeUtils::baseTheme();
    QString baseThemePath = baseThemeInfo.rootDir + "/themes/icons/" + baseThemeInfo.name + "/"THEME_INDEX_FILE;

    // Parse it
    QFile baseThemeFile(baseThemePath);
    if (!baseThemeFile.open(QIODevice::ReadOnly) || !iniParser.read(&baseThemeFile)) {
        qDebug() << "Can't access file";
        return false;
    }

    if (m_theme == baseThemeInfo.name) {
        m_description = iniParser.value("Icon Theme", "Comment");
#ifdef Q_OS_SYMBIAN
        m_description = m_description.left(m_description.indexOf("\n", 0));
#endif
    }

    //Read parameters
    QString directories = iniParser.value("Icon Theme", "Directories");
    QStringList dirList = directories.split(',', QString::SkipEmptyParts);
    baseThemePath.chop(sizeof(THEME_INDEX_FILE) - 1);
    // Save paths
    foreach (const QString &str, dirList) {
        m_dirList.append(QString(baseThemePath + str + '/'));
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

void HbIconTheme::setCurrentTheme(const QString &theme)
{
    if (d->m_theme != theme) {
        d->m_theme = theme;
        d->m_dirList.clear();
        d->loaded = false;
    }
}

/**
 * Returns the name id and directory string of the current theme.
 */
QString HbIconTheme::currentTheme() const
{
    return d->m_theme;
}

/**
 * List of valid subdirectories of a theme
 */
QStringList HbIconTheme::dirList() const
{ 
    if (!d->loaded) {
        d->loadThemeDescriptionFiles(d->m_theme);
    }
    return d->m_dirList;
}

QString HbIconTheme::description() const
{
    if (!d->loaded) {
        d->loadThemeDescriptionFiles(d->m_theme);
    }
    return d->m_description;
}

void HbIconTheme::clearDirList()
{
    d->m_dirList.clear();
    d->loaded = false;
}

void HbIconTheme::emitUpdateIcons(const QStringList &fileNames)
{
    emit iconsUpdated(fileNames);
}
