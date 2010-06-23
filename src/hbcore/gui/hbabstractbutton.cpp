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

#include "hbabstractbutton.h"
#include "hbabstractbutton_p.h"
#include "hbapplication.h"
#include "hbstyleoption_p.h"
#include "hbtooltip.h"
#include "hbinstance.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QPointer>
#include <QStyle>

#include <hbwidgetfeedback.h>

#ifdef HB_GESTURE_FW
#include <hbtapgesture.h>
#include <hbpangesture.h>
#endif

namespace {
    static const int AUTO_REPEAT_DELAY = 300;
    static const int AUTO_REPEAT_INTERVAL = 100;
}

/*!
    @beta
    @hbcore
    \class HbAbstractButton

    \brief The HbAbstractButton class is the abstract base class of
    button widgets, providing functionality common to buttons.

    This class implements an \e abstract button.
    Subclasses of this class handle user actions, and specify how the button
    is drawn.

    HbAbstractButton provides support for both push buttons and checkable
    (toggle) buttons. Checkable buttons are implemented in the HbRadioButton
    and HbCheckBox classes. Push buttons are implemented in the
    HbPushButton and HbToolButton classes; these also provide toggle
    behavior if required.

    Any button can display  text and an icon. setText()
    sets the text; setIcon() sets the icon. If a button is disabled, its background
    is changed to give the button a "disabled" appearance.

    All of the buttons provided by Hb (HbPushButton, HbToolButton,
    HbCheckBox, and HbRadioButton) can display both text and icon.
    You can also set a tool tip for the button with \qtfunc{QGraphicsItem,setTooltip}.

    HbAbstractButton provides most of the states used for buttons:

    \li isDown() indicates whether the button is \e pressed down.

    \li isChecked() indicates whether the button is \e checked.  Only
    checkable buttons can be checked and unchecked (see below).

    \li isEnabled() indicates whether the button can be pressed by the
    user.

    \li setAutoRepeat() sets whether the button will auto-repeat if the
    user holds it down. \l autoRepeatDelay and \l autoRepeatInterval
    define how auto-repetition is done.

    \li setCheckable() sets whether the button is a toggle button or not.

    The difference between isDown() and isChecked() is as follows.
    When the user clicks a toggle button to check it, the button is first
    \e pressed then released into the \e checked state. When the user
    clicks it again (to uncheck it), the button moves first to the
    \e pressed state, then to the \e unchecked state (isChecked() and
    isDown() are both false).

    HbAbstractButton provides four signals:

    \li pressed() is emitted when the stylus is pressed while
    the stylus is inside the button.

    \li released() is emitted when the left stylus is released.

    \li clicked() is emitted when the button is first pressed and then
    released, when the shortcut key is typed, or when click() or
    animateClick() is called.

    \li toggled() is emitted when the state of a toggle button changes.

    To subclass HbAbstractButton, you must reimplement at least
    paint() to draw the button's outline and its text or pixmap. It
    is generally advisable to reimplement sizeHint() as well, and
    sometimes hitButton() (to determine whether a button press is within
    the button). For buttons with more than two states (like tri-state
    buttons), you will also have to reimplement checkStateSet() and
    nextCheckState().
*/

/*!
    \reimp
    \fn int HbAbstractButton::type() const
 */

HbAbstractButtonPrivate::HbAbstractButtonPrivate( QSizePolicy::ControlType type )
    :
    shortcutId(0),
    checkable(false), checked(false), autoRepeat(false), autoExclusive(false),
    down(false), blockRefresh(false),longPress(false),
    autoRepeatDelay(AUTO_REPEAT_DELAY),
    autoRepeatInterval(AUTO_REPEAT_INTERVAL),
    controlType(type), sizeHint(),mSizeHintPolish(false),mRepolishRequested(false)
{
}

HbAbstractButtonPrivate::~HbAbstractButtonPrivate()
{
}

QList<HbAbstractButton *>HbAbstractButtonPrivate::queryButtonList() const
{
    Q_Q( const HbAbstractButton );
    
    QList<HbAbstractButton*>candidates;
    if (q->parentWidget()) {
        candidates =  qFindChildren<HbAbstractButton *>(q->parentWidget());
        if (autoExclusive) {
            for (int i = candidates.count() - 1; i >= 0; --i) {
                HbAbstractButton *candidate = candidates.at(i);
                if (!candidate->autoExclusive())
                    candidates.removeAt(i);
            }
        }
    }
    return candidates;
}

HbAbstractButton *HbAbstractButtonPrivate::queryCheckedButton() const
{
    QList<HbAbstractButton *> buttonList = queryButtonList();
    if (!autoExclusive || buttonList.count() == 1) // no group
        return 0;

    Q_Q( const HbAbstractButton );

    for (int i = 0; i < buttonList.count(); ++i) {
        HbAbstractButton *b = buttonList.at(i);
        if (b->isChecked() && b != q)
            return b;
    }
    return checked  ? const_cast<HbAbstractButton *>(q) : 0;
}

void HbAbstractButtonPrivate::notifyChecked()
{
    if (autoExclusive) {
        if (HbAbstractButton *b = queryCheckedButton())
            b->setChecked(false);
    }
}

void HbAbstractButtonPrivate::moveFocus(int key)
{
    Q_Q( HbAbstractButton );

    QList<HbAbstractButton *> buttonList = queryButtonList();;
    bool exclusive = autoExclusive;
    QGraphicsItem *focusItem = q->scene()->focusItem();
    HbAbstractButton *focusButton = 0;
    if (focusItem && focusItem->isWidget()) {
        QGraphicsWidget *focusWidget = static_cast<QGraphicsWidget*>(focusItem);
        focusButton = qobject_cast<HbAbstractButton *>(focusWidget);
    }
    if (!focusButton || !buttonList.contains(focusButton))
        return;

    HbAbstractButton *candidate = 0;
    int bestScore = -1;
    QRect target = focusItem->sceneBoundingRect().toRect();
    QPoint goal = target.center();

    for (int i = 0; i < buttonList.count(); ++i) {
        HbAbstractButton *button = buttonList.at(i);
        if (button != focusItem && button->isEnabled() && button->isVisible() &&
            (autoExclusive || (button->focusPolicy() & Qt::StrongFocus) == Qt::StrongFocus)) {
            QRect buttonRect = button->sceneBoundingRect().toRect();
            QPoint p = buttonRect.center();

            //Priority to widgets that overlap on the same coordinate.
            //In that case, the distance in the direction will be used as significant score,
            //take also in account orthogonal distance in case two widget are in the same distance.
            int score;
            if ( keyNavigation() && (buttonRect.x() < target.right() && target.x() < buttonRect.right())
                  && (key == Qt::Key_Up || key == Qt::Key_Down)) {
                //one item's is at the vertical of the other
                score = (qAbs(p.y() - goal.y()) << 16) + qAbs(p.x() - goal.x());
            } else if ( keyNavigation() && (buttonRect.y() < target.bottom() && target.y() < buttonRect.bottom())
                        && (key == Qt::Key_Left || key == Qt::Key_Right) ) {
                //one item's is at the horizontal of the other
                score = (qAbs(p.x() - goal.x()) << 16) + qAbs(p.y() - goal.y());
            } else {
                score = (1 << 30) + (p.y() - goal.y()) * (p.y() - goal.y()) + (p.x() - goal.x()) * (p.x() - goal.x());
            }

            if (score > bestScore && candidate)
                continue;
            if ( keyNavigation()) {
                switch(key) {
                case Qt::Key_Up:
                    if (p.y() < goal.y()) {
                        candidate = button;
                        bestScore = score;
                    }
                    break;
                case Qt::Key_Down:
                    if (p.y() > goal.y()) {
                        candidate = button;
                        bestScore = score;
                    }
                    break;
                case Qt::Key_Left:
                    if (p.x() < goal.x()) {
                        candidate = button;
                        bestScore = score;
                    }
                    break;
                case Qt::Key_Right:
                    if (p.x() > goal.x()) {
                        candidate = button;
                        bestScore = score;
                    }
                    break;
                }
            }
        }
    }

    if (exclusive
        && !keyNavigation()
        && candidate
        && focusButton->isChecked()
        && candidate->isCheckable())
        candidate->click();

    if ( keyNavigation() && candidate) {
        if (key == Qt::Key_Up || key == Qt::Key_Left)
            candidate->setFocus(Qt::BacktabFocusReason);
        else
            candidate->setFocus(Qt::TabFocusReason);
    }
}

void HbAbstractButtonPrivate::fixFocusPolicy()
{
    if (!autoExclusive)
        return;

    Q_Q( HbAbstractButton );

    QList<HbAbstractButton *> buttonList = queryButtonList();
    for (int i = 0; i < buttonList.count(); ++i) {
        HbAbstractButton *b = buttonList.at(i);
        if (!b->isCheckable())
            continue;
        b->setFocusPolicy((Qt::FocusPolicy) ((b == q || !q->isCheckable())
                                         ? (b->focusPolicy() | Qt::TabFocus)
                                         :  (b->focusPolicy() & ~Qt::TabFocus)));
    }
}

void HbAbstractButtonPrivate::init()
{
    Q_Q( HbAbstractButton );

	q->setFocusPolicy(Qt::FocusPolicy(qApp->style()->styleHint(QStyle::SH_Button_FocusPolicy)));

#ifdef HB_GESTURE_FW
    q->grabGesture(Qt::TapGesture);
#endif

    // FIXME: size policy is commented out b/c of a bug in Qt #236689, also in our bugtracker.
    //q->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum, controlType));

    //q->setForegroundRole(QPalette::ButtonText); TODO: check
    //q->setBackgroundRole(QPalette::Button); TODO: check
}

void HbAbstractButtonPrivate::refresh()
{
    if (blockRefresh)
        return;

    Q_Q( HbAbstractButton );

    q->updatePrimitives();
}

void HbAbstractButtonPrivate::click()
{
    Q_Q( HbAbstractButton );

    down = false;
    blockRefresh = true;
    bool changeState = true;
    if (checked && queryCheckedButton() == q) {
        // the checked button of an exclusive or autoexclusive group cannot be unchecked
        if (autoExclusive)
            changeState = false;
    }

    QPointer<HbAbstractButton> guard(q);
    if (changeState) {
        q->nextCheckState();
        if (!guard)
            return;
    }
    blockRefresh = false;
    refresh();
    if (guard)
        emitReleased();
    if (guard && !longPress)
        emitClicked();
    if(guard && longPress)
        longPress = false;
}

void HbAbstractButtonPrivate::emitClicked()
{
    Q_Q( HbAbstractButton );

    QPointer<HbAbstractButton> guard(q);
    emit q->clicked(checked);
}

void HbAbstractButtonPrivate::emitPressed()
{
    Q_Q( HbAbstractButton );

    QPointer<HbAbstractButton> guard(q);
    emit q->pressed();
}

void HbAbstractButtonPrivate::emitReleased()
{
    Q_Q( HbAbstractButton );

    QPointer<HbAbstractButton> guard(q);
    emit q->released();
}



/*!
    @beta
    Constructs an abstract button with a \a parent.
*/
HbAbstractButton::HbAbstractButton(QGraphicsItem *parent)
    : HbWidget( *new HbAbstractButtonPrivate, parent)
{
    Q_D( HbAbstractButton );
    d->init();
}

/*!
    @beta
    \internal
 */
HbAbstractButton::HbAbstractButton( HbAbstractButtonPrivate &dd, QGraphicsItem * parent ) :
        HbWidget( dd, parent )
{
    Q_D( HbAbstractButton );
    d->init();
}

/*!
    Destroys the button.
 */
 HbAbstractButton::~HbAbstractButton()
{
}

/*
\property HbAbstractButton::shortcut
\brief the mnemonic associated with the button

void HbAbstractButton::setShortcut(const QKeySequence &key)
{
    Q_ASSERT(mainWindow());
    if (d->shortcutId != 0)
        mainWindow()->releaseShortcut(d->shortcutId);
    d->shortcut = key;
    d->shortcutId = mainWindow()->grabShortcut(key);
}

QKeySequence HbAbstractButton::shortcut() const
{
    return d->shortcut;
}
*/

/*!
    @beta
    Returns whether the button is checkable.

    By default, the button is not checkable.

    \sa setCheckable() checked
*/
bool HbAbstractButton::isCheckable() const
{
    Q_D( const HbAbstractButton );

    return d->checkable;
}

/*!
    @beta
    Sets whether the button is checkable.

    \sa isCheckable()
 */
void HbAbstractButton::setCheckable(bool checkable)
{
    Q_D( HbAbstractButton );

    if (d->checkable == checkable)
        return;

    d->checkable = checkable;
    d->checked = false;
}

/*!
    @beta
    Returns whether the button is checked.

    Only checkable buttons can be checked. By default, the button is unchecked.

    \sa setChecked() setCheckable()
*/
bool HbAbstractButton::isChecked() const
{
    Q_D( const HbAbstractButton );

    return d->checked;
}

/*!
    @beta
    Sets whether the button is checked.

    Only checkable buttons can be checked. By default, the button is unchecked.

    \sa isChecked() isCheckable()
*/
void HbAbstractButton::setChecked(bool checked)
{
    Q_D( HbAbstractButton );

    if (!d->checkable || d->checked == checked) {
        if (!d->blockRefresh)
            checkStateSet();
        return;
    }
    if (!checked && d->queryCheckedButton() == this) {
        // the checked button of an exclusive or autoexclusive group cannot be  unchecked
        if (d->autoExclusive)
            return;
    }

    QPointer<HbAbstractButton> guard(this);

    d->checked = checked;
    if (!d->blockRefresh)
        checkStateSet();

    d->refresh();

    if (guard && checked)
        d->notifyChecked();
    if (guard)
        emit toggled(checked);
}

/*!
    @beta
    Returns whether the button is pressed down.

    If this property is \c true, the button is pressed down.
    The default value is \c false.

    \sa setDown()
*/
bool HbAbstractButton::isDown() const
{
    Q_D( const HbAbstractButton );

    return d->down;
}

/*!
    @beta
    Sets whether the button is pressed down.

    The signals pressed() and clicked() are not emitted
    if you set this property to \c true.

    \sa isDown()
 */
void HbAbstractButton::setDown(bool down)
{
    Q_D( HbAbstractButton );

    if (d->down == down)
        return;
    d->down = down;
    d->refresh();
    if (d->autoRepeat && d->down)
        d->repeatTimer.start(d->autoRepeatDelay, this);
    else
        d->repeatTimer.stop();
}

/*!
    @beta
    Returns whether autoRepeat is enabled.

    If autoRepeat is enabled, then the pressed(), released(), and clicked() signals are emitted at
    regular intervals when the button is down. autoRepeat is off by default.
    The initial delay and the repetition interval are defined in milliseconds by \l
    autoRepeatDelay and \l autoRepeatInterval.

    Note: If a button is pressed down by a shortcut key, then auto-repeat is enabled and timed by the
    system and not by this class. The pressed(), released(), and clicked() signals will be emitted
    like in the normal case.
*/
bool HbAbstractButton::autoRepeat() const
{
    Q_D( const HbAbstractButton );

    return d->autoRepeat;
}

/*!
    @beta
    Sets whether autoRepeat is enabled.
 */
void HbAbstractButton::setAutoRepeat(bool autoRepeat)
{
    Q_D( HbAbstractButton );

    if (d->autoRepeat == autoRepeat)
        return;
    d->autoRepeat = autoRepeat;
    if (d->autoRepeat && d->down)
        d->repeatTimer.start(d->autoRepeatDelay, this);
    else
        d->repeatTimer.stop();
}

/*!
    @beta
    Returns the initial delay of auto-repetition in milliseconds.

    If \l autoRepeat is enabled, then autoRepeatDelay defines the initial
    delay in milliseconds before auto-repetition kicks in.

    \sa autoRepeat, autoRepeatInterval
*/
int HbAbstractButton::autoRepeatDelay() const
{
    Q_D( const HbAbstractButton );

    return d->autoRepeatDelay;
}

/*!
    @beta
    Sets the initial delay of auto-repetition in milliseconds.
 */
void HbAbstractButton::setAutoRepeatDelay(int autoRepeatDelay)
{
    Q_D( HbAbstractButton );

    d->autoRepeatDelay = autoRepeatDelay;
}

/*!
    @beta
    Returns the interval of auto-repetition.

    If \l autoRepeat is enabled, then autoRepeatInterval defines the
    length of the auto-repetition interval in millisecons.

    \sa autoRepeat, autoRepeatDelay
*/
int HbAbstractButton::autoRepeatInterval() const
{
    Q_D( const HbAbstractButton );

    return d->autoRepeatInterval;
}

/*!
    @beta
    Sets the interval of auto-repetition.
 */
void HbAbstractButton::setAutoRepeatInterval(int autoRepeatInterval)
{
    Q_D( HbAbstractButton );

    d->autoRepeatInterval = autoRepeatInterval;
}

/*!
    @beta
    Returns whether auto-exclusivity is enabled.

    If auto-exclusivity is enabled, checkable buttons that belong to the
    same parent widget behave as if they were part of the same
    exclusive button group. In an exclusive button group, only one button
    can be checked at any time; checking another button automatically
    unchecks the previously checked one.

    The property has no effect on buttons that belong to a button
    group.

    autoExclusive is off by default, except for radio buttons.

    \sa setAutoExclusive()
*/
bool HbAbstractButton::autoExclusive() const
{
    Q_D( const HbAbstractButton );

    return d->autoExclusive;
}

/*!
    @beta
    Sets whether auto-exclusivity is enabled.

    \sa autoExclusive()
 */
void HbAbstractButton::setAutoExclusive(bool autoExclusive)
{
    Q_D( HbAbstractButton );

    d->autoExclusive = autoExclusive;
}



/*!
    Performs an animated click: the button is pressed immediately, and
    released \a msec milliseconds later (the default is 100 ms).

    Calling this function again before the button was released will reset
    the release timer.

    All signals associated with a click are emitted as appropriate.

    This function does nothing if the button is \link setEnabled()
    disabled. \endlink

    \sa click()
*/
void HbAbstractButton::animateClick(int msec)
{
    Q_D( HbAbstractButton );

    if (!isEnabled())
        return;
    if (d->checkable && focusPolicy() & Qt::ClickFocus)
        setFocus();
    setDown(true);
    updatePrimitives();

    if (!d->animateTimer.isActive())
        d->emitPressed();
    d->animateTimer.start(msec, this);
}

/*!
    Performs a click.

    All the usual signals associated with a click are emitted as
    appropriate. If the button is checkable, the state of the button is
    toggled.

    This function does nothing if the button is \link setEnabled()
    disabled. \endlink

    \sa animateClick()
 */
void HbAbstractButton::click()
{
    Q_D( HbAbstractButton );

    if (!isEnabled())
        return;
    QPointer<HbAbstractButton> guard(this);
    d->down = true;
    d->emitPressed();
    if (guard) {
        d->down = false;
        nextCheckState();
        if (guard)
            d->emitReleased();
        if (guard && !d->longPress)
            d->emitClicked();
    }
}

/*!
    \fn void HbAbstractButton::toggle()

    Toggles the state of a checkable button.

     \sa checked
*/
void HbAbstractButton::toggle()
{
    Q_D( HbAbstractButton );

    setChecked(!d->checked);
}

/*!
    This virtual handler is called when setChecked() was called,
    unless it was called from within nextCheckState(). It allows
    subclasses to reset their intermediate button states.

    \sa nextCheckState()
 */
void HbAbstractButton::checkStateSet()
{
}

/*!
    This virtual handler is called when a button is clicked. The
    default implementation calls setChecked(!isChecked()) if the button
    isCheckable().  It allows subclasses to implement intermediate button
    states.

    \sa checkStateSet()
*/
void HbAbstractButton::nextCheckState()
{
    if (isCheckable())
        setChecked(!isChecked());
}

/*!
    Returns \c true if \a pos is inside the clickable button rectangle;
    otherwise returns \c false.

    By default, the clickable area is the entire widget. Subclasses
    may reimplement this function to provide support for clickable
    areas of different shapes and sizes.
*/
bool HbAbstractButton::hitButton(const QPointF &pos) const
{
    return rect().contains(pos);
}

/*!
    Initializes \a option with the values from this HbAbstractButton. This method is useful for subclasses when they need a HbStyleOption, but don't want to fill in the pressed state information themselves.
 */
void HbAbstractButton::initStyleOption(HbStyleOption *option) const
{
    HbWidget::initStyleOption(option);
    Q_ASSERT(option);
    option->state |= (isChecked() | isDown() ? QStyle::State_On : QStyle::State_Off);
}

/*!
    \reimp
 */
bool HbAbstractButton::event(QEvent *event)
{
    // as opposed to other widgets, disabled buttons accept mouse
    // events. This avoids surprising click-through scenarios	
    Q_D( HbAbstractButton );
    if (!isEnabled()) {
        switch(event->type()) {
        case QEvent::TabletPress:
        case QEvent::TabletRelease:
        case QEvent::TabletMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::HoverMove:
        case QEvent::HoverEnter:
        case QEvent::HoverLeave:
        case QEvent::ContextMenu:
#ifndef QT_NO_WHEELEVENT
        case QEvent::Wheel:
#endif
        case QEvent::Gesture:
            return true;
        default:
            break;
        }
    }
    if(event->type() ==  QEvent::MouseMove) {
        return true;
    }

#ifndef QT_NO_SHORTCUT
    if (event->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent *>(event);
        if (d->shortcutId != se->shortcutId())
            return false;
        if (!se->isAmbiguous()) {
            if (!d->animateTimer.isActive())
                animateClick();
        } else {
            if (focusPolicy() != Qt::NoFocus)
                setFocus(Qt::ShortcutFocusReason);
            window()->setAttribute(Qt::WA_KeyboardFocusChange);
        }
        return true;
    }
#endif
    return HbWidget::event(event);
}

#ifndef HB_GESTURE_FW
/*!
    \reimp
 */
void HbAbstractButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D( HbAbstractButton );

    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }
    if (hitButton(event->pos())) {
        setDown(true);
        HbWidgetFeedback::triggered(this, Hb::InstantPressed);
        updatePrimitives();
        d->emitPressed();
        event->accept();
    } else {
        event->ignore();
    }
}

/*!
    \reimp
 */
void HbAbstractButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //This check is not required for s60 env
#ifndef Q_OS_SYMBIAN    
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }
#endif

    Q_D( HbAbstractButton );

    //TODO: this code will be unnecessary when event filter of HbToolTipLabel will be implemented!!
    //HbToolTip::hideText();

    if (!d->down) {
        event->ignore();
        return;
    }
    if (hitButton(event->pos()) && !d->longPress) {
        HbWidgetFeedback::triggered(this, Hb::InstantClicked);
    }
    HbWidgetFeedback::triggered(this, Hb::InstantReleased);
    if (hitButton(event->pos())) {
        d->repeatTimer.stop();
        d->click();
        event->accept();
    } else {
        setDown(false);
        event->ignore();
    }
    d->longPress = false;
}

/*!
    \reimp
 */
void HbAbstractButton::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        event->ignore();
        return;
    }

    Q_D( HbAbstractButton );

    bool hit = hitButton(event->pos());
    if (!hit) {
        //TODO: this code will be unnecessary when event filter of HbToolTipLabel will be implemented!!
        //HbToolTip::hideText();
    }

    if (hit != d->down) {
        setDown(!d->down);
        updatePrimitives();
        if (d->down) {
            // this call show tooltip for button,
            // for the case press, move outside and come back to same button.
            HbToolTip::showText(toolTip(), this);
            d->emitPressed();
            HbWidgetFeedback::triggered(this, Hb::InstantPressed);
        } else {
            d->emitReleased();
            d->longPress = false;
            HbWidgetFeedback::triggered(this, Hb::InstantReleased);
        }
        event->accept();
    } else if (!hit) {
        event->ignore();
    }
}
#endif

#ifdef HB_GESTURE_FW
void HbAbstractButton::gestureEvent(QGestureEvent *event)
{
    Q_D(HbAbstractButton);
    
    if (HbTapGesture *tap = qobject_cast<HbTapGesture *>(event->gesture(Qt::TapGesture))) {
        bool hit = hitButton(mapFromScene(event->mapToGraphicsScene(tap->position())));
        switch(tap->state()) {
	        case Qt::GestureStarted:
                if( hit ){
                    setDown(true);
                    HbWidgetFeedback::triggered(this, Hb::InstantPressed);
                    updatePrimitives();
                    d->emitPressed();
                }
                break;
            case Qt::GestureCanceled:
                if(d->down) {
                    HbWidgetFeedback::triggered(this, Hb::InstantReleased);
                    setDown(false);
                    d->longPress = false;
                    d->emitReleased();
                }
                break;
            case Qt::GestureFinished:
                if (!d->down){
                    return;
                }
                if ( hit ){
                    HbWidgetFeedback::triggered(this, Hb::InstantClicked);
                    d->repeatTimer.stop();
                    d->click();
                }else{
                    setDown(false);
                }
                HbWidgetFeedback::triggered(this, Hb::InstantReleased);
                d->longPress = false;
                break;
            default:
                break;
            }
    }
}
#endif

/*!
    \reimp
 */
void HbAbstractButton::keyPressEvent(QKeyEvent *event)
{
    Q_D( HbAbstractButton );

    bool next = true;
    switch (event->key()) {
 
    case Qt::Key_Select:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (!event->isAutoRepeat()) {
            setDown(true);
            updatePrimitives();
            d->emitPressed();
        }
        break;
    case Qt::Key_Up:
    case Qt::Key_Left:
        if ( d->keyNavigation() ) {
            next = false;
        }
        // fall through
    case Qt::Key_Right:
    case Qt::Key_Down:
        if ( d->keyNavigation() ) {
            if ( d->keyNavigation() && (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)) {
                event->ignore();
                return;
            }
            if (d->autoExclusive) {
                // ### Using qobject_cast to check if the parent is a viewport of
                // QAbstractItemView is a crude hack, and should be revisited and
                // cleaned up when fixing task 194373. It's here to ensure that we
                // keep compatibility outside QAbstractItemView.
                d->moveFocus(event->key());
                if (hasFocus()) // nothing happend, propagate
                    event->ignore();
            } else {
                focusNextPrevChild(next);
            }
        }
        break;
    case Qt::Key_Escape:
        if (d->down) {
            setDown(false);
            updatePrimitives();
            d->emitReleased();
            break;
        }
        // fall through
    default:
        event->ignore();
    }
}

/*!
    \reimp
 */
void HbAbstractButton::keyReleaseEvent(QKeyEvent *event)
{
    Q_D( HbAbstractButton );

    if (!event->isAutoRepeat())
        d->repeatTimer.stop();

    switch (event->key()) {
        case Qt::Key_Select:
        case Qt::Key_Enter:
        case Qt::Key_Return: {
            if (!event->isAutoRepeat() /*&& d->down*/) {
                if (d->down && !d->longPress) {
                    HbWidgetFeedback::triggered(this, Hb::InstantClicked);
                }
                d->click();
            }
	    break;
        }
        default:
            event->ignore();
    }
}

/*!
    \reimp
 */
void HbAbstractButton::timerEvent(QTimerEvent *event)
{
    Q_D( HbAbstractButton );

    if (event->timerId() == d->repeatTimer.timerId()) {
        d->repeatTimer.start(d->autoRepeatInterval, this);
        if (d->down) {
            QPointer<HbAbstractButton> guard(this);
            d->emitReleased();
            if (guard) {
                d->emitClicked();
            }
            if (guard) {
                d->emitPressed();
                HbWidgetFeedback::triggered(this, Hb::InstantKeyRepeated);
            }
        }
    } else if (event->timerId() == d->animateTimer.timerId()) {
        d->animateTimer.stop();
        d->click();
    }
}


/*!
    \reimp
 */
void HbAbstractButton::focusInEvent(QFocusEvent *event)
{
    Q_D( HbAbstractButton );

#ifdef QT_KEYPAD_NAVIGATION
    if (!QApplication::keypadNavigationEnabled())
#endif
    d->fixFocusPolicy();
    HbWidget::focusInEvent(event);
}

/*!
    \reimp
 */
void HbAbstractButton::changeEvent(QEvent *event)
{
    Q_D( HbAbstractButton );

    switch (event->type()) {
    case QEvent::EnabledChange:
        if (!isEnabled())
            setDown(false);
        break;
    default:
        d->sizeHint = QSize();
        break;
    }
    HbWidget::changeEvent(event);
}

/*!
    \reimp
*/
void HbAbstractButton::polish(HbStyleParameters& params)
{
        Q_D( HbAbstractButton );

    if (d->mSizeHintPolish) {
        d->mSizeHintPolish = false;
        return;
    }
    d->mRepolishRequested = false;
    HbWidget::polish(params);
}

/*!
    \reimp
*/
QSizeF HbAbstractButton::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_D(const HbAbstractButton);
    if (d->mRepolishRequested && isVisible()) {
        d->mRepolishRequested = false;
        // force the polish event in order to get the real size
        QEvent polishEvent(QEvent::Polish);
        QCoreApplication::sendEvent(const_cast<HbAbstractButton *>(this), &polishEvent);
        d->mSizeHintPolish = true;
    }
    return HbWidget::sizeHint(which, constraint);
}

/*!
    \fn void HbAbstractButton::pressed()

    This signal is emitted when the button is pressed down.

    \sa released(), clicked()
*/

/*!
    \fn void HbAbstractButton::released()

    This signal is emitted when the button is released.

    \sa pressed(), clicked(), toggled()
*/

/*!
    \fn void HbAbstractButton::clicked(bool checked)

    This signal is emitted when the button is activated (i.e. pressed down
    then released while the stylus is inside the button), when the
    shortcut key is typed, or when click() or animateClick() is called.
    Notably, this signal is \e not emitted if you call setDown(),
    setChecked() or toggle().

    If the button is checkable, \a checked is true if the button is
    checked, or false if the button is unchecked.

    \sa pressed(), released(), toggled()
*/

/*!
    \fn void HbAbstractButton::toggled(bool checked)

    This signal is emitted whenever a checkable button changes its state.
    \a checked is true if the button is checked, or false if the button is
    unchecked.

    This may be the result of a user action, click() slot activation,
    or because setChecked() was called.

    \sa checked, clicked()
*/

#include "moc_hbabstractbutton.cpp"
