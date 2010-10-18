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

#ifndef HBLISTVIEWITEM_H
#define HBLISTVIEWITEM_H

#include <hbabstractviewitem.h>
#include <hbglobal.h>
#include <hbnamespace.h>

class HbListViewItemPrivate;
class HbStylePrimitiveData;

class QGraphicsObject;

class HB_WIDGETS_EXPORT HbListViewItem : public HbAbstractViewItem
{
    Q_OBJECT
    Q_ENUMS(StretchingStyle)
    Q_ENUMS(GraphicsSize)

    Q_PROPERTY( Qt::TextFormat textFormat READ textFormat WRITE setTextFormat )
    Q_PROPERTY( StretchingStyle stretchingStyle READ stretchingStyle WRITE setStretchingStyle )
    Q_PROPERTY( GraphicsSize graphicsSize READ graphicsSize WRITE setGraphicsSize )

public:

    enum StretchingStyle {
        NoStretching,
        StretchLandscape
    };

    enum GraphicsSize {
        SmallIcon,
        MediumIcon,
        LargeIcon,
        Thumbnail,
        WideThumbnail
    };

    explicit HbListViewItem(QGraphicsItem *parent=0);
    virtual ~HbListViewItem();

    enum { Type = Hb::ItemType_ListViewItem };
    virtual int type() const;

    virtual HbAbstractViewItem *createItem();

    virtual void updateChildItems();

    Qt::TextFormat textFormat() const;
    void setTextFormat(Qt::TextFormat textFormat);

    StretchingStyle stretchingStyle() const;
    void setStretchingStyle(StretchingStyle style);

    GraphicsSize graphicsSize() const;
    void setGraphicsSize(GraphicsSize size);

    void getSecondaryTextRowCount(int &minimum, int &maximum) const;
    void setSecondaryTextRowCount(int minimum, int maximum);

public slots:
    void updatePrimitives();

protected:    
    HbListViewItem(const HbListViewItem &source);
    HbListViewItem &operator=(const HbListViewItem &source);

    HbListViewItem(HbListViewItemPrivate &dd, QGraphicsItem *parent);

    void polish(HbStyleParameters& params);

    void initPrimitiveData( HbStylePrimitiveData     *primitiveData, 
                            const QGraphicsObject    *primitive,
                            int                      index);

private:

    Q_DECLARE_PRIVATE_D(d_ptr, HbListViewItem)
};

#endif // HBLISTVIEWITEM_H
