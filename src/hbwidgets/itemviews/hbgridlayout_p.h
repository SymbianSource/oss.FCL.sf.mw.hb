
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

#ifndef HBGRIDLAYOUT_P_H
#define HBGRIDLAYOUT_P_H

#include <QObject>
#include <QGraphicsLayout>

#include <hbeffect.h>
#include <hbglobal.h>

class HbGridLayoutPrivate;

class HB_AUTOTEST_EXPORT HbGridLayout : public QObject, public QGraphicsLayout
{
    Q_OBJECT
    Q_ENUMS(layoutDirection)
    Q_INTERFACES(QGraphicsLayout)

    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)

public:

    enum AnimationType {
        RemoveAnimation,
        InsertAnimation
    };

    HbGridLayout( QGraphicsLayoutItem *parent = 0 );
    virtual ~HbGridLayout( );
    
    void addItem(QGraphicsLayoutItem *item);
    void insertItem(int index, QGraphicsLayoutItem *item, bool animate);
    void removeItem(QGraphicsLayoutItem *item, bool animate);

    int indexOf(QGraphicsLayoutItem *item) const;
    QGraphicsLayoutItem *itemAt(int i) const;

    void setSize(const QSizeF &size,int &targetCount);

    void setScrollDirection(Qt::Orientations scrollDirection);
    Qt::Orientations scrollDirection() const;

    void setMargins(qreal topMargin, qreal bottomMargin, qreal leftMargin, qreal rightMargin);
    qreal spacing() const;
    void setSpacing(qreal spacing);

    void setRowCount(int rowCount);
    int rowCount() const;

    void setColumnCount(int columnCount);    
    int columnCount() const;

    int count() const;

    void setRecycling(bool enableRecycling);
    bool isRecyclingEnabled();

    void moveItem(QGraphicsLayoutItem *item, int targetIndex, bool animate = false);

    void recycleItems(QGraphicsLayoutItem *sourceItem, QGraphicsLayoutItem *targetItem);

    bool isAnimated(QGraphicsLayoutItem *item) const;

    virtual void removeAt(int index);
    virtual void setGeometry(const QRectF &rect);
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    virtual void invalidate();

signals:

    void animationFinished(QGraphicsLayoutItem *item, HbGridLayout::AnimationType animationType);

protected slots:

    void effectFinished(const HbEffect::EffectStatus &status);

protected:
    HbGridLayout( HbGridLayoutPrivate &dd, QGraphicsLayoutItem *parent = 0 );
    HbGridLayoutPrivate * const d_ptr;

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbGridLayout )
    Q_DISABLE_COPY( HbGridLayout )
    Q_PRIVATE_SLOT(d_func(), void _q_timerUpdate())
};

#endif // HBGRIDLAYOUT_P_H

