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
#include <hbgridviewitem_p.h>
#include <hbcolorscheme.h>
#include <hbiconitem.h>
#include <hbevent.h>
#include <hbstyleiconprimitivedata.h>

/*!
  \primitives
  \primitive{cg-color-icon} HbIconItem representing the icon colored with this color item's color.
  \primitive{cg-border-icon} HbIconItem representing borders of the color item.
  \primitive{cg-selection-icon} HbIconItem representing the check mark of a selected color item.
  \primitive{background} HbFrameBackground representing the background frame.
  */

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
    QGraphicsObject *mColorItem;
    // borders of color field, beneath mColorItem, or 'none' block
    QGraphicsObject *mBorderItem;
    // selection indication
    QGraphicsObject *mCheckMarkItem;
    // background frame, "grid"
    HbFrameBackground* mFrameBackGround;

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
      mCheckMarkItem(0),
      mFrameBackGround(0)
{
}

HbColorGridViewItemPrivate::~HbColorGridViewItemPrivate()
{
    delete mFrameBackGround;
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
        mBorderItem = q->style()->createPrimitive(HbStyle::PT_IconItem, "cg-border-icon", q);
     }
    if (!mColorItem && !isNoneBlock()) {
        mColorItem = q->style()->createPrimitive(HbStyle::PT_IconItem, "cg-color-icon", q);
    }
    if(!mCheckMarkItem) {
        mCheckMarkItem = q->style()->createPrimitive(HbStyle::PT_IconItem, "cg-selection-icon", q);
    }

    if (!mFrameBackGround) {
        mFrameBackGround = new HbFrameBackground();
        mFrameBackGround->setFrameGraphicsName("qtg_fr_popup_grid_normal");
        mFrameBackGround->setFrameType(HbFrameDrawer::NinePieces);
        q->setDefaultFrame( *mFrameBackGround );
    }
}

void HbColorGridViewItemPrivate::updatePrimitives()
{  
    Q_Q( HbColorGridViewItem );

    if (mBorderItem) {
        HbStyleIconPrimitiveData data;
        q->initPrimitiveData(&data, mBorderItem);
        q->style()->updatePrimitive(mBorderItem, &data, 0);
        mBorderItem->setZValue(mBorderItem->parentItem()->zValue()+1);
        mBorderItem->update();
    }

    if (mColorItem) {
        HbStyleIconPrimitiveData data;
        q->initPrimitiveData(&data, mColorItem);
        q->style()->updatePrimitive(mColorItem, &data, 0);
        mColorItem->setZValue(mColorItem->parentItem()->zValue()+2);
        mColorItem->update();
    }

    if (mCheckMarkItem) {
        HbStyleIconPrimitiveData data;
        q->initPrimitiveData(&data, mCheckMarkItem);
        q->style()->updatePrimitive(mCheckMarkItem, &data, 0);
        mCheckMarkItem->setZValue(mCheckMarkItem->parentItem()->zValue()+3);
        if ( isChecked() ) {
            mCheckMarkItem->show();
        } else {
            mCheckMarkItem->hide();
        }
        mCheckMarkItem->update();
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
    /* HbWidget::polish() will eventually call
       HbStylePrivate::updateThemedItems() which will overwrite our
       carefully set colors. Fortunately, call is behind flag.
     */
    d->themingPending = false;

}


HbColorGridViewItem::HbColorGridViewItem(HbColorGridViewItemPrivate &dd, QGraphicsItem *parent) :
    HbGridViewItem(dd, parent)
{
    Q_D( HbColorGridViewItem );
    d->q_ptr = this; 
    /* HbWidget::polish() will eventually call
       HbStylePrivate::updateThemedItems() which will overwrite our
       carefully set colors. Fortunately, call is behind flag.
     */
    d->themingPending = false;

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
    /* HbWidget::polish() will eventually call
       HbStylePrivate::updateThemedItems() which will overwrite our
       carefully set colors. Fortunately, call is behind flag.
     */
    d->themingPending = false;

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

void HbColorGridViewItem::initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive)
{
    Q_D( const HbColorGridViewItem );
    HbGridViewItem::initPrimitiveData(primitiveData, primitive);

    QString itemName = HbStyle::itemName(primitive);
    if (itemName == QLatin1String("cg-border-icon")) {
        HbStyleIconPrimitiveData *data = hbstyleprimitivedata_cast<HbStyleIconPrimitiveData*>(primitiveData);
        if( d->isNoneBlock() && d->mBorderItem ) {
            data->iconName = QLatin1String("qtg_graf_colorpicker_empty");
        } else {
            data->iconName = QLatin1String("qtg_graf_colorpicker_filled");
        }
        data->color = HbColorScheme::color("qtc_popup_grid_normal");
        data->iconFlags = HbIcon::Colorized;
    } else if (itemName == QLatin1String("cg-color-icon")) {
        HbStyleIconPrimitiveData *data = hbstyleprimitivedata_cast<HbStyleIconPrimitiveData*>(primitiveData);
        data->iconName = QLatin1String("qtg_graf_colorpicker_mask");
        data->color = d->mIndex.data(HbColorGridViewItem::ColorRole).value<QColor>();
        data->iconFlags = HbIcon::Colorized;
    } else if (itemName == QLatin1String("cg-selection-icon")) {
        HbStyleIconPrimitiveData *data = hbstyleprimitivedata_cast<HbStyleIconPrimitiveData*>(primitiveData);
        data->iconName = QLatin1String("qtg_small_tick");
    }
}

void HbColorGridViewItem::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    HbGridViewItem::resizeEvent(event);
    updatePrimitives();
}
