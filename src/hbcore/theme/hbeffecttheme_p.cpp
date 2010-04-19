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

#include <QDebug>
#include <QDir>

#include "hbstandarddirs_p.h"
#include "hbinstance.h"
#include "hbeffecttheme_p.h"
#include "hbthemeutils_p.h"
#include "hbeffectinternal_p.h"

class HB_AUTOTEST_EXPORT HbEffectThemePrivate
{
public:
    HbEffectThemePrivate();
    ~HbEffectThemePrivate();

    bool initialise(const QString &dir);
    QString mThemeName;
    QStringList mDirList;
    QStringList mListOfExistingFolders;
};

bool HbEffectThemePrivate::initialise(const QString &dir)
{
    QMap<int, QString> maplist = HbThemeUtils::constructHierarchyListWithPathInfo(
        QString(), dir, Hb::EffectResource);
        
    mDirList.clear();
        
    QList<QString> list = maplist.values(); // sorted by key
    for (int i = list.count() - 1; i >= 0; --i) { // take highest prio first
        mDirList.append(list.at(i));
    }

    mListOfExistingFolders = HbStandardDirs::findExistingFolderList(mDirList, Hb::EffectResource);
    return mListOfExistingFolders.count() > 0;
}

HbEffectThemePrivate::HbEffectThemePrivate()
    : mThemeName()
{
}

HbEffectThemePrivate::~HbEffectThemePrivate()
{
}

Q_GLOBAL_STATIC(HbEffectTheme, instance)

HbEffectTheme *HbEffectTheme::global()
{
    return instance();
}

QString HbEffectTheme::getEffectXml(const QString &fileNameLogical, bool &fromTheme) const 
{
    fromTheme = false;
    // Assuming logical name will not have '.' and full filepath will
    // always have some extension.
    if (!fileNameLogical.contains('.')) {
        foreach (const QString &dir, d_ptr->mListOfExistingFolders) {
            QString candidateFullName = dir + fileNameLogical + ".fxml";
            QFile resource(candidateFullName);
            if (resource.exists()) {
                if (d_ptr->mListOfExistingFolders.last() != dir) {
                    fromTheme = true;
                }
                return candidateFullName;
            }
        }
    }
    return fileNameLogical;
}

HbEffectTheme::HbEffectTheme()
  : d_ptr(new HbEffectThemePrivate)
{
}

HbEffectTheme::~HbEffectTheme()
{
    delete d_ptr;
}

void HbEffectTheme::setCurrentTheme(const QString& themeName)
{
    bool success = d_ptr->initialise(themeName);
    if (success && d_ptr->mListOfExistingFolders.count() == 2) {
        d_ptr->mThemeName = "hbdefault";
    } else {
        d_ptr->mThemeName = themeName;
    }
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
