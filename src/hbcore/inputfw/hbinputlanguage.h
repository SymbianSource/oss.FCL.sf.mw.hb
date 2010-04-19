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

#ifndef HB_INPUT_LANGUAGE_H
#define HB_INPUT_LANGUAGE_H

#include <QString>
#include <QLocale>

#include <hbinputdef.h>

class HB_CORE_EXPORT HbInputLanguage
{
public:
    HbInputLanguage() : mLanguage((QLocale::Language)0), mVariant(QLocale::AnyCountry)  
        {}

    HbInputLanguage(QLocale::Language language, QLocale::Country variant = QLocale::AnyCountry)
        : mLanguage(language), mVariant(variant)
        {}

    HbInputLanguage(const HbInputLanguage &other) {
        mLanguage = other.mLanguage;
        mVariant = other.mVariant;
    }

    QLocale::Language language() const { return mLanguage; }
    QLocale::Country variant() const { return mVariant; }
    bool undefined() const { return (mLanguage == (QLocale::Language)0); }
    bool defined() const { return (mLanguage != (QLocale::Language)0); }

    bool operator==(const HbInputLanguage &other) const {
        return (mLanguage == other.mLanguage && mVariant == other.mVariant);
    }

    bool operator==(const QLocale::Language language) const {
        return (mLanguage == language && mVariant == QLocale::AnyCountry);
    }

    bool operator!=(const HbInputLanguage &other) const {
        return (mLanguage != other.mLanguage || mVariant != other.mVariant);
    }

    bool operator!=(const QLocale::Language language) const {
         return (mLanguage != language);
    }

    QString localisedName();
    bool isCaseSensitiveLanguage() const;
    bool isLatinAlphabetLanguage() const;
    bool isRightToLeftLanguage() const;
    QString asString() const;
    void fromString(const QString &string);

protected:
    QLocale::Language mLanguage;
    QLocale::Country mVariant;
};

#endif // HB_INPUT_LANGUAGE_H

// End of file
