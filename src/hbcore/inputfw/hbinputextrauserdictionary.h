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

#ifndef HB_INPUT_EXTRA_USER_DICTIONARY_H
#define HB_INPUT_EXTRA_USER_DICTIONARY_H

#include <QObject>
#include <QStringList>

#include <hbinputuserdictionary.h>

/*!
\class HbExtraUDDirectoryEntry
\brief A directory entry inside extra user dictionary data area.

This class is used for internal book keeping in HbExtraUserDictionary data area.
Outside access is provided in case it is needed but typically public api is enough
for accessing extra user dictionary contents.

\sa HbExtraUserDictionary
*/
class HbExtraUDDirectoryEntry
{
public:
    unsigned short start;       /**< Index of word's first QChar in data area. */
    unsigned char length;       /**< Length of the word. */
    unsigned char frequency;    /**< Frequency information */
};

class HbExtraUserDictionaryPrivate;

const int KExtraUserDictionaryMaxWordLength = 256;
const int KExtraUserDictionaryMaxWords = 65535;

class HB_CORE_EXPORT HbExtraUserDictionary : public HbUserDictionary
{
public:
    HbExtraUserDictionary();
    HbExtraUserDictionary(int dbId);
    ~HbExtraUserDictionary();

public: // from HbUserDictionary
    int id() const;
    bool addWord(const QString& newWord, HbPredictionCallback* aCallback = 0);
    bool addWords(const QStringList& wordList);
    bool removeWord(const QString& toBeRemoved);
    int numberOfWords() const;
    QStringList listWords();
    void clear();
    HbPredictionBase* hostEngine() const;

public:
    QString wordAt(int index) const;
    QStringList findMatches(const QString& searchString, bool sortByFrequency = false, Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive);
    bool hasWord(const QString& word, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive) const;

    void setHostEngine(HbPredictionBase* host);
    bool load(const QString& nameOfTheFile = QString());
    bool save(const QString& nameOfTheFile = QString());
    void setId(int dbId);
    bool isAlreadyInMemory() const;
    bool attach();
    QString name();
    QString fileName();
    void incrementUseCount(const QString& word); 

public:
    QChar *rawDataArea() const;
    int rawDataAreaSize() const;
    HbExtraUDDirectoryEntry *directory() const;

protected:
    HbExtraUserDictionaryPrivate* const d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbExtraUserDictionary)
    Q_DISABLE_COPY(HbExtraUserDictionary)
};

#endif // HB_INPUT_EXTRA_USER_DICTIONARY_H

// End of file
