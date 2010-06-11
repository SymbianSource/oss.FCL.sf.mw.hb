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
#include <QDir>
#include <QtAlgorithms>

#include <hbinputdef.h>
#include <hbselectiondialog.h>
#include <hblistwidgetitem.h>
#include <hbinpututils.h>
#include <hbinputsettingproxy.h>
#include <hbinputmethod.h>
#include <hbinputregioncollector_p.h>

#include "hbinputcommondialogs.h"

/// @cond

bool caseInsensitiveLessThanForHbInputLanguage(const HbInputLanguage &s1, const HbInputLanguage &s2)
{
    //Temporaries because localisedName() is a non-const function
    HbInputLanguage t1 = s1;
    HbInputLanguage t2 = s2;
    return t1.localisedName().toLower() < t2.localisedName().toLower();
}

/// @endcond

/*!
Displays language selection dialog and returns selected language.
*/
HbInputLanguage HbInputCommonDialogs::showLanguageSelectionDialog(QLocale::Language selectedLanguage, const QList<HbInputLanguage> languageList)
{
    HbSelectionDialog* langDialog = new HbSelectionDialog();
    langDialog->setObjectName("Language dialog");

#if QT_VERSION >= 0x040600
    // Make sure the language dialog never steals focus.
    langDialog->setFlag(QGraphicsItem::ItemIsPanel, true);
    langDialog->setActive(false);
#endif

    HbInputSettingProxy* settings = HbInputSettingProxy::instance();
    HbInputLanguage currentLang = settings->globalInputLanguage();
    QList<HbInputLanguage> languages;
    if (languageList.count() == 0) {
        HbInputUtils::listSupportedInputLanguages(languages);
    } else {
        languages=languageList;
    }
	
	qStableSort(languages.begin(), languages.end(), caseInsensitiveLessThanForHbInputLanguage);
    QList<HbListWidgetItem*> listItems;
    HbListWidgetItem* item = 0;
    foreach( HbInputLanguage language, languages ) {
                QString langName = language.localisedName();
        if (langName.length() == 0) {
            langName = QString("Unknown");
        }
        item = new HbListWidgetItem();
        item->setText(langName);
        if (language == currentLang ) {
            QList<QVariant> selected;
            selected.append(listItems.count());
            langDialog->setSelectedItems(selected);
        }
        if (language.language() == QLocale::Chinese) {
            item->setData(language.variant(), Hb::UserRole);
        } else {
            item->setData(language.language(), Hb::UserRole);
        }

        listItems.append(item);
    }
    langDialog->setWidgetItems(listItems, true);
    langDialog->setSelectionMode(HbAbstractItemView::SingleSelection);
    langDialog->setDismissPolicy(HbPopup::NoDismiss);
    langDialog->setModal(true);
    //langDialog->exec(); TODO
    HbInputRegionCollector::instance()->attach(langDialog);
    //TODO: needed to check from action which button was pressed (ok, cancel)
    //HbAction* action = langDialog->exec();
    if (langDialog->selectedItems().count()) {
        int selection = langDialog->selectedItems().at(0).toInt();
        HbInputLanguage result;
        if(languages.at(selection).language() == QLocale::Chinese) {
            result = languages.at(selection);
        } else {
            result = languages.at(selection).language();
        }
        return result;
    } else {
        return selectedLanguage;
    }
}

/*!
Displays input method selection dialog. The returned descriptor will contain string defined by HbDefaultModeIdString
constant in its path field in case the system default input was selected. Otherwise the descriptor
points to selected custom input plugin.
*/
HbInputMethodDescriptor HbInputCommonDialogs::showCustomInputMethodSelectionDialog(const HbInputLanguage &language)
{
    Q_UNUSED(language);

    HbSelectionDialog* methodDialog = new HbSelectionDialog();
    methodDialog->setObjectName("Input method dialog");

#if QT_VERSION >= 0x040600
    // Make sure the language dialog never steals focus.
    methodDialog->setFlag(QGraphicsItem::ItemIsPanel, true);
    methodDialog->setActive(false);
#endif

    QList<HbInputMethodDescriptor> customList = HbInputMethod::listCustomInputMethods();

    QList<HbListWidgetItem*> listItems;
    HbListWidgetItem* item = new HbListWidgetItem();
    QString methodName("Default");
    item->setText(methodName);
    listItems.append(item);

    //for (int i=0; i<customList.dllName.count() ; i++) {
    foreach (HbInputMethodDescriptor descriptor, customList) {
    QString displayName = descriptor.displayName();
        if (displayName.length() == 0) {
            displayName = QString("Unknown");
        }
        item = new HbListWidgetItem();
        item->setText(displayName);
        listItems.append(item);
    }
    methodDialog->setWidgetItems(listItems, true);
    methodDialog->setSelectionMode(HbAbstractItemView::SingleSelection);
    methodDialog->setDismissPolicy(HbPopup::NoDismiss);
    methodDialog->setModal(true);
    //methodDialog->exec(); TODO
    HbInputRegionCollector::instance()->attach(methodDialog);
    //TODO: needed to check from action which button was pressed (ok, cancel)
    //HbAction* action = langDialog->exec();

    HbInputMethodDescriptor result;
    if (methodDialog->selectedItems().count()) {
        int selection = methodDialog->selectedItems().first().toInt();
        if (selection == 0) {
           result.setDefault();
        } else {
           result = customList[selection-1];
        }
    }

    return result;
}

// End of file
