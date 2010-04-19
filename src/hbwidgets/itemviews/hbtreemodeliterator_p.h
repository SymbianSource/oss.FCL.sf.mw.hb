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

#ifndef HBTREEMODELITERATOR_P_H
#define HBTREEMODELITERATOR_P_H

#include "hbmodeliterator.h"
#include <QtCore/qobject.h>

class HbTreeModelIteratorPrivate;
class HbAbstractItemContainer;

class HB_AUTOTEST_EXPORT HbTreeModelIterator : public QObject, public HbModelIterator
{
    Q_OBJECT

public:
    HbTreeModelIterator(QAbstractItemModel *model = 0, QModelIndex rootIndex = QModelIndex(), bool useCache = true);
    ~HbTreeModelIterator();

    int indexCount(const QModelIndex &parent = QModelIndex()) const;
    int indexPosition(const QModelIndex &index) const;
    QModelIndex index(int pos, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex child(int pos, const QModelIndex &parent) const;
    int childCount(const QModelIndex &parent) const;

    QModelIndex nextIndex(const QModelIndex &index) const;
    QModelIndex previousIndex(const QModelIndex &index) const;

    void setItemContainer(HbAbstractItemContainer *itemContainer, int expansionKey);

    bool lessThan(const QModelIndex &index1, const QModelIndex &index2) const;

private:
    void setModel(QAbstractItemModel *model, QModelIndex rootIndex = QModelIndex());
    void setRootIndex(QModelIndex rootIndex);

public slots:
    void itemStateChanged(const QModelIndex &index, int stateKey);

private slots:
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsRemoved(const QModelIndex &parent, int start, int end);
    void columnsInserted(const QModelIndex &parent, int start, int end);
    void columnsRemoved(const QModelIndex &parent, int start, int end);

private:
    Q_DECLARE_PRIVATE_D(d, HbTreeModelIterator)
    Q_DISABLE_COPY(HbTreeModelIterator)

    friend class HbAbstractItemContainer; // temporary friend until itemstatemodel will be redesigned
};

#endif // HBTREEMODELITERATOR_P_H
