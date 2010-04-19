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

#ifndef HBDATAGROUPHEADINGWIDGET_H
#define HBDATAGROUPHEADINGWIDGET_H

#include "hbnamespace_p.h"
#include <hbwidget.h>
#include <QGraphicsSceneMouseEvent>

class HbStyleOptionDataGroupHeadingWidget;
class QGraphicsItem;
class HbDataFormViewItem;

QT_FORWARD_DECLARE_CLASS(QGraphicsLinearLayout) 

class HB_AUTOTEST_EXPORT HbDataGroupHeadingWidget : public HbWidget
{
    Q_OBJECT

public:
    HbDataGroupHeadingWidget(QGraphicsItem *parent = 0);
    ~HbDataGroupHeadingWidget();

    void createPrimitives();
    void updatePrimitives();
    void initStyleOption(HbStyleOptionDataGroupHeadingWidget *option);

    enum { Type = HbPrivate::ItemType_DataGroupHeadingWidget };
    int type() const { return Type; }

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);


public:
    QGraphicsItem *mBackgroundItem;
    QGraphicsItem *mHeadingItem;
    QGraphicsItem *mIconItem;
    HbDataFormViewItem* mParent;
    bool mExpanded;
    bool mDown;
    QString mHeading;
};

#endif // HBDATAGROUPHEADINGWIDGET_H
