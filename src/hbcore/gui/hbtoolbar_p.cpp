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

#include "hbtoolbar_p.h"
#include "hbtoolbar.h"
#include "hbaction.h"
#include "hbtoolbutton.h"
#include "hbtoolbutton_p.h"
#include "hbapplication.h"
#include "hbnamespace_p.h"
#include "hbtoolbarextension.h"
#include "hbtoolbarextension_p.h"
#include "hbwidget_p.h"
#include "hbinstance.h"
#include "hbactionmanager_p.h"
#include "hbcolorscheme.h"

#include <hbwidgetfeedback.h>

#include <QGraphicsLinearLayout>
#include <QGraphicsSceneResizeEvent>
#include <QWidget> // for QWIDGETSIZE_MAX
#include <QActionEvent>
#include <QMetaMethod>
#include <QDebug>

#ifdef HB_EFFECTS
#include "hbeffectinternal_p.h"
#define HB_TOOLBAR_ITEM_TYPE "HB_TOOLBAR"
bool HbToolBarPrivate::effectsLoaded = false;
#endif

//If minimum button size cannot be calculated
//more button will appear after default button count is exceeded
static const int DEFAULT_TOOLBUTTON_COUNT = 5;

HbToolBarPrivate::HbToolBarPrivate() :
        mLayout(0),
        mPressedDownButton(0),
        mPreviouslyPressedDownButton(0),
        mOrientation(Qt::Horizontal),
        actionManager(0),
        hasEffects(false),
        hidingInProgress(false),
        delayedHide(false),
        delayedStartEffects(false),
        emitVisibilityChangeSignal(false),
        minimumToolButtonSize(QSizeF()),
        maxToolBarButtons(-1),
        moreExtension(0),
        moreExtensionButton(0),
        mDialogToolBar(false),
        mDoLayoutPending(true),
        mOrientationEffectsRunning(false),
        mSuppressNextAppearEffect(false),
        mAppearedOnce(false),
        initialButtonsPolish(true)
{
}

HbToolBarPrivate::~HbToolBarPrivate()
{
    if (moreExtension) {
        moreExtension->deleteLater();
    }
    foreach (HbToolButton *button, mToolButtons) {
        delete button;
        button = 0;
    }
}

void HbToolBarPrivate::init()
{
}

void HbToolBarPrivate::doLazyInit()
{
#ifdef HB_EFFECTS
    hasEffects = effectsLoaded;
    if (effectsLoaded) {
        return;
    }
    hasEffects = HbEffectInternal::add(HB_TOOLBAR_ITEM_TYPE, "toolbar_appear", "appear");
    if (hasEffects) {
        hasEffects = HbEffectInternal::add(HB_TOOLBAR_ITEM_TYPE,
                                           "toolbar_appear_left", "appear_left");
    }
    if (hasEffects) {
        hasEffects = HbEffectInternal::add(HB_TOOLBAR_ITEM_TYPE,
                                           "toolbar_appear_right", "appear_right");
    }
    if (hasEffects) {
        hasEffects = HbEffectInternal::add(HB_TOOLBAR_ITEM_TYPE,
                                           "toolbar_disappear", "disappear");
    }
    if (hasEffects) {
        hasEffects = HbEffectInternal::add(HB_TOOLBAR_ITEM_TYPE,
                                           "toolbar_disappear_left", "disappear_left");
    }
    if (hasEffects) {
        hasEffects = HbEffectInternal::add(HB_TOOLBAR_ITEM_TYPE,
                                           "toolbar_disappear_right", "disappear_right");
    }
    if (hasEffects) {
        hasEffects = HbEffectInternal::add(HB_TOOLBAR_ITEM_TYPE,
                                           "toolbar_orient_appear", "appear_orient");
    }
    if (hasEffects) {
        hasEffects = HbEffectInternal::add(HB_TOOLBAR_ITEM_TYPE,
                                           "toolbar_orient_disappear", "disappear_orient");
    }

    if (hasEffects) {
        hasEffects = HbEffectInternal::add(HB_TOOLBAR_ITEM_TYPE,
                                           "toolbar_orient_appear_right", "appear_orient_right");
    }
    if (hasEffects) {
        hasEffects = HbEffectInternal::add(HB_TOOLBAR_ITEM_TYPE,
                                           "toolbar_orient_disappear_right", "disappear_orient_right");
    }

    if (hasEffects) {
        hasEffects = HbEffectInternal::add(HB_TOOLBAR_ITEM_TYPE,
                                           "toolbar_orient_appear_left", "appear_orient_left");
    }
    if (hasEffects) {
        hasEffects = HbEffectInternal::add(HB_TOOLBAR_ITEM_TYPE,
                                           "toolbar_orient_disappear_left", "disappear_orient_left");
    }
    effectsLoaded = true;
#endif    
}

//This will be used by hbscreen to control when
//toolbar layouting is handled
void HbToolBarPrivate::delayedConstruction()
{

}

void HbToolBarPrivate::setOrientation( Qt::Orientation orientation )
{
    if ( mOrientation != orientation ) {
        mOrientation = orientation;
        emit core.orientationChanged();
        updateButtonsLayoutDirection();
    }
}

void HbToolBarPrivate::calculateMaximumButtons() {
    Q_Q(HbToolBar);
    maxToolBarButtons = DEFAULT_TOOLBUTTON_COUNT;
    if (!minimumToolButtonSize.isEmpty()) {
        if (mOrientation == Qt::Horizontal) {
            if ((q->size().width() > 0) && (q->size().width() > minimumToolButtonSize.width()))
                maxToolBarButtons = int(q->size().width() / minimumToolButtonSize.width());
        } else {
            if ((q->size().height() > 0) && (q->size().height() > minimumToolButtonSize.height()))
                maxToolBarButtons = int(q->size().height() / minimumToolButtonSize.height());
        }
    }
}

//This is called when new action is added and toolbar is visible and polished.
//we check if a full layout is needed if not just add the
//new button in right place.
void HbToolBarPrivate::actionAdded(int index)
{
    if (fullUpdateNeeded()){
        doLayout();
        return;
    }
    if (mVisibleToolButtons.count()<=maxToolBarButtons){
        HbToolButton *button = mVisibleToolButtons.at(index);
        prepareButtonForToolbar(button);
        mLayout->insertItem(index, button);
    }
    else{
        if (moreExtension){
            prepareButtonForExtension(mVisibleToolButtons.at(index));
            HbToolBarExtensionPrivate::d_ptr(moreExtension)->doLayout();
        }
    }
    updateButtons(maxToolBarButtons);
}

//This is called when new action is removed and toolbar is visible and polished.
//we check if a full layout is needed if not just remove the
//button from right place.
void HbToolBarPrivate::actionRemoved(int index,HbToolButton *button)
{
    if (fullUpdateNeeded(index)){
        doLayout();
        return;
    }
    if (index<=maxToolBarButtons -1){
        if (mLayout && button){
            mLayout->removeItem(button);
            mLayout->invalidate();
            mLayout->activate();
            updateButtons(maxToolBarButtons);
        }
    }
    else if(moreExtension){
        int buttonindex = HbToolBarExtensionPrivate::d_ptr(moreExtension)->mToolButtons.indexOf(button);
        if (buttonindex != -1){
            HbToolBarExtensionPrivate::d_ptr(moreExtension)->mToolButtons.removeAt(buttonindex);
            HbToolBarExtensionPrivate::d_ptr(moreExtension)->doLayout();
        }
    }
}

//This is called when toolbar receives resize event.
//we check if a full layout is needed than call doLayout.
//if maxVisiblebuttons have changed than also we do a complete
//layout

void HbToolBarPrivate::updateToolBarForSizeChange()
{
    if (mDoLayoutPending || maxToolBarButtons == -1 || !minimumToolButtonSize.isValid()){
        doLayout();
        return;
    }
    const int currentMaxButtons = maxToolBarButtons;
    calculateMaximumButtons();
    if (currentMaxButtons == maxToolBarButtons)
        return;

    if (mVisibleToolButtons.count()<=maxToolBarButtons  &&
        (!moreExtensionButton  ||
         (moreExtensionButton &&
          !HbToolButtonPrivate::d_ptr(moreExtensionButton)->action->isVisible()))) {
        return;
    }
    doLayout();
}

//Here we check if a full update for toolbar is needed or not
bool HbToolBarPrivate::fullUpdateNeeded()
{
    if (maxToolBarButtons == -1 || !minimumToolButtonSize.isValid() || !mLayout)
        return true;
    int totalButtons = mVisibleToolButtons.count()-1;

    if (totalButtons>maxToolBarButtons -1 &&
        (!moreExtensionButton  ||
         (moreExtensionButton &&
          !HbToolButtonPrivate::d_ptr(moreExtensionButton)->action->isVisible()))) {
        return true;
    }
    if (totalButtons<=maxToolBarButtons  &&
        (moreExtensionButton &&
         HbToolButtonPrivate::d_ptr(moreExtensionButton)->action->isVisible())) {
        return true;
    }
    return false;
}

//Here we check if a full update for toolbar is needed or not
bool HbToolBarPrivate::fullUpdateNeeded(int index)
{
    if (maxToolBarButtons == -1 || !minimumToolButtonSize.isValid() || !mLayout)
        return true;
    if (index>maxToolBarButtons -1 &&  (!moreExtensionButton  || (moreExtensionButton && !moreExtensionButton ->isVisible())))
        return true;
    if (index<=maxToolBarButtons  && ((moreExtensionButton && moreExtensionButton ->isVisible()) ||
                                      (moreExtension &&  mVisibleToolButtons.count() >= maxToolBarButtons)))
        return true;
    return false;
}

//calculates minimum button size
void HbToolBarPrivate::calculateButtonMinimumSize()
{
    HbToolButton *button = mVisibleToolButtons.first();
    //Avoiding unnecessary polish, so setting parameters for first button
    prepareButtonForToolbar(button);
    minimumToolButtonSize = HbToolButtonPrivate::d_ptr(button)->getMinimumSize();
}


//We prepare  button to be added to toolbarextension(in more button)
void HbToolBarPrivate::prepareButtonForExtension(HbToolButton *button)
{
    if(!moreExtension)
        return;
    HbToolBarExtensionPrivate::d_ptr(moreExtension)->mToolButtons.append(button);
    updateButtonStyle(button, false);
    setExtensionLayout(button, true);
    if (!button->parentItem() || button->parentItem() != moreExtension->contentWidget()){
        button->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
        button->setPreferredSize(QSizeF());
        QAction *action = HbToolButtonPrivate::d_ptr(button)->action;
        QObject::connect(action, SIGNAL(triggered(bool)),
                         moreExtension, SLOT(close()));
        button->setParentItem(moreExtension->contentWidget());
    }
}

//We prepare button to be added for toolbar
void HbToolBarPrivate::prepareButtonForToolbar(HbToolButton *button)
{
    Q_Q(HbToolBar);
    updateButtonStyle(button, true);
    if (!button->parentItem() || button->parentItem() != q){
        bool movingFromExtension = moreExtension && (button->parentItem() == moreExtension->contentWidget());
        button->setParentItem(q);
        setExtensionLayout(button, false);
        // tool buttons should eat all the space in the toolbar
        if (mOrientation == Qt::Horizontal)
            button->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred));
        else
            button->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored));

        if (movingFromExtension){
            QAction *action = HbToolButtonPrivate::d_ptr(button)->action;
            QObject::disconnect(action, SIGNAL(triggered(bool) ),
                                moreExtension, SLOT(close()));
            HbToolButtonPrivate::d_ptr(button)->setExtensionBackgroundVisible(false);
        }
    }
 
}

//resets all visible buttons list.This is called when actionChange event is received
//by toolbar and from doLayout when mDoLayoutPending is true
void HbToolBarPrivate::resetVisibleButtonsList()
{
    mVisibleToolButtons.clear();
    foreach(HbToolButton *button, mToolButtons) {
        QAction *action = HbToolButtonPrivate::d_ptr(button)->action;
        if (action->isVisible()) {
            mVisibleToolButtons.append(button);
            button->setVisible(true);
        } else {
            button->setVisible(false);
        }
    }
}

//creates a new toolbutton and calls actionAdded if update is true.
//update can be true in cases where toolbar is polished and actionAdded
//event is received by toolbar.
//we emit visibilitychanged signal here if this is the firstAction added in cases
//after toolbar is polished and visible.
void HbToolBarPrivate::createToolButton(QAction *newAction, bool update)
{
    Q_Q(HbToolBar);
    if(!newAction)
        return;
    
    HbAction* hbAction = qobject_cast<HbAction*>(newAction);

    // Find out index where to insert button
    int index = q->actions().indexOf(newAction);

    HbToolButton *button = new HbToolButton;

    if (hbAction) {
        // Lets give the action manager possibility to calculate position for the action
        if (hbAction->commandRole() != HbAction::NoRole) {
            // Create action manager when needed
            if (!actionManager) {
                actionManager = new HbActionManager(HbView::ToolBar, q, q->mainWindow());
            }
            if (actionManager){
                index = actionManager->position(hbAction, q->actions(), mVisibleToolButtons.count());
            }
            
        }
        HbToolBarExtension *extension = hbAction->toolBarExtension();
        if (extension && HbToolBarExtensionPrivate::d_ptr(extension)->mToolBar != q) {
            initToolBarExtension(extension);
        }

        button->setAction(hbAction);
    }
    else {
        // The new action's type is QAction.
        HbToolButtonPrivate::d_ptr(button)->action = newAction;
        QObject::connect(newAction, SIGNAL(triggered()), button, SLOT(_q_actionTriggered()));
        QObject::connect(newAction, SIGNAL(changed()), button, SLOT(_q_actionChanged()));
    }

    HbToolButtonPrivate::d_ptr(button)->mDialogToolBar = mDialogToolBar;
    setButtonLayoutDirection(*button);
    mToolButtons.insert(index, button);
    if (newAction->isVisible()){
        int visibleIndex = 0;
        for (int i = 0; i < index; i++) {
            QAction *action = HbToolButtonPrivate::d_ptr(mToolButtons.at(i))->action;
            if (action->isVisible())
                visibleIndex++;
        }
        mVisibleToolButtons.insert(visibleIndex, button);
        if (update){
            if (emitVisibilityChangeSignal) {
                delayedStartEffects = !mSuppressNextAppearEffect;
                QMetaObject::invokeMethod(&core, "visibilityChanged");
                emitVisibilityChangeSignal = false;
            }
            actionAdded(visibleIndex);
        }
    }

}

//called from doLayout.We update the more button and extension as needed
void HbToolBarPrivate::updateExtension(bool moreButtonNeeded)
{
    Q_Q(HbToolBar);
    if (moreExtension) {
        HbToolBarExtensionPrivate::d_ptr(moreExtension)->mToolButtons.clear();
        HbToolBarExtensionPrivate::d_ptr(moreExtension)->contentWidget->setLayout(0);
        HbToolButtonPrivate::d_ptr(moreExtensionButton)->action->setVisible(moreButtonNeeded);
        moreExtensionButton->setVisible(moreButtonNeeded);
    }
    else if (moreButtonNeeded && !moreExtension)  {
        moreExtension = new HbToolBarExtension();
        moreExtensionButton = new HbToolButton(moreExtension->extensionAction(), q);
        moreExtensionButton->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));

        HbIcon extensionIcon("qtg_mono_more");
        moreExtension->extensionAction()->setIcon(extensionIcon);
        initToolBarExtension(moreExtension);
        HbToolButtonPrivate::d_ptr(moreExtensionButton)->setToolBarPosition(
                HbToolButtonPrivate::TB_End);
    }
}

void HbToolBarPrivate::startAppearOrientEffect()
{
    Q_Q(HbToolBar);
    if (!q->isVisible() && !mOrientationEffectsRunning)
        return;
    updateToolBarForSizeChange();
    QGraphicsItem *parentItem = q->parentItem();

    if (parentItem && q->orientation() == Qt::Horizontal) {
        QRectF extRect(0.0,
                       parentItem->boundingRect().height(),
                       q->boundingRect().width(),
                       q->boundingRect().height());
        HbEffect::start(q, HB_TOOLBAR_ITEM_TYPE, "appear_orient");
        
    } else if (parentItem && q->orientation() == Qt::Vertical) {
        QRectF extRect(parentItem->boundingRect().width(),
                       parentItem->boundingRect().height() - q->boundingRect().height(),
                       q->boundingRect().width(),
                       q->boundingRect().height());

        if (q->layoutDirection() == Qt::LeftToRight)
            HbEffect::start(q, HB_TOOLBAR_ITEM_TYPE, "appear_orient_right", q, 0, QVariant(), extRect);
        else if (q->layoutDirection() == Qt::RightToLeft) {
            extRect.setX(-extRect.x());
            HbEffect::start(q, HB_TOOLBAR_ITEM_TYPE, "appear_orient_left", q, 0, QVariant(), extRect);
        }
    }
    q->setVisible(true);
    mOrientationEffectsRunning = false;
}

void HbToolBarPrivate::startDisappearOrientEffect()
{
    Q_Q(HbToolBar);
    if (!q->isVisible())
        return;
    QGraphicsItem *parentItem = q->parentItem();

    if ( moreExtension && moreExtension->isVisible() ) {
        moreExtension->hide();
    }

    if (parentItem && q->orientation() == Qt::Horizontal) {
        QRectF extRect(0.0,
                       parentItem->boundingRect().height(),
                       q->boundingRect().width(),
                       q->boundingRect().height());
        HbEffect::start(q, HB_TOOLBAR_ITEM_TYPE, "disappear_orient",q,"_q_delayedHide");
    } else if (parentItem && q->orientation() == Qt::Vertical) {
        QRectF extRect(parentItem->boundingRect().width(),
                       parentItem->boundingRect().height() - q->boundingRect().height(),
                       q->boundingRect().width(),
                       q->boundingRect().height());

        if (q->layoutDirection() == Qt::LeftToRight)
            HbEffect::start(q, HB_TOOLBAR_ITEM_TYPE, "disappear_orient_right", q,"_q_delayedHide", QVariant(), extRect);
        else if (q->layoutDirection() == Qt::RightToLeft) {
            extRect.setX(-q->boundingRect().width());
            HbEffect::start(q, HB_TOOLBAR_ITEM_TYPE, "disappear_orient_left", q,"_q_delayedHide", QVariant(), extRect);
        }
    }
  mOrientationEffectsRunning = true;
}

//Does a complete update of toolbar
void HbToolBarPrivate::doLayout()
{    
    Q_Q(HbToolBar);
    if (q->actions().isEmpty()){
        mDoLayoutPending = false;
        return;
    }

    if (mDoLayoutPending){
        if (!mToolButtons.count()){
            foreach(QAction *action, q->actions()) {
                createToolButton(action);
            }
        }
        else
            resetVisibleButtonsList();
        mDoLayoutPending = false;
    }
    if (mVisibleToolButtons.isEmpty()){
        return;
    }
    //Saving minimumSize for the more button calculations
    bool firstButton =  mToolButtons.count() == 1 ? true : false;

    mLayout = new QGraphicsLinearLayout( q->orientation() );
    mLayout->setSpacing(0);
    mLayout->setContentsMargins(0,0,0,0);

    // Recalculate minimum button size when adding first button.
    // minimumToolButtonSize might have wrong value incase it has been
    // calculated earlier using button which have been deleted.
    if (!minimumToolButtonSize.isValid() || firstButton ) {
        if (mVisibleToolButtons.count() > 0)
            q->setVisible(true); // The toolbar must be set visible in order to correctly
                                 // calculate minimum button size in all situations (also when
                                 // the toolbar was hidden).
        calculateButtonMinimumSize();
    }

    //Calculate maximum number of buttons in toolbar
    calculateMaximumButtons();
    int visibleItemsCount = mVisibleToolButtons.count();
    bool moreButtonNeeded = visibleItemsCount > maxToolBarButtons;

    if (moreButtonNeeded ||
        (moreExtensionButton &&
         HbToolButtonPrivate::d_ptr(moreExtensionButton)->action->isVisible() != moreButtonNeeded)) {
        updateExtension(moreButtonNeeded);
    }

    //Add toolbuttons to layout or in toolbar extension
    int buttonsinToolBar = moreButtonNeeded ? maxToolBarButtons-1 :maxToolBarButtons;
    for (int i = 0; i < visibleItemsCount; i++) {
        HbToolButton *button = mVisibleToolButtons.at(i);
        if (i < buttonsinToolBar) {
            prepareButtonForToolbar(button);
            mLayout->addItem(button);
        } else if (moreButtonNeeded){
            if (i == maxToolBarButtons-1) {
                mLayout->addItem(moreExtensionButton);
                setExtensionLayout(moreExtensionButton, false);
            }
            prepareButtonForExtension(button);
        }
    }

    if (moreButtonNeeded) {
        HbToolBarExtensionPrivate::d_ptr(moreExtension)->doLayout();
    }    
    updateButtons(maxToolBarButtons);        
    q->setLayout(mLayout);
    q->layout()->setMinimumSize(minimumToolButtonSize);
    //This needs to be done when toolbar is not in a dialog and being added first time to view.
    //Since toolbar needs to make a decision of having a more button or not based on the no of actions and
    //its width. Width is not set correctly till hbscreen layout is activated.HbScreen layout is activated before
    //the paint but its too late for toolbar since all the polish handling is done beforei.e toolbuttons layout is not
    //activated before the paint
    if (initialButtonsPolish && !mDialogToolBar && q->scene()) {
        int slotIndex = q->scene()->metaObject()->indexOfSlot("_q_polishItems()");
        QMetaMethod method = q->scene()->metaObject()->method(slotIndex);
        method.invoke(q->scene());
        for (int i = 0; i < visibleItemsCount; i++) {
            HbToolButton *button = mVisibleToolButtons.at(i);
            if (button && button->layout() && !button->layout()->isActivated())
                button->layout()->activate();
        }
        q->layout()->activate();
    }
    initialButtonsPolish = false;
    if (delayedStartEffects && hasEffects && !mSuppressNextAppearEffect) {
        startAppearEffect();
        delayedStartEffects = false;
    }
}

void HbToolBarPrivate::setExtensionLayout(HbToolButton *button, bool extensionLayout)
{
    if (extensionLayout) {
        if (button->property("toolbutton_extension_layout") == false ||
            button->property("toolbutton_extension_layout").isNull()) {
            HbToolButtonPrivate::d_ptr(button)->setLayoutProperty("toolbutton_extension_layout", true);
        }
    } else {
        if (button->property("toolbutton_extension_layout") == true) {
            HbToolButtonPrivate::d_ptr(button)->setLayoutProperty("toolbutton_extension_layout", false);
        }
    }
}

void HbToolBarPrivate::actionAdded( QActionEvent *event )
{
    Q_Q(HbToolBar);
    // Emit signal when the first action is added
    if (!emitVisibilityChangeSignal && event->action()->isVisible() && !mVisibleToolButtons.count())
        emitVisibilityChangeSignal = true;

    HbAction *action = qobject_cast<HbAction*>(event->action());
    if (action && action->toolBarExtension()) {
        initToolBarExtension(action->toolBarExtension());
    }

    if (!polished)
        mDoLayoutPending = true;
    else{
        createToolButton(event->action(),q->isVisible());
        if (!event->action()->isVisible())
            return;
        if (!q->isVisible()) {
            mDoLayoutPending = true;
        }
    }
}

void HbToolBarPrivate::actionRemoved( QActionEvent *event )
{
    if (!mToolButtons.count())
        return;
    Q_Q(HbToolBar);
    HbToolButton *button = 0;
    for (int i = 0; i < mToolButtons.count(); i++) {
        button = mToolButtons.at(i);
        if (button && HbToolButtonPrivate::d_ptr(button)->action == event->action()) {
            mToolButtons.removeAt(i);            
            // Emit signal when the only action is removed
            if (mToolButtons.count() == 0) {
                if (moreExtensionButton)
                    moreExtensionButton->setVisible(false);
                mVisibleToolButtons.clear();
                delete button;
                q->setLayout(0);
                mLayout = 0;
                //reset toolbar size
                q->resize(0, 0);
                if (q->isVisible())
                    QMetaObject::invokeMethod(&core, "visibilityChanged", Qt::QueuedConnection);
                return;
            }
            int index = mVisibleToolButtons.indexOf(button);
            if (index != -1){
                mVisibleToolButtons.removeAt(index);
                if (mVisibleToolButtons.isEmpty()){
                    q->setLayout(0);
                    mLayout = 0;
                }
                else
                    actionRemoved(index,button);
            }
            delete button;

            break;
        }
    }
}

void HbToolBarPrivate::updateButtonStyle(HbToolButton *button, bool notInExtension)
{
    Q_Q(HbToolBar);
    Q_UNUSED(notInExtension);
    if (!button)
        return;
    // Set style for all toolbuttons. If toolbar orientation is vertical then
    // style mustn't be HbToolButton::ToolButtonTextAndIcon unless the button is in
    // the toolbar extension.
    // button orientation is orthogonal to the toolbar orientation
    Qt::Orientation buttonOrientation = q->orientation() == Qt::Vertical ?
                                        Qt::Horizontal : Qt::Vertical;
    if (buttonOrientation != HbToolButtonPrivate::d_ptr(button)->orientation) {
        HbToolButtonPrivate::d_ptr(button)->setOrientation( buttonOrientation );
    } else {
        button->updatePrimitives();
    }
}

void HbToolBarPrivate::updateButtons(int maxToolbarButtons)
{
    Q_Q(HbToolBar);

    if (mVisibleToolButtons.isEmpty()) {
        return;
    }

    // button orientation is orthogonal to the toolbar orientation
    Qt::Orientation buttonOrientation = q->orientation() == Qt::Vertical ?
                                        Qt::Horizontal : Qt::Vertical;

    int totalVisibleItems = mVisibleToolButtons.count();
    bool moreButtonNeeded = (maxToolbarButtons < totalVisibleItems);

    //Update more button graphics
    if (moreButtonNeeded && moreExtensionButton) {
        HbToolButtonPrivate::d_ptr(moreExtensionButton)->setOrientation(buttonOrientation);
    }

    if (totalVisibleItems == 1 && !moreButtonNeeded) {
        HbToolButtonPrivate *first = HbToolButtonPrivate::d_ptr(mVisibleToolButtons.first());
        first->setToolBarPosition(HbToolButtonPrivate::TB_OnlyOne);
    } else if (maxToolbarButtons > 1 && mVisibleToolButtons.count() > 1) {
        HbToolButtonPrivate *first = HbToolButtonPrivate::d_ptr(mVisibleToolButtons.first());
        first->setToolBarPosition(HbToolButtonPrivate::TB_Beginning);
        
        int lastMiddleButton = (moreButtonNeeded) ?
                               maxToolbarButtons - 2 : mVisibleToolButtons.count() - 2;
        if (!moreButtonNeeded) {
            HbToolButtonPrivate *last = HbToolButtonPrivate::d_ptr(mVisibleToolButtons.last());
            last->setToolBarPosition(HbToolButtonPrivate::TB_End);
        }
        for (int i = 1; i <= lastMiddleButton; ++i) {
            HbToolButtonPrivate *middle = HbToolButtonPrivate::d_ptr(mVisibleToolButtons.at(i));
            middle->setToolBarPosition(HbToolButtonPrivate::TB_Middle);
        }
    }
}

void HbToolBarPrivate::initToolBarExtension(HbToolBarExtension *extension) {
    Q_Q(HbToolBar);
    HbToolBarExtensionPrivate::d_ptr(extension)->mToolBar = q;

    if (mOrientation == Qt::Horizontal) {
        HbToolBarExtensionPrivate::d_ptr(extension)->setAlignment(Qt::AlignTop);
    } else if (mOrientation == Qt::Vertical && q->layoutDirection() == Qt::LeftToRight) {
        HbToolBarExtensionPrivate::d_ptr(extension)->setAlignment(Qt::AlignLeft);
    } else {
        HbToolBarExtensionPrivate::d_ptr(extension)->setAlignment(Qt::AlignRight);
    }

    if (q->scene()) q->scene()->addItem(extension);

    q->connect(&core, SIGNAL(orientationChanged()), extension, SLOT(_q_toolbarOrientationChanged()));
}

void HbToolBarPrivate::updateToolBarExtensions() {
    Q_Q(HbToolBar);
    if (mOrientation == Qt::Vertical) {
        Qt::Alignment alignment = Qt::AlignLeft;
        if (q->layoutDirection() == Qt::RightToLeft) {
            alignment = Qt::AlignRight;
        }
        if (moreExtension) {
            HbToolBarExtensionPrivate::d_ptr(moreExtension)->setAlignment(alignment);
        }
        foreach (QAction *action, q->actions()) {
            HbAction *hbaction = static_cast<HbAction*>(action);
            if (hbaction && hbaction->toolBarExtension()) {
                HbToolBarExtensionPrivate::d_ptr(
                        hbaction->toolBarExtension())->setAlignment(alignment);
            }
        }
    }
}

void HbToolBarPrivate::setButtonLayoutDirection( HbToolButton& button ) {
    Q_Q(HbToolBar);
    if (q->layoutDirection() !=  button.layoutDirection() && q->orientation() == Qt::Horizontal) {
        button.setLayoutDirection(q->layoutDirection());
    }
    if (q->layoutDirection() ==  button.layoutDirection() && q->orientation() == Qt::Vertical) {
        if (q->layoutDirection() == Qt::RightToLeft) {
            button.setLayoutDirection(Qt::LeftToRight);
        }
        else {
            button.setLayoutDirection(Qt::RightToLeft);
        }
    }
}

void HbToolBarPrivate::updateButtonsLayoutDirection() {
    foreach (HbToolButton *button, mToolButtons) {
        setButtonLayoutDirection(*button);
    }
    if (moreExtensionButton) {
        setButtonLayoutDirection(*moreExtensionButton);
    }
}

void HbToolBarPrivate::updateDialogToolbar(bool enable)
{
    mDialogToolBar = enable;
    foreach (HbToolButton *button, mToolButtons) {
        HbToolButtonPrivate::d_ptr(button)->mDialogToolBar = mDialogToolBar;
        button->updatePrimitives();
    }
    if (moreExtensionButton) {
        HbToolButtonPrivate::d_ptr(moreExtensionButton)->mDialogToolBar = mDialogToolBar;
        moreExtensionButton->updatePrimitives();
    }
}


#ifdef HB_EFFECTS
void HbToolBarPrivate::_q_delayedHide(HbEffect::EffectStatus status)
{
    Q_UNUSED(status);

    Q_Q(HbToolBar);

    // Apply forceHide only if the effect started successfully
    if (status.reason != Hb::EffectNotStarted) {
        delayedHide = false;
        q->hide();
        delayedHide = hasEffects;
    } else {
        delayedHide = false;
    }
    hidingInProgress = false;
}
#endif // HB_EFFECTS

void HbToolBarPrivate::startAppearEffect()
{
    mSuppressNextAppearEffect = false;
    // The first appear effect is suppressed regardless of requested or not.
    // There are too many cases to support during application start-up
    // (where the appear effect is very wrong) so disable it everywhere
    // for the first time.
    if (!mAppearedOnce) {
        mAppearedOnce = true;
        return;
    }
#ifdef HB_EFFECTS
    Q_Q(HbToolBar);
    QGraphicsItem *parentItem = q->parentItem();
    if (parentItem && mOrientation == Qt::Horizontal) {
        QRectF extRect(0.0,
                       parentItem->boundingRect().height(),
                       q->boundingRect().width(),
                       q->boundingRect().height());
        HbEffect::start(q, HB_TOOLBAR_ITEM_TYPE, "appear", 0, 0, QVariant(), extRect);
    } else if (parentItem && mOrientation == Qt::Vertical &&
               q->layoutDirection() == Qt::LeftToRight) {
        QRectF extRect(parentItem->boundingRect().width(),
                       parentItem->boundingRect().height() - q->boundingRect().height(),
                       q->boundingRect().width(),
                       q->boundingRect().height());
        HbEffect::start(q, HB_TOOLBAR_ITEM_TYPE, "appear_right", 0, 0, QVariant(), extRect);
    } else {
        QRectF extRect(-q->boundingRect().width(),
                       q->scenePos().y(),
                       q->boundingRect().width(),
                       q->boundingRect().height());
        HbEffect::start(q, HB_TOOLBAR_ITEM_TYPE, "appear_left", 0, 0, QVariant(), extRect);
    }
#endif//HB_EFFECTS
}


void HbToolBarPrivate::startDisappearEffect()
{
#ifdef HB_EFFECTS
    Q_Q(HbToolBar);
    QGraphicsItem *parentItem = q->parentItem();
    if (parentItem && q->orientation() == Qt::Horizontal) {
        QRectF extRect(0.0,
                       parentItem->boundingRect().height(),
                       q->boundingRect().width(),
                       q->boundingRect().height());
        if( !HbEffect::start(q, HB_TOOLBAR_ITEM_TYPE, "disappear", q, "_q_delayedHide",
                             QVariant(), extRect)) {
            delayedHide = false;
        }
    } else if (parentItem && q->orientation() == Qt::Vertical && 
               q->layoutDirection() == Qt::LeftToRight) {
        QRectF extRect(parentItem->boundingRect().width(),
                       parentItem->boundingRect().height() - q->boundingRect().height(),
                       q->boundingRect().width(),
                       q->boundingRect().height());
        if( !HbEffect::start(q, HB_TOOLBAR_ITEM_TYPE, "disappear_right", q, "_q_delayedHide",
                             QVariant(), extRect)) {
            delayedHide = false;
        }
    } else if (parentItem) {
        QRectF extRect(-q->boundingRect().width(),
                       parentItem->boundingRect().height() - q->boundingRect().height(),
                       q->boundingRect().width(),
                       q->boundingRect().height());
        if( !HbEffect::start(q, HB_TOOLBAR_ITEM_TYPE, "disappear_left", q, "_q_delayedHide",
                             QVariant(), extRect)) {
            delayedHide = false;
        }
    }
#endif//HB_EFFECTS
}

void HbToolBarPrivate::suppressNextAppearEffect(bool suppress)
{
    mSuppressNextAppearEffect = suppress;
}
