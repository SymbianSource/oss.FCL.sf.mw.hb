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
    \brief HbSelectionDialog class allows user to create a list of options out of which one or more can be selected.  
    
    SelectionDialog is a modal dialog, for which user has to use default or custom action buttons to dismiss the dialog.

    There are 2 modes of selection for SelectionDialog. SingleSelection or MultiSelection.<br>

    <b>SingleSelection</b>: User can select one item from the list at a time and the SelectionDialog will be dismissed 
    as soon as the user selects the item.<br>

    <b>MultiSelection</b>: User can select multiple items from the list. To dismiss the dialog user has to explicitly press "OK/Cancel" button.
    User can anytime press "Cancel" button to close the dialog without any selection.

    User can provide the data for options in different forms. It can be simple list of strings, list of custom 
    items or a model itself.

    Below is a snippet of code which shows SelectionDialog in a single selection mode.

    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,55}

    slot implementation:

    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,56}

*/


/*!
    Constructor of HbSelectionDialog

    \param parent Parent item to SelectionDialog.
*/
HbSelectionDialog::HbSelectionDialog(QGraphicsItem* parent): 
                                HbDialog(*new HbSelectionDialogPrivate, parent)
{
    Q_D(HbSelectionDialog);
    d->init();
    setDismissPolicy(NoDismiss);
}

/*!
    Destructor 
 */
HbSelectionDialog::~HbSelectionDialog()
{
}

/*!
   \reimp
 */
void HbSelectionDialog::showEvent(QShowEvent *event)
{    
    HbDialog::showEvent(event);
}

/*!
    Sets the \a SelectionMode of the list.
    
    \param mode It can be SingleSelection or MultiSelection.

    \sa selectionMode()
*/
void HbSelectionDialog::setSelectionMode(HbAbstractItemView::SelectionMode mode)
{
    Q_D(HbSelectionDialog);
    
    d->setSelectionMode(mode);
}

/*!
    Returns current SelectionMode of the list.Default value is \a NoSelection.

    \sa setSelectionMode()
*/    
HbAbstractItemView::SelectionMode HbSelectionDialog::selectionMode() const
{
    Q_D(const HbSelectionDialog);
    return d->mSelectionMode;
}

/*!
    Sets the string list items to be displayed.

    \param items list of string items as input to SelectionDialog in QStringList format.
    \param currentIndex index of item to be selected in the SelectionDialog by default.

    \sa stringItems()
*/    
void HbSelectionDialog::setStringItems(const QStringList &items,int currentIndex)
{
    Q_D(HbSelectionDialog);
    d->setStringItems(items,currentIndex);
}

/*!
    Returns list of string list items earlier set by setStringItems().

    \sa setStringItems()
*/    
QStringList HbSelectionDialog::stringItems() const
{
    Q_D(const HbSelectionDialog);
    return d->stringItems();
}

/*!
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
    set the item selected.
    It can select one item if \a Selection mode is \a SingleSelection
    it can select more item if \a SelectionMode is \a MultiSelection.

    \param items indices of the items to be selected in SelectionDialog.

    \sa selectedItems
*/
void HbSelectionDialog::setSelectedItems(const QList<QVariant> items) 
{
    Q_D(HbSelectionDialog);
    d->setSelectedItems(items);
}

/*!
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
    Sets the list of custom list items to be displayed.\a items is the
    list of custom items.\a bTransferOwnership is a flag defining the owner
    of the items. If \a true, items will be deleted when dialog is deleted else
    user is responsible for deleting the items.Default value is \a false.
    \a current is the index of default selection.

    \param items list of custom items to be set as input to SelectionDialog.
    \param transferOwnership Transfer the ownership of items to SelectionDialog by passing true else false.
    \param currentIndex index of the item to be selected in SelectionDialog.

    \sa widgetItems();
*/    
void HbSelectionDialog::setWidgetItems(const QList<HbListWidgetItem*> &items,bool transferOwnership,int currentIndex)
{
    Q_D(HbSelectionDialog);
    d->setWidgetItems(items,transferOwnership,currentIndex);
}

/*!
    Returns list of custom list items earlier set by setWidgetItems().
    
    \sa setWidgetItems().
*/    
QList<HbListWidgetItem*> HbSelectionDialog::widgetItems() const
{
    Q_D(const HbSelectionDialog);
    return d->widgetItems();
}

/*!
    Sets the Model containing data for the list items.

    \param model which has data for items to be set as input to SelectionDialog. 

    \sa model()
*/    
void HbSelectionDialog::setModel(QAbstractItemModel* model)
{
    Q_D(HbSelectionDialog);
    d->setModel(model);
}

/*!
    Returns model eariler set by setModel().

    \sa setModel()
*/    
QAbstractItemModel* HbSelectionDialog::model() const
{
    Q_D(const HbSelectionDialog);
    return d->model();
}

#include "moc_hbselectiondialog.cpp"
