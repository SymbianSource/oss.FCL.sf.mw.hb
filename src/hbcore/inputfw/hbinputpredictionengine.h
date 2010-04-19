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

#ifndef HB_PREDICTION_ENGINE_H
#define HB_PREDICTION_ENGINE_H

#include <QStringList>
#include <QKeyEvent>

#include <hbinputlanguage.h>

class HbUserDictionary;
class HbPredictionCallback;
struct HbKeyPressProbability;

/*!
Enumerates bits for predictione engine features.
*/
enum HbInputPredictionFeatureFlag {
    HbPredFeatureNone               = 0x00000000,
    HbPredFeatureReordering         = 0x00000001,  /**< Candidates are reordered accoriding to the frequency. */
    HbPredFeatureUserDictionary     = 0x00000002,  /**< User dictionary is used for storing user specific words. */
    HbPredFeatureWordCompletion     = 0x00000004,  /**< Auto word completion feature is used. */
    HbPredFeatureNextWordPrediction = 0x00000008,  /**< Next word prediction feature is used. */
    HbPredFeatureExtraDictionaries  = 0x00000010,  /**< The engine supports additional language databases. */
    HbPredfeatureErrorCorrection    = 0x00000020   /**< The engine supports typing correction feature. */
};

Q_DECLARE_FLAGS(HbInputPredictionFeature, HbInputPredictionFeatureFlag)

/*!
Enumerates bits for prediction engine interface types
*/
enum HbPredictionInterfaceTypeFlag {
    HbPredInterfaceNone       = 0x00000000,
    HbPredInterfaceLatinBased = 0x00000001,
    HbPredInterfaceChinese    = 0x00000002,
    HbPredInterfaceJapanese   = 0x00000004,
    HbPredInterfaceHidden     = 0x00000008     /**< The engine is exluded from factory queries and must be instantiated directly */
};

Q_DECLARE_FLAGS(HbPredictionInterfaceType, HbPredictionInterfaceTypeFlag)


class HB_CORE_EXPORT HbPredictionBase
{        
public:
    virtual ~HbPredictionBase();

protected:
    HbPredictionBase();

public:
    virtual QList<HbInputLanguage> languages() const = 0;
    virtual void setWord(const QString& word, HbPredictionCallback* callback = 0) = 0;
    virtual void updateCandidates(int& bestGuessLocation, bool& noMoreCandidates) = 0;
    virtual void appendKeyPress(const int keycode, const Qt::KeyboardModifiers modifiers, const HbTextCase textCase = HbTextCaseNone, HbPredictionCallback* callback = 0) = 0;
    virtual void deleteKeyPress(HbPredictionCallback* callback = 0) = 0;
    virtual void commit(const QString &word = QString()) = 0;
    virtual void clear() = 0;
    virtual void addUsedWord(const QString& word) = 0;
    virtual HbInputPredictionFeature features() const = 0;
    virtual QString vendorIdString() const = 0;
    virtual QString engineVersion() const = 0;
    virtual bool supportsKeyboardType(const HbInputLanguage& language, HbKeyboardType keyboard) const = 0;

    virtual HbUserDictionary* userDictionary() const;
    virtual void setExtraUserDictionary(int id);
    virtual void setExtraUserDictionaries(const QList<int>& idList);
    virtual bool setLanguage(const HbInputLanguage &language, HbInputModeType inputMode = HbInputModeNone);
    virtual HbInputLanguage language() const;
    virtual void setKeyboard(HbKeyboardType keyboardType) = 0;
    virtual HbKeyboardType keyboardType() const;
};


class HB_CORE_EXPORT HbPredictionEngine : public HbPredictionBase
{
public:
    /*!
    Specifies error correction levels. 
    */
    enum HbErrorCorrectionLevel {
        HbErrorCorrectionLevelNone = 0,
        HbErrorCorrectionLevelLow,
        HbErrorCorrectionLevelMedium,
        HbErrorCorrectionLevelHigh
    };

public:
    virtual void setCandidateList(QStringList* candidateList) = 0;
    virtual QStringList candidateList() = 0;
    virtual int inputLength() = 0;
    virtual void appendCharacter(const QChar aChar, const HbTextCase textCase = HbTextCaseNone, HbPredictionCallback* callback = 0) = 0;

    virtual QStringList nextWordCandidateList(HbPredictionCallback* callback = 0);
    virtual bool setErrorCorrectionLevel(HbErrorCorrectionLevel level);
    virtual HbErrorCorrectionLevel errorCorrectionLevel() const;
    virtual bool setSecondaryLanguage(const HbInputLanguage& language);
    virtual HbInputLanguage secondaryLanguage() const;
    virtual QString currentWord() const;
    virtual bool enableFeature(HbInputPredictionFeature feature);
    virtual bool isFeatureEnabled(HbInputPredictionFeature feature);
    virtual bool disableFeature(HbInputPredictionFeature feature);
};


class HB_CORE_EXPORT HbPredictionEngineChinese : public HbPredictionEngine
{
public:
    virtual void setInputMode(HbInputModeType imMode) = 0;
    virtual HbInputModeType inputMode() const = 0;

    virtual bool spelling(int index, QString &out) = 0;
    virtual bool selectSpelling(int index) = 0;
    virtual QStringList allSpellings() = 0;
    virtual int spellingCount() const = 0;

    virtual QStringList getCandidates(int startIndex, int count) = 0;
    virtual bool selectCandidate(int index) = 0;
    virtual bool selectCandidate(const QString& candidate) = 0; 
    virtual bool candidateExist(int index) = 0;

    virtual bool pressKey(const int keycode, const Qt::KeyboardModifiers modifiers, const int textCase = 0) = 0;
    virtual bool isInputModeSupported(HbInputModeType imMode) = 0;
    // used for hwr engine
    virtual bool addStroke(const QList<QPointF>& traceData) = 0;
    virtual bool inlineSpelling(int idx, QString &out) = 0;
};

#endif // HB_PREDICTION_ENGINE

// End of file
