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

#ifndef HBLISTVIEWITEMPRIVATE_H
#define HBLISTVIEWITEMPRIVATE_H

#include <hbmainwindow.h>

#include "hblistviewitem.h"
#include "hbabstractviewitem_p.h"
#include "hbabstractitemview.h"

class QGraphicsObject;

class HbListViewItemShared : public HbAbstractViewItemShared
{
public:

    HbListViewItemShared() :
        HbAbstractViewItemShared(),
        mTextFormat(Qt::PlainText),
        mStretchingStyle(HbListViewItem::NoStretching),
        mGraphicsSize(HbListViewItem::MediumIcon),
        mMinimumSecondaryTextRowCount(-1),
        mMaximumSecondaryTextRowCount(-1)
    {
    }

    ~HbListViewItemShared()
    {
    }


public:

    Qt::TextFormat    mTextFormat;
    HbListViewItem::StretchingStyle mStretchingStyle;
    HbListViewItem::GraphicsSize mGraphicsSize;
    int mMinimumSecondaryTextRowCount;
    int mMaximumSecondaryTextRowCount;
};

class HbListViewItemPrivate : public HbAbstractViewItemPrivate
{
    Q_DECLARE_PUBLIC(HbListViewItem)

public:

    HbListViewItemPrivate(HbListViewItem *prototype);

    HbListViewItemPrivate(HbListViewItem *prototype, HbListViewItemShared *shared);
    
    HbListViewItemPrivate(const HbListViewItemPrivate &source);
    
    HbListViewItemPrivate &operator=(const HbListViewItemPrivate &source);

    virtual ~HbListViewItemPrivate();

    void init();

    void setDecorationRole(const QVariant &value,
                           const int index);
    void setDisplayRole(const QString &value,
                        const int index);

    inline HbStyle::PrimitiveType displayPrimitive() const {
        HbStyle::PrimitiveType primitive = HbStyle::PT_TextItem;
        const HbListViewItemShared *sd = static_cast<const HbListViewItemShared *>(mSharedData.constData());

        if (sd->mTextFormat != Qt::PlainText) {
            primitive = HbStyle::PT_RichTextItem;
        }
        return primitive;
    }

    inline bool isLandscape() const {
        Q_Q( const HbListViewItem );
        HbAbstractItemView *view = q->itemView();
        if (    view
            &&  view->mainWindow()) {
            return (view->mainWindow()->orientation() == Qt::Horizontal);
        }
        return false;
    }

    inline bool isStretching() const {
        if (static_cast<const HbListViewItemShared*>(mSharedData.constData())->mStretchingStyle == HbListViewItem::StretchLandscape
            && isLandscape()) {
            return true;
        }
        return false;
    }

    inline bool isMultilineSupported() const {
        HbListViewItem::GraphicsSize graphicsSize = static_cast<const HbListViewItemShared*>(mSharedData.constData())->mGraphicsSize;
        return !(   graphicsSize == HbListViewItem::Thumbnail
                ||  graphicsSize == HbListViewItem::WideThumbnail
                ||  isStretching()); 
    }

    inline HbStyle::PrimitiveType decorationPrimitive(const QVariant& decoration) {
        HbStyle::PrimitiveType primitive = HbStyle::PT_None;

        if (decoration.isValid()) {
            if (decoration.canConvert<HbIcon>() || decoration.canConvert<QIcon>()) {
                primitive = HbStyle::PT_IconItem;
            }
        }
        return primitive;
    }

    QList<QGraphicsObject *> mDisplayRoleTextItems;
    QList<QGraphicsObject *> mDecorationRoleItems;

    QStringList     mStringList;
    QVariantList    mDecorationList;
};

#endif // HBLISTVIEWITEMPRIVATE_H
