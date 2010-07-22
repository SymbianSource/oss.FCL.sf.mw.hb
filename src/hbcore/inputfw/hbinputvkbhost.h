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

#ifndef HB_INPUT_VKB_HOST_H
#define HB_INPUT_VKB_HOST_H

#include <QObject>
#include <QSizeF>
#include <QRectF>
#include <QPointer>

#include <hbinputdef.h>

class QWidget;
class QGraphicsWidget;
class HbVirtualKeyboard;
class HbInputMethod;

class HB_CORE_EXPORT HbVkbHost : public QObject
{
    Q_OBJECT

public:
    /*!
    Enumerates different keypad states.
    */
    enum HbVkbStatus {
        HbVkbStatusClosed,
        HbVkbStatusMinimized,
        HbVkbStatusOpened,
    };

public:
    virtual ~HbVkbHost() {}
    virtual void openKeypad(HbVirtualKeyboard *vkb = 0, HbInputMethod *owner = 0, bool animationAllowed = true) = 0;
    virtual void openMinimizedKeypad(HbVirtualKeyboard *vkb, HbInputMethod *owner) = 0;
    virtual void closeKeypad(bool animationAllowed = true) = 0;
    virtual void minimizeKeypad(bool animationAllowed = true) = 0;
    virtual HbVkbStatus keypadStatus() const = 0;
    virtual QSizeF keyboardArea() const = 0;
    virtual HbVirtualKeyboard *activeKeypad() const = 0;
    virtual QRectF applicationArea() const = 0;
    virtual HbVkbStatus keypadStatusBeforeOrientationChange() const = 0;

    static void attachHost(HbVkbHost *host, QObject *object);
    static void detachHost(QObject *object);
    static HbVkbHost *getVkbHost(QObject *object);
    static HbVkbHost *activeVkbHost();

public slots:
    virtual void ensureCursorVisibility() = 0;

signals:
    void aboutToOpen();
    void aboutToClose();
    void keypadOpened();
    void keypadClosed();

protected:
    virtual void refresh() = 0;
    virtual bool stateTransitionOngoing() const = 0;

    friend class HbInputMethod;
    friend class HbVkbHostBridge;
};

#endif // HB_INPUT_VKB_HOST_H

// End of file
