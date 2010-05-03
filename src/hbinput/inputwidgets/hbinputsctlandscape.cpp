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

#include <QGraphicsGridLayout>
#include <QSignalMapper>
#include <QKeyEvent>
#include <math.h>

#include <hbinstance.h>
#include <hbinputmethod.h>
#include <hbinputkeymap.h>
#include <hbinputsettingproxy.h>
#include <hbabstractedit.h>

#include "hbinputsctlandscape.h"
#include "hbinputtouchkeypadbutton.h"
#include "hbinputvkbwidget.h"
#include "hbinputcharpreviewpane.h"

// private includes
#include "hbinputsctlandscape_p.h"
#include "hbinputvkbwidget_p.h"

/*!
    @proto
    @hbinput
    \class HbInputSctLandscape
    \deprecated class HbInputSctLandscape
    \brief A widget for displaying special character table in landscape mode.
    
    This widget displays special character table. Characters are organized in grid
    format and there is also separate are for displaying most frquently used special
    characters. The widget inherits from touch keypad base class. When a character
    is selected it will emit signal sctCharacterSelected.
    
    \sa HbInputVkbWidget
*/

/// @cond

const int HbSctNumberOfColumns = 10;
const int HbSctNumberOfRows = 4;
const int HbNumberOfSctButtons = 39;
// this includes space and other buttons which are not going to be part of the special characters
const int HbNumberOfOtherButtons = 7;
const qreal HbSctButtonPreferredHeight = 52.5;
const QSizeF HbSctInitialDimensions(64.0, HbSctButtonPreferredHeight);

const QString HbSctLandscapeButtonTextLayout = "_hb_sctl_button_text_layout";
const QString HbSctLandscapeButtonIconLayout = "_hb_sctl_button_icon_layout";

const int HbSmileyRangeButton = Qt::Key_F1;
const int HbSpecialCharacterRangeButton = Qt::Key_F2;
const QString HbSmileyButtonObjName = "SCT smiley";
const int HbSmileyButtonIndex = 9;
struct HbVirtualSctKey
{
    int mKey;
    int mRow;
    int mColumn;
    int mRowSpan;
    int mColumnSpan;
};

// this is a template keymapping table which will be used to 
// layout buttons. Qt::Key_Question represnts a button which can 
// be mapped to a special character / Smiley.
HbVirtualSctKey sctVkbTable[] =
{
    // first row
    {Qt::Key_Question,0,0,1,1 },
    {Qt::Key_Question,0,1,1,1 },
    {Qt::Key_Question,0,2,1,1 },
    {Qt::Key_Question,0,3,1,1 },
    {Qt::Key_Question,0,4,1,1 },
    {Qt::Key_Question,0,5,1,1 },
    {Qt::Key_Question,0,6,1,1},
    {Qt::Key_Question,0,7,1,1},
    {Qt::Key_Question,0,8,1,1},
    // Smiley button
    {Qt::Key_F1,0,9,1,1},

    // seKey_Questioncond row
    {Qt::Key_Question,1,0,1,1 },
    {Qt::Key_Question,1,1,1,1 },
    {Qt::Key_Question,1,2,1,1 },
    {Qt::Key_Question,1,3,1,1 },
    {Qt::Key_Question,1,4,1,1 },
    {Qt::Key_Question,1,5,1,1 },
    {Qt::Key_Question,1,6,1,1 },
    {Qt::Key_Question ,1,7,1,1 },
    {Qt::Key_Question,1,8,1,1 },
    {Qt::Key_Backspace, 1, 9, 1, 1 },

    // third row
    {Qt::Key_Question,2,0,1,1 },
    {Qt::Key_Question,2,1,1,1 },
    {Qt::Key_Question,2,2,1,1 },
    {Qt::Key_Question,2,3,1,1 },
    {Qt::Key_Question,2,4,1,1 },
    {Qt::Key_Question,2,5,1,1 },
    {Qt::Key_Question,2,6,1,1 },
    {Qt::Key_Question,2,7,1,1 },
    {Qt::Key_Question,2,8,1,1 },
    {Qt::Key_Enter, 2, 9, 1, 1 },

    // fourth row
    // Character range button
    {Qt::Key_F2, 3, 0, 1, 1 },
    {Qt::Key_Control, 3, 1, 1, 1 },
    {Qt::Key_Question,3,2,1,1 },
    {Qt::Key_Question,3,3,1,1 },
    {Qt::Key_Space, 3, 4, 1, 2 },
    {Qt::Key_Question,3,6,1,1 },
    {Qt::Key_Question,3,7,1,1 },
    {Qt::Key_Question,3,8,1,1 },
    // application button
    {Qt::Key_F3,3,9,1,1 }
};

HbInputSctLandscapePrivate::HbInputSctLandscapePrivate()
:mStartIndex(0),
mCurrentPage(0),
mActiveView(HbInputSctLandscape::HbSctViewSpecialCharacter),
mPreviewPane(0),
mClickMapper(0),
mSize(QSizeF())
{
    mFlickAnimation = true;
}

/*
This function sets a keypad button as a function button with given parameters.
*/
void HbInputSctLandscapePrivate::setAsFunctionButton(int index, const HbIcon &icon, const QString &text)
{
    mSctButtons.at(index)->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
    mSctButtons.at(index)->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
    mSctButtons.at(index)->setIcon(icon);
    mSctButtons.at(index)->setText(text);
    mSctButtons.at(index)->setAsStickyButton(false);
}

/*
This function latches keypad button with given index. Since there could be only one
latch button. We need to reset others.
*/
void HbInputSctLandscapePrivate::latchRangeButton(int buttonId)
{
    // we need to latch only one out of available range buttons.
    for (int i = HbNumberOfSctButtons - 2; ; i--) {
        // iterate till we get a character key. since character should not be latched.
        if (mSctButtons.at(i)->type() == Hb::ItemType_InputFunctionButton) {
            mSctButtons.at(i)->setLatch(buttonId == sctVkbTable[i].mKey);
        } else {
            break;
        }
    }
}

/*
This function defines the layout porperties for sct.
*/
void HbInputSctLandscapePrivate::createSctButtons()
{
    Q_Q(HbInputSctLandscape);

    q->setupToolCluster();

    for (int i = 0; i < HbNumberOfSctButtons-1; i++) {
        HbTouchKeypadButton *button = new HbTouchKeypadButton(q, QString(""), q);
        q->connect(button, SIGNAL(pressed()), mPressMapper, SLOT(map()));
        mPressMapper->setMapping(button, i);
        q->connect(button, SIGNAL(released()), mReleaseMapper, SLOT(map()));
        mReleaseMapper->setMapping(button, i);
        q->connect(button, SIGNAL(clicked()), mClickMapper, SLOT(map()));
        mClickMapper->setMapping(button, i);

        mSctButtons.append(button);
        mButtonLayout->addItem(button, sctVkbTable[i].mRow, sctVkbTable[i].mColumn, 
            sctVkbTable[i].mRowSpan, sctVkbTable[i].mColumnSpan);
        if (sctVkbTable[i].mKey != Qt::Key_Question) {
            button->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
            button->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
        } else {
            button->setAsStickyButton(true);
            q->connect(button, SIGNAL(enteredInNonStickyRegion()), q, SLOT(_q_enteredInNonStickyRegion()));
            button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbSctLandscapeButtonTextLayout);
        }
        HbIcon icon;
        switch (sctVkbTable[i].mKey) {
            case Qt::Key_Backspace:
                icon.setIconName("qtg_mono_backspace1");
                button->setIcon( icon );
                button->setAutoRepeatDelay(HbRepeatTimeout);
                button->setAutoRepeatInterval(HbRepeatTimeoutShort);
                button->setAutoRepeat(true);
                button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbSctLandscapeButtonIconLayout);
                break;
            case Qt::Key_Enter:
                icon.setIconName("qtg_mono_enter");
                button->setIcon( icon );
                button->setAutoRepeatDelay(HbRepeatTimeout);
                button->setAutoRepeatInterval(HbRepeatTimeoutShort);
                button->setAutoRepeat(true);
                button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbSctLandscapeButtonIconLayout);
                break;
            case Qt::Key_Space:
                button->setFrameIcon("qtg_mono_space");
                button->setAutoRepeatDelay(HbRepeatTimeout);
                button->setAutoRepeatInterval(HbRepeatTimeoutShort);
                button->setAutoRepeat(true);
                break;
            case Qt::Key_Shift:
                icon.setIconName("qtg_mono_shift");
                button->setIcon(icon);
                button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbSctLandscapeButtonIconLayout);
                break;
            case Qt::Key_Control:
                icon.setIconName("qtg_mono_alpha_mode");
                button->setIcon(icon);
                button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbSctLandscapeButtonIconLayout);
                break;
            case Qt::Key_F1:
                button->setIcon(HbIcon("qtg_mono_smiley"));
                button->setObjectName(HbSmileyButtonObjName);
                button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbSctLandscapeButtonIconLayout);
                break;
            case Qt::Key_F2:
                button->setIcon(HbIcon("qtg_mono_special_characters_qwerty"));
                button->setObjectName(HbSmileyButtonObjName);
                button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbSctLandscapeButtonIconLayout);
                break;
            default:
                break;
        }
    }

    mSctButtons.append(mApplicationButton);
    mButtonLayout->addItem(mApplicationButton,
                           sctVkbTable[HbNumberOfSctButtons-1].mRow,
                           sctVkbTable[HbNumberOfSctButtons-1].mColumn,
                           sctVkbTable[HbNumberOfSctButtons-1].mRowSpan,
                           sctVkbTable[HbNumberOfSctButtons-1].mColumnSpan);
}

/*
This function defines the layout porperties for sct.
*/
void HbInputSctLandscapePrivate::setLayoutDimensions(QSizeF dimensions)
{
    // only update the dimensions if they are not previously set
    if (mSize == dimensions) {
        return;
    }
    mSize = dimensions;

    mButtonLayout->setContentsMargins(0.0, 0.0, 0.0, 0.0);

    for (int i = 0; i < HbSctNumberOfColumns; i++) {
        mButtonLayout->setColumnFixedWidth(i, dimensions.width());
    }

    for (int i = 0; i < HbSctNumberOfRows; i++) {
        mButtonLayout->setRowFixedHeight(i, dimensions.height());
    }

    mButtonLayout->setHorizontalSpacing(0.0);
    mButtonLayout->setVerticalSpacing(0.0);
    foreach (HbTouchKeypadButton* button, mSctButtons) {
        if (button) {
            button->setInitialSize(dimensions);
        }
    }
}

/*
Sets the sct button. Once sct character buttons are set it latches the active range 
button.
*/
void HbInputSctLandscapePrivate::setActiveView(HbInputVkbWidget::HbSctView view)
{
    Q_Q(HbInputSctLandscape);
    mActiveView = view;

    switch (mActiveView) {
        case HbInputSctLandscape::HbSctViewSpecialCharacter:
            setSctButtons(mSpecialCharacterSet);
            latchRangeButton(HbSpecialCharacterRangeButton);
            break;
        case HbInputSctLandscape::HbSctViewSmiley:
            q->showSmileyPicker(HbSctNumberOfRows, HbSctNumberOfColumns);
            break;
        default:
            break;
    };
}

/*
apply editor constraints on buttons
*/
void HbInputSctLandscapePrivate::applyEditorConstraints()
{
    HbInputFocusObject *focusedObject = 0;
    if (mOwner) {
        focusedObject = mOwner->focusObject();
    }

    if(!focusedObject || isKeyboardDimmed()) {
    // dont need to apply constraints when keypad is dimmed.
    // applyEditorConstraints will be called from setKeyboardDimmed(false)
        return;
    }

    for (int i=0; i < mSctButtons.size(); i++) {
        if (sctVkbTable[i].mKey == Qt::Key_Question || sctVkbTable[i].mKey == Qt::Key_Space) {
            QString buttonText = (sctVkbTable[i].mKey == Qt::Key_Space) ? QString(" ") : mSctButtons.at(i)->text();
            if (buttonText.isEmpty() || !focusedObject->characterAllowedInEditor(buttonText[0])) {
                // if the data mapped to button is empty or the data mapped is not allowed to the editor 
                mSctButtons.at(i)->setFade(true);
            } else {
                mSctButtons.at(i)->setFade(false);
            }
        }
    }

    // now set latch buttons
    switch (mActiveView) {
        case HbInputSctLandscape::HbSctViewSpecialCharacter:
            latchRangeButton(HbSpecialCharacterRangeButton);
            break;
        case HbInputSctLandscape::HbSctViewSmiley:
            latchRangeButton(HbSmileyRangeButton);
            break;
        default:
            break;
    };

    // we should disable smiley range button in case we have editor which is url/email/password etc editor.
    mSctButtons.at(HbSmileyButtonIndex)->setFade(focusedObject->editorInterface().editorClass() != HbInputEditorClassUnknown
        ||!isSmileysEnabled());
}

/*
Sets passed characters on sct buttons.
*/
void HbInputSctLandscapePrivate::setSctButtons(const QString& aCharSet)
{
    int i = 0;
    int j = 0;

    for (; i < mSctButtons.size() && (j + mStartIndex) < aCharSet.size(); i++) {
        // we need to map only on a character type button.
        if (sctVkbTable[i].mKey == Qt::Key_Question || sctVkbTable[i].mKey == Qt::Key_Space) {
            const QChar &character = (sctVkbTable[i].mKey == Qt::Key_Space) ? ' ' : aCharSet[(j++) + mStartIndex];
            mSctButtons.at(i)->setIcon(HbIcon());
            mSctButtons.at(i)->setText(character);
        }
    }

    for (; i < mSctButtons.size(); i++) {
        if (sctVkbTable[i].mKey == Qt::Key_Question) {
            mSctButtons.at(i)->setText(QString(""));
        }
    }

    mCurrentPage = mStartIndex/(HbNumberOfSctButtons-HbNumberOfOtherButtons);
    mStartIndex += j;
    if (mStartIndex == aCharSet.size()) {
        // We have reached end of special character list, reset the mStartIndex to 0
        // so that we show first set of special characters next time
        mStartIndex = 0;
    }
    applyEditorConstraints();
}

/*
Gets the special character sets from set keymapping.
*/
void HbInputSctLandscapePrivate::getSpecialCharacters()
{
    mSpecialCharacterSet.clear();
    if (mKeymap) {
        const HbKeyboardMap* keymap = mKeymap->keyboard(HbKeyboardSctLandscape);
        if (keymap == 0) {
            return;
        }
        foreach (const HbMappedKey* mappedKey, keymap->keys) {
            mSpecialCharacterSet.append(mappedKey->characters(HbModifierNone));
        }
    }
}

/*
This function returns the keyCode for the given index.
*/
int HbInputSctLandscapePrivate::keyCode(int buttonId)
{
    return sctVkbTable[buttonId].mKey;
}

/*
Handles button press events.
*/
void HbInputSctLandscapePrivate::handleStandardButtonPress(int buttonId)
{
    // A new button is pressed so we should close 
    // preview pane on the previous button.
    if (mPreviewPane->isVisible()) {
        mPreviewPane->hide();
    }

    if (buttonId < 0) {
        return;
    }

    // if the button is not faded, then show character preview popup.
    if ((HbInputSettingProxy::instance()->isCharacterPreviewForQwertyEnabled()) && !(mSctButtons.at(buttonId)->isFaded())) {
        if (sctVkbTable[buttonId].mKey == Qt::Key_Question) {
            const QString &text = mSctButtons.at(buttonId)->text();
            if (text.size()) {
                QStringList list(text);
                mPreviewPane->showCharacters(list, mSctButtons.at(buttonId)->sceneBoundingRect());
                return;
            }
        }
    }

}

/*
Handles button clicks.
*/
void HbInputSctLandscapePrivate::handleStandardButtonClick(int buttonId)
{
    Q_Q(HbInputSctLandscape);

    switch (sctVkbTable[buttonId].mKey) {
    case Qt::Key_Question: {
            QString buttonText = mSctButtons.at(buttonId)->text();
            if (buttonText.length() > 0) {
                emit q->sctCharacterSelected(buttonText.at(0));
            }
            break;
        }
    case HbSpecialCharacterRangeButton:
        if(mActiveView != HbInputVkbWidget::HbSctViewSpecialCharacter) {
            //first time coming to special character view.
            mStartIndex = 0;
        }
        setActiveView(HbInputSctLandscape::HbSctViewSpecialCharacter);
        break;
    case HbSmileyRangeButton:
        if(mActiveView != HbInputVkbWidget::HbSctViewSmiley) {
            //first time coming to special character view.
            mStartIndex = 0;
        }
        // dont show the smiley picker if the button is inactive
        if (!mSctButtons[HbSmileyButtonIndex]->isFaded()) {
            setActiveView(HbInputSctLandscape::HbSctViewSmiley);
        }
        break;
    default:
        // left are enter, backspace and space buttons. they should be handled by plugins.
        // we should pass both the press and release event. As mode handlers work according to
        // the press and release event.
        QKeyEvent pressEvent(QEvent::KeyPress, sctVkbTable[buttonId].mKey, Qt::NoModifier);
        if (mOwner) {
            mOwner->filterEvent(&pressEvent);
            QKeyEvent releaseEvent(QEvent::KeyRelease, sctVkbTable[buttonId].mKey, Qt::NoModifier);
            mOwner->filterEvent(&releaseEvent);
        }
    };
}

/*
Handles the sct keypad button releas. Internally it hides character preview pane
if visible.
*/
void HbInputSctLandscapePrivate::handleStandardButtonRelease(int buttonId)
{
    Q_UNUSED(buttonId);
    if (mPreviewPane->isVisible()) {
        mPreviewPane->hide();
    }
}

/*!
This slot is called when we slide our fingures on top of the keypad buttons and 
while sliding our fingure comes on top of a non sticky button Or on a region outside
the keypad area.
*/
void HbInputSctLandscapePrivate::_q_enteredInNonStickyRegion()
{
    if (mPreviewPane->isVisible()) {
        mPreviewPane->hide();
    }
}

/*!
Handles virtual key clicks
*/
void HbInputSctLandscapePrivate::_q_mappedKeyClick(int buttonid)
{
    handleStandardButtonClick(buttonid);
}

/// @endcond

/*!
\deprecated HbInputSctLandscape::HbInputSctLandscape(HbInputMethod*, const HbKeymap*, QGraphicsItem*)
    is deprecated.
*/
HbInputSctLandscape::HbInputSctLandscape(HbInputMethod* owner, const HbKeymap *keymap, QGraphicsItem* parent)
                    : HbInputVkbWidget(*new HbInputSctLandscapePrivate, parent)
{
    Q_D(HbInputSctLandscape);
    d->q_ptr = this;
    d->mOwner = owner;

    d->mButtonLayout = new QGraphicsGridLayout();

    d->mClickMapper = new QSignalMapper(this);

    // create buttons.
    d->createSctButtons();

    // preview pane
    d->mPreviewPane = new HbCharPreviewPane();
    d->mPreviewPane->hide();

    // connect mappers.
    connect(d->mPressMapper, SIGNAL(mapped(int)), this, SLOT(mappedKeyPress(int)));
    connect(d->mReleaseMapper, SIGNAL(mapped(int)), this, SLOT(mappedKeyRelease(int)));
    connect(d->mClickMapper, SIGNAL(mapped(int)), this, SLOT(_q_mappedKeyClick(int)));

    connect(this, SIGNAL(flickEvent(HbInputVkbWidget::HbFlickDirection)), this, SLOT(flickTriggered(HbInputVkbWidget::HbFlickDirection)));

    // now set the keymap data.
    setKeymap(keymap);
}

/*!
\deprecated HbInputSctLandscape::HbInputSctLandscape(HbInputSctLandscapePrivate&, QGraphicsItem*)
    is deprecated.
*/
HbInputSctLandscape::HbInputSctLandscape(HbInputSctLandscapePrivate &dd, QGraphicsItem* parent)
            : HbInputVkbWidget(dd, parent)
{
}

/*!
\deprecated HbInputSctLandscape::~HbInputSctLandscape()
    is deprecated.
*/
HbInputSctLandscape::~HbInputSctLandscape()
{
}

/*!
\reimp
\deprecated HbInputSctLandscape::keyboardType() const
    is deprecated.
*/
HbKeyboardType HbInputSctLandscape::keyboardType() const
{
    return HbKeyboardSctLandscape;
}

/*!
\deprecated HbInputSctLandscape::setSct(HbSctView, bool)
    is deprecated.
*/
void HbInputSctLandscape::setSct(HbSctView view , bool enableMostUsedCharacterPane)
{
    // for the time being disabling 
    // most used character pane
    Q_UNUSED(enableMostUsedCharacterPane);
    Q_D(HbInputSctLandscape);

    d->mStartIndex  = 0;
    setupToolCluster();
    d->mStartIndex = 0;
    d->setActiveView(view);
}

/*!
\reimp
\deprecated HbInputSctLandscape::setKeymap(const HbKeymap*)
    is deprecated.
*/
void HbInputSctLandscape::setKeymap(const HbKeymap* keymap)
{
    Q_D(HbInputSctLandscape);
    HbInputVkbWidget::setKeymap(keymap);
    d->getSpecialCharacters();
}

/*!
\reimp
\deprecated HbInputSctLandscape::keypadLayout()
    is deprecated.
*/
QGraphicsLayout *HbInputSctLandscape::keypadLayout()
{
    Q_D(HbInputSctLandscape);
    return d->mButtonLayout;
}

/*!
\reimp
\deprecated HbInputSctLandscape::aboutToOpen(HbVkbHost*)
    is deprecated.
*/
void HbInputSctLandscape::aboutToOpen(HbVkbHost *host)
{
    Q_D(HbInputSctLandscape);
    HbInputVkbWidget::aboutToOpen(host);

    // calculate each button width and height
    QSizeF keypadSize = keypadButtonAreaSize();

    keypadSize.setWidth(keypadSize.width() / (qreal)HbSctNumberOfColumns);
    keypadSize.setHeight(keypadSize.height() / (qreal)HbSctNumberOfRows);

    d->setLayoutDimensions(keypadSize);
}

/*!
\reimp
\deprecated HbInputSctLandscape::aboutToClose(HbVkbHost*)
    is deprecated.
*/
void HbInputSctLandscape::aboutToClose(HbVkbHost *host)
{
    Q_D(HbInputSctLandscape);
    HbInputVkbWidget::aboutToClose(host);
    if (d->mPreviewPane->isVisible()) {
        d->mPreviewPane->hide();
    }
}

/*!
\deprecated HbInputSctLandscape::flickTriggered(HbInputVkbWidget::HbFlickDirection)
    is deprecated.
*/
void HbInputSctLandscape::flickTriggered(HbInputVkbWidget::HbFlickDirection direction)
{
    Q_D(HbInputSctLandscape);

    // left/right flick event has occured, hence hide the preview pane
    if (d->mPreviewPane->isVisible()) {
        d->mPreviewPane->hide();
    }

    // total number of actual buttons available for displaying special characters
    int iNumSctButtons = HbNumberOfSctButtons-HbNumberOfOtherButtons;
    if(direction == HbInputVkbWidget::HbFlickDirectionLeft) {
        d->mCurrentPage--;
        if(d->mCurrentPage<0) {
            if (d->mSpecialCharacterSet.size()) {
                d->mCurrentPage = (int)ceil((float)d->mSpecialCharacterSet.size()/iNumSctButtons)-1;
            } else {
                d->mCurrentPage = 0;
            }
        }
        d->mStartIndex = d->mCurrentPage*iNumSctButtons;
    }
    d->setActiveView(HbInputSctLandscape::HbSctViewSpecialCharacter);
}

#include "moc_hbinputsctlandscape.cpp"
// End of file
