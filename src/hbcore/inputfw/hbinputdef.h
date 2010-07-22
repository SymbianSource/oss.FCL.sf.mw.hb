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

#ifndef HB_INPUT_DEF_H
#define HB_INPUT_DEF_H

#include <QFlags>
#include <QString>
#include <hbglobal.h>

enum HbKeyboardMasks {
    HbQwertyKeyboardMask     = 0x00010000,
    HbTouchInputMask         = 0x00020000,
    HbHwrKeyboardMask        = 0x00040000
};

enum HbInputModeMasks {
    HbChineseModeMask  = 0x00010000,
    HbIndicModeMask    = 0x00020000,
    HbHwrModeMask      = 0x00040000,
    HbNumericModeMask  = 0x00080000
};

/*!
\struct HbKeyPressProbability

Abstract data type for representing an ambiguous key input.
keycode represents the key being hit and probability represents the
probability of the key being hit.
*/
struct HbKeyPressProbability {
    int keycode;
    qreal probability;
};

/*!
\enum HbKeyboardType

Enumerates known keyboard types. Specific bits are reserved
for keyboard classes. For example all, qwerty based
keyboards have bit HbQwertyKeyboardMask set and all touch keypads have
bit HbTouchInputMask set.
*/
enum HbKeyboardTypeFlag {
    HbKeyboardNone                          = 0x00000000,

    HbKeyboard12Key                         = 0x00000001,       /**< Deprecated, use HbKeyboardHardwarePortrait instead. */
    HbKeyboardHardwarePortrait                          = HbKeyboard12Key,  /**< Hardware keyboard for portrait orientation. */
    HbKeyboardQwerty                        = 0x00000001 | HbQwertyKeyboardMask,  /**< Deprecated, use HbKeyboardHardwareLandcape instead */
    HbKeyboardHardwareLandcape                          = HbKeyboardQwerty, /**< Hardware keyboard for landscape mode. */
    HbKeyboardVirtual12Key                  = 0x00000001 | HbTouchInputMask,    /**< Deprecated, use HbKeyboardTouchPortrait instead. */
    HbKeyboardTouchPortrait                             = HbKeyboardVirtual12Key,           /**< Touch keyboard for portrait mode. */
    HbKeyboardVirtualQwerty                 = 0x00000002 | HbTouchInputMask | HbQwertyKeyboardMask, /**< Touchscreen version of qwerty keyboard */
    HbKeyboardTouchLandscape                            = HbKeyboardVirtualQwerty, /**< Touch keyboard for landscape mode. */
    HbKeyboardSctPortrait                   = 0x00000003 | HbTouchInputMask,                       /**< Special character selection keypad for portrait view */
    HbKeyboardSctLandscape                  = 0x00000004 | HbTouchInputMask | HbQwertyKeyboardMask, /**< Special character selection keypad for landscape view */
    HbKeyboardHwr                           = 0x00000005 | HbTouchInputMask | HbHwrKeyboardMask,   /**< Hand writing recognition keypad */
    HbKeyboardVirtual12KeyLabels            = 0x00000006 | HbTouchInputMask                        /**< Key labels for virtual 12 key keyboard */
};

Q_DECLARE_FLAGS(HbKeyboardType, HbKeyboardTypeFlag)

/*!
\enum HbInputModeType

Enumerates known input modes. Specific bits are reserved
for classifying different input mode types. For example, all Chinese modes
have bit HbChineseModeMask set and all Japanese modes have
bit HbJapaneseModeMask set. HbInputModeCustom is a special case.
That input mode never appears in standard input mode set, but it must
be activated separately from UI.
*/
enum HbInputModeTypeFlag {
    HbInputModeNone                    = 0x00000000,

    // Default mode
    HbInputModeDefault                 = 0x00000001,                      /**< Default input mode. This may mean diffierent things for different languages. */

    // Numeric input mode
    HbInputModeNumeric                 = 0x00000001 | HbNumericModeMask,  /**< Numeric input mode */

    // Special modes for all languages
    HbInputModeSpeechToText            = 0x00000500,                      /**< Speech-to-text input mode */

    // Chinese modes
    HbInputModePinyin                  = 0x00000001 | HbChineseModeMask,  /**< Chinese PinYin input */
    HbInputModeStroke                  = 0x00000002 | HbChineseModeMask,  /**< Chinese Stroke input */
    HbInputModeZhuyin                  = 0x00000003 | HbChineseModeMask,  /**< Chinese ZhuYin input */
    HbInputModePinyinPhrase            = 0x00000004 | HbChineseModeMask,  /**< Chinese PinYin input (phrase version) */
    HbInputModeStrokePhrase            = 0x00000005 | HbChineseModeMask,  /**< Chinese Stroke input (phrase version) */
    HbInputModeZhuyinPhrase            = 0x00000006 | HbChineseModeMask,  /**< Chinese ZhuYin input (phrase version) */
    HbInputModeCangjieNormal           = 0x00000007 | HbChineseModeMask,  /**< Chinese CangJie input, normal mode */
    HbInputModeCangjieEasy             = 0x00000008 | HbChineseModeMask,  /**< Chinese CangJie input, easy mode */
    HbInputModeCangjieAdvanced         = 0x00000009 | HbChineseModeMask,  /**< Chinese CangJie input, advanced mode */

    // Indic modes
    HbInputModeIndicPhonetic           = 0x00000001 | HbIndicModeMask,     /**< Indic phonetic input mode */

    // Handwriting modes
    HbInputModeHwrLatin                = 0x00000001 | HbHwrModeMask,                       /**< Latin based hand writing recognition */
    HbInputModeHwrLatinFull            = 0x00000002 | HbHwrModeMask,                       /**< Latin based hand writing recognition, full screen */
    HbInputModeHwrChinese              = 0x00000003 | HbHwrModeMask | HbChineseModeMask,   /**< Chinese hand writing recognition */
    HbInputModeHwrChineseFull          = 0x00000004 | HbHwrModeMask | HbChineseModeMask,   /**< Chinese hand writing recognition, full screen */

    // Custom made mode that does not belong to any pre-existing category
    HbInputModeCustom                  = HbChineseModeMask - 1             /**< Custom input mode */
};
Q_DECLARE_FLAGS(HbInputModeType, HbInputModeTypeFlag)

/*!
\enum HbModifiers

Enumerates supported key event modifiers. There are modifiers for
shift, fn and chr keys. If a modifier bit is set, then the corresponding key
is pressed.
*/
enum HbModifier {
    HbModifierNone         = 0x00000000,
    HbModifierShiftPressed = 0x00000001,     /**< Shift-key is pressed */
    HbModifierChrPressed   = 0x00000002,     /**< Chr-key is pressed */
    HbModifierFnPressed    = 0x00000004      /**< Fn-key is pressed */
};
Q_DECLARE_FLAGS(HbModifiers, HbModifier)

/*!
\enum HbKeyboardSettingFlags

Enumerates keyboard setting flags.
*/
enum HbKeyboardSettingFlag {
    HbKeyboardSettingNone         = 0x00000000,
    HbKeyboardSetting12key        = 0x00000001,
    HbKeyboardSettingQwerty       = 0x00000010
};
Q_DECLARE_FLAGS(HbKeyboardSettingFlags, HbKeyboardSettingFlag)

/*!
\enum HbTextCase

Enumerates supported text case states. HbTextCaseAutomatic is a state
where the framework automatically tries to conclude when upper case
state is needed.
*/
enum HbTextCase {
    HbTextCaseNone,
    HbTextCaseLower,             /**< Lower text case. */
    HbTextCaseUpper,             /**< Upper text case. */
    HbTextCaseAutomatic          /**< Automatic capital letters. */
};

/*!
\enum HbFnState

Enumerates possible states for Fn key.
*/
enum HbFnState {
    HbFnOff,    /**< Fn key inactive. */
    HbFnNext,   /**< Fn key active for the next keypress. */
    HbFnOn      /**< Fn key locked as active. */
};

/*!
\enum HbEditorConstraint

Enumerates editor constraints. This is a bit vector. Editors
may declare a set of constraints that control its behaviour. Constraint
bits are set through editor interface class.

\sa HbEditorInterface
*/
enum HbEditorConstraint {
    HbEditorConstraintNone                 = 0,
    HbEditorConstraintAutoCompletingField  = 0x01,   /**< This is auto-completing editor. It remembers what has been typed previously to same editor class. */
    HbEditorConstraintIgnoreFocus          = 0x02,   /**< Editor rejects input framework focus. */
    HbEditorConstraintFixedInputMode       = 0x04,   /**< Editor doesn't allow initial input mode to be changed. */
    HbEditorConstraintLatinAlphabetOnly    = 0x08    /**< Editor allows only languages and input modes that produce latin alphabets. */
};
Q_DECLARE_FLAGS(HbEditorConstraints, HbEditorConstraint)
Q_DECLARE_OPERATORS_FOR_FLAGS(HbEditorConstraints)

/*!
\enum HbInputDigitType

Enumerates known digit type settings.
*/
enum HbInputDigitType {
    HbDigitTypeNone = 0,
    HbDigitTypeLatin,          /**< Latin digits in use */
    HbDigitTypeArabicIndic,    /**< Arabic indic digits in use */
    HbDigitTypeEasternArabic,  /**< Eastern arabic digits in use */
    HbDigitTypeDevanagari      /**< Devanagari digits in use */
};

/*!
Enumerates operation modes for touch keypad.
*/
enum HbKeypadMode {
    EModeAbc,        /**< Display aplhanumeric keypad */
    EModeNumeric     /**< Display number mode keypad */
};

/*!
Enumerates known editor classes. If this value is set through
HbEditorInterface, it will have an effect on some input functionality.
*/
enum HbInputEditorClass {
    HbInputEditorClassUnknown = 0,
    HbInputEditorClassEmail,
    HbInputEditorClassUrl,
    HbInputEditorClassUsername,
    HbInputEditorClassPassword,
    HbInputEditorClassPhoneNumber,
    HbInputEditorClassStreetAddress,
    HbInputEditorClassZipCode,
    HbInputEditorClassSIPAddress,
    HbInputEditorClassFirstName,
    HbInputEditorClassLastName,
    HbInputEditorClassFullName,
    HbInputEditorClassFaxNumber,
    HbInputEditorClassTitle,
    HbInputEditorClassProfession,
    HbInputEditorClassCity,
    HbInputEditorClassDestination,
    HbInputEditorClassNetworkDomain,
    HbInputEditorClassNetworkName,

    HbInputEditorClassLastItem          // Keep this last, but never use.
};

/*!
Enumerates primary candidate modes.
*/
enum HbPrimaryCandidateMode {
    HbPrimaryCandidateModeBestPrediction,        /**< Display best prediction as the primary candidate */
    HbPrimaryCandidateModeExactTyping            /**< Display exact typing as the primary candidate */
};

/*!
Enumerates typing correction levels.
*/
enum HbTypingCorrectionLevel {
    HbTypingCorrectionLevelLow,
    HbTypingCorrectionLevelMedium,
    HbTypingCorrectionLevelHigh
};

const int HbInputMinKeypressTimeout = 400;
const int HbInputMaxKeypressTimeout = 3000;

#define CUSTOM_INPUT_MASK 0xffff0000
#define CUSTOM_INPUT_ID_MASK 0x0000ffff

#endif // HB_INPUT_DEF_H

// End of file
