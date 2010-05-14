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

#ifndef HBGROUPBOX_H
#define HBGROUPBOX_H

#include <hbwidget.h>

class HbGroupBoxPrivate;

class HB_WIDGETS_EXPORT HbGroupBox : public HbWidget
{
    Q_OBJECT
    Q_PROPERTY( QString heading READ heading WRITE setHeading )
    Q_PROPERTY( bool collapsable READ isCollapsable WRITE setCollapsable )
    Q_PROPERTY( bool collapsed READ isCollapsed WRITE setCollapsed )    
    Q_PROPERTY( bool marqueeHeading READ marqueeHeading WRITE setMarqueeHeading )    

public:
    explicit HbGroupBox( QGraphicsItem *parent = 0 );
    ~HbGroupBox( );

    void setHeading( const QString &text );
    QString heading( ) const;   

    bool isCollapsable( ) const;
    bool isCollapsed( ) const;

    void setMarqueeHeading( bool marquee = false );
    bool marqueeHeading( ) const;
    
    void setContentWidget( HbWidget *widget );
    HbWidget *contentWidget( ) const;

    virtual QGraphicsItem *primitive( HbStyle::Primitive primitive ) const;

    enum { Type = Hb::ItemType_GroupBox };
    int type( ) const { return Type; }

public slots:
    void updatePrimitives( );
    void setCollapsed( bool collapsed = true );
    void setCollapsable( bool collapsable = true );

    signals:
    void clicked(); 
    void longPress(const QPointF &delta); 
    void toggled(bool state);

protected:
    HbGroupBox( HbGroupBoxPrivate &dd, QGraphicsItem *parent );

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbGroupBox )
    Q_DISABLE_COPY( HbGroupBox )
};

#endif  //HbGroupBox_H

