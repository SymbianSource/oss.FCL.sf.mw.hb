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

#include "hbinputmethodselectionlist.h"

#include <QGraphicsLinearLayout>

#include <hbmainwindow.h>
#include <hblistwidget.h>
#include <hblistwidgetitem.h>
#include <hbabstractviewitem.h>
#include <hbinputmethod.h>
#include <hbinputsettingproxy.h>
#include <hbinputregioncollector_p.h>
#include "hbinputpopupbase_p.h"
/*!
@stable
@hbinput
\class HbInputMethodSelectionList
\brief Selection list popup for picking input methods.
*/

/// @cond

class HbInputMethodSelectionListPrivate : public HbInputPopupBasePrivate
{
    Q_DECLARE_PUBLIC(HbInputMethodSelectionList)

public:
    HbInputMethodSelectionListPrivate();

    void createSelectionList();

public:
    HbListWidget *mList;
    QList<int> mMethodIndices;

};

HbInputMethodSelectionListPrivate::HbInputMethodSelectionListPrivate()
 : mList(0)
{
}

void HbInputMethodSelectionListPrivate::createSelectionList()
{
    Q_Q(HbInputMethodSelectionList);

    mList = new HbListWidget();
    mList->setSelectionMode(HbAbstractItemView::SingleSelection);
    mList->setItemRecycling(false);

    HbInputSettingProxy *proxy = HbInputSettingProxy::instance();
    Qt::Orientation orientation = q->mainWindow()->orientation();
    HbInputMethodDescriptor descriptor = proxy->preferredInputMethod(orientation);
    QByteArray customData = proxy->preferredInputMethodCustomData(orientation);

    QList<HbInputMethodDescriptor> methodList = HbInputMethod::listCustomInputMethods(q->mainWindow()->orientation(), proxy->globalInputLanguage());
    methodList.insert(0, HbInputMethod::defaultInputMethod(q->mainWindow()->orientation()));

    int selectedIndex = -1;
    int index = -1;
    for (int i = 0; i < methodList.count(); ++i) {
        // If descriptor contains multiple display names, it supports multiple input methods
        // which should be added one by one to the list
        QStringList displayNames = methodList.at(i).displayNames();
        if (!displayNames.isEmpty()) {
            QList<HbIcon> icons = methodList.at(i).icons();
            for (int j = 0; j < displayNames.count(); ++j) {
                HbListWidgetItem* item = new HbListWidgetItem();
                QString displayName = displayNames.at(j);
                item->setText(displayName);
                if (j < icons.count()) {
                    item->setIcon(icons.at(j));
                } else {
                    item->setIcon(methodList.at(i).icon());
                }
                mList->addItem(item);
                mMethodIndices.append(i);
                ++index;

                if (descriptor.pluginNameAndPath() == methodList.at(i).pluginNameAndPath() &&
                    QString::fromUtf8(customData) == displayName) {
                  selectedIndex = index;
               }
            }
        } else {
            // Descriptor contains information about one input method
            HbListWidgetItem* item = new HbListWidgetItem();
            QString displayName = methodList.at(i).displayName();
            item->setText(displayName);
            item->setIcon(methodList.at(i).icon());
            mList->addItem(item);
            mMethodIndices.append(i);
            ++index;
    
            if (descriptor.pluginNameAndPath() == methodList.at(i).pluginNameAndPath() &&
                descriptor.displayName() == displayName ) {
                selectedIndex = index;
            }
        }
    }

    // Default input method doesn't have valid plugin name, so set the index directly
    if (descriptor.isDefault() || descriptor.pluginNameAndPath().isEmpty()) {
        selectedIndex = 0;
    }

    HbAbstractViewItem *viewItem = mList->viewItem(selectedIndex);
    if (viewItem) {
        mList->setCurrentIndex(viewItem->modelIndex(), QItemSelectionModel::Select);
    }
}

/// @endcond

/*!
Constructs input method selection list
*/
HbInputMethodSelectionList::HbInputMethodSelectionList(QGraphicsWidget* parent)
 : HbInputPopupBase(*new HbInputMethodSelectionListPrivate(), parent)
{
    Q_D(HbInputMethodSelectionList);
    HbInputRegionCollector::instance()->attach(this);

    qreal listWidth(300);
    style()->parameter(QString("expr(var(hb-param-screen-short-edge)-(2*var(hb-param-margin-gene-screen)))"), listWidth);
    qreal margin(5);
    style()->parameter(QString("hb-param-margin-gene-popup"), margin);

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout();
    layout->setContentsMargins(margin, margin, margin, margin);

    // set default values for popup
    setTimeout(HbDialog::NoTimeout);
    setBackgroundFaded(false);
    setDismissPolicy(TapOutside);
    setPreferredWidth(listWidth);
    setModal(true);

    d->createSelectionList();

    layout->addItem(d->mList);
    QGraphicsWidget *content = new QGraphicsWidget(this);
    content->setLayout(layout);
    setContentWidget(content);

    connect(d->mList, SIGNAL(activated(HbListWidgetItem*)), this, SLOT(activateSelectedMethod(HbListWidgetItem*)));
    connect(d->mList, SIGNAL(longPressed(HbListWidgetItem*, const QPointF&)), this, SLOT(activateSelectedMethod(HbListWidgetItem*)));
}

/*!
Destructs the object.
*/
HbInputMethodSelectionList::~HbInputMethodSelectionList()
{
}

/*!
Called when input method is selected from the list. Signal inputMethodSelected
is emitted with selected input method as parameter.
*/
void HbInputMethodSelectionList::activateSelectedMethod(HbListWidgetItem *item)
{
    Q_D(HbInputMethodSelectionList);

    int index = d->mMethodIndices.at(d->mList->row(item));
    QByteArray customData;

    HbInputMethodDescriptor selectedMethod;

    if (index == 0) {
        selectedMethod = HbInputMethod::defaultInputMethod(mainWindow()->orientation());

        if (!selectedMethod.displayNames().isEmpty()) {
            customData = selectedMethod.displayNames().at(d->mList->row(item)).toUtf8();
        }
    } else {
        HbInputSettingProxy *proxy = HbInputSettingProxy::instance();
        QList<HbInputMethodDescriptor> customList = HbInputMethod::listCustomInputMethods(mainWindow()->orientation(), proxy->globalInputLanguage());
        selectedMethod = customList.at(index - 1);

        int firstItemIndex = d->mMethodIndices.indexOf(index);

        if (!selectedMethod.displayNames().isEmpty()) {
            customData = selectedMethod.displayNames().at(d->mList->row(item) - firstItemIndex).toUtf8();
        }
    }

    close();

    emit inputMethodSelected(selectedMethod, customData);
}

// End of file
