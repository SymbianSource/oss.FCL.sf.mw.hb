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

#ifndef HB_TOUCH_KEYPAD_BUTTON_H
#define HB_TOUCH_KEYPAD_BUTTON_H

#include <hbinputdef.h>     // For HB_INPUT_EXPORT
#include <hbpushbutton.h>

class HbInputVkbWidget;
class HbTouchKeypadButtonPrivate;
class HbStyleOptionLabel;
class HbFrameItem;

class HB_INPUT_EXPORT HbTouchKeypadButton : public HbPushButton
{
    Q_OBJECT
    Q_PROPERTY(int keyCode READ keyCode WRITE setKeyCode)

public:
    enum HbTouchButtonType {
        HbTouchButtonNormal,
        HbTouchButtonFunction,
        HbTouchButtonNormalInActive,
        HbTouchButtonFnInActive
    };

    enum HbTouchButtonState {
        HbTouchButtonReleased,
        HbTouchButtonPressed,
        HbTouchButtonLatched
    };

public:
    HbTouchKeypadButton(HbInputVkbWidget* owner, const QString &text, QGraphicsWidget *parent = 0);
    HbTouchKeypadButton(HbInputVkbWidget* owner, const HbIcon &icon, const QString &text, QGraphicsItem *parent = 0 );
    virtual ~HbTouchKeypadButton();
    
    int keyCode() const;
    void setKeyCode(int code);
    virtual void setText(const QString &text);
    virtual void setAdditionalText(const QString &additionalText);
    bool isFaded();
    void setFade(bool fade);
    void setButtonType(HbTouchButtonType buttonType);
    int getButtonType();
    HbFrameItem * getFrameIcon();
    void setBackgroundAttributes(HbTouchButtonState buttonState);
    int type() const;
    void setFrameIcon(const QString& frameIconFileName);
    void setAsStickyButton(bool isSticky);
    bool isStickyButton() const;
    void setLatch(bool enable);
    bool isLatched() const;
    void setInitialSize(const QSizeF& initialSize);

signals:
    void enteredInNonStickyRegion();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void gestureEvent(QGestureEvent *event);
    void setBackground(const QString& backgroundFrameFilename);
    virtual void changeEvent( QEvent *event );
    virtual void updatePrimitives();
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    QVariant itemChange(  GraphicsItemChange change, const QVariant & value );

protected:
   HbTouchKeypadButtonPrivate * const d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbTouchKeypadButton)
    Q_DISABLE_COPY(HbTouchKeypadButton)
};

#endif // HB_TOUCH_KEYPAD_BUTTON_H

// End of file

