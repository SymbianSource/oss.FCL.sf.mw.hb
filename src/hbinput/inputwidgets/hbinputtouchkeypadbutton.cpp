/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include <hbframedrawer.h>
#include <hbframedrawerpool_p.h>
#include <hbframeitem.h>
#include <QGraphicsSceneResizeEvent>
#include <hbtextitem.h>
#include <hbiconitem.h>
#include <hbevent.h>
#ifdef HB_EFFECTS
#include <hbeffect.h>
#endif

#include "hbinputtouchkeypadbutton.h"
#include "hbinputvkbwidget.h"
#include "hbinputvkbwidget_p.h"

/*!
@proto
@hbinput
\class HbTouchKeypadButton
\brief A button widget to be used in touch keypads.

Expands HbPushButton functionality to suit touch keypad purposes. It handles virtual keyboard closing gesture
that is initiated from within the button area and knows how to act as a sticky input button. Sticky buttons propagate
mouse press state to neighboring button when a drag event crosses widget boundary. This is needed for example in virtual qwerty where
user must be able to slide finger across the keyboard.  
*/

/// @cond

const QString HbNormalBackground("qtg_fr_input_btn_keypad_normal");
const QString HbNormalPressedBackground("qtg_fr_input_btn_keypad_pressed");
const QString HbNormalInActiveBackground("qtg_fr_input_btn_keypad_disabled");
const QString HbNormalLatchedBackground("qtg_fr_input_btn_keypad_latched");

const QString HbFunctionBackground("qtg_fr_input_btn_function_normal");
const QString HbFunctionPressedBackground("qtg_fr_input_btn_function_pressed");
const QString HbFuncInActiveBackground("qtg_fr_input_btn_function_disabled");
const QString HbFunctionLatchedBackground("qtg_fr_input_btn_function_latched");

inline HbTouchKeypadButton* hbtouchkeypadbutton_cast(QGraphicsItem *item)
{
    if( item->isWidget() && qobject_cast<HbTouchKeypadButton *>(static_cast<QGraphicsWidget*>(item)) ) {
        return static_cast<HbTouchKeypadButton *>(item);
    }
    return 0;
}

class HbTouchKeypadButtonPrivate
{
public:
    HbTouchKeypadButtonPrivate(HbInputVkbWidget* owner)
        : mOwner(owner),
          mFaded(false),
          mButtonType(HbTouchKeypadButton::HbTouchButtonNormal),
          mFrameIcon(0),
          mStickyKey(false),
          mLatch(false)
    {}

public:
    HbInputVkbWidget* mOwner;
    bool mFaded;
    HbTouchKeypadButton::HbTouchButtonType mButtonType;
    HbFrameItem *mFrameIcon;
    bool mStickyKey;
    bool mLatch;
    int mKeyCode;
};

/// @endcond

/*!
Constructs the object. aOwner is the owning touch keypad widget.
*/
HbTouchKeypadButton::HbTouchKeypadButton(HbInputVkbWidget *owner,
                                         const QString &text,
                                         QGraphicsWidget *parent)
                                         : HbPushButton(text, parent), d_ptr(new HbTouchKeypadButtonPrivate(owner))
{
    #ifdef HB_EFFECTS
        HbEffect::disable(this);
    #endif

    this->setToolTip(QString());
    setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
    setProperty("buttonType", "normal");
}

/*!
Constructs the object. aOwner is the owning touch keypad widget.
*/
HbTouchKeypadButton::HbTouchKeypadButton(HbInputVkbWidget *owner,
                                         const HbIcon &icon,
                                         const QString &text,
                                         QGraphicsItem *parent)
                                         : HbPushButton(icon, text, parent), d_ptr(new HbTouchKeypadButtonPrivate(owner))
{
    #ifdef HB_EFFECTS
        HbEffect::disable(this);
    #endif

    this->setToolTip(QString());
    setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
    setProperty("buttonType", "normal");
}

/*!
Destructs the object.
*/
HbTouchKeypadButton::~HbTouchKeypadButton()
{
    delete d_ptr;
}

/*!
Handles mouse press event. The event is first directed to owner keypad and then
handled normally.
*/
void HbTouchKeypadButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbTouchKeypadButton);
    //Since this is a virtual keypress, it is ambiguous. Update the probable
    //keys from the this particular key press. It is required very much for 
    //QWERTY keypad but for ITU-T it may not be. Since it does not add too much 
    //of performance overhead, let it be there.
    if(d->mOwner && d->mOwner->d_func()) { 
        d->mOwner->d_func()->updateMouseHitItem(this, mapToScene(event->pos()));
    }

    /* If the Normal button is InActive(i.e. Faded) and there is no text mapped to that button and for Inactive Function buttons
    we should not handle MousePressEvent.However, we need to redirect MousePressEvent to the VKB Widget 
    because we need to handle keypad closegesture i.e.   the keypad should close when flick veritically on Faded Buttons
    Note: We have to handle MousePressEvent for Normal Inactive Buttons which have text mapped to it, because we need to handle long key press */
    if(!(d->mButtonType == HbTouchButtonNormalInActive && text().isEmpty())) {
        HbPushButton::mousePressEvent(event);
        if(event->isAccepted()) {
            setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonPressed);
        }
    }
    if(event->isAccepted()) {
        d->mOwner->d_func()->redirectMousePressEvent(event);
    }
}

/*!
Handles mouse release event. The event is first directed to owner keypad
and then handled normally.
*/
void HbTouchKeypadButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbTouchKeypadButton);
	//Since this is a virtual keypress, it is ambiguous. Update the probable
    //keys from the this partucular key press. It is required very much for 
    //QWERTY keypad but for ITU-T it may not be. Since it does not add too much 
    //of performance overhead, let it be there.
    //This updation happens on both press and release of the key. This is 
    //because press and release events may not happen on the same button.
	// should be updated before the release signal is emitted here
    d->mOwner->d_func()->updateMouseHitItem(this, mapToScene(event->pos()));

    d->mOwner->d_func()->redirectMouseReleaseEvent(event);

    /* If the Normal button is InActive(i.e. Faded) and there is no text mapped to that button and for Inactive Function buttons
    we should not handle MousePressEvent.However, we need to redirect MousePressEvent to the VKB Widget 
    because we need to handle keypad closegesture i.e.   the keypad should close when flick veritically on Faded Buttons
    Note: We have to handle MousePressEvent for Normal Inactive Buttons which have text mapped to it, because we need to handle long key press */
    if(!(d->mButtonType == HbTouchButtonNormalInActive && text().isEmpty())) {
        if (d->mLatch) {
            setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonLatched);
        } else {
            setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
        }
        
        // redirectMouseReleaseEvent() is called first so that the mouserelease is handled first and checked for
        // any flick events, and based on that handle the necessary mouse release actions
        // else it would give rise to a situation that after a flick, the selected button where release happens
        // will additionally handle, and give rise to undesired results.
        HbPushButton::mouseReleaseEvent(event);
    }


    ungrabMouse();
}

/*!
Handles mouse move event. In case this is a sticky key movement will activate a mousePressEvent on another button
and the next button is set as the grabber item.
*/
void HbTouchKeypadButton::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbTouchKeypadButton);

    bool transfered = false;
    bool hasHitButton = hitButton(event->pos());
    // if it is a sticky key and the mouse pointer moved out of the current button's boundary
    // that means we need to activate and pass the grabber to the button which is having the mouse pointer
    if (d->mStickyKey && !hasHitButton) {
        // get the list of item's at current mouse position
        QList<QGraphicsItem *> list = scene()->items(event->scenePos());
        for (int i =0; i < list.count(); i++) {
            // let's check if we have HbTouchKeypadButton
            HbTouchKeypadButton *button  = hbtouchkeypadbutton_cast(list.at(i));
            if (button  && button->isEnabled() && (button->parent() == parent())) {
                // we found a button which contains the current mouse pointer position
                // now we will be making the found button as mouse grabber if it is a 
                // sticky button.
                if (button->isStickyButton()) {
                    // release old button
                    ungrabMouse();
                    event->setButton(Qt::LeftButton);
                    HbPushButton::mouseReleaseEvent(event);
                    if (d->mLatch) {
                        setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonLatched);
                    } else {
                        setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
                    }
                    updatePrimitives();
                    // now call the mousepressEvent function of the button under mouse
                    QGraphicsSceneMouseEvent pressEvent;
                    pressEvent.setButton(Qt::LeftButton);
                    button->mousePressEvent(&pressEvent);

                    // now to make button under cursor to get mouse events we have to manually make that button
                    // a mouse grabber item. after this button will start recieving the button movements.
                    button->grabMouse();
                    transfered = true;
                }
                break;
            }
        }

        if (!transfered) {
            emit enteredInNonStickyRegion();
        }
    }
    // handling for non-sticy buttons
    if(!(d->mStickyKey)) {
        if (!hasHitButton && isDown()) {
            // the mouse moved out of the button pressed
            emit d->mOwner->mouseMovedOutOfButton();
            HbAbstractButton::setDown(false);
            // if we move out of a button, the released attribute should be reflected
            setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
            return;
        }   
        else if (hasHitButton && !isDown()) {   
            // makes sure that press event is regenerated
            HbPushButton::mouseMoveEvent(event);
            // if we return back to this button, the attribute should be pressed
            setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonPressed);
        return;
        }

    }

    if (!transfered) {
        if (isDown()) {
            // If mouse pointer moved away from the button, button needs to be updated
            if (!hasHitButton && d->mStickyKey) {
                if (d->mLatch) {
                    setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonLatched);
                } else {
                    setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
                }
            }
            if ( d->mStickyKey ) {
                HbPushButton::mouseMoveEvent(event);
                // Button is no longer pressed down so the button should be released
                if (!isDown() ) {
                        // setting button to down so that clicked and pressed signals are not emitted 
                    // by the button.
                    setDown(true);
                    event->setButton(Qt::LeftButton);
                    // we should not redirect this event to vkb at this point.
                    // as this was a mouse move event and we dont want vkb to be closed
                    // during the mouse move event.
                    HbPushButton::mouseReleaseEvent(event);
                }
            }
        } else if (d->mStickyKey) {
            // this condition satisfies when we are coming back to 
            // sticky button after moved out of its geometry. 
            // In this case button_down == false
            if (hasHitButton) {
                if (d->mLatch) {
                    setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonLatched);
                } else {
                    setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonPressed);
                }
            }
            // now passing to base class will emit a pressed signal().
            HbPushButton::mouseMoveEvent(event);
        }
    }
}

/*!
\reimp
*/
void HbTouchKeypadButton::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_D(HbTouchKeypadButton);

    HbPushButton::resizeEvent(event);

    // setting the draw rect for the frameitem in this button
    // get the new size, and use the new size to the frameitem
    if (d->mFrameIcon ) {
        QSizeF mySize = event->newSize();
        QRectF rect = QRectF(mySize.width()*0.1, mySize.height()*0.3, mySize.width()*0.8, mySize.height());
        d->mFrameIcon->setGeometry( rect );
    }
}

/*!
Returns true if button is Faded
*/
bool HbTouchKeypadButton::isFaded()
{
    Q_D(HbTouchKeypadButton);
    return d->mFaded;
}

/*!
Sets the button fade status. Fading does not mean disabling the button.
It just fades the button and does not change the Enabling properties of the button.
*/
void HbTouchKeypadButton::setFade(bool fade)
{
    Q_D(HbTouchKeypadButton);
    if (d->mFaded == fade) {
        return;
    }

    d->mFaded =  fade;

    // now set button's text, type and background attributes based on d->mFaded value
    if(d->mFaded) {
        if (d->mFrameIcon) {
            d->mFrameIcon->setOpacity(0.2);
        }
        if(HbTouchButtonNormal == getButtonType() ){
            setButtonType(HbTouchKeypadButton::HbTouchButtonNormalInActive);
        } else if(HbTouchButtonFunction == getButtonType()) {
            setButtonType(HbTouchKeypadButton::HbTouchButtonFnInActive);
        }
    } else { 
        if (d->mFrameIcon) {
            d->mFrameIcon->setOpacity(1.0);
        }
        if(HbTouchButtonNormalInActive == getButtonType()){
            setButtonType(HbTouchKeypadButton::HbTouchButtonNormal);
        } else if(HbTouchButtonFnInActive == getButtonType()) {
            setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
        }
    }
    setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
}

/*!
Sets button type.

\sa type
*/
void HbTouchKeypadButton::setButtonType(HbTouchButtonType buttonType)
{
    Q_D(HbTouchKeypadButton);
    d->mButtonType = buttonType;
    if (buttonType == HbTouchButtonNormal ||
        buttonType == HbTouchButtonNormalInActive) {
        setProperty("buttonType", "normal");
    } else if (buttonType == HbTouchButtonFunction ||
               buttonType == HbTouchButtonFnInActive){
        setProperty("buttonType", "function");
    }
}

/*!
Gets button type.
This function will be removed once the long press on qwerty buttons give accented char
functionality is removed
\sa type
*/
int HbTouchKeypadButton::getButtonType()
{
    Q_D(HbTouchKeypadButton);
    return d->mButtonType;
}

/*!
This function will be removed once the long press on qwerty buttons give accented char
functionality is removed
*/
HbFrameItem * HbTouchKeypadButton::getFrameIcon()
{
    Q_D(HbTouchKeypadButton);
    return d->mFrameIcon;
}

/*!
Sets button's background attributes.

\sa HbTouchButtonState
*/
void HbTouchKeypadButton::setBackgroundAttributes(HbTouchButtonState buttonState)
{
    Q_D(HbTouchKeypadButton);

    if(d->mButtonType == HbTouchButtonNormal) {
        if(buttonState == HbTouchKeypadButton::HbTouchButtonPressed) {
            setBackground(HbNormalPressedBackground);
        } else if (buttonState == HbTouchKeypadButton::HbTouchButtonLatched) {
            setBackground(HbNormalLatchedBackground);
        } else {
            setBackground(HbNormalBackground);
        }
    } else if(d->mButtonType == HbTouchButtonFunction) {
        if(buttonState == HbTouchKeypadButton::HbTouchButtonPressed) {
            setBackground(HbFunctionPressedBackground);
        } else if (buttonState == HbTouchKeypadButton::HbTouchButtonLatched) {
            setBackground(HbFunctionLatchedBackground);
        } else{
            setBackground(HbFunctionBackground);
        }
    } else if(d->mButtonType == HbTouchButtonFnInActive){
        setBackground(HbFuncInActiveBackground);
    } else if(d->mButtonType == HbTouchButtonNormalInActive) {
        setBackground(HbNormalInActiveBackground);
    } else {
        setBackground(HbFuncInActiveBackground);
    }
}

/*!
Sets button's background graphics. Parameter string is a resource name.
*/
void HbTouchKeypadButton::setBackground(const QString& backgroundFrameFilename)
{
    HbFrameDrawer* drawer = frameBackground();
    if (!drawer || drawer->frameGraphicsName() != backgroundFrameFilename) {
        setFrameBackground(HbFrameDrawerPool::get(backgroundFrameFilename, HbFrameDrawer::NinePieces, size()));
        update();
    }
}

/*
Currently HbPushButton is not capable of being skinned with multiple images (using HbFrameDraw)
hence maintaining a separate frame item per button, that will set the multi piece image and use it
*/
void HbTouchKeypadButton::setFrameIcon(const QString& frameIconFileName )
{
    Q_D(HbTouchKeypadButton);

    if (!d->mFrameIcon ) {
        d->mFrameIcon = new HbFrameItem(this);
        HbFrameDrawer *framedrawer = new HbFrameDrawer(frameIconFileName, HbFrameDrawer::ThreePiecesHorizontal);
        d->mFrameIcon->setFrameDrawer(framedrawer);
    } else {
        d->mFrameIcon->frameDrawer().setFrameGraphicsName(frameIconFileName);
    }
}

/*!
\reimp
*/
int HbTouchKeypadButton::type() const
{
    Q_D(const HbTouchKeypadButton);

    if (d->mButtonType == HbTouchButtonFunction ||
        d->mButtonType == HbTouchButtonFnInActive) {
        return Hb::ItemType_InputFunctionButton;
    } else if (d->mButtonType == HbTouchButtonNormal ||
        d->mButtonType == HbTouchButtonNormalInActive) {
        return Hb::ItemType_InputCharacterButton;
    } else {
        return Hb::ItemType_InputCharacterButton;
    }
}

/*!
Sets button's stickyness state.

\sa isStickyButton
*/
void HbTouchKeypadButton::setAsStickyButton(bool isSticky)
{
    Q_D(HbTouchKeypadButton);
    d->mStickyKey = isSticky;
}

/*!
Returns true if button is sticky button. Sticky buttons propagates press state to neighboring button
as a result of a drag event that crosses button border. Original button get release event and the neighbour
target button receives press event. This feature is needed for implementing slides where finger moves
across several buttons. 

\sa setAsStickyButton
*/
bool HbTouchKeypadButton::isStickyButton() const
{
    Q_D(const HbTouchKeypadButton);
    return d->mStickyKey;
}

/*!
Sets button's latched state.
*/
void HbTouchKeypadButton::setLatch(bool enable)
{
    Q_D(HbTouchKeypadButton);

    d->mLatch = enable;
    if (d->mLatch) {
        setProperty("state", "latched");
        setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonLatched);
    } else {
        setProperty("state", "normal");
        setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
    }
}

/*!
Returns true if button is in latched state.
*/
bool HbTouchKeypadButton::isLatched() const
{
    Q_D(const HbTouchKeypadButton);
    return d->mLatch;
}

/*!
Returns the keycode that the button is mapped to.
*/
int HbTouchKeypadButton::keyCode() const
{
    Q_D(const HbTouchKeypadButton);
    return d->mKeyCode;
}

/*!
Sets the keycode that the button is mapped to.
*/
void HbTouchKeypadButton::setKeyCode(int code)
{
    Q_D(HbTouchKeypadButton);
    d->mKeyCode = code;
}

/*!
\reimp
*/
void HbTouchKeypadButton::setText(const QString &text)
{
    // Workaround for pushbutton feature
    if (!text.isNull()) {
        HbPushButton::setText(text);
    } else {
        HbPushButton::setText(QString(""));
    }
}

/*!
\reimp
*/
void HbTouchKeypadButton::setAdditionalText(const QString &additionalText)
{
    if (!additionalText.isNull()) {
        HbPushButton::setAdditionalText(additionalText);
    } else {
        HbPushButton::setAdditionalText(QString(""));
    }
}

/*!
\reimp
*/
void HbTouchKeypadButton::changeEvent( QEvent *event )
{
    if ( event->type() == HbEvent::ThemeChanged ) {
        updatePrimitives();
        setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
    }
    HbPushButton::changeEvent(event);
}

/*!
\reimp
*/
void HbTouchKeypadButton::updatePrimitives()
{
    Q_D(HbTouchKeypadButton);
    HbPushButton::updatePrimitives();

    if (d->mFrameIcon && d->mFaded) {
        d->mFrameIcon->setOpacity(0.2);
    }
}

/*!
\reimp
*/
QSizeF HbTouchKeypadButton::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    QSizeF sh;
    switch (which) {
        case Qt::MinimumSize:
            sh = QSizeF(50, 50);
            break;
        case Qt::PreferredSize:
            sh = HbAbstractButton::sizeHint(which, constraint);
            break;
        case Qt::MaximumSize:
            sh = QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            break;
        default:
            sh = HbAbstractButton::sizeHint(which, constraint);
            break;
    }
    return sh;
}

/*!
Notification for state change.
*/
QVariant HbTouchKeypadButton::itemChange( GraphicsItemChange change, const QVariant & value )
{
	// If the button is being hidden and it has the press background, 
	// need to set it to released background. This fix is needed for the error:
	// In ITU-T long press * key and then return back to alpha mode, the * key 
	// has button pressed background.
	if (QGraphicsItem::ItemVisibleHasChanged == change && !value.toBool()) { 
		if (isDown()) {
			setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
		}
	}
	return HbPushButton::itemChange(change, value);
}

/*!
Sets initial keypad button size to make vkb layouting faster. Calls setPreferredSize internally.
*/
void HbTouchKeypadButton::setInitialSize(const QSizeF& initialSize)
{
    setPreferredSize(initialSize);
    QGraphicsItem* backgroundPrimitive = primitive(HbStyle::P_PushButton_background);
    if (backgroundPrimitive) {
        HbIconItem *iconItem = static_cast<HbIconItem*>(backgroundPrimitive);
        iconItem->setSize(initialSize);
    }
}
// End of file
