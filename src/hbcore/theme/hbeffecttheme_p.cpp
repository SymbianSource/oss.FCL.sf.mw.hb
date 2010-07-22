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

#include "hbeffecttheme_p.h"
#include "hbthemecommon_p.h"
#include "hbeffectinternal_p.h"
#include "hbthemeindex_p.h"
#include "hbtheme.h"
#include "hbtheme_p.h"

#include <QDebug>
#include <QDir>

static const char *effectFileSuffix = ".fxml";

class HB_AUTOTEST_EXPORT HbEffectThemePrivate
{
public:
    HbEffectThemePrivate();
    ~HbEffectThemePrivate();

    void initialise(const QString &themeName);
public:
    QString mThemeName;
};

void HbEffectThemePrivate::initialise(const QString &themeName)
{
    mThemeName = themeName;
}

HbEffectThemePrivate::HbEffectThemePrivate()
    : mThemeName()
{
}

HbEffectThemePrivate::~HbEffectThemePrivate()
{
}

Q_GLOBAL_STATIC(HbEffectTheme, globalEffectTheme)
HbEffectTheme *HbEffectTheme::self = 0;

HbEffectTheme *HbEffectTheme::instance()
{
    return globalEffectTheme();
}

QString HbEffectTheme::getEffectXml(const QString &fileNameLogical, bool &fromTheme) const 
{
#ifdef THEME_INDEX_TRACES
    qDebug() <<  "ThemeIndex: getEffectXml effect: " << fileNameLogical;
#endif  

    if (HbThemeUtils::isLogicalName(fileNameLogical)) {
        // Try to get themed icon information from theme index
        QString resourceName(fileNameLogical);
        resourceName.append(effectFileSuffix);

        HbThemeIndexResource resource(resourceName);
        if (resource.isValid()) {
            fromTheme = true;
            return resource.fullFileName();
        }
    }

    // Not a logical name or not found in theme
    fromTheme = false;
    return fileNameLogical;   
}

HbEffectTheme::HbEffectTheme()
  : d_ptr(new HbEffectThemePrivate)
{
    self = this;
}

HbEffectTheme::~HbEffectTheme()
{
    delete d_ptr;
}

void HbEffectTheme::setCurrentTheme(const QString &themeName)
{
    d_ptr->initialise(themeName);
    HbEffectInternal::reloadFxmlFiles();
}

void HbEffectTheme::clearDirList()
{
    d_ptr->mThemeName.clear();
}

QString HbEffectTheme::currentTheme() const
{
    return d_ptr->mThemeName;
}
