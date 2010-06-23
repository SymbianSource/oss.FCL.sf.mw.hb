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

#ifndef HB_INPUT_KEYMAP_DATA_H
#define HB_INPUT_KEYMAP_DATA_H

#include <hbinputdef.h>
#include <hbinputlanguage.h>

#include <QStringList>
#include <QList>
#include <QMap>

class QChar;
class QString;
class HbKeymapPrivate;

class HB_CORE_EXPORT HbMappedKey
{
public:
    HbMappedKey();
    ~HbMappedKey();
    QChar keycode;
    QStringList chars;
    const QString characters(const HbModifiers modifiers) const;
};

struct HbKeyboardMap {
public:
    HbKeyboardType type;
    QList<HbMappedKey *> keys;
};

class HB_CORE_EXPORT HbKeymap
{
public:
    explicit HbKeymap(HbInputLanguage language);
    virtual ~HbKeymap();

    void addKeyboard(HbKeyboardMap *keyboard);

    const HbKeyboardMap *keyboard(HbKeyboardType keyboard) const;
    HbInputLanguage language() const;
    virtual const HbMappedKey *keyForIndex(HbKeyboardType keyboard, int keyIndex) const;
    virtual const HbMappedKey *keyForKeycode(HbKeyboardType keyboard, QChar keyCode) const;
    virtual const HbMappedKey *keyForCharacter(HbKeyboardType keyboard, QChar character) const;
    static bool isDeadKey(int key);
    static void combineCharacter(QChar deadKey, QChar key, QChar &firstKey, QChar &secondKey);

private:
    HbKeymapPrivate *mPrivate;
};

#endif // HB_INPUT_KEYMAP_DATA_H

// End of file
