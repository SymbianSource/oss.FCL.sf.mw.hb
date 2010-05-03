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
#include "hblistdialog.h"
#include "hbwidget_p.h"
#include "hblistdialog_p.h"
#include "hblabel.h"
#include "hbaction.h"
#include "hbabstractviewitem.h"
#include <hbinstance.h>
#include "hbglobal_p.h"

#include <QtDebug>
#include <QGraphicsScene>
#include <hblistwidgetitem.h>
#include <hblistwidget.h>
#include <hbradiobuttonlist.h>
/*!
  \this class is deprecated. Use HbSelectionDialog.
*/

/*!
  \deprecated HbListDialog(QGraphicsItem*)
	is deprecated. \this is deprecated, use HbSelectionDialog.

  Constructor	
*/

HbListDialog::HbListDialog(QGraphicsItem* parent): 
                                HbDialog(*new HbListDialogPrivate, parent)
{
	HB_DEPRECATED("HbListDialog class is deprecated. Use HbSelectionDialog");

    Q_D(HbListDialog);
    d->init();
    setDismissPolicy(NoDismiss);
}

/*!
  @beta
  Destructor 
 */
HbListDialog::~HbListDialog()
{
}

/*!
 @beta
 Protected. 
 */
void HbListDialog::showEvent(QShowEvent *event)
{    
    HbDialog::showEvent(event);
}

/*!
    @beta
   Sets the \a SelectionMode of the list. It can be 
   \a SingleSelection or \a MultiSelection or \a NoSelection. 
   If \a ListMode is \a RadioButtonListInput, \a MultiSelection mode will not 
   work. Default value is \a NoSelection.
   \sa selectionMode. 
 */
void HbListDialog::setSelectionMode(HbAbstractItemView::SelectionMode mode)
{
	Q_D(HbListDialog);
	
	d->setSelectionMode(mode);
}

/*!
    @beta
    Returns current \a SelectionMode of the list. It can be set
    by \a setSelectionMode(). Default value is \a NoSelection.
*/	
HbAbstractItemView::SelectionMode HbListDialog::selectionMode() const
{
	Q_D(const HbListDialog);
	return d->mSelectionMode;
}

/*!
    @beta
    Sets the string list items to be displayed.\a items is the
    list of strings and \a current is the index of default selection.
    \sa stringItems()
*/	
void HbListDialog::setStringItems(const QStringList &items,int current)
{
	Q_D(HbListDialog);
	d->setStringItems(items,current);
}

/*!
    @beta
    Returns list of string list items earlier set by \a setStringItems().
*/	
QStringList HbListDialog::stringItems() const
{
	Q_D(const HbListDialog);
	return d->stringItems();
}

/*!
   @beta
   Returns list of selected indexes. List contains only one item if
   \a SelectionMode is \a NoSelection or \a SingleSelection. It may 
   contain more items if \a SelectionMode is \a MultiSelection.
   \sa 	setSelectionMode(), \sa selectionMode()
 */
QList<int> HbListDialog::selectedItems() const
{
	Q_D(const HbListDialog);
	return d->selectedItems();
}

/*!
   @beta
   set the item selected.
   It can select one item if \a Selection mode is \a SingleSelection
   it can select more item if \a SelectionMode is \a MultiSelection.
*/
void HbListDialog::setSelectedItems(QList<int> items) 
{
	Q_D(HbListDialog);
	d->setSelectedItems(items);
}

/*!
    @beta
   Returns list of selected model indexes. List contains only one item if
   \a SelectionMode is \a NoSelection or \a SingleSelection. It may 
   contain more items if \a SelectionMode is \a MultiSelection.
   \sa 	setSelectionMode(), \sa selectionMode()
 */
QModelIndexList HbListDialog::selectedModelIndexes() const
{
	Q_D(const HbListDialog);
	return d->selectedModelIndexes();
}

/*!
    @beta
    Sets the list of custom list items to be displayed.\a items is the
    list of custom items.\a bTransferOwnership is a flag defining the owner
    of the items. If \a true, items will be deleted when dialog is deleted else
    user is responsible for deleting the items.Default value is \a false.
    \a current is the index of default selection.
    \sa widgetItems();
*/	
void HbListDialog::setWidgetItems(QList<HbListWidgetItem*> &items,bool bTransferOwnership,int current)
{
	Q_D(HbListDialog);
	d->setWidgetItems(items,bTransferOwnership,current);
}

/*!
    @beta
    Returns list of custom list items earlier set by \a setWidgetItems().
*/	
QList<HbListWidgetItem*> HbListDialog::widgetItems() const
{
	Q_D(const HbListDialog);
	return d->widgetItems();
}

/*!
    @beta
    Sets the Model containing data for the list items.
*/	
void HbListDialog::setModel(QAbstractItemModel* model)
{
	Q_D(HbListDialog);
	d->setModel(model);
}

/*!
    @beta
    Returns Model stored by list.
*/	
QAbstractItemModel* HbListDialog::model() const
{
	Q_D(const HbListDialog);
	return d->model();
}
/*!                                  
    Static convenience function to let the user select item(s) from a
    string list. \a label is the text which is shown to the user (it
    should say what should be entered). \a list is the string list which 
    is inserted into the list and \a current is 
    the number of the item which should be the current item. 

    If \a ok is non-null \e *\a ok will be set to true if the user
    pressed \gui OK and to false if the user pressed \gui Cancel. \a mode
    defines the selection mode of the list. It can be either of \NoSelection,
    \SingleSelection and \MultiSelection. The dialog's parent is \a parent. 
    The dialog will be modal.

    this functions connects to the receiverd slot with signature finished(HbAction*).
    there the selectedItems has to be queried.
*/
void HbListDialog::getStringItems(const QString &label, 
                                    const QStringList &list,
                                    QObject *receiver,
                                    const char *member,
                                    int current,
                                    HbAbstractItemView::SelectionMode mode, 
                                    QGraphicsScene *scene, 
                                    QGraphicsItem *parent)
{
	HbListDialog *dlg = new HbListDialog(parent);
    if (scene && !parent) {
        scene->addItem(dlg);
    }
	QStringList result;
	QList<int> selIndexes;
    selIndexes << current;
    if(!label.isNull()) {
        dlg->setHeadingWidget(new HbLabel(label));
    }
    dlg->setStringItems(list,current);
	dlg->setSelectionMode(mode);
    dlg->setSelectedItems(selIndexes);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->open(receiver,member);
}

/*!
   Static convenience function to let the user select item(s) from a
    list of user defined items. \a label is the text which is shown to the 
    user (it should say what should be entered). \a list is the list 
    of user defined items shown to the user.Ownership is not transferred.
    \a current is the number of the item which should be the current item. 

    If \a ok is non-null \e *\a ok will be set to true if the user
    pressed \gui OK and to false if the user pressed \gui Cancel. \a mode
    defines the selection mode of the list. It can be either of \NoSelection,
    \SingleSelection and \MultiSelection. The dialog's parent is \a parent. 
    The dialog will be modal.

    this functions connects to the receiverd slot with signature finished(HbAction*).
    there the selectedItems has to be queried.
*/
void HbListDialog::getWidgetItems(const QString &label, 
                                    QList<HbListWidgetItem*> &list,
                                    QObject *receiver,
                                    const char *member,
                                    int current,
                                    HbAbstractItemView::SelectionMode mode, 
                                    QGraphicsScene *scene, 
                                    QGraphicsItem *parent)
{
    HbListDialog *dlg = new HbListDialog(parent);
    if (scene && !parent) {
        scene->addItem(dlg);
    }
    QList<HbListWidgetItem*> result;
    QList<int> selIndexes;
    selIndexes << current;
    if(!label.isNull()) {
        dlg->setHeadingWidget(new HbLabel(label));
    }
	dlg->setWidgetItems(list,false,current);
	dlg->setSelectionMode(mode);
    dlg->setSelectedItems(selIndexes);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->open(receiver,member);
}

/*!
    Static convenience function to let the user select item(s) from a
    list of items defined by a user set model.\a label is the text which 
    is shown to the user (it should say what should be entered). \a model 
    is user defined model which list will use to render the items.
	Ownership is not transferred.
    If \a ok is non-null \e *\a ok will be set to true if the user
    pressed \gui OK and to false if the user pressed \gui Cancel. \a mode
    defines the selection mode of the list. It can be either of \NoSelection,
    \SingleSelection and \MultiSelection. 
    The dialog's parent is \a parent. The dialog will be modal.

    this functions connects to the receiverd slot with signature finished(HbAction*).
    there the selectedItems has to be queried.
*/
void HbListDialog::getModelIndexes(const QString &label, QAbstractItemModel* model,
                                   QObject *receiver,
                                   const char *member,
                                   HbAbstractItemView::SelectionMode mode, QGraphicsScene *scene, QGraphicsItem *parent)
{
    HbListDialog *dlg = new HbListDialog(parent);
    if (scene && !parent) {
        scene->addItem(dlg);
    }
    QModelIndexList result;
    if(!label.isNull()) {
        dlg->setHeadingWidget(new HbLabel(label));
    }
	dlg->setModel(model);
	dlg->setSelectionMode(mode);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->open(receiver,member);
}


/*!
    \deprecated HbListDialog::getStringItems(const QString&,const QStringList,int,bool*,HbAbstractItemView::SelectionMode,QGraphicsScene*,QGraphicsItem*)
        is deprecated. Please use the other available HbListDialog::getStringItems(...) API.
    
	Static convenience function to let the user select item(s) from a
    string list. \a label is the text which is shown to the user (it
    should say what should be entered). \a list is the string list which 
    is inserted into the list and \a current is 
    the number of the item which should be the current item. 

    If \a ok is non-null \e *\a ok will be set to true if the user
    pressed \gui OK and to false if the user pressed \gui Cancel. \a mode
    defines the selection mode of the list. It can be either of \NoSelection,
    \SingleSelection and \MultiSelection. The dialog's parent is \a parent. 
    The dialog will be modal.

    This function returns the list of selected string items.
    List will contain only one item if \a SelectionMode is \a NoSelection or \a SingleSelection 
    and may contain more items in case of \a MultiSelection.
*/
QStringList HbListDialog::getStringItems(const QString &label, const QStringList &list,int current,
                                                bool *ok,HbAbstractItemView::SelectionMode mode, QGraphicsScene *scene, QGraphicsItem *parent)
{
    HB_DEPRECATED("HbListDialog::getStringItems is deprecated. Use HbListDialog::getStringItems(const QString &,const QStringList &,QObject *receiver,const char *,int,HbAbstractItemView::SelectionMode, QGraphicsScene *, QGraphicsItem *)");

	Q_UNUSED(parent)
	
	HbListDialog *dlg = new HbListDialog();
    if (scene) {
        scene->addItem(dlg);
    }
    QStringList result;
    QList<int> selIndexes;
    selIndexes << current;
    if(!label.isNull())
        dlg->setHeadingWidget(new HbLabel(label));
    dlg->setStringItems(list,current);
    dlg->setSelectionMode(mode);
    dlg->setSelectedItems(selIndexes);
    HbAction* action = dlg->exec();
    if(action == dlg->secondaryAction()){ //Cancel was pressed
        if(ok)
            *ok = false;
    }
    else{ //OK was pressed
        if(ok)
            *ok = true;
        selIndexes = dlg->selectedItems();
    }
    delete dlg;
    for(int i = 0; i < selIndexes.count(); i++ ){
        result += list[selIndexes[i]];	
    }
    return result;
}

/*!
    \deprecated HbListDialog::getWidgetItems(const QString &, QList<HbListWidgetItem*> &,int,bool *,HbAbstractItemView::SelectionMode, QGraphicsScene *, QGraphicsItem *)
        is deprecated. Use the other available HbListDialog::getWidgetItems(...) API.

    Static convenience function to let the user select item(s) from a
    list of user defined items. \a label is the text which is shown to the 
    user (it should say what should be entered). \a list is the list 
    of user defined items shown to the user.Ownership is not transferred.
    \a current is the number of the item which should be the current item. 

    If \a ok is non-null \e *\a ok will be set to true if the user
    pressed \gui OK and to false if the user pressed \gui Cancel. \a mode
    defines the selection mode of the list. It can be either of \NoSelection,
    \SingleSelection and \MultiSelection. The dialog's parent is \a parent. 
    The dialog will be modal.

    This function returns the list of selected widget items.
    List will contain only one item if \a SelectionMode is \a NoSelection or
    \SingleSelection and may contain more items in case of \a MultipleSelection.
*/
QList<HbListWidgetItem*> HbListDialog::getWidgetItems(const QString &label, QList<HbListWidgetItem*> &list,int current,
                                                        bool *ok,HbAbstractItemView::SelectionMode mode, QGraphicsScene *scene, QGraphicsItem *parent)
{
    HB_DEPRECATED("HbListDialog::getWidgetItems is deprecated. Use HbListDialog::getWidgetItems(const QString &, QList<HbListWidgetItem*> &list,QObject *,const char *,int,HbAbstractItemView::SelectionMode, QGraphicsScene *, QGraphicsItem *)");

    Q_UNUSED(parent)
    HbListDialog *dlg = new HbListDialog();
    if (scene) {
        scene->addItem(dlg);
    }
    QList<HbListWidgetItem*> result;
    QList<int> selIndexes;
    selIndexes << current;
    if(!label.isNull())
        dlg->setHeadingWidget(new HbLabel(label));
    dlg->setWidgetItems(list,false,current);
    dlg->setSelectionMode(mode);
    dlg->setSelectedItems(selIndexes);
    HbAction* action = dlg->exec();
    if(action == dlg->secondaryAction()){ //Cancel was pressed
        if(ok)
            *ok = false;
    }
    else{ //OK was pressed
        if(ok)
            *ok = true;
        selIndexes = dlg->selectedItems();
    }
	for(int i = 0; i < selIndexes.count(); i++ ){
		result.append(list.at(selIndexes[i]));	
	}
	
	delete dlg;

	return result;
}

/*!
    \deprecated HbListDialog::getModelIndexes(const QString &, QAbstractItemModel* ,bool *,HbAbstractItemView::SelectionMode , QGraphicsScene *, QGraphicsItem *)
        is deprecated. Use the other available HbListDialog::getModexIndexes(...) API.

    Static convenience function to let the user select item(s) from a
    list of items defined by a user set model.\a label is the text which 
    is shown to the user (it should say what should be entered). \a model 
    is user defined model which list will use to render the items.
	Ownership is not transferred.
    If \a ok is non-null \e *\a ok will be set to true if the user
    pressed \gui OK and to false if the user pressed \gui Cancel. \a mode
    defines the selection mode of the list. It can be either of \NoSelection,
    \SingleSelection and \MultiSelection. 
    The dialog's parent is \a parent. The dialog will be modal.

    This function returns the model indexes selcted items in the list.
    List will contain only one item if \a SelectionMode is \a SingleSelection or
    \NoSelection and may contain more items in case of \a MultipleSelection.
*/
QModelIndexList HbListDialog::getModelIndexes(const QString &label, QAbstractItemModel* model,bool *ok,HbAbstractItemView::SelectionMode mode, QGraphicsScene *scene, QGraphicsItem *parent)
{
    HB_DEPRECATED("HbListDialog::getModelIndexes is deprecated. Use HbListDialog::getModelIndexes(const QString &, QAbstractItemModel*,QObject *,const char *,HbAbstractItemView::SelectionMode, QGraphicsScene *, QGraphicsItem *)");

    Q_UNUSED(parent)
    HbListDialog *dlg = new HbListDialog();
    if (scene) {
        scene->addItem(dlg);
    }
    QModelIndexList result;
        if(!label.isNull())
            dlg->setHeadingWidget(new HbLabel(label));
	dlg->setModel(model);
	dlg->setSelectionMode(mode);
	HbAction* action = dlg->exec();
	if(action == dlg->secondaryAction()){ //Cancel was pressed
		if(ok)
			*ok = false;
	}
	else{ //OK was pressed
		if(ok)
			*ok = true;
		result = dlg->selectedModelIndexes();
	}
	dlg->setModel(0);
	delete dlg;
	return result;
}

#include "moc_hblistdialog.cpp"
