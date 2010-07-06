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
#include "hbinputsettinglist.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>

#include <hbmainwindow.h>
#include <hblabel.h>
#include <hbpushbutton.h>
#include <hblistwidget.h>
#include <hblistwidgetitem.h>
#include <hbinputmethod.h>
#include <hbinputsettingproxy.h>
#include <hbinpututils.h>
#include <hbinputpredictionfactory.h>
#include <hbinputregioncollector_p.h>
#include <hbinputmethodselectionlist.h>

#include "hbdialog_p.h"

const QString settingsIcon("qtg_mono_settings");
const QString inputMethodIcon("qtg_mono_virtual_input");

const qreal HbSelectionListMarginInUnits = 0.8;
const qreal HbSelectionListLandscapeAlignInUnits = 9.4;

/// @cond

class HbInputSettingListPrivate : public HbDialogPrivate
{
    Q_DECLARE_PUBLIC(HbInputSettingList)

public:
    HbInputSettingListPrivate();
    ~HbInputSettingListPrivate();

    qreal languageNameWidth();
    void showInputMethodSelectionList();

public:
    HbPushButton *mLanguageButton;
    HbPushButton *mPredictionButton;
    HbListWidget *mOptionList;
    HbInputLanguage mPrimaryLanguage;
    HbInputLanguage mSecondaryLanguage;
    QList<QString> mPredictionValues;
    HbInputMethodSelectionList *mInputMethodSelectionList;
};

HbInputSettingListPrivate::HbInputSettingListPrivate()
 : mLanguageButton(0), mPredictionButton(0), mOptionList(0), mInputMethodSelectionList(0)
{
}

HbInputSettingListPrivate::~HbInputSettingListPrivate()
{
    delete mInputMethodSelectionList;
}

qreal HbInputSettingListPrivate::languageNameWidth()
{
    qreal nameWidth(0);
    QList<HbInputLanguage> languages;
    HbInputUtils::listSupportedInputLanguages(languages);
    QFontMetrics fontMetrics(mLanguageButton->font());

    foreach(HbInputLanguage language, languages) {
        qreal width = fontMetrics.width(language.localisedName());
        if (width > nameWidth) {
            nameWidth = width;
        }
    }

    return nameWidth;
}

void HbInputSettingListPrivate::showInputMethodSelectionList()
{
    Q_Q(HbInputSettingList);

    delete mInputMethodSelectionList;
    mInputMethodSelectionList = new HbInputMethodSelectionList();
    mInputMethodSelectionList->setObjectName("Input method selection list");

    qreal unitValue = HbDeviceProfile::profile(q->mainWindow()).unitValue();

    QPointF position(q->scenePos().x() + q->size().width(),
                     mOptionList->scenePos().y() - HbSelectionListMarginInUnits * unitValue);

    if (q->mainWindow()->orientation() == Qt::Horizontal) {
        position.setX(position.x() - HbSelectionListLandscapeAlignInUnits * unitValue);
    }
    
    mInputMethodSelectionList->setPreferredPos(position, HbPopup::BottomRightCorner);

    QObject::connect(mInputMethodSelectionList, SIGNAL(inputMethodSelected(const HbInputMethodDescriptor &, const QByteArray &)),
                     q, SLOT(closeSettings(const HbInputMethodDescriptor &, const QByteArray &)));
    mInputMethodSelectionList->show();
}

/// @endcond

/*!
Constructs input setting list
*/
HbInputSettingList::HbInputSettingList(QGraphicsWidget *parent)
    : HbDialog(*new HbInputSettingListPrivate(), parent)
{
    Q_D(HbInputSettingList);
    HbInputRegionCollector::instance()->attach(this);

    // Get correct size from style parameters
    HbStyle style;
    qreal listWidth(300);
    style.parameter(QString("expr(var(hb-param-screen-short-edge)-(2*var(hb-param-margin-gene-screen)))"), listWidth);
    qreal margin(5);
    style.parameter(QString("hb-param-margin-gene-popup"), margin);

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
    d->mOptionList->setContentsMargins(0, 0, 0, 0);
    d->mOptionList->setPreferredWidth(listWidth - 2 * margin);

    gridLayout->addItem(languageLabel, 0, 0);
    gridLayout->addItem(d->mLanguageButton, 0, 1);
    gridLayout->addItem(predictionLabel, 1, 0);
    gridLayout->addItem(d->mPredictionButton, 1, 1);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    // Width for language button is based on the width of language name string and button margins
    qreal buttonMargin(20);
    style.parameter(QString("expr(var(hb-param-margin-gene-left)+var(hb-param-margin-gene-right))"), buttonMargin);
    gridLayout->setColumnFixedWidth(1, buttonMargin + d->languageNameWidth());

    mainLayout->addItem(gridLayout);
    mainLayout->addItem(d->mOptionList);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    QGraphicsWidget *content = new QGraphicsWidget(this);
    content->setContentsMargins(0, 0, 0, 0);
    content->setLayout(mainLayout);
    setContentWidget(content);

    d->mPredictionValues.append(tr("Off"));
    d->mPredictionValues.append(tr("On"));

    // set default values for popup
    setTimeout(HbDialog::NoTimeout);
    setBackgroundFaded(false);
    setDismissPolicy(TapOutside);
    setContentsMargins(margin, margin, margin, margin);
    setPreferredWidth(listWidth);

    // Make sure the input settings list never steals focus.
    setFlag(QGraphicsItem::ItemIsPanel, true);
    setActive(false);

    connect(d->mLanguageButton, SIGNAL(clicked(bool)), this, SLOT(languageButtonClicked()));
    connect(d->mPredictionButton, SIGNAL(clicked(bool)), this, SLOT(predictionButtonClicked()));
    connect(d->mOptionList, SIGNAL(activated(HbListWidgetItem *)), this, SLOT(listItemActivated(HbListWidgetItem *)));
    connect(d->mOptionList, SIGNAL(longPressed(HbListWidgetItem *, const QPointF &)), this, SLOT(listItemActivated(HbListWidgetItem *)));

    HbInputSettingProxy *settings = HbInputSettingProxy::instance();
    connect(settings, SIGNAL(globalInputLanguageChanged(const HbInputLanguage &)), this, SLOT(primaryLanguageChanged(const HbInputLanguage &)));
    connect(settings, SIGNAL(globalSecondaryInputLanguageChanged(const HbInputLanguage &)), this, SLOT(secondaryLanguageChanged(const HbInputLanguage &)));
    connect(settings, SIGNAL(predictiveInputStateChanged(HbKeyboardSettingFlags, bool)), this, SLOT(predictionStatusChanged(HbKeyboardSettingFlags, bool)));

    connect(this, SIGNAL(aboutToClose()), this, SLOT(aboutToClose()));
}

/*!
Destructs the object.
*/
HbInputSettingList::~HbInputSettingList()
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

    QList<HbInputMethodDescriptor> customList = HbInputMethod::listCustomInputMethods(mainWindow()->orientation(), d->mPrimaryLanguage);
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
Closes input method selection if settings is closed
*/
void HbInputSettingList::aboutToClose()
{
    Q_D(HbInputSettingList);

    if (d->mInputMethodSelectionList) {
        d->mInputMethodSelectionList->close();
    }
}

/*!
Swaps current primary and secondary languages
*/
void HbInputSettingList::languageButtonClicked()
{
    Q_D(HbInputSettingList);

    HbInputSettingProxy *settings = HbInputSettingProxy::instance();
    HbPredictionFactory *predFactory = HbPredictionFactory::instance();
    if (d->mSecondaryLanguage == HbInputLanguage()) {
        emit inputSettingsButtonClicked();
    } else {
        HbInputLanguage language = d->mPrimaryLanguage;
        bool oldPLangSupportsPrediction = (predFactory->predictionEngineForLanguage(language) != NULL);
        d->mPrimaryLanguage = d->mSecondaryLanguage;
        d->mSecondaryLanguage = language;

        HbInputSettingProxy::instance()->setGlobalInputLanguage(d->mPrimaryLanguage);
        bool langSupportsPrediction = (predFactory->predictionEngineForLanguage(d->mPrimaryLanguage) != NULL);
        HbInputSettingProxy::instance()->setGlobalSecondaryInputLanguage(d->mSecondaryLanguage);

        if (oldPLangSupportsPrediction != langSupportsPrediction) {
            settings->setPredictiveInputStatusForActiveKeyboard(langSupportsPrediction);
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
        close();
    } else {
        d->showInputMethodSelectionList();
    }
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
Closes settings and emits inputMethodSelected signal with given parameter
*/
void HbInputSettingList::closeSettings(const HbInputMethodDescriptor &descriptor, const QByteArray &customData)
{
    close();
    emit inputMethodSelected(descriptor, customData);
}

// End of file
