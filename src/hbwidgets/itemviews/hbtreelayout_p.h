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

#ifndef HBTREELAYOUT_H
#define HBTREELAYOUT_H

#include <QGraphicsLayout>
#include <hbglobal.h>

class HbTreeLayoutPrivate;
class HbTreeView;

class HB_AUTOTEST_EXPORT HbTreeLayout : public QGraphicsLayout
{
public:
    HbTreeLayout( QGraphicsLayoutItem *parent = 0 );
    virtual ~HbTreeLayout();

    void addItem(QGraphicsLayoutItem *item, int level);
    void insertItem(int index, QGraphicsLayoutItem *item, int level);
    int indexOf(QGraphicsLayoutItem *item) const;
    void removeItem( QGraphicsLayoutItem *item );
    void setItemLevel(QGraphicsLayoutItem *item, int level);
    void setIndentation(qreal indentation);
    qreal indentation() const;

    qreal smallestItemHeight() const;

public: // From base classes

    virtual int count() const;
    virtual QGraphicsLayoutItem *itemAt(int i) const;
    virtual void removeAt(int index);

    virtual void setGeometry( const QRectF &rect );
    virtual QSizeF sizeHint ( Qt::SizeHint which, const QSizeF &constraint = QSizeF( ) ) const;
    virtual void invalidate();

private:
    HbTreeLayoutPrivate *d;
};

#endif // HBTREELAYOUT_H

