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

#ifndef HB_INPUT_VKB_WIDGET_H
#define HB_INPUT_VKB_WIDGET_H

#include <hbdialog.h>
#include <hbinputdef.h>
#include <hbinputvirtualkeyboard.h>

const int HbRepeatTimeout = 1000;
const int HbRepeatTimeoutShort = 150;

const QString backgroundGraphics("qtg_fr_input_v_bg");
const QString HbInputVkbHandleIcon("qtg_graf_input_v_swipe");
const qreal HbCloseHandleHeight = 0;
const qreal HbCloseHandleHeightInUnits = 2.68;
const qreal HbCloseHandleWidthInUnits = 18.8;

class HbInputVkbWidgetPrivate;
class HbInputMethod;
class HbView;
class HbKeymap;
class HbPanGesture;
class HbFrameDrawer;

class HB_INPUT_EXPORT HbInputVkbWidget : public HbWidget, public HbVirtualKeyboard
{
    Q_OBJECT

public:
    enum HbFlickDirection {
        HbFlickDirectionNone = 0,
        HbFlickDirectionLeft,
        HbFlickDirectionRight,
        HbFlickDirectionUp,
        HbFlickDirectionDown
    };

    enum HbVkbCloseMethod {
        HbVkbCloseMethodButtonDrag = 0x1,
        HbVkbCloseMethodCloseButton = 0x2,
        HbVkbCloseMethodCloseButtonDrag = HbVkbCloseMethodCloseButton | HbVkbCloseMethodButtonDrag,
        HbVkbCloseMethodCloseGesture = 0x4,
    };

    enum HbSctView {
        HbSctViewSpecialCharacter,
        HbSctViewSmiley
    };

    HbInputVkbWidget(QGraphicsItem *parent = 0);
    virtual ~HbInputVkbWidget();

public: // From HbVirtualKeyboard
    QWidget *asWidget();
    QGraphicsWidget *asGraphicsWidget();
    QSizeF preferredKeyboardSize();
    virtual void aboutToOpen(HbVkbHost *host);
    virtual void aboutToClose(HbVkbHost *host);
    virtual void keyboardOpened(HbVkbHost *host);
    virtual void keyboardClosed(HbVkbHost *host);
    virtual void keyboardAnimationFrame(HbVkbAnimationType type, qreal x);

    virtual void setKeymap(const HbKeymap *keymap);

    virtual void setMode(HbKeypadMode mode, HbModifiers modifiers);
    virtual HbKeypadMode mode() const;
    virtual HbModifiers modifiers() const;

    virtual void setContentItem(QGraphicsLayoutItem *item);
    virtual QGraphicsLayoutItem *contentItem() const;

    virtual void setKeyboardDimmed(bool dimmed);
    virtual void setBackgroundDrawing(bool backgroundEnabled);
    virtual QList<HbKeyPressProbability> probableKeypresses();
    virtual void animKeyboardChange();

    HbFlickDirection flickDirection();

protected: // From QGraphicsItem
    virtual QPainterPath shape() const;
    virtual void changeEvent(QEvent *event);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual int type() const {
        return Hb::ItemType_InputVkbWidget;
    }
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    virtual void gestureEvent(QGestureEvent *event);

protected:
    // layout
    QSizeF keypadButtonAreaSize();    
    HbFrameDrawer* iconDrawer();

public slots:
    void showSettingList();
    void showSettingsView();
    void closeSettingsView();
    void executeMethodDialog();
    virtual void closeSettingList();
    void settingsClosed();
    void showSmileyPicker(int rows = 0, int columns = 0);
    void keypadLanguageChangeAnimationUpdate(qreal value);
    void keypadLanguageChangeFinished();

    virtual void sendKeyPressEvent(const QKeyEvent &event);
    virtual void sendKeyDoublePressEvent(const QKeyEvent &event);
    virtual void sendKeyReleaseEvent(const QKeyEvent &event);
    virtual void sendLongPressEvent(const QKeyEvent &event);
    virtual void sendKeyChangeEvent(const QKeyEvent &releaseEvent, const QKeyEvent &pressEvent);

signals:
    void keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod closeMethod);
    void flickEvent(HbInputVkbWidget::HbFlickDirection direction);
    void smileySelected(QString text);
    void mouseMovedOutOfButton();
    void aboutToActivateCustomAction(HbAction *custAction);
    void settingsListClosed();
protected:
    HbInputVkbWidget(HbInputVkbWidgetPrivate &dd, QGraphicsItem *parent);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputVkbWidget)
    Q_DISABLE_COPY(HbInputVkbWidget)
    Q_PRIVATE_SLOT(d_func(), void _q_activateInputMethod(const HbInputMethodDescriptor &, const QByteArray &))
    Q_PRIVATE_SLOT(d_func(), void _q_settingsClosed(HbAction *action))
    Q_PRIVATE_SLOT(d_func(), void _q_smileyPickerClosed())

    friend class HbTouchKeypadButton;
    friend class HbInputUsedSymbolPane;
};

#endif // HB_INPUT_VKB_WIDGET_H

// End of file
