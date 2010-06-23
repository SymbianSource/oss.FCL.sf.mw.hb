/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
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

#include "hbtoolbarextension.h"
#include "hbtoolbarextension_p.h"
#include "hbaction.h"
#include "hbtoolbutton.h"
#include "hbtoolbutton_p.h"
#include "hbdialog_p.h"
#include "hbdeviceprofile.h"
#include "hbtoolbar_p.h"
#include "hbmainwindow.h"
#ifdef HB_EFFECTS
#include <hbeffect.h>
#include <hbeffectinternal_p.h>
bool HbToolBarExtensionPrivate::extensionEffectsLoaded = false;
#endif

#include <QDebug>
#include <QGraphicsGridLayout>
#include <QEventLoop>
#include <QPainter>
#include <QGraphicsLinearLayout> 

/*!
    @stable
    @hbcore
    \class HbToolBarExtension

    \brief HbToolBarExtension is a popup style component that adds
    extra functionality to an HbToolBar. A toolbar can contain more
    than one toolbar extension.  An extension popup is opened when a
    toolbar button with associated with the extension is triggered.

    A toolbar extension uses the QGraphicsWidget action API to manage
    buttons. In addition the HbDialog API can be used to fill the
    extension popup with custom widgets (e.g.  list or line edit). If
    custom content widget is set, buttons generated based on actions
    will not be visible.

    An example of how to add a toolbar extension button to the toolbar:
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,27}
*/

/*!
    \reimp
    \fn int HbToolBarExtension::type() const
 */

/*!
  \primitives
  \primitive{background} HbFrameItem representing the extension background.
  */

HbToolBarExtensionPrivate::HbToolBarExtensionPrivate() :
        HbDialogPrivate(),
        mToolButtons(),
        mLayout(0),
        extensionAction(0),
        mAlignment(Qt::AlignTop),
        mDefaultContentWidget(false),
        // default values, in case CSS parsing fails
        mMargins(0),
        mRowsPortrait(4),
        mRowsLandscape(3),
        mColsPortrait(3),
        mColsLandscape(4),
        lazyInitDone(false),
        orientationConnectDone(false),
        //
        mExtendedButton(0),
        mToolBar(0)
{
}

HbToolBarExtensionPrivate::~HbToolBarExtensionPrivate()
{
}

void HbToolBarExtensionPrivate::init()
{
    Q_Q(HbToolBarExtension);
    extensionAction = new HbAction(q);
    extensionAction->setToolBarExtension(q);
    q->setTimeout(HbDialog::NoTimeout);
    q->setDismissPolicy(HbPopup::TapOutside);
    q->setBackgroundFaded(false);
    q->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
}

void HbToolBarExtensionPrivate::doLazyInit()
{
    Q_Q(HbToolBarExtension);
    if ( !lazyInitDone ) {
        q->setBackgroundItem( HbStyle::P_ToolBarExtension_background );
#ifdef HB_EFFECTS
        if (!extensionEffectsLoaded){
            HbEffectInternal::add("HB_TBE", "tbe_button_click", "clicked");
            extensionEffectsLoaded = true;
        }
#endif
        lazyInitDone = true;
    }
}

void HbToolBarExtensionPrivate::initialiseContent()
{
    Q_Q(HbToolBarExtension);

    QGraphicsWidget *tbeContentWidget = q->contentWidget();

    if ( !tbeContentWidget ) {
        tbeContentWidget = new QGraphicsWidget(q);
        q->setContentWidget( tbeContentWidget );
        mDefaultContentWidget = true;
    }
}

/*!
    Creates a new grid layout and populates tool buttons into it.
 */
void HbToolBarExtensionPrivate::doLayout()
{
    Q_Q(HbToolBarExtension);
    if (!q->mainWindow() ) return;  // workaround unittest

    int columns( q->mainWindow()->orientation() == Qt::Vertical ? 
                 mColsPortrait : mColsLandscape );
    int maxRow( q->mainWindow()->orientation() == Qt::Vertical ? 
                mRowsPortrait : mRowsLandscape );
    int column (0);
    int row(0);
    initialiseContent();
    if (!mDefaultContentWidget)
        return;

    foreach (HbToolButton* button, mToolButtons) {
        button->setVisible(HbToolButtonPrivate::d_ptr(button)->action->isVisible());
    }

    mLayout = new QGraphicsGridLayout();
    mLayout->setContentsMargins( mMargins, mMargins, mMargins, mMargins );
    mLayout->setSpacing(0.0);  // if non zero spacing needed, add to css
    for ( int i(0), j(0), ie(mToolButtons.count()); i < ie; ++i ) {
        HbToolButton *button = mToolButtons.at(i);
        if ( HbToolButtonPrivate::d_ptr(button)->action->isVisible() ) {
            // Calculate the row and column indices
            column = ( j % columns );
            row = ( (j - column) / columns );
            if ( row >= maxRow ) {
                qWarning() << "Too many items in extension!";
            }
            HbToolButtonPrivate::d_ptr(button)->setExtensionBackgroundVisible(true);
            mLayout->addItem( button, row, column );
            ++j;
        }
    }
    q->contentWidget()->setLayout(mLayout);
}

void HbToolBarExtensionPrivate::placeToolBarExtension()
{
    Q_Q(HbToolBarExtension);

    if (mExtendedButton) {
        if (mAlignment == Qt::AlignLeft) {
            QPointF position = QPointF(mExtendedButton->scenePos().x(),
                                       mExtendedButton->scenePos().y() +
                                       (mExtendedButton->geometry().height()/2));
            q->setPreferredPos(position, HbPopup::RightEdgeCenter);
        } else if (mAlignment == Qt::AlignRight) {
            QPointF position = QPointF(mExtendedButton->scenePos().x() +
                                       (mExtendedButton->geometry().width()),
                                       mExtendedButton->scenePos().y() +
                                       (mExtendedButton->geometry().height()/2));
            q->setPreferredPos(position, HbPopup::LeftEdgeCenter);
        } else if (mAlignment == Qt::AlignTop) {
            QPointF position = QPointF(mExtendedButton->scenePos().x() +
                                       (mExtendedButton->geometry().width()/2),
                                       mExtendedButton->scenePos().y());
            q->setPreferredPos(position, HbPopup::BottomEdgeCenter);
        }
    }
}

void HbToolBarExtensionPrivate::actionAdded( QActionEvent *event )
{
    Q_Q(HbToolBarExtension);

    HbToolButton *button = 0;

    HbAction *hbAction = qobject_cast<HbAction *>( event->action() );

    if (hbAction) {
        button = new HbToolButton(hbAction, q->contentWidget());
    } else {
        button = new HbToolButton(q->contentWidget());
        HbToolButtonPrivate::d_ptr(button)->action = event->action();
        QObject::connect(event->action(), SIGNAL(triggered()), button, SLOT(_q_actionTriggered()));
        QObject::connect(event->action(), SIGNAL(changed()), button, SLOT(_q_actionChanged()));
    }

    if ((hbAction && !hbAction->icon().isNull()) || !event->action()->icon().isNull()) {
        if (HbToolButtonPrivate::d_ptr(button)->action->text().isEmpty()) {
            button->setToolButtonStyle(HbToolButton::ToolButtonIcon);
        } else {
            button->setToolButtonStyle(HbToolButton::ToolButtonTextAndIcon);
        }
    } else {
        button->setToolButtonStyle(HbToolButton::ToolButtonText);
    }

    button->setProperty("toolbutton_extension_layout", true);
    button->setSizePolicy( QSizePolicy( QSizePolicy::Preferred,
                                        QSizePolicy::Preferred) );

    // Find out index where to insert button
    int index = q->actions().indexOf( event->action() );

    mToolButtons.insert( index, button );

    q->connect(button, SIGNAL(clicked()), q, SLOT(_q_animateButtonClicked()));
    q->connect(event->action(), SIGNAL(triggered()), q, SLOT(close()));

    if (contentWidget){
        doLayout();
    }
}

void HbToolBarExtensionPrivate::actionRemoved( QActionEvent *event )
{
    for ( int i(0); i < mToolButtons.count(); ++i ) {
        HbToolButton *button = mToolButtons.at(i);
        if ( HbToolButtonPrivate::d_ptr(button)->action == event->action() ) {
            mToolButtons.removeAt(i);                   
            if (contentWidget) {
                mLayout->removeAt(i);
                doLayout();
            }
            delete button;
            return;
        }
    }
}

void HbToolBarExtensionPrivate::actionChanged()
{
    if (contentWidget) {
        doLayout(); // for action()->setVisible(visible) support
    }
}

void HbToolBarExtensionPrivate::setAlignment(Qt::Alignment alignment)
{
    mAlignment = alignment;
}

void HbToolBarExtensionPrivate::setExtensionAction(HbAction *action)
{
    Q_Q(HbToolBarExtension);
    if (extensionAction == action) {
        return;
    }
    if (extensionAction) {
        delete extensionAction;
    }
    extensionAction = action;
    extensionAction->setToolBarExtension(q);

}

void HbToolBarExtensionPrivate::_q_orientationChanged()
{
    Q_Q(HbToolBarExtension);
    if (mToolBar) {
        if (mToolBar->orientation() == Qt::Horizontal) {
            HbToolBarExtensionPrivate::d_ptr(q)->setAlignment(Qt::AlignTop);
        } else if (mToolBar->orientation() == Qt::Vertical 
                   && q->layoutDirection() == Qt::LeftToRight) {
            HbToolBarExtensionPrivate::d_ptr(q)->setAlignment(Qt::AlignLeft);
        } else {
            HbToolBarExtensionPrivate::d_ptr(q)->setAlignment(Qt::AlignRight);
        }
    }
    q->repolish();
    doLayout();
}

void HbToolBarExtensionPrivate::_q_animateButtonClicked()
{
#ifdef HB_EFFECTS
    Q_Q(HbToolBarExtension);
    HbToolButton *button = static_cast<HbToolButton *>(q->sender());
    if (button) {
        HbEffect::start(button, "HB_TBE", "clicked");
    }
#endif
}

// ======== MEMBER FUNCTIONS ========

/*!
    Constructs a new HbToolBarExtension. \a parent graphics item can be set.
 */
HbToolBarExtension::HbToolBarExtension( QGraphicsItem *parent ) :
        HbDialog(*new HbToolBarExtensionPrivate(), parent)
{
    Q_D(HbToolBarExtension);
    d->q_ptr = this;

    d->init();
}

/*!
    Destructor.
 */
HbToolBarExtension::~HbToolBarExtension()
{
    disconnect();
}

/*!
    \overload

    Creates a new action with the given \a text. 
    This action is added to the end of the toolbar extension.
    TODO: If the grid is already full, this call will be ignored.
    TODO: Find a way to notificate the caller.
*/
HbAction *HbToolBarExtension::addAction( const QString &text )
{
    HbAction *action = new HbAction( text, this );
    addAction(action);
    return action;
}

/*!
    \overload

    Creates a new action with the given \a icon and \a text.
    This action is added to the end of the toolbar extension.
    TODO: If the grid is already full, this call will be ignored.
    TODO: Find a way to notificate the caller.
*/
HbAction *HbToolBarExtension::addAction( const HbIcon &icon, 
                                         const QString &text )
{
    HbAction *action = new HbAction( icon, text, this );
    addAction(action);
    return action;
}

/*!
    \overload

    Creates a new action with the given \a text. 
    This action is added to the end of the toolbar extension.
    TODO: If the grid is already full, this call will be ignored.
    The action's \link HbAction::triggered()
    triggered()\endlink signal is connected to \a member in \a
    receiver.
    TODO: Find a way to notificate the caller.
*/
HbAction *HbToolBarExtension::addAction( const QString &text, 
                                         const QObject *receiver, 
                                         const char *member )
{
    HbAction *action = new HbAction( text, this );
    QObject::connect( action, SIGNAL( triggered(bool) ), receiver, member );
    addAction(action);
    return action;
}

/*!
    \overload

    Creates a new action with the given  \a icon and \a text. 
    This action is added to the end of the toolbar extension.
    TODO: If the grid is already full, this call will be ignored.
    The action's \link HbAction::triggered()
    triggered()\endlink signal is connected to \a member in \a
    receiver.
    TODO: Find a way to notificate the caller.
*/
HbAction *HbToolBarExtension::addAction( const HbIcon &icon, 
                                         const QString &text, 
                                         const QObject *receiver, 
                                         const char *member )
{
    HbAction *action = new HbAction( icon, text, this );
    QObject::connect( action, SIGNAL( triggered(bool) ), receiver, member );
    addAction(action);
    return action;
}

/*!
    Returns the action associated with this extension.
 */
HbAction *HbToolBarExtension::extensionAction() const
{
    Q_D( const HbToolBarExtension );
    return d->extensionAction;
}

/*!
    Protected constructor.
*/
HbToolBarExtension::HbToolBarExtension( HbToolBarExtensionPrivate &dd, QGraphicsItem *parent ) :
        HbDialog( dd, parent )
{
}

/*!
    \reimp
 */
bool HbToolBarExtension::event( QEvent *event )
{
    Q_D( HbToolBarExtension );    
    if ( event->type() == QEvent::ActionAdded ) {
        d->actionAdded( static_cast<QActionEvent *>(event) );
        return true;
    } else if ( event->type() == QEvent::ActionRemoved ) {
        d->actionRemoved( static_cast<QActionEvent *>(event) );    
        return true;
    } else if (event->type() == QEvent::ActionChanged ) {
        d->actionChanged();
        return true;
    } else if ( event->type() == QEvent::GraphicsSceneResize ) {
        d->doLayout();
        // fall trough
    }
    return HbDialog::event(event);
}

/*!
  \reimp
  */
void HbToolBarExtension::polish( HbStyleParameters &params )
{    
    Q_D(HbToolBarExtension);  
    d->doLazyInit();
    const QString Margins       = "content-margins";
    const QString RowsPortrait  = "max-rows-portrait";
    const QString RowsLandscape = "max-rows-landscape";
    const QString ColsPortrait  = "max-columns-portrait";
    const QString ColsLandscape = "max-columns-landscape";

    params.addParameter( Margins );
    params.addParameter( RowsPortrait );
    params.addParameter( RowsLandscape );
    params.addParameter( ColsPortrait );
    params.addParameter( ColsLandscape );
    d->initialiseContent();
    if (d->mDefaultContentWidget) {       
        HbDialog::polish(params);
        if ( params.value( Margins ).isValid() 
             && params.value( RowsPortrait ).isValid() 
             && params.value( RowsLandscape ).isValid() 
             && params.value( ColsPortrait ).isValid() 
             && params.value( ColsLandscape ).isValid() ) {
            d->mMargins = params.value( Margins ).toReal();
            d->mRowsPortrait  = params.value( RowsPortrait ).toInt();
            d->mRowsLandscape = params.value( RowsLandscape ).toInt();
            d->mColsPortrait  = params.value( ColsPortrait ).toInt();
            d->mColsLandscape = params.value( ColsLandscape ).toInt();
            d->doLayout();
        }
    } else {
        HbDialog::polish(params);
    }
}

QVariant HbToolBarExtension::itemChange(GraphicsItemChange change, 
                                        const QVariant &value)
{
    Q_D(HbToolBarExtension);
    if (change == QGraphicsItem::ItemVisibleHasChanged) {
        if (value.toBool()) {
            HbMainWindow* w(mainWindow());
            if(w && !d->orientationConnectDone) {
                QObject::connect(w,SIGNAL(orientationChanged(Qt::Orientation)),
                                 this, SLOT(_q_orientationChanged()));
                d->orientationConnectDone = true;
            }
            d->placeToolBarExtension();
        }
    }

    return HbDialog::itemChange(change, value);
}

#include "moc_hbtoolbarextension.cpp"
