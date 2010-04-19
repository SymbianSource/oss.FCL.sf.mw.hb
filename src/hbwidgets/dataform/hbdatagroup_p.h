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

#ifndef HBDATAGROUP_H
#define HBDATAGROUP_H

#include <hbdataformviewitem.h>

#include "hbnamespace_p.h"

class HbDataGroupPrivate;
class HbStyleOptionDataGroup;

class HB_AUTOTEST_EXPORT HbDataGroup : public HbDataFormViewItem
{
    Q_OBJECT   

public:

    explicit HbDataGroup( QGraphicsItem *parent = 0 );
    virtual ~HbDataGroup( );
    
    void setDescription(const QString &description);
    QString description() const;
    bool setExpanded( bool expanded );
    bool isExpanded( ) const;   

    //CRC: Is this required here ?
    void updateGroupPageName(int index , const QString &page);    
    QGraphicsItem* primitive( HbStyle::Primitive primitive );

    enum { Type = HbPrivate::ItemType_DataGroup};
    int type() const { return Type; }

    virtual HbDataFormViewItem* createItem( );
    virtual void updateChildItems( );
    virtual bool canSetModelIndex( const QModelIndex &index ) const;
    void polish( HbStyleParameters& params );

public slots:
    void updatePrimitives( );
    void pageChanged( int index );

protected:
    HbDataGroup( const HbDataGroup &source );
    void initStyleOption( HbStyleOptionDataGroup *option );    
    virtual QSizeF sizeHint( Qt::SizeHint which, const QSizeF &constraint ) const;
    virtual void pressStateChanged( bool value, bool animate );

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbDataGroup )
};

#endif // HBDATAGROUP_H
