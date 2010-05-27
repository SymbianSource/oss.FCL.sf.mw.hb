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
    \brief HbListWidget represents a list

    This class has been provided with the class HbListWidgetItem as a convenience
    API. It provides an item-based interface for adding and removing items. 

    SetModel() function of the base class HbListView cannot be used. If the user wants to start 
    using a list view with e.g. a directory model (QDirModel) this convenience API cannot be used.
    
    HbListWidget can be used without HbListWidgetItem to populate 
    simple list with single row items consisting of a text and an icon. 

    If multirow items are needed the HbListWidgetItem must be used.

    \snippet{unittest_hblistwidget.cpp,1}

    See HbListWidgetItem for more usage examples.

    \sa HbListWidgetItem, HbListView 
*/

/*!
    \fn void HbListWidget::pressed(HbListWidgetItem *item)

    This signal is emitted when a list item is pressed.
    The pressed item is specified by \a item.

    See also released(), longPressed() and activated().
*/

/*!
  \fn void HbListWidget::released(HbListWidgetItem *item)

    This signal is emitted when a list item is no more pressed.
    The released item is specified by \a item.

    See also pressed(), longPressed() and activated().
*/

/*!
    \fn void HbListWidget::activated(HbListWidgetItem *item)

    This signal is emitted when the item specified by \a item is activated by the user.
    How to activate items depends on the input method; e.g., with mouse by clicking the item,
    or with touch input by tapping the item, or with hardware keys by pressing the Return
    or Enter key when the item is current, or with soft keys by pressing choosing "Select"
    when the item is current.

    See also pressed(), released() and longPressed().
*/

/*!
    \fn void HbListWidget::longPressed(HbListWidgetItem *item, const QPointF &coords)

    This signal is emitted when a list item is long pressed.
    The pressed item is specified by \a item and \a coords.
    See also pressed(), released() and activated().
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
    Destructs the list view.
 */ 
HbListWidget::~HbListWidget()
{
}

/*!
    Returns the total number of rows.
*/
int HbListWidget::count() const
{
    return modelIterator()->indexCount();
}

/*!
    Returns the current row number
 */
int HbListWidget::currentRow() const
{
    return modelIterator()->indexPosition(currentIndex());
}

/*!
   Sets current \a row. This function sets current index into selection model. 
   Selection is never changed.
 */
void HbListWidget::setCurrentRow(int row)
{
    Q_D(HbListWidget);
    setCurrentIndex(d->mModelIterator->index(row));
}

/*!
    Returns a pointer to the current HbListWidgetItem. 
    Ownership not transferred to the caller.
    Should not be deleted by the caller.
 */
HbListWidgetItem *HbListWidget::currentItem() const 
{
    Q_D( const HbListWidget );
    return d->mListModel->item(modelIterator()->indexPosition(currentIndex()));
}

/*!
    Sets the current item to item.
 */
void HbListWidget::setCurrentItem (HbListWidgetItem *item)
{
    setCurrentRow(row(item));
}

/*!
    Returns a pointer to an item specified by \a row.
    Ownership not transferred to the caller.
    Should not be deleted by the caller.
 */
HbListWidgetItem *HbListWidget::item (int row) const
{
    Q_D( const HbListWidget );
    return d->mListModel->item(row);
}

/*!
    Creates new one row item based on a string and an icon and append it to the end of the list.
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
    Creates new one row item based on a string and append it to the end of the list.
 */ 
void HbListWidget::addItem (const QString &text)
{
    Q_D( HbListWidget );

    HbListWidgetItem *item = new HbListWidgetItem();
    item->setText(text);
    d->mListModel->appendRow(item);
}

/*!
   Appends an item created by the user to the end of the list.
 */
void HbListWidget::addItem (HbListWidgetItem *item)
{
    Q_D( HbListWidget );

    if (item) {
        d->mListModel->appendRow(item);
    }
}

/*!
    Creates a new item based on the string and an icon into a specified location in the list            
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
    Creates a new item based on the string into a specified location in the list.
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
   Inserts an item created by the user to the specified place on the list.
 */
void HbListWidget::insertItem (int row, HbListWidgetItem *item)
{
    Q_D( HbListWidget );

    if (item) {
        d->mListModel->insert(row, item);
    }
}

/*!
    Deletes every item in the list     
 */
void HbListWidget::clear() 
{
    Q_D( HbListWidget );

    if (d->mListModel) {
        d->mListModel->clear();
    }
}

/*!
    Returns the row containing the given item.
 */
int HbListWidget::row(const HbListWidgetItem *item) const
{
    Q_D( const HbListWidget );

    return d->mListModel->index(const_cast<HbListWidgetItem *>(item)).row();
}

/*!
    \reimp

    Setting own model to HbListWidget is not allowed.
*/
void HbListWidget::setModel(QAbstractItemModel *model, HbAbstractViewItem *prototype)
{
    Q_UNUSED(model);
    Q_UNUSED(prototype);
    Q_ASSERT_X(false, "HbListWidget::setModel()", "Model cannot be set by user if convenience API is used");
}


/*!
    Set the text for a simple one row item or the first text row for a 
    multi row item. Only the text is replaced, no icon removed. 
    Use HbListWidgetItem to modify multirow items.
*/
void HbListWidget::setText(int row, const QString &text)
{
    Q_D( const HbListWidget );

    HbListWidgetItem *item = d->mListModel->item(row);
    item->setText(text);
}

/*!
    Set the icon for a simple one row item or the first icon for a 
    multi row item. Only the icon is replaced, no text removed.
    Use HbListWidgetItem to modify multirow items.
*/
void HbListWidget::setIcon(int row, const HbIcon& icon)
{
    Q_D( const HbListWidget );

    HbListWidgetItem *item = d->mListModel->item(row);
    item->setIcon(icon);
}

/*!
    Removes and returns the item from the given row in the list widget; otherwise returns 0.
    Items removed from a list widget will not be managed, and will need to be deleted manually.   
    Use this function to remove items form the list.    
*/
HbListWidgetItem *HbListWidget::takeItem(int row)
{
    Q_D( const HbListWidget );

    if (row < 0 || row >= d->mListModel->rowCount())
        return 0;
    return d->mListModel->take(row);
}

/*!
    HbListWidget model supports arranging items with drag and drop, so 
    this method returns true unless the widget is in some selection mode. 
*/
bool HbListWidget::setArrangeMode(const bool arrangeMode)
{
    Q_D( HbListWidget );

    if (arrangeMode != d->mArrangeMode) {
        if (arrangeMode == true) {
            if (d->mSelectionMode != HbAbstractItemView::NoSelection) {
                return false;
            }
            verticalScrollBar()->setInteractive(true);
        } else {
            verticalScrollBar()->setInteractive(false);
        }

        d->mArrangeMode = arrangeMode;

        if (d->mArrangeMode == true) {
            d->mOriginalFriction = d->mFrictionEnabled;
            setFrictionEnabled(false);
        } else {
            setFrictionEnabled(d->mOriginalFriction);
        }
    }
    return true;
}

/*!
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
