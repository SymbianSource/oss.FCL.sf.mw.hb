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

#include "hbcolordialog.h"
#include "hbcolordialog_p.h"
#include "hbcolorgridviewitem_p.h"
#include <hbgridview.h>

#include <QStandardItemModel>
#include <QGraphicsScene>

class HbColorDialogContentWidget : public HbWidget
{
    Q_OBJECT

public:
    HbGridView *mGridView;
    HbColorDialogPrivate *d;
    enum { Type = HbPrivate::ItemType_ColorDialogContentWidget };
    int type() const { return Type; }

    HbColorDialogContentWidget( HbGridView* gridView, 
                                HbColorDialog *q,
                                HbColorDialogPrivate *priv, 
                                QGraphicsItem* parent = 0 ) 
        : HbWidget(parent), mGridView(gridView), d(priv)
    {
        mGridView = createColorGrid(q, SLOT(_q_setCurrentSelectedColor(QModelIndex)));
        mGridView->setVerticalScrollBarPolicy(HbScrollArea::ScrollBarAsNeeded);
        HbStyle::setItemName(this, "this");
        d->mGridView = mGridView;
    }
    
    HbGridView* createColorGrid(QObject *receiver, const char *method)
    {
        mGridView = new HbGridView(this); 
        mGridView->setVerticalScrollBarPolicy(HbScrollArea::ScrollBarAsNeeded); 
        HbStyle::setItemName(mGridView, "color_grid_view");
       
        mGridView->setColumnCount(4);
        mGridView->setItemPrototype(new HbColorGridViewItem(mGridView));
        if ( receiver ) {
            QObject::connect(mGridView, SIGNAL(activated(QModelIndex)), receiver, method);
        }
        mGridView->setClampingStyle(HbScrollArea::StrictClamping);
        return mGridView;
        
    }
};


HbColorDialogPrivate::HbColorDialogPrivate() 
  : HbDialogPrivate(),
    mContentWidget(0),
    mGridView(0),
    mCurrentItem(0),
    mSelectedColor(QColor()),
    mInitialColor(QColor()),
    mColorModel(0),
    mPolished(false),
    mNoneBlockVisible(false),
    mNoneBlockSelected(false),
    mDefaultColors(true)

{
}

HbColorDialogPrivate::~HbColorDialogPrivate()
{
    delete mColorModel;
}

void HbColorDialogPrivate::init()
{
    Q_Q(HbColorDialog);
    mContentWidget = new HbColorDialogContentWidget(mGridView ,q, this);
    
    q->setContentWidget(mContentWidget);
    q->setDismissPolicy(HbPopup::TapOutside);
    q->setTimeout(HbPopup::NoTimeout);
    q->setBackgroundFaded(false);
    q->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
    mColorModel=new QStandardItemModel();
    mGridView->setModel(mColorModel);
}

void HbColorDialogPrivate::_q_setCurrentSelectedColor(const QModelIndex &index)
{
    Q_Q(HbColorDialog);

    QColor color = index.data(HbColorGridViewItem::ColorRole).value<QColor>();

    HbColorGridViewItem *previousItem = mCurrentItem;

    if(previousItem){
        mColorModel->itemFromIndex(previousItem->modelIndex())->setData(false,Qt::CheckStateRole);
        previousItem->updatePrimitives();
    }

    mCurrentItem = qgraphicsitem_cast<HbColorGridViewItem*>(mGridView->itemByIndex(index));

    if(mCurrentItem){
        mColorModel->itemFromIndex(mCurrentItem->modelIndex())->setData(true,Qt::CheckStateRole);
        mCurrentItem->updatePrimitives();
    }

    if(color.isValid()) {
        mSelectedColor = color;
        emit q->colorSelected(color);
    }
    else {
        mSelectedColor = QColor();
        emit q->noneSelected();
    }
    emit q->finished( mSelectedColor, !color.isValid() );

    q->close();
}

void HbColorDialogPrivate::noneSelected()
{
    Q_Q(HbColorDialog);
    mNoneBlockSelected = true;
    emit q->noneSelected();
    q->close();
}

/*!
  This method creates the items for the colorlist and appends items to model.
*/
void HbColorDialogPrivate::addDatatoModel(const QList<QColor> &colorList)
{
    Q_ASSERT(mColorModel);

    QStandardItem *item=NULL;

    for ( int i=0, count=colorList.count(); i < count; i++ ) {
        item = new QStandardItem();
        QMap<int, QVariant> data;
        data.insert(HbColorGridViewItem::ColorRole, colorList.at(i));
        mColorModel->appendRow(item);
        mColorModel->setItemData(mColorModel->indexFromItem(item), data);
    }
}

/*!
  @beta
  @hbwidgets
  \class HbColorDialog
  \brief HbColorDialogPrivate provides the popup widget for color selection.

  The user can launch color dialog and choose one of the color from it.
  For example , colordialog can be used to change the background color in an application.

  The static functions are convenience methods to launch the dialog and choose the color.

  By default, colors provided by graphics team will be shown in the dialog.
  Application can override default colors and choose its own set of colors to be shown.

  Below is the example to add application colors.

  \code
  HbColorDialog dlg;

  QList<QColor> list;
  list << Qt::white << Qt::red;

  dlg.addColor(Qt::black);
  dlg.addColors(list);
    
  dlg.show(...);
  \endcode

  NoneBlock is displayed in colordialog and user can choose None explicitly.
  NoneBlock visibilty is configurable by applicaion.

  \sa HbDialog

*/

/*!
  @beta
  \fn void colorSelected(const QColor &color)

  This signal is emitted when the ColorDialog is about to close, after clicking the color.
  \a color  selected color by User.

*/

/*!
  @beta
  \fn void noneSelected()

  This signal is emitted when the User has clicked on NoneBlock. 

*/

/*!
  @beta
  Constructs a new HbColorDialog with \a parent

  \param parent An optional parameter.

*/

HbColorDialog::HbColorDialog(QGraphicsItem *parent) :
    HbDialog(*new HbColorDialogPrivate, parent)
{
    Q_D(HbColorDialog);
    d->init();
}

HbColorDialog::HbColorDialog(HbColorDialogPrivate &dd, QGraphicsItem *parent) :
    HbDialog(dd, parent)
{
    Q_D(HbColorDialog);
    d->init();
}

/*!
  @beta
  Sets the \a initialColor to be shown on dialog's launch. This is indicated by tick mark graphics on dialogs launch.
  \sa currentColor

*/
void HbColorDialog::setInitialColor(const QColor &initialColor)
{
    Q_D(HbColorDialog);
    d->mInitialColor = initialColor;

}

/*!
  @beta
  Sets the \a initialColor to be shown on dialog's launch. This is indicated by tick mark graphics on dialogs launch.
  \sa currentColor

*/
QColor HbColorDialog::initialColor() const
{
    Q_D(const HbColorDialog);
    return d->mInitialColor;
}

/*!
  @beta
  Returns the last Selected Color.
  The Default value will be invalid Color ( QColor() ).
  \sa setInitialColor
    
*/
QColor HbColorDialog::currentColor() const
{
    Q_D(const HbColorDialog);
    return d->mSelectedColor;
}

/*!
  @beta
  Adds the \a color specified by application.
*/
void HbColorDialog::addColor(const QColor &color)
{
    Q_D(HbColorDialog);
    if(!(color.isValid()))
        return;

    if(d->mDefaultColors){
        d->mColorModel->clear();
    }

    QStandardItem *item = new QStandardItem();
    QMap<int, QVariant> data;
    data.insert(HbColorGridViewItem::ColorRole, color);

    d->mColorModel->appendRow(item);
    d->mColorModel->setItemData(d->mColorModel->indexFromItem(item), data);

    d->mDefaultColors=false;
}

/*! 
  @beta
  Adds list of \a colorList colors specified by application.
*/

void HbColorDialog::addColors(const QList<QColor> &colorList)
{
    Q_D(HbColorDialog);

    if(colorList.empty())
        return;

    if(d->mDefaultColors){
        d->mColorModel->clear();
    }

    d->addDatatoModel(colorList);
    d->mDefaultColors=false;
}

/*!
  @beta
  Set visibility of NoneBlock. 
*/

void HbColorDialog::setNoneBlockVisible(bool visible)
{
    Q_D(HbColorDialog);
    if ( d->mNoneBlockVisible == visible ) {
        return;
    }
    d->mNoneBlockVisible = visible;

    if ( visible ) {
        QStandardItem *item = new QStandardItem();
        item->setData(true, HbColorGridViewItem::ColorRole);
        d->mColorModel->insertRow(0,item);
    } else {
        d->mColorModel->removeRow(0);
    }
}

/*!
  @beta
  Returns visibility of NoneBlock.
  Default value is false.
*/

bool HbColorDialog::isNoneBlockVisible() const
{
    Q_D(const HbColorDialog);
    return d->mNoneBlockVisible;
}

/*!
  @beta
  Returns whether None is selected or not.
*/

bool HbColorDialog::isNoneSelected() const
{
    Q_D(const HbColorDialog);
    return d->mNoneBlockSelected;
}

/*!
  @alpha
  Launches color dialog to allow the user to choose a color.

  The \a colorList is list of colors specified by application to be shown in dialog.

  If \a initialColor is the default color to be highlighted on dialog's launch.By default first color in dialog is highlighted.
     
  The \a receiver and \a member argument specifies QObject and slot to call 
  when color is selected. Connection is done and disconnected automatically.

  If \a isNoneBlockVisible is specified by application for NoneBlock visibility. 

  \a isNoneSelected is out parameter, indicates whether none is selected or not.

  \a parent is the parent of the dialog.
    
*/

void HbColorDialog::getColor( const QList<QColor> &colorList, 
                              const QColor &initialColor,
                              QObject* receiver,
                              const char* member,
                              bool isNoneBlockVisible, 
                              QGraphicsScene *scene, 
                              QGraphicsItem *parent )
{
    HbColorDialog* dlg = new HbColorDialog(parent);

    if (scene && !parent) {
        scene->addItem(dlg);
    }

    dlg->addColors(colorList);
    dlg->setInitialColor(initialColor);
    dlg->setNoneBlockVisible(isNoneBlockVisible);
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);
    if ( receiver && member ) {
        connect(dlg, SIGNAL(finished(QColor,bool)), receiver, member);
    }
    dlg->open();
}

/*!
  @beta
  reimplements the showEvent.
  Its required to indicate the highlighted item.
*/
void HbColorDialog::showEvent(QShowEvent *event)
{
    Q_D(HbColorDialog);
    d->mGridView->setModel(d->mColorModel);
    HbDialog::showEvent(event);
}

void HbColorDialog::polish(HbStyleParameters &params)
{
    Q_D(HbColorDialog);
    const int colors = 16;
    const int rmask = 0xff0000;
    const int gmask = 0xff00;
    const int bmask = 0xff;
    const QString color = "color%1";
    for ( int i=0; i < colors; i++ ) {
        params.addParameter( color.arg(i) );
    }

    style()->setItemName( this, "HbColorDialog" );
    HbDialog::polish(params);

    if (!d->mPolished) {
        d->mPolished = true;
        for ( int i=0; i < colors; i++ ) {
            unsigned int c(0);
            unsigned int r(0);
            unsigned int g(0);
            unsigned int b(0);
            
            if ( params.value(color.arg(i)).isValid() ) {
                c = params.value(color.arg(i)).toInt();
                r = (c & rmask) >> 16;
                g = (c & gmask) >> 8;
                b =  c & bmask;
                QColor color(r,g,b);
                if ( color.isValid() ) {
                    d->mDefaultColorList << color;
                }
            }
        }

        if ( d->mDefaultColors ) {
            d->addDatatoModel( d->mNoneBlockVisible ?
                               d->mDefaultColorList.mid(0, (unsigned int)d->mDefaultColorList.count() -1 ) :
                               d->mDefaultColorList );
        }
    }    
    // check, if user has set a default color
    QModelIndex currentIndex = d->mColorModel->item(0)->index();
    if ( !d->mInitialColor.isValid() && d->mNoneBlockVisible ) {  // use QColor() for noneblock
        d->mColorModel->item(0)->setData(true,Qt::CheckStateRole);
    } else {
        for(int i=0, e=d->mColorModel->rowCount(); i<e; i++) {
            QStandardItem* item= d->mColorModel->item(i);
            QColor color= item->data(HbColorGridViewItem::ColorRole).value<QColor>();
            
            if(color==d->mInitialColor){
                item->setData(true,Qt::CheckStateRole);
                currentIndex = item->index();
            } else {
                item->setData(false,Qt::CheckStateRole);
            }
        }
    }
    d->mCurrentItem = qgraphicsitem_cast<HbColorGridViewItem*>(d->mGridView->itemByIndex(currentIndex));
}


#include "moc_hbcolordialog.cpp"
#include "hbcolordialog.moc"

