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

#ifndef HB_INPUT_SCT_KEYBOARD_H
#define HB_INPUT_SCT_KEYBOARD_H

#include <hbinputdef.h>
#include "hbinputvkbwidget.h"

class QGraphicsItem;
class HbSctKeyboardPrivate;

class HB_INPUT_EXPORT HbSctKeyboard : public HbInputVkbWidget
{
    Q_OBJECT

public:
    HbSctKeyboard(HbInputMethod *owner, const HbKeymap *keymap, QGraphicsItem *parent = 0);
    ~HbSctKeyboard();

public:  // From HbVirtualKeyboard
    HbKeyboardType keyboardType() const;
    void setMode(HbKeypadMode mode, HbModifiers modifiers);

public: // From HbInputVkbWidget
    QSizeF preferredKeyboardSize();

public slots:
    void sendKeyPressEvent(const QKeyEvent &event);
    void sendKeyReleaseEvent(const QKeyEvent &event);
    void sendKeyChangeEvent(const QKeyEvent &pressEvent, const QKeyEvent &releaseEvent);

protected:
    HbSctKeyboard(HbSctKeyboardPrivate &dd, HbInputMethod *owner, const HbKeymap *keymap, QGraphicsItem* parent);

private slots:
    void updateButtonPreviewStatus(bool status);
    void changePage(HbInputVkbWidget::HbFlickDirection flickDirection);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbSctKeyboard)
    Q_DISABLE_COPY(HbSctKeyboard)
};

#endif // HB_INPUT_SCT_KEYBOARD_H

// End of file
