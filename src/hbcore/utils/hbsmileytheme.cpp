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

#include "hbsmileytheme.h"
#include "hbsmileythemeparser_p.h"
#include <QVariant>
#include <QSharedData>
#include <QHash>
#include <QSet>

class HbSmileyThemePrivate : public QSharedData
{
public:

    HbSmileyThemePrivate();
    HbSmileyThemePrivate(const HbSmileyThemePrivate &other);
    ~HbSmileyThemePrivate();

public: // data

    QHash<QString,QSet<QString> > smileyToPatterns;
    QHash <QString, QString> patternToSmiley;
};

HbSmileyThemePrivate::HbSmileyThemePrivate()
{
}

HbSmileyThemePrivate::HbSmileyThemePrivate(const HbSmileyThemePrivate &other) : QSharedData(other),
        smileyToPatterns(other.smileyToPatterns),
        patternToSmiley(other.patternToSmiley)
{
}

HbSmileyThemePrivate::~HbSmileyThemePrivate()
{
}


/*!
    @beta
    @hbcore
    \class HbSmileyTheme
    \brief HbSmileyTheme is container class for defining a smiley theme.

    Smiley is a sequence of keyboard characters used to represent a person's mood or emotion, especially :) or :-)
    or other smiling depiction. Smileys are also represented by graphics so called emoticons.
    In HbSmileyTheme \a smiley refers to emoticons and \a pattern refers to character sequence representation of a smiley.
    In a smiley theme each smiley is associated with one or multiple patterns. Each smiley is identified by a string that
    is the file name of the graphics file representing the corresponding emoticon.

    This class implements copy-on-write semantics, so copy constructor and assignment operator
    only perform shallow copy and are fast. A deep copy is done if the theme properties are changed.

    Brief example of how to create a smiley themes and use it.

    Example usage:
    \dontinclude ultimatecodesnippet/ultimatecodesnippet.cpp
    \skip Start of snippet 46
    \until End of snippet 46

    Smiley themes can be loaded from smiley theme (.sml) files.

    Example usage:
    \dontinclude ultimatecodesnippet/ultimatecodesnippet.cpp
    \skip Start of snippet 47
    \until End of snippet 47

    Here is an example of a smiley theme file describing the file syntax.

    \include unittest_hbsmileytheme/my_smileys_theme.sml


    \sa HbAbstractEdit
*/


/*!
* Default constructor. Constructs a null smiley theme.
*
*  \sa isNull()
*/
HbSmileyTheme::HbSmileyTheme()
{
    d = new HbSmileyThemePrivate();
}

/*!
* Constructs a smiley theme from a smiley theme definition file.
* If the file does not exist or is of an unknown format, the smiley theme becomes a null theme.
*
*  \sa isNull() load()
*/
HbSmileyTheme::HbSmileyTheme(const QString &fileName)
{
    d = new HbSmileyThemePrivate();
    load(fileName);
}


/*!
*   Constructs a smiley theme that is a copy of the given smiley theme \a other.
*/
HbSmileyTheme::HbSmileyTheme(const HbSmileyTheme &other) : d(other.d)
{
}

/*!
*   Assigns the given smiley theme \a other to this smiley theme and returns a reference
*   to this smiley theme.
*
*   \sa HbSmileyTheme()
*/
HbSmileyTheme &HbSmileyTheme::operator=(const HbSmileyTheme &other)
{
    if (&other != this) {
        d = other.d;
    }
    return *this;
}

/*!
* Destroys the smiley theme.
*/
HbSmileyTheme::~HbSmileyTheme()
{
}

/*!
* Loads a smiley theme from a smiley theme definition file
* Returns true if the smiley theme was successfully loaded; otherwise returns false.
*/
bool HbSmileyTheme::load (const QString &fileName)
{
    HbSmileyTheme tmpTheme;
    HbSmileyThemeParser parser;

    bool ret = parser.parseDefinitionFile(fileName,tmpTheme);
    if (ret) {
        *this = tmpTheme;
    }
    return ret;
}


/*!
* Returns true if the smiley theme doesn't contain any smileys.
* This is the case when the smiley theme is default constructed.
*/
bool HbSmileyTheme::isNull() const
{
    return d->smileyToPatterns.isEmpty();
}

/*!
* Inserts a smiley to the smiley theme with a given pattern.
* \a smiley identifies smiley and represents the icon name of the smiley.
* It can be any name that is valid for an icon name for HbIcon.
*
* \a pattern is a textual representation of the smiley.
* If the smiley already exists with \a smiley in the theme then \a pattern will be
* an additional textual representation for the smiley.
*
* \sa remove() replace()
*/
void HbSmileyTheme::insert(const QString &smiley, const QString &pattern)
{
    if (!smiley.isEmpty() && !pattern.isEmpty()) {
        d.detach();
        QSet<QString> patterns = d->smileyToPatterns.value(smiley);
        patterns << pattern;
        d->smileyToPatterns[smiley] = patterns;
        d->patternToSmiley[pattern] = smiley;
    }
}


/*!
* Inserts a smiley to the smiley theme with a given pattern list.
* \a smiley identifies smiley and represents the icon name of the smiley.
* It can be any name that is valid for an icon name for HbIcon.
*
*
* \a patterns is a list of textual representation of the smiley.
* If the smiley already exists with \a smiley in the theme then \a pattern list will be
* additional textual representations for the smiley.
*
* \sa remove() replace()
*/
void HbSmileyTheme::insert(const QString &smiley, const QStringList &patterns)
{
    foreach(QString pattern, patterns) {
        insert(smiley,pattern);
    }
}

/*!
* Replaces the smiley patterns for \a smiley.
* This is equivalent to calling remove(smiley) and subsequently insert(smiley,patterns).
*
* \sa remove() insert()
*/
void HbSmileyTheme::replace(const QString &smiley, const QStringList &patterns)
{
    remove(smiley);
    insert(smiley,patterns);
}

/*!
* Replaces the smiley patterns for \a smiley with a single \a pattern.
* This is equivalent to calling remove(smiley) and subsequently insert(smiley,pattern).
*
*
* \sa remove() insert()
*/
void HbSmileyTheme::replace(const QString &smiley, const QString &pattern)
{
    remove(smiley);
    insert(smiley,pattern);
}

/*!
* Removes \a smiley from the smiley theme along with its associated patterns.
*
* \sa insert() replace() removePattern()
*/
void HbSmileyTheme::remove(const QString &smiley)
{
    QSet<QString> tmpPatterns = d->smileyToPatterns.value(smiley);
    if (!tmpPatterns.isEmpty()) {
        d.detach();
        d->smileyToPatterns.remove(smiley);
        foreach(QString pattern, tmpPatterns) {
            d->patternToSmiley.remove(pattern);
        }
    }
}

/*!
* Removes \a pattern from the theme.
*
*
* \sa insert() replace() remove()
*/
void HbSmileyTheme::removePattern(const QString &pattern)
{
    QString smiley = d->patternToSmiley.value(pattern);
    if (!smiley.isEmpty()) {
        QSet<QString> tmpPatterns = d->smileyToPatterns.value(smiley);
        if (!tmpPatterns.isEmpty()) {
            tmpPatterns.remove(pattern);
            d.detach();
            if (tmpPatterns.isEmpty()) {
                d->smileyToPatterns.remove(smiley);
            } else {
                d->smileyToPatterns[smiley] = tmpPatterns;
            }
            d->patternToSmiley.remove(pattern);
        }
    }
}

/*!
*
* Remove all smileys from the theme and sets the smiley theme to the same state as
* when default constructed.
*
* \sa isNull()
*/
void HbSmileyTheme::clear()
{
    if (!isNull()) {
        d.detach();
        d->smileyToPatterns.clear();
        d->patternToSmiley.clear();        
    }
}

/*!
* Returns an existing smiley from the smiley theme for a given \a pattern
* If the smiley doesn't exist it returns a null string.
*
* \sa smileys()
*/
QString HbSmileyTheme::smiley(const QString &pattern) const
{
    return d->patternToSmiley.value(pattern);
}


/*!
* Returns the existing smileys from the smiley theme.
*
* \sa patterns()
*/
QStringList HbSmileyTheme::smileys() const
{
    return d->smileyToPatterns.keys();
}


/*!
* Returns the currently associated textual patterns for \a smiley.
*
* \sa smileys()
*/
QStringList HbSmileyTheme::patterns(const QString &smiley) const
{
    QSet<QString> tmpPatterns = d->smileyToPatterns.value(smiley);
    return tmpPatterns.toList();
}

/*!
* Convenience method.
* Returns the textual patterns for all the smileys in the theme.
*
* \sa smileys()
*/
QStringList HbSmileyTheme::patterns() const
{
    return d->patternToSmiley.keys();
}


/*!
* Returns the smiley theme as a QVariant.
*/
HbSmileyTheme::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

