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

#ifndef HBDATAFORMMODELITEM_H
#define HBDATAFORMMODELITEM_H

#include <Qt>
#include <QMap>
#include <QList>
#include <QVariant>
#include <QString>

#include <hbglobal.h>

class HbDataFormModelItemPrivate;
class QAbstractItemModel;

class HB_WIDGETS_EXPORT HbDataFormModelItem
{

public:

    enum Roles {
        KeyRole = Qt::UserRole + 1,
        LabelRole,
        ItemTypeRole,
        PropertyRole,
        DescriptionRole
    };

    enum DataItemType {
        FormPageItem,
        GroupItem,
        GroupPageItem,
        SliderItem,
        VolumeSliderItem,
        CheckBoxItem,
        TextItem,
        ToggleValueItem,
        RadioButtonListItem,
        MultiselectionItem,
        ComboBoxItem,
        CustomItemBase = FormPageItem + 100,
        CustomItemEnd = CustomItemBase + 100
    };

    HbDataFormModelItem(HbDataFormModelItem::DataItemType type,
        const QString &label = QString(),
        const HbDataFormModelItem *parent = 0);
    explicit HbDataFormModelItem(const HbDataFormModelItem *parent = 0);
    virtual ~HbDataFormModelItem();

    void appendChild(HbDataFormModelItem *child);
    void insertChild(int index, HbDataFormModelItem *child);
    void insertChildren(int startIndex, int count, QList<HbDataFormModelItem*> items);
    void removeChild(int index);
    void removeChildren(int startIndex, int count);

    HbDataFormModelItem* childAt(int index) const;
    int indexOf(const HbDataFormModelItem* child) const;
    int childCount() const;

    void setModel(const QAbstractItemModel *model);
    QAbstractItemModel* model() const;

    QVariant data(int role) const;
    void setData(int role, const QVariant &value);

    QVariant contentWidgetData(const QString &propertyName) const;
    QHash<QString, QVariant> contentWidgetData() const;
    void setContentWidgetData(const QString &propertyName, const QVariant &value);

    void setParent(HbDataFormModelItem* parent);
    HbDataFormModelItem* parent() const;

    void setType(HbDataFormModelItem::DataItemType type);
    HbDataFormModelItem::DataItemType type() const;

    void setLabel(const QString &label);
    QString label() const;

    void setDescription(const QString& description);
    QString description() const;

    void setIcon(const QString &icon);
    QString icon() const;

    void setEnabled(bool enabled);
    bool isEnabled() const;

    Qt::ItemFlags flags() const;
   
protected:
    HbDataFormModelItemPrivate * const d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbDataFormModelItem)
};
#endif // HBDATAFORMMODELITEM_H

