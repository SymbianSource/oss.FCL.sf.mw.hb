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

#ifndef HBSMILEYTHEME_H
#define HBSMILEYTHEME_H

#include <hbglobal.h>
#include <QExplicitlySharedDataPointer>
#include <QMetaType>
#include <QString>
#include <QStringList>

class HbSmileyThemePrivate;

class HB_CORE_EXPORT HbSmileyTheme
{

public:
    HbSmileyTheme();
    HbSmileyTheme(const QString &fileName);
    HbSmileyTheme(const HbSmileyTheme &other);
    HbSmileyTheme &operator=(const HbSmileyTheme &other);
    ~HbSmileyTheme();
    
    bool load (const QString &fileName);

    bool isNull() const;
    inline bool operator!() const { return isNull(); }

    void insert(const QString &smiley, const QString &pattern);
    void insert(const QString &smiley, const QStringList &patterns);
    void replace(const QString &smiley, const QStringList &patterns);
    void replace(const QString &smiley, const QString &pattern);
    void remove(const QString &smiley);
    void removePattern(const QString &pattern);
    void clear();

    QString smiley(const QString &pattern) const;
    QStringList smileys() const;

    QStringList patterns(const QString &smiley) const;
    QStringList patterns() const;

    operator QVariant() const;

private:
    QExplicitlySharedDataPointer<HbSmileyThemePrivate> d;
};

Q_DECLARE_METATYPE(HbSmileyTheme)

#endif // HBSMILEYTHEME_H
