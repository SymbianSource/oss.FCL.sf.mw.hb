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
#include <hbinputcontextplugin.h>
#include <hbinputmodeproperties.h>

#include "hbinputcheckboxlist_p.h"
#include "hbinputsettingproxy_p.h"
#include <hbinputmethoddescriptor.h>
#include <hbinputmethod.h>
#include <hbmainwindow.h>

/*!
@stable
@hbinput
\class HbInputSettingWidget
\brief Displays settings for Hb inputs.

This is a input settings widget to be embedded in e.g. control panel view or input settings popup.
*/

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
const QString KHwrVerySlowName("Very Slow");
const QString KHwrSlowName("Slow");
const QString KHwrNormalName("Normal");
const QString KHwrFastName("Fast");
const QString KHwrVeryFastName("Very Fast");

// strings used for default language
const QString KDefaultChineseName("Chinese");
const QString KDefaultEnglishName("English");

// hwr speed index are exactly saming as enum HbHwrWritingSpeed
const int KHwrSpeedCount = 5;

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

class HbCnInputModeMap
{
public:
    int mMode;
    QString mModeName;
};

const HbCnInputModeMap modesMap[] = {
    {KPinyinMode, KPinyinName},
    {KStrokeMode, KStrokeName},
    {KZhuyinMode, KZhuyinName},
    {KCangjieNormalMode, KCangjieNormalName},
    {KCangjieEasyMode, KCangjieEasyName},
    {KCangjieAdvancedMode, KCangjieAdvancedName},
    {KHwrMode, KHwrName}
};

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
    // following API used by chinese
    int inputModeToIndex(const int &inputMode, const QList<int> &inputModeList);
    int indexToInputmode(int index, const QList<int> &inputModeList);
    void createChineseSettingGroup(HbDataFormModel *model);
    void createValidModesList(QStringList &imModeNames, QList<int> &imModeList);
    QString inputModeName(int mode) const;
    int inputModeByGivenName(const QString& imName) const;
    int defaultModeByGivenLang(const HbInputLanguage &lang) const;

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

    HbHwrWritingSpeed  mHwrSpeed;
    QList<int> mCnPortraitInputModeList;
    QList<int> mCnLandscapeInputModeList;
    QList<int> mCangjieInputModeList;
    QStringList mCnPortraitInputModeNames;
    QStringList mCnLandscapeInputModeNames;
    QStringList mCnCangjieInputModeNames;
    QStringList mHwrSpeedNames;
    QStringList mCnDefaultLanguageNames;
    HbDataFormModelItem *mCnDefaultLanguageItem;
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
      mCangjieItem(0),      
      mCnCangjieInputMode(KCangjieNormalMode),
      mCnDefaultLanguageItem(0)

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
        QModelIndex index = mModel->indexFromItem(mSecondaryLanguageItem ? mSecondaryLanguageItem->parent(): mPrimaryLanguageItem->parent());
        mForm->setExpanded(index, false);
        index = mModel->indexFromItem(mKeypressTimeoutItem->parent());
        mForm->setExpanded(index, false);
        index = mModel->indexFromItem(mPredictionItem->parent());
        mForm->setExpanded(index, false);        
    }
}

void HbInputSettingWidgetPrivate::createValidModesList(QStringList &imModeNames, QList<int> &imModeList)
{    
    Qt::Orientation orientation = Qt::Horizontal;
    if (mForm) {
       orientation = mForm->mainWindow()->orientation();
    }

    QList<HbInputMethodDescriptor> methodList = HbInputMethod::listCustomInputMethods(orientation,
        HbInputSettingProxy::instance()->globalInputLanguage());
    methodList.insert(0, HbInputMethod::defaultInputMethod(orientation));

    foreach(const HbInputMethodDescriptor &des, methodList) {
        QStringList displayNames = des.displayNames();
        if (!displayNames.isEmpty()) {
            imModeNames += displayNames;
        } else {
            QString displayName = des.displayName();
            imModeNames.append(displayName);
        }
    }

    imModeNames.removeDuplicates();
    // filter out the input mode that not valid to current screen orientation
    if (imModeNames.contains(KCangjieGeneralName)) {
        imModeNames.removeOne(KCangjieGeneralName);
    }

    for (int i = 0; i < imModeNames.count(); ++i) {
        imModeList.append(inputModeByGivenName(imModeNames.at(i)));
    }
}

QString HbInputSettingWidgetPrivate::inputModeName(int mode) const
{
    int cnImCnts = sizeof(modesMap) / sizeof(modesMap[0]);
    if (mode >= 0 && mode < cnImCnts) {
        return modesMap[mode].mModeName;
    }

    return QString();
}

int HbInputSettingWidgetPrivate::inputModeByGivenName(const QString& imName) const
{
    int cnMode = KChineseInputModeNone;

    if (imName == KCangjieGeneralName) {
        HbCangjieDetailMode cjDetail = HbInputSettingProxy::instance()->detailedCangjieMode();
        switch (cjDetail) {
        case HbCangjieEasy: {
            cnMode = KCangjieEasyMode;
        }
            break;
        case HbCangjieAdvanced: {
            cnMode = KCangjieAdvancedMode;
        }
            break;
        case HbCangjieNormal: 
        default: {
            cnMode = KCangjieNormalMode;
        }
            break;
        }
    } else {
        int cnImCnts = sizeof(modesMap) / sizeof(modesMap[0]);
        for (int i = 0; i < cnImCnts; ++i) {
            if (modesMap[i].mModeName == imName) {
                cnMode = modesMap[i].mMode;
            }
        }
    }

    if (KChineseInputModeNone == cnMode) {
        HbInputLanguage lang = HbInputSettingProxy::instance()->globalInputLanguage();
        cnMode = defaultModeByGivenLang(lang);
    }
    
    return cnMode;
}

int HbInputSettingWidgetPrivate::defaultModeByGivenLang(const HbInputLanguage &lang) const
{
    int imMode = KChineseInputModeNone;
    if (lang.variant() == QLocale::China) {
        imMode = KPinyinMode;
    } else if (lang.variant() == QLocale::HongKong) {
        imMode = KStrokeMode;
    } else if (lang.variant() == QLocale::Taiwan) {
        imMode = KZhuyinMode;
    }

    return imMode;
}

void HbInputSettingWidgetPrivate::createChineseSettingGroup(HbDataFormModel *model)
{
    Q_Q(HbInputSettingWidget);
    QByteArray ba = HbInputSettingProxy::instance()->preferredInputMethodCustomData(Qt::Vertical);
    QString imName(ba);
    HbInputLanguage lang = HbInputSettingProxy::instance()->globalInputLanguage();
    mCnPortraitInputMode = inputModeByGivenName(imName);

    ba = HbInputSettingProxy::instance()->preferredInputMethodCustomData(Qt::Horizontal);
    imName = QString(ba);
    mCnLandscapeInputMode = inputModeByGivenName(imName);

    mHwrSpeedNames.clear();
    mCnPortraitInputModeList.clear();
    mCnLandscapeInputModeList.clear();
    mCnPortraitInputModeNames.clear();
    mCnLandscapeInputModeNames.clear();
    mCnCangjieInputModeNames.clear();
    mCnDefaultLanguageNames.clear();

    createValidModesList(mCnPortraitInputModeNames, mCnPortraitInputModeList);
    mCnLandscapeInputModeNames = mCnPortraitInputModeNames;
    mCnLandscapeInputModeList = mCnPortraitInputModeList;

    // append cangjie to landscape related list if need
    if (mPrimaryInputLanguage == HbInputLanguage(QLocale::Chinese, QLocale::HongKong) && 
        !mCnLandscapeInputModeNames.contains(KCangjieGeneralName)) {

        int index = HbInputSettingProxy::instance()->detailedCangjieMode();
        switch(index) {
        case 0:
            mCnCangjieInputMode = KCangjieEasyMode;
            break;
        case 1: 
            mCnCangjieInputMode = KCangjieNormalMode;
            break;
        case 2:
            mCnCangjieInputMode = KCangjieAdvancedMode;
            break;
        default:
            break;
        }

        QStringList tmpInputmodeNames;
        QList<int> tmpInputmode;
        tmpInputmodeNames << mCnLandscapeInputModeNames.at(0) << KCangjieGeneralName << mCnLandscapeInputModeNames.at(1);
        tmpInputmode << mCnLandscapeInputModeList.at(0) << mCnCangjieInputMode << mCnLandscapeInputModeList.at(1);
        mCnLandscapeInputModeList.clear();
        mCnLandscapeInputModeNames.clear();
        mCnLandscapeInputModeList = tmpInputmode;
        mCnLandscapeInputModeNames = tmpInputmodeNames;
        mCangjieInputModeList << KCangjieEasyMode << KCangjieNormalMode << KCangjieAdvancedMode;
        mCnCangjieInputModeNames << KEasy << KNormal << KAdvanced;
    }

    mHwrSpeed = HbInputSettingProxy::instance()->hwrWritingSpeed();
    mHwrSpeedNames << KHwrVerySlowName << KHwrSlowName << KHwrNormalName << KHwrFastName << KHwrVeryFastName;
    mCnDefaultLanguageNames << KDefaultChineseName << KDefaultEnglishName;

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
        mHwrSpeedItem->setContentWidgetData(QString("currentIndex"), mHwrSpeed);
        mHwrSpeedItem->setContentWidgetData(QString("objectName"), QString("handwriting_speed"));
        mForm->addConnection(mHwrSpeedItem, SIGNAL(currentIndexChanged(int)), q, SLOT(setHwrSpeed(int)));

        mCnDefaultLanguageItem = new HbDataFormModelItem(HbDataFormModelItem::ComboBoxItem, QObject::tr("Default language for keyboard input"));
        mChineseInputGroup->appendChild(mCnDefaultLanguageItem);
        int defaultLanguageIndex = HbInputSettingProxy::instance()->useWesternDefaultKeypadForChinese() ? 1 : 0;
        mCnDefaultLanguageItem->setContentWidgetData(QString("items"), mCnDefaultLanguageNames);
        mCnDefaultLanguageItem->setContentWidgetData(QString("currentIndex"), defaultLanguageIndex);
        mCnDefaultLanguageItem->setContentWidgetData(QString("objectName"), QString("default_language_for_keyboard_input"));
        mForm->addConnection(mCnDefaultLanguageItem, SIGNAL(currentIndexChanged(int)), q, SLOT(setDefaultLanguageForKeyboardInput(int)));
    }

    int cangjieIdx = HbInputSettingProxy::instance()->detailedCangjieMode();
    if (mPrimaryInputLanguage == HbInputLanguage(QLocale::Chinese, QLocale::HongKong)) {
        mCangjieItem = new HbDataFormModelItem(HbDataFormModelItem::ComboBoxItem, QObject::tr("Cangjie mode"));
        mChineseInputGroup->appendChild(mCangjieItem);
        mCangjieItem->setContentWidgetData(QString("items"), mCnCangjieInputModeNames);
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

    if (mSecondaryLanguageItem) {
        mSecondaryLanguageItem->setContentWidgetData(QString("currentIndex"), languageToIndex(mSecondaryInputLanguage, mSecondaryLanguages));
    }
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

    if(mChineseInputGroup && mCnDefaultLanguageItem) {
        int defaultLanguageIndex = HbInputSettingProxy::instance()->useWesternDefaultKeypadForChinese() ? 1 : 0;
        mCnDefaultLanguageItem->setContentWidgetData(QString("currentIndex"), defaultLanguageIndex);
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

    // expand language selection
    QModelIndex index = mModel->indexFromItem(mLanguageGroup);
    mForm->setExpanded(index, true);

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
    if (d->mPrimaryInputLanguage.language() == QLocale::Chinese) {
        HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Horizontal, HbInputMethodDescriptor());
        HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Vertical, HbInputMethodDescriptor());        
    }
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
            HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Horizontal, HbInputMethodDescriptor());
            HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Vertical, HbInputMethodDescriptor());
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

        resetChineseInputMode(d->mPrimaryInputLanguage);
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
    QByteArray customData;
    HbInputMethodDescriptor descriptor;

    if (d->mCnPortraitInputMode != KHwrMode) {
        QString imName = d->inputModeName(d->mCnPortraitInputMode);
        customData.append(imName.toLatin1().data());
        customData.append((char)0);
        descriptor = HbInputMethod::defaultInputMethod(Qt::Vertical);
    } else {
        customData.append(KHwrName.toLatin1().data());
        customData.append((char)0);
        QList<HbInputMethodDescriptor> methodList = HbInputMethod::listCustomInputMethods(Qt::Vertical, HbInputSettingProxy::instance()->globalInputLanguage());
        foreach(const HbInputMethodDescriptor &des, methodList) {
            if (des.displayName() == "Chinese Hand Writing Recognition") {
                descriptor = des;
                break;
            }
        }
    }

    Qt::Orientation orientation = Qt::Vertical;
    if (d->mForm) {
        orientation = d->mForm->mainWindow()->orientation();
    }

    HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Vertical, descriptor, customData);
    if (orientation == Qt::Vertical) {
        HbInputMethod::activeInputMethod()->activateInputMethod(descriptor);
    }
}

/*!
Saves the landscape input method
*/
void HbInputSettingWidget::setLandscapeInputMethod(int index)
{
    Q_D(HbInputSettingWidget);
    d->mCnLandscapeInputMode = d->indexToInputmode(index, d->mCnLandscapeInputModeList);
    QByteArray customData;
    HbInputMethodDescriptor descriptor;

    if (d->mCnLandscapeInputMode != KHwrMode) {
        QString imName;
        if (d->mCnLandscapeInputMode == KCangjieNormalMode || 
            d->mCnLandscapeInputMode == KCangjieEasyMode  || 
            d->mCnLandscapeInputMode == KCangjieAdvancedMode) {
            imName = KCangjieGeneralName;
        } else {
            imName = d->inputModeName(d->mCnLandscapeInputMode);
        }

        customData.append(imName.toLatin1().data());
        customData.append((char)0);
        descriptor = HbInputMethod::defaultInputMethod(Qt::Horizontal);
    } else {
        customData.append(KHwrName.toLatin1().data());
        customData.append((char)0);
        QList<HbInputMethodDescriptor> methodList = HbInputMethod::listCustomInputMethods(Qt::Horizontal, HbInputSettingProxy::instance()->globalInputLanguage());
        foreach(const HbInputMethodDescriptor &des, methodList) {
            if (des.displayName() == "Chinese Hand Writing Recognition") {
                descriptor = des;
                break;
            }
        }
    }

    Qt::Orientation orientation = Qt::Horizontal;
    if (d->mForm) {
        orientation = d->mForm->mainWindow()->orientation();
    }

    HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Horizontal, descriptor, customData);
    if (orientation == Qt::Horizontal) {
        HbInputMethod::activeInputMethod()->activateInputMethod(descriptor);
    }
}

/*!
Saves the cangjie input mode
*/
void HbInputSettingWidget::setCangjieMode(int index)
{
    Q_D(HbInputSettingWidget);
    d->mCnCangjieInputMode = d->indexToInputmode(index, d->mCangjieInputModeList);
    d->mCnLandscapeInputMode = d->mCnCangjieInputMode;

    QString imName = KCangjieGeneralName;
    QByteArray customData;
    customData.append(imName.toLatin1().data());
    customData.append((char)0);

    HbInputMethodDescriptor descriptor;
    descriptor = HbInputMethod::defaultInputMethod(Qt::Horizontal);
     
    HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Horizontal, descriptor, customData);
    HbInputSettingProxy::instance()->setDetailedCangjieMode(HbCangjieDetailMode(index));
}

/*!
Saves the handwriting speed
*/
void HbInputSettingWidget::setHwrSpeed(int index)
{
    // hwr speed index in setting widget are exactly same as enum HbHwrWritingSpeed of setting proxy
    if (index >= 0 && index < KHwrSpeedCount) {
        HbInputSettingProxy::instance()->setHwrWritingSpeed(HbHwrWritingSpeed(index));
    }
}

/*!
Saves the portrait input method
*/
void HbInputSettingWidget::resetChineseInputMode(HbInputLanguage lang)
{
    Q_D(HbInputSettingWidget);
    d->mCnPortraitInputMode = d->defaultModeByGivenLang(lang);
    d->mCnLandscapeInputMode = d->defaultModeByGivenLang(lang);
    HbInputSettingProxy::instance()->setDetailedCangjieMode(HbCangjieNormal);

    QByteArray portraitCustomData;
    QByteArray landscapeCustomData;
    QString imName = d->inputModeName(d->mCnPortraitInputMode);
    portraitCustomData.append(imName.toLatin1().data());
    portraitCustomData.append((char)0);
    HbInputMethodDescriptor portraitDescriptor = HbInputMethod::defaultInputMethod(Qt::Vertical);

    if (d->mCnLandscapeInputMode == KCangjieNormalMode || 
        d->mCnLandscapeInputMode == KCangjieEasyMode || 
        d->mCnLandscapeInputMode == KCangjieAdvancedMode) {
        imName = KCangjieGeneralName;
    } else {
        imName = d->inputModeName(d->mCnLandscapeInputMode);
    }

    landscapeCustomData.append(imName.toLatin1().data());
    landscapeCustomData.append((char)0);
    HbInputMethodDescriptor landscapeDescriptor = HbInputMethod::defaultInputMethod(Qt::Horizontal);

    HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Vertical, (const HbInputMethodDescriptor &)portraitDescriptor, portraitCustomData);
    HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Horizontal, (const HbInputMethodDescriptor &)landscapeDescriptor, landscapeCustomData);

    return;
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

/*!
Set default language for keyboard input
*/
void HbInputSettingWidget::setDefaultLanguageForKeyboardInput(int index)
{
    if(index == 1) {
        HbInputSettingProxy::instance()->setWesternDefaultKeypadForChinese(true);
    } else {
        HbInputSettingProxy::instance()->setWesternDefaultKeypadForChinese(false);
    }
}

// End of file
