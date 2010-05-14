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

#ifndef HB_INPUT_VKB_HOST_BRIDGE_H
#define HB_INPUT_VKB_HOST_BRIDGE_H

#include <hbinputvkbhost.h>

class HbVkbHostBridgePrivate;

class HB_CORE_EXPORT HbVkbHostBridge : public HbVkbHost
{
    Q_OBJECT

public:
    static HbVkbHostBridge *instance();

    ~HbVkbHostBridge();
    void openKeypad(HbVirtualKeyboard *vkb, HbInputMethod* owner, bool animationAllowed);
    void openMinimizedKeypad(HbVirtualKeyboard *vkb, HbInputMethod* owner);
    void closeKeypad(bool animationAllowed);
    void minimizeKeypad(bool animationAllowed);
    HbVkbStatus keypadStatus() const;
    QSizeF keyboardArea() const;  
    HbVirtualKeyboard* activeKeypad() const;
    QRectF applicationArea() const;
    HbVkbStatus keypadStatusBeforeOrientationChange() const;

    HbVkbHost *activeHost() const;
    bool connectHost(HbVkbHost *host);

public slots:
    void ensureCursorVisibility();

signals:
    void stateTransitionCompleted();

protected:
    void refresh();
    bool stateTransitionOngoing() const;

private:
    HbVkbHostBridge();

private:
    HbVkbHostBridgePrivate *const d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbVkbHostBridge)
    Q_DISABLE_COPY(HbVkbHostBridge)
};

#endif // HB_INPUT_VKB_HOST_BRIDGE_H

// End of file

