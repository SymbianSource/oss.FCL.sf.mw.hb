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
#include "hbinputsmileypicker.h"

#include <hbgridview.h>
#include <hbview.h>
#include <hbwidget.h>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QFileInfoList>
#include <QDir>

#include <HbMainWindow>
#include <HbFrameItem>
#include <HbFrameDrawer>
#include <hbinputpopupbase_p.h>
#include <hbinputregioncollector_p.h>

const int HbLandscapeRows = 3;
const int HbLandscapeColumns = 7;
const int HbPortraitRows = 4;
const int HbPortraitColumns = 5;

/// @cond

class HbInputSmileyPickerPrivate: public HbInputPopupBasePrivate
{
    Q_DECLARE_PUBLIC(HbInputSmileyPicker)

public:
    HbInputSmileyPickerPrivate(int rows, int columns);
    ~HbInputSmileyPickerPrivate();

    void getSmilies(const QStringList &smileys);
    void _q_activated(const QModelIndex &index);

    // member variables.
    HbGridView *mView;
    QStandardItemModel *mModel;
};

HbInputSmileyPickerPrivate::HbInputSmileyPickerPrivate(int rows, int columns)
    : mView(0), mModel(0)
{
    Q_UNUSED(rows);
    Q_UNUSED(columns);

    // create a view and set the rows and columns.
    mView = new HbGridView();
    mView->setScrollDirections(Qt::Horizontal);
    mView->setHorizontalScrollBarPolicy(HbScrollArea::ScrollBarAsNeeded);
    mModel = new QStandardItemModel(mView);
    mView->setModel(mModel);
}

HbInputSmileyPickerPrivate::~HbInputSmileyPickerPrivate()
{
}

void HbInputSmileyPickerPrivate::getSmilies(const QStringList &smileys)
{
    mModel->clear();
    QStandardItem *item = 0;
    foreach(const QString &smiley, smileys) {
        item = new QStandardItem();
        item->setData(HbIcon(smiley), Qt::DecorationRole);
        mModel->appendRow(item);
    }
}

void HbInputSmileyPickerPrivate::_q_activated(const QModelIndex &index)
{
    Q_Q(HbInputSmileyPicker);
    if (!hidingInProgress) {
        HbIcon smileyIcon = index.model()->data(index, Qt::DecorationRole).value<HbIcon>();
        emit q->selected(smileyIcon.iconName());
        q->close();
    }
}

/// @endcond

/*!
@stable
@hbinput
\class HbInputSmileyPicker
\brief Smiley picker widget

Smiley picker is a dialog containing a grid of emoticons in the form of icons.
It emits selected signal with corresponding emoticon text once a smiley is clicked.

\sa HbDialog
\sa HbGridView
*/
HbInputSmileyPicker::HbInputSmileyPicker(int rows, int columns, QGraphicsItem *parent, QStringList smileys)
    : HbInputPopupBase(*new HbInputSmileyPickerPrivate(rows, columns), parent)
{
    Q_D(HbInputSmileyPicker);
    HbInputRegionCollector::instance()->attach(this);

    if (!rows || !columns) {
        if (mainWindow()->orientation() == Qt::Horizontal) {
            rows = HbLandscapeRows;
            columns = HbLandscapeColumns;
        } else {
            rows = HbPortraitRows;
            columns = HbPortraitColumns;
        }
    }
    d->mView->setRowCount(rows);
    d->mView->setColumnCount(columns);

    // set dialog properties
    setFocusPolicy(Qt::ClickFocus);
    setDismissPolicy(TapAnywhere);
    setBackgroundFaded(false);
    setTimeout(NoTimeout);
    setContentWidget(d->mView);

    // extract smilies.
    d->getSmilies(smileys);

    // connect signals
    connect(d->mView, SIGNAL(activated(QModelIndex)), this, SLOT(_q_activated(QModelIndex)));
}

/*!
Destructs the object.
*/
HbInputSmileyPicker::~HbInputSmileyPicker()
{
}

/*!
This a virtual functions in QGraphicsWidget. It is called whenever the smiley picker widgets is shown.
Here in this function we are are scrolling to a position where we can see
first row and column
*/
void HbInputSmileyPicker::showEvent(QShowEvent *event)
{
    Q_D(HbInputSmileyPicker);
    QStandardItem *item = d->mModel->item(0);
    // when ever we do a show smiley picker.
    // we should scroll back to the first position.
    // otherwise we will show the smiley grid in previous state.
    if (item) {
        d->mView->scrollTo(item->index());
    }
    HbInputPopupBase::showEvent(event);
}

#include "moc_hbinputsmileypicker.cpp"
