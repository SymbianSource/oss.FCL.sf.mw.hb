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

#include <hbgridviewitem.h>
#include <hbgridviewitem_p.h>
#include <hbiconitem.h>
#include <hbtextitem.h>

#include <hbeffect.h>

#include "hbgridview_p.h"

HbGridViewItemPrivate::HbGridViewItemPrivate(HbAbstractViewItem *prototype) :
    HbAbstractViewItemPrivate(prototype), 
    mIconItem(0),
    mTextItem(0),
    mTextValid(false),
    mIconValid(false)
{
}

HbGridViewItemPrivate::~HbGridViewItemPrivate()
{
}

void HbGridViewItemPrivate::init()
{
    if (isPrototype()) {
        HbEffect::add("gridviewitem-focus", "gridviewitem_press", "pressed");
        HbEffect::add("gridviewitem-focus", "gridviewitem_release", "released");
        mSharedData->mItemType = "gridviewitem";

        HbEffect::add("gridviewitem", "gridviewitem_appear", "appear");
        HbEffect::add("gridviewitem", "gridviewitem_disappear", "disappear");
    }
    mContentChangedSupported = true;
    mItemsChanged = false;
}

bool HbGridViewItemPrivate::isTextValid(const QVariant& text) const 
{
    return text.canConvert<QString> ();
};

bool HbGridViewItemPrivate::isIconValid(const QVariant& icon) const
{
    return (icon.canConvert<QIcon>() || icon.canConvert<HbIcon>()) ?
            true : false;
};

QString HbGridViewItemPrivate::text(const QVariant& text) const 
{
    return text.toString();
};
    
HbIcon HbGridViewItemPrivate::icon(const QVariant& icon) const
{
    if (icon.canConvert<QIcon> ()) {
        return HbIcon(icon.value<QIcon>());
    }
    else {
        return icon.value<HbIcon>();
    }
}

void HbGridViewItemPrivate::updateTextItem(const HbGridViewPrivate &viewPrivate)
{
    Q_Q(HbGridViewItem);
    QVariant textVariant = mIndex.data(Qt::DisplayRole);
    bool newTextValid = isTextValid(textVariant);
    QString newText;
    if (newTextValid) newText = textVariant.toString();

    if (viewPrivate.mTextVisible) {
        if (newTextValid != mTextValid
            || mText != newText) {
            // change in content
            mText = newText;
            mTextValid = newTextValid;
        } // else - no text change
    } else {
        mTextValid = false;
        mText.clear();
    }

    if (    mTextValid
        &&  !mTextItem) {
        mItemsChanged = true;
        mTextItem = q->style()->createPrimitive(HbStyle::PT_TextItem, QLatin1String("text"), q);
        HbTextItem *textItem = qobject_cast<HbTextItem*>(mTextItem);
        if (textItem) {
            textItem->setAlignment(Qt::AlignCenter);
        }
    } else if (   !mTextValid
               && mTextItem) {
        mItemsChanged = true;
        delete mTextItem;
        mTextItem = 0;
    }
}

void HbGridViewItemPrivate::updateIconItem(const HbGridViewPrivate &viewPrivate)
{
    Q_Q(HbGridViewItem);
    // icon item creation, if needed
    QVariant iconVariant = mIndex.data(Qt::DecorationRole);
    bool newIconValid = isIconValid(iconVariant);
    HbIcon newIcon;
    if (newIconValid) newIcon = icon(iconVariant);

    if (viewPrivate.mIconVisible) {
        if (newIconValid != mIconValid
            || mIcon != newIcon) {
            // change in content
            mIcon = newIcon;
            mIconValid = newIconValid;
        }
    } else {
        mIconValid = false;
        mIcon = HbIcon();
    }

    if (    mIconValid
        &&  !mIconItem) {
        mItemsChanged = true;
        mIconItem = q->style()->createPrimitive(HbStyle::PT_IconItem, QLatin1String("icon"), 0);
        mIconItem->setParentItem(q); // To enable asynchronous icon loading.
        HbIconItem *iconItem = qobject_cast<HbIconItem*>(mIconItem);
        if (iconItem) {
            iconItem->setAlignment(Qt::AlignCenter);
        }
    }
    else if (   !mIconValid
            &&  mIconItem) {
        mItemsChanged = true;
        delete mIconItem;
        mIconItem = 0;
    }
}

