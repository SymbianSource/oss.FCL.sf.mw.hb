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

#ifndef HB_INPUT_STATE_H
#define HB_INPUT_STATE_H

#include <hbinputlanguage.h>

/*!
@alpha
@hbcore
\class HbInputState
\brief Holds all the information for defining an input state.

This class describes the state of the input framework.
Active input method is notified every time the state changes.
If the active input method cannot handle new state, the framework will find 
suitable handler for it. The input state is a combination of input mode, text case,
keyboard type and language.

This class is for input method developers, it should never be needed in application code.

\sa HbInputMethod
*/
class HbInputState
{
public:
    HbInputState()
        : mModeType(HbInputModeNone),
          mTextCase(HbTextCaseNone),
          mKeyboardType(HbKeyboardNone),
          mLanguage(HbInputLanguage())
    {}

    HbInputState(HbInputModeType modeType, HbTextCase textCase, HbKeyboardType keyboardType, const HbInputLanguage &language = HbInputLanguage())
        : mModeType(modeType),
          mTextCase(textCase),
          mKeyboardType(keyboardType),
          mLanguage(language)
    {}

    void operator=(const HbInputState& other) {
        mModeType = other.mModeType;
        mTextCase = other.mTextCase;
        mKeyboardType = other.mKeyboardType;
        mLanguage = other.mLanguage;
    }

    bool operator==(const HbInputState& other) {
        if (mModeType == other.mModeType
            && mTextCase == other.mTextCase
            && mKeyboardType == other.mKeyboardType
            && mLanguage == other.mLanguage) {
                return true;
        }
        return false;
    }

    /*!
    This is same as compare operator except for the language value. If either one of the
    states being compared has undefined language value, it will match to any language.
    If both language values are defined, then they are compared directly.
    */
    bool isMatch(const HbInputState& other) {
        if (mModeType == other.mModeType
            && mTextCase == other.mTextCase
            && mKeyboardType == other.mKeyboardType
            && (mLanguage == other.mLanguage ||
                mLanguage.undefined() ||           // Undefined matches to anything.
        other.mLanguage.undefined())) {
                return true;
        }
        return false;
    }

    bool operator!=(const HbInputState& other) {
        if (mModeType != other.mModeType
            || mTextCase != other.mTextCase
            || mKeyboardType != other.mKeyboardType
            || mLanguage != other.mLanguage) {
                return true;
        }
        return false;
    }

    /*!
    Returns input mode.
    */
    HbInputModeType inputMode() const { return mModeType; }

    /*!
    Sets input mode.
    */
    void setInputMode(HbInputModeType newMode) { mModeType = newMode; }

    /*!
    Returns text case.
    */
    HbTextCase textCase() const { return mTextCase; }

    /*!
    Sets text case.
    */
    void setTextCase(HbTextCase newCase) { mTextCase = newCase; }

    /*!
    Returns keyboard type.
    */
    HbKeyboardType keyboard() const { return mKeyboardType; }

    /*!
    Sets keyboard type.
    */
    void setKeyboard(HbKeyboardType newKeyboard) { mKeyboardType = newKeyboard; }

    /*!
    Returns language.
    */
    HbInputLanguage language() const { return HbInputLanguage(mLanguage); }

    /*!
    Sets language. 
    */
    void setLanguage(const HbInputLanguage &newLanguage) { mLanguage = newLanguage; }

private:
    HbInputModeType mModeType;
    HbTextCase mTextCase;
    HbKeyboardType mKeyboardType;
    HbInputLanguage mLanguage;
};

#endif // HB_INPUT_STATE_H

// End of file
