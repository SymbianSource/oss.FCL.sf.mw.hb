/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbPlugins module of the UI Extensions for Mobile.
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

#ifndef HBINDICATORMENUCONTENT_P_H
#define HBINDICATORMENUCONTENT_P_H

#include "hbnamespace_p.h"

#include <QStandardItemModel>
#include <hbwidget.h>
#include <hblistview.h>
#include <hblistviewitem.h>

#include <hbglobal.h>
#include <hbstyle.h>

class HbIconItem;
class HbTextItem;
class HbListView;
class HbIndicatorInterface;
class HbListWidgetItem;
class HbStyleOptionIndicatorMenu;
class HbIndicatorMenuContent;
class HbIndicatorMenuClock;
class HbIndicatorMenuDate;

class HbIndicatorListItem : public HbListViewItem
{
    Q_OBJECT
public:
    explicit HbIndicatorListItem(QGraphicsItem *parent=0);
    HbAbstractViewItem *createItem();
    void updateChildItems();
};

class IndicatorList : public HbListView
{
    Q_OBJECT
    Q_PROPERTY(int indicatorCount READ indicatorCount)
public:
    IndicatorList(HbIndicatorMenuContent *content);
    int indicatorCount() const {
        return model()->rowCount();
    }
protected:
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsRemoved(const QModelIndex &parent,int start,int end);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    QSizeF sizeHint (Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;
    void showEvent(QShowEvent *event);

private:
    HbIndicatorMenuContent *mContent;
    mutable bool mUpdateListSize;
    mutable QSizeF mSize;
};

static const int IndicatorTypes = 3;

class HbIndicatorMenuContent : public HbWidget
{
    Q_OBJECT
    Q_PROPERTY(int indicatorCount READ indicatorCount)

public:
    explicit HbIndicatorMenuContent(QGraphicsItem *parent = 0);
    ~HbIndicatorMenuContent();

    enum { Type = HbPrivate::ItemType_IndicatorMenu };
    int type() const { return Type; }

    int indicatorCount() const;

    static HbIndicatorInterface *indicatorFromIndex(
        const QModelIndex &modelIndex);

signals:
    void aboutToClose();
    void userActivity();
    void userActivityStarted();
    void userActivityEnded();

public slots:
    void updatePrimitives();
    void itemActivated(const QModelIndex &modelIndex);
    void indicatorsActivated(QList<HbIndicatorInterface*> addedIndicators);
    void indicatorActivated(HbIndicatorInterface *activatedIndicator);
    void indicatorRemoved(HbIndicatorInterface *indicatorRemoved);

protected:
    void initStyleOption(HbStyleOptionIndicatorMenu *option) const;

protected slots:
    void indicatorUpdated();
    void indicatorlist_scrollingStarted();
    void indicatorlist_scrollingEnded();

private:
    void setData(HbIndicatorInterface *source,
                 const QModelIndex &modelIndex);
    int listIndexFromIndicator(HbIndicatorInterface *indicator);

private:
    int mGroupTypeIndeces[IndicatorTypes]; //index with HbIndicatorInterface::GroupType
                                           //to get the index to the list
                                           //where the indicator should be added.
    HbIndicatorMenuClock *mClock;
    HbIndicatorMenuDate *mDate;
    QGraphicsItem *mHeaderBackground;
    HbListView *mIndicatorList;
    QStandardItemModel indicatorModel;
};

#endif // HBINDICATORMENUCONTENT_P_H
