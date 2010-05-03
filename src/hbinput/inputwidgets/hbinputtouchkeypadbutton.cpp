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
#include <hbtapgesture.h>

#include "hbinputtouchkeypadbutton.h"
#include "hbinputvkbwidget.h"
#include "hbinputvkbwidget_p.h"

/*!
@proto
@hbinput
\class HbTouchKeypadButton
\deprecated class HbTouchKeypadButton
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
\deprecated HbTouchKeypadButton::HbTouchKeypadButton(HbInputVkbWidget *, const QString &, QGraphicsWidget *)
    is deprecated.
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
\deprecated HbTouchKeypadButton::HbTouchKeypadButton(HbInputVkbWidget *, const HbIcon &, const QString &, QGraphicsItem *)
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
\deprecated HbTouchKeypadButton::~HbTouchKeypadButton()
*/
HbTouchKeypadButton::~HbTouchKeypadButton()
{
    delete d_ptr;
}

/*!
\reimp
\deprecated HbTouchKeypadButton::mousePressEvent(QGraphicsSceneMouseEvent *)
    is deprecated.
*/
void HbTouchKeypadButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
}

/*!
\reimp
\deprecated HbTouchKeypadButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
    is deprecated.
*/
void HbTouchKeypadButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
}

/*!
\reimp
\deprecated HbTouchKeypadButton::mouseMoveEvent(QGraphicsSceneMouseEvent *)
    is deprecated.
*/
void HbTouchKeypadButton::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
}

/*!
\reimp
\deprecated HbTouchKeypadButton::gestureEvent(QGestureEvent *)
    is deprecated.
*/
void HbTouchKeypadButton::gestureEvent(QGestureEvent *event)
{
    Q_D(HbTouchKeypadButton);
    if (HbTapGesture *tap = qobject_cast<HbTapGesture*>(event->gesture(Qt::TapGesture))) {
        switch(tap->state()) {
        case Qt::GestureStarted:
            if (d->mOwner && d->mOwner->d_func()) {
                d->mOwner->d_func()->updateMouseHitItem(this, tap->scenePosition());
            }
            if (!(d->mButtonType == HbTouchButtonNormalInActive && text().isEmpty())) {
                setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonPressed);
            }
            break;
        case Qt::GestureUpdated:
            // Handle tap-and-hold?
            break;
        case Qt::GestureFinished:
            if (!(d->mButtonType == HbTouchButtonNormalInActive && text().isEmpty())) {
                if (d->mLatch) {
                    setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonLatched);
                } else {
                    setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
                }
                break;
        case Qt::GestureCanceled:
                setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
                break;
        default:
                break;
            }
        }
    }
    HbPushButton::gestureEvent(event);
}

/*!
\reimp
\deprecated HbTouchKeypadButton::resizeEvent(QGraphicsSceneResizeEvent *)
    is deprecated.
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
\deprecated HbTouchKeypadButton::isFaded()
    is deprecated.
*/
bool HbTouchKeypadButton::isFaded()
{
    Q_D(HbTouchKeypadButton);
    return d->mFaded;
}

/*!
\deprecated HbTouchKeypadButton::setFade(bool)
    is deprecated.
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
\deprecated  HbTouchKeypadButton::setButtonType(HbTouchButtonType)
    is deprecated.
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
\deprecated HbTouchKeypadButton::getButtonType()
    is deprecated.
*/
int HbTouchKeypadButton::getButtonType()
{
    Q_D(HbTouchKeypadButton);
    return d->mButtonType;
}

/*!
\deprecated HbTouchKeypadButton::getFrameIcon()
    is deprecated.
*/
HbFrameItem * HbTouchKeypadButton::getFrameIcon()
{
    Q_D(HbTouchKeypadButton);
    return d->mFrameIcon;
}

/*!
\deprecated HbTouchKeypadButton::setBackgroundAttributes(HbTouchButtonState)
    is deprecated.
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
\deprecated HbTouchKeypadButton::setBackground(const QString&)
    is deprecated.
*/
void HbTouchKeypadButton::setBackground(const QString& backgroundFrameFilename)
{
    HbFrameDrawer* drawer = frameBackground();
    if (!drawer || drawer->frameGraphicsName() != backgroundFrameFilename) {
        setFrameBackground(HbFrameDrawerPool::get(backgroundFrameFilename, HbFrameDrawer::NinePieces, size()));
        update();
    }
}

/*!
\deprecated HbTouchKeypadButton::setFrameIcon(const QString&)
    is deprecated.
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
\deprecated HbTouchKeypadButton::type() const
    is deprecated.
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
\deprecated  HbTouchKeypadButton::setAsStickyButton(bool)
    is deprecated.
*/
void HbTouchKeypadButton::setAsStickyButton(bool isSticky)
{
    Q_D(HbTouchKeypadButton);
    d->mStickyKey = isSticky;
}

/*!
\deprecated HbTouchKeypadButton::isStickyButton() const
    is deprecated.
*/
bool HbTouchKeypadButton::isStickyButton() const
{
    Q_D(const HbTouchKeypadButton);
    return d->mStickyKey;
}

/*!
\deprecated HbTouchKeypadButton::setLatch(bool)
    is deprecated
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
\deprecated HbTouchKeypadButton::isLatched() const
    is deprecated.
*/
bool HbTouchKeypadButton::isLatched() const
{
    Q_D(const HbTouchKeypadButton);
    return d->mLatch;
}

/*!
\deprecated HbTouchKeypadButton::keyCode() const
    is deprecated.
*/
int HbTouchKeypadButton::keyCode() const
{
    Q_D(const HbTouchKeypadButton);
    return d->mKeyCode;
}

/*!
\deprecated HbTouchKeypadButton::setKeyCode(int)
    is deprecated.
*/
void HbTouchKeypadButton::setKeyCode(int code)
{
    Q_D(HbTouchKeypadButton);
    d->mKeyCode = code;
}

/*!
\reimp
\deprecated HbTouchKeypadButton::setText(const QString &)
    is deprecated.
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
\deprecated HbTouchKeypadButton::setAdditionalText(const QString &)
    is deprecated.
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
\deprecated HbTouchKeypadButton::changeEvent(QEvent *)
    is deprecated.
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
\deprecated HbTouchKeypadButton::updatePrimitives()
    is deprecated.
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
\deprecated HbTouchKeypadButton::sizeHint(Qt::SizeHint, const QSizeF &) const
    is deprecated.
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
\deprecated HbTouchKeypadButton::itemChange(GraphicsItemChange, const QVariant &)
    is deprecated.
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
\deprecated HbTouchKeypadButton::setInitialSize(const QSizeF&)
    is deprecated.
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
