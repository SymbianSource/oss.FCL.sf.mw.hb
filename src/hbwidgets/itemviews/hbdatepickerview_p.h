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

#ifndef HBDATEPICKERVIEW_P_H
#define HBDATEPICKERVIEW_P_H

#include <hbabstractitemview.h>

class HbDatePickerViewPrivate;
class QGestureEvent;

class HbDatePickerView: public HbAbstractItemView
{
    Q_OBJECT
public:
        HbDatePickerView(QGraphicsItem* parent = 0);
        ~HbDatePickerView();
        void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);
        void setSelected(int index);
        int selected() const;
        void setLoopingEnabled(bool looping);
        bool isLoopinEnabled()const;

protected:
        void updatePrimitives();
        void initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive);
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
        void rowsAboutToBeInserted(const QModelIndex &index, int start, int end);
        void rowsInserted(const QModelIndex &parent, int start, int end);
        void rowsRemoved(const QModelIndex &parent, int start, int end);
        void gestureEvent(QGestureEvent *event);
        void currentIndexChanged(const QModelIndex &current, const QModelIndex &previous);
        void orientationAboutToBeChanged();

signals:
        void itemSelected(int);
private:
        Q_DECLARE_PRIVATE(HbDatePickerView)
        Q_PRIVATE_SLOT(d_func(), void _q_scrollingStarted())
        Q_PRIVATE_SLOT(d_func(), void _q_scrollingEnded())
        Q_PRIVATE_SLOT(d_func(), void _q_itemSelected(QPointF))
};

#endif // HBDATEPICKERVIEW_P_H
