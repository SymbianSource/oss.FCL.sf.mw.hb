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
#ifndef HBLISTWIDGET_P_H
#define HBLISTWIDGET_P_H

#include "hblistwidget.h"
#include "hblistview_p.h"

#include <QObject>

class QModelIndex;
class HbListModel;
class HbListWidget;

class HbListWidgetPrivate : public HbListViewPrivate
{
    Q_DECLARE_PUBLIC(HbListWidget)
public:

    HbListWidgetPrivate(){}
    virtual ~HbListWidgetPrivate(){}
    void init( HbListModel *listModel );

public:
    HbListModel        *mListModel;

public: // public slots:
    void _q_itemActivated(const QModelIndex &modelIndex);
    void _q_itemPressed(const QModelIndex &modelIndex);
    void _q_itemReleased(const QModelIndex &modelIndex);
    void _q_itemLongPress(HbAbstractViewItem *viewItem, const QPointF &point);
};

#endif // HBLISTWIDGET_P_H

