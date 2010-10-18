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

#include "hbinputbutton.h"

#include <QTime>
#include <hbdeviceprofile.h>

/*!
@stable
@hbinput
\class HbInputButton
\brief Provides vkb button definitions used by HbInputButtonGroup

\sa HbInputButtonGroup
*/

/// @cond

// Defines how much the button size should be increased from its original size.
const qreal HbInputThresholdMultiplier = 0.25;

// Threshold for touch point position comparison
const qreal HbInputTouchThreshold = 5.0;

// Timeout for increased touch down region
const int HbInputInstantTouchTimeout = 500;

// Threshold for increased touch down region in mm
const qreal HbInputInstantTouchThreshold = 3.5;

const QPoint HbInputInvalidPoint = QPoint(-1, -1);

class HbInputButtonPrivate
{
public:
    HbInputButtonPrivate();
    HbInputButtonPrivate(int keyCode, const QPoint &position, const QSize &size);
    HbInputButtonPrivate(HbInputButton::HbInputButtonType type, HbInputButton::HbInputButtonState state,
                         const QPoint &position, const QSize &size, int keyCode, bool autoRepeat,
                         const QList<QString> &texts, const QString &mappedCharacters, const QList<HbIcon> &icons);

    void setDefaultGraphics(int keyCode);

    HbInputButton::HbInputButtonType mType;
    HbInputButton::HbInputButtonState mState;
    HbInputButton::HbInputButtonState mPreviousState;
    QPoint mPosition;
    QSize mSize;
    int mKeyCode;
    bool mAutoRepeat;
    QList<QString> mTexts;
    QString mMappedCharacters;
    QList<HbIcon> mIcons;
    QRectF mBoundingRect;
    QPointF mTouchPoint;
    QPointF mLastInteractionPoint;
    bool mTap;
    QTime mTouchTime;
    qreal mTouchThreshold;
};

HbInputButtonPrivate::HbInputButtonPrivate()
    : mType(HbInputButton::ButtonTypeNormal), mState(HbInputButton::ButtonStateReleased),
      mPreviousState(HbInputButton::ButtonStateReleased), mPosition(0, 0), mSize(1, 1),
      mKeyCode(-1), mAutoRepeat(false), mTouchPoint(HbInputInvalidPoint),
      mLastInteractionPoint(HbInputInvalidPoint), mTap(false)
{
    for (int i = 0; i < HbInputButton::ButtonTextIndexCount; ++i) {
        mTexts.append("");
    }

    for (int i = 0; i < HbInputButton::ButtonIconIndexCount; ++i) {
        mIcons.append(HbIcon());
    }

    // Convert millimeters to pixels
    mTouchThreshold = HbDeviceProfile::current().ppmValue() * HbInputInstantTouchThreshold;
}

HbInputButtonPrivate::HbInputButtonPrivate(int keyCode, const QPoint &position, const QSize &size)
    : mType(HbInputButton::ButtonTypeNormal), mState(HbInputButton::ButtonStateReleased),
      mPreviousState(HbInputButton::ButtonStateReleased), mPosition(position),
      mSize(size), mKeyCode(keyCode), mAutoRepeat(false), mTouchPoint(HbInputInvalidPoint),
      mLastInteractionPoint(HbInputInvalidPoint), mTap(false)
{
    for (int i = 0; i < HbInputButton::ButtonTextIndexCount; ++i) {
        mTexts.append("");
    }

    for (int i = 0; i < HbInputButton::ButtonIconIndexCount; ++i) {
        mIcons.append(HbIcon());
    }

    if (keyCode != HbInputButton::ButtonKeyCodeCharacter &&
        keyCode != HbInputButton::ButtonKeyCodeSpace) {
        mType = HbInputButton::ButtonTypeFunction;
    }

    setDefaultGraphics(keyCode);

    if (mSize.width() < 1) {
        mSize.setWidth(1);
    }
    if (mSize.height() < 1) {
        mSize.setHeight(1);
    }

    if (mKeyCode == HbInputButton::ButtonKeyCodeDelete || mKeyCode == HbInputButton::ButtonKeyCodeSpace) {
        mAutoRepeat = true;
    }

    // Convert millimeters to pixels
    mTouchThreshold = HbDeviceProfile::current().ppmValue() * HbInputInstantTouchThreshold;
}

HbInputButtonPrivate::HbInputButtonPrivate(HbInputButton::HbInputButtonType type, HbInputButton::HbInputButtonState state,
        const QPoint &position, const QSize &size, int keyCode, bool autoRepeat,
        const QList<QString> &texts, const QString &mappedCharacters, const QList<HbIcon> &icons)
    : mType(type), mState(state), mPreviousState(state), mPosition(position), mSize(size),
      mKeyCode(keyCode), mAutoRepeat(autoRepeat), mMappedCharacters(mappedCharacters),
      mTouchPoint(HbInputInvalidPoint), mLastInteractionPoint(HbInputInvalidPoint), mTap(false)
{
    for (int i = 0; i < HbInputButton::ButtonTextIndexCount; ++i) {
        if (i < texts.count()) {
            mTexts.append(texts.at(i));
        } else {
            mTexts.append(QString());
        }
    }

    for (int i = 0; i < HbInputButton::ButtonIconIndexCount; ++i) {
        if (i < icons.count()) {
            mIcons.append(icons.at(i));
        } else {
            mIcons.append(HbIcon());
        }
    }

    if (mSize.width() < 1) {
        mSize.setWidth(1);
    }
    if (mSize.height() < 1) {
        mSize.setHeight(1);
    }

    // Convert millimeters to pixels
    mTouchThreshold = HbDeviceProfile::current().ppmValue() * HbInputInstantTouchThreshold;
}

void HbInputButtonPrivate::setDefaultGraphics(int keyCode)
{
    switch(keyCode) {
    case HbInputButton::ButtonKeyCodeDelete:
        mIcons.replace(HbInputButton::ButtonTextIndexPrimary, HbIcon(HbInputButtonIconDelete));
        break;
    case HbInputButton::ButtonKeyCodeShift:
        mIcons.replace(HbInputButton::ButtonTextIndexPrimary, HbIcon(HbInputButtonIconShift));
        break;
    case HbInputButton::ButtonKeyCodeSymbol:
        mIcons.replace(HbInputButton::ButtonTextIndexPrimary, HbIcon(HbInputButtonIconSymbol));
        break;
    case HbInputButton::ButtonKeyCodeEnter:
        mIcons.replace(HbInputButton::ButtonTextIndexPrimary, HbIcon(HbInputButtonIconEnter));
        break;
    case HbInputButton::ButtonKeyCodeSpace:
        mIcons.replace(HbInputButton::ButtonTextIndexPrimary, HbIcon(HbInputButtonIconSpace));
        break;
    case HbInputButton::ButtonKeyCodeAlphabet:
        mIcons.replace(HbInputButton::ButtonTextIndexPrimary, HbIcon(HbInputButtonIconSymbol));
        break;
    case HbInputButton::ButtonKeyCodeSmiley:
        mIcons.replace(HbInputButton::ButtonTextIndexPrimary, HbIcon(HbInputButtonIconSmiley));
        break;
    default:
        break;
    }
}

/// @endcond

/*!
    \enum HbInputButton::HbInputButtonKeyCode

    This enum defines a set of predefined key codes for input button. ButtonKeyCodeCustom should
    always be the last item so that all values bigger than ButtonKeyCodeCustom can be interpreted
    as custom key codes
 */

/*!
    \enum HbInputButton::HbInputButtonType

    This enum defines different button types. Button's graphics, text layout and functionality
    depend of the this type. ButtonTypeCount should always be the last value.
 */

/*!
    \enum HbInputButton::HbInputButtonState

    This enum defines button states. State mostly affects the button's visual look in addition
    to what can be done with the button (i.e. no interaction with disabled buttons, released and
    latched buttons can be pressed). ButtonStateCount should always be the last value.
 */

/*!
    \enum HbInputButton::HbInputButtonTextIndex

    This enum defines set of text indices that can be used when setting texts to different parts of
    a button using setText function. ButtonTextIndexCount should always be the last value.
 */

/*!
    \enum HbInputButton::HbInputButtonIconIndex

    This enum defines set of icon indices that can be used when setting icons to different parts of
    a button using setIcon function. ButtonIconIndexCount should always be the last value.
 */

/*!
Constructor
*/
HbInputButton::HbInputButton()
    : d_ptr(new HbInputButtonPrivate)
{
}

/*!
Constructor

keyCode should usually be one of HbInputButtonKeyCode values, but it can also be any other
integer value.
position is button's position in grid cell units.
size is button's size in grid cell units.
*/
HbInputButton::HbInputButton(int keyCode, const QPoint &position, const QSize &size)
    : d_ptr(new HbInputButtonPrivate(keyCode, position, size))
{
}

/*!
Destructor
*/
HbInputButton::~HbInputButton()
{
    delete d_ptr;
}

/*!
Updates button's type.

\sa type
*/
void HbInputButton::setType(HbInputButtonType type)
{
    Q_D(HbInputButton);

    d->mType = type;
}

/*!
Returns button's type.

\sa setType
*/
HbInputButton::HbInputButtonType HbInputButton::type() const
{
    Q_D(const HbInputButton);

    return d->mType;
}

/*!
Updates button's state.
State change sequence latched, pressed, released will result
in a latched state. Otherwise the new state will be the given state.

\sa state
*/
void HbInputButton::setState(HbInputButtonState state)
{
    Q_D(HbInputButton);

    if (state == ButtonStateReleased) {
        d->mTap = false;
        clearLastTriggeredPosition();
    }

    if (d->mState == ButtonStatePressed &&
        d->mPreviousState == ButtonStateLatched &&
        state == ButtonStateReleased) {
        d->mState = d->mPreviousState;
    } else {
        d->mPreviousState = d->mState;
        d->mState = state;
    }
}

/*!
Returns button's state.

\sa setState
*/
HbInputButton::HbInputButtonState HbInputButton::state() const
{
    Q_D(const HbInputButton);

    return d->mState;
}

/*!
Updates button's position.

position is button's position in grid cell units.

\sa position
*/
void HbInputButton::setPosition(const QPoint &position)
{
    Q_D(HbInputButton);

    d->mPosition = position;
}

/*!
Returns button's position.

\sa setPosition
*/
QPoint HbInputButton::position() const
{
    Q_D(const HbInputButton);

    return d->mPosition;
}

/*!
Updates button's size.

size is button's size in grid cell units.

\sa size
*/
void HbInputButton::setSize(const QSize &size)
{
    Q_D(HbInputButton);

    d->mSize = size;
}

/*!
Returns button's size.

\sa setSize
*/
QSize HbInputButton::size() const
{
    Q_D(const HbInputButton);

    return d->mSize;
}

/*!
Updates button's key code.

\sa keyCode
*/
void HbInputButton::setKeyCode(int keyCode)
{
    Q_D(HbInputButton);

    d->mKeyCode = keyCode;
}

/*!
Returns button's key code.

\sa setKeyCode
*/
int HbInputButton::keyCode() const
{
    Q_D(const HbInputButton);

    return d->mKeyCode;
}

/*!
Updates button's auto repeat status.

\sa autoRepeat
*/
void HbInputButton::setAutoRepeat(bool autoRepeat)
{
    Q_D(HbInputButton);

    d->mAutoRepeat = autoRepeat;
}

/*!
Returns button's auto repeat status.

\sa setAutoRepeat
*/
bool HbInputButton::autoRepeat() const
{
    Q_D(const HbInputButton);

    return d->mAutoRepeat;
}

/*!
Updates specified button text.

\sa setTexts
\sa text
\sa texts
*/
void HbInputButton::setText(const QString &text, HbInputButtonTextIndex index)
{
    Q_D(HbInputButton);

    if (index >= 0 && index < ButtonTextIndexCount) {
        d->mTexts.replace(index, text);
    }
}

/*!
Updates all button's texts.
Button can have three different texts. Text position and size
will depend on other buttons texts and icons. If list contains
more strings, then the rest will be ignored. Icon with same index
as text will override the text.

\sa text
\sa texts
*/
void HbInputButton::setTexts(const QList<QString> &texts)
{
    Q_D(HbInputButton);

    for (int i = 0; i < ButtonTextIndexCount; ++i) {
        if (i < texts.count()) {
            d->mTexts.replace(i, texts.at(i));
        } else {
            d->mTexts[i].clear();
        }
    }
}

/*!
Returns specified button text.

\sa setText
\sa setTexts
*/
QString HbInputButton::text(HbInputButtonTextIndex index) const
{
    Q_D(const HbInputButton);

    if (index < 0 || index >= ButtonTextIndexCount) {
        return QString();
    }

    return d->mTexts.at(index);
}

/*!
Returns all button's texts.

\sa setText
\sa setTexts
*/
QList<QString> HbInputButton::texts() const
{
    Q_D(const HbInputButton);

    return d->mTexts;
}

/*!
Updates characters that are mapped to this button.

\sa mappedCharacters
*/
void HbInputButton::setMappedCharacters(const QString &mappedCharacters)
{
    Q_D(HbInputButton);

    d->mMappedCharacters = mappedCharacters;
}

/*!
Returns characters that are mapped to this button.

\sa setMappedCharacters
*/
QString HbInputButton::mappedCharacters() const
{
    Q_D(const HbInputButton);

    return d->mMappedCharacters;
}

/*!
Updates specified button icon.

\sa setIcons
\sa icon
\sa icons
*/
void HbInputButton::setIcon(const HbIcon &icon, HbInputButtonIconIndex index)
{
    Q_D(HbInputButton);

    if (index >= 0 && index < ButtonIconIndexCount) {
        d->mIcons.replace(index, icon);
    }
}

/*!
Updates all button icons.
Button can have three different icons. Icon position
will depend on other buttons icons and texts. If list contains
more icons, then the rest will be ignored. Icon with same index
as text will override the text.

\sa icon
\sa icons
*/
void HbInputButton::setIcons(const QList<HbIcon> &icons)
{
    Q_D(HbInputButton);

    for (int i = 0; i < ButtonIconIndexCount; ++i) {
        if (i < icons.count()) {
            d->mIcons.replace(i, icons.at(i));
        } else {
            d->mIcons.replace(i, HbIcon());
        }
    }
}

/*!
Returns specified button icon.

\sa setIcon
\sa setIcons
*/
HbIcon HbInputButton::icon(HbInputButtonIconIndex index) const
{
    Q_D(const HbInputButton);

    if (index < 0 || index >= ButtonIconIndexCount) {
        return HbIcon();
    }
    return d->mIcons.at(index);
}

/*!
Returns all button's icons.

\sa setIcon
\sa setIcons
*/
QList<HbIcon> HbInputButton::icons() const
{
    Q_D(const HbInputButton);

    return d->mIcons;
}

/*!
Updates buttons bounding rectangle.

\sa boundingRect
*/
void HbInputButton::setBoundingRect(const QRectF &rect)
{
    Q_D(HbInputButton);

    d->mBoundingRect = rect;
}

/*!
Returns button's bounding rectangle.

\sa setBoundingRect
*/
QRectF HbInputButton::boundingRect() const
{
    Q_D(const HbInputButton);

    return d->mBoundingRect;
}

/*!
\brief Returns an active touch area for current button.

Button contains bounding rectangle for actual size of the item, while touch area
can actually extend over buttons physical boundaries. This is to make interacting
with button easier.

\return Active touch area as a rectangle, centered to its button.
\sa setBoundingRect
*/
QRectF HbInputButton::activeTouchArea() const
{
    Q_D(const HbInputButton);

    QRectF threshold(d->mBoundingRect);

    qreal mod_w = d->mBoundingRect.width() * HbInputThresholdMultiplier;
    qreal mod_h = d->mBoundingRect.height() * HbInputThresholdMultiplier;
    threshold.adjust(-mod_w, -mod_h, mod_w, mod_h);

    return threshold;
}

/*!
\brief Set position where last user interaction occurred.
*/
void HbInputButton::setLastTriggeredPosition(const QPointF &position)
{
    Q_D(HbInputButton);

    d->mLastInteractionPoint = position;
}

/*!
\brief Clear last user interaction data.
*/
void HbInputButton::clearLastTriggeredPosition()
{
    Q_D(HbInputButton);

    d->mLastInteractionPoint = HbInputInvalidPoint;
}

/*!
Checks whether the new touch point position is close enough to the previous
one that hit this button so that the new touch point can
be interpreted as the same as the old one.
*/
bool HbInputButton::wasTriggeredAt(const QPointF &position) const
{
    Q_D(const HbInputButton);

    QRectF rect(d->mLastInteractionPoint.x() - HbInputTouchThreshold, d->mLastInteractionPoint.y() - HbInputTouchThreshold,
                2 * HbInputTouchThreshold, 2 * HbInputTouchThreshold);

    return rect.contains(position);
}

/*!
\brief Set position where the button was first touched.
*/
void HbInputButton::setTouchPointPosition(const QPointF &position)
{
    Q_D(HbInputButton);

    d->mTouchPoint = position;
    d->mTap = true;
    d->mTouchTime.start();
}

/*!
Returns the current touch position which in case of
a short tap is the touch down position and otherwise the
last triggered position.
*/
QPointF HbInputButton::currentTouchPointPosition() const
{
    Q_D(const HbInputButton);

    if (d->mTap) {
        return d->mTouchPoint;
    }
    return d->mLastInteractionPoint;
}

/*!
\brief Checks whether move to given \a position should be ignored or processed normally

True is returned if given position is close enough to touch down point during
short timeout after the button has been pressed or after timeout if the
position is inside button's active touch area. Otherwise false is returned and
move event should be handled normally.
*/
bool HbInputButton::suppressMoveEvent(const QPointF &position)
{
    Q_D(HbInputButton);

    // Check if the timeout from the touch down event has passed
    if (d->mTap && d->mTouchTime.elapsed() < HbInputInstantTouchTimeout) {
        // Create rectangle around touch down position
        QRectF rect(d->mTouchPoint.x() - d->mTouchThreshold, d->mTouchPoint.y() - d->mTouchThreshold,
                    2 * d->mTouchThreshold, 2 * d->mTouchThreshold);

        // If inside created rectangle, update last position and ignore the event
        if (rect.contains(position)) {
            setLastTriggeredPosition(position);
            return true;
        }
    // Check if new position is still inside active button
    } else if (activeTouchArea().contains(position)) {
        setLastTriggeredPosition(position);
        d->mTap = false;
        return true;
    }

    d->mTap = false;
    return false;
}

// End of file
