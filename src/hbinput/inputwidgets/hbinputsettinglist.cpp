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

#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#if QT_VERSION >= 0x040600
#include <QGraphicsDropShadowEffect>
#endif

#include <hblabel.h>
#include <hbpushbutton.h>
#include <hblistwidget.h>
#include <hblistwidgetitem.h>
#include <hbinputmethod.h>
#include <hbinputsettingproxy.h>
#include <hbinpututils.h>
#include <hbinputpredictionfactory.h>
#include "hbinputsettinglist.h"
#include "hbdialog_p.h"

const QString settingsIcon("qtg_mono_settings");
const QString inputMethodIcon("qtg_mono_virtual_input");

/// @cond

class HbInputSettingListPrivate : public HbDialogPrivate
{
public:
    HbInputSettingListPrivate();

    qreal languageNameWidth();

public:
    HbPushButton *mLanguageButton;
    HbPushButton *mPredictionButton;
    HbListWidget *mOptionList;
    HbInputLanguage mPrimaryLanguage;
    HbInputLanguage mSecondaryLanguage;
    QList<QString> mPredictionValues;
};

HbInputSettingListPrivate::HbInputSettingListPrivate()
 : mLanguageButton(0), mPredictionButton(0), mOptionList(0)
{
}

qreal HbInputSettingListPrivate::languageNameWidth()
{
    qreal nameWidth(0);
    QList<HbInputLanguage> languages;
    HbInputUtils::listSupportedInputLanguages(languages);
    QFontMetrics fontMetrics(HbFontSpec(HbFontSpec::Primary).font());

    foreach (HbInputLanguage language, languages) {
        qreal width = fontMetrics.width(language.localisedName());
        if (width > nameWidth) {
            nameWidth = width;
        }
    }

    return nameWidth;
}

/// @endcond

/*!
Constructs input setting list
*/
HbInputSettingList::HbInputSettingList(QGraphicsWidget* parent)
 : HbDialog(*new HbInputSettingListPrivate(), parent)
{
    Q_D(HbInputSettingList);

    QGraphicsLinearLayout *mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    QGraphicsGridLayout *gridLayout = new QGraphicsGridLayout();
    
    HbLabel *languageLabel = new HbLabel(tr("Language"), this);
    languageLabel->setFontSpec(HbFontSpec(HbFontSpec::Primary));
    d->mLanguageButton = new HbPushButton(QString(), this);
    d->mLanguageButton->setFontSpec(HbFontSpec(HbFontSpec::Primary));
    d->mLanguageButton->setObjectName("Language setting button");

    HbLabel *predictionLabel = new HbLabel(tr("Prediction"), this);
    predictionLabel->setFontSpec(HbFontSpec(HbFontSpec::Primary));
    d->mPredictionButton = new HbPushButton(QString(), this);
    d->mPredictionButton->setFontSpec(HbFontSpec(HbFontSpec::Primary));
    d->mPredictionButton->setObjectName("Prediction setting button");

    d->mOptionList = new HbListWidget(this);
    d->mOptionList->setFontSpec(HbFontSpec(HbFontSpec::Primary));
    d->mOptionList->setVerticalScrollBarPolicy(HbScrollArea::ScrollBarAlwaysOff);
    d->mOptionList->setObjectName("Input options list");
    d->mOptionList->addItem(HbIcon(settingsIcon), tr("Input settings"));

    gridLayout->addItem(languageLabel, 0, 0);
    gridLayout->addItem(d->mLanguageButton, 0, 1);
    gridLayout->addItem(predictionLabel, 1, 0);
    gridLayout->addItem(d->mPredictionButton, 1, 1);

    qreal buttonWidth = 30 + d->languageNameWidth();
    gridLayout->setColumnFixedWidth(0, 300 - buttonWidth);
    gridLayout->setColumnFixedWidth(1, buttonWidth);

    qreal buttonHeight = buttonWidth * 0.4;
    gridLayout->setRowFixedHeight(0, buttonHeight);
    gridLayout->setRowFixedHeight(1, buttonHeight);

    mainLayout->addItem(gridLayout);
    mainLayout->addItem(d->mOptionList);
    setLayout(mainLayout);

    d->mPredictionValues.append(tr("Off"));
    d->mPredictionValues.append(tr("On"));

    // set default values for popup
    setTimeout(HbDialog::NoTimeout);
    setBackgroundFaded(false);
    setDismissPolicy(TapOutside);

#if QT_VERSION >= 0x040600
    // Make sure the custom button list never steals focus.
    setFlag(QGraphicsItem::ItemIsPanel, true);
    setActive(false);

    // enable drop shadow for the setting list
// Effect deletion is crashing -> Effect temporarily removed
//    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
//    effect->setBlurRadius(8);
//    setGraphicsEffect(effect);
#endif

    connect(d->mLanguageButton, SIGNAL(clicked(bool)), this, SLOT(languageButtonClicked()));
    connect(d->mPredictionButton, SIGNAL(clicked(bool)), this, SLOT(predictionButtonClicked()));
    connect(d->mOptionList, SIGNAL(activated(HbListWidgetItem*)), this, SLOT(listItemActivated(HbListWidgetItem*)));
    connect(d->mOptionList, SIGNAL(longPressed(HbListWidgetItem*, const QPointF&)), this, SLOT(listItemActivated(HbListWidgetItem*)));

    HbInputSettingProxy *settings = HbInputSettingProxy::instance();
    connect(settings, SIGNAL(globalInputLanguageChanged(const HbInputLanguage &)), this, SLOT(primaryLanguageChanged(const HbInputLanguage &)));
    connect(settings, SIGNAL(globalSecondaryInputLanguageChanged(const HbInputLanguage &)), this, SLOT(secondaryLanguageChanged(const HbInputLanguage &)));
    connect(settings, SIGNAL(predictiveInputStateChanged(HbKeyboardSettingFlags, bool)), this, SLOT(predictionStatusChanged(HbKeyboardSettingFlags, bool)));
}

/*!
Destructs the object.
*/
HbInputSettingList::~HbInputSettingList()
{
}

/*!
\deprecated HbInputSettingList::showSettingList()
    is deprecated. Use updateSettingList and open functions instead.
*/
void HbInputSettingList::showSettingList()
{
}

/*!
Updates settings list with current values
*/
void HbInputSettingList::updateSettingList()
{
    Q_D(HbInputSettingList);

    HbInputSettingProxy *settings = HbInputSettingProxy::instance();
    d->mPrimaryLanguage = settings->globalInputLanguage();
    d->mSecondaryLanguage = settings->globalSecondaryInputLanguage();

    d->mLanguageButton->setText(d->mPrimaryLanguage.localisedName());
    d->mPredictionButton->setText(d->mPredictionValues.at(settings->predictiveInputStatusForActiveKeyboard()));

    QList<HbInputMethodDescriptor> customList = HbInputMethod::listCustomInputMethods();
    bool showInputMethod = true;
    if (customList.count() < 1) {
        showInputMethod = false;
    }

    if (d->mOptionList->count() == 1 && showInputMethod) {
        d->mOptionList->insertItem(0, HbIcon(inputMethodIcon), tr("Input methods"));
    } else if (d->mOptionList->count() == 2 && !showInputMethod) {
        delete d->mOptionList->takeItem(0);
    }
}

/*!
Enables/disables language selection button
*/
void HbInputSettingList::setLanguageSelectionEnabled(bool enabled)
{
    Q_D(HbInputSettingList);

    d->mLanguageButton->setEnabled(enabled);
}

/*!
Enables/disables prediction selection button
*/
void HbInputSettingList::setPredictionSelectionEnabled(bool enabled)
{
    Q_D(HbInputSettingList);

    d->mPredictionButton->setEnabled(enabled);
}

/*!
Swaps current primary and secondary languages
*/
void HbInputSettingList::languageButtonClicked()
{
    Q_D(HbInputSettingList);

    if (d->mSecondaryLanguage == HbInputLanguage()) {
        emit inputSettingsButtonClicked();
    } else {
        HbInputLanguage language = d->mPrimaryLanguage;
        d->mPrimaryLanguage = d->mSecondaryLanguage;
        d->mSecondaryLanguage = language;

        HbInputSettingProxy::instance()->setGlobalInputLanguage(d->mPrimaryLanguage);
        HbInputSettingProxy::instance()->setGlobalSecondaryInputLanguage(d->mSecondaryLanguage);
        
        if (!HbPredictionFactory::instance()->predictionEngineForLanguage(d->mPrimaryLanguage)) {
            HbInputSettingProxy::instance()->setPredictiveInputStatus(false);
        }
    }

    close();
}

/*!
Toggles prediction state
*/
void HbInputSettingList::predictionButtonClicked()
{
    HbInputSettingProxy::instance()->togglePrediction();

    close();
}

/*!
Opens input settings or input method selection
*/
void HbInputSettingList::listItemActivated(HbListWidgetItem *item)
{
    Q_D(HbInputSettingList);

    if (d->mOptionList->row(item) == d->mOptionList->count() - 1) {
        emit inputSettingsButtonClicked();
    } else {
        emit inputMethodsButtonClicked();
    }

    close();
}

/*!
Sets current primary language and updates the language button text
*/
void HbInputSettingList::primaryLanguageChanged(const HbInputLanguage &newLanguage)
{
    Q_D(HbInputSettingList);

    d->mPrimaryLanguage = HbInputLanguage(newLanguage);
    d->mLanguageButton->setText(d->mPrimaryLanguage.localisedName());
}

/*!
Sets current secondary language
*/
void HbInputSettingList::secondaryLanguageChanged(const HbInputLanguage &newLanguage)
{
    Q_D(HbInputSettingList);

    d->mSecondaryLanguage = HbInputLanguage(newLanguage);
}

/*!
\deprecated HbInputSettingList::predictionStatusChanged(int)
    is deprecated. Use predictionStatusChanged(bool) instead.

Updates prediction button text
*/
void HbInputSettingList::predictionStatusChanged(int newStatus)
{
    Q_UNUSED(newStatus);
}

/*!
Updates prediction button text
*/
void HbInputSettingList::predictionStatusChanged(HbKeyboardSettingFlags keyboardType, bool newStatus)
{
    Q_D(HbInputSettingList);
    Q_UNUSED(keyboardType);
    Q_UNUSED(newStatus);

    bool status = HbInputSettingProxy::instance()->predictiveInputStatusForActiveKeyboard();
    d->mPredictionButton->setText(d->mPredictionValues.at(status));
}

/*!
\deprecated HbInputSettingList::saveSettings()
    is deprecated. Will be removed.
*/
void HbInputSettingList::saveSettings()
{
}

// End of file
