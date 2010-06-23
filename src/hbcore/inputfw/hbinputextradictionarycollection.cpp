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
#include "hbinputextradictionarycollection.h"

#include "hbinputextrauserdictionary.h"
#include "hbinputextradictionaryfactory.h"

/*!
@proto
@hbcore
\class HbExtraDictionaryCollection
\brief Virtual collection of extra user dictionaries.

This class binds together a number of extra user dictionaries. It knows how to
perform search and data access operations as if the collection was single big extra dictionary.
This feature may become handy if a prediction engine is able to attach only one extra
dictionary but the framework needs to attach several of them at once.

It is also possible to temporarily disable and enable dictionaries in the collection without
removing them from memory. That makes it possible to easily fine tune search operations
efficiently.

\sa HbExtraUserDictionary
\sa HbExtraDictionaryFactory
*/

/// @cond

class HbExtraDictionaryCollectionPrivate
{
public:
    HbExtraDictionaryCollectionPrivate();
    ~HbExtraDictionaryCollectionPrivate();

    void addDictionaries(QList<int>& newOnes);
    void addDictionary(int newId);
    void addDictionary(HbExtraUserDictionary *newOne);

    void removeDictionary(int dictId);
    void removeDictionary(HbExtraUserDictionary *dict);

    bool isAlreadyInList(int dictId) const;

    int isDisabled(int index) const {
        return ((0x0001 << index) & deactivated);
    }

    void enable(int index) {
        deactivated &= !(0x0001 << index);
    }

    void disable(int index) {
        deactivated |= (0x0001 << index);
    }

    int dictionaryIndex(int id) const;

public:
    QList<HbExtraUserDictionary *> dictionaries;
    unsigned short deactivated;     // Bit vector
};


HbExtraDictionaryCollectionPrivate::HbExtraDictionaryCollectionPrivate() : deactivated(0)
{
}

HbExtraDictionaryCollectionPrivate::~HbExtraDictionaryCollectionPrivate()
{
}

void HbExtraDictionaryCollectionPrivate::addDictionaries(QList<int>& newOnes)
{
    for (int i = 0; i < newOnes.count(); i++) {
        if (!isAlreadyInList(newOnes[i])) {
            HbExtraUserDictionary *newDict = HbExtraDictionaryFactory::instance()->createDictionary(newOnes[i]);
            if (newDict) {
                dictionaries.append(newDict);
            }
        }
    }
}

void HbExtraDictionaryCollectionPrivate::addDictionary(int newId)
{
    if (!isAlreadyInList(newId)) {
        HbExtraUserDictionary *newDict = HbExtraDictionaryFactory::instance()->createDictionary(newId);
        if (newDict) {
            dictionaries.append(newDict);
        }
    }
}

void HbExtraDictionaryCollectionPrivate::addDictionary(HbExtraUserDictionary *newOne)
{
    if (newOne && !isAlreadyInList(newOne->id())) {
        dictionaries.append(newOne);
    }
}

void HbExtraDictionaryCollectionPrivate::removeDictionary(int dictId)
{
    int index = dictionaryIndex(dictId);
    if (index >= 0) {
        dictionaries.removeAt(index);

    }
}

void HbExtraDictionaryCollectionPrivate::removeDictionary(HbExtraUserDictionary *dict)
{
    for (int i = 0; i < dictionaries.count(); i++) {
        if (dictionaries[i] == dict) {
            dictionaries.removeAt(i);
        }
    }
}

bool HbExtraDictionaryCollectionPrivate::isAlreadyInList(int dictId) const
{
    for (int i = 0; i < dictionaries.count(); i++) {
        if (dictionaries[i]->id() == dictId) {
            return true;
        }
    }

    return false;
}

int HbExtraDictionaryCollectionPrivate::dictionaryIndex(int id) const
{
    for (int i = 0; i < dictionaries.count(); i++) {
        if (dictionaries[i]->id() == id) {
            return i;
        }
    }

    return -1;
}

/// @endcond

/*!
Constructs the object.
*/
HbExtraDictionaryCollection::HbExtraDictionaryCollection() : d_ptr(new HbExtraDictionaryCollectionPrivate)
{
}

/*!
Constructs the object and attaches given list of dictionaries. Uses HbExtraDictionaryFactory for
loading dictionaries.
*/
HbExtraDictionaryCollection::HbExtraDictionaryCollection(QList<int> dictionaries) : d_ptr(new HbExtraDictionaryCollectionPrivate)
{
    Q_D(HbExtraDictionaryCollection);
    d->addDictionaries(dictionaries);
}

/*!
Destructs the object.
*/
HbExtraDictionaryCollection::~HbExtraDictionaryCollection()
{
    delete d_ptr;
}

/*!
Adds given dictionary to the collection if it is avaivale. Uses HbExtraDictionaryFactory
for loading the dictionary. Returns true if dictionary was found and succesfully added.
*/
bool HbExtraDictionaryCollection::addDictionary(int id)
{
    Q_D(HbExtraDictionaryCollection);

    if (d->dictionaries.count() < HbMaxDictionariesInCollection) {
        d->addDictionary(id);
        return true;
    }

    return false;
}

/*!
Adds given dictionary to the collection.
*/
bool HbExtraDictionaryCollection::addDictionary(HbExtraUserDictionary *dictionary)
{
    Q_D(HbExtraDictionaryCollection);

    if (d->dictionaries.count() < HbMaxDictionariesInCollection) {
        d->addDictionary(dictionary);
        return true;
    }

    return false;
}

/*!
Removes dictionary from the collection.
*/
void HbExtraDictionaryCollection::removeDictionary(int id)
{
    Q_D(HbExtraDictionaryCollection);
    d->removeDictionary(id);
}

/*!
Removes dictionary from the collection.
*/
void HbExtraDictionaryCollection::removeDictionary(HbExtraUserDictionary *dictionary)
{
    Q_D(HbExtraDictionaryCollection);
    d->removeDictionary(dictionary);
}

/*!
Returns list of all dictionaries in the collection. Also disabled dictionaries
will be included.
*/
QList<int> HbExtraDictionaryCollection::dictionaries() const
{
    Q_D(const HbExtraDictionaryCollection);

    QList<int> results;

    for (int i = 0; i < d->dictionaries.count(); i++) {
        results.append(d->dictionaries[i]->id());
    }

    return QList<int>(results);
}

/*!
Returns the total number of words in all enabled dictionaries in the collection.
*/
int HbExtraDictionaryCollection::numberOfWords() const
{
    Q_D(const HbExtraDictionaryCollection);

    int ret = 0;

    for (int i = 0; i < d->dictionaries.count(); i++) {
        if (!d->isDisabled(i)) {
            ret += d->dictionaries[i]->numberOfWords();
        }
    }

    return ret;
}

/*!
Returns the total number of words in all dictionaries in the collection (includes both enabled and disabled).
*/
int HbExtraDictionaryCollection::totalNumberOfWords() const
{
    Q_D(const HbExtraDictionaryCollection);

    int ret = 0;

    for (int i = 0; i < d->dictionaries.count(); i++) {
        ret += d->dictionaries[i]->numberOfWords();
    }

    return ret;
}


/*!
Returns all the words in the collection that begin with contents of searchString.
Search is case insensitive. Only enabled dictionaries are part of the search.
Empty string will match to all words.
*/
QStringList HbExtraDictionaryCollection::findMatches(const QString &aSearchString, Qt::CaseSensitivity caseSensitivity)
{
    Q_D(HbExtraDictionaryCollection);

    QStringList results;

    for (int i = 0; i < d->dictionaries.count(); i++) {
        if (!d->isDisabled(i)) {
            results += d->dictionaries[i]->findMatches(aSearchString, caseSensitivity);
        }
    }

    return QStringList(results);
}

/*!
Diables given dictionary, but still keeps it as part of the collection.
All access and search operators will skip disabled dictionary until it is
enabled again.

\sa enableDictionary
*/
void HbExtraDictionaryCollection::disableDictionary(int id)
{
    Q_D(HbExtraDictionaryCollection);

    int index =  d->dictionaryIndex(id);
    if (index >= 0) {
        d->disable(index);
    }
}

/*!
Enables given dictionary.

\sa disableDictionary
*/
void HbExtraDictionaryCollection::enableDictionary(int id)
{
    Q_D(HbExtraDictionaryCollection);

    int index =  d->dictionaryIndex(id);
    if (index >= 0) {
        d->enable(index);
    }
}

/*!
Returns true if given dictionary is disabled.
*/
bool HbExtraDictionaryCollection::isDisabled(int dictionaryId) const
{
    Q_D(const HbExtraDictionaryCollection);

    int index =  d->dictionaryIndex(dictionaryId);
    if (index >= 0) {
        return d->isDisabled(index);
    }

    return false;
}

/*!
Returns word at the given index in the set of enabled dictionaries. Returns empty string
if the index is out of bounds.
*/
QString HbExtraDictionaryCollection::wordAt(int index) const
{
    Q_D(const HbExtraDictionaryCollection);

    int total = 0;
    for (int i = 0; i < d->dictionaries.count(); i++) {
        if (!d->isDisabled(i)) {
            const int numwords = d->dictionaries[i]->numberOfWords();
            if (index < total + numwords) {
                return d->dictionaries[i]->wordAt(index - total);
            } else {
                total += numwords;
            }
        }
    }

    return QString();
}

/*!
Returns true if given word exists in any of the active dictionaries in the collection.
*/
bool HbExtraDictionaryCollection::hasWord(const QString &word) const
{
    Q_D(const HbExtraDictionaryCollection);

    for (int i = 0; i < d->dictionaries.count(); i++) {
        if (!d->isDisabled(i)) {
            if (d->dictionaries[i]->hasWord(word)) {
                return true;
            }
        }
    }

    return false;
}

/*!
Increments use count (frequency) for given word. Applied to all the instances of given word
in enabled part of the collection.
*/
void HbExtraDictionaryCollection::incrementUseCount(const QString &word)
{
    Q_D(const HbExtraDictionaryCollection);

    for (int i = 0; i < d->dictionaries.count(); i++) {
        if (!d->isDisabled(i)) {
            d->dictionaries[i]->incrementUseCount(word);
        }
    }
}

// End of file
