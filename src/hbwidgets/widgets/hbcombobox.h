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

#ifndef HBCOMBOBOX_H
#define HBCOMBOBOX_H

#include <hbwidget.h>

class HbValidator;
class HbComboBoxPrivate;
class HbStyleOptionComboBox;

class HB_WIDGETS_EXPORT HbComboBox : public HbWidget
{
    Q_OBJECT
    Q_ENUMS( InsertPolicy )
    Q_PROPERTY( bool editable READ isEditable WRITE setEditable ) 
    Q_PROPERTY( int count READ count )
    Q_PROPERTY( QStringList items READ items WRITE setItems )
    Q_PROPERTY( int currentIndex READ currentIndex WRITE setCurrentIndex )
    Q_PROPERTY( QString currentText READ currentText )

public:

    enum InsertPolicy {
        NoInsert,
        InsertAtTop,
        InsertAtCurrent,
        InsertAtBottom,
        InsertAfterCurrent,
        InsertBeforeCurrent,
        InsertAlphabetically
    };

    explicit HbComboBox( QGraphicsItem *parent = 0 );
    virtual ~HbComboBox( );

    enum {
        Type = Hb::ItemType_ComboBox
    };
    int type( ) const {
        return Type; 
    }

    void addItem( const QString &text, const QVariant &userData = QVariant() );
    void addItem( const HbIcon &icon, const QString &text,
        const QVariant &userData = QVariant( ) );

    void addItems( const QStringList &texts );

    void insertItem( int index, const QString &text, const QVariant &userData = QVariant() );
    void insertItem( int index, const HbIcon &icon, const QString &text,
        const QVariant & userData = QVariant( ) );
    void insertItems( int index, const QStringList &texts );

    int count( ) const;

    void setItems( const QStringList &texts );
    QStringList items( ) const;

    void setItemIcon( int index, const HbIcon &icon );
    HbIcon itemIcon( int index ) const;

    void setItemText( int index, const QString &text );
    QString itemText( int index ) const;

    void setModel( QAbstractItemModel *model );
    QAbstractItemModel *model( ) const;

    void setEditable( bool editable );
    bool isEditable( ) const;

    void setItemData( int index, const QVariant &value, int role = Qt::UserRole );
    QVariant itemData( int index, int role = Qt::UserRole ) const;

    void setValidator( HbValidator *validator );
    const HbValidator *validator( ) const;

    QString currentText( ) const;

    int currentIndex( ) const;

    void removeItem( int index );

    inline int findText( const QString &text,
        Qt::MatchFlags flags = Qt::MatchExactly|Qt::MatchCaseSensitive ) const {
            return findData( text, Qt::DisplayRole, flags );
    }

    int findData( const QVariant &data, int role = Qt::UserRole,
        Qt::MatchFlags flags = Qt::MatchExactly|Qt::MatchCaseSensitive ) const;

    QGraphicsItem *primitive( HbStyle::Primitive primitive ) const;

public slots:
    void updatePrimitives( );
    void clear( );
    void clearEditText( );
    void setCurrentIndex( int index );
    void setEditText( const QString &text );

signals:
    void currentIndexChanged( int index );
    void currentIndexChanged( const QString &text );
    void editTextChanged( const QString &text );
    void editingFinished( );

protected:
    HbComboBox( HbComboBoxPrivate &dd, QGraphicsItem *parent = 0 );
    void initStyleOption( HbStyleOptionComboBox *option ) const;
    void resizeEvent( QGraphicsSceneResizeEvent *event );
    void keyPressEvent( QKeyEvent *event );
    void keyReleaseEvent( QKeyEvent *event );
    void changeEvent( QEvent *event );
    bool eventFilter( QObject *obj, QEvent *event );
    QVariant itemChange ( GraphicsItemChange change, const QVariant & value );

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbComboBox )
    Q_DISABLE_COPY( HbComboBox )
    Q_PRIVATE_SLOT( d_func( ), void _q_textChanged( const QModelIndex &aIndex ) )
    Q_PRIVATE_SLOT( d_func( ), void _q_textCompleted( const QModelIndex &aIndex ) )
    Q_PRIVATE_SLOT( d_func( ), void _q_textChanged( const QString &aString ) )
};

#endif // HBCOMBOBOX_H

