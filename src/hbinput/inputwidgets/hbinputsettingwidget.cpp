/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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
#include "hbinputsettingwidget.h"

#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbinpututils.h>
#include <hbinputsettingproxy.h>
#include <hbinputpredictionfactory.h>
#include <QtAlgorithms>

#include "hbinputcheckboxlist_p.h"

const QString statusOff = QObject::tr("Off");
const QString statusOn = QObject::tr("On");
const QString bestPrediction = QObject::tr("Best prediction");
const QString exactTyping = QObject::tr("Exact typing");


/// @cond

class HbInputSettingWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbInputSettingWidget)

public:
    HbInputSettingWidgetPrivate(HbDataForm *dataForm);

    void initialize();
    void createSettingItems();
    void fillLanguageList(QStringList &list, QList<HbInputLanguage> &languageList, const QString &replace = QString(" "));
    int languageToIndex(const HbInputLanguage &language, const QList<HbInputLanguage> &languageList);
    HbInputLanguage indexToLanguage(int index, const QList<HbInputLanguage> &languageList);
    void createSecondaryLanguageList();
    void updateContentWidgetData();
public:
    HbDataForm *mForm;
    HbDataFormModelItem *mPrimaryLanguageItem;
    HbDataFormModelItem *mSecondaryLanguageItem;
    HbDataFormModelItem *mKeypressTimeoutItem;
    HbDataFormModelItem *mCharacterPreviewItem;
    HbDataFormModelItem *mPredictionItem;
    HbDataFormModelItem *mAutoCompletionItem;
    HbDataFormModelItem *mCorrectionLevelItem;
    HbDataFormModelItem *mPrimaryCandidateItem;
    HbInputLanguage mPrimaryInputLanguage;
    HbInputLanguage mSecondaryInputLanguage;
    QList<HbInputLanguage> mPrimaryLanguages;
    QList<HbInputLanguage> mSecondaryLanguages;
    bool mPredictionStatusForITUT;
    bool mPredictionStatusForQwerty;
    bool mCharacterPreviewEnabled;
    int mKeypressTimeout;
    bool mAutocompletionForITUT;
    bool mAutocompletionForQwerty;
    HbTypingCorrectionLevel mTypingCorrectionLevel;
    HbPrimaryCandidateMode mPrimaryCandidateMode;
    HbInputSettingWidget *q_ptr;
    HbDataFormModel *mModel;
};

/*!
Constructs setting widget
*/
HbInputSettingWidgetPrivate::HbInputSettingWidgetPrivate(HbDataForm *dataForm)
 : mForm(dataForm), mPrimaryLanguageItem(NULL),
   mSecondaryLanguageItem(NULL), mKeypressTimeoutItem(NULL),
   mCharacterPreviewItem(NULL), mPredictionItem(NULL),
   mAutoCompletionItem(NULL), mCorrectionLevelItem(NULL),
   mPrimaryCandidateItem(NULL), q_ptr(NULL), mModel(0)
{
}

/*!
Initializes setting widget
*/
void HbInputSettingWidgetPrivate::initialize()
{
    HbInputSettingProxy *settings = HbInputSettingProxy::instance();
    mPrimaryInputLanguage = settings->globalInputLanguage();
    mSecondaryInputLanguage = settings->globalSecondaryInputLanguage();
    mPredictionStatusForITUT = settings->predictiveInputStatus(HbKeyboardSetting12key);
    mPredictionStatusForQwerty = settings->predictiveInputStatus(HbKeyboardSettingQwerty);
    mCharacterPreviewEnabled = settings->isCharacterPreviewForQwertyEnabled();
    mKeypressTimeout = settings->keypressTimeout();
    mAutocompletionForITUT = settings->isAutocompletionEnabled(HbKeyboardSetting12key);
    mAutocompletionForQwerty = settings->isAutocompletionEnabled(HbKeyboardSettingQwerty);
    mPrimaryCandidateMode = settings->primaryCandidateMode();
    mTypingCorrectionLevel = settings->typingCorrectionLevel();

    HbInputUtils::listSupportedInputLanguages(mPrimaryLanguages);
    createSecondaryLanguageList();
    // if the model is already constructed we do not need to recreate the setting items
    if (!mModel) {
        createSettingItems();
    } else {
        // simply update the settings dependant content widget data of all the items
        updateContentWidgetData();
        //make sure that the items are not expanded
        QModelIndex index = mModel->indexFromItem(mSecondaryLanguageItem->parent());
        mForm->setExpanded(index, false);
        index = mModel->indexFromItem(mKeypressTimeoutItem->parent());
        mForm->setExpanded(index, false);
        index = mModel->indexFromItem(mPredictionItem->parent());
        mForm->setExpanded(index, false);        
    }
}

void HbInputSettingWidgetPrivate::updateContentWidgetData() 
{
    // current primary language
    mPrimaryLanguageItem->setContentWidgetData(QString("currentIndex"), languageToIndex(mPrimaryInputLanguage, mPrimaryLanguages));

    mSecondaryLanguageItem->setContentWidgetData(QString("currentIndex"), languageToIndex(mSecondaryInputLanguage, mSecondaryLanguages));
    // key press timeout
    mKeypressTimeoutItem->setContentWidgetData(QString("sliderPosition"), mKeypressTimeout);
    if (mCharacterPreviewEnabled) {
        mCharacterPreviewItem->setContentWidgetData(QString("text"), statusOn);
        mCharacterPreviewItem->setContentWidgetData(QString("additionalText"), statusOff);
    } else {
        mCharacterPreviewItem->setContentWidgetData(QString("text"), statusOff);
        mCharacterPreviewItem->setContentWidgetData(QString("additionalText"), statusOn);
    }
    QList<QVariant> predictionEnabled;
    predictionEnabled << mPredictionStatusForQwerty << mPredictionStatusForITUT;
    mPredictionItem->setContentWidgetData(QString("selectedItems"), predictionEnabled);

    QList<QVariant> autocompletionEnabled;
    autocompletionEnabled << mAutocompletionForQwerty << mAutocompletionForITUT;
    mAutoCompletionItem->setContentWidgetData(QString("selectedItems"), autocompletionEnabled);
    
    mCorrectionLevelItem->setContentWidgetData(QString("selected"), mTypingCorrectionLevel);

    if (mPrimaryCandidateMode == HbPrimaryCandidateModeBestPrediction) {
        mPrimaryCandidateItem->setContentWidgetData(QString("text"), bestPrediction);
        mPrimaryCandidateItem->setContentWidgetData(QString("additionalText"), exactTyping);
    } else {
        mPrimaryCandidateItem->setContentWidgetData(QString("text"), exactTyping);
        mPrimaryCandidateItem->setContentWidgetData(QString("additionalText"), bestPrediction);
    }
}

/*!
Creates setting items to this widget
*/
void HbInputSettingWidgetPrivate::createSettingItems()
{
    Q_Q(HbInputSettingWidget);

    mModel = new HbDataFormModel();

    HbInputCheckBoxList *customPrototype = new HbInputCheckBoxList(mForm);
    QList<HbAbstractViewItem*> prototypes = mForm->itemPrototypes();
    prototypes.append(customPrototype);
    mForm->setItemPrototypes(prototypes);

    HbDataFormModelItem *languageGroup = mModel->appendDataFormGroup(QObject::tr("Language"));

    mPrimaryLanguageItem = new HbDataFormModelItem(HbDataFormModelItem::ComboBoxItem, QObject::tr("Primary Writing language"));
    languageGroup->appendChild(mPrimaryLanguageItem);
    QStringList writingLanguageItems;
    fillLanguageList(writingLanguageItems, mPrimaryLanguages);
    mPrimaryLanguageItem->setContentWidgetData(QString("items"), writingLanguageItems);
    mPrimaryLanguageItem->setContentWidgetData(QString("currentIndex"), languageToIndex(mPrimaryInputLanguage, mPrimaryLanguages));
    mPrimaryLanguageItem->setContentWidgetData(QString("objectName"), QString("primary_writing_language"));
    mForm->addConnection(mPrimaryLanguageItem, SIGNAL(currentIndexChanged(int)), q, SLOT(setPrimaryLanguage(int)));

    mSecondaryLanguageItem = new HbDataFormModelItem(HbDataFormModelItem::ComboBoxItem, QObject::tr("Secondary Writing language"));
    languageGroup->appendChild(mSecondaryLanguageItem);
    QStringList secondaryLanguageItems;
    fillLanguageList(secondaryLanguageItems, mSecondaryLanguages, QObject::tr("None"));
    mSecondaryLanguageItem->setContentWidgetData(QString("items"), secondaryLanguageItems);
    mSecondaryLanguageItem->setContentWidgetData(QString("currentIndex"), languageToIndex(mSecondaryInputLanguage, mSecondaryLanguages));
    mSecondaryLanguageItem->setContentWidgetData(QString("objectName"), QString("secondary_writing_language"));
    mForm->addConnection(mSecondaryLanguageItem, SIGNAL(currentIndexChanged(int)), q, SLOT(setSecondaryLanguage(int)));

    HbDataFormModelItem *keyboardGroup = mModel->appendDataFormGroup(QObject::tr("Keyboard"));

    mKeypressTimeoutItem = new HbDataFormModelItem(HbDataFormModelItem::SliderItem, QObject::tr("Keypress Timeout"));
    keyboardGroup->appendChild(mKeypressTimeoutItem);
    mKeypressTimeoutItem->setContentWidgetData(QString("minimum"), HbInputMinKeypressTimeout);
    mKeypressTimeoutItem->setContentWidgetData(QString("maximum"), HbInputMaxKeypressTimeout);
    mKeypressTimeoutItem->setContentWidgetData(QString("sliderPosition"), mKeypressTimeout);
    mKeypressTimeoutItem->setContentWidgetData(QString("objectName"), QString("keypress_timeout"));
    mForm->addConnection(mKeypressTimeoutItem, SIGNAL(valueChanged(int)), q, SLOT(setKeypressTimeoutValue(int)));

    mCharacterPreviewItem = new HbDataFormModelItem(HbDataFormModelItem::ToggleValueItem, QObject::tr("Character bubble"));
    keyboardGroup->appendChild(mCharacterPreviewItem);
    if (mCharacterPreviewEnabled) {
        mCharacterPreviewItem->setContentWidgetData(QString("text"), statusOn);
        mCharacterPreviewItem->setContentWidgetData(QString("additionalText"), statusOff);
    } else {
        mCharacterPreviewItem->setContentWidgetData(QString("text"), statusOff);
        mCharacterPreviewItem->setContentWidgetData(QString("additionalText"), statusOn);
    }
    mCharacterPreviewItem->setContentWidgetData(QString("objectName"), QString("character_bubble"));
    


    HbDataFormModelItem *textInputGroup = mModel->appendDataFormGroup(QObject::tr("Intelligent Text Input"));

    HbDataFormModelItem::DataItemType checkboxList =
        static_cast<HbDataFormModelItem::DataItemType>(HbDataFormModelItem::CustomItemBase);

    mPredictionItem = new HbDataFormModelItem(checkboxList, QObject::tr("Prediction"));
    textInputGroup->appendChild(mPredictionItem);
    QStringList predictionValues;
    predictionValues << QObject::tr("Qwerty") << QObject::tr("Virtual ITU-T");
    mPredictionItem->setContentWidgetData(QString("items"), predictionValues);
    QList<QVariant> predictionEnabled;
    predictionEnabled << mPredictionStatusForQwerty << mPredictionStatusForITUT;
    mPredictionItem->setContentWidgetData(QString("selectedItems"), predictionEnabled);
    mPredictionItem->setContentWidgetData(QString("objectName"), QString("prediction"));
    mForm->addConnection(mPredictionItem, SIGNAL(activated(const QModelIndex &)), q, SLOT(setPredictionState(const QModelIndex &)));

    mAutoCompletionItem = new HbDataFormModelItem(checkboxList, QObject::tr("Autocompletion"));
    textInputGroup->appendChild(mAutoCompletionItem);
    QStringList autoCompletionValues;
    autoCompletionValues << QObject::tr("Qwerty") << QObject::tr("Virtual ITU-T");
    mAutoCompletionItem->setContentWidgetData(QString("items"), autoCompletionValues);
    QList<QVariant> autocompletionEnabled;
    autocompletionEnabled << mAutocompletionForQwerty << mAutocompletionForITUT;
    mAutoCompletionItem->setContentWidgetData(QString("selectedItems"), autocompletionEnabled);
    mAutoCompletionItem->setContentWidgetData(QString("objectName"), QString("autocompletion"));
    mForm->addConnection(mAutoCompletionItem, SIGNAL(activated(const QModelIndex &)), q, SLOT(setAutocompletionState(const QModelIndex &)));

    mCorrectionLevelItem = new HbDataFormModelItem(HbDataFormModelItem::RadioButtonListItem, QObject::tr("Typing Correction"));
    textInputGroup->appendChild(mCorrectionLevelItem);
    QStringList correctionLevels;
    correctionLevels << QObject::tr("Low") << QObject::tr("Medium") << QObject::tr("High");
    mCorrectionLevelItem->setContentWidgetData(QString("items"), correctionLevels);
    mCorrectionLevelItem->setContentWidgetData(QString("selected"), mTypingCorrectionLevel);
    mCorrectionLevelItem->setContentWidgetData(QString("objectName"), QString("typing_correction"));
    mForm->addConnection(mCorrectionLevelItem, SIGNAL(itemSelected(int)), q, SLOT(setCorrectionLevel(int)));

    mPrimaryCandidateItem = new HbDataFormModelItem(HbDataFormModelItem::ToggleValueItem, QObject::tr("Primary Candidate"));
    textInputGroup->appendChild(mPrimaryCandidateItem);
    if (mPrimaryCandidateMode == HbPrimaryCandidateModeBestPrediction) {
        mPrimaryCandidateItem->setContentWidgetData(QString("text"), bestPrediction);
        mPrimaryCandidateItem->setContentWidgetData(QString("additionalText"), exactTyping);
    } else {
        mPrimaryCandidateItem->setContentWidgetData(QString("text"), exactTyping);
        mPrimaryCandidateItem->setContentWidgetData(QString("additionalText"), bestPrediction);
    }
    mPrimaryCandidateItem->setContentWidgetData(QString("objectName"), QString("primary_candidate"));
    mForm->setModel(mModel);    
    QObject::connect(mModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)), q, SLOT(dataChange(QModelIndex, QModelIndex)));
}

/*!
Fills given list with language names in the language list
*/
void HbInputSettingWidgetPrivate::fillLanguageList(QStringList &list, QList<HbInputLanguage> &languageList, const QString &replace)
{
    foreach(HbInputLanguage language, languageList)  {
        QString langName = language.localisedName();
        if (langName.length() == 0) {
            langName = replace;
        }
        list << langName;
    }
}

/*!
Returns index of the given language at the language list
*/
int HbInputSettingWidgetPrivate::languageToIndex(const HbInputLanguage &language, const QList<HbInputLanguage> &languageList)
{
    for (int i = 0; i < languageList.count(); ++i) {
        if (languageList.at(i) == language) {
            return i;
        }
    }
    return -1;
}

/*!
Returns language in the given index at the language list
*/
HbInputLanguage HbInputSettingWidgetPrivate::indexToLanguage(int index, const QList<HbInputLanguage> &languageList)
{
    if (index >= 0 && index < languageList.count()) {
        return languageList.at(index);
    } else {
        return HbInputLanguage();
    }
}

/*!
Creates list of secondary languages
*/
void HbInputSettingWidgetPrivate::createSecondaryLanguageList()
{
    mSecondaryLanguages.clear();

    mSecondaryLanguages.append(HbInputLanguage());

    if (mPrimaryInputLanguage.language() != QLocale::Chinese) {
        foreach(HbInputLanguage language, mPrimaryLanguages) {
            if (language != mPrimaryInputLanguage &&
                language != QLocale::Chinese) {
                mSecondaryLanguages.append(language);
            }
        }
    }
}

/// @endcond

/*!
Constructs input setting widget
*/
HbInputSettingWidget::HbInputSettingWidget(HbDataForm *dataForm, QGraphicsWidget* parent)
 : QObject(parent), d_ptr(new HbInputSettingWidgetPrivate(dataForm))
{
    Q_D(HbInputSettingWidget);
    d->q_ptr = this;
}

/*!
Destructs the object
*/
HbInputSettingWidget::~HbInputSettingWidget()
{
    delete d_ptr;
}

/*!
Initializes the data form object with input settings
*/
void HbInputSettingWidget::initializeWidget()
{
    Q_D(HbInputSettingWidget);

    d->initialize();

    HbInputSettingProxy *settings = HbInputSettingProxy::instance();
    connect(settings, SIGNAL(globalInputLanguageChanged(const HbInputLanguage &)), this, SLOT(updateGlobalInputLanguage(const HbInputLanguage &)));
    connect(settings, SIGNAL(globalSecondaryInputLanguageChanged(const HbInputLanguage &)), this, SLOT(updateGlobalSecondaryInputLanguage(const HbInputLanguage &)));
    connect(settings, SIGNAL(predictiveInputStateChanged(HbKeyboardSettingFlags, bool)), this, SLOT(updatePredictiveInputState(HbKeyboardSettingFlags, bool)));
    connect(settings, SIGNAL(characterPreviewStateForQwertyChanged(bool)), this, SLOT(updateCharacterPreviewStateForQwerty(bool)));
    connect(settings, SIGNAL(keypressTimeoutChanged(int)), this, SLOT(updateKeypressTimeout(int)));
    connect(settings, SIGNAL(autocompletionStateChanged(HbKeyboardSettingFlags, bool)), this, SLOT(updateAutocompletionState(HbKeyboardSettingFlags, bool)));
    connect(settings, SIGNAL(typingCorrectionLevelChanged(HbTypingCorrectionLevel)), this, SLOT(updateTypingCorrectionLevel(HbTypingCorrectionLevel)));
    connect(settings, SIGNAL(primaryCandidateModeChanged(HbPrimaryCandidateMode)), this, SLOT(updatePrimaryCandidateMode(HbPrimaryCandidateMode)));
}

/*!
Called by framework when primary language is changed
*/
void HbInputSettingWidget::updateGlobalInputLanguage(const HbInputLanguage &newLanguage)
{
    Q_D(HbInputSettingWidget);

    if (d->mPrimaryInputLanguage != newLanguage) {
        setPrimaryLanguage(d->languageToIndex(newLanguage, d->mPrimaryLanguages));
        d->mPrimaryLanguageItem->setContentWidgetData(QString("currentIndex"), d->languageToIndex(d->mPrimaryInputLanguage, d->mPrimaryLanguages));
    }
}

/*!
Called by framework when secondary language is changed
*/
void HbInputSettingWidget::updateGlobalSecondaryInputLanguage(const HbInputLanguage &newLanguage)
{
    Q_D(HbInputSettingWidget);

    if (d->mSecondaryInputLanguage != newLanguage) {
        setSecondaryLanguage(d->languageToIndex(newLanguage, d->mSecondaryLanguages));
        d->mSecondaryLanguageItem->setContentWidgetData(QString("currentIndex"), d->languageToIndex(d->mSecondaryInputLanguage, d->mSecondaryLanguages));
    }
}

/*!
Called by framework when prediction status is changed
*/
void HbInputSettingWidget::updatePredictiveInputState(HbKeyboardSettingFlags keyboardType, bool newState)
{
    Q_D(HbInputSettingWidget);

    bool changed = false;
    if (keyboardType & HbKeyboardSetting12key &&
        d->mPredictionStatusForITUT != newState) {
        d->mPredictionStatusForITUT = newState;
        changed = true;
    } else if (keyboardType & HbKeyboardSettingQwerty &&
        d->mPredictionStatusForQwerty != newState) {
        d->mPredictionStatusForQwerty = newState;
        changed = true;
    }

    if (changed) {
        QList<QVariant> predictionEnabled;
        predictionEnabled << d->mPredictionStatusForQwerty << d->mPredictionStatusForITUT;
        d->mPredictionItem->setContentWidgetData(QString("selectedItems"), predictionEnabled);
    }
}

/*!
Called by framework when character preview state is changed
*/
void HbInputSettingWidget::updateCharacterPreviewStateForQwerty(bool newState)
{
    Q_D(HbInputSettingWidget);

    if (d->mCharacterPreviewEnabled != newState) {
        d->mCharacterPreviewEnabled = newState;
        if (d->mCharacterPreviewEnabled) {
            d->mCharacterPreviewItem->setContentWidgetData(QString("text"), statusOn);
            d->mCharacterPreviewItem->setContentWidgetData(QString("additionalText"), statusOff);
        } else {
            d->mCharacterPreviewItem->setContentWidgetData(QString("text"), statusOff);
            d->mCharacterPreviewItem->setContentWidgetData(QString("additionalText"), statusOn);
        }
    }
}

/*!
Called by framework when keypress timeout is changed
*/
void HbInputSettingWidget::updateKeypressTimeout(int newTimeout)
{
    Q_D(HbInputSettingWidget);

    if (d->mKeypressTimeout != newTimeout) {
        d->mKeypressTimeout = newTimeout;
        d->mKeypressTimeoutItem->setContentWidgetData(QString("sliderPosition"), d->mKeypressTimeout);
    }
}

/*!
Called by framework when autocompletion state is changed
*/
void HbInputSettingWidget::updateAutocompletionState(HbKeyboardSettingFlags keyboardType, bool newState)
{
    Q_D(HbInputSettingWidget);

    bool changed = false;
    if (keyboardType & HbKeyboardSetting12key &&
        d->mAutocompletionForITUT != newState) {
        d->mAutocompletionForITUT = newState;
        changed = true;
    } else if (keyboardType & HbKeyboardSettingQwerty &&
        d->mAutocompletionForQwerty != newState) {
        d->mAutocompletionForQwerty = newState;
        changed = true;
    }

    if (changed) {
        QList<QVariant> autocompletionEnabled;
        autocompletionEnabled << d->mAutocompletionForQwerty << d->mAutocompletionForITUT;
        d->mAutoCompletionItem->setContentWidgetData(QString("selectedItems"), autocompletionEnabled);
    }
}

/*!
Called by framework when typing correction level is changed
*/
void HbInputSettingWidget::updateTypingCorrectionLevel(HbTypingCorrectionLevel newLevel)
{
    Q_D(HbInputSettingWidget);

    if (d->mTypingCorrectionLevel != newLevel) {
        d->mTypingCorrectionLevel = newLevel;
        d->mCorrectionLevelItem->setContentWidgetData(QString("selected"), d->mTypingCorrectionLevel);
    }
}

/*!
Called by framework when primary candidate mode is changed
*/
void HbInputSettingWidget::updatePrimaryCandidateMode(HbPrimaryCandidateMode newMode)
{
    Q_D(HbInputSettingWidget);

    if (d->mPrimaryCandidateMode != newMode) {
        d->mPrimaryCandidateMode = newMode;
        if (d->mPrimaryCandidateMode == HbPrimaryCandidateModeBestPrediction) {
            d->mPrimaryCandidateItem->setContentWidgetData(QString("text"), bestPrediction);
            d->mPrimaryCandidateItem->setContentWidgetData(QString("additionalText"), exactTyping);
        } else {
            d->mPrimaryCandidateItem->setContentWidgetData(QString("text"), exactTyping);
            d->mPrimaryCandidateItem->setContentWidgetData(QString("additionalText"), bestPrediction);
        }
    }
}

/*!
Saves the new primary language and modifies the secondary language list if necessary
*/
void HbInputSettingWidget::setPrimaryLanguage(int index)
{
    Q_D(HbInputSettingWidget);

	HbInputSettingProxy *settings = HbInputSettingProxy::instance();
    HbPredictionFactory *predFactory = HbPredictionFactory::instance();
    bool oldPLangSupportsPrediction = (predFactory->predictionEngineForLanguage(d->mPrimaryInputLanguage) != NULL);		
    d->mPrimaryInputLanguage = d->indexToLanguage(index, d->mPrimaryLanguages);
    HbInputSettingProxy::instance()->setGlobalInputLanguage(d->mPrimaryInputLanguage);
    bool langSupportsPrediction = (predFactory->predictionEngineForLanguage(d->mPrimaryInputLanguage) != NULL);		
	if( oldPLangSupportsPrediction != langSupportsPrediction) {
		if(settings->predictiveInputStatus(HbKeyboardSetting12key) != langSupportsPrediction) {
			settings->setPredictiveInputStatus(HbKeyboardSetting12key, langSupportsPrediction);
		} 
		if (settings->predictiveInputStatus(HbKeyboardSettingQwerty) != langSupportsPrediction) {
			settings->setPredictiveInputStatus(HbKeyboardSettingQwerty, langSupportsPrediction);
		}
	} 	

    HbInputLanguage secondaryLanguage = d->mSecondaryInputLanguage;
    // Update secondary language list
    d->createSecondaryLanguageList();
    QStringList secondaryLanguageItems;
    d->fillLanguageList(secondaryLanguageItems, d->mSecondaryLanguages, tr("None"));
    d->mSecondaryLanguageItem->setContentWidgetData(QString("items"), secondaryLanguageItems);

    if (d->mPrimaryInputLanguage != secondaryLanguage) {
        d->mSecondaryLanguageItem->setContentWidgetData(QString("currentIndex"), d->languageToIndex(secondaryLanguage, d->mSecondaryLanguages));
    }
}

/*!
Saves the new secondary language
*/
void HbInputSettingWidget::setSecondaryLanguage(int index)
{
    Q_D(HbInputSettingWidget);

    d->mSecondaryInputLanguage = d->indexToLanguage(index, d->mSecondaryLanguages);
    HbInputSettingProxy::instance()->setGlobalSecondaryInputLanguage(d->mSecondaryInputLanguage);
}

/*!
Saves the keypress timeout value
*/
void HbInputSettingWidget::setKeypressTimeoutValue(int value)
{
    Q_D(HbInputSettingWidget);

    d->mKeypressTimeout = value;
    HbInputSettingProxy::instance()->setKeypressTimeout(d->mKeypressTimeout);
}

/*!
Saves the new character preview state
*/
void HbInputSettingWidget::setCharacterPreviewState()
{
    Q_D(HbInputSettingWidget);

    d->mCharacterPreviewEnabled = !d->mCharacterPreviewEnabled;
    HbInputSettingProxy::instance()->setCharacterPreviewForQwerty(d->mCharacterPreviewEnabled);
}

/*!
Saves the new prediction state for selected keyboard
*/
void HbInputSettingWidget::setPredictionState(const QModelIndex &index)
{
    Q_D(HbInputSettingWidget);

    if (index.row() == 0) {
        d->mPredictionStatusForQwerty = !d->mPredictionStatusForQwerty;
        HbInputSettingProxy::instance()->setPredictiveInputStatus(HbKeyboardSettingQwerty, d->mPredictionStatusForQwerty);
    } else {
        d->mPredictionStatusForITUT = !d->mPredictionStatusForITUT;
        HbInputSettingProxy::instance()->setPredictiveInputStatus(HbKeyboardSetting12key, d->mPredictionStatusForITUT);
    }
}

/*!
Saves the new autocompletion state for selected keyboard
*/
void HbInputSettingWidget::setAutocompletionState(const QModelIndex &index)
{
    Q_D(HbInputSettingWidget);

    if (index.row() == 0) {
        d->mAutocompletionForQwerty = !d->mAutocompletionForQwerty;
        HbInputSettingProxy::instance()->setAutocompletionStatus(HbKeyboardSettingQwerty, d->mAutocompletionForQwerty);
    } else {
        d->mAutocompletionForITUT = !d->mAutocompletionForITUT;
        HbInputSettingProxy::instance()->setAutocompletionStatus(HbKeyboardSetting12key, d->mAutocompletionForITUT);
    }
}

/*!
Saves the new typing correction level
*/
void HbInputSettingWidget::setCorrectionLevel(int index)
{
    Q_D(HbInputSettingWidget);

    d->mTypingCorrectionLevel = static_cast<HbTypingCorrectionLevel>(index);
    HbInputSettingProxy::instance()->setTypingCorrectionLevel(d->mTypingCorrectionLevel);
}

/*!
Saves the new primary candidate mode
*/
void HbInputSettingWidget::setPrimaryCandidateMode()
{
    Q_D(HbInputSettingWidget);

    if (d->mPrimaryCandidateMode == HbPrimaryCandidateModeBestPrediction) {
        d->mPrimaryCandidateMode = HbPrimaryCandidateModeExactTyping;
    } else {
        d->mPrimaryCandidateMode = HbPrimaryCandidateModeBestPrediction;
    }
    HbInputSettingProxy::instance()->setPrimaryCandidateMode(d->mPrimaryCandidateMode);
}
/*
    This slot is called when ever data in the form model is changed
*/
void HbInputSettingWidget::dataChange(const QModelIndex &startIn, const QModelIndex &endIn)
{
    Q_D(HbInputSettingWidget);
    Q_UNUSED(endIn);
    HbDataFormModelItem *item = d->mModel->itemFromIndex(startIn);    
    if(item == d->mPrimaryCandidateItem) {
        setPrimaryCandidateMode();
    } else if(item == d->mCharacterPreviewItem) {
        setCharacterPreviewState();
    } 
}

/*
 Resets the widget and disconnects the signals connected to the settings proxy
*/

void HbInputSettingWidget::resetWidget()
{
    HbInputSettingProxy *settings = HbInputSettingProxy::instance();
    disconnect(settings, SIGNAL(globalInputLanguageChanged(const HbInputLanguage &)), this, SLOT(updateGlobalInputLanguage(const HbInputLanguage &)));
    disconnect(settings, SIGNAL(globalSecondaryInputLanguageChanged(const HbInputLanguage &)), this, SLOT(updateGlobalSecondaryInputLanguage(const HbInputLanguage &)));
    disconnect(settings, SIGNAL(predictiveInputStateChanged(HbKeyboardSettingFlags, bool)), this, SLOT(updatePredictiveInputState(HbKeyboardSettingFlags, bool)));
    disconnect(settings, SIGNAL(characterPreviewStateForQwertyChanged(bool)), this, SLOT(updateCharacterPreviewStateForQwerty(bool)));
    disconnect(settings, SIGNAL(keypressTimeoutChanged(int)), this, SLOT(updateKeypressTimeout(int)));
    disconnect(settings, SIGNAL(autocompletionStateChanged(HbKeyboardSettingFlags, bool)), this, SLOT(updateAutocompletionState(HbKeyboardSettingFlags, bool)));
    disconnect(settings, SIGNAL(typingCorrectionLevelChanged(HbTypingCorrectionLevel)), this, SLOT(updateTypingCorrectionLevel(HbTypingCorrectionLevel)));
    disconnect(settings, SIGNAL(primaryCandidateModeChanged(HbPrimaryCandidateMode)), this, SLOT(updatePrimaryCandidateMode(HbPrimaryCandidateMode)));    
}


// End of file
