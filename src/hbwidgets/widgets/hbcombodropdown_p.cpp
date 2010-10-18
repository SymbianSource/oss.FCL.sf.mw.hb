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
#include "hbwidget_p.h"
#include <hblistview.h>
#include <hbwidgetfeedback.h>
#include <hbinputvirtualkeyboard.h>

#include <hbtapgesture.h>
#include <hbpangesture.h>
#include <QGestureEvent>
#include <QGraphicsSceneMouseEvent>

#include <hbeffect.h>

class HbComboDropDownPrivate : public HbWidgetPrivate
{
};
HbComboDropDown::HbComboDropDown( HbComboBoxPrivate *comboBoxPrivate, QGraphicsItem *parent )
    :HbWidget( *new HbComboDropDownPrivate(), parent ),
     mList( 0 ),
     comboPrivate( comboBoxPrivate ),
     vkbOpened( false ),
     backgroundPressed( false )
{
    Q_D(HbComboDropDown);
    d->setBackgroundItem(HbStylePrivate::P_ComboBoxPopup_background);
    #if QT_VERSION >= 0x040600
    //this is to keep the focus in the previous widget.
    setFlag( QGraphicsItem::ItemIsPanel, true );
    setActive( false );
    setAcceptTouchEvents( true );
    #endif
}

HbComboDropDown::~HbComboDropDown( )
{
}

void HbComboDropDown::createList( )
{
   mList = new HbListView( this );
   HbComboListViewItem *protoType = new HbComboListViewItem( this );
   mList->setItemPrototype( protoType );
   HbStyle::setItemName( mList, "list" );
   mList->setUniformItemSizes( true );
   mList->setSelectionMode( HbAbstractItemView::SingleSelection );
}

void HbComboDropDown::keypadOpened( )
{
    vkbOpened = true;
    comboPrivate->vkbOpened( );
}

void HbComboDropDown::keypadClosed( )
{
    vkbOpened = false;
    comboPrivate->vkbClosed( );
}

#ifdef HB_EFFECTS
void HbComboDropDown::dismissEffectFinished( HbEffect::EffectStatus status )
{
    Q_UNUSED( status );

    setVisible( false );
}
#endif

bool HbComboDropDown::eventFilter( QObject *obj, QEvent *event )
{
    Q_UNUSED( obj );
    bool accepted = false;

    if ( isVisible( ) ) {
        switch( event->type( ) )
        {
        case QEvent::Gesture:
            {
                if( QGestureEvent *gestureEvent = static_cast<QGestureEvent *>( event ) ) {                        
                    HbTapGesture *tapGesture = qobject_cast<HbTapGesture *>(gestureEvent->gesture(Qt::TapGesture));
                    if( tapGesture && tapGesture->state() == Qt::GestureStarted ) {
                        QRectF dropDownSceneRect = mapToScene(boundingRect( )).boundingRect();
                        if(!dropDownSceneRect.contains(tapGesture->sceneStartPos())) {
                            if( !vkbOpened ) {                                      
                                HbWidgetFeedback::triggered( this, Hb::InstantPopupClosed );
                                comboPrivate->showDismissEffect( );
                                comboPrivate->q_ptr->setProperty("state","normal");
                                backgroundPressed = true;
                                accepted = true;
                            } else {
                                //if vkb is opened then dismiss drop down only if click happened outside drop down and
                                //vkb area
                                if( comboPrivate->mEditable ) {
                                    HbEditorInterface editorInterface( comboPrivate->q_ptr );
                                    HbVkbHost *host = editorInterface.vkbHost( );
                                    if( host ) {
                                        //get the scene rect of vkb
                                        QGraphicsWidget *vkbWidget = host->activeKeypad( )->asGraphicsWidget( );
                                        QRectF tmp = host->applicationArea( );
                                        QRectF vkbArea = vkbWidget->mapToScene( tmp ).boundingRect( );
                                        if( !vkbArea.contains( tapGesture->sceneStartPos() ) ) {
                                            comboPrivate->showDismissEffect( );
                                            HbWidgetFeedback::triggered( this, Hb::InstantPopupClosed );
                                            comboPrivate->q_ptr->setProperty( "state", "normal" );
                                            backgroundPressed = true;
                                            accepted = true;
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        //if its a pan gesture then don't accept the event so that list can be scrolled
                        //even if mouse is outside drop down area. Also tap might finish outside the
                        //dropdown area
                        if( !qobject_cast<HbPanGesture *>( 
                                gestureEvent->gesture( Qt::PanGesture ) ) &&
                            !(tapGesture && tapGesture->state() != Qt::GestureStarted)) {
                            accepted = true;
                        }
                    }
                }
            }
            break;
        case QEvent::GraphicsSceneMousePress:
            //dont accept the mouse press event if vkb is pressed, so that vkb can take events
            if( vkbOpened ) {
                if( comboPrivate->mEditable ) {
                    HbEditorInterface editorInterface( comboPrivate->q_ptr );
                    HbVkbHost *host = editorInterface.vkbHost( );
                    if( host ) {
                        //get the scene rect of vkb
                        QGraphicsWidget *vkbWidget = host->activeKeypad( )->asGraphicsWidget( );
                        QRectF tmp = host->applicationArea( );
                        QRectF vkbArea = vkbWidget->mapToScene( tmp ).boundingRect( );

                        QGraphicsSceneMouseEvent *mouseEvent =  
                            static_cast< QGraphicsSceneMouseEvent * >( event ); 
                        if( vkbArea.contains( mouseEvent->scenePos( ) ) ) {
                            break;
                        }
                    }
                }
            }
        case QEvent::TouchBegin:
            accepted = true;
            break;
        default:
            break;
        }
    }
    return accepted;
}

QVariant HbComboDropDown::itemChange( GraphicsItemChange change, const QVariant & value )
{
    switch( change ) {
        case QGraphicsItem::ItemVisibleHasChanged:
            if( !value.toBool( ) ) {
                comboPrivate->resetGeometryChangeFlag( );
            }
            break;
        default:
            break;
    }
    return HbWidget::itemChange( change, value );
}

#include "moc_hbcombodropdown_p.cpp"

