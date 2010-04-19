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

#include "hbtoolbar.h"
#include "hbtoolbar_p.h"
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
#include <QDebug>
#include <QGesture>

#ifdef HB_EFFECTS
#include "hbeffectinternal_p.h"
#define HB_TOOLBAR_ITEM_TYPE "HB_TOOLBAR"
bool HbToolBarPrivate::effectsLoaded = false;
#endif

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
        polishButtons(false),
        minimumToolButtonSize(QSizeF()),
        maxToolBarButtons(-1),
        moreExtension(0),
        moreExtensionButton(0),
        mDialogToolBar(false),
        mDoLayout(true),
        mDoLayoutPending(true),
        mOrientationEffectsRunning(false),
        mSuppressNextAppearEffect(false)
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
    Q_Q(HbToolBar);
    q->grabGesture(Qt::PanGesture);
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
    mDoLayout = true;
    doLazyInit();
    doLayout();
}

void HbToolBarPrivate::setOrientation( Qt::Orientation orientation )
{
    if ( mOrientation != orientation ) {
        mOrientation = orientation;
        polishButtons = true;
        emit core.orientationChanged();
    }

}

void HbToolBarPrivate::calculateMaximumButtons() {
    Q_Q(HbToolBar);
    maxToolBarButtons = 2;
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
        prepareButtonForToolbar(button, index);
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

//This is called when toolbar recives resizeevent.
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
         (moreExtensionButton && !moreExtensionButton->action()->isVisible()))) {
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
         (moreExtensionButton && !moreExtensionButton->action()->isVisible()))) {
        return true;
    }
    if (totalButtons<=maxToolBarButtons  &&
        (moreExtensionButton && moreExtensionButton->action()->isVisible())) {
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
    prepareButtonForToolbar(button,false);
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
        QObject::connect(button->action(), SIGNAL(triggered(bool)),
                         moreExtension, SLOT(close()));
        button->setParentItem(moreExtension->contentWidget());
    }
}

//We prepare button to be added for toolbar
void HbToolBarPrivate::prepareButtonForToolbar(HbToolButton *button,bool handlePolish)
{
    Q_Q(HbToolBar);
    updateButtonStyle(button, true);
    if (!button->parentItem() || button->parentItem() != q){
        bool movingFromExtension = moreExtension && (button->parentItem() == moreExtension->contentWidget());
        setExtensionLayout(button, false);
        // tool buttons should eat all the space in the toolbar
        button->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
        if (movingFromExtension){
            QObject::disconnect(button->action(), SIGNAL(triggered(bool) ),
                                moreExtension, SLOT(close()));
            HbToolButtonPrivate::d_ptr(button)->setBackgroundVisible(true);
            polishButtons = true;
        }
        button->setParentItem(q);
    }
    if (polishButtons && handlePolish){
        QApplication::sendPostedEvents(button,QEvent::Polish);
        QApplication::sendPostedEvents(button,QEvent::LayoutRequest);
    }
}

//resets all visible buttons list.This is called when actionChange event is recieved
//by toolbar and from doLayout when mDoLayoutPending is true
void HbToolBarPrivate::resetVisibleButtonsList()
{
    mVisibleToolButtons.clear();
    foreach(HbToolButton *button, mToolButtons) {
        if (button->action()->isVisible()) {
            mVisibleToolButtons.append(button);
        } else {
            button->setVisible(false);
        }
    }
}

//creates a new toolbutton and calls actionAdded if update is true.
//update can be true in cases where toolbar is polished and actionAdded
//event is recieved by toolbar.
//we emit visibilitychanged signal here if this is the firstAction added in cases
//after toolbar is polished and visible.
void HbToolBarPrivate::createToolButton(QAction *Action, bool update)
{
    Q_Q(HbToolBar);
    if(!Action)
        return;
    HbAction* action = qobject_cast<HbAction*>(Action);

    if (action) {
        // Find out index where to insert button
        int index = q->actions().indexOf(Action);

        // Lets give the action manager possibility to calculate position for the action
        if (action->commandRole() != HbAction::NoRole) {
            // Create action manager when needed
            if (!actionManager) {
                actionManager = new HbActionManager(HbView::ToolBar, q, q->mainWindow());
            }
            if (actionManager){
                index = actionManager->position(action, q->actions());
            }
            
        }
        HbToolButton *button = new HbToolButton(action);
        HbToolButtonPrivate::d_ptr(button)->mDialogToolBar = mDialogToolBar;
        setButtonLayoutDirection(*button);
        mToolButtons.insert(index, button);
        if (action->isVisible()){
            int visibleIndex = 0;
            for (int i = 0; i < index; i++) {
                if (mToolButtons.at(i)->action()->isVisible())
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
    else {
        qWarning() << "Use HbAction instead of QAction!";
    }
}

//called from doLayout.We update the more button and extension as needed
void HbToolBarPrivate::updateExtension(bool moreButtonNeeded)
{
    Q_Q(HbToolBar);
    if (moreExtension) {
        HbToolBarExtensionPrivate::d_ptr(moreExtension)->mToolButtons.clear();
        HbToolBarExtensionPrivate::d_ptr(moreExtension)->contentWidget->setLayout(0);
        moreExtensionButton->setVisible(moreButtonNeeded);
        moreExtensionButton->action()->setVisible(moreButtonNeeded);
    }
    else if (moreButtonNeeded && !moreExtension)  {
        moreExtension = new HbToolBarExtension();
        moreExtensionButton = new HbToolButton(moreExtension->extensionAction(), q);
        moreExtensionButton->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));

        HbIcon extensionIcon("qtg_mono_more");
        extensionIcon.setColor(HbColorScheme::color("foreground"));
        moreExtension->extensionAction()->setIcon(extensionIcon);
        initToolBarExtension(moreExtension);
        HbToolButtonPrivate::d_ptr(moreExtensionButton)->setToolBarPosition(
                HbStyleOptionToolButton::TB_End);
        moreExtensionButton->setToolButtonStyle(HbToolButton::ToolButtonIcon);
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
    if (!minimumToolButtonSize.isValid()) {
        calculateButtonMinimumSize();
    }

    //Calculate maximum number of buttons in toolbar
    calculateMaximumButtons();
    int visibleItemsCount = mVisibleToolButtons.count();
    bool moreButtonNeeded = visibleItemsCount > maxToolBarButtons;
    if (moreButtonNeeded ||
        (moreExtensionButton && moreExtensionButton->action()->isVisible() != moreButtonNeeded)) {
        updateExtension(moreButtonNeeded);
    }


    mLayout = new QGraphicsLinearLayout( q->orientation() );
    mLayout->setSpacing(0);
    mLayout->setContentsMargins(0,0,0,0);

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
    QApplication::sendPostedEvents(q,QEvent::LayoutRequest);
    polishButtons = false;
    if (delayedStartEffects && hasEffects){
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
    if (!emitVisibilityChangeSignal && polished && event->action()->isVisible() && !mVisibleToolButtons.count())
        emitVisibilityChangeSignal = true;

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
        if (button->action() == event->action()) {         
            mToolButtons.removeAt(i);
            // Emit signal when the only action is removed
            if (mToolButtons.count() == 0) {
                if (moreExtensionButton)
                    moreExtensionButton->setVisible(false);
                //reset toolbar size
                q->resize(0, 0);
                if (q->isVisible())
                    QMetaObject::invokeMethod(&core, "visibilityChanged", Qt::QueuedConnection);
            }
            break;
        }
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
}

void HbToolBarPrivate::updateButtonStyle(HbToolButton *button,bool notInExtension)
{
    Q_Q(HbToolBar);
    if (!button)
        return;
    // Set style for all toolbuttons. If toolbar orientation is vertical then
    // style mustn't be HbToolButton::ToolButtonTextAndIcon unless the button is in
    // the toolbar extension.
    // button orientation is orthogonal to the toolbar orientation
    Qt::Orientation buttonOrientation = q->orientation() == Qt::Vertical ?
                                        Qt::Horizontal : Qt::Vertical;
    HbToolButtonPrivate::d_ptr(button)->setOrientation( buttonOrientation );
    if (!button->action()->icon().isNull()) {
        if ((notInExtension) &&
            q->orientation() == Qt::Vertical) {
            button->setToolButtonStyle(HbToolButton::ToolButtonIcon);
        } else if (!button->action()->text().isEmpty()) {
            button->setToolButtonStyle(HbToolButton::ToolButtonTextAndIcon);
        } else {
            button->setToolButtonStyle(HbToolButton::ToolButtonIcon);
        }
    } else {
        button->setToolButtonStyle(HbToolButton::ToolButtonText);
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
        first->setToolBarPosition(HbStyleOptionToolButton::TB_OnlyOne);
    } else if (maxToolbarButtons > 1 && mVisibleToolButtons.count() > 1) {
        HbToolButtonPrivate *first = HbToolButtonPrivate::d_ptr(mVisibleToolButtons.first());
        first->setToolBarPosition(HbStyleOptionToolButton::TB_Beginning);
        
        int lastMiddleButton = (moreButtonNeeded) ?
                               maxToolbarButtons - 2 : mVisibleToolButtons.count() - 2;
        if (!moreButtonNeeded) {
            HbToolButtonPrivate *last = HbToolButtonPrivate::d_ptr(mVisibleToolButtons.last());
            last->setToolBarPosition(HbStyleOptionToolButton::TB_End);        
        }
        for (int i = 1; i <= lastMiddleButton; ++i) {
            HbToolButtonPrivate *middle = HbToolButtonPrivate::d_ptr(mVisibleToolButtons.at(i));
            middle->setToolBarPosition(HbStyleOptionToolButton::TB_Middle);
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

    q->connect(&core, SIGNAL(orientationChanged()), extension, SLOT(_q_orientationChanged()));
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
    if (q->layoutDirection() !=  button.layoutDirection()) {
        button.setLayoutDirection(q->layoutDirection());
    }
}

void HbToolBarPrivate::updateButtonsLayoutDirection() {
    foreach (HbToolButton *button, mToolButtons) {
        setButtonLayoutDirection(*button);
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
    } else {
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
