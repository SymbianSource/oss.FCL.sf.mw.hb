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
#include "hbcolorgridviewitem_p.h"
#include <hbstyleoptioncolorgridviewitem.h>
#include <hbgridviewitem_p.h>
#include <hbcolorscheme.h>
#include <hbiconitem.h>

class HbColorGridViewItemPrivate: public HbGridViewItemPrivate
{
    Q_DECLARE_PUBLIC( HbColorGridViewItem )

public:
    explicit HbColorGridViewItemPrivate(HbAbstractViewItem *prototype);
    ~HbColorGridViewItemPrivate();

    void createPrimitives();
    void updatePrimitives();
    void updateChildItems();

    bool isChecked() const;
    bool isNoneBlock() const;

    // center of color field, colored according to color
    QGraphicsItem *mColorItem;
    // borders of color field, beneath mColorItem, or 'none' block
    QGraphicsItem *mBorderItem;
    // selection indication
    QGraphicsItem *mCheckMarkItem;

private:
    static HbColorGridViewItemPrivate *d_ptr(HbColorGridViewItem *item)
    {
        Q_ASSERT(item);
        return item->d_func();
    } 

};

HbColorGridViewItemPrivate::HbColorGridViewItemPrivate(HbAbstractViewItem *prototype) 
    : HbGridViewItemPrivate(prototype),
      mColorItem(0),
      mBorderItem(0),
      mCheckMarkItem(0)
{
}

HbColorGridViewItemPrivate::~HbColorGridViewItemPrivate()
{
}

bool HbColorGridViewItemPrivate::isChecked() const
{
    return mIndex.data(Qt::CheckStateRole).value<bool>();
}

bool HbColorGridViewItemPrivate::isNoneBlock() const
{
    return mIndex.data(HbColorGridViewItem::ColorRole).value<bool>();
}

void HbColorGridViewItemPrivate::createPrimitives()
{
    Q_Q( HbColorGridViewItem );

    if (!mBorderItem ) {
        mBorderItem = q->style()->createPrimitive(HbStyle::P_ColorGridViewItem_borderIcon, q);
        static_cast<HbIconItem*>(mBorderItem)->setFlags(HbIcon::Colorized);
        q->style()->setItemName( mBorderItem, "cg-border-icon" );
     }
    if (!mColorItem && !isNoneBlock()) {
        mColorItem = q->style()->createPrimitive(HbStyle::P_ColorGridViewItem_colorIcon, q);
        static_cast<HbIconItem*>(mColorItem)->setFlags(HbIcon::Colorized);
        q->style()->setItemName( mColorItem, "cg-color-icon" );
    }
    if(!mCheckMarkItem) {
            mCheckMarkItem = q->style()->createPrimitive(HbStyle::P_ColorGridViewItem_checkIcon, q);
            q->style()->setItemName( mCheckMarkItem, "cg-selection-icon" );
    }
}

void HbColorGridViewItemPrivate::updatePrimitives()
{  
    Q_Q( HbColorGridViewItem );

    HbStyleOptionColorGridViewItem option;
    q->initStyleOption(&option);
   
    if (mBorderItem) {
        q->style()->updatePrimitive(mBorderItem, HbStyle::P_ColorGridViewItem_borderIcon, &option);
    }

    if (mColorItem) {
        q->style()->updatePrimitive(mColorItem, HbStyle::P_ColorGridViewItem_colorIcon, &option);
    }

    if (mCheckMarkItem) {
        q->style()->updatePrimitive(mCheckMarkItem, HbStyle::P_ColorGridViewItem_checkIcon, &option);
        if ( isChecked() ) {
            mCheckMarkItem->show();
        } else {
            mCheckMarkItem->hide();
        }
    }
    
}

void HbColorGridViewItemPrivate::updateChildItems()
{
    createPrimitives();
    updatePrimitives();		
}

HbColorGridViewItem::HbColorGridViewItem(QGraphicsItem *parent) :
    HbGridViewItem(*new HbColorGridViewItemPrivate(this), parent)
{
    Q_D( HbColorGridViewItem );
    d->q_ptr = this;   
}


HbColorGridViewItem::HbColorGridViewItem(HbColorGridViewItemPrivate &dd, QGraphicsItem *parent) :
    HbGridViewItem(dd, parent)
{
    Q_D( HbColorGridViewItem );
    d->q_ptr = this; 
}

/*!
 Copy constructor
 */
HbColorGridViewItem::HbColorGridViewItem(const HbColorGridViewItem &source) :
    HbGridViewItem(*new HbColorGridViewItemPrivate(*source.d_func()),
            source.parentItem())
{
    Q_D( HbColorGridViewItem );
    d->q_ptr = this;  
}


HbColorGridViewItem::~HbColorGridViewItem()
{
}

HbAbstractViewItem* HbColorGridViewItem::createItem()
{
    return new HbColorGridViewItem(*this);
}

void HbColorGridViewItem::updateChildItems()
{
    Q_D( HbColorGridViewItem );
    d->updateChildItems();
    HbGridViewItem::updateChildItems();
}

void HbColorGridViewItem::updatePrimitives()
{
    Q_D( HbColorGridViewItem );
    d->updatePrimitives();
    HbGridViewItem::updatePrimitives();
}


QGraphicsItem * HbColorGridViewItem::primitive(HbStyle::Primitive primitive) const
{
    Q_D( const HbColorGridViewItem );

    if (primitive == HbStyle::P_ColorGridViewItem_checkIcon) {
        return d->mCheckMarkItem;
    }

    if (primitive == HbStyle::P_ColorGridViewItem_colorIcon) {
        return d->mColorItem;
    }

    if (primitive == HbStyle::P_ColorGridViewItem_borderIcon) {
        return d->mBorderItem;
    }

    return HbGridViewItem::primitive(primitive);
}

void HbColorGridViewItem::initStyleOption(HbStyleOptionColorGridViewItem *option) const
{
    Q_D( const HbColorGridViewItem ); 

    if( d->isNoneBlock() && d->mBorderItem ) {
        option->borderIcon = "qtg_graf_colorpicker_empty";
    } else {
        option->borderIcon = "qtg_graf_colorpicker_filled";
    }
    
    option->color = d->mIndex.data(HbColorGridViewItem::ColorRole).value<QColor>(); 
    option->borderColor = HbColorScheme::color("qtc_popup_grid_normal");
    HbGridViewItem::initStyleOption(option);
}

void HbColorGridViewItem::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    HbGridViewItem::resizeEvent(event);
    updatePrimitives();
}
