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
#ifndef HB_LANGUAGE_DATABASE_H
#define HB_LANGUAGE_DATABASE_H

#include <QObject>
#include <QList>

#include <hbinputdef.h>

class HB_CORE_EXPORT HbLanguageDatabase
{
public:
    virtual ~HbLanguageDatabase()
    {
    }

    virtual int hostId() const = 0;
    virtual int languageCode() const = 0;
    virtual char* languageData(void* aNativeParams = 0) const = 0;
    virtual int nativeId() const = 0;
};

class HB_CORE_EXPORT HbLanguageDatabaseInterface : public QObject
{
    Q_OBJECT

public:
    virtual ~HbLanguageDatabaseInterface()
    {
    }
    virtual QList<int> listLanguages() = 0;
    virtual HbLanguageDatabase* languageDatabase(int aLanguage) = 0;
};

#endif // HB_LANGUAGE_DATABASE_H

// End of file
