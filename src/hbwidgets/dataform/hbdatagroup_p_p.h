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

#ifndef HBDATAGROUP_P_H
#define HBDATAGROUP_P_H

#include "hbdatagroup_p.h"
#include "hbdataformviewitem_p.h"

class HbComboBox;
class HbDataGroupHeadingWidget;

class HB_AUTOTEST_EXPORT HbDataGroupPrivate : public HbDataFormViewItemPrivate
{
    Q_DECLARE_PUBLIC(HbDataGroup)

public:
    HbDataGroupPrivate( HbDataGroup* item );
    HbDataGroupPrivate ( const HbDataGroupPrivate &source );
    ~HbDataGroupPrivate( );

    void init( );
    void changeExpansionState( bool expanded );

    void setHeading( const QString &heading );
    QString heading( ) const;

    //QString groupPage() const;
    void setGroupPage(const QString &page);
    void removeGroupPage(const QString &page);
    int activePage();
    bool setExpanded( bool expanded );
    void setEnabled( bool enabled );
	int pageIndex(const QModelIndex &index) const ;
	QModelIndex pageModelIndex(int index) const ; 

public:
    static HbDataGroupPrivate *d_ptr( HbDataGroup *item ) {
        Q_ASSERT( item );
        return item->d_func();
    }

public:   
    HbComboBox *mPageCombo;
    QString mPageString;
    HbDataGroupHeadingWidget *mGroupHeading;
    QGraphicsItem *mPageComboBackgroundItem;
};

#endif // HBDATAGROUP_P_H
