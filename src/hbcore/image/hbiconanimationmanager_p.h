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

#ifndef HBICONANIMATIONMANAGER_P_H
#define HBICONANIMATIONMANAGER_P_H

#include "hbiconanimationmanager.h"
#include "hbiconanimationparser_p.h"
#include "hbiconanimationdatatypes_p.h"
#include <hbglobal.h>
#include <QStringList>
#include <QSet>

class HbForegroundWatcher;
class HbIconAnimation;
class HbView;

class HB_CORE_PRIVATE_EXPORT HbIconAnimationManagerPrivate : public QObject
{
    Q_OBJECT

public:
    HbIconAnimationManagerPrivate();
    ~HbIconAnimationManagerPrivate();

    bool addDefinitionFile(const QString &definitionFileName);
    
    void addDefinition(
        const QString &iconName,
        const HbIconAnimationDefinition &definition);

    void removeDefinitionFile(const QString &definitionFileName);
    void removeDefinition(const QString &iconName);

    HbIconAnimationDefinition getDefinition(const QString &iconName);

    void animPlaying(HbIconAnimation *anim);
    void animNotPlaying(HbIconAnimation *anim);

    static HbIconAnimationManagerPrivate *d_ptr(HbIconAnimationManager *mgr)
    {
        return mgr->d;
    }

public slots:
    void handleAnimationResume();
    void handleAnimationPause();
    void handleThemeChanged();
    void handleThemeChangeFinished();
    void handleViewChanged(HbView*);

private:
    Q_DISABLE_COPY(HbIconAnimationManagerPrivate)

    HbIconAnimationParser parser;

    // Save some memory. Filenames are not duplicated in each item in AnimHash.
    QStringList definitionFileNames;
    // Animation definitions are contained here
    AnimHash animations;

    HbForegroundWatcher *fgWatcher;

    QVector<HbIconAnimation *> playingAnims;

    // All the definition names that were added via this instance.
    // Needed for proper theme-change handling. See handleThemeChangeFinished().
    QSet<QString> allDefNames;

    bool viewChangeConnected;
};

#endif // HBICONANIMATIONMANAGER_P_H
