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
#include <QInputContextPlugin>
#include <QLibrary>
#include <QPluginLoader>
#include <QDir>
#include <QtAlgorithms>

#include "hbinputcheckboxlist_p.h"
#include "hbinputsettingproxy_p.h"
#include <hbinputmethoddescriptor.h>

const QString statusOff = QObject::tr("Off");
const QString statusOn = QObject::tr("On");
const QString bestPrediction = QObject::tr("Best prediction");
const QString exactTyping = QObject::tr("Exact typing");

/// @cond
// input modes name for chinese
const QString KPinyinName("Pinyin");
const QString KStrokeName("Stroke");
const QString KZhuyinName("Zhuyin");
const QString KCangjieNormalName("CangjieNormal");
const QString KCangjieEasyName("CangjieEasy");
const QString KCangjieAdvancedName("CangjieAdvanced");
const QString KHwrName("Handwriting");
const QString KHwrVerySlowName("VerySlow");
const QString KHwrSlowName("Slow");
const QString KHwrNormalName("Normal");
const QString KHwrFastName("Fast");
const QString KHwrVeryFastName("VeryFast");

// strings used for represent cangjie
const QString KCangjieGeneralName("Cangjie");
const QString KEasy("Easy");
const QString KNormal("Normal");
const QString KAdvanced("Advanced");

// input modes value for chinese
const int KChineseInputModeNone = -1;
const int KPinyinMode = 0;
const int KStrokeMode = 1;
const int KZhuyinMode = 2;
const int KCangjieNormalMode = 3;
const int KCangjieEasyMode = 4;
const int KCangjieAdvancedMode = 5;
const int KHwrMode = 6;

// define chinese input plugin name
#ifdef Q_OS_WIN
const QString KCnHwrPluginName("HbChineseHwrd.dll");
#else
#ifdef Q_OS_SYMBIAN
const QString KCnHwrPluginName("HbChineseHwr.qtplugin");
#else
const QString KCnHwrPluginName("HbChineseHwr.dll");
#endif
#endif

#ifdef Q_OS_WIN
const QString KCnVItutPluginName("HbChineseVItutd.dll");
#else
#ifdef Q_OS_SYMBIAN
const QString KCnVItutPluginName("HbChineseVItut.qtplugin");
#else
const QString KCnVItutPluginName("HbChineseVItut.dll");
#endif
#endif

#ifdef Q_OS_WIN
const QString KCnVkbPluginName("HbChineseVkbd.dll");
#else
#ifdef Q_OS_SYMBIAN
const QString KCnVkbPluginName("HbChineseVkb.qtplugin");
#else
const QString KCnVkbPluginName("HbChineseVkb.dll");
#endif
#endif

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
    int inputModeToIndex(const int &inputMode, const QList<int> &inputModeList);
    int indexToInputmode(int index, const QList<int> &inputModeList);
    void createChineseSettingGroup(HbDataFormModel *model);

    QInputContextPlugin *pluginInstance(const QString &pluginFileName) const;
    HbInputMethodDescriptor findInputMethodDescriptor(const QString &inputMethodString);
    void setInputMethodVar(Qt::Orientation orientation, QString &inputMethodString, QByteArray &num);
    QByteArray createHwrSpeedData(QByteArray preferredCustomData, int index);
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

    // member variables for chinese
    HbDataFormModelItem *mLanguageGroup;
    HbDataFormModelItem *mChineseInputGroup;
    HbDataFormModelItem *mPortraitInputMethodItem;
    HbDataFormModelItem *mLandscapeInputMethodItem;
    HbDataFormModelItem *mHwrSpeedItem;
    HbDataFormModelItem *mCangjieItem;
    int mCnPortraitInputMode;
    int mCnLandscapeInputMode;
    int mCnCangjieInputMode;

    int mHwrSpeed;
    QList<int> mCnPortraitInputModeList;
    QList<int> mCnLandscapeInputModeList;
    QList<int> mCangjieInputModeList;
    QStringList mCnPortraitInputModeNames;
    QStringList mCnLandscapeInputModeNames;
    QStringList mCnCangjieInputModeNames;
    QStringList mHwrSpeedNames;
};

/*!
Constructs setting widget
*/
HbInputSettingWidgetPrivate::HbInputSettingWidgetPrivate(HbDataForm *dataForm)
    : mForm(dataForm),
      mPrimaryLanguageItem(0),
      mSecondaryLanguageItem(0),
      mKeypressTimeoutItem(0),
      mCharacterPreviewItem(0),
      mPredictionItem(0),
      mAutoCompletionItem(0),
      mCorrectionLevelItem(0),
      mPrimaryCandidateItem(0),
      q_ptr(0),
      mModel(0),
      mLanguageGroup(0),
      mChineseInputGroup(0),
      mPortraitInputMethodItem(0),
      mLandscapeInputMethodItem(0),
      mHwrSpeedItem(0),
      mCangjieItem(0)
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

void HbInputSettingWidgetPrivate::createChineseSettingGroup(HbDataFormModel *model)
{
    Q_Q(HbInputSettingWidget);
    int imMode = KChineseInputModeNone;
    QByteArray ba = HbInputSettingProxy::instance()->preferredInputMethodCustomData(Qt::Vertical);
    QString portraitCustomData(ba);
    QString imName = portraitCustomData.split(" ").at(0);
    HbInputLanguage lang = HbInputSettingProxy::instance()->globalInputLanguage();

    if (imName == KPinyinName) {
        imMode = KPinyinMode;
    } else if (imName == KStrokeName) {
        imMode = KStrokeMode;
    } else if (imName == KZhuyinName) {
        imMode = KZhuyinMode;
    } else if (imName == KHwrName) {
        imMode = KHwrMode;
    } else {
        if (lang.variant() == QLocale::China) {
            imMode = KPinyinMode;
        } else if (lang.variant() == QLocale::HongKong) {
            imMode = KStrokeMode;
        } else if (lang.variant() == QLocale::Taiwan) {
            imMode = KZhuyinMode;
        }
    }

    mCnPortraitInputMode = imMode;

    ba = HbInputSettingProxy::instance()->preferredInputMethodCustomData(Qt::Horizontal);
    QString landscapeCustomData(ba);
    imName = landscapeCustomData.split(" ").at(0);

    mCnCangjieInputMode = KCangjieNormalMode;
    if (imName == KPinyinName) {
        mCnLandscapeInputMode = KPinyinMode;
    } else if (imName == KStrokeName) {
        mCnLandscapeInputMode = KStrokeMode;
    } else if (imName == KZhuyinName) {
        mCnLandscapeInputMode = KZhuyinMode;
    } else if (imName == KCangjieNormalName) {
        mCnLandscapeInputMode = KCangjieNormalMode;
        mCnCangjieInputMode = KCangjieNormalMode;
    } else if (imName == KCangjieEasyName) {
        mCnLandscapeInputMode = KCangjieNormalMode;
        mCnCangjieInputMode = KCangjieEasyMode;
    } else if (imName == KCangjieAdvancedName) {
        mCnLandscapeInputMode = KCangjieNormalMode;
        mCnCangjieInputMode = KCangjieAdvancedMode;
    } else if (imName == KHwrName) {
        mCnLandscapeInputMode = KHwrMode;
    } else {
        if (lang.variant() == QLocale::China) {
            mCnLandscapeInputMode = KPinyinMode;
        } else if (lang.variant() == QLocale::HongKong) {
            mCnLandscapeInputMode = KStrokeMode;
        } else if (lang.variant() == QLocale::Taiwan) {
            mCnLandscapeInputMode = KZhuyinMode;
        }
    }

    Qt::Orientation orientation = HbInputSettingProxy::instance()->screenOrientation();
    ba = HbInputSettingProxy::instance()->preferredInputMethodCustomData(orientation);
    QString customData(ba);
    if (customData.split(" ").count() > 1) {
        imName = customData.split(" ").at(1);
        if (imName == KHwrVerySlowName) {
            mHwrSpeed = 0;
        } else if (imName == KHwrSlowName) {
            mHwrSpeed = 1;
        } else if (imName == KHwrNormalName) {
            mHwrSpeed = 2;
        } else if (imName == KHwrFastName) {
            mHwrSpeed = 3;
        } else if (imName == KHwrVeryFastName) {
            mHwrSpeed = 4;
        } else {
            mHwrSpeed = 2;
        }
    } else {
        mHwrSpeed = 2;
    }

    mHwrSpeedNames.clear();
    mCnPortraitInputModeList.clear();
    mCnLandscapeInputModeList.clear();
    mCnPortraitInputModeNames.clear();
    mCnLandscapeInputModeNames.clear();
    mCnCangjieInputModeNames.clear();

    mHwrSpeedNames << "Very slow" << "Slow" << "Normal" << "Fast" << "Very Fast";
    if (mPrimaryInputLanguage == HbInputLanguage(QLocale::Chinese, QLocale::China)) {
        mCnPortraitInputModeList << KPinyinMode << KStrokeMode << KHwrMode;
        mCnLandscapeInputModeList << KPinyinMode << KStrokeMode << KHwrMode;
        mCnPortraitInputModeNames << KPinyinName << KStrokeName << KHwrName;
        mCnLandscapeInputModeNames << KPinyinName << KStrokeName << KHwrName;
    } else if (mPrimaryInputLanguage == HbInputLanguage(QLocale::Chinese, QLocale::HongKong)) {
        mCnPortraitInputModeList << KStrokeMode << KHwrMode;
        mCnLandscapeInputModeList << KStrokeMode << KCangjieNormalMode << KHwrMode;
        mCangjieInputModeList << KCangjieEasyMode << KCangjieNormalMode << KCangjieAdvancedMode;
        mCnPortraitInputModeNames << KStrokeName << KHwrName;
        mCnLandscapeInputModeNames << KStrokeName << KCangjieGeneralName << KHwrName;
        mCnCangjieInputModeNames << KEasy << KNormal << KAdvanced;
    } else if (mPrimaryInputLanguage == HbInputLanguage(QLocale::Chinese, QLocale::Taiwan)) {
        mCnPortraitInputModeList << KZhuyinMode << KHwrMode;
        mCnLandscapeInputModeList << KZhuyinMode << KHwrMode;
        mCnPortraitInputModeNames << KZhuyinName << KHwrName;
        mCnLandscapeInputModeNames << KZhuyinName << KHwrName;
    }

    if (!mChineseInputGroup) {
        mChineseInputGroup = model->appendDataFormGroup(QObject::tr("Chinese Input"));
        mPortraitInputMethodItem = new HbDataFormModelItem(HbDataFormModelItem::ComboBoxItem, QObject::tr("Portrait mode input method"));
        mChineseInputGroup->appendChild(mPortraitInputMethodItem);
        mPortraitInputMethodItem->setContentWidgetData(QString("items"), mCnPortraitInputModeNames);
        int imIdx = inputModeToIndex(mCnPortraitInputMode, mCnPortraitInputModeList);
        QVariant varPor;
        varPor.setValue(imIdx);
        mPortraitInputMethodItem->setContentWidgetData(QString("currentIndex"), varPor);
        mPortraitInputMethodItem->setContentWidgetData(QString("objectName"), QString("portrait_input_method"));
        mForm->addConnection(mPortraitInputMethodItem, SIGNAL(currentIndexChanged(int)), q, SLOT(setPortraitInputMethod(int)));

        mLandscapeInputMethodItem = new HbDataFormModelItem(HbDataFormModelItem::ComboBoxItem, QObject::tr("Landscape mode input method"));
        mChineseInputGroup->appendChild(mLandscapeInputMethodItem);
        mLandscapeInputMethodItem->setContentWidgetData(QString("items"), mCnLandscapeInputModeNames);
        int imIdx_lan = inputModeToIndex(mCnLandscapeInputMode, mCnLandscapeInputModeList);
        QVariant varLan;
        varLan.setValue(imIdx_lan);
        mLandscapeInputMethodItem->setContentWidgetData(QString("currentIndex"), varLan);
        mLandscapeInputMethodItem->setContentWidgetData(QString("objectName"), QString("landscape_input_method"));
        mForm->addConnection(mLandscapeInputMethodItem, SIGNAL(currentIndexChanged(int)), q, SLOT(setLandscapeInputMethod(int)));

        mHwrSpeedItem = new HbDataFormModelItem(HbDataFormModelItem::ComboBoxItem, QObject::tr("Handwriting speed"));
        mChineseInputGroup->appendChild(mHwrSpeedItem);
        mHwrSpeedItem->setContentWidgetData(QString("items"), mHwrSpeedNames);
        int hwr = mHwrSpeed;
        mHwrSpeedItem->setContentWidgetData(QString("currentIndex"), hwr);
        mHwrSpeedItem->setContentWidgetData(QString("objectName"), QString("handwriting_speed"));
        mForm->addConnection(mHwrSpeedItem, SIGNAL(currentIndexChanged(int)), q, SLOT(setHwrSpeed(int)));
    }

    if (mPrimaryInputLanguage == HbInputLanguage(QLocale::Chinese, QLocale::HongKong)) {
        mCangjieItem = new HbDataFormModelItem(HbDataFormModelItem::ComboBoxItem, QObject::tr("Cangjie mode"));
        mChineseInputGroup->appendChild(mCangjieItem);
        mCangjieItem->setContentWidgetData(QString("items"), mCnCangjieInputModeNames);
        int cangjieIdx = inputModeToIndex(mCnCangjieInputMode, mCangjieInputModeList);
        QVariant varCang;
        varCang.setValue(cangjieIdx);
        mCangjieItem->setContentWidgetData(QString("currentIndex"), varCang);
        mCangjieItem->setContentWidgetData(QString("objectName"), QString("cangjie_mode"));
        mForm->addConnection(mCangjieItem, SIGNAL(currentIndexChanged(int)), q, SLOT(setCangjieMode(int)));
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
    QList<HbAbstractViewItem *> prototypes = mForm->itemPrototypes();
    prototypes.append(customPrototype);
    mForm->setItemPrototypes(prototypes);

    mLanguageGroup = mModel->appendDataFormGroup(QObject::tr("Language"));

    mPrimaryLanguageItem = new HbDataFormModelItem(HbDataFormModelItem::ComboBoxItem, QObject::tr("Primary Writing language"));
    mLanguageGroup->appendChild(mPrimaryLanguageItem);
    QStringList writingLanguageItems;
    fillLanguageList(writingLanguageItems, mPrimaryLanguages);
    mPrimaryLanguageItem->setContentWidgetData(QString("items"), writingLanguageItems);
    mPrimaryLanguageItem->setContentWidgetData(QString("currentIndex"), languageToIndex(mPrimaryInputLanguage, mPrimaryLanguages));
    mPrimaryLanguageItem->setContentWidgetData(QString("objectName"), QString("primary_writing_language"));
    mForm->addConnection(mPrimaryLanguageItem, SIGNAL(currentIndexChanged(int)), q, SLOT(setPrimaryLanguage(int)));

    if (mPrimaryInputLanguage.language() != QLocale::Chinese) {
        mSecondaryLanguageItem = new HbDataFormModelItem(HbDataFormModelItem::ComboBoxItem, QObject::tr("Secondary Writing language"));
        mLanguageGroup->appendChild(mSecondaryLanguageItem);
        QStringList secondaryLanguageItems;
        fillLanguageList(secondaryLanguageItems, mSecondaryLanguages, QObject::tr("None"));
        mSecondaryLanguageItem->setContentWidgetData(QString("items"), secondaryLanguageItems);
        mSecondaryLanguageItem->setContentWidgetData(QString("currentIndex"), languageToIndex(mSecondaryInputLanguage, mSecondaryLanguages));
        mSecondaryLanguageItem->setContentWidgetData(QString("objectName"), QString("secondary_writing_language"));
        mForm->addConnection(mSecondaryLanguageItem, SIGNAL(currentIndexChanged(int)), q, SLOT(setSecondaryLanguage(int)));
    }
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
    correctionLevels << QObject::tr("Off") << QObject::tr("Medium") << QObject::tr("High");
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

    if (mPrimaryInputLanguage.language() == QLocale::Chinese) {
        createChineseSettingGroup(mModel);
    }
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

/*!
Returns index of the given inputmode at the inputmode list
*/
int HbInputSettingWidgetPrivate::inputModeToIndex(const int &inputMode, const QList<int> &inputModeList)
{
    for (int i = 0; i < inputModeList.count(); ++i) {
        if (inputModeList.at(i) == inputMode) {
            return i;
        }
    }
    return -1;
}

/*!
Returns inputmode in the given index at the inputmode list
*/
int HbInputSettingWidgetPrivate::indexToInputmode(int index, const QList<int> &inputModeList)
{
    if (index >= 0 && index < inputModeList.count()) {
        return inputModeList.at(index);
    } else {
        return KChineseInputModeNone;
    }
}

QInputContextPlugin *HbInputSettingWidgetPrivate::pluginInstance(const QString &pluginFileName) const
{
    if (QLibrary::isLibrary(pluginFileName)) {
        QPluginLoader loader(pluginFileName);
        QObject *plugin = loader.instance();
        if (plugin) {
            return qobject_cast<QInputContextPlugin *>(plugin);
        }
    }

    return 0;
}

HbInputMethodDescriptor HbInputSettingWidgetPrivate::findInputMethodDescriptor(const QString &inputMethodString)
{
    HbInputMethodDescriptor descriptor;
    // Query plugin paths and scan the folders.
    QStringList folders = HbInputSettingProxy::instance()->inputMethodPluginPaths();
    foreach(const QString &folder, folders) {
        QDir dir(folder);
        for (unsigned int i = 0; i < dir.count(); i++) {
            QString path = QString(dir.absolutePath());
            if (path.right(1) != "\\" && path.right(1) != "/") {
                path += QDir::separator();
            }
            path += inputMethodString;
            QInputContextPlugin *inputContextPlugin = pluginInstance(path);
            if (inputContextPlugin) {
                descriptor.setPluginNameAndPath(dir.absolutePath() + QDir::separator() + inputMethodString);

                // For each found plugin, check if there is already a list item for it.
                // If not, then add one.
                QStringList contextKeys = inputContextPlugin->keys();
                foreach(QString key, contextKeys) {
                    descriptor.setKey(key);
                    descriptor.setDisplayName(inputContextPlugin->displayName(key));
                }
                break;
            }
        }
    }
    return descriptor;
}

void HbInputSettingWidgetPrivate::setInputMethodVar(Qt::Orientation orientation, QString &inputMethodString, QByteArray &customData)
{
    int inputMode = KChineseInputModeNone;
    if (orientation == Qt::Vertical) {
        inputMode = mCnPortraitInputMode;
    } else if (orientation == Qt::Horizontal) {
        inputMode = mCnLandscapeInputMode;
    } else {
        return;
    }

    switch (inputMode) {
    case KPinyinMode: {
        orientation == Qt::Vertical ? inputMethodString = KCnVItutPluginName : inputMethodString = KCnVkbPluginName;
        customData.append(KPinyinName.toLatin1().data());
    }
    break;
    case KStrokeMode: {
        orientation == Qt::Vertical ? inputMethodString = KCnVItutPluginName : inputMethodString = KCnVkbPluginName;
        customData.append(KStrokeName.toLatin1().data());
    }
    break;
    case KZhuyinMode: {
        orientation == Qt::Vertical ? inputMethodString = KCnVItutPluginName : inputMethodString = KCnVkbPluginName;
        customData.append(KZhuyinName.toLatin1().data());
    }
    break;
    case KHwrMode: {
        inputMethodString = KCnHwrPluginName;
        customData.append(KHwrName.toLatin1().data());
    }
    break;
    case KCangjieNormalMode: {
        inputMethodString = KCnVkbPluginName;
        customData.append(KCangjieNormalName.toLatin1().data());
    }
    break;
    case KCangjieEasyMode: {
        inputMethodString = KCnVkbPluginName;
        customData.append(KCangjieEasyName.toLatin1().data());
    }
    break;
    case KCangjieAdvancedMode: {
        inputMethodString = KCnVkbPluginName;
        customData.append(KCangjieAdvancedName.toLatin1().data());
    }
    break;
    default:
        break;
    }

    QByteArray preferredCustomData = HbInputSettingProxy::instance()->preferredInputMethodCustomData(orientation);
    QString imName(preferredCustomData);
    QStringList temp = imName.split(" ");
    if (temp.count() > 1) {
        customData.append(" ");
        customData.append(temp.at(1).toLatin1().data());	
    } else {
        customData.append(" ");
        customData.append(KHwrNormalName.toLatin1().data());
    }
    customData.append((char)0);
    return;
}

QByteArray HbInputSettingWidgetPrivate::createHwrSpeedData(QByteArray preferredCustomData, int index)
{
    QString imName(preferredCustomData);
    QStringList temp = imName.split(" ");

    QByteArray customData;
    customData.append(temp.at(0).toLatin1().data());
    customData.append(" ");
    switch (index) {
    case 0:
        customData.append(KHwrVerySlowName.toLatin1().data());
        break;
    case 1:
        customData.append(KHwrSlowName.toLatin1().data());
        break;
    case 2:
        customData.append(KHwrNormalName.toLatin1().data());
        break;
    case 3:
        customData.append(KHwrFastName.toLatin1().data());
        break;
    case 4:
        customData.append(KHwrVeryFastName.toLatin1().data());
        break;
    default:
        break;
    }
    customData.append((char)0);
    return customData;
}

/// @endcond

/*!
Constructs input setting widget
*/
HbInputSettingWidget::HbInputSettingWidget(HbDataForm *dataForm, QGraphicsWidget *parent)
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
    if (oldPLangSupportsPrediction != langSupportsPrediction) {
        if (settings->predictiveInputStatus(HbKeyboardSetting12key) != langSupportsPrediction) {
            settings->setPredictiveInputStatus(HbKeyboardSetting12key, langSupportsPrediction);
        }
        if (settings->predictiveInputStatus(HbKeyboardSettingQwerty) != langSupportsPrediction) {
            settings->setPredictiveInputStatus(HbKeyboardSettingQwerty, langSupportsPrediction);
        }
    }

    if (d->mPrimaryInputLanguage.language() != QLocale::Chinese) {
        HbInputLanguage secondaryLanguage = d->mSecondaryInputLanguage;
        // Update secondary language list
        d->createSecondaryLanguageList();
        QStringList secondaryLanguageItems;
        d->fillLanguageList(secondaryLanguageItems, d->mSecondaryLanguages, tr("None"));
        if (d->mSecondaryLanguageItem) {
            d->mSecondaryLanguageItem->setContentWidgetData(QString("items"), secondaryLanguageItems);
            if (d->mPrimaryInputLanguage != secondaryLanguage) {
                d->mSecondaryLanguageItem->setContentWidgetData(QString("currentIndex"), d->languageToIndex(secondaryLanguage, d->mSecondaryLanguages));
            }
        } else {
            d->mSecondaryLanguageItem = new HbDataFormModelItem(HbDataFormModelItem::ComboBoxItem, QObject::tr("Secondary Writing language"));
            d->mLanguageGroup->appendChild(d->mSecondaryLanguageItem);
            d->mSecondaryLanguageItem->setContentWidgetData(QString("items"), secondaryLanguageItems);
            d->mSecondaryLanguageItem->setContentWidgetData(QString("currentIndex"), d->languageToIndex(secondaryLanguage, d->mSecondaryLanguages));
            d->mSecondaryLanguageItem->setContentWidgetData(QString("objectName"), QString("secondary_writing_language"));
            d->mForm->addConnection(d->mSecondaryLanguageItem, SIGNAL(currentIndexChanged(int)), this, SLOT(setSecondaryLanguage(int)));
        }

        HbDataFormModel *model = qobject_cast<HbDataFormModel *>(d->mForm->model());
        if (d->mChineseInputGroup) {
            model->removeItem(d->mChineseInputGroup);
            d->mChineseInputGroup = NULL;
        }
    } else {
        HbDataFormModel *model = qobject_cast<HbDataFormModel *>(d->mForm->model());
        if (d->mChineseInputGroup) {
            model->removeItem(d->mChineseInputGroup);
            d->mChineseInputGroup = NULL;
        }

        if (d->mSecondaryLanguageItem) {
            model->removeItem(d->mSecondaryLanguageItem);
            d->mSecondaryLanguageItem = NULL;
        }

        resetChineseInputMode();
        d->createChineseSettingGroup(model);
        d->mForm->setModel(model);
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

/*!
Saves the portrait input method
*/
void HbInputSettingWidget::setPortraitInputMethod(int index)
{
    Q_D(HbInputSettingWidget);
    d->mCnPortraitInputMode = d->indexToInputmode(index, d->mCnPortraitInputModeList);
    QString inputMethodString;
    QByteArray customData;
    d->setInputMethodVar(Qt::Vertical, inputMethodString, customData);

    const HbInputMethodDescriptor descriptor = d->findInputMethodDescriptor(inputMethodString);
    HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Vertical, (const HbInputMethodDescriptor &)descriptor, customData);
}

/*!
Saves the landscape input method
*/
void HbInputSettingWidget::setLandscapeInputMethod(int index)
{
    Q_D(HbInputSettingWidget);
    d->mCnLandscapeInputMode = d->indexToInputmode(index, d->mCnLandscapeInputModeList);
    QString inputMethodString;
    QByteArray customData;
    d->setInputMethodVar(Qt::Horizontal, inputMethodString, customData);

    const HbInputMethodDescriptor descriptor = d->findInputMethodDescriptor(inputMethodString);
    HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Horizontal, descriptor, customData);
}

/*!
Saves the cangjie input mode
*/
void HbInputSettingWidget::setCangjieMode(int index)
{
    Q_D(HbInputSettingWidget);
    d->mCnCangjieInputMode = d->indexToInputmode(index, d->mCangjieInputModeList);
    d->mCnLandscapeInputMode = d->mCnCangjieInputMode;

    QString inputMethodString;
    QByteArray customData;
    d->setInputMethodVar(Qt::Horizontal, inputMethodString, customData);

    const HbInputMethodDescriptor descriptor = d->findInputMethodDescriptor(inputMethodString);
    HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Horizontal, descriptor, customData);
}


/*!
Saves the handwriting speed
*/
void HbInputSettingWidget::setHwrSpeed(int index)
{
    Q_D(HbInputSettingWidget);
    Qt::Orientation currentOrientation = HbInputSettingProxy::instance()->screenOrientation();
    QByteArray portraitCustomData = HbInputSettingProxy::instance()->preferredInputMethodCustomData(Qt::Vertical);
    QByteArray landscapeCustomData = HbInputSettingProxy::instance()->preferredInputMethodCustomData(Qt::Horizontal);
    HbInputMethodDescriptor portraitDes = HbInputSettingProxy::instance()->preferredInputMethod(Qt::Vertical);
    HbInputMethodDescriptor landscapeDes = HbInputSettingProxy::instance()->preferredInputMethod(Qt::Horizontal);
    QByteArray portraitHwrspeed = d->createHwrSpeedData(portraitCustomData, index);
    QByteArray landscapeHwrspeed = d->createHwrSpeedData(landscapeCustomData, index);

    if (currentOrientation == Qt::Vertical) {
        HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Horizontal, landscapeDes, landscapeHwrspeed);
        HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Vertical, portraitDes, portraitHwrspeed);

    } if (currentOrientation == Qt::Horizontal) {
        HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Vertical, portraitDes, portraitHwrspeed);
        HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Horizontal, landscapeDes, landscapeHwrspeed);
    }
}

/*!
Saves the portrait input method
*/
void HbInputSettingWidget::resetChineseInputMode()
{
    Q_D(HbInputSettingWidget);
    HbInputLanguage lang = HbInputSettingProxy::instance()->globalInputLanguage();

    if (lang.variant() == QLocale::China) {
        d->mCnPortraitInputMode = KPinyinMode;
        d->mCnLandscapeInputMode = KPinyinMode;
    } else if (lang.variant() == QLocale::HongKong) {
        d->mCnPortraitInputMode = KStrokeMode;
        d->mCnLandscapeInputMode = KStrokeMode;
    } else if (lang.variant() == QLocale::Taiwan) {
        d->mCnPortraitInputMode = KZhuyinMode;
        d->mCnLandscapeInputMode = KZhuyinMode;
    }

    QString portraitInputMethodString;
    QString landscapeInputMethodString;
    QByteArray portraitCustomData;
    QByteArray landscapeCustomData;
    d->setInputMethodVar(Qt::Vertical, portraitInputMethodString, portraitCustomData);
    d->setInputMethodVar(Qt::Horizontal, landscapeInputMethodString, landscapeCustomData);

    const HbInputMethodDescriptor portraitDescriptor = d->findInputMethodDescriptor(portraitInputMethodString);
    const HbInputMethodDescriptor landscapeDescriptor = d->findInputMethodDescriptor(landscapeInputMethodString);
    HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Vertical, (const HbInputMethodDescriptor &)portraitDescriptor, portraitCustomData);
    HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Horizontal, (const HbInputMethodDescriptor &)landscapeDescriptor, landscapeCustomData);
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
