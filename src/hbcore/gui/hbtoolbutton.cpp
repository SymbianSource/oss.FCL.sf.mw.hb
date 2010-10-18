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

#include "hbtoolbutton.h"
#include "hbtoolbutton_p.h"
#include "hbtooltip.h"
#include "hbtoolbarextension.h"
#include "hbtoolbarextension_p.h"
#include "hbaction.h"
#include "hbaction_p.h"
#include "hbcolorscheme.h"
#include "hbtextitem.h"
#include "hbiconitem.h"
#include "hbview.h"
#include "hbmainwindow.h"
#include "hbframeitem.h"
#include "hbevent.h"

#include "hbglobal_p.h" // remove when removing HB_DEPRECATED

#include "hbstyle_p.h" // for iconmodes...

#include <hbstyleframeprimitivedata.h>
#include <hbstyleiconprimitivedata.h>
#include <hbstyletextprimitivedata.h>

#include <QGraphicsSceneHelpEvent>
#include <QGraphicsSceneMouseEvent>
#include <QCoreApplication>

/*!
    @stable
    @hbcore
    \class HbToolButton
    \brief The HbToolButton class provides a quick-access button for actions.

    A tool button is a special button that provides quick-access to
    a specific action. Unlike a normal push button HbPushButton, a tool
    button represents an action. In other words, HbToolButton is
    synchronized with an instance of HbAction.

    The action may also be associated with other parts of the user interface,
    as menu items and keyboard shortcuts. Sharing actions in this way helps
    make the user interface more consistent and is often less work to implement.

    Tool buttons are normally created indirectly when actions are added to a
    toolbar with HbToolBar::addAction(). It is also possible to
    construct tool buttons directly in the same way as any other widget, and
    arrange them alongside other widgets in layouts.

    A tool button's background is set as HbIcon. This makes it possible to
    specify different images for the normal and pressed states.

    Example usage:
    \code
    HbAction *action = new HbAction(icon, name, this);
    HbToolButton *toolButton = new HbToolButton(action, this);
    layout->addItem(toolButton);
    \endcode

    \sa HbAction, HbPushButton
*/

/*!
    \fn void HbToolButton::triggered(HbAction *action)

    This signal is emitted when the \a action is triggered.
 */

/*!
    \reimp
    \fn int HbToolButton::type() const
 */

/*!
  \primitives
  \primitive{background} HbFrameItem representing the background frame of the button.
  \primitive{icon} HbIconItem representing the icon of the button.
  \primitive{text} HbTextItem representing button text.
  */

HbToolButtonPrivate::HbToolButtonPrivate() :
    action(0),
    textItem(0),
    iconItem(0),
    frameItem(0),
    customBackground(),
    backgroundVisible(true),
    toolBarPosition(TB_None),
    orientation(Qt::Vertical),
    mDialogToolBar(false),
    toolbarExtensionFrame(false),
    mButtonSize(QSizeF())
{    
}

HbToolButtonPrivate::~HbToolButtonPrivate()
{
}

void HbToolButtonPrivate::createPrimitives()
{
    Q_Q(HbToolButton);
    if (backgroundVisible) {
        if (!frameItem){
            frameItem = q->style()->createPrimitive(HbStyle::PT_FrameItem, "background", q);
        }
    } else if (frameItem) {
        delete frameItem;
        frameItem = 0;
    }
    if (!textItem) {
        textItem = q->style()->createPrimitive(HbStyle::PT_TextItem, "text", q);
        }
    HbStyleTextPrimitiveData textData;
    textData.textWrapping = Hb::TextWordWrap;
    q->style()->updatePrimitive(textItem, &textData, q);
        
    if (!iconItem) {
        iconItem = q->style()->createPrimitive(HbStyle::PT_IconItem, "icon", q);
    }
    
}

void HbToolButtonPrivate::setOrientation(Qt::Orientation orientation)
{
    if (this->orientation != orientation) {
        this->orientation = orientation;
        Q_Q(HbToolButton);
        q->setMinimumSize(QSizeF());
        q->repolish();
    }    
}

void HbToolButtonPrivate::setToolBarPosition(ToolButtonPosition position)
{
    Q_Q(HbToolButton);
    if (toolBarPosition != position) {
        toolBarPosition = position;
        // required for toolbar()->action()[i]->setVisible(visible)
        // to work for all cases
        if (q->isVisible() && polished) {
            q->updatePrimitives();
        }
    }    
}

void HbToolButtonPrivate::setExtensionBackgroundVisible(bool visible)
{
    Q_Q(HbToolButton);
    if (toolbarExtensionFrame != visible) {
        toolbarExtensionFrame = visible;
        // required to make extension orientation switch from
        // landscape to portrait work correctly with automatic more
        // extension.
        q->repolish();
    }
}

void HbToolButtonPrivate::setBackgroundVisible(bool visible)
{
    Q_Q(HbToolButton);
    if (backgroundVisible != visible) {
        backgroundVisible = visible;
        // required to make extension orientation switch from
        // landscape to portrait work correctly with automatic more
        // extension.
        q->repolish();
    }
}

void HbToolButtonPrivate::setLayoutProperty(const char *name, bool value)
{
    Q_Q(HbToolButton);
    q->setProperty(name, value);
        q->repolish();
}


bool HbToolButtonPrivate::useTransparentGraphics() const
{
    Q_Q(const HbToolButton);
    if (q->mainWindow() && q->mainWindow()->currentView()) {
        if (q->mainWindow()->currentView()->viewFlags() & HbView::ViewTitleBarTransparent) {
            return true;
        }
    }
    return false;
}

bool HbToolButtonPrivate::isToolBarExtension() const
{
    return (action != 0 && toolbarExtensionFrame);
}

void HbToolButtonPrivate::framePrimitiveData(HbStyleFramePrimitiveData *data)
{
    Q_Q(HbToolButton);

    data->fillWholeRect = true;

    if (orientation == Qt::Vertical) {
        data->frameType = HbFrameDrawer::ThreePiecesHorizontal;
    } else {
        data->frameType = HbFrameDrawer::ThreePiecesVertical;
    }

    QStringList list;
    QString frameGraphicsFooter;
    // data->state already set by abstractbutton's data init
    QIcon::Mode mode = HbStylePrivate::iconMode(data->state);
    QIcon::State state = HbStylePrivate::iconState(data->state);

    // custom background
    if (!q->background().isNull()) {
        data->frameGraphicsName = customBackground.iconName(mode, state);
        return;
    }

    // in toolbar extension
    if(isToolBarExtension()) {
        if (mode == QIcon::Normal && state == QIcon::On) {
            if(!q->isCheckable()){
                data->frameGraphicsName = QLatin1String("qtg_fr_popup_grid_pressed");
            } else {
                data->frameGraphicsName = QLatin1String("qtg_fr_tb_ext");
            }
        } else {
            data->frameGraphicsName = QLatin1String("");
        }
        data->frameType = HbFrameDrawer::NinePieces;
        return;
    }

    if (!toolBarPosition) {
        if (mode == QIcon::Disabled && state == QIcon::Off) {
            data->frameGraphicsName = QLatin1String("qtg_fr_btn_disabled");
        } else if (mode == QIcon::Normal && state == QIcon::On) {
            if(!q->isCheckable()){
                data->frameGraphicsName = QLatin1String("qtg_fr_btn_pressed");
            } else {
                data->frameGraphicsName = QLatin1String("qtg_fr_btn_latched");
            }
        } else if (mode == QIcon::Selected && state == QIcon::Off) {
            data->frameGraphicsName = QLatin1String("qtg_fr_btn_highlight");
        } else {
            data->frameGraphicsName = QLatin1String("qtg_fr_btn_normal");
        }
        data->frameType = HbFrameDrawer::NinePieces;
        return;
    }
    // For toolbar:

    QString frameGraphicsHeader;
    if (!mDialogToolBar){
        if (useTransparentGraphics()) {
            frameGraphicsHeader = orientation == Qt::Vertical ?
                                  QLatin1String("qtg_fr_tb_trans_h_"):
                                  QLatin1String("qtg_fr_tb_trans_v_");
        } else {
            frameGraphicsHeader = orientation == Qt::Vertical ?
                                  QLatin1String("qtg_fr_tb_h_"):
                                  QLatin1String("qtg_fr_tb_v_");
        }
    } else {
        frameGraphicsHeader = QLatin1String("qtg_fr_popup_sk_");
        data->mirroringMode = HbIcon::LayoutDirection;
    }

    switch (toolBarPosition) {
        case TB_OnlyOne:
            if (orientation == Qt::Vertical) {
                list << QLatin1String("_l") << QLatin1String("_c") << QLatin1String("_r");
            } else {
                list << QLatin1String("_t") << QLatin1String("_c") << QLatin1String("_b");
            }
            break;
        case TB_Beginning:
            if (orientation== Qt::Vertical) {
                list << QLatin1String("_l") << QLatin1String("_c") << QLatin1String("_cr");
            } else {
                list << QLatin1String("_t") << QLatin1String("_c") << QLatin1String("_cb");
            }
            break;
        case TB_Middle:
            if (orientation == Qt::Vertical) {
                list << QLatin1String("_cl") << QLatin1String("_c") << QLatin1String("_cr");
            } else {
                list << QLatin1String("_ct") << QLatin1String("_c") << QLatin1String("_cb");
            }
            break;
        case TB_End:
            if (orientation== Qt::Vertical) {
                list << QLatin1String("_cl") << QLatin1String("_c") << QLatin1String("_r");
            } else {
                list << QLatin1String("_ct") << QLatin1String("_c") << QLatin1String("_b");
            }
            break;

        default:
        case TB_None:
            break;
    } // switch case end

    data->fileNameSuffixList = list;
    if (mode == QIcon::Disabled && state == QIcon::Off) {
        frameGraphicsFooter = QLatin1String("disabled");
    } else if (mode == QIcon::Normal && state == QIcon::On) {
        if(!q->isCheckable()) {
            frameGraphicsFooter = QLatin1String("pressed");
        } else {
            frameGraphicsFooter = QLatin1String("latched");
        }
    } else if (mode == QIcon::Selected && state == QIcon::Off) {
        frameGraphicsFooter = QLatin1String("highlight");
    } else {
        frameGraphicsFooter = QLatin1String("normal");
    }
    data->frameGraphicsName = QString ("%0%1").arg(frameGraphicsHeader).arg(frameGraphicsFooter);
    data->mirroringMode = HbIcon::LayoutDirection;
}

void HbToolButtonPrivate::iconPrimitiveData(HbStyleIconPrimitiveData *data)
{
    Q_Q(HbToolButton);
    if (q->action()) {
        data->icon = q->action()->icon();
    } else if(action){
        data->icon = action->icon();
    } else {
        data->icon = HbIcon();
    }
    data->iconMode = HbStylePrivate::iconMode(data->state);
    data->iconState = HbStylePrivate::iconState(data->state);
    return;
}
void HbToolButtonPrivate::textPrimitiveData(HbStyleTextPrimitiveData *data)
{
    if(action) {
        data->text = action->text();
    } else {
        data->text = QString();
    }
}

QSizeF HbToolButtonPrivate::getMinimumSize()
{
    Q_Q(HbToolButton);
    mRepolishRequested = true;
    polishPending = false;
    //Workaround (causing extra polish)
    mSizeHintPolish = false;
    //workaround ends
    q->updateGeometry();
    QSizeF size = q->minimumSize();
    return size;
}

void HbToolButtonPrivate::_q_actionTriggered()
{
    Q_Q(HbToolButton);
    // Only emit triggered signal for HbActions, not QActions.
    // triggered(QAction*) requires an API change we don't want to do right now.
    HbAction *hbAction = qobject_cast<HbAction *>(action);
    if (hbAction)
        emit q->triggered(hbAction);
}

void HbToolButtonPrivate::_q_actionChanged()
{
    Q_Q(HbToolButton);
    HbAction *hbAction = qobject_cast<HbAction *>(action);
    bool oldIconProperty = q->property("icon").toBool();
    bool oldTextProperty = q->property("text").toBool();
    if ((hbAction && !hbAction->icon().isNull()) || (action && !action->icon().isNull())) {
        q->setProperty("icon", true);
    } else {
        q->setProperty("icon", false);
    }
    if ((!action->text().isEmpty() && (orientation == Qt::Vertical || isToolBarExtension())) ||
        (orientation == Qt::Horizontal && !q->property("icon").toBool())) {
        q->setProperty("text", true);
    } else {
        q->setProperty("text", false);
    }
    if (oldTextProperty != q->property("text").toBool() ||
        oldIconProperty != q->property("icon").toBool()) {
        q->repolish();
    } else if (polished) {
        q->updatePrimitives();
    }
}

void HbToolButtonPrivate::showToolTip()
{
    Q_Q(HbToolButton);
    HbToolTip::showText(q->toolTip(), q, orientation == Qt::Vertical ? Qt::AlignTop : Qt::AlignLeft);
}

/*!
    Constructs a new HbToolButton with \a parent.
*/
HbToolButton::HbToolButton(QGraphicsItem *parent) :
    HbAbstractButton(*new HbToolButtonPrivate, parent)
{
    Q_D(HbToolButton);
    d->q_ptr = this;
}

/*!
    Constructs a new HbToolButton with \a action and \a parent.

    Ownership of the \a action is not transferred to the tool button.
 */
HbToolButton::HbToolButton(HbAction *action, QGraphicsItem *parent) :
    HbAbstractButton(*new HbToolButtonPrivate, parent)
{
    Q_D(HbToolButton);
    d->q_ptr = this;
    setAction(action);
    setProperty("state", "normal");
}

/*!
    Destructs the tool button.
 */
HbToolButton::~HbToolButton()
{
}

/*!
    Returns the action of the button.

    \sa setAction()
 */
HbAction *HbToolButton::action() const
{
    Q_D(const HbToolButton);
    return qobject_cast<HbAction *>(d->action);
}

/*!
    Sets the \a action of the button.

    Ownership of the \a action is not transferred to the tool button.

    \sa action()
 */
void HbToolButton::setAction(HbAction *action)
{
    Q_D(HbToolButton);
    if (d->action == action) {
        return;
    }

    if (d->action) {
        disconnect(d->action, SIGNAL(triggered()), this, SLOT(_q_actionTriggered()));
        disconnect(d->action, SIGNAL(changed()), this, SLOT(_q_actionChanged()));
    }

    QAction *oldAction = d->action;
    d->action = action;
    if (d->action) {
        connect(action, SIGNAL(triggered()), this, SLOT(_q_actionTriggered()));
        connect(action, SIGNAL(changed()), this, SLOT(_q_actionChanged()));
    }
    // If action was null then there is a chance that the iconitem is not yet created.
    // If the new action is null then we may need to get rid of the icon completely.
    if ((!oldAction && action) || (oldAction && !action)) {
        repolish(); // will call createPrimitives()
    }
    if (isVisible() && d->polished) {
        updatePrimitives();
    }
}

/*!
    Returns the background image of the button.
 */
HbIcon HbToolButton::background() const
{
    Q_D(const HbToolButton);
    return d->customBackground;
}

/*!
    Sets the \a background image of the button.
 */
void HbToolButton::setBackground(const HbIcon &background)
{
    Q_D(HbToolButton);
    if (d->customBackground != background) {
        d->customBackground = background;
        if (isVisible() || d->polished)
            updatePrimitives();
    }
}

/*!
    \reimp
 */
void HbToolButton::updatePrimitives()
{
    Q_D(HbToolButton);
    
    if (d->action) {
        setCheckable(d->action->isCheckable());
        setChecked(d->action->isChecked());
        setEnabled(d->action->isEnabled());
        HbAction *hbAction = qobject_cast<HbAction *>(d->action);
        if (hbAction && hbAction->toolTip() != toolTip()) {
            // by default, HbAction does not have tooltip. If QAction
            // does not have tooltip, it returns button text. This is
            // here to enable empty tooltip text by using HbAction.
            setToolTip(hbAction->toolTip());
        } else if(!hbAction && d->action->toolTip() != toolTip()) {
            setToolTip(d->action->toolTip());
        }
        setVisible(d->action->isVisible());
    } else {
        setCheckable(false);
        setChecked(false);
        setEnabled(false);
        setToolTip(QString());
    }

    setProperty("dialogtoolbar", d->mDialogToolBar);
    if (d->frameItem) {
        HbStyleFramePrimitiveData data;
        initPrimitiveData(&data, d->frameItem);
        style()->updatePrimitive(d->frameItem, &data, this);
        d->frameItem->update();
    }
    if (d->iconItem) {
        HbStyleIconPrimitiveData data;
        initPrimitiveData(&data, d->iconItem);

        bool itemHasNoContents = false;
        if (data.icon.isSet()) {
            if (data.icon.value().isNull()) {
                itemHasNoContents = true;
            } else {
                style()->updatePrimitive(d->iconItem, &data, this);
                d->iconItem->update();
            }
        }
        if (itemHasNoContents) {
            setProperty("icon", false);
        } else {
            setProperty("icon", true);
        }
        d->iconItem->setFlag(QGraphicsItem::ItemHasNoContents, itemHasNoContents);
    }
    if (d->textItem) {
        HbStyleTextPrimitiveData data;
        initPrimitiveData(&data, d->textItem);
        bool itemHasNoContents = false;
        if (data.text.isSet()) {
            if ( data.text.value().isEmpty() || data.text.value().isNull()) {
                itemHasNoContents = true;
            }
        }
        if ((!property("icon").toBool() && d->orientation == Qt::Horizontal) ||
            (!itemHasNoContents && (d->orientation == Qt::Vertical || d->isToolBarExtension()))) {
            setProperty("text", true);
            itemHasNoContents = false;
        } else {
            setProperty("text", false);
            itemHasNoContents = true;
        }
        d->textItem->setFlag(QGraphicsItem::ItemHasNoContents, itemHasNoContents);
        if(!itemHasNoContents) {
            style()->updatePrimitive(d->textItem, &data, this);
            /* HbDialog::setPrimaryAction deprecation action coloring - begin */
            if (d->action && d->action->property("invalid_addition").isValid() ) {
                HbTextItem *textItem = qgraphicsitem_cast<HbTextItem*>(d->textItem);
                textItem->setTextColor(QColor("magenta"));
            }
            /* HbDialog::setPrimaryAction deprecation action coloring - end */
            d->textItem->update();
        }

                /* HbDialog::setPrimaryAction deprecation action coloring - begin */
                if (d->action && d->action->property("invalid_addition").isValid() ) {
                    HbIconItem *iconItem = qgraphicsitem_cast<HbIconItem*>(d->iconItem);
                    iconItem->setColor(QColor("magenta"));
                }
                /* HbDialog::setPrimaryAction deprecation action coloring - end */
    }
}


/*!
    \internal
 */
HbToolButton::HbToolButton(HbToolButtonPrivate &dd, QGraphicsItem *parent) :
    HbAbstractButton(dd, parent)
{
}

/*!
    \reimp
 */
void HbToolButton::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_D(HbToolButton);
    QGraphicsWidget::resizeEvent(event);
    if (event->newSize() !=  event->oldSize() && d->polished && isVisible()) {
        updatePrimitives();
    }
    HbAction *hbAction = qobject_cast<HbAction *>(d->action);
    if ( hbAction && hbAction->toolBarExtension()) {
        HbToolBarExtensionPrivate::d_ptr(hbAction->toolBarExtension())->placeToolBarExtension();
    }
}

/*!
    \reimp
 */
void HbToolButton::nextCheckState()
{
    Q_D(HbToolButton);
    HbAbstractButton::nextCheckState();
    if (!d->action) {
        return;
    }
    HbAction *hbAction = qobject_cast<HbAction *>(d->action);
    if ( hbAction && hbAction->toolBarExtension() ) {
        HbToolBarExtensionPrivate::d_ptr(hbAction->toolBarExtension())->mExtendedButton = this;
        hbAction->toolBarExtension()->show();
    }

    d->action->trigger();
}

/*!
    \reimp
 */
void HbToolButton::checkStateSet()
{
    Q_D(HbToolButton);
    if (d->checked || (d->checkable && d->down)) {
        setProperty( "state", "latched" );
    } else if (d->down) {
        setProperty( "state", "pressed" );
    } else {
        setProperty( "state", "normal" );
    }
}

/*!
    \reimp
 */
bool HbToolButton::sceneEvent(QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneHelp) {
        Q_D(HbToolButton);
        // Check whether toolbutton is inside a toolbar.
        if (d->toolBarPosition != HbToolButtonPrivate::TB_None) {
            d->showToolTip();
            event->accept();
            return true;
        }
    }
    return HbAbstractButton::sceneEvent(event);
}

void HbToolButton::initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive)
{
    Q_D(HbToolButton);
    HbAbstractButton::initPrimitiveData(primitiveData, primitive);
    QString itemName = HbStyle::itemName(primitive);
    if (itemName == QLatin1String("background")) {
        d->framePrimitiveData(hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData));
    } else if (itemName == QLatin1String("text")) {
        d->textPrimitiveData(hbstyleprimitivedata_cast<HbStyleTextPrimitiveData*>(primitiveData));
    } else if (itemName == QLatin1String("icon")) {
        d->iconPrimitiveData(hbstyleprimitivedata_cast<HbStyleIconPrimitiveData*>(primitiveData));
    }
}

/*!
    \reimp
 */
bool HbToolButton::event(QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneMouseRelease) {
        mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent*>(event));
        return true;
    } else if(event->type() == HbEvent::ThemeChanged) {
        Q_D(HbToolButton);
        changeEvent(event);
        if (d->frameItem) {
            HbFrameItem *item = static_cast<HbFrameItem*>(d->frameItem);
            item->frameDrawer().themeChanged();
        }
    }

    return HbAbstractButton::event(event);
}

void HbToolButton::polish(HbStyleParameters &params)
{
    Q_D(HbToolButton);
    setProperty("orientation", d->orientation);
    d->createPrimitives();
    updatePrimitives();
    HbAbstractButton::polish(params);
}

#include "moc_hbtoolbutton.cpp"
