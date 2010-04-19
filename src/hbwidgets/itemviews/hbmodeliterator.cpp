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

#include "hbmodeliterator.h"
#include "hbmodeliterator_p.h"

/*!
    @alpha
    @hbwidgets
    \class HbModelIterator
    \brief Provides functionallity to work with QModelIndex.

    The responsibility of HbModelIterator class is to give possibility to work with
    QAbstractItemModel independently from internal implementation.
    It allows setting model, root index which is used as a base for calculations.
    It gives functionality to get number of items in model that have root index
    as a parent.
    HbModelIterator class provides basic mapping between QModelIndex and position
    of item in model. It also provide easy way to iterate through model by
    providing previous and next operations.
*/

/*!
    Constructs a new HbModelIterator.
*/
HbModelIterator::HbModelIterator(HbModelIteratorPrivate &dd)
    : d(&dd)
{
    Q_D(HbModelIterator);
    d->q_ptr = this;
}

/*!
    Constructs a new HbModelIterator.
*/
HbModelIterator::HbModelIterator(QAbstractItemModel *model, QModelIndex rootIndex)
    : d(new HbModelIteratorPrivate())
{
    Q_D(HbModelIterator);
    d->q_ptr = this;
    setModel(model, rootIndex);
}

/*!
    Destructs the HbModelIterator instance.
*/
HbModelIterator::~HbModelIterator()
{
    delete d;
}

/*!
    Returns the total model index count that can be traversed with nextIndex
    and previousIndex functions for that parent.
    Calling indexCount without any parameter or with QModelIndex do calculations
    for rootIndex.
*/
int HbModelIterator::indexCount(const QModelIndex &parent) const
{
    Q_D(const HbModelIterator);
    if (d->mModel) {
        if (parent.isValid()) {
            return d->mModel->rowCount(parent);
        } else {
            return d->mModel->rowCount(d->mRootIndex);
        }
    }
    return 0;
}

/*!
    Returns the position of \a index from the first index.
*/
int HbModelIterator::indexPosition(const QModelIndex &index) const
{
    return index.row();
}

/*!
    Return ModelIndex for specified pos.
    If parent was not specified or QModelIndex was used calculate index from rootIndex.
*/
QModelIndex HbModelIterator::index(int pos, const QModelIndex &parent) const
{
    Q_D(const HbModelIterator);
    if (d->mModel) {
        if (parent.isValid()) {
            return d->mModel->index(pos, 0, parent);
        } else {
            return d->mModel->index(pos, 0, d->mRootIndex);
        }
    }
    return QModelIndex();
}

/*!
    Returns the next model index from \a index. If QModelIndex() is given as a parameter
    this should return the first model index.

    \note Returned QModelIndex() is interpret so that there is no next index from given one.
*/
QModelIndex HbModelIterator::nextIndex(const QModelIndex &index) const
{
    Q_D(const HbModelIterator);
    if (d->mModel) {
        if (index.isValid()) {
            return d->mModel->index(index.row() + 1, 0, index.parent());
        } else {
            return d->first(d->mRootIndex);
        }
    }
    return QModelIndex();
}

/*!
    Returns the previous model index from \a index. If QModelIndex() is given as a parameter
    this should return the last model index.

    \note Returned QModelIndex() is interpret so that there is no previous index from given one.
*/
QModelIndex HbModelIterator::previousIndex(const QModelIndex &index) const
{
    Q_D(const HbModelIterator);
    if (d->mModel) {
        if (index.isValid()) {
            return d->mModel->index(index.row() - 1, 0, index.parent());
        } else {
            return d->last(d->mRootIndex);
        }
    }
    return QModelIndex();
}

/*!
    Compare to indexes.
    Returns true if index1 is less than index2.
    Important: compared indexes need to have the same parent!
    Function make assumption that they have and they are from the same model.
*/
bool HbModelIterator::lessThan(const QModelIndex &index1, const QModelIndex &index2) const
{

    return index1.row() < index2.row();
}

/*!
    Returns model.
*/
QAbstractItemModel *HbModelIterator::model() const
{
    Q_D(const HbModelIterator);
    return d->mModel;
}

/*!
    Return root index.
*/
QModelIndex HbModelIterator::rootIndex() const
{
    Q_D(const HbModelIterator);
    return d->mRootIndex;
}

/*!
    Set model and root index.
    Do not take ownership of the model.
*/
void HbModelIterator::setModel(QAbstractItemModel *model, QModelIndex rootIndex)
{
    Q_D(HbModelIterator);
    d->setModel(model, rootIndex);
}

/*!
    Set root index.
*/
void HbModelIterator::setRootIndex(QModelIndex rootIndex)
{
    Q_D(HbModelIterator);
    d->setRootIndex(rootIndex);
}
