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

#include "hbinputkeymap.h"

#include <QChar>
#include <QString>
#include <QStringList>

const int DeadKeyTable[14] = {0x060, 0x0B4, 0x05E, 0x0A8, 0x022, 0x2C7, 0x2D8, 0x0B0, 0x2DB, 0x2DD, 0x07E, 0x0B8, 0x201A, 0x0B7};

const int DeadKeyCombineTable[59][3] = {
    {0x060, 0x061, 0x0e0},
    {0x060, 0x065, 0x0e8},
    {0x060, 0x069, 0x0ec},
    {0x060, 0x06F, 0x0f2},
    {0x060, 0x075, 0x0f9},

    {0x0b4, 0x061, 0x0e1},
    {0x0b4, 0x065, 0x0e9},
    {0x0b4, 0x069, 0x0ed},
    {0x0b4, 0x06F, 0x0f3},
    {0x0b4, 0x075, 0x0fa},
    {0x0b4, 0x079, 0x0fd},
    {0x0b4, 0x063, 0x107},
    {0x0b4, 0x06c, 0x13A},
    {0x0b4, 0x06e, 0x144},
    {0x0b4, 0x072, 0x155},
    {0x0b4, 0x053, 0x15b},
    {0x0b4, 0x074, 0x163},
    {0x0b4, 0x07A, 0x17A},

    {0x05e, 0x061, 0x0e2},
    {0x05e, 0x065, 0x0ea},
    {0x05e, 0x069, 0x0ee},
    {0x05e, 0x06F, 0x0f4},
    {0x05e, 0x075, 0x0fb},

    {0x0a8, 0x061, 0x0e4},
    {0x0a8, 0x065, 0x0eb},
    {0x0a8, 0x069, 0x0ef},
    {0x0a8, 0x06F, 0x0f6},
    {0x0a8, 0x075, 0x0fc},
    {0x0a8, 0x079, 0x0ff},

    {0x022, 0x061, 0x0e4},
    {0x022, 0x065, 0x0eb},
    {0x022, 0x069, 0x0ef},
    {0x022, 0x06F, 0x0f6},
    {0x022, 0x075, 0x0fc},
    {0x022, 0x079, 0x0ff},

    {0x2c7, 0x065, 0x11b},
    {0x2c7, 0x063, 0x10d},
    {0x2c7, 0x064, 0x10f},
    {0x2c7, 0x06C, 0x13e},
    {0x2c7, 0x06E, 0x148},
    {0x2c7, 0x072, 0x159},
    {0x2c7, 0x053, 0x161},
    {0x2c7, 0x074, 0x165},
    {0x2c7, 0x07A, 0x17E},

    {0x2d8, 0x061, 0x103},

    {0x0b0, 0x061, 0x0c5},
    {0x0b0, 0x075, 0x16f},

    {0x2db, 0x061, 0x105},
    {0x2db, 0x065, 0x119},

    {0x2dd, 0x06F, 0x151},
    {0x2dd, 0x075, 0x171},

    {0x07e, 0x061, 0x0e3},
    {0x07e, 0x06F, 0x0f5},
    {0x07e, 0x06E, 0x0f1},

    {0x0b8, 0x063, 0x0e7},
    {0x0b8, 0x072, 0x15f},

    {0x201a, 0x063, 0x0e7},
    {0x201a, 0x072, 0x15f},

    {0x0b7, 0x07A, 0x17c}
};

class HbKeymapPrivate
{

public:
    HbKeymapPrivate(HbInputLanguage language);
    ~HbKeymapPrivate();

public:
    QMap<HbKeyboardType, HbKeyboardMap *> mKeyboards;
    HbInputLanguage mLanguage;
};


HbKeymapPrivate::HbKeymapPrivate(HbInputLanguage language)
    : mLanguage(language)
{
}

HbKeymapPrivate::~HbKeymapPrivate()
{
    foreach(HbKeyboardMap *keymap, mKeyboards) {
        foreach(HbMappedKey *key, keymap->keys) {
            delete key;
        }
        delete keymap;
    }
    mKeyboards.clear();
}

/// @endcond

/*!
@stable
@hbcore
\class HbMappedKey

\brief Contains definition for characters associated with one keyboard key.

*/

/*!
Constructs the object.
*/
HbMappedKey::HbMappedKey()
{

}

/*!
Destructs the object.
*/
HbMappedKey::~HbMappedKey()
{
}

/*!
Returns QString of characters associated with the given modifiers in this key if defined,
otherwise returns an empty QString.
*/
const QString HbMappedKey::characters(const HbModifiers modifiers) const
{
    switch (modifiers) {
    case HbModifierNone:
        if (chars.count() >= 1) {
            return chars.at(0);
        } else {
            return QString();
        }
    case HbModifierShiftPressed:
        if (chars.count() >= 2) {
            return chars.at(1);
        } else {
            return QString();
        }
    case HbModifierFnPressed:
        if (chars.count() >= 3) {
            return chars.at(2);
        } else {
            return QString();
        }
    case HbModifierShiftPressed | HbModifierFnPressed:
        if (chars.count() >= 4) {
            return chars.at(3);
        } else {
            return QString();
        }
    default:
        return QString();
    }
}
/*!
@stable
@hbcore
\class HbKeymap
\brief Contains information on characters mapped to keyboard keys.

HbKeymaps contain keymap data for different keyboards and a single language.
Usually a HbKeymap is created by HbKeymapFactory and already contains the data
for all keyboards defined in the system.

The keyboards can be accessed using the keyboard() function, which returns a HbKeyboardMap struct, containing
a list of HbMappedKeys. HbMappedKeys each contain keycode and strings for characters available using different modifiers.
Any of the character strings except noModifier may be null if they are not defined.

Also helper methods are provided for finding keys containing certain keycodes or mapped characters.

\sa HbKeymapFactory
\sa HbInputLanguage
*/

/*!
Constructs the object.
*/
HbKeymap::HbKeymap(HbInputLanguage language)
{
    mPrivate = new HbKeymapPrivate(language);
}

/*!
Destructs the object.
*/
HbKeymap::~HbKeymap()
{
    delete mPrivate;
}

/*!
Adds a keyboard definition to the keymap. Usually needed only by the HbKeymapFactory.

\param keyboard Keyboard to be added in the internal structure.
*/
void HbKeymap::addKeyboard(HbKeyboardMap *keyboard)
{
    if (keyboard) {
        mPrivate->mKeyboards.insert(keyboard->type, keyboard);
    }
}

/*!
Returns a keyboard of the type \a keyboard or 0 if not found.

\param keyboard The type of the keyboard.
*/
const HbKeyboardMap *HbKeymap::keyboard(HbKeyboardType keyboard) const
{
    HbKeyboardMap *keyboardMap = 0;
    if (mPrivate->mKeyboards.contains(keyboard)) {
        keyboardMap = mPrivate->mKeyboards.value(keyboard);
    }
    return keyboardMap;
}

/*!
Returns the language of this keymap.

\sa HbInputLanguage
*/
HbInputLanguage HbKeymap::language() const
{
    return mPrivate->mLanguage;
}

/*!
Returns the key in the keyboard of type \a keyboard, on the position \a keyIndex or 0 if not found.

\param keyboard The type of the keyboard.
\param keyIndex Index of the key, first key at index 0.
*/
const HbMappedKey *HbKeymap::keyForIndex(HbKeyboardType keyboard, int keyIndex) const
{
    HbMappedKey *key = 0;
    if (mPrivate->mKeyboards.contains(keyboard)) {
        HbKeyboardMap *keymap = mPrivate->mKeyboards.value(keyboard);
        if (keymap->keys.count() > keyIndex && keyIndex >= 0) {
            key = keymap->keys.at(keyIndex);
        }
    }

    return key;
}

/*!
Returns the key on the keyboard \a keyboard with specified \a keycode or 0 if not found.

\param keyboard The type of the keyboard.
\param keycode Keycode to look for.
*/
const HbMappedKey *HbKeymap::keyForKeycode(HbKeyboardType keyboard, QChar keycode) const
{
    HbMappedKey *key = 0;
    if (mPrivate->mKeyboards.contains(keyboard)) {
        HbKeyboardMap *keymap = mPrivate->mKeyboards.value(keyboard);
        foreach(HbMappedKey *mappedKey, keymap->keys) {
            if (mappedKey->keycode == keycode) {
                key = mappedKey;
                break;
            }
        }
    }

    return key;
}

/*!
Returns the key on the keyboard \a keyboard containing a mapping to character \a character or 0
if not found.

\param keyboard The type of the keyboard.
\param character Character to look for.
*/
const HbMappedKey *HbKeymap::keyForCharacter(HbKeyboardType keyboard, QChar character) const
{
    HbMappedKey *key = 0;
    if (mPrivate->mKeyboards.contains(keyboard)) {
        HbKeyboardMap *keymap = mPrivate->mKeyboards.value(keyboard);
        foreach(HbMappedKey *mappedKey, keymap->keys) {
            foreach(const QString &charstring, mappedKey->chars) {
                if (charstring.contains(character)) {
                    key = mappedKey;
                    break;
                }
            }
            if (key) {
                break;
            }
        }
    }

    return key;
}

/*!
Returns true if key is a dead key

\param key Key to check.
*/

bool HbKeymap::isDeadKey(int key)
{
    int size = sizeof(DeadKeyTable) / sizeof(DeadKeyTable[0]);
    for (int i = 0; i < size; i++) {
        if (key == DeadKeyTable[i]) {
            return true;
        }
    }
    return false;
}

/*!
Returns key combination for dead key + another key

\param deadKey Dead key
\param key Key after dead key.
\param firstKey possible first combination result key
\param secondKey possible second combination result key
*/
void HbKeymap::combineCharacter(QChar deadKey, QChar key, QChar &firstKey, QChar &secondKey)
{
    QChar lowerKey = key.toLower();
    if (key == Qt::Key_Space) {
        // deadkey + space = deadkey
        firstKey = deadKey;
        secondKey = 0;
    } else if (key == Qt::Key_Backspace) {
        // deadkey + backspace = nothing
        firstKey = 0;
        secondKey = 0;
    } else {
        firstKey = deadKey;
        secondKey = key;
        int size = sizeof(DeadKeyCombineTable) / sizeof(DeadKeyCombineTable[0][0]) / 3;
        for (int i = 0; i < size; i++) {
            if (deadKey == DeadKeyCombineTable[i][0] && lowerKey == DeadKeyCombineTable[i][1]) {
                if (key.isUpper()) {
                    firstKey = ((QChar)DeadKeyCombineTable[i][2]).toUpper();
                } else {
                    firstKey = DeadKeyCombineTable[i][2];
                }
                secondKey = 0;
                break;
            }
        }
    }
    return;
}

// End of file
