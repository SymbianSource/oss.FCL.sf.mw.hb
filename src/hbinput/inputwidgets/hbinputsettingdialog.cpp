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
#include <QGraphicsLayout>
#if QT_VERSION >= 0x040600
#include <QGraphicsDropShadowEffect>
#endif

#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>
#include <hbdataformviewitem.h>
#include <hbaction.h>
#include <hbcombobox.h>
#include <hbpushbutton.h>
#include <hbinputpredictionfactory.h>
#include <hbdeviceprofile.h>

#include <hbinputlanguage.h>

#include <hbdialog_p.h>

#include "hbinputsettingproxy.h"
#include "hbinpututils.h"

#include "hbinputsettingdialog.h"

/// @cond

class HbInputSettingDialogPrivate : public HbDialogPrivate
{
public:
    HbInputSettingDialogPrivate();
    ~HbInputSettingDialogPrivate();
    void createSecondaryLanguageList();
    void fillLanguageList(QStringList &list, QList<HbInputLanguage> &languageList, QString replace = QString(" "));
    int languageToIndex(HbInputLanguage &language, QList<HbInputLanguage> &languageList);
    HbInputLanguage indexToLanguage(int index, QList<HbInputLanguage> &languageList);

public:
    HbDataForm *mForm;
    QList<HbInputLanguage> mPrimaryLanguages;
    QList<HbInputLanguage> mSecondaryLanguages;
    HbInputLanguage mPrimaryInputLanguage;
    HbInputLanguage mSecondaryInputLanguage;
    int mPredictionStatus;
    HbComboBox *mPrimaryComboBox;
    HbComboBox *mSecondaryComboBox;
    HbPushButton *mPredictionButton;
};

HbInputSettingDialogPrivate::HbInputSettingDialogPrivate()
 : mPrimaryComboBox(0), mSecondaryComboBox(0), mPredictionButton(0)
{
    mForm = new HbDataForm();

    HbInputSettingProxy *settings = HbInputSettingProxy::instance();
    mPrimaryInputLanguage = settings->globalInputLanguage();
    mSecondaryInputLanguage = settings->globalSecondaryInputLanguage();
    mPredictionStatus = settings->predictiveInputStatus();

    HbInputUtils::listSupportedInputLanguages(mPrimaryLanguages);
    createSecondaryLanguageList();
}

HbInputSettingDialogPrivate::~HbInputSettingDialogPrivate()
{
}

void HbInputSettingDialogPrivate::createSecondaryLanguageList()
{
    mSecondaryLanguages.clear();

    mSecondaryLanguages.append(HbInputLanguage());

    if (mPrimaryInputLanguage.language() != QLocale::Chinese) {
        for (int i = 0; i < mPrimaryLanguages.count(); ++i) {
            if (mPrimaryLanguages.at(i) != mPrimaryInputLanguage &&
                mPrimaryLanguages.at(i).language() != QLocale::Chinese) {
                mSecondaryLanguages.append(mPrimaryLanguages.at(i));
            }
        }
    }
}

void HbInputSettingDialogPrivate::fillLanguageList(QStringList &list, QList<HbInputLanguage> &languageList, QString replace)
{
    for (int i = 0; i < languageList.count(); ++i) {
        QString langName = languageList[i].localisedName();
        if (langName.length() == 0) {
            langName = replace;
        }
        list<<langName;
    }
}

int HbInputSettingDialogPrivate::languageToIndex(HbInputLanguage &language, QList<HbInputLanguage> &languageList)
{
    for (int i = 0; i < languageList.count(); ++i) {
        if (languageList.at(i) == language) {
            return i;
        }
    }
    return -1;
}

HbInputLanguage HbInputSettingDialogPrivate::indexToLanguage(int index, QList<HbInputLanguage> &languageList)
{
    if (index >= 0 && index < languageList.count()) {
        return languageList.at(index);
    } else {
        return HbInputLanguage();
    }
}

/// @endcond

// ---------------------------------------------------------------------------
// HbInputSettingDialog::HbInputSettingDialog
//
// ---------------------------------------------------------------------------
//
HbInputSettingDialog::HbInputSettingDialog(HbSettingItems items, QGraphicsWidget *parent)
    : HbDialog(*new HbInputSettingDialogPrivate, parent)
{
    Q_D(HbInputSettingDialog);

    d->setPriority(HbPopupPrivate::VirtualKeyboard + 1);  // Should be shown on top of virtual keyboard.

    HbAction *ok = new HbAction(tr("Ok"), this);
    ok->setObjectName("Language selection popup ok");
    setPrimaryAction(ok);

    HbAction *cancel = new HbAction(tr("Cancel"), this);
    cancel->setObjectName("Language selection popup cancel");
    setSecondaryAction(cancel);

    connect(ok, SIGNAL(triggered()), this, SLOT(selected()));

    setObjectName("Language selection popup");
    setTimeout(HbPopup::NoTimeout);
    setDismissPolicy(HbPopup::NoDismiss);
    setModal(false);
    setBackgroundFaded(false);

#if QT_VERSION >= 0x040600
    // Make sure the keypad never steals focus.
    setFlag(QGraphicsItem::ItemIsPanel, true);
    setActive(false);

    // enable drop shadow for the preview pane
// Effect deletion is crashing -> Effect temporarily removed
//    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
//    effect->setBlurRadius(8);
//    setGraphicsEffect(effect);
#endif

    HbDataFormModel *model = new HbDataFormModel();
    d->mForm->setModel(model);

    // Create drop down list for writing language selection
    if(items & HbSettingItemWritingLang) {
        HbDataFormModelItem *writingLanguage =
            model->appendDataFormItem(HbDataFormModelItem::ComboBoxItem, tr("Primary Writing language"));
        QStringList writingLanguageItems;
        d->fillLanguageList(writingLanguageItems, d->mPrimaryLanguages);
        writingLanguage->setContentWidgetData(QString("items"),writingLanguageItems);
        writingLanguage->setContentWidgetData(QString("currentIndex"),d->languageToIndex(d->mPrimaryInputLanguage, d->mPrimaryLanguages));
        writingLanguage->setData(HbDataFormModelItem::KeyRole, QString("writing_language"));
        d->mForm->addConnection(writingLanguage, SIGNAL(currentIndexChanged(int)), this, SLOT(primaryLanguageChanged(int)));
    }
    // Create drop down list for secondary language selection
    if(items & HbSettingItemSecondaryLang) {
        HbDataFormModelItem *secondaryLanguage =
            model->appendDataFormItem(HbDataFormModelItem::ComboBoxItem, tr("Secondary Writing language"));
        QStringList secondaryLanguageItems;
        d->fillLanguageList(secondaryLanguageItems, d->mSecondaryLanguages, tr("None"));
        secondaryLanguage->setContentWidgetData(QString("items"),secondaryLanguageItems);
        secondaryLanguage->setContentWidgetData(QString("currentIndex"),d->languageToIndex(d->mSecondaryInputLanguage, d->mSecondaryLanguages));
        secondaryLanguage->setData(HbDataFormModelItem::KeyRole, QString("secondary_language"));
        d->mForm->addConnection(secondaryLanguage, SIGNAL(currentIndexChanged(int)), this, SLOT(secondaryLanguageChanged(int)));
    }
    if(items & HbSettingItemPrediction) {
        QString statusOff = tr("Off");
        QString statusOn = tr("On");

        // Create toggle item for prediction mode
        HbDataFormModelItem *predictionStatus =
            model->appendDataFormItem(HbDataFormModelItem::ToggleValueItem, tr("Prediction"));
        if (d->mPredictionStatus) {
            predictionStatus->setContentWidgetData(QString("text"), statusOn);
            predictionStatus->setContentWidgetData(QString("additionalText"), statusOff);
        } else {
            predictionStatus->setContentWidgetData(QString("text"), statusOff);
            predictionStatus->setContentWidgetData(QString("additionalText"), statusOn);
        }
        predictionStatus->setData(HbDataFormModelItem::KeyRole, QString("prediction_status"));
        d->mForm->addConnection(predictionStatus, SIGNAL(clicked(bool)), this, SLOT(predictionStatusChanged()));
        QModelIndex predIndex = model->indexFromItem(predictionStatus);
        HbDataFormViewItem *item = d->mForm->dataFormViewItem(predIndex); 
        HbWidget *contentWidget = item->dataItemContentWidget();
        d->mPredictionButton = static_cast<HbPushButton*>(contentWidget);
    }

    setContentWidget(d->mForm);
}

// ---------------------------------------------------------------------------
// HbInputSettingDialog::~HbInputSettingDialog
//
// ---------------------------------------------------------------------------
//
HbInputSettingDialog::~HbInputSettingDialog()
{
}

// ---------------------------------------------------------------------------
// HbInputSettingDialog::selected
// Called when new settings are accepted
// ---------------------------------------------------------------------------
//
void HbInputSettingDialog::selected()
{
    Q_D(HbInputSettingDialog);

    HbInputSettingProxy *settings = HbInputSettingProxy::instance();
    HbPredictionFactory *predFactory = HbPredictionFactory::instance();

    if (d->mPrimaryInputLanguage != settings->globalInputLanguage()) {
        settings->setGlobalInputLanguage(d->mPrimaryInputLanguage);
    }

    if (d->mSecondaryInputLanguage != settings->globalSecondaryInputLanguage()) {
        settings->setGlobalSecondaryInputLanguage(d->mSecondaryInputLanguage);
    }
    // the primary language supports prediction if there is a prediction engine for it.
    bool langSupportsPrediction = (predFactory->predictionEngineForLanguage(settings->globalInputLanguage()) != NULL);	
    // if the primary language does not support prediction, set prediction status to false
    if (!langSupportsPrediction) {		
        d->mPredictionStatus = false;
    }
    //  synchronize the prediction status in settings to prediction status set through the settings dialog.	
    if(d->mPredictionStatus != settings->predictiveInputStatus()) {
        settings->setPredictiveInputStatus(d->mPredictionStatus);
    }
}

/*!
\deprecated HbInputSettingDialog::settingItemDisplayed(const QModelIndex&)
  is deprecated and will be removed.
*/
void HbInputSettingDialog::settingItemDisplayed(const QModelIndex &index)
{
    Q_UNUSED(index);
}

// ---------------------------------------------------------------------------
// HbInputSettingDialog::primaryLanguageChanged
// Called when user changes primary language
// ---------------------------------------------------------------------------
//
void HbInputSettingDialog::primaryLanguageChanged(int index)
{
    Q_D(HbInputSettingDialog);

    HbInputSettingProxy *settings = HbInputSettingProxy::instance();
    HbPredictionFactory *predFactory = HbPredictionFactory::instance();
    int oldPLangSupportsPrediction = (predFactory->predictionEngineForLanguage(d->mPrimaryInputLanguage) != NULL);		
    d->mPrimaryInputLanguage = d->indexToLanguage(index, d->mPrimaryLanguages);
    int langSupportsPrediction = (predFactory->predictionEngineForLanguage(d->mPrimaryInputLanguage) != NULL);		
    if( oldPLangSupportsPrediction != langSupportsPrediction ) {
        if(langSupportsPrediction) { // language supports prediction
            // first we need to enable the button then only we can click on it
            if (d->mPredictionButton && settings->predictiveInputStatus() != d->mPredictionStatus) {
                d->mPredictionButton->click();
            } else if (!d->mPredictionButton && settings->predictiveInputStatus() != d->mPredictionStatus) { 
                // for numeric editors we dont have prediction button but we need to change the prediction status while changing language
                d->mPredictionStatus = settings->predictiveInputStatus();
            }
        } else { // language does not supports prediction
            if(d->mPredictionButton && (d->mPredictionStatus)) {
                d->mPredictionButton->click();		
            } else if (!d->mPredictionButton && langSupportsPrediction != d->mPredictionStatus) {
                // for numeric editors we dont have prediction button but we need to change the prediction status
                d->mPredictionStatus = langSupportsPrediction;	
            }
        }
        if(d->mPredictionButton) {
            d->mPredictionButton->setEnabled(langSupportsPrediction);
        }
    }
    if (d->mSecondaryComboBox) {
        HbInputLanguage secondaryLanguage = d->mSecondaryInputLanguage;

        // Update secondary language list
        d->createSecondaryLanguageList();
        QStringList secondaryLanguageItems;
        d->fillLanguageList(secondaryLanguageItems, d->mSecondaryLanguages, tr("None"));
        d->mSecondaryComboBox->setItems(secondaryLanguageItems);

        if (d->mPrimaryInputLanguage != secondaryLanguage) {
            d->mSecondaryComboBox->setCurrentIndex(d->languageToIndex(secondaryLanguage, d->mSecondaryLanguages));
        }
    }
}

// ---------------------------------------------------------------------------
// HbInputSettingDialog::secondaryLanguageChanged
// Called when user changes secondary language
// ---------------------------------------------------------------------------
//
void HbInputSettingDialog::secondaryLanguageChanged(int index)
{
    Q_D(HbInputSettingDialog);

    d->mSecondaryInputLanguage = d->indexToLanguage(index, d->mSecondaryLanguages);
}

// ---------------------------------------------------------------------------
// HbInputSettingDialog::predictionStatusChanged
// Called when user changes prediction status
// ---------------------------------------------------------------------------
//
void HbInputSettingDialog::predictionStatusChanged()
{
    Q_D(HbInputSettingDialog);

    d->mPredictionStatus = (d->mPredictionStatus + 1) % 2;
}

/*!
\reimp
*/
void HbInputSettingDialog::showEvent(QShowEvent *event)
{
    Q_D(HbInputSettingDialog);
    d->mForm->setPreferredWidth(HbDeviceProfile::profile(this).logicalSize().width() * 0.9);
    HbPredictionFactory *predFactory = HbPredictionFactory::instance();
    HbInputSettingProxy *settings = HbInputSettingProxy::instance();
    if (!predFactory->predictionEngineForLanguage(settings->globalInputLanguage())) {
        if(d->mPredictionButton && d->mPredictionButton->isEnabled()) {
            d->mPredictionButton->setEnabled(false);
            d->mPredictionStatus = false;
        }
    } else {
        if (d->mPredictionButton && !d->mPredictionButton->isEnabled()) { 
            d->mPredictionButton->setEnabled(true);
        }
        d->mPredictionStatus = settings->predictiveInputStatus();
    }
    HbDialog::showEvent(event);
}

// End of file
