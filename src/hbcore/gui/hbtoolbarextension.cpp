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
#include "hbstyle_p.h"
#include <hbframeitem.h>
#ifdef HB_EFFECTS
#include <hbeffect.h>
#include <hbeffectinternal_p.h>
bool HbToolBarExtensionPrivate::extensionEffectsLoaded = false;
#endif

#include <QDebug>
#include <QGraphicsGridLayout>
#include <QEventLoop>
#include <QGraphicsLinearLayout>
    
/*!
  @stable
  @hbcore
  \class HbToolBarExtension
  \brief The HbToolBarExtension class provides a popup extension to a toolbar.
    
  You can use a toolbar extension to extend the main toolbar
  (HbToolBar class) in a view with additional actions that are placed
  in a subsidiary toolbar. Alternatively, a toolbar extension can
  contain a widget, such as a list or grid. This is useful, for
  example, for providing navigation between views when there are more
  views than can fit as actions on the toolbar itself.
    
  A toolbar can contain more than one toolbar extension. A toolbar
  extension opens when the user triggers the toolbar action that is
  associated with the extension, usually by tapping it. The user
  dismisses the toolbar extension by selecting an option (which runs a
  command) or by tapping outside the extension.
    
  The following image shows a toolbar that has two extensions: the
  leftmost one contains a list widget and the rightmost one contains
  three standard actions.
    
  \image html toolbarextension.png A toolbar that has two extensions
    
  Use addAction() to create an action and add it to the toolbar
  extension. There are several overloads of this function, which allow
  you to specify both a text and image or just a text and also to
  connect the action's \link HbAction::triggered() triggered()\endlink
  signal to a slot on a receiver object. Use the insertAction(),
  addActions() and insertActions() methods (which are inherited from
  QGraphicsWidget) to add existing actions to the toolbar
  extension. Use clearActions() to clear all of the actions and
  removeAction() to remove individual actions.
    
  The order of the actions within the toolbar extension controls the
  order of the buttons that the user sees. addAction() and
  addActions() append the actions to the end of the toolbar and
  insertAction() and insertActions() enable you to specify the
  required position.
    
  You can use the HbDialog API to fill the toolbar extension popup
  with widgets (such as a list, grid or line edit). If you do this,
  any actions that you add to the toolbar extension will not be
  visible.
    
  \section _usecases_hbtoolbarextension Using the HbToolBarExtension class
    
  \subsection _uc_001_hbtoolbarextension Creating a toolbar extension containing actions
  The following example demonstrates how to add a toolbar extension button to the toolbar
  and how to add actions to the toolbar extension.
  \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,27}
    
  \subsection _uc_002_hbtoolbarextension Creating a toolbar extension containing a widget
    
  The following example demonstrates creating a toolbar extension containing a single-selection
  list widget.
    
  \code
  // Create the toolbar.
  HbToolBar *toolBar = new HbToolBar();
    
  // Add the action that will open the toolbar extension.
  HbAction *radioAction = toolBar->addAction("Channel");
    
  // Create the toolbar extension.
  HbToolBarExtension *radioExtension = new HbToolBarExtension();
    
  // Set the heading.
  HbLabel* heading = new HbLabel(QString("Channel"));
  radioExtension->setHeadingWidget(heading);
    
  // Create a list widget.
  HbListWidget *list = new HbListWidget();
  list->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    
  // Make the list single selection.
  list->setSelectionMode(HbAbstractItemView::SingleSelection);
    
  // Add list items.
  list->addItem("574.7");
  list->addItem("976.5");
  list->addItem("108.67");
    
  // Add the list widget to the toolbar extension object.
  radioExtension->setContentWidget(list);
    
  // Add the toolbar extension to the toolbar action that will open it.
  radioAction->setToolBarExtension(radioExtension);
  \endcode
*/
    
/*!
  \fn int HbToolBarExtension::type() const
*/
    
/*
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
    mRowsPortrait(4),
    mRowsLandscape(3),
    mColsPortrait(3),
    mColsLandscape(4),
    lazyInitDone(false),
    orientationConnectDone(false),
    mExtendedButton(0),
    mToolBar(0)
{
    frameType = HbPopup::Weak;
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
    if ( !lazyInitDone ) {
        Q_Q(HbToolBarExtension);
        QGraphicsObject *backgroundItem = q->style()->createPrimitive(HbStyle::PT_FrameItem, "background", q);
        
        HbStyleFramePrimitiveData data;
        q->initPrimitiveData(&data, backgroundItem);
        q->style()->updatePrimitive(backgroundItem, &data, 0);
        q->setBackgroundItem(backgroundItem, -1);
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
    mLayout->setContentsMargins(0, 0, 0, 0);
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
        if (!q->contentWidget()) {
            initialiseContent(); // create now to prevent mem leak below
        }
        button = new HbToolButton(hbAction, q->contentWidget());
    } else {
        button = new HbToolButton(q->contentWidget());
        HbToolButtonPrivate::d_ptr(button)->action = event->action();
        QObject::connect(event->action(), SIGNAL(triggered()), button, SLOT(_q_actionTriggered()));
        QObject::connect(event->action(), SIGNAL(changed()), button, SLOT(_q_actionChanged()));
    }

    if ((hbAction && !hbAction->icon().isNull()) || !event->action()->icon().isNull()) {
        q->setProperty("icon", true);
    } else {
        q->setProperty("icon", false);
    }
    if (!HbToolButtonPrivate::d_ptr(button)->action->text().isEmpty()) {
        q->setProperty("text", true);
    } else {
        q->setProperty("text", false);
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

void HbToolBarExtensionPrivate::_q_toolbarOrientationChanged()
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
}

void HbToolBarExtensionPrivate::_q_orientationChanged()
{
    Q_Q(HbToolBarExtension);
    q->close();
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
}
    
    
/*!  
  Creates a new action with the given \a text and adds the action
  to the end of the toolbar extension, provided space is
  available. The space available in a toolbar extension depends on the
  screen size and orientation. When there is no free space, this
  function does nothing. There is currently no notification when there
  is no free space.
    
  \overload
  \return The new action.
*/
HbAction *HbToolBarExtension::addAction( const QString &text )
{
    HbAction *action = new HbAction( text, this );
    addAction(action);
    return action;
}
    
    
/*!  
  Creates a new action with the given \a icon and \a text and adds
  the action to the end of the toolbar extension, provided space is
  available. The space available in a toolbar extension depends on the
  screen size and orientation. When there is no free space, this
  function does nothing. There is currently no notification when there
  is no free space.
    
  \overload
  \return The new action.
*/
HbAction *HbToolBarExtension::addAction( const HbIcon &icon, 
                                         const QString &text )
{
    HbAction *action = new HbAction( icon, text, this );
    addAction(action);
    return action;
}
    
    
/*!  
  Creates a new action with the given \a text, adds the action to
  the end of the toolbar extension (provided space is available), and
  connects the action's \link HbAction::triggered()
  triggered()\endlink signal to a receiver object's slot.
    
  The space available in a toolbar extension depends on the screen
  size and orientation.  When there is no free space, this function
  does not add the action to the toolbar extension. There is currently
  no notification when there is no free space.
    
  \overload
  \param text The text for the new action.
  \param receiver The object that is to receive the new action's signal.
  \param member The slot on the receiver to which the action's signal is to connect.
  \return The new action.
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
  Creates a new action with the given \a icon and \a text, adds the
  action to the end of the toolbar extension (provided space is
  available), and connects the action's \link HbAction::triggered()
  triggered()\endlink signal to a receiver object's slot.
    
  The space available in a toolbar extension depends on the screen
  size and orientation.  When there is no free space, this function
  does not add the action to the toolbar extension. There is currently
  no notification when there is no free space.
    
  \overload
  \param icon The image for the new action.
  \param text The text for the new action.
  \param receiver The object that is to receive the new action's signal.
  \param member The slot on the receiver to which the action's signal is to connect.
  \return The new action.
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
  Returns the action associated with this toolbar extension. This
  is the action in the toolbar to which this toolbar extension belongs
  that opens this toolbar extension when triggered.
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
    
 */
void HbToolBarExtension::polish( HbStyleParameters &params )
{
    if (isVisible()) {
        Q_D(HbToolBarExtension);
        d->doLazyInit();
        const QLatin1String RowsPortrait("max-rows-portrait");
        const QLatin1String RowsLandscape("max-rows-landscape");
        const QLatin1String ColsPortrait("max-columns-portrait");
        const QLatin1String ColsLandscape("max-columns-landscape");

        params.addParameter( RowsPortrait );
        params.addParameter( RowsLandscape );
        params.addParameter( ColsPortrait );
        params.addParameter( ColsLandscape );
        d->initialiseContent();
        if (d->mDefaultContentWidget) {
            HbDialog::polish(params);
            if ( params.value( RowsPortrait ).isValid()
                && params.value( RowsLandscape ).isValid()
                && params.value( ColsPortrait ).isValid()
                && params.value( ColsLandscape ).isValid() ) {
                d->mRowsPortrait  = params.value( RowsPortrait ).toInt();
                d->mRowsLandscape = params.value( RowsLandscape ).toInt();
                d->mColsPortrait  = params.value( ColsPortrait ).toInt();
                d->mColsLandscape = params.value( ColsLandscape ).toInt();
                d->doLayout();
            }
            return;
        }
    }
    HbDialog::polish(params);
}

QVariant HbToolBarExtension::itemChange(GraphicsItemChange change, 
                                        const QVariant &value)
{
    Q_D(HbToolBarExtension);
    if (change == QGraphicsItem::ItemVisibleHasChanged) {
        if (value.toBool()) {
            HbMainWindow* w(mainWindow());
            if(w && !d->orientationConnectDone) {
                QObject::disconnect( w , SIGNAL(aboutToChangeOrientation()), this, SLOT(_q_orientationChanged()));
                QObject::connect( w , SIGNAL(aboutToChangeOrientation()), this, SLOT(_q_orientationChanged()));
                d->orientationConnectDone = true;
            }
            d->placeToolBarExtension();
        }
    }

    return HbDialog::itemChange(change, value);
}

void HbToolBarExtension::initPrimitiveData(HbStylePrimitiveData *primitiveData,
                                           const QGraphicsObject *primitive)
{
    Q_D(HbToolBarExtension);
    HbDialog::initPrimitiveData(primitiveData, primitive);
    QString itemName = HbStyle::itemName(primitive);
    if (itemName == QLatin1String("background")) {
        HbStyleFramePrimitiveData *data = hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData);
        data->frameGraphicsName = d->frameType == HbPopup::Strong ?
            QLatin1String("qtg_fr_popup") :
            QLatin1String("qtg_fr_popup_trans");
        data->frameType = HbFrameDrawer::NinePieces;
    }
}

void HbToolBarExtensionPrivate::doSetFrameType(HbPopup::FrameType newFrameType)
{
    Q_Q(HbToolBarExtension);
    switch( newFrameType ) {
    case HbPopup::Weak:
        q->setBackgroundItem(new HbFrameItem(QLatin1String("qtg_fr_popup_trans"), HbFrameDrawer::NinePieces), -1);
        break;
    case HbPopup::Strong:
    default:
        q->setBackgroundItem(new HbFrameItem(QLatin1String("qtg_fr_popup"), HbFrameDrawer::NinePieces), -1);
        break;
    }

}
#include "moc_hbtoolbarextension.cpp"
