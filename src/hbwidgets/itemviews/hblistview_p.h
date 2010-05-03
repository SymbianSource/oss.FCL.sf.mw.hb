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

#ifndef HBLISTVIEW_PRIVATE_H
#define HBLISTVIEW_PRIVATE_H

#include "hblistview.h"
#include "hbglobal.h"
#include "hbabstractitemview_p.h"

#include <qobjectdefs.h>
#include <QGraphicsWidget>
#include <QAbstractItemModel>
#include <QTimer>
#include <QItemSelectionModel>
#include <QTransform>

#include <hbeffect.h>

QT_BEGIN_NAMESPACE
class QModelIndex;
class QPersistentModelIndex;
class QAbstractItemModel;
class QTimeLine;
class QGraphicsLinearLayout;
class QPanGesture;
QT_END_NAMESPACE

class HbListView;
class HbListViewItem;
class HbScrollBar;
class HbListLayout;
class HbGestureSceneFilter;
class HbListHighlight;
class HbGesture;

class HbListViewPrivate : public HbAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(HbListView)

public:

    HbListViewPrivate();
    virtual ~HbListViewPrivate();

    void init();
    void moveDraggedItemTo(const QPointF &mousePosition);

    virtual bool panTriggered(QGestureEvent *event);

    virtual bool animationEnabled(bool insertOperation);
public:
    bool mArrangeMode;
    QPersistentModelIndex mDraggedItemIndex;
    HbAbstractViewItem *mDraggedItem;
    QTime mMousePressTimer;
    QPointF mMousePressPos;
    QPointF mScrollStartMousePos;
    QPointF mLastScrollPos;
    QTransform mOriginalTransform;

    bool mOriginalFriction;    
};

#endif // HBLISTVIEW_PRIVATE_H

