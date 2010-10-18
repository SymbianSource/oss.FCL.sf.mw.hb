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

#ifndef HBCOMBOBOX_P_H
#define HBCOMBOBOX_P_H

#include "hbdialog_p.h"
#include "hbcombodropdown_p.h"
#include "hblineedit_p.h"
#include "hbwidget_p.h"

#include <hbcombobox.h>
#include <hblineedit.h>
#include <hbwidgetfeedback.h>
#include <hbinputeditorinterface.h>
#include <hbinputvkbhost.h>
#include <hblistviewitem.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
class QGraphicsLayoutItem;
class HbDialog;
class HbComboDropDown;
class QModelIndex;
class QSortFilterProxyModel;
class QCompleter;
class HbCustomLineEdit;
class QGraphicsItem;
class HbTextItem;

class HbComboBoxPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC( HbComboBox )

public:
    HbComboBoxPrivate( );
    ~HbComboBoxPrivate( );
    void init( );
    void createPrimitives( );
    void setModel( QAbstractItemModel * model );
    void positionDropDown( );
    void setCompletion( bool completion );
    void setEditable(  bool editable );
    QString itemText( const QModelIndex &index ) const;
    QIcon itemIcon( const QModelIndex &index ) const;
    int itemRole( ) const;
    void addDropDownToScene( );
    void setCurrentIndex( const QModelIndex &index );
    void touchAreaPressEvent( );
    void touchAreaReleaseEvent( );
    void touchAreaClicked( );
    void vkbOpened( );
    void vkbClosed( );
    void _q_textChanged( const QModelIndex & aIndex );
    void _q_textCompleted( const QModelIndex & aIndex );
    void _q_textChanged( const QString & aString );
    void currentIndexChanged( const QModelIndex &index );
    QModelIndex findData( const QVariant &data ) const;
    void showPopup( QAbstractItemModel* aModel, QModelIndex aIndex = QModelIndex( ) );
    void createDropDown( );
    void calculateListItemHeight( );
    void resetGeometryChangeFlag( );
    void showDismissEffect( );

public:
    HbCustomLineEdit* mLineEdit;
    QGraphicsItem* mTextItem;
    QGraphicsItem *mButton;
    HbComboDropDown* mDropDown;
    QAbstractItemModel* mModel;
    QSortFilterProxyModel* mProxyModel;
    QModelIndex mCurrentIndex;
    QCompleter* mCompleter;
    HbComboBox::InsertPolicy insertPolicy;
    QGraphicsItem *mBackgroundItem;
    QGraphicsItem *mButtonTouchAreaItem;
    bool mIsDown;
    bool mEditable;    
    bool mIsDorpdownCreated;
    bool mIsDropwnToSceneAdded;
    bool mHasDownEffect;
    bool mHasUpEffect;
    QString mText;
    qreal mListItemHeight;
    int mDropDownRowsInPortrait;
    int mDropDownRowsInLandscape;
    QMap< QGraphicsWidget *, bool > widgetGeometryChange;
};

class HbComboListViewItem : public HbListViewItem
{
    Q_OBJECT
public:
    explicit HbComboListViewItem ( QGraphicsItem *parent = 0 ) : HbListViewItem( parent ) {
    }

    HbAbstractViewItem *createItem( ) {
        return new HbComboListViewItem( *this );
    }
};

class HbCustomLineEdit : public HbLineEdit
{

    Q_OBJECT
public:
    HbCustomLineEdit( QGraphicsWidget *parent, HbComboBoxPrivate *comboPriv ) :
        HbLineEdit( *new HbLineEditPrivate, parent ),
        comboBoxPrivate( comboPriv ),
        VkbLaunched( false ) {
    }

    ~HbCustomLineEdit() {
    }

protected:
    void focusInEvent( QFocusEvent *event ) {
        HbEditorInterface editorInterface( this );
        HbVkbHost *host = editorInterface.vkbHost( );
        if ( host && comboBoxPrivate->mDropDown && !VkbLaunched ) {
            VkbLaunched = true;
            connect( host, SIGNAL( keypadClosed (  ) ), comboBoxPrivate->mDropDown,
                SLOT( keypadClosed(  ) ) );
            connect( host, SIGNAL( keypadOpened (  ) ), comboBoxPrivate->mDropDown,
                SLOT( keypadOpened(  ) ) );
        }
        HbLineEdit::focusInEvent( event );
    }
private:
    HbComboBoxPrivate *comboBoxPrivate;
    bool VkbLaunched;
};

#endif // HBCOMBOBOX_P_H

