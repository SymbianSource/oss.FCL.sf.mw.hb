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
#include "hbradiobuttonlist.h"

#include "hblistview_p.h"
#include "hblistviewitem.h"
#include "hblistitemcontainer_p.h"
#include "hbradiobuttonlistviewitem_p.h"
#include "hbmodeliterator.h"

#include <QStringListModel>
#include <QGraphicsSceneMouseEvent>

//TODO: consider moving these two to proper place (widget params?)
const int HB_RADIOBUTTONLIST_ITEM_ACTIVATION_TIMER = 150;
const int HB_RADIOBUTTONLIST_PREVIEW_TIMER = 500;

/*!
    \class HbRadioButtonList
    \brief HbRadioButtonList provides a widget for a string list with radio buttons
    @alpha
    @hbwidgets

    This class has been provided for convenience for a simple creation of a radio button list.
    The same functionality could be implemented using HbListWidget or HbListView API.

    The Radio Button List consists of items with a radio button and a line of text.
    Only one item may be selected at a time. Once an item is selected, it can be deselected only by 
    selecting another item.  Initial item selection may be set at the list construction. If not 
    set, the list is shown without a selection.

    The Radio Button List has two working modes, Preview Mode and No Preview Mode. In No Preview 
    Mode it emits activated signal after the first tap to the item. In Preview Mode the first tap
    causes emission of the preview signal and the item is not yet selected. The second consecutive 
    tap on the item causes the activation of the item. The application has the responsibility of 
    the preview itself. 

    User does not need to set the model using base class function setModel() because the model is 
    created internally. If necessary the model can be changed e.g. to QDirModel. A use case for 
    this is to view a contents of a folder containing sound clips. Application can then play
    a preview of these sound clips based on the signal emitted by the radio button list.

    An example of how to create a radiobuttonlist inside a popup.
    \snippet{decoratorlistdemo/contentwidget.cpp,3}

    By default the radio button list has uniformItemSizes property set to true.

*/

/*!
    \fn void HbRadioButtonList::itemSelected(int index)

    This signal is emitted when a list item is selected by user action.
    
    The pressed item is specified by \a index.

*/

/*!
    \fn void HbRadioButtonList::startPreview(int index)

    This signal is emitted when a list item is pressed the first time when the PreviewMode 
    is set to Preview. The user of this API should use this signal to start application 
    specific preview.

    If the preview mode is NoPreview this signal is not emitted.

    The pressed item is specified by \a item.
*/

/*!
    \enum HbRadioButtonList::PreviewMode

    The preview mode defines whether the list is in the preview mode or not. 
*/

/*!
    \var HbRadioButtonList::Preview

    In this mode the application is signaled to show a preview. 
    This signal is emitted after a short delay after the list item has been pressed.
    The reason behing the short delay is that the user could select the item without seeing or 
    listening the start of the preview. 
*/

/*!
    \var HbRadioButtonList::NoPreview

    In this mode the application is not signaled for a preview.
*/

class HbRadioButtonListPrivate: public HbListViewPrivate {
    Q_DECLARE_PUBLIC(HbRadioButtonList)
public:
    HbRadioButtonListPrivate();
    ~HbRadioButtonListPrivate();

    void init(QAbstractItemModel *model, int selected=-1, HbRadioButtonList::PreviewMode previewMode=HbRadioButtonList::NoPreview);
    void emitStartPreview(int row);
    void calculateItemHeight();
    void _q_itemActivated(const QModelIndex &modelIndex);
    void _q_itemActivationTimerExpired();
    void _q_itemPreviewTimerExpired();

    QModelIndex mCurrentIndex; //Note that this is not the same as d->mHitItem
    HbRadioButtonList::PreviewMode mPreviewMode;
    bool mPreviewGoingOn;
    bool mStartUp;
};

HbRadioButtonListPrivate::HbRadioButtonListPrivate() :
    HbListViewPrivate(),
    mPreviewMode(HbRadioButtonList::NoPreview),
    mPreviewGoingOn(false),
    mStartUp(true)
{     
}

HbRadioButtonListPrivate::~HbRadioButtonListPrivate() 
{
}

void HbRadioButtonListPrivate::_q_itemActivated(const QModelIndex &modelIndex)
{
    Q_Q(HbRadioButtonList);
    emit q->itemSelected(modelIndex.row());
}

void HbRadioButtonListPrivate::_q_itemActivationTimerExpired()
{
    Q_Q(HbRadioButtonList);
    q->HbListView::emitActivated(mCurrentIndex);
}

void HbRadioButtonListPrivate::_q_itemPreviewTimerExpired()
{
    emitStartPreview(mCurrentIndex.row()); 
}

void HbRadioButtonListPrivate::init(QAbstractItemModel *model, int selected, 
                                    HbRadioButtonList::PreviewMode previewMode)
{
    Q_Q(HbRadioButtonList);
    q->setModel(model, new HbRadioButtonListViewItem());
    q->setSelectionMode(q->HbAbstractItemView::SingleSelection);
    q->setSelected(selected);   
    q->setPreviewMode(previewMode);
    q->setLongPressEnabled(false);
    q->setUniformItemSizes(true);
}

void HbRadioButtonListPrivate::emitStartPreview(int row)
{
    Q_Q(HbRadioButtonList);
    emit q->startPreview(row); 
}

/*!
    Constructs the radio button list with a given \a parent.
*/
HbRadioButtonList::HbRadioButtonList(QGraphicsItem *parent) :
    HbListView(*new HbRadioButtonListPrivate, new HbListItemContainer, parent)
{
    Q_D(HbRadioButtonList);
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(_q_itemActivated(QModelIndex))); 
    d->init(new QStringListModel(this));
}

/*!
    Constructs the radio button list with a given \a parent, initial \a list, pre-selected item \a selected and \a previewMode.
*/
HbRadioButtonList::HbRadioButtonList(const QStringList &list, int selected, PreviewMode previewMode, QGraphicsItem *parent) :
    HbListView(*new HbRadioButtonListPrivate, new HbListItemContainer, parent)
{
    Q_D(HbRadioButtonList);
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(_q_itemActivated(QModelIndex))); 
    d->init(new QStringListModel(list, this), selected, previewMode);
}

/*!
    \internal
*/
HbRadioButtonList::HbRadioButtonList(HbRadioButtonListPrivate &dd, HbAbstractItemContainer *container, QGraphicsItem *parent) :
    HbListView(dd, container, parent)
{
    Q_D(HbRadioButtonList);
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(_q_itemActivated(QModelIndex))); 
    d->init( new QStringListModel(this) );
}

/*!
    Destructs the radio button list.
*/
HbRadioButtonList::~HbRadioButtonList ()
{
}

/*!
    Replaces the whole list content with a new list \a list. This resets the selected property 
    to -1.
    This function does not work if the user has been chenged the model.
*/
void HbRadioButtonList::setItems(const QStringList &list)
{
    Q_D(HbRadioButtonList);
    QStringListModel *stringListModel = qobject_cast<QStringListModel *>(d->mModelIterator->model());
    if (stringListModel) {
        stringListModel->setStringList(list);
        updateGeometry();
    } else {
        //TODO: set the item here using the base model class
        //or document that this is not in use with own models
    }
}

/*!
    Returns the whole list as a QStringList.
    This function does not work if the user has been changed the model.
*/
QStringList HbRadioButtonList::items() const
{
    Q_D(const HbRadioButtonList);
    QStringListModel *stringListModel = qobject_cast<QStringListModel *>(d->mModelIterator->model());
    if (stringListModel)
        return stringListModel->stringList();
    else {
        //TODO: get the item here using the base model class
        //or document that this is not in use with own models
        return QStringList();
    }       
}

/*!
    Sets the selected item to be \a index.
*/
void HbRadioButtonList::setSelected(int index)
{
    Q_D(HbRadioButtonList);
    if(index < d->mModelIterator->indexCount()){
        setCurrentIndex(d->mModelIterator->index(index), QItemSelectionModel::SelectCurrent);
        if (d->mStartUp) {
           QModelIndex topIndex;
           if (index > 0) {
               topIndex = d->mModelIterator->index(index - 1);
           } else {
               topIndex = d->mModelIterator->index(index);
           }
           scrollTo(topIndex, HbAbstractItemView::PositionAtTop);
        }
    }
}

/*!
    Returns the index on selected item, -1 if there are no selected item.
*/
int HbRadioButtonList::selected() const
{
    return currentIndex().row(); 
}

/*!
    Sets the preview mode to be \a previewMode.
*/
void HbRadioButtonList::setPreviewMode(PreviewMode previewMode)
{
    Q_D(HbRadioButtonList);
    d->mPreviewMode=previewMode;
}

/*!
    Returns current preview mode.
*/
HbRadioButtonList::PreviewMode HbRadioButtonList::previewMode() const
{
    Q_D(const HbRadioButtonList);
    return d->mPreviewMode;
}

/*!
    \reimp
*/
int HbRadioButtonList::type() const
{
   return Hb::ItemType_RadioButtonList;
}

/*!
    \reimp 
*/
void HbRadioButtonList::emitActivated(const QModelIndex &modelIndex)
{
    Q_D( HbRadioButtonList );
     
    if(d->mPreviewMode == HbRadioButtonList::Preview && d->mCurrentIndex != modelIndex){
        QTimer::singleShot(HB_RADIOBUTTONLIST_PREVIEW_TIMER, this, SLOT(_q_itemPreviewTimerExpired()));          
        d->mCurrentIndex=modelIndex;
    }else {
        //Now we let the activate signal go to the application
        d->mCurrentIndex=modelIndex;
        QTimer::singleShot(HB_RADIOBUTTONLIST_ITEM_ACTIVATION_TIMER, this, SLOT(_q_itemActivationTimerExpired()));
    }
}

/*!
    \reimp
*/
void HbRadioButtonList::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbRadioButtonList);     
    if(d->mPreviewMode == HbRadioButtonList::Preview ){    
        HbAbstractViewItem *hitItem = d->itemAt(event->scenePos());
        if (hitItem){
            if (hitItem->modelIndex() == d->mCurrentIndex) {
                d->mPreviewGoingOn=false;
            } else {
                d->mPreviewGoingOn=true;
            }
        }
    }
    HbListView::mouseReleaseEvent(event);
}

/*!
    \reimp
*/
QItemSelectionModel::SelectionFlags HbRadioButtonList::selectionCommand(const HbAbstractViewItem *item,
                                                                          const QEvent *event)
{  
    Q_D(HbRadioButtonList);

    if (!item->modelIndex().isValid() 
        || selectionMode() != SingleSelection
        || event->type() != QEvent::GraphicsSceneMouseRelease){
            return QItemSelectionModel::NoUpdate;
    } else {
        if(d->mPreviewGoingOn==true) {
            return QItemSelectionModel::NoUpdate;
        } else {
            return QItemSelectionModel::ClearAndSelect;
        }
    }
}


/*!
    \reimp
*/
bool HbRadioButtonList::event(QEvent *e)
{
    Q_D(HbRadioButtonList);
    bool result = HbListView::event(e);
    if (e->type()==QEvent::LayoutRequest) {
        d->mStartUp=false;
    }
    return result;
}

/*!
    \reimp
*/
QSizeF HbRadioButtonList::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    return HbListView::sizeHint(which, constraint);
}

/*!
    \reimp
*/
void HbRadioButtonList::rowsInserted(const QModelIndex &parent, int start, int end)
{
    HbListView::rowsInserted(parent,start,end);
}

/*!
    \reimp
*/
void HbRadioButtonList::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    HbListView::rowsRemoved(parent,start,end);
}

/*!
    \reimp
*/
void HbRadioButtonList::reset()
{
    HbListView::reset();
}
#include "moc_hbradiobuttonlist.cpp"

