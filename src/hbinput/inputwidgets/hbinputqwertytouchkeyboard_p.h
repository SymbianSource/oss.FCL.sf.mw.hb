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

#ifndef HB_INPUT_QWERTY_TOUCH_KEYBOARD_PRIVATE_H
#define HB_INPUT_QWERTY_TOUCH_KEYBOARD_PRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb Inputs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <hbinputqwertytouchkeyboard.h>
#include "hbinputvkbwidget_p.h"

class HbCharPreviewPane;

class HB_INPUT_PRIVATE_EXPORT HbQwertyKeyboardPrivate : public HbInputVkbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbQwertyKeyboard) 

public:

    HbQwertyKeyboardPrivate();
    ~HbQwertyKeyboardPrivate();
    void handleStandardButtonPress(int buttonId);
    void handleStandardButtonRelease(int buttonId);
    void handleStandardButtonClick(int aButtonId);

    void setLayoutDimensions(QSizeF dimensions);
    QSizeF calculateDimensions(QSizeF size);
    void initializeKeyboard(bool refreshButtonText);
    void initializeNumericKeyboard();
    bool showPreview(int keycode);
    void setButtonObjectName(HbTouchKeypadButton& button, int row, int column, Qt::Key specialKey);
    void launchPreviewPane(const QStringList& list);

    void applyEditorConstraints();
    void setRockerPosition();
    void removeExistingSignalMappings();
    void initialize4x10Keypad(bool refreshButtonText);
    void initialize4x11Keypad(bool refreshButtonText);
    void constructKeypad();
    QString textForKey(int key);
    int indexForKeycode(int keyCode);
    int keyCode(int buttonId);
    int keyCode(HbTouchKeypadButton *button);
    void getAllowedSctCharcters(QString & allowedSctCharacters);
    void updateButtonsText();
public:
    QList<HbTouchKeypadButton*> mButtons;
    QList<HbStyleOptionLabel*> mKeypadButtonOption;
    int mCtrlBtnIndex;

    int mPressedButtonIndex;
    HbCharPreviewPane *mPreviewPane;
    bool mInStickyRegion;
    bool mLongKeyPressCharsShown;
    QSignalMapper *mClickMapper;
    bool mKeypadCreated;
    bool mKeymapChanged;
    HbQwertyKeyboardSize mKeyboardSize;
    QSizeF mSize;
};

#endif //HB_INPUT_QWERTY_TOUCH_KEYBOARD_PRIVATE_H

// End of file
