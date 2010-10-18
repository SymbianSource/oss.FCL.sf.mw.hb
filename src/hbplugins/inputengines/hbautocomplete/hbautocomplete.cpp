/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbPlugins module of the UI Extensions for Mobile.
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
#include "hbautocomplete.h"
#include <hbinputextrauserdictionary.h>
#include <hbinputextradictionarycollection.h>
#include <hbinputextradictionaryfactory.h>

/// @cond

class HbAutoCompletePrivate
{
public:
    HbAutoCompletePrivate() 
         : mActiveDictionary(0),
           mActiveCollection(0),
           mCandidatesOutdated(false)
    {}

public:
    HbExtraUserDictionary *mActiveDictionary;
    HbExtraDictionaryCollection *mActiveCollection;
    QString mCurrentWord;
    QStringList mCandidates;
    bool mCandidatesOutdated;
    QList<HbInputLanguage> mLanguages;
};

/// @endcond

const QString HbAutoCompleteEngineVersion("0.1");

/*!
Constructs the object.
*/
HbAutoComplete::HbAutoComplete() : d_ptr(new HbAutoCompletePrivate) 
{
    Q_D(HbAutoComplete);

    d->mLanguages.append(HbInputLanguage());
}

/*!
Destructs the object.
*/ 
HbAutoComplete::~HbAutoComplete()
{
    delete d_ptr;
}

/*!
\reimp
*/
QList<HbInputLanguage> HbAutoComplete::languages() const
{
    Q_D(const HbAutoComplete);
    return QList<HbInputLanguage>(d->mLanguages);
}

/*!
\reimp
*/
void HbAutoComplete::deleteKeyPress(HbPredictionCallback* callback)
{
    Q_UNUSED(callback)
    Q_D(HbAutoComplete);

    d->mCurrentWord.chop(1);
    d->mCandidatesOutdated = true;
}

/*!
\reimp
*/
void HbAutoComplete::commit(const QString &word)
{
    Q_UNUSED(word);
    Q_D(HbAutoComplete);

    if (d->mCurrentWord.size()) {
        addUsedWord(d->mCurrentWord);
    }

    clear();
}

/*!
\reimp
*/
void HbAutoComplete::clear()
{
    Q_D(HbAutoComplete);

    d->mCurrentWord.clear();
    d->mCandidates.clear();
}

/*!
\reimp
*/
void HbAutoComplete::addUsedWord(const QString& word)
{
    Q_D(HbAutoComplete);

    if (word.size() > 0) {
        if (d->mActiveDictionary) {
            d->mActiveDictionary->addWord(word);
            d->mActiveDictionary->incrementUseCount(word);
        } else if (d->mActiveCollection) {
            if (!d->mActiveCollection->hasWord(word)) {
                QList<int> dictionaries = d->mActiveCollection->dictionaries();
                for (int i = 0; i < dictionaries.count(); i++) {
                    if (!d->mActiveCollection->isDisabled(dictionaries[i])) {
                        HbExtraUserDictionary *firstOne = HbExtraDictionaryFactory::instance()->existingDictionary(dictionaries.at(i));
                        if (firstOne) {
                            firstOne->addWord(word);
                        }
                    }
                }
            } else {
                d->mActiveCollection->incrementUseCount(word);
            }
        }
    } 
}

/*!
\reimp
*/
void HbAutoComplete::appendKeyPress(const int keycode, const Qt::KeyboardModifiers modifiers, const HbTextCase textCase, HbPredictionCallback* callback)
{
    Q_UNUSED(keycode)
    Q_UNUSED(modifiers)
    Q_UNUSED(textCase)
    Q_UNUSED(callback)
}

/*!
\reimp
*/
void HbAutoComplete::setWord(const QString& word, HbPredictionCallback* callback)
{
    Q_UNUSED(callback)
    Q_D(HbAutoComplete);

    if (word.size() < KExtraUserDictionaryMaxWordLength) {
        d->mCurrentWord = word;
        d->mCandidatesOutdated = true;
    }
}

/*!
\reimp
*/
void HbAutoComplete::updateCandidates(int& bestGuessLocation, bool& noMoreCandidates)
{
    Q_D(HbAutoComplete);

    bestGuessLocation = 0;
    noMoreCandidates = false;
 
    if (d->mCandidatesOutdated) {
        if (d->mActiveDictionary && d->mCurrentWord.size() > 0) {
            d->mCandidates = d->mActiveDictionary->findMatches(d->mCurrentWord, true); 
        } else if (d->mActiveCollection &&  d->mCurrentWord.size() > 0) {
            d->mCandidates = d->mActiveCollection->findMatches(d->mCurrentWord); 
        }
        d->mCandidatesOutdated = false;
    }
}

/*!
\reimp
*/
bool HbAutoComplete::updateCandidates(int& bestGuessLocation)
{
    // The real implementation will be moved into this method after the deprecation period.
    bool tmp = false;
    updateCandidates(bestGuessLocation, tmp);
    return tmp;
}

/*!
\reimp
*/
HbInputPredictionFeature HbAutoComplete::features() const
{
    return (HbInputPredictionFeature)(HbPredFeatureExtraDictionaries | HbPredFeatureWordCompletion);
}

/*!
\reimp
*/
QString HbAutoComplete::vendorIdString() const
{
    return HbAutoCompleteVendorIdString;    
}

/*!
\reimp
*/
QString HbAutoComplete::engineVersion() const
{
    return HbAutoCompleteEngineVersion;
}

/*!
\reimp
*/
bool HbAutoComplete::supportsKeyboardType(const HbInputLanguage &language, HbKeyboardType keyboard) const
{
    Q_UNUSED(language)
    Q_UNUSED(keyboard)

    // Keyboard doesn't matter, all the communication is done via
    // appendKeyPressUnicode for now.
    return true;
}

/*!
\reimp
*/
void HbAutoComplete::setExtraUserDictionary(int aId)
{
    Q_D(HbAutoComplete);

   // Deactivate possible collection.
    delete d->mActiveCollection;
    d->mActiveCollection = 0;

    d->mActiveDictionary = HbExtraDictionaryFactory::instance()->createDictionary(aId);
    if (d->mActiveDictionary) {
        d->mActiveDictionary->setHostEngine(this);
    }
    clear();
}

/*!
\reimp
*/
void HbAutoComplete::setExtraUserDictionaries(const QList<int>& idList)
{
    Q_UNUSED(idList)
    Q_D(HbAutoComplete);

    d->mActiveDictionary = 0;  // Not owned, so doesn't need to be deleted.
    delete d->mActiveCollection;

    d->mActiveCollection = new HbExtraDictionaryCollection(idList);
    clear();
}

/*!
\reimp
*/
int HbAutoComplete::inputLength()
{
    Q_D(HbAutoComplete);
 
    return d->mCurrentWord.size();
}

/*!
\reimp
*/
void HbAutoComplete::setCandidateList(QStringList* candidateList)
{
    Q_UNUSED(candidateList)
}

/*!
\reimp
*/
QStringList HbAutoComplete::candidateList()
{
    Q_D(HbAutoComplete);

    if (d->mCandidatesOutdated) {
        if (d->mActiveDictionary && d->mCurrentWord.size() > 0) {
            d->mCandidates = d->mActiveDictionary->findMatches(d->mCurrentWord, true); 
        } else if (d->mActiveCollection && d->mCurrentWord.size() > 0) {
            d->mCandidates = d->mActiveCollection->findMatches(d->mCurrentWord);
        } else {
            d->mCandidates.clear();
        }
        d->mCandidatesOutdated = false;
    }

    return QStringList(d->mCandidates);
}

/*!
\reimp
*/
QStringList HbAutoComplete::candidates()
{
    // Move the implementation here after the deprecation period.
    return candidateList();
}

/*!
\reimp
*/
void HbAutoComplete::setKeyboard(HbKeyboardType aKeyboardType)
{
    Q_UNUSED(aKeyboardType)
}

/*!
\reimp
*/
void HbAutoComplete::appendCharacter(const QChar aChar, const HbTextCase textCase, HbPredictionCallback* callback)
{
    Q_UNUSED(callback)
    Q_UNUSED(textCase)
    Q_D(HbAutoComplete);

    if (d->mCurrentWord.size() < KExtraUserDictionaryMaxWordLength) {
        d->mCurrentWord.append(aChar);
        d->mCandidatesOutdated = true;
    }
}

// End of file 

