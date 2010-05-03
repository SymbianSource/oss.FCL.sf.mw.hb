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

#include <hbapplication.h>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QVector>
#include <QSignalMapper>
#include <QKeyEvent>
#include <QPointer>
#include <math.h>

#include <hbinstance.h>
#include <hbinputmethod.h>
#include <hbinputkeymap.h>
#include <hbinputvkbhost.h>
#include <hbinputsettingproxy.h>
#include <hbabstractedit.h>
#include "hbinputsctportrait.h"
#include "hbinputtouchkeypadbutton.h"
#include "hbinputvkbwidget.h"

#include "hbinputsctportrait_p.h"
#include "hbinputvkbwidget_p.h"

/*!
    @proto
    @hbinput
    \class HbInputSctPortrait
    \deprecated class HbInputSctPortrait
    \brief A widget for displaying special character table in portrait mode.
    
    This widget displays special character table. Characters are organized in grid
    format. The widget inherits from touch keypad base class. When a character
    is selected it will emit signal sctCharacterSelected.
    
    \sa HbInputVkbWidget
    \sa HbInputTopSctLine
*/

/// @cond

const int HbSctGridColumns = 5;
const int HbSctGridRows = 5;
const int HbNumSctButtons = HbSctGridColumns*HbSctGridRows;
const qreal HbSctButtonPreferredHeight = 56.0;
const QSizeF HbSctInitialDimensions(90.0, HbSctButtonPreferredHeight);

const int HbDelButtonId = HbSctGridColumns-1;
const int HbAbcButtonId = 2*HbSctGridColumns-1;
const int HbSpecialCharacterButtonId = 3*HbSctGridColumns-1;
const int HbSmileyButtonId = 4*HbSctGridColumns-1;

const QString HbDelButtonObjName = "SCT delete";
const QString HbAbcButtonObjName = "SCT abc";
const QString HbSpecialCharacterButtonObjName = "SCT special character";
const QString HbSmileyButtonObjName = "SCT smiley";
const QString HbCustomButtonObjName = "SCT custom button ";

const QString HbSctPortraitButtonTextLayout = "_hb_sctp_button_text_layout";
const QString HbSctPortraitButtonIconLayout = "_hb_sctp_button_icon_layout";

HbInputSctPortraitPrivate::HbInputSctPortraitPrivate()
 : mActiveView(HbInputSctPortrait::HbSctViewSpecialCharacter),
    mClickMapper(0),
    mStartIndex(0),
    mCurrentPage(0),
    mSize(QSizeF())
{
    mFlickAnimation = true;
}

/*
This function defines the layout porperties for sct.
*/
void HbInputSctPortraitPrivate::createSctButtons()
{
    Q_Q(HbInputSctPortrait);

    q->setupToolCluster();

    if (mSctButtons.size() == 0) {
        for (int i = 0; i < HbNumSctButtons-1; ++i) {
            HbTouchKeypadButton *button = new HbTouchKeypadButton(q, QString(""), q);
            q->connect(button, SIGNAL(pressed()),mPressMapper, SLOT(map()));
            mPressMapper->setMapping(button, i);
            q->connect(button, SIGNAL(released()),mReleaseMapper, SLOT(map()));
            mReleaseMapper->setMapping(button, i);
            q->connect(button, SIGNAL(clicked()), mClickMapper, SLOT(map()));
            mClickMapper->setMapping(button, i);
            mSctButtons.append(button);
            button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbSctPortraitButtonTextLayout);
        }

        mSctButtons.append(mApplicationButton);

        for (int i = 0; i < HbNumSctButtons; ++i) {
            mButtonLayout->addItem(mSctButtons.at(i), i/HbSctGridColumns, i%HbSctGridColumns);
        }
    }
}

/*
This function defines the layout porperties for sct.
*/
void HbInputSctPortraitPrivate::setLayoutDimensions(QSizeF dimensions)
{
    // only update the dimensions if they are not previously set
    if (mSize == dimensions) {
        return;
    }
    mSize = dimensions;

    mButtonLayout->setContentsMargins(0.0, 0.0, 0.0, 0.0);

    for (int i = 0; i < HbSctGridColumns; i++) {
        mButtonLayout->setColumnFixedWidth(i, dimensions.width());
    }
    for (int i = 0; i < HbSctGridRows; i++) {
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


void HbInputSctPortraitPrivate::initialize()
{
    mSctButtons.at(HbDelButtonId)->setText("");
    mSctButtons.at(HbDelButtonId)->setIcon(HbIcon("qtg_mono_backspace2"));
    mSctButtons.at(HbDelButtonId)->setObjectName(HbDelButtonObjName);
    mSctButtons.at(HbDelButtonId)->setAutoRepeatDelay(HbRepeatTimeout);
    mSctButtons.at(HbDelButtonId)->setAutoRepeatInterval(HbRepeatTimeoutShort);
    mSctButtons.at(HbDelButtonId)->setAutoRepeat(true);
    mSctButtons.at(HbDelButtonId)->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbSctPortraitButtonIconLayout);

    mSctButtons.at(HbAbcButtonId)->setIcon(HbIcon("qtg_mono_alpha_mode"));
    mSctButtons.at(HbAbcButtonId)->setObjectName(HbAbcButtonObjName); 
    mSctButtons.at(HbAbcButtonId)->setObjectName(HbAbcButtonObjName); 
    mSctButtons.at(HbAbcButtonId)->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbSctPortraitButtonIconLayout);

    mSctButtons.at(HbSpecialCharacterButtonId)->setIcon(HbIcon("qtg_mono_special_characters_itut"));
    mSctButtons.at(HbSpecialCharacterButtonId)->setObjectName(HbSpecialCharacterButtonObjName); 
    mSctButtons.at(HbSpecialCharacterButtonId)->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbSctPortraitButtonIconLayout);

    mSctButtons.at(HbSmileyButtonId)->setIcon(HbIcon("qtg_mono_smiley"));
    mSctButtons.at(HbSmileyButtonId)->setObjectName(HbSmileyButtonObjName); 
    mSctButtons.at(HbSmileyButtonId)->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbSctPortraitButtonIconLayout);

    if (mApplicationButton) {
        mApplicationButton->setObjectName(HbCustomButtonObjName + QString::number(1));
    }

    for (int i = HbSctGridColumns-1; i < HbNumSctButtons-1; i+=HbSctGridColumns) {
        mSctButtons.at(i)->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
        mSctButtons.at(i)->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
    }
}

/*
apply editor constraints on buttons
*/
void HbInputSctPortraitPrivate::applyEditorConstraints()
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

    for (int i=0; i < mSctButtons.size()-1; ++i) {
        if (i%HbSctGridColumns != HbSctGridColumns-1) {
            QString buttonText = mSctButtons.at(i)->text();
            if (buttonText.isEmpty() || !focusedObject->characterAllowedInEditor(buttonText[0])) {
                // if the data mapped to button is empty or the data mapped is not allowed to the editor 
                mSctButtons.at(i)->setFade(true);
            } else {
                mSctButtons.at(i)->setFade(false);
            }
        }
    }

    mSctButtons.at(HbSmileyButtonId)->setFade(focusedObject->editorInterface().isNumericEditor()
        || !focusedObject->editorInterface().editorClass() == HbInputEditorClassUnknown
        || !isSmileysEnabled());
}

void HbInputSctPortraitPrivate::setSctButtons(const QString &aCharSet)
{
    Q_Q(HbInputSctPortrait);
    q->setupToolCluster();

    int i = 0;
    int j = 0;
    for (; i < mSctButtons.size()-1 && (j+mStartIndex) < aCharSet.size(); ++i) {
        if (i%HbSctGridColumns != HbSctGridColumns-1) {
            const QChar &character = aCharSet[j+mStartIndex];
            mSctButtons.at(i)->setText(character);
            mSctButtons.at(i)->setObjectName("Sct portrait " + QString(character));
            j++;
        }
    }

    for (; i < mSctButtons.size()-1; ++i) {
        if (i%HbSctGridColumns != HbSctGridColumns-1) {
            mSctButtons.at(i)->setText("");
        }
    }

    mCurrentPage = mStartIndex/(HbNumSctButtons-HbSctGridRows);
    mStartIndex += j;
    if (mStartIndex == aCharSet.size()) {
        // We have reached end of special character list, reset the mStartIndex to 0
        // so that we show first set of special characters next time
        mStartIndex = 0;
    }
    applyEditorConstraints();
}


void HbInputSctPortraitPrivate::setActiveView(HbInputVkbWidget::HbSctView view)
{
    Q_Q(HbInputSctPortrait);
    mActiveView = view;

    switch (view) {
    case HbInputSctPortrait::HbSctViewSpecialCharacter:
        setSctButtons(mSpecialCharacterSet);
        mSctButtons.at(HbSpecialCharacterButtonId)->setLatch(true);
        mSctButtons.at(HbSmileyButtonId)->setLatch(false);
        break;

    case HbInputSctPortrait::HbSctViewSmiley:
        q->showSmileyPicker(HbSctGridRows, HbSctGridColumns);
        break;

    default:
        break;
    };
}

/*
Gets the special character sets from set keymapping.
*/
void HbInputSctPortraitPrivate::getSpecialCharacters()
{
    mSpecialCharacterSet.clear();
    if (mKeymap) {
        const HbKeyboardMap* keyboardMap = mKeymap->keyboard(HbKeyboardSctPortrait);
        if (keyboardMap) {
            foreach (const HbMappedKey* mappedKey, keyboardMap->keys) {
                mSpecialCharacterSet.append(mappedKey->characters(HbModifierNone));
            }
        }
    }
}



int HbInputSctPortraitPrivate::keyCode(int buttonId)
{
    int code = 0;
    if (buttonId == HbDelButtonId) {
        code = Qt::Key_Delete;
    } else if (buttonId == HbAbcButtonId) {
		code = Qt::Key_Control;
    } else if (buttonId == HbSpecialCharacterButtonId) {
        code = Qt::Key_F1;
    } else if (buttonId == HbSmileyButtonId) {
        code = Qt::Key_F2;
    }
    return code;
}

/*!

*/
void HbInputSctPortraitPrivate::handleStandardButtonPress(int buttonId)
{
    Q_UNUSED(buttonId);
    //dont need to do anything here
   
}

/*
Handles button clicks.
*/
void HbInputSctPortraitPrivate::handleStandardButtonClick(int buttonId)
{
    Q_Q(HbInputSctPortrait);

    if (buttonId >= 0 && buttonId < HbNumSctButtons &&
		buttonId%HbSctGridColumns != HbSctGridColumns-1) {
			QString buttonText = mSctButtons.at(buttonId)->text();
			if (mSctButtons.at(buttonId) && !mSctButtons.at(buttonId)->isFaded()) {
				if (buttonText.length() > 0) {
					emit q->sctCharacterSelected(buttonText.at(0));
				}
			}
	} else if (keyCode(buttonId) == Qt::Key_F1) {
        if(mActiveView != HbInputVkbWidget::HbSctViewSpecialCharacter) {
            mStartIndex = 0;
        }
        setActiveView(HbInputVkbWidget::HbSctViewSpecialCharacter);
    } else if (keyCode(buttonId) == Qt::Key_F2) {
        if(mActiveView != HbInputVkbWidget::HbSctViewSmiley) {
            mStartIndex = 0;
        }
        // dont show the smiley picker, if the button is inactive
        if (!mSctButtons.at(HbSmileyButtonId)->isFaded()) {
            setActiveView(HbInputSctPortrait::HbSctViewSmiley);
        }
    } else {
        // we should pass both the press and release event. As mode handlers work according to
        // the press and release event.
        QKeyEvent pressEvent(QEvent::KeyPress, keyCode(buttonId), Qt::NoModifier);
        if (mOwner) {
            mOwner->filterEvent(&pressEvent);
            QKeyEvent releaseEvent(QEvent::KeyRelease, keyCode(buttonId), Qt::NoModifier);
            mOwner->filterEvent(&releaseEvent);
        }
    }
}

/*
Handles the sct keypad button releas. Internally it hides character preview pane
if visible.
*/
void HbInputSctPortraitPrivate::handleStandardButtonRelease(int buttonId)
{
    Q_UNUSED(buttonId);
    //dont need to do anything here
}

/*!
Handles virtual key clicks
*/
void HbInputSctPortraitPrivate::_q_mappedKeyClick(int buttonid)
{
    handleStandardButtonClick(buttonid);
}
/// @endcond

/*!
\deprecated HbInputSctPortrait::HbInputSctPortrait(HbInputMethod*, const HbKeymap *, QGraphicsItem*)
    is deprecated.
*/
HbInputSctPortrait::HbInputSctPortrait(HbInputMethod* owner, const HbKeymap *keymap, QGraphicsItem* parent)
                    : HbInputVkbWidget(*new HbInputSctPortraitPrivate, parent)
{
    Q_D(HbInputSctPortrait);
    d->q_ptr = this;
    d->mOwner = owner;

    d->mButtonLayout = new QGraphicsGridLayout();
    d->mButtonLayout->setSpacing(0.0);
    d->mButtonLayout->setContentsMargins(0.0, 0.0, 0.0, 0.0); 

    d->mClickMapper = new QSignalMapper(this);

    // create buttons.
    d->createSctButtons();

    // connect mappers.
    connect(d->mPressMapper, SIGNAL(mapped(int)), this, SLOT(mappedKeyPress(int)));
    connect(d->mReleaseMapper, SIGNAL(mapped(int)), this, SLOT(mappedKeyRelease(int)));
    connect(d->mClickMapper, SIGNAL(mapped(int)), this, SLOT(_q_mappedKeyClick(int)));

    connect(this, SIGNAL(flickEvent(HbInputVkbWidget::HbFlickDirection)), this, SLOT(flickTriggered(HbInputVkbWidget::HbFlickDirection)));

    // now set the keymap data.
    setKeymap(keymap);
}

/*!
\deprecated HbInputSctPortrait::HbInputSctPortrait(HbInputSctPortraitPrivate &, QGraphicsItem*)
    is deprecated.
*/
HbInputSctPortrait::HbInputSctPortrait(HbInputSctPortraitPrivate &dd, QGraphicsItem* parent)
    : HbInputVkbWidget(dd, parent)
{
}

/*!
\deprecated HbInputSctPortrait::~HbInputSctPortrait()
    is deprecated.
*/
HbInputSctPortrait::~HbInputSctPortrait()
{
}

/*!
\deprecated HbInputSctPortrait::keyboardType() const
    is deprecated.
*/
HbKeyboardType HbInputSctPortrait::keyboardType() const
{
    return HbKeyboardSctPortrait;
}

/*!
\deprecated HbInputSctPortrait::setSct(HbSctView)
    is deprecated.
*/
void HbInputSctPortrait::setSct(HbSctView view)
{
    Q_D(HbInputSctPortrait);

    d->initialize();

    d->mStartIndex = 0;
    d->setActiveView(view);
}

/*!
\deprecated HbInputSctPortrait::setKeymap(const HbKeymap*)
    is deprecated.
*/
void HbInputSctPortrait::setKeymap(const HbKeymap* keymap)
{
    Q_D(HbInputSctPortrait);
    HbInputVkbWidget::setKeymap(keymap);
    d->getSpecialCharacters();
}

/*!
\deprecated HbInputSctPortrait::keypadLayout()
    is deprecated.
*/
QGraphicsLayout *HbInputSctPortrait::keypadLayout()
{
    Q_D(HbInputSctPortrait);
    return d->mButtonLayout;
}

/*!
\deprecated HbInputSctPortrait::aboutToOpen(HbVkbHost*)
    is deprecated.
*/
void HbInputSctPortrait::aboutToOpen(HbVkbHost *host)
{
    Q_D(HbInputSctPortrait);

    HbInputVkbWidget::aboutToOpen(host);

    QSizeF keypadSize = keypadButtonAreaSize();
    keypadSize.setWidth(keypadSize.width() / (qreal)HbSctGridColumns);
    keypadSize.setHeight(keypadSize.height() / (qreal)HbSctGridRows);
    d->setLayoutDimensions(keypadSize);
}

/*!
\deprecated HbInputSctPortrait::flickTriggered(HbInputVkbWidget::HbFlickDirection)
    is deprecated.
*/
void HbInputSctPortrait::flickTriggered(HbInputVkbWidget::HbFlickDirection direction)
{
    Q_D(HbInputSctPortrait);

    d->initialize();
    int iNumSctButtons = HbNumSctButtons - HbSctGridRows;
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
    d->setActiveView(HbInputSctPortrait::HbSctViewSpecialCharacter);
}

#include "moc_hbinputsctportrait.cpp"

// End of file
