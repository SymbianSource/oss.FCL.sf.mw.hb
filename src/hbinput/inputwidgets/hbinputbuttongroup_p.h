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

#ifndef HB_INPUT_BUTTON_GROUP_PRIVATE_H
#define HB_INPUT_BUTTON_GROUP_PRIVATE_H

#include "hbwidget_p.h"

#include "hbinputbuttongroup.h"
#include "hbinputbutton.h"

class HB_INPUT_PRIVATE_EXPORT HbInputButtonGroupPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbInputButtonGroup)

public:

    enum HbInputButtonTextType {
        ButtonTextTypeSingle,
        ButtonTextTypePrimary,
        ButtonTextTypeSecondaryFirstRow,
        ButtonTextTypeSecondarySecondRow,
        ButtonTextTypeLabel
    };

    HbInputButtonGroupPrivate();
    ~HbInputButtonGroupPrivate();

    virtual void updateGraphics(const QSizeF &size);
    virtual void updateTextLayouts(const QSizeF &size);
    virtual void updateCustomActions();
    virtual void updateButtonGrid(const QSizeF &size);
    virtual void updateColorArray();

    virtual void showButtonPreview(HbInputButton * const item);
    virtual void hideButtonPreview(HbInputButton * const item);
    virtual void showCharacterSelectionPreview(HbInputButton * const item);

    virtual void pressEvent(const QPointF &position, bool emitSignal = true);
    virtual void doublePressEvent(const QPointF &position, bool emitSignal = true);
    virtual void moveEvent(const QPointF &oldPosition, const QPointF &newPosition);
    virtual void releaseEvent(const QPointF &position, bool emitSignal = true);
    virtual void longPressEvent();

    virtual void calculateButtonProbabilities(const QPointF &position);

    void _q_customActionDestroyed(QObject *object);

protected:
    virtual void createPrimarySingleTextLayout(int index, const QHash<int, QString> &textContent, const QSizeF &size);
    virtual void createPrimaryTextLayout(int index, const QHash<int, QString> &textContent, const QSizeF &size);
    virtual void createSecondaryTextLayout(int index, const QHash<int, QString> &textContentt, const QSizeF &size);

    virtual void layoutTextLine(HbInputButtonTextType textType, const HbInputButton *button, const QSizeF &cellSize,
                                QTextLine &textLine, const QSizeF &textSize);

    virtual QString buttonGraphics(HbInputButton::HbInputButtonType type, HbInputButton::HbInputButtonState state);
    virtual QString buttonColor(HbInputButton::HbInputButtonType type, HbInputButton::HbInputButtonState state);

    virtual qreal fontSize(HbInputButtonTextType textType);

    void startLongPress(int index);
    void cancelLongPress(int index);

public:
    qreal mUnitValue;
    QList<HbFrameDrawer*> mButtonDrawers;
    QList<QTextLayout*> mTextLayouts;
    QList<QColor> mColors;
    QList<QTimer*> mLongPressTimers;
    QList<int> mLongPressButtons;
    QSize mGridSize;
    QList<HbInputButton*> mButtonData;
    QList<int> mUsedCustomButtons;
    QList<HbAction*> mCustomActions;
    QHash<QPair<int, int>, int> mButtonGridPositions;
    qreal mButtonBorderSize;
    bool mEnabled;
    bool mButtonPreviewEnabled;
    bool mCharacterSelectionPreviewEnabled;
    bool mMultiTouchEnabled;
    QHash<int, HbInputButtonGroup*> mButtonPreview;
    HbDialog *mCharacterSelectionPreview;
    HbFrameDrawer *mBackground;
    QList<HbKeyPressProbability> mProbabilities;
};

#endif // HB_INPUT_BUTTON_GROUP_PRIVATE_H

// End of file
