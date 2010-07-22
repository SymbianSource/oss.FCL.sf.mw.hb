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

#ifndef HBMODELITERATOR_H
#define HBMODELITERATOR_H

#include <hbglobal.h>
#include <QAbstractItemModel>
#include <QExplicitlySharedDataPointer>

class HbModelIteratorPrivate;
class HbAbstractItemView;

class HB_WIDGETS_EXPORT HbModelIterator
{
public:
    explicit HbModelIterator(QAbstractItemModel *model = 0, QModelIndex rootIndex = QModelIndex());
    virtual ~HbModelIterator();

    virtual int indexCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int indexPosition(const QModelIndex &index) const;
    virtual QModelIndex index(int pos, const QModelIndex &parent = QModelIndex()) const;

    virtual QModelIndex nextIndex(const QModelIndex &index) const;
    virtual QModelIndex previousIndex(const QModelIndex &index) const;

    virtual bool lessThan(const QModelIndex &index1, const QModelIndex &index2) const;

    QAbstractItemModel *model() const;
    QModelIndex rootIndex() const;

private:
    virtual void setModel(QAbstractItemModel *model, QModelIndex rootIndex = QModelIndex());
    virtual void setRootIndex(QModelIndex rootIndex);

    friend class HbAbstractItemView;
    friend class HbAbstractItemViewPrivate;

protected:
    HbModelIterator(HbModelIteratorPrivate &dd);

    HbModelIteratorPrivate *const d;

private:
    Q_DECLARE_PRIVATE_D(d, HbModelIterator)
    Q_DISABLE_COPY(HbModelIterator)
};

#endif // HBMODELITERATOR_H
