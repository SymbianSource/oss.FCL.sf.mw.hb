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

#ifndef HBDATAFORM_H
#define HBDATAFORM_H

#include <hbabstractitemview.h>

class HbDataFormPrivate;
class HbStyleOptionDataForm;
class HbDataFormViewItem;
class HbDataFormModelItem;

class HB_WIDGETS_EXPORT HbDataForm : public HbAbstractItemView
{
    Q_OBJECT

    Q_PROPERTY(QString heading READ heading WRITE setHeading)
    Q_PROPERTY(QString description READ description WRITE setDescription)

public:

    explicit HbDataForm(QGraphicsItem *parent = 0);
    virtual ~HbDataForm();

    enum { Type = Hb::ItemType_DataForm };
    virtual int type() const {
        return Type;
    }

    virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);

    void setExpanded(const QModelIndex &index, bool expanded);
    bool isExpanded(const QModelIndex &index) const;

    void setHeading(const QString &heading);
    QString heading() const;

    void setDescription(const QString &description);
    QString description() const;

    QGraphicsItem* primitive(HbStyle::Primitive primitive) const;

    virtual void setModel(QAbstractItemModel *model, HbAbstractViewItem *prototype = 0);

    void addConnection(HbDataFormModelItem *item, const char* signal, 
        QObject *receiver, const char* slot);
    void removeConnection(HbDataFormModelItem *item, const char* signal, 
        QObject *receiver, const char* slot);
    void removeAllConnection();
    void removeAllConnection(HbDataFormModelItem *item);


signals:
    void itemShown(const QModelIndex &index);

protected slots:

    virtual void rowsInserted(const QModelIndex &parent, int start, int end);
    virtual void rowsAboutToBeRemoved(const QModelIndex &index, int start, int end);    

    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

protected:
    HbDataForm(HbDataFormPrivate &dd, HbAbstractItemContainer *container, 
        QGraphicsItem * parent);

    void initStyleOption(HbStyleOptionDataForm *option);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbDataForm)
    Q_DISABLE_COPY(HbDataForm)
    Q_PRIVATE_SLOT(d_func(), void _q_page_changed(int))
    
    friend class HbDataFormViewItem;
};

#endif // HBDATAFORM_H
