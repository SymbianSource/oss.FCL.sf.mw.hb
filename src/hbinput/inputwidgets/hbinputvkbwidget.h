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
#include <hbinputvirtualrocker.h>

const int HbRepeatTimeout = 1000;
const int HbRepeatTimeoutShort = 150;

const QString backgroundGraphics("qtg_fr_input_bg");
const QString HbInputVkbHandleIcon("qtg_graf_input_swipe");
const qreal HbCloseHandleHeight = 20.0;

class HbInputVkbWidgetPrivate;
class HbAction;
class HbInputMethod;
class HbView;
class HbKeymap;

class HB_INPUT_EXPORT HbInputVkbWidget : public HbWidget, public HbVirtualKeyboard
{
    Q_OBJECT

public:
    enum HbFlickDirection 
    {
        HbFlickDirectionNone = 0,
        HbFlickDirectionLeft,
        HbFlickDirectionRight,
        HbFlickDirectionUp,
        HbFlickDirectionDown
    };

    enum HbVkbCloseMethod
    {
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
    virtual HbFlickDirection flickDirection();

public: // From HbVirtualKeyboard
    QWidget* asWidget();
    QGraphicsWidget* asGraphicsWidget();
    QSizeF preferredKeyboardSize();
    QSizeF minimizedKeyboardSize();
    virtual void aboutToOpen(HbVkbHost *host);
    virtual void aboutToClose(HbVkbHost *host);
    virtual void keyboardOpened(HbVkbHost *host);
    virtual void keyboardClosed(HbVkbHost *host);
    virtual void keyboardMinimized(HbVkbHost *host);
    virtual void keyboardAnimationFrame(HbVkbAnimationType type, qreal x);

    virtual void setKeymap(const HbKeymap* keymap);

    virtual void setMode(HbKeypadMode mode, HbModifiers modifiers);
    virtual HbKeypadMode mode() const;
    virtual HbModifiers modifiers() const;
    virtual void setupToolCluster();

    virtual void setRockerVisible(bool visible);
    virtual bool isRockerVisible() const;

    virtual void setKeyboardDimmed(bool dimmed);
    virtual void setBackgroundDrawing(bool backgroundEnabled);
    virtual QList<HbKeyPressProbability> probableKeypresses();
    virtual void animKeyboardChange();


protected: // From QGraphicsItem
    virtual QPainterPath shape () const;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* aEvent); 
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* aEvent);
    virtual void changeEvent(QEvent *event);
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    virtual int type() const { return Hb::ItemType_InputVkbWidget; }
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

protected:
    // layout
    virtual QGraphicsLayout *keypadLayout();
    QSizeF keypadButtonAreaSize();

public slots:
    void showSettingList();
    void executeSettingsDialog();
    void executeMethodDialog();
    void closeSettingList();
    void mappedKeyPress(int aButtonId);
    void mappedKeyRelease(int aButtonId);
    void settingsClosed();
    void togglePredictionStatus();
    void showSmileyPicker(int rows, int columns);

    void refreshApplicationButton();
    void keypadLanguageChangeAnimationUpdate(qreal aValue);
    void keypadLanguageChangeFinished();

signals:
    void keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod closeMethod);
    void rockerDirection(int aDirection, HbInputVirtualRocker::RockerSelectionMode aSelectionMode);
    void flickEvent(HbInputVkbWidget::HbFlickDirection direction);
    void smileySelected(QString text);
    void mouseMovedOutOfButton();
protected:
    HbInputVkbWidget(HbInputVkbWidgetPrivate &dd, QGraphicsItem* parent); 

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputVkbWidget)
    Q_DISABLE_COPY(HbInputVkbWidget)

    friend class HbTouchKeypadButton;
    friend class HbInputUsedSymbolPane;
};

#endif // HB_INPUT_VKB_WIDGET_H

// End of file
