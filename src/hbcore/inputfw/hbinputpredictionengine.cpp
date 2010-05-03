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

#include "hbinputpredictionengine.h"

/*!
@alpha
@hbcore
\class HbPredictionBase
\brief A base class for prediction engine abstration layer.

This API defines abstract interface for predictive text input engines.
The base class is extended by language specific versions.

\sa HbPredictionEngine
\sa HbPredictionEngineChinese
\sa HbPredictionCallback
*/

/*!
\fn virtual QList<HbInputLanguage> HbPredictionBase::languages() const
Returns a list of languages the engine supports.
*/

/*!
\fn virtual void HbPredictionBase::setWord(const QString& word, HbPredictionCallback* callback = 0)
Sets current word. Given word will be the active word.  
*/

/*!
\deprecated HbPredictionBase::updateCandidates(int&, bool&)
    is deprecated. Use bool updateCandidates(int&) instead.
\fn virtual void HbPredictionBase::updateCandidates(int& bestGuessLocation, bool& noMoreCandidates)
*/

/*!
\fn bool HbPredictionBase::updateCandidates(int& bestGuessLocation)
Updates the candidate list to match active input sequence. Typically appendKeyPress and deleteKeyPress
do that automatically but in same cases it may be required to update the situation explicitly.
Paramter bestGuessLocation specifies the best match index in the candidate list. Typically it is
the first location but with some engines it may vary.
Returns true if active input sequence produces valid candidates. Otherwise returns false and
doesn not update the existing candidate list (in other words HbPredictionEngine::candidates() will
still return same set of candidates as it did before calling this method unsuccesfully).
*/

/*!
\fn virtual void HbPredictionBase::appendKeyPress(const int keycode, const Qt::KeyboardModifiers modifiers, const HbTextCase textCase = HbTextCaseNone, HbPredictionCallback* callback = 0)
Handles a key press event.

\sa appendCharacter
*/ 

/*!
\fn virtual void HbPredictionBase::deleteKeyPress(HbPredictionCallback* callback = 0)
Deletes one key press from the active input sequence.
*/

/*!
\fn virtual void HbPredictionBase::commit(const QString &word = QString())
Commits ongoing input operation and clears the active word. If the word is given as a parameter,
the result is same as after calling setWord(word) and commit().
*/

/*!
\fn virtual void HbPredictionBase::clear()
Clears the active word without commiting it.
*/

/*!
\fn virtual void HbPredictionBase::addUsedWord(const QString& word)
Notifies the engine that a given word was used as part of the text. Commit
will do this automatically.
*/

/*!
\fn virtual HbInputPredictionFeature HbPredictionBase::features() const
Returns a bit vector of supported prediction features.

\sa HbInputPredictionFeature
*/

/*!
\fn virtual QString HbPredictionBase::vendorIdString() const
Returns vendor sepcific id string. The format of the returned string depends
on the engine.
*/

/*!
\fn virtual QString HbPredictionBase::engineVersion() const
Returns engine version. The format of the returned string depends on the
engine.
*/

/*!
\fn virtual bool HbPredictionBase::supportsKeyboardType(const HbInputLanguage &language, HbKeyboardType keyboard)
Returns true if the engine supports given combination of language and keyboard type.
*/

HbPredictionBase::HbPredictionBase()
{
}

HbPredictionBase::~HbPredictionBase()
{
}

/*!
Actives single extra user dictionary. Usable only if the engine supports extra user dictionaries
(see HbPredFeatureExtraDictionaries feature flag). The default implementation is empty,
orverride if the engine supports extra dictioanries.

\sa setExtraUserDictionaries
\sa HbExtraUserDictionary
*/
void HbPredictionBase::setExtraUserDictionary(int id)
{
    Q_UNUSED(id)
}

/*!
Activates a collection of extra user dictionaries. Usable only if the engine supports extra user dictionaries
(see HbPredFeatureExtraDictionaries feature flag). The default implementation is empty,
orverride if the engine supports extra dictioanries.

\sa setExtraUserDictionary
\sa HbExtraUserDictionary
*/
void HbPredictionBase::setExtraUserDictionaries(const QList<int>& idList)
{
    Q_UNUSED(idList)
}

/*!
Returns pointer to active user dictionary. There are two kind of user dictionaries, default and
extra. This one returns the default user dictionary. Default implementation of this method is empty, 
override if the engine supports user dictionaries.

\sa setExtraUserDictionary
\sa HbExtraUserDictionary
*/
HbUserDictionary* HbPredictionBase::userDictionary() const
{
    return 0;
}

/*!
Sets active prediction language. Returns true if given language is recognized
by the engine and was succesfully activated. 
*/
bool HbPredictionBase::setLanguage(const HbInputLanguage &language, HbInputModeType inputMode)
{
    Q_UNUSED(language); 
    Q_UNUSED(inputMode);

    return false;
}

/*!
Returns active prediction language.  
*/
HbInputLanguage HbPredictionBase::language() const
{
    return HbInputLanguage();
}

/*!
Returns the keyboard type engine is using.
*/
HbKeyboardType HbPredictionBase::keyboardType() const
{
    return HbKeyboardNone;
}

/*!
\class HbPredictionEngine
\brief Prediction API for latin based langauges.

This interface defines abstract prediction API for latin-based languages. 
Also some non-latin languages whose prediction features are functionally similar to
latin-based languages use this API. Those are for example Arabic and Hebrew.
*/


/*!
\deprecated HbPredictionEngine::setCandidateList(QStringList*)
     is deprecated.
*/

/*!
\deprecated HbPredictionEngine::candidateList()
    is deprecated. use QStringList candidates() instead.
\fn virtual QStringList HbPredictionEngine::candidateList()
Returns the list of prediction candidates for current input sequence.
*/

/*!
\fn virtual QStringList HbPredictionEngine::candidates()
Returns the list of prediction candidates for current input sequence.
*/

/*
\fn virtual int HbPredictionEngine::inputLength()
Returns the number of key presses since previous clear or commit operation.
Candidate length may exceed this value if word completion is used.
*/

/*!
\fn virtual void HbPredictionEngine::setKeyboard(HbKeyboardType aKeyboardType)
Sets the keyboard type.
*/

/*!
\fn virtual void appendCharacter(QChar aChar, const HbTextCase textCase = HbTextCaseNone, HbPredictionCallback* callback = 0)
Appends a unicode character. The difference between this method and appendKeyPress is that the
engine doesn't resolve character from key mappings but uses given character directly.

\sa appendKeyPress
*/

/*!
Sets error correction level. Returns true if the given error correction level
is supported and was successfully activated. This method works only if the engine
supports error correction.

\sa errorCorrectionLevel
\sa supportedFeatures
*/
bool HbPredictionEngine::setErrorCorrectionLevel(HbErrorCorrectionLevel level)
{
    if (level == HbErrorCorrectionLevelNone) {
        return true;
    }

    return false;
}

/*!
Returns active error correction level.
*/
HbPredictionEngine::HbErrorCorrectionLevel HbPredictionEngine::errorCorrectionLevel() const
{
    return HbErrorCorrectionLevelNone;
}

/*!
Sets active secondary language for prediction engine. Returns true if given language is recognized
by the engine and was succesfully activated. 
*/
bool HbPredictionEngine::setSecondaryLanguage(const HbInputLanguage& language)
{
    Q_UNUSED(language);
    return false;
}

/*!
Returns active secondary prediction language.  
*/
HbInputLanguage HbPredictionEngine::secondaryLanguage() const
{
    return HbInputLanguage(QLocale::Language(0), QLocale::AnyCountry);
}

/*!
Returns the topmost candidate for active input sequence.

\sa candidateList
*/
QString HbPredictionEngine::currentWord() const
{
    return QString("");
}

/*!
Enables a specified feature for the prediction engine. 
Returns true if the engine supports the feature and is able to enable the feature,
otherwise, returns false.

\sa HbPredictionEngine::isFeatureEnabled()
\sa HbPredictionEngine::disableFeature()
*/
bool HbPredictionEngine::enableFeature(HbInputPredictionFeature feature)
{
    Q_UNUSED(feature);
    return false;
}

/*!
Returns true if the engine supports the feature and is enabled,
otherwise, returns false.

\sa HbPredictionEngine::enableFeature()
\sa HbPredictionEngine::disableFeature()
*/
bool HbPredictionEngine::isFeatureEnabled(HbInputPredictionFeature feature)
{
    Q_UNUSED(feature);
    return false;
}

/*!
Disables a specified feature for the prediction engine. 
Returns true if the engine supports the feature and is able to disable the feature,
otherwise, returns false.
Note: There is temporary default implementation for this method. It will be made
pure virtual after suitable transition period.

\sa HbPredictionEngine::isFeatureEnabled()
\sa HbPredictionEngine::enableFeature()
*/
bool HbPredictionEngine::disableFeature(HbInputPredictionFeature feature)
{
    Q_UNUSED(feature);
    return false;
}

/*!
Returns list of next word candidates. The forms this list based on what has been
written so far. This method will return candidates only if the engine
supports next word prediction.

\sa supportedFeatures
*/
QStringList HbPredictionEngine::nextWordCandidateList(HbPredictionCallback* callback)
{
    Q_UNUSED(callback);
    return QStringList();
}

/*!
\class HbPredictionEngineChinese
\brief Abstract API for predictive Chinese input.

This interface extends predictiv einput abstraction layer with methods specific
to predictive Chinese input. Those are mainly related to spelling candidates and candidate
page handling.
*/

/*!
\deprecated HbPredictionEngineChinese::pressKey(const int keycode, const Qt::KeyboardModifiers modifiers, const int textCase)
    is deprecated. use base class AppendkeyPress instead.
*/

/*!
\deprecated HbPredictionEngineChinese::HbPredictionEngineChinese::getCandidates(int, int)
    is deprecated. Use base class candidates() instead.
*/

// End of file
