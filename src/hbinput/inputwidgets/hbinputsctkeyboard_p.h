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

#ifndef HB_INPUT_SCT_KEYBOARD_PRIVATE_H
#define HB_INPUT_SCT_KEYBOARD_PRIVATE_H

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

#include "hbinputsctkeyboard.h"
#include "hbinputvkbwidget_p.h"
class HbInputButtonGroup;

class HB_INPUT_PRIVATE_EXPORT HbSctKeyboardPrivate : public HbInputVkbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbSctKeyboard)

public:
    HbSctKeyboardPrivate();
    ~HbSctKeyboardPrivate();

    void init();
    int keyCode(int buttonId);
    void applyEditorConstraints();
    void applyEditorConstraints(HbInputFocusObject *focusedObject, HbInputButtonGroup *buttonGroup);
    void updateKeyCodes();
    void updateButtons();
    int keyAtIndex(int index);
    int countActiveKeys();
    void changePage(HbInputVkbWidget::HbFlickDirection flickDirection);

    HbKeyboardType mType;
    int mColumns;
    int mRows;
    int mCharacterButtons;
    int mPages;
    int mActivePage;
};

#endif // HB_INPUT_SCT_KEYBOARD_PRIVATE_H

// End of file
