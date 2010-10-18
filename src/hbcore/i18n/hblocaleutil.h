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

#ifndef HBLOCALEUTIL_H
#define HBLOCALEUTIL_H

#include <QHash>
#include <hbglobal.h>

struct HbLanguageRegionMapping
{
    QString languageId;
    QString regionId;
    QString collationId;
};

class HB_CORE_EXPORT HbLocaleUtil
{
public:
    static QString currentLanguage();
    static QStringList supportedLanguages();
    static QString localisedLanguageName( const QString &language );
    static bool changeLanguage( const QString &language );
    
    static QString currentRegion();
    static QStringList supportedRegions();
    static QString localisedRegionName( const QString &region );
    static bool changeRegion( const QString &region );
    
    static QString currentCollation();
    static QStringList supportedCollations();
    static QString localisedCollationName( const QString &collation );
    static bool changeCollation( const QString &collation );
    
    static bool changeLocale( const QString &language );
    
    static QList<HbLanguageRegionMapping> languageRegionMappings( bool onlySupported=true );
};

#endif /* HBLOCALEUTIL_H */
