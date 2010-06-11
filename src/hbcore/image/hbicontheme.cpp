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

    void loadThemeDescriptionFile();

public:
    QString m_theme;
    QString m_description;
    QString m_name;
    bool loaded;
};

HbIconThemePrivate::HbIconThemePrivate() : loaded(false)
{
}

HbIconThemePrivate::~HbIconThemePrivate()
{
}

void HbIconThemePrivate::loadThemeDescriptionFile()
{
    // TODO: index file is accessed in several places, create utility
    HbThemeIndexInfo info = HbThemeUtils::getThemeIndexInfo(ActiveTheme);

    QString indexFileName;

    indexFileName.append(info.path);
    indexFileName.append("/icons/");
    indexFileName.append(info.name);
    indexFileName.append("/" THEME_INDEX_FILE);

    HbIniParser iniParser;
    QFile themeFile(indexFileName);

    if (!themeFile.open(QIODevice::ReadOnly) || !iniParser.read(&themeFile)) {
#ifdef HB_THEME_SERVER_TRACES 
        qDebug() << "HbIconTheme: Can't access file: " << indexFileName;
#endif
        return;
    }
    m_description = iniParser.value("Icon Theme", "Comment");
    m_name = iniParser.value("Icon Theme", "Name");
#ifdef Q_OS_SYMBIAN
    m_description = m_description.left(m_description.indexOf("\n", 0));
    m_name = m_name.left(m_name.indexOf("\n", 0));
#endif
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
    if (!theme.isEmpty()) {
        d->m_theme = theme;
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

QString HbIconTheme::description() const
{
    if (!d->loaded) {
        d->loadThemeDescriptionFile();
    }
    return d->m_description;
}

QString HbIconTheme::name() const
{
    if (!d->loaded) {
        d->loadThemeDescriptionFile();
    }
    return d->m_name;
}

void HbIconTheme::emitUpdateIcons(const QStringList &fileNames)
{
    emit iconsUpdated(fileNames);
}
