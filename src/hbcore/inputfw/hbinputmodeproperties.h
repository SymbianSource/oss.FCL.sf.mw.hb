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

#ifndef HB_INPUT_MODE_PROPERTIES_H
#define HB_INPUT_MODE_PROPERTIES_H

#include <QStringList>

#include <hbinputdef.h>
#include <hbinputlanguage.h>
#include <hbinputstate.h>

/*!
@alpha
@hbcore
\class HbInputModeProperties
\brief Binds together all the properties that define an input mode.

This class is needed when the framework resolves input state handler. An input method
plugin reports a set of implemented input modes as an array of HbInputModeProperties
converted to strings.

This class is not needed in application code.

\sa HbInputState
\sa HbInputMethod
*/
class HbInputModeProperties
{
public:
    HbInputModeProperties() {
    }

    HbInputModeProperties(HbInputModeType mode, const HbInputLanguage &language, HbKeyboardType keyboard)
        : mMode(mode), mLanguage(language), mKeyboard(keyboard) {
    }

    HbInputModeProperties(const HbInputState &state)
        : mMode(state.inputMode()), mLanguage(state.language()), mKeyboard(state.keyboard()) {
    }

    HbInputModeProperties &operator=(const HbInputModeProperties &other) {
        mMode = other.mMode;
        mLanguage = other.mLanguage;
        mKeyboard = other.mKeyboard;
        return *this;
    }

    bool operator==(const HbInputModeProperties &other) const {
        if (mMode == other.mMode
            && mLanguage == other.mLanguage
            && mKeyboard == other.mKeyboard) {
            return true;
        }
        return false;
    }

    bool operator!=(const HbInputModeProperties &other) const {
        if (mMode != other.mMode
            || mLanguage != other.mLanguage
            || mKeyboard != other.mKeyboard) {
            return true;
        }
        return false;
    }

    /*!
    Returns input mode.
    */
    HbInputModeType inputMode() const {
        return mMode;
    }

    /*!
    Sets input mode.
    */
    void setInputMode(HbInputModeType newInputMode) {
        mMode = newInputMode;
    }

    /*!
    Return language.
    */
    HbInputLanguage language() const {
        return HbInputLanguage(mLanguage);
    }

    /*!
    Sets language.
    */
    void setLanguage(const HbInputLanguage &newLanguage) {
        mLanguage = newLanguage;
    }

    /*!
    Returns keyboard type.
    */
    HbKeyboardType keyboard() const {
        return mKeyboard;
    }

    /*!
    Sets keyboard type.
    */
    void setKeyboard(HbKeyboardType newKeyboard) {
        mKeyboard = newKeyboard;
    }

    /*!
    Returns mode properties in string format. This is used for input method resolving and
    only needed by input method developers.
    */
    QString asString() const {
        return mLanguage.asString() + QString(' ') + QString::number(mMode) + QString(' ') + QString::number(mKeyboard);
    }

    /*!
    Creates property object from a string generated with asString.
    */
    static HbInputModeProperties fromString(const QString& string) {
        HbInputModeProperties result;

        // See asString() for order, amount and type of string parts
        QStringList parts = string.split(' ');
        if (parts.count() == 4) {
            QString languageStr = parts[0] + QString(' ') + parts[1];
            HbInputLanguage language;
            language.fromString(languageStr);
            result.setLanguage(language);
            result.setInputMode((HbInputModeType)parts[2].toLong());
            result.setKeyboard((HbKeyboardType)parts[3].toLong());
        }

        return result;
    }

private:
    HbInputModeType mMode;
    HbInputLanguage mLanguage;
    HbKeyboardType mKeyboard;
};

#endif // HB_INPUT_MODE_PROPERTIES_H

// End of file
