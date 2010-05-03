/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
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
#include "hbselectiondialog.h"
#include "hbwidget_p.h"
#include "hbselectiondialog_p.h"
#include "hblabel.h"
#include "hbaction.h"
#include "hbabstractviewitem.h"
#include <hbinstance.h>

#include <QtDebug>
#include <QGraphicsScene>
#include <hblistwidgetitem.h>
#include <hblistwidget.h>
#include <hbradiobuttonlist.h>


/*!
    @beta
    @hbwidgets
    \class HbSelectionDialog
    \brief HbSelectionDialog class allows user create a list of options out of which one or more can be selected.  
    
    HbSelectionDialog is a modal dialog which means once it is displayed, user can not perform any action
    untill dialog is closed.

    There can be 2 modes of selection. SingleSelection or MultiSelection. If it is SingleSelection, dialog is closed
    as soon as user clicks one of the options.In case of MultiSelection, user has to explicitly press "OK" button to
    close it after selecting the item(s). User can anytime press "Cancel" button to close the dialog without selecting 
    anything.

    User can provide the data for options in different forms. It can be simple list of strings, list of custom 
    items or a model itself.
*/


/*!
    @beta
    Constructor of HbSelectionDialog

    \param parent. Parent widget
*/
HbSelectionDialog::HbSelectionDialog(QGraphicsItem* parent): 
                                HbDialog(*new HbSelectionDialogPrivate, parent)
{
    Q_D(HbSelectionDialog);
    d->init();
    setDismissPolicy(NoDismiss);
}

/*!
    @beta
    Destructor 
 */
HbSelectionDialog::~HbSelectionDialog()
{
}

/*!
   @beta

   \reimp
 */
void HbSelectionDialog::showEvent(QShowEvent *event)
{    
    HbDialog::showEvent(event);
}

/*!
    @beta
    Sets the \a SelectionMode of the list.
    
    \param mode. It can be SingleSelection or MultiSelection .Default value is \a NoSelection.

    \sa selectionMode()
*/
void HbSelectionDialog::setSelectionMode(HbAbstractItemView::SelectionMode mode)
{
	Q_D(HbSelectionDialog);
	
	d->setSelectionMode(mode);
}

/*!
    @beta
    Returns current SelectionMode of the list.Default value is \a NoSelection.

    \sa setSelectionMode()
*/	
HbAbstractItemView::SelectionMode HbSelectionDialog::selectionMode() const
{
	Q_D(const HbSelectionDialog);
	return d->mSelectionMode;
}

/*!
    @beta
    Sets the string list items to be displayed.

    \param items. A items is the list of strings
    \param currentIndex. A currentIndex is the index of default selection

    \sa stringItems()
*/	
void HbSelectionDialog::setStringItems(const QStringList &items,int currentIndex)
{
	Q_D(HbSelectionDialog);
	d->setStringItems(items,currentIndex);
}

/*!
    @beta
    Returns list of string list items earlier set by setStringItems().

    \sa setStringItems()
*/	
QStringList HbSelectionDialog::stringItems() const
{
	Q_D(const HbSelectionDialog);
	return d->stringItems();
}

/*!
    @beta
    Returns list of selected indexes. List contains only one item if
    \a SelectionMode is \a NoSelection or \a SingleSelection. It may 
    contain more items if \a SelectionMode is \a MultiSelection.
   
    \sa setSelectionMode(), 
    \sa selectionMode()
 */
QList<QVariant> HbSelectionDialog::selectedItems() const
{
	Q_D(const HbSelectionDialog);
	return d->selectedItems();
}

/*!
    @beta
    set the item selected.
    It can select one item if \a Selection mode is \a SingleSelection
    it can select more item if \a SelectionMode is \a MultiSelection.

    \param items.

    \sa selectedItems
*/
void HbSelectionDialog::setSelectedItems(const QList<QVariant> items) 
{
	Q_D(HbSelectionDialog);
	d->setSelectedItems(items);
}

/*!
    @beta
    Returns list of selected model indexes. List contains only one item if
    \a SelectionMode is \a NoSelection or \a SingleSelection. It may 
    contain more items if \a SelectionMode is \a MultiSelection.
    
    \sa setSelectionMode(),
    \sa selectionMode()
 */
QModelIndexList HbSelectionDialog::selectedModelIndexes() const
{
	Q_D(const HbSelectionDialog);
	return d->selectedModelIndexes();
}

/*!
    @beta
    Sets the list of custom list items to be displayed.\a items is the
    list of custom items.\a bTransferOwnership is a flag defining the owner
    of the items. If \a true, items will be deleted when dialog is deleted else
    user is responsible for deleting the items.Default value is \a false.
    \a current is the index of default selection.

    \param items. items is the list of custom items
    \param transferOwnership. true or false
    \param currentIndex

    \sa widgetItems();
*/	
void HbSelectionDialog::setWidgetItems(const QList<HbListWidgetItem*> &items,bool transferOwnership,int currentIndex)
{
	Q_D(HbSelectionDialog);
	d->setWidgetItems(items,transferOwnership,currentIndex);
}

/*!
    @beta
    Returns list of custom list items earlier set by setWidgetItems().
    
    \sa setWidgetItems().
*/	
QList<HbListWidgetItem*> HbSelectionDialog::widgetItems() const
{
	Q_D(const HbSelectionDialog);
	return d->widgetItems();
}

/*!
    @beta
    Sets the Model containing data for the list items.

    \param model. 

    \sa model()
*/	
void HbSelectionDialog::setModel(QAbstractItemModel* model)
{
	Q_D(HbSelectionDialog);
	d->setModel(model);
}

/*!
    @beta
    Returns model eariler set by setModel().

    \sa setModel()
*/	
QAbstractItemModel* HbSelectionDialog::model() const
{
	Q_D(const HbSelectionDialog);
	return d->model();
}


#include "moc_hbselectiondialog.cpp"
