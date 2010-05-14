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

#ifndef HBABSTRACTVKBHOST_H
#define HBABSTRACTVKBHOST_H

#include <QSizeF>
#include <QRectF>

#include <hbinputvkbhost.h>

class HbView;
class HbAbstractVkbHostPrivate;
class HbWidget;

class HB_CORE_EXPORT HbAbstractVkbHost : public HbVkbHost
{
    Q_OBJECT

public:
    HbAbstractVkbHost(HbWidget* containerWidget);
    HbAbstractVkbHost(QGraphicsWidget* containerWidget);
    ~HbAbstractVkbHost();

public: // From HbVkbHost
    void openKeypad(HbVirtualKeyboard *vkb = 0, HbInputMethod* owner = 0, bool animationAllowed = true);
    void openMinimizedKeypad(HbVirtualKeyboard *vkb, HbInputMethod* owner);
    void closeKeypad(bool animationAllowed = true);
    void minimizeKeypad(bool animationAllowed = true);
    HbVkbStatus keypadStatus() const;  
    HbVirtualKeyboard *activeKeypad() const;
    QRectF applicationArea() const;  
    QSizeF keyboardArea() const;
    HbVkbStatus keypadStatusBeforeOrientationChange() const;

protected: // From HbVkbHost
    void refresh();
    bool stateTransitionOngoing() const;

public:
    void resizeKeyboard();
    QSizeF confirmedKeyboardSize()const;
    QRectF activeViewRect() const;

public slots:
    virtual void ensureCursorVisibility();
    virtual void orientationAboutToChange();
    virtual void orientationChanged(Qt::Orientation orientation);
    virtual void animValueChanged(qreal aValue);
    virtual void animationFinished(); 
    virtual void currentViewChanged(HbView*);
    virtual void stateTransitionCompleted();

protected:
    HbAbstractVkbHost() : d_ptr(0) {}
    HbAbstractVkbHost(HbAbstractVkbHostPrivate *dd);

protected:
    HbAbstractVkbHostPrivate* const d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbAbstractVkbHost)
    Q_DISABLE_COPY(HbAbstractVkbHost)
};

#endif // HBABSTRACTVKBHOST_H

// End of file

