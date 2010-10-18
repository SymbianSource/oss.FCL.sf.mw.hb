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

#include "hbabstractslidercontrol.h"
#include "hbabstractslidercontrol_p.h"
#include <qevent.h>
#include <limits.h>
#include <hbwidgetfeedback.h>

#ifdef HB_EFFECTS
#define HB_SLIDERCONTROL_TYPE "HB_SLIDERCONTROL"
#endif


/*!
    \class HbAbstractSliderControl
    \brief The HbAbstractSliderControl class provides an integer value within a range.

    The class is designed as a common super class for all slider like widgets.

    Here are the main properties of the class:

    \li HbAbstractSliderControl::value: The bounded integer that HbAbstractSliderControl maintains.
    \li HbAbstractSliderControl::minimum: The lowest possible value.
    \li HbAbstractSliderControl::maximum: The highest possible value.
    \li HbAbstractSliderControl::singleStep: The smaller of two natural steps that an
    abstract sliders provides and typically corresponds to the user
    pressing an arrow key.
    \li HbAbstractSliderControl::pageStep: The larger of two natural steps that an abstract
    slider provides and typically corresponds to the user pressing
    PageUp or PageDown.
    \li HbAbstractSliderControl::hasTracking: Whether slider tracking is enabled.
    \li HbAbstractSliderControl::sliderPosition: The current position of the slider. If \l
    tracking is enabled (the default), this is identical to \l value.

    Unity (1) may be viewed as a third step size. setValue() lets you
    set the current value to any integer in the allowed range, not
    just minimum() + \e n * singleStep() for integer values of \e n.
    Some widgets may allow the user to set any value at all; others
    may just provide multiples of singleStep() or pageStep().

    HbAbstractSliderControl emits a comprehensive set of signals:

    \li HbAbstractSliderControl::valueChanged() Emitted when the value has changed. The \l tracking
            determines whether this signal is emitted during user interaction.
    \li HbAbstractSliderControl::sliderPressed(): Emitted when the user starts to drag the slider.
    \li HbAbstractSliderControl::sliderMoved(): Emitted when the user drags the slider.
    \li HbAbstractSliderControl::sliderReleased(): Emitted when the user releases the slider.
    \li HbAbstractSliderControl::actionTriggered(): Emitted when a slider action was triggerd.
    \li HbAbstractSliderControl::rangeChanged(): Emitted when the range has changed.

    HbAbstractSliderControl provides a virtual sliderChange() function that is
    well suited for updating the on-screen representation of
    sliders. By calling triggerAction(), subclasses trigger slider
    actions. Two helper functions \b QStyle::sliderPositionFromValue() and
    \b QStyle::sliderValueFromPosition() help subclasses and styles to map
    screen coordinates to logical range values.
*/

/*!
    \enum HbAbstractSliderControl::SliderAction

    This enum defines available slider actions.
 */

/*!
    \var HbAbstractSliderControl::SliderNoAction

    No action.
 */

/*!
    \var HbAbstractSliderControl::SliderSingleStepAdd

    Add a single step.
 */

/*!
    \var HbAbstractSliderControl::SliderSingleStepSub

    Subtract a single step.
 */

/*!
    \var HbAbstractSliderControl::SliderPageStepAdd

    Add a page step.
 */

/*!
    \var HbAbstractSliderControl::SliderPageStepSub

    Subtract a page step.
 */

/*!
    \var HbAbstractSliderControl::SliderToMinimum

    Move the slider to its minimum.
 */

/*!
    \var HbAbstractSliderControl::SliderToMaximum

    Move the slider to its maximum.
 */

/*!
    \var HbAbstractSliderControl::SliderMove

    Move the slider to its position.
 */

/*!
    \fn void HbAbstractSliderControl::valueChanged(int value)

    This signal is emitted when the slider value has changed, with the
    new slider \a value as argument.
*/

/*!
    \fn void HbAbstractSliderControl::sliderPressed()

    This signal is emitted when the user presses the slider with the
    mouse, or programmatically when setSliderDown(true) is called.

    \sa sliderReleased(), sliderMoved(), isSliderDown()
*/

/*!
    \fn void HbAbstractSliderControl::sliderMoved(int value)

    This signal is emitted when sliderDown is true and the slider moves. This
    usually happens when the user is dragging the slider. The \a value
    is the new slider position.

    This signal is emitted even when tracking is turned off.

    \sa setTracking(), valueChanged(), isSliderDown(),
    sliderPressed(), sliderReleased()
*/

/*!
    \fn void HbAbstractSliderControl::sliderReleased()

    This signal is emitted when the user releases the slider with the
    mouse, or programmatically when setSliderDown(false) is called.

    \sa sliderPressed() sliderMoved() sliderDown
*/

/*!
    \fn void HbAbstractSliderControl::rangeChanged(int min, int max)

    This signal is emitted when the slider range has changed, with \a
    min being the new minimum, and \a max being the new maximum.

    \sa minimum, maximum
*/

/*!
    \fn void HbAbstractSliderControl::actionTriggered(int action)

    This signal is emitted when the slider action \a action is
    triggered. Actions are \l SliderSingleStepAdd, \l
    SliderSingleStepSub, \l SliderPageStepAdd, \l SliderPageStepSub,
    \l SliderToMinimum, \l SliderToMaximum, and \l SliderMove.

    When the signal is emitted, the \l sliderPosition has been
    adjusted according to the action, but the \l value has not yet
    been propagated (meaning the valueChanged() signal was not yet
    emitted), and the visual display has not been updated. In slots
    connected to this signal you can thus safely adjust any action by
    calling setSliderPosition() yourself, based on both the action and
    the slider's value.

    \sa triggerAction()
*/

/*!
    \enum HbAbstractSliderControl::SliderChange

    This enum defines different slider changes.
 */

/*!
    \reimp
    \fn int HbAbstractSliderControl::type() const
 */

/*!
    \var HbAbstractSliderControl::SliderRangeChange

    The range has changed.
 */

/*!
    \var HbAbstractSliderControl::SliderOrientationChange

    The orientation has changed.
 */

/*!
    \var HbAbstractSliderControl::SliderStepsChange

    The steps have changed.
 */

/*!
    \var HbAbstractSliderControl::SliderValueChange

    The value has changed.
 */

HbAbstractSliderControlPrivate::HbAbstractSliderControlPrivate()
    : minimum(0), maximum(100), singleStep(1), pageStep(10),
    value(0), position(0), pressValue(-1), tracking(true), blocktracking(false), pressed(false),
    invertedAppearance(false), invertedControls(false),
    orientation(Qt::Vertical), repeatAction(HbAbstractSliderControl::SliderNoAction)
{

}

/*!
   
 */
HbAbstractSliderControlPrivate::~HbAbstractSliderControlPrivate()
{
}

/*!
   
 */
void HbAbstractSliderControlPrivate::setSteps(int single, int page)
{
    Q_Q(HbAbstractSliderControl);
    singleStep = qAbs(single);
    pageStep = qAbs(page);
    q->sliderChange(HbAbstractSliderControl::SliderStepsChange);
}

/*!
    @beta
    Constructs an abstract slider with \a parent.

    The \l minimum defaults to \c 0, the \l maximum to \c 100, with a \l
    singleStep size of \c 1 and a \l pageStep size of \c 10, and an initial
    \l value of \c 0.
*/
HbAbstractSliderControl::HbAbstractSliderControl(QGraphicsItem *parent)
    : HbWidget(*new HbAbstractSliderControlPrivate, parent)
{
    Q_D(HbAbstractSliderControl);
    d->q_ptr = this;
}

/*!
    @beta
    Constructs an abstract slider with \a orientation and \a parent.

    The \l minimum defaults to \c 0, the \l maximum to \c 100, with a \l
    singleStep size of \c 1 and a \l pageStep size of \c 10, and an initial
    \l value of \c 0.
*/
HbAbstractSliderControl::HbAbstractSliderControl(Qt::Orientation orientation, QGraphicsItem *parent)
    : HbWidget(*new HbAbstractSliderControlPrivate, parent)
{
    Q_D(HbAbstractSliderControl);
    d->q_ptr = this;
    setOrientation(orientation);
}

/*!
    @beta
    \internal
 */
HbAbstractSliderControl::HbAbstractSliderControl(HbAbstractSliderControlPrivate &dd, QGraphicsItem *parent) :
    HbWidget(dd, parent)
{
}

/*!
    Destroys the slider.
*/
HbAbstractSliderControl::~HbAbstractSliderControl()
{
}

/*!
    @beta
    Sets the \a orientation.

    The default value is \c Qt::Vertical.

    \sa orientation()
 */
void HbAbstractSliderControl::setOrientation(Qt::Orientation orientation)
{
    Q_D(HbAbstractSliderControl);
    if (d->orientation == orientation)
        return;
    d->orientation = orientation;
    updateGeometry();
    sliderChange(SliderOrientationChange);
}

/*!
    @beta
    Returns the orientation.

    The default value is \c Qt::Vertical.

    \sa setOrientation()
 */
Qt::Orientation HbAbstractSliderControl::orientation() const
{
    Q_D(const HbAbstractSliderControl);
    return d->orientation;
}

/*!
    @beta
    Sets the minimum value of the slider.

    When setting this property, the \l maximum is adjusted if
    necessary to ensure that the range remains valid. Also the
    slider's current value is adjusted to be within the new range.

    \sa minimum()
*/
void HbAbstractSliderControl::setMinimum(int min)
{
    Q_D(HbAbstractSliderControl);
    setRange(min, qMax(d->maximum, min));
}

/*!
    @beta
    Returns the minimum value of the slider.

    The default value is \c 0.

    \sa setMinimum()
*/
int HbAbstractSliderControl::minimum() const
{
    Q_D(const HbAbstractSliderControl);
    return d->minimum;
}

/*!
    @beta
    Sets the maximum value of the slider.

    When setting this property, the \l minimum is adjusted if
    necessary to ensure that the range remains valid.  Also the
    slider's current value is adjusted to be within the new range.

    \sa maximum()
*/
void HbAbstractSliderControl::setMaximum(int max)
{
    Q_D(HbAbstractSliderControl);
    setRange(qMin(d->minimum, max), max);
}

/*!
    @beta
    Returns the maximum value of the slider.

    The default value is \c 100.

    \sa setMaximum()
*/
int HbAbstractSliderControl::maximum() const
{
    Q_D(const HbAbstractSliderControl);
    return d->maximum;
}

/*!
    @beta
    This function is provided for convenience. This function
    sets the slider's minimum to \a min and its maximum to \a max.

    If \a max is smaller than \a min, \a min becomes the only legal
    value.

    \sa minimum() maximum()
*/
void HbAbstractSliderControl::setRange(int min, int max)
{
    Q_D(HbAbstractSliderControl);
    int oldMin = d->minimum;
    int oldMax = d->maximum;
    d->minimum = min;
    d->maximum = qMax(min, max);
    if (oldMin != d->minimum || oldMax != d->maximum) {
        sliderChange(SliderRangeChange);
        emit rangeChanged(d->minimum, d->maximum);
        setValue(d->value); // re-bound
    }
}

/*!
    @beta
    Sets the single step of the slider.

    \sa singleStep()
*/
void HbAbstractSliderControl::setSingleStep(int step)
{
    Q_D(HbAbstractSliderControl);
    if (step != d->singleStep)
        d->setSteps(step, d->pageStep);
}

/*!
    @beta
    Returns the single step of the slider.

    Single step is the smaller of two natural steps that an
    abstract sliders provides and typically corresponds to the user
    pressing an arrow key.

    The default value is \c 1.

    \sa setSingleStep()
*/
int HbAbstractSliderControl::singleStep() const
{
    Q_D(const HbAbstractSliderControl);
    return d->singleStep;
}

/*!
    @beta
    Sets the page step of the slider.

    \sa pageStep()
*/
void HbAbstractSliderControl::setPageStep(int step)
{
    Q_D(HbAbstractSliderControl);
    if (step != d->pageStep)
        d->setSteps(d->singleStep, step);
}

/*!
    @beta
    Returns the page step of the slider.

    Page step is the larger of two natural steps that an abstract slider provides
    and typically corresponds to the user pressing PageUp or PageDown.

    The default value is \c 10.

    \sa setPageStep()
*/
int HbAbstractSliderControl::pageStep() const
{
    Q_D(const HbAbstractSliderControl);
    return d->pageStep;
}

/*!
    @beta
    Sets whether slider tracking is enabled.

    The default value is \c true.

    \sa hasTracking()
*/
void HbAbstractSliderControl::setTracking(bool enable)
{
    Q_D(HbAbstractSliderControl);
    d->tracking = enable;
}

/*!
    @beta
    Returns \c true whether slider tracking is enabled.

    If tracking is enabled (the default), the slider emits the
    valueChanged() signal while the slider is being dragged. If
    tracking is disabled, the slider emits the valueChanged() signal
    only when the user releases the slider.

    \sa setTracking()
*/
bool HbAbstractSliderControl::hasTracking() const
{
    Q_D(const HbAbstractSliderControl);
    return d->tracking;
}

/*!
    @beta
    Sets whether the slider is pressed down.

    The property is set by subclasses in order to let the abstract
    slider know whether or not \l tracking has any effect.

    Changing the slider down property emits the sliderPressed() and
    sliderReleased() signals.

    \sa isSliderDown()
*/
void HbAbstractSliderControl::setSliderDown(bool down)
{
    Q_D(HbAbstractSliderControl);
    bool doEmit = d->pressed != down;

    d->pressed = down;

    if (doEmit) {
        if (down) {
            emit sliderPressed();
        } else {
            emit sliderReleased();
        }
    }

    if (!down && d->position != d->value)
        triggerAction(SliderMove);
}

/*!
    @beta
    Returns \c true whether the slider is pressed down.

    \sa setSliderDown()
*/
bool HbAbstractSliderControl::isSliderDown() const
{
    Q_D(const HbAbstractSliderControl);
    return d->pressed;
}

/*!
    @beta
    Sets the current slider position.

    \sa sliderPosition()
*/
void HbAbstractSliderControl::setSliderPosition(int position)
{
    Q_D(HbAbstractSliderControl);
    position = d->bound(position);
    if (position == d->position)
        return;
    d->position = position;
    if (!d->tracking)
        update();
    if (d->pressed) {
        emit sliderMoved(position);
        if(singleStep() != 0 ) {
            HbWidgetFeedback::continuousTriggered(this, Hb::ContinuousDragged);
        }
    }
    if (d->tracking && !d->blocktracking)
        triggerAction(SliderMove);
}

/*!
    @beta
    Returns the current slider position.

    If \l tracking is enabled (the default), this is identical to \l value.

    \sa setSliderPosition()
*/
int HbAbstractSliderControl::sliderPosition() const
{
    Q_D(const HbAbstractSliderControl);
    return d->position;
}

/*!
    @beta
    Returns the slider's current value.

    The default value is \c 0.

    \sa setValue()
*/
int HbAbstractSliderControl::value() const
{
    Q_D(const HbAbstractSliderControl);
    return d->value;
}

/*!
    @beta
    Sets the slider's current value.

    The slider forces the value to be within the legal range: \l
    minimum <= \c value <= \l maximum.

    Changing the value also changes the \l sliderPosition.

    \sa value()
*/
void HbAbstractSliderControl::setValue(int value)
{
    Q_D(HbAbstractSliderControl);
    value = d->bound(value);
    if (d->value == value && d->position == value)
        return;
    d->value = value;
    if (d->position != value) {
        d->position = value;
        if (d->pressed)
            emit sliderMoved((d->position = value));
    }

    sliderChange(SliderValueChange);
    emit valueChanged(value);
}

/*!
    @beta
    Returns \c true whether the slider shows its values inverted.

    If this property is \c false (the default), the minimum and maximum will
    be shown in its classic position for the inherited widget. If the
    value is \c true, the minimum and maximum appear at their opposite location.

    \sa setInvertedAppearance()
*/
bool HbAbstractSliderControl::invertedAppearance() const
{
    Q_D(const HbAbstractSliderControl);
    return d->invertedAppearance;
}

/*!
    @beta
    Sets whether the slider shows its values inverted.

    \sa invertedAppearance()
*/
void HbAbstractSliderControl::setInvertedAppearance(bool invert)
{
    Q_D(HbAbstractSliderControl);
    d->invertedAppearance = invert;
    sliderChange(SliderAppearanceChange);
}

/*!
    @beta
    Returns \c true whether the slider inverts its wheel and key events.

    If this property is \c false (the default), scrolling the mouse wheel "up"
    and using keys like page up will increase the slider's value towards its
    maximum. Otherwise pressing page up will move value towards the slider's minimum.

    \sa setInvertedControls()
*/
bool HbAbstractSliderControl::invertedControls() const
{
    Q_D(const HbAbstractSliderControl);
    return d->invertedControls;
}

/*!
    @beta
    Sets  whether the slider inverts its wheel and key events.

    \sa invertedControls()
*/
void HbAbstractSliderControl::setInvertedControls(bool invert)
{
    Q_D(HbAbstractSliderControl);
    d->invertedControls = invert;
}

/*!
    @beta
    Triggers a slider \a action.  Possible actions are \l
    SliderSingleStepAdd, \l SliderSingleStepSub, \l SliderPageStepAdd,
    \l SliderPageStepSub, \l SliderToMinimum, \l SliderToMaximum, and \l
    SliderMove.

    \sa actionTriggered()
 */
void HbAbstractSliderControl::triggerAction(SliderAction action)
{
    Q_D(HbAbstractSliderControl);
    d->blocktracking = true;
    switch (action) {
    case SliderSingleStepAdd:
        setSliderPosition(d->overflowSafeAdd(d->singleStep));
        break;
    case SliderSingleStepSub:
        setSliderPosition(d->overflowSafeAdd(-d->singleStep));
        break;
    case SliderPageStepAdd:
        setSliderPosition(d->overflowSafeAdd(d->pageStep));
        break;
    case SliderPageStepSub:
        setSliderPosition(d->overflowSafeAdd(-d->pageStep));
        break;
    case SliderToMinimum:
        setSliderPosition(d->minimum);
        break;
    case SliderToMaximum:
        setSliderPosition(d->maximum);
        break;
    case SliderMove:
    case SliderNoAction:
        break;
    };
    emit actionTriggered(action);
    d->blocktracking = false;
    setValue(d->position);
}

/*!
    @beta
    Sets action \a action to be triggered repetitively in intervals
    of \a repeatTime, after an initial delay of \a thresholdTime.

    \sa triggerAction() repeatAction()
 */
void HbAbstractSliderControl::setRepeatAction(SliderAction action, int pressValue, int thresholdTime, int repeatTime)
{
    Q_D(HbAbstractSliderControl);
    d->pressValue = pressValue;
    if ((d->repeatAction = action) == SliderNoAction) {
        if (d->repeatActionTimer.isActive()) {
             HbWidgetFeedback::continuousStopped(this, Hb::ContinuousDragged);
        }
        d->repeatActionTimer.stop();
    } else {
        d->repeatActionTime = repeatTime;
        d->repeatActionTimer.start(thresholdTime, this);
    }
}

/*!
    @beta
    Returns the current repeat action.
    \sa setRepeatAction()
 */
HbAbstractSliderControl::SliderAction HbAbstractSliderControl::repeatAction() const
{
    Q_D(const HbAbstractSliderControl);
    return d->repeatAction;
}

/*!
    Reimplemented from QObject::timerEvent().
 */
void HbAbstractSliderControl::timerEvent(QTimerEvent *event)
{
    Q_D(HbAbstractSliderControl);
    if (event->timerId() == d->repeatActionTimer.timerId()) {
        if (d->repeatActionTime) { // was threshold time, use repeat time next time
            d->repeatActionTimer.start(d->repeatActionTime, this);
            d->repeatActionTime = 0;
        }
        switch (d->repeatAction) {
        case SliderPageStepAdd:
        case SliderPageStepSub:
            d->adjustSliderPosition(d->pageStep);
            if(singleStep() !=  0) {
                HbWidgetFeedback::continuousTriggered(this, Hb::ContinuousDragged);
            }
            break;

        case SliderSingleStepAdd:

        case SliderSingleStepSub:
            d->adjustSliderPosition(d->singleStep);
            if(singleStep() != 0) {
                HbWidgetFeedback::continuousTriggered(this, Hb::ContinuousDragged);
            }
            break;
        default:
            triggerAction(d->repeatAction);
            break;
        }
    }
}

/*!
    @beta
    Reimplement this virtual function to track slider changes such as
    \l SliderRangeChange, \l SliderOrientationChange, \l
    SliderStepsChange, or \l SliderValueChange. The default
    implementation only updates the display and ignores the \a change
    parameter.
 */
void HbAbstractSliderControl::sliderChange(SliderChange)
{
    update();
}

/*!
    Reimplemented from QGraphicsWidget::changeEvent().
 */
void HbAbstractSliderControl::changeEvent(QEvent *event)
{
    Q_D(HbAbstractSliderControl);
    switch (event->type()) {
    case QEvent::EnabledChange:
        if (!isEnabled()) {
            d->repeatActionTimer.stop();
            setSliderDown(false);
        }
        // fall through...
    default:
        HbWidget::changeEvent(event);
    }
}

/*!
    Reimplemented from QGraphicsItem::focusInEvent().
 */
void HbAbstractSliderControl::focusInEvent(QFocusEvent *event)
{
    Q_D(HbAbstractSliderControl);
    d->origValue = d->value;
    QGraphicsWidget::focusInEvent(event);
}

/*!
    Reimplemented from QGraphicsItem::keyPressEvent().
 */
void HbAbstractSliderControl::keyPressEvent(QKeyEvent *event)
{
    Q_D(HbAbstractSliderControl);
    SliderAction action = SliderNoAction;
    switch (event->key()) {

        // It seems we need to use invertedAppearance for Left and right, otherwise, things look weird.
        case Qt::Key_Left:
            if( d->keyNavigation() && orientation()==Qt::Horizontal){
                if (layoutDirection() == Qt::RightToLeft)
                    action = d->invertedAppearance ? SliderSingleStepSub : SliderSingleStepAdd;
                else
                    action = !d->invertedAppearance ? SliderSingleStepSub : SliderSingleStepAdd;
                event->accept();
            }
            //The slider widget will handle it
            else {
                 HbWidget::keyPressEvent(event);
            }
            break;
        case Qt::Key_Right:
            if( d->keyNavigation() && orientation()==Qt::Horizontal){
                if (layoutDirection() == Qt::RightToLeft)
                    action = d->invertedAppearance ? SliderSingleStepAdd : SliderSingleStepSub;
                else
                    action = !d->invertedAppearance ? SliderSingleStepAdd : SliderSingleStepSub;
                event->accept();
            }
            else{
                 HbWidget::keyPressEvent(event);
            }
            break;
        case Qt::Key_Up:
            if( d->keyNavigation() && orientation()==Qt::Vertical){
                action = d->invertedControls ? SliderSingleStepSub : SliderSingleStepAdd;
                event->accept();
            }
            else{
                 HbWidget::keyPressEvent(event);
            }
            break;
        case Qt::Key_Down:
            if( d->keyNavigation() && orientation()==Qt::Vertical){
                action = d->invertedControls ? SliderSingleStepAdd : SliderSingleStepSub;
                event->accept();
            }
            else {
                 HbWidget::keyPressEvent(event);
            }
           break;
        case Qt::Key_PageUp:
            action = d->invertedControls ? SliderPageStepSub : SliderPageStepAdd;
            event->accept();
            break;
        case Qt::Key_PageDown:
            action = d->invertedControls ? SliderPageStepAdd : SliderPageStepSub;
            event->accept();
            break;
        case Qt::Key_Home:
            action = SliderToMinimum;
            event->accept();
            break;
        case Qt::Key_End:
            action = SliderToMaximum;
            event->accept();
            break;
        default:
            HbWidget::keyPressEvent(event);
            break;
    }
    if (action)
        triggerAction(action);
   
    
}

/*! \fn int HbAbstractSliderControl::minValue() const

    Use minimum() instead.
*/

/*! \fn int HbAbstractSliderControl::maxValue() const

    Use maximum() instead.
*/

/*! \fn int HbAbstractSliderControl::lineStep() const

    Use singleStep() instead.
*/

/*! \fn void HbAbstractSliderControl::setMinValue(int v)

    Use setMinimum() instead.
*/

/*! \fn void HbAbstractSliderControl::setMaxValue(int v)

    Use setMaximum() instead.
*/

/*! \fn void HbAbstractSliderControl::setLineStep(int v)

    Use setSingleStep() instead.
*/

/*! \fn void HbAbstractSliderControl::addPage()

    Use triggerAction(HbAbstractSliderControl::SliderPageStepAdd) instead.
*/

/*! \fn void HbAbstractSliderControl::subtractPage()

    Use triggerAction(HbAbstractSliderControl::SliderPageStepSub) instead.
*/

/*! \fn void HbAbstractSliderControl::addLine()

    Use triggerAction(HbAbstractSliderControl::SliderSingleStepAdd) instead.
*/

/*! \fn void HbAbstractSliderControl::subtractLine()

    Use triggerAction(HbAbstractSliderControl::SliderSingleStepSub) instead.
*/

/*! \fn void HbAbstractSliderControl::setSteps(int single, int page)

    Use setSingleStep(\a single) followed by setPageStep(\a page)
    instead.
*/
