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

#ifndef HBDATAFORMVIEWITEM_P_H
#define HBDATAFORMVIEWITEM_P_H

#include <hbdataformviewitem.h>
#include <hbdataformmodelitem.h>

#include "hbabstractviewitem_p.h"

class HbPushButton;
class HbRadioButtonList;
class HbLabel;
class HbListDialog;
class HbAction;

QT_FORWARD_DECLARE_CLASS(QGraphicsLinearLayout)

/*
HbToggleItem holds a HbPushButton and toggles the text whenever user clicks on it . 
The two texts (primary and secondary ) are toggled in HbDataFormModelItem and primary 
is set on Button.
*/
class HbToggleItem : public HbWidget
{
    Q_OBJECT

public:
    HbToggleItem( QGraphicsItem* parent = 0 );
    ~HbToggleItem( );
    HbWidget * contentWidget( ) const;

protected:
    virtual bool event ( QEvent * e );

public slots:
    void toggleValue( );
signals:
    void valueChanged(QPersistentModelIndex, QVariant);

private:
    HbPushButton* mButton;
    //CRC: we can have thse variables in some common base class
    HbDataFormViewItem* mViewItem;
    HbDataFormModelItem* mModelItem;
    HbDataFormModel * mModel;
};

//radio item class declaration
class HbRadioItem : public HbWidget
{
    Q_OBJECT

public:
    HbRadioItem( QGraphicsItem* parent = 0 );
    ~HbRadioItem( );
    HbWidget* contentWidget( );

    //void setItemEnabled(bool enable);

protected:
    virtual bool event( QEvent * e ); 

public slots:
    void itemSelected( int index );
    void buttonClicked();
signals:
    void valueChanged(QPersistentModelIndex, QVariant);

private:
    HbRadioButtonList* mRadioButton;
    HbPushButton* mButton;
    bool mButtonVisible;
    QStringList mItems;
    int mSelected;
    HbDataFormViewItem *mViewItem;
    HbDataFormModelItem* mModelItem;
    HbDataFormModel * mModel;
};

//multi selectio item class declaration
class HbMultiSelectionItem : public HbWidget
{
    Q_OBJECT

public:
    HbMultiSelectionItem( QGraphicsItem* parent = 0 );
    ~HbMultiSelectionItem( );
    HbWidget* contentWidget( ) const;

protected:
    virtual bool event( QEvent * e ); 

public slots:
    void launchMultiSelectionList( );
    void dialogClosed(HbAction*);

signals:
    void valueChanged(QPersistentModelIndex, QVariant);

private:
    HbPushButton* mButton;
    QStringList mItems;
    QList<int> mSelectedItems;
    HbDataFormViewItem *mViewItem;
    HbDataFormModelItem* mModelItem;
    HbDataFormModel * mModel;
    HbListDialog* mQuery;

};


class HbDataFormViewItemPrivate : public HbAbstractViewItemPrivate
{
    Q_DECLARE_PUBLIC(HbDataFormViewItem)

public:
    explicit HbDataFormViewItemPrivate( HbDataFormViewItem *prototype );
    HbDataFormViewItemPrivate ( const HbDataFormViewItemPrivate &source );
    HbDataFormViewItemPrivate& operator=( const HbDataFormViewItemPrivate &source );
    virtual ~HbDataFormViewItemPrivate( );

    void init( );
    //void _q_item_value_changed( QVariant value );

    void createContentWidget( );

    void createPrimitives( );
    void updatePrimitives( ); 

    void setLabel( const QString &label );
    QString label( ) const;

    void setIcon( const QString &label );
    QString icon( ) const;
    void setDescription( const QString& description );
    QString description() const;
    void updateLabel(const QString& label);

    void setEnabled(bool enabled);
public:
    static HbDataFormViewItemPrivate *d_ptr(HbDataFormViewItem *item) {
        Q_ASSERT(item);
        return item->d_func();
    }
public:

    QString mProperty;
    QString mLabel;
    QString mIcon;
    HbDataFormModelItem::DataItemType mType;
    HbWidget *mContentWidget;
    QGraphicsItem *mBackgroundItem;
    QGraphicsItem *mLabelItem;
    QGraphicsItem *mIconItem;
    QGraphicsItem *mDescriptionItem;
    QVariant mCurrentValue;//CRC why is this required
    bool mSetAllProperty;
    HbDataFormModel* mModel;
    HbDataFormModelItem *mModelItem;
    QString mDescription;
};

#endif //HBDATAFORMVIEWITEM_P_H
