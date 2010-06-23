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

#include "hblistwidget.h"
#include "hblistwidget_p.h"

#include "hblistmodel_p.h"
#include <hblistviewitem.h>
#include <hblistwidgetitem.h>
#include "hblistitemcontainer_p.h"
#include "hbmodeliterator.h"

#include <QModelIndex>
#include <hbicon.h>
#include <hbframedrawer.h>
#include <hbscrollbar.h>

#include <QDebug>

/*!
    @beta
    @hbwidgets
    \class HbListWidget
    \brief The HbListWidget class is for simple lists consisting of single row items of an icon and some text, and together with HbListWidgetItem consisting of icon and at most two-row text items.

    
    HbListWidget provides a convenience API for adding, modifying, and removing single row items in simple lists. Single row items consist of some text and an icon in one line. If you want list items with two texts or two icons, that is, with multirow items, then you can create HbListWidgetItem objects and add them to an HbListWidget. See HbListWidgetItem for the detailed description of the multirow item. 

     HbListWidget model supports arranging items with drag and drop.
     
     \link HbAbstractItemView::setModel(QAbstractItemModel *model, HbAbstractViewItem *prototype=0) SetModel()\endlink method of the base class HbAbstractItemView cannot be used. HbListWidget cannot be used for list views with a directory model (QDirModel), for example.
    
    \section _usecases_hblistwidget Using the HbListWidget class
    
    \subsection _uc_hblistwidget_001 Creating a list and adding one single row item into it.
    
    The following code snippet first creates a list and then adds one item which contains some text and an icon to the list.
   
    \snippet{unittest_hblistwidget.cpp,1}

    \sa HbListWidgetItem, HbListView for more use cases.
*/

/*!
    \fn void HbListWidget::pressed(HbListWidgetItem *item)

    This signal is emitted when the user presses an item in the list. \a item indicates the pressed item.

    \sa released(), longPressed(), activated()
*/

/*!
  \fn void HbListWidget::released(HbListWidgetItem *item)

    This signal is emitted when when release event occurs for \a item.

    \sa pressed(), longPressed(), activated()
    \sa HbAbstractViewItem::released(const QPointF &position)
*/

/*!
    \fn void HbListWidget::activated(HbListWidgetItem *item)
    
    This signal is emitted when the user activates an item in the list. \a item indicates the activated item. The activation of the item depends on the input method. An item can be activated in the following ways:
    - by clicking the item with a mouse
    - by tapping the item on a touch screen
    - by pressing Return or Enter hardware keys when the item has focus
    - by pressing Select soft key when the item has focus
    
    \sa pressed(), released(), longPressed()
*/

/*!
    \fn void HbListWidget::longPressed(HbListWidgetItem *item, const QPointF &coords)

    This signal is emitted when the user presses an item in the list for a long time. \a item indicates the activated item and \a coords the screen position where the long press happened.

    \sa pressed(), released(), activated()
*/

/*!
    Constructs a list widget with \a parent.
*/
HbListWidget::HbListWidget(QGraphicsItem *parent)
    : HbListView( *new HbListWidgetPrivate, new HbListItemContainer, parent )
{
    Q_D( HbListWidget );
    d->q_ptr = this;
    d->init( new HbListModel(this) );
    HbListView::setModel(d->mListModel);

    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(_q_itemActivated(QModelIndex)));  
    connect(this, SIGNAL(pressed(QModelIndex)), this, SLOT(_q_itemPressed(QModelIndex)));
    connect(this, SIGNAL(released(QModelIndex)), this, SLOT(_q_itemReleased(QModelIndex)));
    connect(this, SIGNAL(longPressed(HbAbstractViewItem*, QPointF)), this, SLOT(_q_itemLongPress(HbAbstractViewItem*, QPointF)));
}

/*!
    Destructor.
*/ 
HbListWidget::~HbListWidget()
{
}

/*!
    Returns the total number of rows in the list.
*/
int HbListWidget::count() const
{
    return modelIterator()->indexCount();
}

/*!
    Returns the current row number in the list. A current row is a row having focus.
*/
int HbListWidget::currentRow() const
{
    return modelIterator()->indexPosition(currentIndex());
}

/*!
    Sets the current row to \a row. A current row is a row having focus.
    
    \sa currentRow()
*/
void HbListWidget::setCurrentRow(int row)
{
    Q_D(HbListWidget);
    setCurrentIndex(d->mModelIterator->index(row));
}

/*!
    Returns a pointer to the current list item.  A current list item is a list item having focus.
    Ownership is not transferred so the item should not be deleted by the caller.
    
    \sa currentRow()
*/
HbListWidgetItem *HbListWidget::currentItem() const 
{
    Q_D( const HbListWidget );
    return d->mListModel->item(modelIterator()->indexPosition(currentIndex()));
}

/*!
    Sets the current item to \a item. A current item is an item having focus.
*/
void HbListWidget::setCurrentItem (HbListWidgetItem *item)
{
    setCurrentRow(row(item));
}

/*!
    Returns a pointer to the list item specified by \a row.
    Ownership is not transferred to the caller so the item should not be deleted by the caller.
*/
HbListWidgetItem *HbListWidget::item (int row) const
{
    Q_D( const HbListWidget );
    return d->mListModel->item(row);
}

/*!
    Creates a single row item specified by \a icon and \a text and appends it to the end of the list.
*/
void HbListWidget::addItem (const HbIcon &icon, const QString &text)
{
    Q_D( HbListWidget );

    HbListWidgetItem *item = new HbListWidgetItem();
    item->setText(text);
    item->setIcon(icon);
    d->mListModel->appendRow(item);
}

/*!
    Creates a new single row item spcified by \a text and appends it to the end of the list.
*/ 
void HbListWidget::addItem (const QString &text)
{
    Q_D( HbListWidget );

    HbListWidgetItem *item = new HbListWidgetItem();
    item->setText(text);
    d->mListModel->appendRow(item);
}

/*!
   Appends \a item to the end of the list.
*/
void HbListWidget::addItem (HbListWidgetItem *item)
{
    Q_D( HbListWidget );

    if (item) {
        d->mListModel->appendRow(item);
    }
}

/*!
    Creates a list widget item with \a icon and \a text and inserts it into the list at the \a row location. Regarding \a row the following rules apply:
    - if \a row is negative the item is inserted into the first row
    - if \a row is greater than the number of list's items the item is added at the last item
*/
void HbListWidget::insertItem (int row, const HbIcon &icon, const QString &text)
{
    Q_D( HbListWidget );

    //Do not enter parent in constr!   
    HbListWidgetItem *item = new HbListWidgetItem();
    item->setText(text);
    item->setIcon(icon);
    d->mListModel->insert(row, item);
}

/*!
    Creates a list widget item with \a text and inserts it into the list at the \a row location. Regarding \a row the following rules apply:
    - if \a row is negative the item is inserted into the first row
    - if \a row is greater than the number of list's items the item is added at the last item
*/
void HbListWidget::insertItem (int row, const QString &text)
{
    Q_D( HbListWidget );

    //Do not enter parent in constr! 
    HbListWidgetItem *item = new HbListWidgetItem();
    item->setText(text);
    d->mListModel->insert(row, item);
}

/*!
    Creates a list widget item with \a item and inserts it into the list at the \a row location. Regarding \a row the following rules apply:
    - if \a row is negative the item is inserted into the first row
    - if \a row is greater than the number of list's items the item is added at the last item
*/
void HbListWidget::insertItem (int row, HbListWidgetItem *item)
{
    Q_D( HbListWidget );

    if (item) {
        d->mListModel->insert(row, item);
    }
}

/*!
    Deletes all the items in the list.     
*/
void HbListWidget::clear() 
{
    Q_D( HbListWidget );

    if (d->mListModel) {
        d->mListModel->clear();
    }
}

/*!
    Returns the row number of given \a item.
*/
int HbListWidget::row(const HbListWidgetItem *item) const
{
    Q_D( const HbListWidget );

    return d->mListModel->index(const_cast<HbListWidgetItem *>(item)).row();
}

/*!
    This method cannot be used. If you call it in debug builds you will get an ASSERT failure i.e. Q_ASSERT_X(). In release builds nothing will happen.
*/
void HbListWidget::setModel(QAbstractItemModel *model, HbAbstractViewItem *prototype)
{
    Q_UNUSED(model);
    Q_UNUSED(prototype);
    Q_ASSERT_X(false, "HbListWidget::setModel()", "Model cannot be set by user if convenience API is used");
}


/*!
    Replaces the text on the given \a row with the given \a text. Only the text is replaced, the icon stays unchanged. In case of a multi-row only the first text row is replaced. Use HbListWidgetItem to modify multirow items.
    
    \param text - Text to be shown in the list widget item.
    \param row - Row of the list widget item.
    
*/
void HbListWidget::setText(int row, const QString &text)
{
    Q_D( const HbListWidget );

    HbListWidgetItem *item = d->mListModel->item(row);
    item->setText(text);
}

/*!
    Replaces the icon on the given \a row with the given \a icon. Only the icon is replaced, the text stays unchanged. The item can be a single row or a multirow item. Use HbListWidgetItem to modify multirow items.
    
    \param icon - Icon to be shown in the list widget item.
    \param row - Row of the list widget item.
*/
void HbListWidget::setIcon(int row, const HbIcon& icon)
{
    Q_D( const HbListWidget );

    HbListWidgetItem *item = d->mListModel->item(row);
    item->setIcon(icon);
}

/*!
    Returns the list widget item of given row and then removes the item from the list. Items removed from a list must be deleted manually.
    
    \param row - Row to be removed. 

    \return
    - The list widget item of \a row if it exists in the list.
    - 0, if \a row doesn't exist in the list.
*/
HbListWidgetItem *HbListWidget::takeItem(int row)
{
    Q_D( const HbListWidget );

    if (row < 0 || row >= d->mListModel->rowCount())
        return 0;
    return d->mListModel->take(row);
}

/*!
    HbListWidget supports arranging items with drag and drop. If \a arrangeMode is \c true and the list is not in the selection mode, the list is put into the arrange list mode which allows the user to rearrange the list by dragging and dropping items. If \a arrangeMode is \c false, then the arrange list mode is cancelled. 
    
    The list can be in either arrange or selection mode but not in both modes at any given time.
    
    \return
    - \c True if the arrange list mode is successfully set according to \a arrangeMode. This includes the case where the requested \a arrangeMode is the same as what was already set for the list.
    - \c False if the requested \a arrangeMode cannot be set.   
*/
bool HbListWidget::setArrangeMode(const bool arrangeMode)
{
    Q_D( HbListWidget );

    if (arrangeMode != d->mArrangeMode) {
        if (d->mSelectionMode != HbAbstractItemView::NoSelection) {
            return false;
        }
        d->arrangeModeSetup(arrangeMode);
    }
    return true;
}

/*!
    Moves an item from given source row to given target row. The operation is cancelled in the following cases:
    - Source and target rows are the same.
    - Either or both given rows do not exist in the list.
    
    \param from - Source row that is the row from where the item is moved.
    \param to - Target row that is the row to where the item is moved.
*/
 void HbListWidget::move(const QModelIndex &from, const QModelIndex &to)
{
    int fromRow = from.row();
    int toRow = to.row();

    if (from == to
        || fromRow < 0
        || toRow < 0
        || fromRow >= model()->rowCount()
        || toRow >= model()->rowCount()) {
        return;
    }

    HbListWidgetItem *item = takeItem(fromRow);
    insertItem(toRow, item);
}

/*!
    Constructs a list widget with private view widget \a dd and \a parent.
*/
HbListWidget::HbListWidget( HbListWidgetPrivate& dd, HbAbstractItemContainer *container, QGraphicsItem* parent ):
        HbListView( dd, container, parent )
{
    Q_D( HbListWidget );
    d->q_ptr = this;
    
    d->init( new HbListModel(this) );
    HbListView::setModel(d->mListModel);

    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(_q_itemActivated(QModelIndex)));  
    connect(this, SIGNAL(pressed(QModelIndex)), this, SLOT(_q_itemPressed(QModelIndex)));
    connect(this, SIGNAL(released(QModelIndex)), this, SLOT(_q_itemReleased(QModelIndex)));
    connect(this, SIGNAL(longPressed(HbAbstractViewItem*, QPointF)), this, SLOT(_q_itemLongPress(HbAbstractViewItem*, QPointF)));
}

#include "moc_hblistwidget.cpp"
