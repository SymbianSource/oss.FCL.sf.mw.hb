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

#include "hbcombodropdown_p.h"
#include "hbcombobox_p.h"
#include <hblistview.h>
#include <hbwidgetfeedback.h>

#ifdef HB_GESTURE_FW
#include <hbtapgesture.h>
#include <hbpangesture.h>
#endif

HbComboDropDown::HbComboDropDown( HbComboBoxPrivate *comboBoxPrivate, QGraphicsItem *parent )
        :HbWidget( parent ),
         mList( 0 ),
         comboPrivate( comboBoxPrivate ),
         vkbOpened( false ),
         backgroundPressed( false )
{
    setBackgroundItem( HbStyle::P_ComboBoxPopup_background );
    #if QT_VERSION >= 0x040600
        //this is to keep the focus in the previous widget.
        setFlag( QGraphicsItem::ItemIsPanel, true );
        setActive( false );
    #endif
    //setFlag(QGraphicsItem::ItemIsPanel);
    //setPanelModality(PanelModal);
}

HbComboDropDown::~HbComboDropDown( )
{

}

void HbComboDropDown::createList( )
{
   mList = new HbListView( this );
   mList->setLongPressEnabled(false);
   HbComboListViewItem *protoType = new HbComboListViewItem(this);
   mList->setItemPrototype( protoType );
   HbStyle::setItemName( mList, "list" );
   mList->setUniformItemSizes( true );
   mList->setSelectionMode( HbAbstractItemView::SingleSelection );
}

void HbComboDropDown::keypadOpened( )
{
    vkbOpened = true;
    comboPrivate->vkbOpened();
}

void HbComboDropDown::keypadClosed( )
{
    vkbOpened = false;
    comboPrivate->vkbClosed();
}

bool HbComboDropDown::eventFilter( QObject *obj, QEvent *event )
{
    Q_UNUSED( obj );
    bool accepted = false;

    if ( isVisible( ) && !vkbOpened ) {
        switch( event->type( ) )
        {
        case QEvent::GraphicsSceneMousePress:
        case QEvent::GraphicsSceneMouseDoubleClick:
            {
                if( !( this->isUnderMouse( ) ) ) {
                    backgroundPressed = true;
                    accepted = true;
                }
            }
            break;
        case QEvent::GraphicsSceneMouseRelease:
            {
                if( !( this->isUnderMouse( ) ) && backgroundPressed ) {
                    HbWidgetFeedback::triggered( this, Hb::InstantPopupClosed );
                    setVisible( false );
                    backgroundPressed = false;
                    accepted = true;
                }
            }
            break;
        case QEvent::Gesture:
            {
                if( !this->isUnderMouse() ) {
                    //if its a pan gesture then don't accept the event so that list can be scrolled
                    //even if mouse is outside drop down area
                    if(QGestureEvent *gestureEvent = static_cast<QGestureEvent *>( event ) ) {
                        if( !qobject_cast<HbPanGesture *>( gestureEvent->gesture( Qt::PanGesture ) ) ) {
                            accepted = true;
                        }
                    }
                }
            }
            break;
        default:
            break;
        }
    }

    return accepted;
}

#include "moc_hbcombodropdown_p.cpp"

