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
#include "hbinputextrauserdictionary.h"
#include "hbinputextrauserdictionary_p.h"

#include <qbytearray.h> // For memmove()

#include <QFile>
#include <QDir>
#include <QSharedMemory>
#include <QVector>

#include "hbinputsettingproxy.h"

const int HbExtraDictMaxFrequency = 255;

const QString RomanHundreds[10] = {"C","CC","CCC","CD","D","DC","DCC","DCCC","CM","M"};
const QString RomanTens[9] = {"X","XX","XXX","XL","L","LX","LXX","LXXX","XC"};
const QString RomanOnes[9] = {"I","II","III","IV","V","VI","VII","VIII","IX"};

/*!
@stable
@hbcore
\class HbExtraUserDictionary
\brief A generic implementation of HbUserDictionary class.

This class provides generic all-purpose implementation of HbUserDictionary class.
It uses shared memory and words are stored in plain text format. Entries are sorted and
a binary search algorithm is provided for finding matches. There is a random access
operator for read operations. It also knows how to save its contents to disk and load it again.
There is a separate factory class for creating and accessing HbExtraUserDictionary
instances.

The dictionary data is organized so that there is a directory area and data area. Directory area
contains an array of HbExtraUDDirectoryEntry items. They specify where each word begins in the
data area and how long it is. Data area contains characters in a single long string. There are
methods for accessing directory and data area in case direct access is needed.
Typically this is not needed and default search and access operators are enough.

Example uses cases for extra dictonaries are additional prediction engine databases and
acting as a backend for auto-completing fields.

\sa HbUserDictionary
\sa HbExtraDictionaryFactory
*/

/// @cond

bool HbExtraUserDictionaryPrivate::createSharedBlock(int aSize)
{
    if (sharedMemory.isAttached()) {
        return true;
    }

    if (id == 0) {
        return false;
    }

    sharedMemory.setKey(name());

    if (!sharedMemory.attach()) {
        if (sharedMemory.error() != QSharedMemory::NotFound) {
            qDebug("HbExtraUserDictionaryPrivate: QSharedMemory::attached returned error %d", sharedMemory.error());
            return false;
        }

        if (!sharedMemory.create(aSize)) {
            qDebug("HbExtraUserDictionaryPrivate: Unable to create shared memory block!");
            return false;
        }

        dataHeader()->numWords = 0;
        dataHeader()->numUsers = 0;
        dataHeader()->modified = false;
        dataHeader()->dataSize = 0;
    }

    dataHeader()->numUsers++;
    return true;
}

QString HbExtraUserDictionaryPrivate::name() const
{
    
    QString num(convertToRomanNumerals(id));
    return QString(KExtraUserDictKeyBase) + num;
}

QString HbExtraUserDictionaryPrivate::fileName() const
{
    return HbInputSettingProxy::extraDictionaryPath() + QDir::separator() + name() + QString(KExtraFileExt);
}

QString HbExtraUserDictionaryPrivate::convertToRomanNumerals(int id) const
{
    int numId = id; 
    QString retNum;
    
    // Append Roman Thousand's to the string
    int index = 10;
    int count = numId/1000;
    for (int i = 0; i < count; i++) {
        retNum.append(RomanHundreds[index-1]);
    }

    // Append Roman Hundred's to the string
    numId = numId % 1000;
    index = numId / 100;
    if (index) {
        retNum.append(RomanHundreds[index - 1]);
    }

    // Append Roman Ten's to the string
    numId = numId % 100;
    index = numId / 10;
    if (index) {
        retNum.append(RomanTens[index - 1]);
    }
    // Append Roman single digit numerals to the string
    numId = numId % 10;
    index = numId / 1;
    if (index) {
        retNum.append(RomanOnes[index - 1]);
    }
    return retNum;
}

bool HbExtraUserDictionaryPrivate::save(QString fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write((char *)&id, sizeof(int));
        if (sharedMemory.isAttached()) {
            file.write((char *)sharedMemory.data(), KExtraUDBlockSize);
        }
        file.close();
        dataHeader()->modified = false;
        return true;
    }

    return false;
}

int HbExtraUserDictionaryPrivate::findFirstMatch(int start, int end, const QString &searchString, int knownMatch, Qt::CaseSensitivity caseSensitivity) const
{
    HbExtraUDDirectoryEntry *dir = directory();
    QChar *data = dataArea();

    if (start >= end) {
        if (QString(&data[dir[start].start], dir[start].length).startsWith(searchString, Qt::CaseInsensitive)) {
            return start;
        }
        return knownMatch;
    }

    int half = (start + end) / 2;
    if (QString(&data[dir[half].start], dir[half].length).startsWith(searchString, Qt::CaseInsensitive)) {
        knownMatch = half;
        return findFirstMatch(start, half, searchString, knownMatch, caseSensitivity);
    }

    if (compareWords(half, searchString, caseSensitivity) > 0) {
        return findFirstMatch(half + 1, end, searchString, knownMatch, caseSensitivity);
    }

    return findFirstMatch(start, half, searchString, knownMatch, caseSensitivity);
}

void HbExtraUserDictionaryPrivate::removeEntry(int index)
{
    HbExtraUDDirectoryEntry *dir = directory();
    QChar *data = dataArea();

    int start = dir[index].start;
    int length = dir[index].length;

    // First move trailing directory entries and the beginning of the data area
    // by size of one dictionary entry item.
    memmove((char *)&dir[index], (char *)&dir[index + 1], (dataHeader()->numWords - index) * sizeof(HbExtraUDDirectoryEntry) + start * sizeof(QChar));

    // Then move trailing part of the data area.
    memmove(((char *)&data[start]) - sizeof(HbExtraUDDirectoryEntry), (char *)&data[start + length], (dataHeader()->dataSize - (start + length)) * 2);

    // Update word count.
    dataHeader()->numWords--;
    dataHeader()->dataSize -= length;
    dataHeader()->modified = true;

    // Then update remaining dictionary entries.
    const int rounds = dataHeader()->numWords;
    for (int i = index; i < rounds; i++) {
        dir[i].start -= length;
    }
}

void HbExtraUserDictionaryPrivate::addEntry(int index, const QString &newWord)
{
    HbExtraUDDirectoryEntry *dir = directory();
    QChar *data = dataArea();

    const int origNumWords = dataHeader()->numWords;
    if (origNumWords > 0) {
        if (index < origNumWords) {
            // First move the trailing part of the data area to make space for the new word.
            memmove((char *)&data[dir[index].start + newWord.size()] + sizeof(HbExtraUDDirectoryEntry),
                    (char *)(&data[dir[index].start]),
                    (dataHeader()->dataSize - dir[index].start) * 2);

            // Then move the trailing part of the dictionary and the leading part of the data area.
            memmove((char *)(&dir[index + 1]),
                    (char *)(&dir[index]),
                    ((dataHeader()->numWords - index) * sizeof(HbExtraUDDirectoryEntry)) + (dir[index].start * 2));
        } else {
            // This will be the last one. Just make room for new directory entry.
            memmove((char *)data + sizeof(HbExtraUDDirectoryEntry),
                    (char *)data,
                    dataHeader()->dataSize * 2);
        }
    }

    // Update word count.
    dataHeader()->numWords++;
    dataHeader()->dataSize += newWord.size();
    dataHeader()->modified = true;

    data = dataArea();  // data area starting point has changed, refresh.

    if (index == 0) {
        dir[index].start = 0;
    } else {
        dir[index].start = dir[index - 1].start + dir[index - 1].length;
    }
    dir[index].length = newWord.size();
    dir[index].frequency = 0;

    const int startPoint = dir[index].start;
    for (int i = 0; i < newWord.size(); i++) {
        data[startPoint + i] = newWord[i];
    }

    // Then update remaining dictionary entries.
    const int rounds = dataHeader()->numWords;
    for (int i = index + 1; i < rounds; i++) {
        dir[i].start += newWord.size();
    }
}

int HbExtraUserDictionaryPrivate::findWord(int startIndex, int endIndex, const QString &newWord, Qt::CaseSensitivity caseSensitivity) const
{
    if (startIndex >= endIndex) {
        if (startIndex < dataHeader()->numWords && compareWords(startIndex, newWord, caseSensitivity) == 0) {
            return startIndex;
        }

        return -1;   // No matches
    }

    int half = (startIndex + endIndex) / 2;
    int compare = compareWords(half, newWord, caseSensitivity);
    if (compare == 0) {
        return half;
    } else if (compare > 0) {
        return findWord(half + 1, endIndex, newWord, caseSensitivity);
    } else {
        return findWord(startIndex, half, newWord, caseSensitivity);
    }
}

int HbExtraUserDictionaryPrivate::findIndexForNewWord(int start, int end, const QString &newWord) const
{
    if (start >= end) {
        if (dataHeader()->numWords == 0) {
            return 0;
        }

        if (compareWords(start, newWord) > 0) {
            return start + 1;
        } else {
            return start;
        }
    }

    int half = (start + end) / 2;
    int compare = compareWords(half, newWord);
    if (compare == 0) {
        return half;
    } else if (compare > 0) {
        return findIndexForNewWord(half + 1, end, newWord);
    } else {
        return findIndexForNewWord(start, half, newWord);
    }
}

int HbExtraUserDictionaryPrivate::compareWords(int index, const QString &otherWord, Qt::CaseSensitivity caseSensitivity) const
{
    HbExtraUDDirectoryEntry *dir = directory();
    QChar *data = dataArea();

    const int start = dir[index].start;
    const int rounds = (dir[index].length > otherWord.size() ? otherWord.size() : dir[index].length);
    if (caseSensitivity == Qt::CaseSensitive) {
        for (int i = 0; i < rounds; i++) {
            if (data[start + i] == otherWord[i]) {
                continue;
            }

            if (otherWord[i] > data[start + i]) {
                return 1;
            }

            return -1;
        }
    } else {
        for (int i = 0; i < rounds; i++) {
            if (data[start + i].toCaseFolded() == otherWord[i].toCaseFolded()) {
                continue;
            }

            if (otherWord[i].toCaseFolded() > data[start + i].toCaseFolded()) {
                return 1;
            }

            return -1;
        }
    }

    if (dir[index].length == otherWord.size()) {
        return 0;  // Match!
    }

    if (otherWord.size() > dir[index].length) {
        return 1;
    }

    return -1;
}

bool HbExtraUserDictionaryPrivate::hasEnoughSpaceForNewWord(const QString &newWord) const
{
    if ((unsigned int)dataAreaSize() - (dataHeader()->dataSize * 2) >= (newWord.size() * 2) + sizeof(HbExtraUDDirectoryEntry)) {
        return true;
    }

    return false;
}

/// @endcond

/*!
Constructs the object.
*/
HbExtraUserDictionary::HbExtraUserDictionary() : d_ptr(new HbExtraUserDictionaryPrivate)
{
}

/*!
Constructs the object. This version tries to attachs to given user dictionary and
if not found, initializes it.
*/
HbExtraUserDictionary::HbExtraUserDictionary(int dbId) : d_ptr(new HbExtraUserDictionaryPrivate)
{
    setId(dbId);
    attach();
}

/*!
Destructs the object.
*/
HbExtraUserDictionary::~HbExtraUserDictionary()
{
    Q_D(HbExtraUserDictionary);

    d->lock();
    if (d->sharedMemory.isAttached()) {
        if (d->dataHeader()->numUsers <= 1 && d->dataHeader()->modified) {
            d->save(fileName());
        }
        d->dataHeader()->numUsers--;
    }
    d->unlock();

    delete d;
}

/*!
Returns user dictionary id value.
*/
int HbExtraUserDictionary::id() const
{
    Q_D(const HbExtraUserDictionary);
    return d->id;
}

/*!
Adds single word to the database. Returns true if there was enough space for new
word and the word was successfully added. Returns false if word already
exists.

\sa addWords
\sa removeWord
*/
bool HbExtraUserDictionary::addWord(const QString &newWord, HbPredictionCallback *callback)
{
    Q_UNUSED(callback);
    Q_D(HbExtraUserDictionary);

    d->lock();

    if (d->findWord(0, d->dataHeader()->numWords - 1, newWord) >= 0) {
        // Already there.
        d->unlock();
        return false;
    }

    if (newWord.size() < KExtraUserDictionaryMaxWordLength &&
        d->hasEnoughSpaceForNewWord(newWord) &&
        d->dataHeader()->numWords < KExtraUserDictionaryMaxWords) {
        int newIndex = d->findIndexForNewWord(0, d->dataHeader()->numWords - 1, newWord);
        d->addEntry(newIndex, newWord);

        d->unlock();
        return true;
    }

    d->unlock();
    return false;
}

/*!
Adds a list of words to the database. Returns true if all the words were
successfully added to the database.

\sa addWord
\sa removeWord
*/
bool HbExtraUserDictionary::addWords(const QStringList &wordList)
{
    bool ret = true;

    for (int i = 0; i < wordList.count(); i++) {
        ret &= addWord(wordList[i]);
    }

    return ret;
}

/*!
Removes a word from the database. Returns true if the word was found and removed.

\sa addWord
\sa addWords
*/
bool HbExtraUserDictionary::removeWord(const QString &toBeRemoved)
{
    Q_D(HbExtraUserDictionary);

    d->lock();

    int index = d->findWord(0, d->dataHeader()->numWords - 1, toBeRemoved, Qt::CaseInsensitive);
    if (index >= 0) {
        d->removeEntry(index);
        d->unlock();
        return true;
    }

    d->unlock();
    return false;
}

/*!
Returns the number of words in the dictionary.
*/
int HbExtraUserDictionary::numberOfWords() const
{
    Q_D(const HbExtraUserDictionary);

    int ret = 0;

    if (d->dataHeader()) {
        ret = d->dataHeader()->numWords;
    }

    return ret;
}

/*!
Returns a list of strings containing all the words currently stored in the database.
*/
QStringList HbExtraUserDictionary::listWords()
{
    Q_D(HbExtraUserDictionary);

    d->lock();

    QStringList result;

    if (d->sharedMemory.isAttached()) {
        HbExtraUDDirectoryEntry *dir = d->directory();
        QChar *data = d->dataArea();

        for (int i = 0; i < d->dataHeader()->numWords; i++) {
            result << QString(&data[dir[i].start], dir[i].length);
        }
    }
    d->unlock();

    return QStringList(result);
}

/*!
Removes all the words from the dictionary. Memory block will remain active in the memory.
*/
void HbExtraUserDictionary::clear()
{
    Q_D(HbExtraUserDictionary);

    d->lock();

    HbExtraUDHeader *header = d->dataHeader();

    if (d->sharedMemory.isAttached()) {
        if (header->numWords) {
            header->modified = true;
        }
        header->dataSize = 0;
        header->numWords = 0;
    }

    d->unlock();
}

/*!
Returns all the dictionary words that begin with contents of searchString.
Search is case insensitive. Empty string will match to all words.
*/
QStringList HbExtraUserDictionary::findMatches(const QString &searchString, bool sortByFrequency, Qt::CaseSensitivity caseSensitivity)
{
    Q_UNUSED(caseSensitivity);  // Will be taken into use...
    Q_D(HbExtraUserDictionary);

    QVector<unsigned char> frequencies;
    QStringList results;

    d->lock();

    if (d->dataHeader()->numWords) {
        HbExtraUDDirectoryEntry *dir = d->directory();
        QChar *data = d->dataArea();

        int first = d->findFirstMatch(0, d->dataHeader()->numWords - 1, searchString);
        if (first >= 0) {
            results.append(QString(&data[dir[first].start], dir[first].length));
            frequencies.append(dir[first].frequency);

            const int rounds = d->dataHeader()->numWords;
            for (int i = first + 1; i < rounds; i++) {
                QString candidate(&data[dir[i].start], dir[i].length);
                if (candidate.startsWith(searchString, Qt::CaseInsensitive)) {
                    if (sortByFrequency) {
                        const unsigned char freq = dir[i].frequency;
                        int j = 0;
                        for (; j < frequencies.count(); j++) {
                            if (freq >= frequencies[j]) {
                                frequencies.insert(j, freq);
                                results.insert(j, candidate);
                                break;
                            }
                        }
                        if (j >= frequencies.count()) {
                            frequencies.append(freq);
                            results.append(candidate);
                        }
                    } else {
                        results.append(candidate);
                    }
                } else {
                    break;
                }
            }
        }
    }

    d->unlock();

    return QStringList(results);
}

/*!
Returns pointer to host prediction engine.
*/
HbPredictionBase *HbExtraUserDictionary::hostEngine() const
{
    Q_D(const HbExtraUserDictionary);
    return d->hostEngine;
}

/*!
Sets host prediction engine.
*/
void HbExtraUserDictionary::setHostEngine(HbPredictionBase *host)
{
    Q_D(HbExtraUserDictionary);
    d->hostEngine = host;
}

/*!
Loads dictionary from disk.
*/
bool HbExtraUserDictionary::load(const QString &nameOfTheFile)
{
    Q_D(HbExtraUserDictionary);

    QString realFileName = nameOfTheFile;
    if (realFileName.size() == 0) {
        realFileName = fileName();
    }

    if (attach()) {
        d->lock();
        QFile file(realFileName);
        if (file.open(QIODevice::ReadOnly)) {
            int numUsers = d->dataHeader()->numUsers;
            file.read((char *)&d->id, sizeof(int));
            file.read((char *)d->sharedMemory.data(), KExtraUDBlockSize);
            file.close();
            d->dataHeader()->numUsers = numUsers;
            d->unlock();
            return true;
        }
        d->unlock();
    }

    return false;
}

/*!
Saves dictionary to disk. If no file name is give, fileName() will
be used.

\sa fileName
*/
bool HbExtraUserDictionary::save(const QString &nameOfTheFile)
{
    Q_D(HbExtraUserDictionary);

    QString realFileName = nameOfTheFile;
    if (realFileName.size() == 0) {
        realFileName = fileName();
    }

    bool ret = false;
    d->lock();
    ret = d->save(realFileName);
    d->unlock();

    return ret;
}

/*!
Sets dictionary id.
*/
void HbExtraUserDictionary::setId(int aId)
{
    Q_D(HbExtraUserDictionary);
    d->id = aId;
}

/*!
Returns true if dictionary is already active in memory.
*/
bool HbExtraUserDictionary::isAlreadyInMemory() const
{
    Q_D(const HbExtraUserDictionary);

    if (d->sharedMemory.isAttached()) {
        return true;
    }

    QSharedMemory tmpMemory;
    tmpMemory.setKey(d->name());

    if (tmpMemory.attach()) {
        return true;
    }

    return false;
}

/*!
Returns the name of the dictionary without path.
*/
QString HbExtraUserDictionary::name()
{
    Q_D(HbExtraUserDictionary);
    return d->name();
}

/*!
Returns the name of the dictionary with path included.
*/
QString HbExtraUserDictionary::fileName()
{
    Q_D(HbExtraUserDictionary);
    return d->fileName();
}

/*!
Returns true if there was an instance already in memory and attach was successful. Otherwise
this was the first instance for this id and empty memory block was allocated for it. Id value
must be set, otherwise returns false.
*/
bool HbExtraUserDictionary::attach()
{
    Q_D(HbExtraUserDictionary);
    return d->createSharedBlock(KExtraUDBlockSize);
}

/*!
Returns pointer to raw data area. Words are stored to data area as a single string of characters.
Directory defines where each words begins and ends. Words in the data area are sorted by unicode values.
This method is provided for sake of efficiency for those who need direct access and know what they are doing.

\sa rawDataAreaSize
\sa directory
*/
QChar *HbExtraUserDictionary::rawDataArea() const
{
    Q_D(const HbExtraUserDictionary);
    return d->dataArea();
}

/*!
Returs the maximum size of raw data area. This size will decrease every time new words are
added to the dictionary, because directory size will increase. This method is provided for sake
of efficiency for those who need direct access and know what they are doing.

\sa rawDataArea
*/
int HbExtraUserDictionary::rawDataAreaSize() const
{
    Q_D(const HbExtraUserDictionary);
    return d->dataAreaSize();
}

/*!
Returns direct access to internal directory in the data area. The array contains as many items as there are
words in the dictionary. This method is provided for sake of efficiency for those who need direct access
and know what they are doing.

\sa rawDataArea
*/
HbExtraUDDirectoryEntry *HbExtraUserDictionary::directory() const
{
    Q_D(const HbExtraUserDictionary);
    return d->directory();
}

/*!
The random access operator. Returns word at given index. Returns empty string
if the index is invalid.
*/
QString HbExtraUserDictionary::wordAt(int index) const
{
    Q_D(const HbExtraUserDictionary);

    if (index < d->dataHeader()->numWords) {
        HbExtraUDDirectoryEntry *dir = d->directory();
        QChar *data = d->dataArea();

        return QString(&data[dir[index].start], dir[index].length);
    }

    return QString();
}

/*!
Increases word frequency counter for given word if it is in the dictionary.
*/
void HbExtraUserDictionary::incrementUseCount(const QString &word)
{
    Q_D(const HbExtraUserDictionary);

    if (d->dataHeader()->numWords) {
        HbExtraUDDirectoryEntry *dir = d->directory();

        int first = d->findFirstMatch(0, d->dataHeader()->numWords - 1, word);
        if (first >= 0 && dir[first].frequency < HbExtraDictMaxFrequency) {
            dir[first].frequency++;
            d->dataHeader()->modified = true;
        }
    }
}

/*!
Returns true if given word exits in the dictionary.
*/
bool HbExtraUserDictionary::hasWord(const QString &word, Qt::CaseSensitivity caseSensitivity) const
{
    Q_D(const HbExtraUserDictionary);

    if (d->dataHeader()->numWords) {
        QChar *data = d->dataArea();
        HbExtraUDDirectoryEntry *dir = d->directory();
        int first = d->findFirstMatch(0, d->dataHeader()->numWords - 1, word, -1, caseSensitivity);
        if (first >= 0) {
            if (caseSensitivity == Qt::CaseSensitive) {
                if (QString(&data[dir[first].start], dir[first].length) == word) {
                    return true;
                }

                const int rounds = d->dataHeader()->numWords;
                for (int i = first + 1; i <= rounds; i++) {
                    QString candidate(&data[dir[i].start], dir[i].length);
                    if (candidate.startsWith(word, Qt::CaseInsensitive)) {
                        if (candidate == word) {
                            return true;
                        }
                    } else {
                        break;
                    }
                }
            } else {
                if (QString(&data[dir[first].start], dir[first].length).toCaseFolded() == word.toCaseFolded()) {
                    return true;
                }
                const int rounds = d->dataHeader()->numWords;
                for (int i = first + 1; i <= rounds; i++) {
                    QString candidate(&data[dir[i].start], dir[i].length);
                    if (candidate.startsWith(word, Qt::CaseInsensitive)) {
                        if (candidate.toCaseFolded() == word.toCaseFolded()) {
                            return true;
                        }
                    } else {
                        break;
                    }
                }
            }
        }
    }

    return false;
}

// End of file
