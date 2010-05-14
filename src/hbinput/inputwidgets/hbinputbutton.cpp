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

/// @cond

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
    QPoint mPosition;
    QSize mSize;
    int mKeyCode;
    bool mAutoRepeat;
    QList<QString> mTexts;
    QString mMappedCharacters;
    QList<HbIcon> mIcons;
    QRectF mBoundingRect;
};

HbInputButtonPrivate::HbInputButtonPrivate()
 : mType(HbInputButton::ButtonTypeNormal), mState(HbInputButton::ButtonStateReleased),
   mPosition(0, 0), mSize(1, 1), mKeyCode(-1), mAutoRepeat(false)
{
    for (int i = 0; i < HbInputButton::ButtonTextIndexCount; ++i) {
        mTexts.append("");
    }

    for (int i = 0; i < HbInputButton::ButtonIconIndexCount; ++i) {
        mIcons.append(HbIcon());
    }
}

HbInputButtonPrivate::HbInputButtonPrivate(int keyCode, const QPoint &position, const QSize &size)
 : mType(HbInputButton::ButtonTypeNormal), mState(HbInputButton::ButtonStateReleased),
   mPosition(position), mSize(size), mKeyCode(keyCode), mAutoRepeat(false)
{
    for (int i = 0; i < HbInputButton::ButtonTextIndexCount; ++i) {
        mTexts.append("");
    }

    for (int i = 0; i < HbInputButton::ButtonIconIndexCount; ++i) {
        mIcons.append(HbIcon());
    }

    if (keyCode != HbInputButton::ButtonKeyCodeCharacter) {
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
}

HbInputButtonPrivate::HbInputButtonPrivate(HbInputButton::HbInputButtonType type, HbInputButton::HbInputButtonState state,
                                           const QPoint &position, const QSize &size, int keyCode, bool autoRepeat,
                                           const QList<QString> &texts, const QString &mappedCharacters, const QList<HbIcon> &icons)
 : mType(type), mState(state), mPosition(position), mSize(size), mKeyCode(keyCode), mAutoRepeat(autoRepeat),
   mMappedCharacters(mappedCharacters)
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
}

void HbInputButtonPrivate::setDefaultGraphics(int keyCode)
{
    if (keyCode == HbInputButton::ButtonKeyCodeDelete) {
        mIcons.replace(HbInputButton::ButtonTextIndexPrimary, HbIcon(HbInputButtonIconDelete));
    } else if (keyCode == HbInputButton::ButtonKeyCodeShift) {
        mIcons.replace(HbInputButton::ButtonTextIndexPrimary, HbIcon(HbInputButtonIconShift));
    } else if (keyCode == HbInputButton::ButtonKeyCodeSymbol) {
        mIcons.replace(HbInputButton::ButtonTextIndexPrimary, HbIcon(HbInputButtonIconSymbol));
    } else if (keyCode == HbInputButton::ButtonKeyCodeEnter) {
        mIcons.replace(HbInputButton::ButtonTextIndexPrimary, HbIcon(HbInputButtonIconEnter));
    } else if (keyCode == HbInputButton::ButtonKeyCodeSpace) {
        mIcons.replace(HbInputButton::ButtonTextIndexPrimary, HbIcon(HbInputButtonIconSpace));
    } else if (keyCode == HbInputButton::ButtonKeyCodeAlphabet) {
        mIcons.replace(HbInputButton::ButtonTextIndexPrimary, HbIcon(HbInputButtonIconAlphabet));
    } else if (keyCode == HbInputButton::ButtonKeyCodePageChange) {
        mIcons.replace(HbInputButton::ButtonTextIndexPrimary, HbIcon(HbInputButtonIconPageChange));
    } else if (keyCode == HbInputButton::ButtonKeyCodeSmiley) {
        mIcons.replace(HbInputButton::ButtonTextIndexPrimary, HbIcon(HbInputButtonIconSmiley));
    }
}

/// @endcond

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
Updates buttons type.

\sa type
*/
void HbInputButton::setType(HbInputButtonType type)
{
    Q_D(HbInputButton);

    d->mType = type;
}

/*!
Returns buttons type.

\sa setType
*/
HbInputButton::HbInputButtonType HbInputButton::type() const
{
    Q_D(const HbInputButton);

    return d->mType;
}

/*!
Updates buttons state.

\sa state
*/
void HbInputButton::setState(HbInputButtonState state)
{
    Q_D(HbInputButton);

    d->mState = state;
}

/*!
Returns buttons state.

\sa setState
*/
HbInputButton::HbInputButtonState HbInputButton::state() const
{
    Q_D(const HbInputButton);

    return d->mState;
}

/*!
Updates buttons position.

position is button's position in grid cell units.

\sa position
*/
void HbInputButton::setPosition(const QPoint &position)
{
    Q_D(HbInputButton);

    d->mPosition = position;
}

/*!
Returns buttons position.

\sa setPosition
*/
QPoint HbInputButton::position() const
{
    Q_D(const HbInputButton);

    return d->mPosition;
}

/*!
Updates buttons size.

size is button's size in grid cell units.

\sa size
*/
void HbInputButton::setSize(const QSize &size)
{
    Q_D(HbInputButton);

    d->mSize = size;
}

/*!
Returns buttons size.

\sa setSize
*/
QSize HbInputButton::size() const
{
    Q_D(const HbInputButton);

    return d->mSize;
}

/*!
Updates buttons key code.

\sa keyCode
*/
void HbInputButton::setKeyCode(int keyCode)
{
    Q_D(HbInputButton);

    d->mKeyCode = keyCode;
}

/*!
Returns buttons key code.

\sa setKeyCode
*/
int HbInputButton::keyCode() const
{
    Q_D(const HbInputButton);

    return d->mKeyCode;
}

/*!
Updates buttons auto repeat status.

\sa autoRepeat
*/
void HbInputButton::setAutoRepeat(bool autoRepeat)
{
    Q_D(HbInputButton);

    d->mAutoRepeat = autoRepeat;
}

/*!
Returns buttons auto repeat status.

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
Updates all button texts.
Button can have three different texts. Text position and size
will depend of other buttons texts and icons. If list contains
more strings, then the rest will be ignored. Icon with same index
than text will override the text.

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
            d->mTexts.replace(i, QString());
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
Returns all button texts.

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
will depend of other buttons icons and texts. If list contains
more icons, then the rest will be ignored. Icon with same index
than text will override the text.

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
Returns all button icon.

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
Returns buttons bounding rectangle.

\sa setBoundingRect
*/
QRectF HbInputButton::boundingRect() const
{
    Q_D(const HbInputButton);

    return d->mBoundingRect;
}

// End of file
