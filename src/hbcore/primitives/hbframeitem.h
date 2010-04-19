/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
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

#ifndef HBFRAMEITEM_H
#define HBFRAMEITEM_H

#include <hbglobal.h>
#include <hbnamespace.h>
#include <hbwidgetbase.h>
#include <hbframedrawer.h>
#include <QGraphicsItem>

class HbFrameItemPrivate;

QT_BEGIN_NAMESPACE
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
QT_END_NAMESPACE

class HB_CORE_EXPORT HbFrameItem : public HbWidgetBase
{
    Q_OBJECT

public:
    explicit HbFrameItem(QGraphicsItem *parent = 0);
    explicit HbFrameItem(HbFrameDrawer *drawer, QGraphicsItem *parent = 0);
    explicit HbFrameItem(const QString &frameGraphicsName,
                         HbFrameDrawer::FrameType frameGraphicsType,
                         QGraphicsItem *parent = 0);

    ~HbFrameItem();

    HbFrameDrawer &frameDrawer() const;
    void setFrameDrawer(HbFrameDrawer *drawer);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    enum { Type = Hb::ItemType_FrameItem };
    int type() const { return Type; }

protected:
    void changeEvent(QEvent *event);

private:
    Q_DISABLE_COPY(HbFrameItem)

private:
    friend class HbFrameItemPrivate;
    HbFrameItemPrivate *d;
};

#endif // HBFRAMEITEM_H
