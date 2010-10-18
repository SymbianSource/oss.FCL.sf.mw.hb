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

#ifndef HB_INPUT_BUTTON_GROUP_H
#define HB_INPUT_BUTTON_GROUP_H

#include <hbwidget.h>

#include <hbinputbutton.h>

class HbInputButtonGroupPrivate;
class HbAction;
class HbFrameDrawer;

class HB_INPUT_EXPORT HbInputButtonGroup : public HbWidget
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)

public:
    enum HbInputButtonTextType {
        ButtonTextTypeSingle,
        ButtonTextTypePrimary,
        ButtonTextTypeSecondaryFirstRow,
        ButtonTextTypeSecondarySecondRow,
        ButtonTextTypeLabel,
        ButtonTextTypeCount
    };

    explicit HbInputButtonGroup(QGraphicsItem *parent = 0);
    explicit HbInputButtonGroup(const QSize &size, QGraphicsItem *parent = 0);
    ~HbInputButtonGroup();

    void setGridSize(const QSize &size);
    QSize gridSize() const;

    void setButtons(const QList<HbInputButton *> &data);
    void setButton(HbInputButton *data, int index);
    void setButton(HbInputButton *data, int column, int row);
    void setButton(HbInputButton *data, HbInputButton::HbInputButtonKeyCode keyCode);
    QList<HbInputButton *> buttons() const;
    HbInputButton *button(int index) const;
    HbInputButton *button(int column, int row) const;
    HbInputButton *button(HbInputButton::HbInputButtonKeyCode keyCode) const;

    void setCustomButtonActions(const QList<HbAction *> &actions);
    QList<HbAction *> customButtonActions() const;

    void setButtonBorderSize(qreal borderSize);
    qreal buttonBorderSize() const;

    void setButtonPreviewEnabled(bool enabled);
    bool isButtonPreviewEnabled() const;
    void setCharacterSelectionPreviewEnabled(bool enabled);
    bool isCharacterSelectionPreviewEnabled() const;

    void setMultiTouchEnabled(bool enabled);
    bool isMultiTouchEnabled() const;

    void setBackground(HbFrameDrawer *background);

    qreal fontSize(HbInputButtonTextType textType);
    void setFontSize(HbInputButtonTextType textType,qreal size);
    void resetFontSizes();

    QList<HbKeyPressProbability> buttonProbabilities() const;

public: // From QGraphicsItem
    void setEnabled(bool enabled);
    bool isEnabled() const;

public: // From QGraphicsLayoutItem
    void setGeometry(const QRectF &rect);

protected: // From QGraphicsItem
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    bool sceneEvent(QEvent *event);
    void changeEvent(QEvent *event);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    int type() const {
        return Hb::ItemType_InputButtonGroup;
    }

protected:
    HbInputButtonGroup(HbInputButtonGroupPrivate &dd, QGraphicsItem *parent = 0);
    HbInputButtonGroup(HbInputButtonGroupPrivate &dd, const QSize &size, QGraphicsItem *parent = 0);

public slots:
    virtual void cancelButtonPress();

protected slots:
    virtual void emitButtonPressed(const QKeyEvent &event);
    virtual void emitButtonDoublePressed(const QKeyEvent &event);
    virtual void emitButtonReleased(const QKeyEvent &event);
    virtual void emitButtonLongPressed(const QKeyEvent &event);
    virtual void emitPressedButtonChanged(const QKeyEvent &releaseEvent, const QKeyEvent &pressEvent);

signals:
    void buttonPressed(const QKeyEvent &event);
    void buttonDoublePressed(const QKeyEvent &event);
    void buttonReleased(const QKeyEvent &event);
    void buttonLongPressed(const QKeyEvent &event);
    void pressedButtonChanged(const QKeyEvent &releaseEvent, const QKeyEvent &pressEvent);
    void aboutToActivateCustomAction(HbAction *custAction);

private slots:
    void longPressEvent();
    void updateCustomButtons();

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputButtonGroup)
    Q_DISABLE_COPY(HbInputButtonGroup)
    Q_PRIVATE_SLOT(d_func(), void _q_customActionDestroyed(QObject *))
};

#endif // HB_INPUT_BUTTON_GROUP_H

// End of file
