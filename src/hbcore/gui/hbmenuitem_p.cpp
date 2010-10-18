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

#include "hbmenuitem_p.h"
#include "hbmenuitem_p_p.h"
#include "hbmenu.h"
#include "hbmenu_p.h"
#include "hbaction.h"
#include "hbstyle.h"
#include "hbiconitem.h"
#include "hbevent.h"
#include "hbcolorscheme.h"
#include "hbwidgetfeedback.h"
#include "hbstyleiconprimitivedata.h"
#include "hbstyleframeprimitivedata.h"
#include "hbstyletextprimitivedata.h"
#include "hbtapgesture.h"
#include "hbnamespace_p.h"

#include <QGraphicsScene>
#ifdef HB_GESTURE_FW
#include <QGesture>
#endif

Q_DECLARE_METATYPE (QAction*)

namespace {
    const char* FRAME_ITEM_NAME = "frame";
    const char* TEXT_ITEM_NAME = "text";
    const char* SUBMENU_INDICATOR_ITEM_NAME = "submenu-indicator";
    const char* CHECK_INDICATOR_ITEM_NAME = "check-indicator";
    const char* SEPARATOR_ITEM_NAME = "separator";
};

/* macros */

// intended to be called from within the updatePrimitive slot.
#define UPDATE_PRIMITIVE( item, styleType ) \
    if (item) { \
        styleType _data; \
        initPrimitiveData(&_data, item); \
        style()->updatePrimitive(item, &_data, this); \
    } \
    
// intended to be called from within the private _q_updateItem slot.
#define PRIVATE_UPDATE_ITEM( check, styleType, item) \
    if (check) { \
        styleType _data; \
        q->initPrimitiveData(&_data, item); \
        q->style()->updatePrimitive(item, &_data, q); \
    } \

/*!
    \class HbMenuItem
    \brief HbMenuItem is a menu item graphics widget.

    HbMenuItem represents a single action in HbMenu.

    Usually touch and non-touch devices have mostly the same actions but a
    slightly different menu structure. High-end touch devices might prefer
    relatively large one level menus with direct access to all available
    actions, whereas low-end non-touch devices might fall back to using
    small but deep menus, offering the same actions but at different level.

    \sa HbAction HbMenu
*/

/*!
    \property HbMenuItem::menuType
    \brief
*/

/*!
    \primitives
    \primitive{frame} HbFrameItem representing the background frame of the menu item.
    \primitive{focus-indicator} HbFrameItem representing the background frame of the menu item when the item is focused.
    \primitive{text} HbTextItem representing the menu item text.
    \primitive{submenu-indicator} HbIconItem representing the icon that indicates that the menu item opens a sub-menu.
    \primitive{check-indicator} HbIconItem representing the check icon of the menu item.
    \primitive{separator} HbIconItem representing a menu separator.
  */

        HbMenuItemPrivate::HbMenuItemPrivate() :
        HbWidgetPrivate(),
        action(0),
        menu(0),
        mTextItem(0),
        mArrowItem(0),
        mCheckItem(0),
        mSeparatorItem(0),
        mFrame(0),
        mIndicateFocus(false),
        mRecycled(false),
        mCheckChanged(false),
        mTextChanged(false),
        mOldCheckState(false)
{
}

HbMenuItemPrivate::~HbMenuItemPrivate()
{
}

/*
    Just Creates the primitives.
*/
void HbMenuItemPrivate::createPrimitives()
{
    Q_Q(HbMenuItem);

    HbAction *hbAction = qobject_cast<HbAction*>(action);

    if (!mFrame && action && !action->isSeparator()) {
        mFrame = q->style()->createPrimitive(HbStyle::PT_FrameItem, FRAME_ITEM_NAME, q);
        mFrame->setAcceptedMouseButtons(Qt::NoButton);
        mFrame->setZValue(-4.0);
    }

    if (action) {
        // construct the textItem if needed.
        if (!mTextItem) {
            mTextItem = q->style()->createPrimitive(HbStyle::PT_TextItem, TEXT_ITEM_NAME, q);
            mTextItem->setAcceptedMouseButtons(Qt::NoButton);
        }
    } else if (mTextItem) {
        delete mTextItem;
        mTextItem = 0;
    }

    if (hbAction && hbAction->menu() && !action->isSeparator()) {
        if (!mArrowItem) {
            mArrowItem = q->style()->createPrimitive(HbStyle::PT_IconItem, SUBMENU_INDICATOR_ITEM_NAME, q);
            mArrowItem->setAcceptedMouseButtons(Qt::NoButton);
        }
    } else if (mArrowItem){
        delete mArrowItem;
        mArrowItem = 0;
    }

    if (action && action->isCheckable() && !action->isSeparator()) {
        if (!mCheckItem) {
            mCheckItem = q->style()->createPrimitive(HbStyle::PT_IconItem, CHECK_INDICATOR_ITEM_NAME, q);
            mCheckItem->setAcceptedMouseButtons(Qt::NoButton);
        }
    } else if (mCheckItem) {
        delete mCheckItem;
        mCheckItem = 0;
    }

    if (action && action->isSeparator()) {
        if (!mSeparatorItem) {
            mSeparatorItem = q->style()->createPrimitive(HbStyle::PT_IconItem, SEPARATOR_ITEM_NAME, q);
            mSeparatorItem->setAcceptedMouseButtons(Qt::NoButton);
            HbIconItem *separator = qgraphicsitem_cast<HbIconItem*>(mSeparatorItem);
            if (separator) {
                separator->setAspectRatioMode(Qt::IgnoreAspectRatio);
                separator->setAlignment(Qt::AlignCenter);
                separator->setIconName(QLatin1String("qtg_graf_popup_separator"));
            }
        }
    } else if (mSeparatorItem) {
        delete mSeparatorItem;
        mSeparatorItem = 0;
    }
}

/* 
    Deletes any existing primitives, then calls createPrimitives.
*/
void HbMenuItemPrivate::recreatePrimitives()
{
    delete mFrame; 
    mFrame = 0;

    delete mTextItem; 
    mTextItem = 0;

    delete mArrowItem;
    mArrowItem = 0;

    delete mCheckItem;
    mCheckItem = 0;

    delete mSeparatorItem;
    mSeparatorItem = 0;

    createPrimitives();
}

void HbMenuItemPrivate::_q_updateItem(bool forcedUpdate)
{
    Q_Q(HbMenuItem);

    if (action) {
        if (forcedUpdate) {
            q->repolish();
        }

        if (forcedUpdate || evaluateRecreateNecessary()) {
            q->recreatePrimitives();
            q->updatePrimitives();
        } else {
            PRIVATE_UPDATE_ITEM( mTextChanged, HbStyleTextPrimitiveData, mTextItem)
            PRIVATE_UPDATE_ITEM( mCheckChanged, HbStyleIconPrimitiveData, mCheckItem)
        }

        if (action->isEnabled() != q->isEnabled() || forcedUpdate) {
            q->setEnabled(action->isEnabled());
        }
    }
}


/*
    Determines if the action corresponding to the item
    has changed enough that we need to recreate and update
    all the primitives.

    This function also sets flags for if the check-state
    text, and icon have changed.
*/
bool HbMenuItemPrivate::evaluateRecreateNecessary()
{
    bool retVal = false;

    if (action) {
        if (!action->text().isNull() && !mTextItem) {
            retVal = true;
        } else if (action->text() != mOldText) {
            mTextChanged = true;
        }

        if (!retVal && (action->isCheckable() && !mCheckItem)) {
            retVal = true;
        } else if (action->isChecked() != mOldCheckState) {
            mCheckChanged = true;
        }

        if (!retVal && action->isSeparator() && !mSeparatorItem) {
            retVal = true;
        }
    }

    return retVal;
}

/*!
    Constructs a new HbMenuItem with \a action and \a parent. Ownership of the
    \a action remains on it's parent.
*/
HbMenuItem::HbMenuItem(HbMenu *menu, QGraphicsItem *parent)
    : HbWidget(*new HbMenuItemPrivate(), parent)
{

    Q_D(HbMenuItem);

    d->q_ptr = this;
    d->menu = menu;
    grabGesture(Qt::TapGesture);
    setAcceptedMouseButtons (Qt::NoButton);
}

/*!
    Destructs the menu item.
*/
HbMenuItem::~HbMenuItem()
{
}

/*!
    Returns the action representing this menu item.
*/
QAction* HbMenuItem::action() const
{
    Q_D(const HbMenuItem);
    return d->action;
}

/*!
    Returns the menu which handles this item.
*/
HbMenu* HbMenuItem::menu() const
{
    Q_D(const HbMenuItem);
    return d->menu;
}

/*!
    \reimp
    
    Called when the widget needs to initialize its primitives' data.

    Reimplemented from HbWidgetBase.  
*/
void HbMenuItem::initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive)
{
    Q_D( HbMenuItem );

    HbWidget::initPrimitiveData(primitiveData, primitive);

    QString itemName = HbStyle::itemName(primitive);

    if (itemName == SUBMENU_INDICATOR_ITEM_NAME) {
        // submenu indicator
        HbStyleIconPrimitiveData *data =
            hbstyleprimitivedata_cast<HbStyleIconPrimitiveData*>(primitiveData);
        if (data) {
            data->iconName = QLatin1String("qtg_mono_options_menu");
        }

    } else if (itemName == CHECK_INDICATOR_ITEM_NAME && d->action) {
        // check indicator
        HbStyleIconPrimitiveData *data = 
            hbstyleprimitivedata_cast<HbStyleIconPrimitiveData*>(primitiveData);
        if (data) {
            d->mOldCheckState = d->action->isChecked();
            if (d->mOldCheckState) {
                data->iconName = QLatin1String("qtg_small_tick");
            } else {
                data->iconName = QLatin1String("");
            }
        }

    } else if (itemName == TEXT_ITEM_NAME && d->action) {
        // text
        HbStyleTextPrimitiveData *data =
            hbstyleprimitivedata_cast<HbStyleTextPrimitiveData*>(primitiveData);
        if (data) {
            d->mOldText = d->action->text();
            data->text = d->mOldText;
        }

    } else if (itemName == FRAME_ITEM_NAME) {
        // frame
        HbStyleFramePrimitiveData *data = 
            hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData);
        if (data) {
            data->frameType = HbFrameDrawer::NinePieces;
            if (d->mIndicateFocus) {
                data->frameGraphicsName = QLatin1String("qtg_fr_popup_list_pressed");
            } else {
                data->frameGraphicsName = QLatin1String("qtg_fr_popup_list_normal");
            }
        }
    } 
}

/*!
    \reimp

    updates primitives.

    Reimplemented from HbWidgetBase.
*/
void HbMenuItem::updatePrimitives()
{
    Q_D( HbMenuItem );
    HbWidget::updatePrimitives();

    UPDATE_PRIMITIVE( d->mTextItem, HbStyleTextPrimitiveData )
    UPDATE_PRIMITIVE( d->mArrowItem, HbStyleIconPrimitiveData )
    UPDATE_PRIMITIVE( d->mCheckItem, HbStyleIconPrimitiveData )
    UPDATE_PRIMITIVE( d->mSeparatorItem, HbStyleIconPrimitiveData )
    UPDATE_PRIMITIVE( d->mFrame, HbStyleFramePrimitiveData )
}


/*!
    \reimp
 */
void HbMenuItem::changeEvent(QEvent *event)
{
 if (event->type() == QEvent::FontChange ||
     event->type() == QEvent::PaletteChange ||
     event->type() == QEvent::ParentChange ||
     event->type() == QEvent::StyleChange)
     return;
    HbWidget::changeEvent(event);
    if (event->type() == QEvent::LayoutDirectionChange) {
        Q_D(HbMenuItem);
        if (d->polished)
            repolish();
    }
    else if (event->type() == HbEvent::ThemeChanged) {
        Q_D(HbMenuItem);
        d->_q_updateItem();
    }
}
#ifdef HB_GESTURE_FW
void HbMenuItem::gestureEvent(QGestureEvent *event)
{
    if(HbTapGesture *gesture = qobject_cast<HbTapGesture *>(event->gesture(Qt::TapGesture))) {
        if (gesture->state() == Qt::GestureStarted) {
            if (scene()) {
                scene()->setProperty(HbPrivate::OverridingGesture.latin1(),Qt::TapGesture);
            }
            gesture->setProperty(HbPrivate::ThresholdRect.latin1(), mapRectToScene(boundingRect()).toRect());
            HbWidgetFeedback::triggered(this, Hb::InstantPressed);
            pressStateChanged(true);
            event->accept();
        } else if (gesture->state() == Qt::GestureFinished) {
            if (scene()) {
                scene()->setProperty(HbPrivate::OverridingGesture.latin1(),QVariant());
            }
            HbWidgetFeedback::triggered(this, Hb::InstantReleased);
            pressStateChanged(false);
            event->accept();            
            HbMenuPrivate::d_ptr(menu())->_q_triggerAction(this);
        } else if (gesture->state() == Qt::GestureCanceled) {
            if (scene()) {
                scene()->setProperty(HbPrivate::OverridingGesture.latin1(),QVariant());
            }
            pressStateChanged(false);
        }
    }
}
#endif

/*!
    Sets the action,which is represented by the menu item.
*/
void HbMenuItem::setAction(QAction *action)
{
    Q_D(HbMenuItem);
    bool actionChanged = d->action != action;
    if (d->action && actionChanged){
        disconnect(d->action, SIGNAL(changed()), this, SLOT(_q_updateItem()));
    }

    if (actionChanged) {
        d->action = action;
        d->_q_updateItem();
        connect(d->action, SIGNAL(changed()), this, SLOT(_q_updateItem()));
    } else if (d->mRecycled){
        d->_q_updateItem(true);
        connect(d->action, SIGNAL(changed()), this, SLOT(_q_updateItem()));
        d->mRecycled = false;
    }
}

void HbMenuItem::recycleItem()
{
    Q_D(HbMenuItem);
    d->mRecycled = true;
    if (d->action){
        disconnect(d->action, SIGNAL(changed()), this, SLOT(_q_updateItem()));
    }
}

bool HbMenuItem::checkboxExists()
{
    Q_D(HbMenuItem);
    return d->action && d->action->isCheckable();
}

bool HbMenuItem::submenuExists()
{
    Q_D(HbMenuItem);
    HbAction *hbAction = qobject_cast<HbAction*>(d->action);
    return hbAction && hbAction->menu();

}

bool HbMenuItem::separatorExists()
{
    Q_D(HbMenuItem);
    return d->action && d->action->isSeparator();
}

/*!
    Returns the type of the menu where menu item belongs.
*/
HbMenu::MenuType HbMenuItem::menuType() const
{
    Q_D(const HbMenuItem);
    if (d->menu) {
        return menu()->menuType();
    } else {
        return HbMenu::ContextMenu;
    }
}

/*!
    \creates and updates the focusitem primitive
     when menu item has been pressed
*/
void HbMenuItem::pressStateChanged(bool value)
{
    Q_D(HbMenuItem);

    bool tempVal = this->isEnabled() ? value : false;

    if (tempVal != d->mIndicateFocus) {
        d->mIndicateFocus = tempVal;
        updatePrimitives();
    }
}

/*!
    \reimp
 */
QVariant HbMenuItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    Q_D(HbMenuItem);
    // Remove highlighting on a menuitem when the menu disappears.
    if (change == QGraphicsItem::ItemVisibleHasChanged && !value.toBool()) {
        pressStateChanged(false);
    } else if (change == QGraphicsItem::ItemVisibleChange) {
        if (value.toBool()) {
            if (d->polished && !testAttribute(Qt::WA_Resized)) {
                adjustSize();
                setAttribute(Qt::WA_Resized, false);
            }
            return QGraphicsItem::itemChange(change, value);
        }
    }

    return HbWidget::itemChange(change, value);
}

/*
    \reimp

    Called to recreate primitives
*/
void HbMenuItem::recreatePrimitives()
{
    Q_D( HbMenuItem );

    d->recreatePrimitives();

    HbWidget::recreatePrimitives();
}

#include "moc_hbmenuitem_p.cpp"
