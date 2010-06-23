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

#ifndef HBDATAFORM_P_H
#define HBDATAFORM_P_H

#include <hbdataform.h>
#include <QHash>
#include "hbabstractitemview_p.h"
#include "hbtreemodeliterator_p.h"

QT_BEGIN_NAMESPACE
class QModelIndex;
class QPersistentModelIndex;
class QSettings;
QT_END_NAMESPACE

class HbAbstractItem;
class HbDataFormViewItem;
class HbDataFormHeadingWidget;
class HbDataFormModelItem;

struct ItemSignal
{
    QObject *receiver;
    QString signal;
    QString slot;
};

class HbDataFormPrivate : public HbAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(HbDataForm)

public:

    HbDataFormPrivate();
    virtual ~HbDataFormPrivate();
    int childCount(const QModelIndex &index) const;
    void init();
    void addFormPage(const QString& page);
    void removeFormPage(const QString& page);
    void _q_page_changed(int index);

    void _q_item_displayed(const QModelIndex &index);
    void makeConnection(QModelIndex index, HbWidget* widget);
    void connectNow(HbDataFormModelItem * modelItem, QString signal, 
                    QObject *receiver, QString slot);
    void removeConnection(HbDataFormModelItem * item, 
                                  QString signal, 
                                  QObject *receiver, 
                                  QString slot);
    void removeAllConnection();
    void removeAllConnection(HbDataFormModelItem *item);
    inline HbTreeModelIterator *treeModelIterator() const;
    void emitActivated(const QModelIndex &modelIndex);

public:
    HbDataFormHeadingWidget* mHeadingWidget;
    QHash<HbDataFormModelItem*, ItemSignal> mConnectionList;

public:
    static HbDataFormPrivate *d_ptr(HbDataForm *form) {
        Q_ASSERT(form);
        return form->d_func();
    }
    friend class HbDataFormViewItem;
};

HbTreeModelIterator *HbDataFormPrivate::treeModelIterator() const
{
    return static_cast<HbTreeModelIterator*>(mModelIterator);
}

#endif // HBDATAFORM_P_H
