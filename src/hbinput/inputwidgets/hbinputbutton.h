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

#ifndef HB_INPUT_BUTTON_H
#define HB_INPUT_BUTTON_H

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QString>

#include <hbicon.h>
#include <hbinputdef.h>

class HbInputButtonPrivate;

const QString HbInputButtonIconDelete("qtg_mono_backspace1");
const QString HbInputButtonIconDelete2("qtg_mono_backspace2");
const QString HbInputButtonIconShift("qtg_mono_shift");
const QString HbInputButtonIconSymbol("qtg_mono_sym_qwerty");
const QString HbInputButtonIconSymbol2("qtg_mono_sym_itut");
const QString HbInputButtonIconEnter("qtg_mono_enter");
const QString HbInputButtonIconSpace("qtg_mono_space_vkb");
const QString HbInputButtonIconSpace2("qtg_mono_space_itut");
const QString HbInputButtonIconAlphabet("qtg_mono_alpha_mode");
const QString HbInputButtonIconPageChange("qtg_mono_special_characters_qwerty");
const QString HbInputButtonIconPageChange2("qtg_mono_special_characters_itut");
const QString HbInputButtonIconSmiley("qtg_mono_smiley");

class HB_INPUT_EXPORT HbInputButton : public QObject
{
    Q_OBJECT

    Q_ENUMS(HbInputButtonType ButtonType)
    Q_ENUMS(HbInputButtonState ButtonState)
    Q_PROPERTY(HbInputButton::HbInputButtonType buttonType READ type WRITE setType)
    Q_PROPERTY(HbInputButton::HbInputButtonState buttonState READ state WRITE setState)
    Q_PROPERTY(QPoint position READ position WRITE setPosition)
    Q_PROPERTY(QSize size READ size WRITE setSize)
    Q_PROPERTY(int keyCode READ keyCode WRITE setKeyCode)
    Q_PROPERTY(bool autoRepeat READ autoRepeat WRITE setAutoRepeat)
    Q_PROPERTY(QList<QString> texts READ texts WRITE setTexts)
    Q_PROPERTY(QString mappedCharacters READ mappedCharacters WRITE setMappedCharacters)
    Q_PROPERTY(QList<HbIcon> icons READ icons WRITE setIcons)
    Q_PROPERTY(QRectF boundingRect READ boundingRect WRITE setBoundingRect)

public:
    enum HbInputButtonKeyCode {
        ButtonKeyCodeDelete = Qt::Key_Delete,
        ButtonKeyCodeSpace = Qt::Key_Space,
        ButtonKeyCodeAsterisk = Qt::Key_Asterisk,
        ButtonKeyCodeShift = Qt::Key_Shift,
        ButtonKeyCodeControl = Qt::Key_Control,
        ButtonKeyCodeEnter = Qt::Key_Enter,
        ButtonKeyCodeCharacter = Qt::Key_unknown + 1,
        ButtonKeyCodeSymbol,
        ButtonKeyCodeAlphabet,
        ButtonKeyCodeSmiley,
        ButtonKeyCodePageChange,
        ButtonKeyCodeSettings,
        ButtonKeyCodeCustom
    };

    enum HbInputButtonType {
        ButtonTypeNormal,
        ButtonTypeFunction,
        ButtonTypeLabel,
        ButtonTypeCount
    };

    enum HbInputButtonState {
        ButtonStateReleased,
        ButtonStatePressed,
        ButtonStateLatched,
        ButtonStateDisabled,
        ButtonStateCount
    };

    enum HbInputButtonTextIndex {
        ButtonTextIndexPrimary,
        ButtonTextIndexSecondaryFirstRow,
        ButtonTextIndexSecondarySecondRow,
        ButtonTextIndexCount
    };

    enum HbInputButtonIconIndex {
        ButtonIconIndexPrimary,
        ButtonIconIndexSecondaryFirstRow,
        ButtonIconIndexSecondarySecondRow,
        ButtonIconIndexCount
    };

    HbInputButton();
    HbInputButton(int keyCode, const QPoint &position, const QSize &size = QSize(1, 1));

    ~HbInputButton();

    void setType(HbInputButtonType type);
    HbInputButtonType type() const;

    void setState(HbInputButtonState state);
    HbInputButtonState state() const;

    void setPosition(const QPoint &position);
    QPoint position() const;

    void setSize(const QSize &size);
    QSize size() const;

    void setKeyCode(int keyCode);
    int keyCode() const;

    void setAutoRepeat(bool autoRepeat);
    bool autoRepeat() const;

    void setText(const QString &text, HbInputButtonTextIndex index);
    void setTexts(const QList<QString> &texts);
    QString text(HbInputButtonTextIndex index) const;
    QList<QString> texts() const;

    void setMappedCharacters(const QString &mappedCharacters);
    QString mappedCharacters() const;

    void setIcon(const HbIcon &icon, HbInputButtonIconIndex index);
    void setIcons(const QList<HbIcon> &icons);
    HbIcon icon(HbInputButtonIconIndex index) const;
    QList<HbIcon> icons() const;

    void setBoundingRect(const QRectF &rect);
    QRectF boundingRect() const;

    QRectF activeTouchArea() const;

    void setLastTriggeredPosition(const QPointF &position);
    void clearLastTriggeredPosition();
    bool wasTriggeredAt(const QPointF &position) const;

    void setTouchPointPosition(const QPointF &position);
    QPointF currentTouchPointPosition() const;
    bool suppressMoveEvent(const QPointF &position);

protected:
    HbInputButtonPrivate *const d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputButton)
    Q_DISABLE_COPY(HbInputButton)
};

#endif // HB_INPUT_BUTTON_H

// End of file
