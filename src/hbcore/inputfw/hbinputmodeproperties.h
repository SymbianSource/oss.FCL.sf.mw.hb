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

#include <hbinputdef.h>
#include <hbinputlanguage.h>

/*!
@alpha
@hbcore
\class HbInputModeProperties
\brief Binds together all the properties that define an input mode.

Input mode properties structure is needed when the freworks resolves which input method should
serve the active input state. This class is mostly used inside the input framework. The only exception is
HbInputMethod::listInputModes method, which is something each HbInputMethod instance must implement. 

\sa HbInputState
\sa HbInputMethod
*/
class HbInputModeProperties
{
public:
    HbInputModeProperties()
    {
    }

    HbInputModeProperties(HbInputModeType aMode, const HbInputLanguage &aLanguage, HbKeyboardType aKeyboard)
        : iMode(aMode), iLanguage(aLanguage), iKeyboard(aKeyboard)
    {
    }

    HbInputModeProperties& operator=(const HbInputModeProperties& aMode) {
        iMode = aMode.iMode;
        iLanguage = aMode.iLanguage;
        iKeyboard = aMode.iKeyboard;
        return *this;
    }

    bool operator==(const HbInputModeProperties& aMode) const {
        if (iMode == aMode.iMode
            && iLanguage == aMode.iLanguage
            && iKeyboard == aMode.iKeyboard) {
                return true;
        }
        return false;
    }

    bool operator!=(const HbInputModeProperties& aMode) const {
        if (iMode != aMode.iMode
            || iLanguage != aMode.iLanguage
            || iKeyboard != aMode.iKeyboard) {
                return true;
        }
        return false;
    }

    /*!
    Returns input mode.
    */
    HbInputModeType inputMode() const { return iMode; }

    /*!
    Sets input mode.
    */
    void setInputMode(HbInputModeType newInputMode) { iMode = newInputMode; }

    /*!
    Return language.
    */
    HbInputLanguage language() const { return HbInputLanguage(iLanguage); }

    /*!
    Sets language.
    */
    void setLanguage(const HbInputLanguage &newLanguage) { iLanguage = newLanguage; } 

    /*!
    Returns keyboard type.
    */
    HbKeyboardType keyboard() const { return iKeyboard; }

    /*!
    Sets keyboard type.
    */
    void setKeyboard(HbKeyboardType newKeyboard) { iKeyboard = newKeyboard; }

    /*!
    Returns mode properties in string format. This is used for input method resolving and
    only needed by input method developers.
    */
    QString asString() const {
        return iLanguage.asString() + QString(" ") + QString::number(iMode) + QString(" ") + QString::number(iKeyboard);
    }

public:
    HbInputModeType iMode;
    HbInputLanguage iLanguage;
    HbKeyboardType iKeyboard;
};

#endif // HB_INPUT_MODE_PROPERTIES_H

// End of file
