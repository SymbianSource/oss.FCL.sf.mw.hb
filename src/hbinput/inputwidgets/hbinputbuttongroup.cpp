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
#include "hbinputbuttongroup.h"
#include "hbinputbuttongroup_p.h"

#include <QPainter>
#include <QTextLayout>
#include <QGraphicsSceneMouseEvent>
#include <QTouchEvent>
#include <QTimer>
#include <QApplication>
#include <QInputContext>

#include <hbmainwindow.h>
#include <hbaction.h>
#include <hbevent.h>
#include <hbcolorscheme.h>
#include <hbinputpopupbase.h>
#include <hbframeitem.h>
#include <hbwidgetfeedback.h>
#include <hbdeviceprofile.h>
#include <hbinputregioncollector_p.h>
#include <hbinputsettingproxy.h>
#include "hbframedrawerpool_p.h"

#include "hbinputbutton.h"

/*!
@stable
@hbinput
\class HbInputButtonGroup
\brief A widget presenting group of buttons for virtual keyboards.

HbInputButtonGroup is a widget displaying a mat of buttons, performance-optimized
as a single widget instead of multiple separate button widgets.

\sa HbInputButton
*/

/// @cond

const QString HbNormalBackground("qtg_fr_input_btn_keypad_normal");
const QString HbNormalPressedBackground("qtg_fr_input_btn_keypad_pressed");
const QString HbNormalInActiveBackground("qtg_fr_input_btn_keypad_disabled");
const QString HbNormalLatchedBackground("qtg_fr_input_btn_keypad_latched");

const QString HbFunctionBackground("qtg_fr_input_btn_function_normal");
const QString HbFunctionPressedBackground("qtg_fr_input_btn_function_pressed");
const QString HbFunctionInActiveBackground("qtg_fr_input_btn_function_disabled");
const QString HbFunctionLatchedBackground("qtg_fr_input_btn_function_latched");

const QString HbPreviewBackground("qtg_fr_character_preview");

const QString HbNormalColor("qtc_input_button_normal");
const QString HbNormalColorPressed("qtc_input_button_pressed");
const QString HbNormalColorInActive("qtc_input_button_disabled");
const QString HbNormalColorLatched("qtc_input_button_latched");

const QString HbFunctionColor("qtc_input_function_normal");
const QString HbFunctionColorPressed("qtc_input_function_pressed");
const QString HbFunctionColorInActive("qtc_input_function_disabled");
const QString HbFunctionColorLatched("qtc_input_function_latched");

const QString HbButtonPreviewColor("qtc_input_preview_normal");
const QString HbCharacterSelectionPreviewColor("qtc_input_button_accented_normal");
const QString HbHwrSctKeyboard("hwr_symbol_keypad");

const int HbLongPressTimeout = 800;
const int HbAutoRepeatTimeout = 100;

const int HbTextTypeCount = HbInputButton::ButtonTypeCount * HbInputButton::ButtonStateCount;
const int HbTextLayoutCount = HbTextTypeCount * 3;

const qreal HbTextSizeInUnits = 4.6;
const qreal HbPrimaryTextSizeInUnits = 5.37;
const qreal HbSecondaryTextSizeInUnits = 3.36;
const qreal HbLabelTextSizeInUnits = 9;
const qreal HbPrimaryIconSizeInUnits = 5;
const qreal HbSecondaryIconSizeInUnits = 3.36;
const qreal HbHorizontalMarginInUnits = 0.75;
const qreal HbVerticalMarginInUnits = 1.14;
const qreal HbPreviewWidthInUnits = 10;
const qreal HbPreviewHeightInUnits = 16;
const qreal HbPreviewMarginInUnits = 3;

const qreal HbTouchAreaSizeInUnits = 8;

HbInputButtonGroupPrivate::HbInputButtonGroupPrivate()
    : mUnitValue(0), mGridSize(1, 1), mButtonBorderSize(1.0), mEnabled(true),
      mButtonPreviewEnabled(false), mCharacterSelectionPreviewEnabled(false),
      mMultiTouchEnabled(true), mCharacterSelectionPreview(0), mBackground(0),
      mHasMouseGrab(false)
{
    for (int i = 0; i < HbTextLayoutCount; ++i) {
        mTextLayouts.append(0);
    }

    updateColorArray();
}

HbInputButtonGroupPrivate::~HbInputButtonGroupPrivate()
{
    foreach(HbFrameDrawer *drawer, mButtonDrawers) {
        HbFrameDrawerPool::release(drawer);
    }

    foreach(HbInputButton *button, mButtonData) {
        delete button;
    }
    mButtonData.clear();

    foreach(QTextLayout *layout, mTextLayouts) {
        delete layout;
    }

    foreach(QTimer *timer, mLongPressTimers) {
        delete timer;
    }

    delete mCharacterSelectionPreview;

    HbFrameDrawerPool::release(mBackground);
}

void HbInputButtonGroupPrivate::updateGraphics(const QSizeF &size)
{
    if (!size.width() && !size.height()) {
        return;
    }

    qreal cellWidth = size.width() / mGridSize.width();
    qreal cellHeight = size.height() / mGridSize.height();

    for (int i = 0; i < mButtonData.count(); ++i) {
        HbInputButton *item = mButtonData.at(i);

        qreal width = cellWidth * item->size().width() - 2 * mButtonBorderSize;
        qreal height = cellHeight * item->size().height() - 2 * mButtonBorderSize;
        QSizeF frameSize = QSizeF(width, height);
        HbFrameDrawer *drawer = 0;
        QColor color;
        if (mEnabled) {
            drawer = HbFrameDrawerPool::get(buttonGraphics(item->type(), item->state()), HbFrameDrawer::NinePieces, frameSize);
            color = mColors.at(item->type() * HbInputButton::ButtonStateCount + item->state());
        } else {
            drawer = HbFrameDrawerPool::get(buttonGraphics(item->type(), HbInputButton::ButtonStateDisabled), HbFrameDrawer::NinePieces, frameSize);
            color = mColors.at(item->type() * HbInputButton::ButtonStateCount + HbInputButton::ButtonStateDisabled);
        }

        if (i < mButtonDrawers.count()) {
            if (drawer != mButtonDrawers.at(i)) {
                HbFrameDrawerPool::release(mButtonDrawers.at(i));
                mButtonDrawers.replace(i, drawer);
            } else {
                HbFrameDrawerPool::release(drawer);
            }
        } else {
            mButtonDrawers.append(drawer);
        }

        QList<HbIcon> icons = item->icons();
        for (int i = 0; i < icons.count(); ++i) {
            if (!icons.at(i).isNull()) {
                icons[i].setColor(color);
            }
        }
        item->setIcons(icons);
    }

    for (int i = mButtonDrawers.count() - 1; i >= mButtonData.count(); --i) {
        HbFrameDrawerPool::release(mButtonDrawers.at(i));
        mButtonDrawers.removeAt(i);
    }
}

void HbInputButtonGroupPrivate::updateTextLayouts(const QSizeF &size)
{
    if (!size.width() && !size.height()) {
        return;
    }

    QHash<int, QString> textContent;
    for (int i = 0; i < HbTextLayoutCount; ++i) {
        delete mTextLayouts[i];
        mTextLayouts[i] = 0;
    }

    // Sort different button texts to correct text content based on the
    // button type and state
    for (int i = 0; i < mButtonData.count(); ++i) {
        HbInputButton *item = mButtonData.at(i);

        int index = item->type() * HbInputButton::ButtonStateCount + item->state();
        if (!mEnabled) {
            index = item->type() * HbInputButton::ButtonStateCount + HbInputButton::ButtonStateDisabled;
        }

        if (!item->text(HbInputButton::ButtonTextIndexPrimary).isEmpty() &&
            item->icon(HbInputButton::ButtonIconIndexPrimary).isNull()) {
            int primaryIndex = index;
            if (item->text(HbInputButton::ButtonTextIndexSecondaryFirstRow).isEmpty() &&
                item->icon(HbInputButton::ButtonIconIndexSecondaryFirstRow).isNull() &&
                item->text(HbInputButton::ButtonTextIndexSecondarySecondRow).isEmpty() &&
                item->icon(HbInputButton::ButtonIconIndexSecondarySecondRow).isNull()) {
                primaryIndex += HbTextTypeCount;
            }
            textContent[primaryIndex] += item->text(HbInputButton::ButtonTextIndexPrimary);
            textContent[primaryIndex] += QChar(QChar::LineSeparator);
        }

        index += HbTextTypeCount * 2;

        if (!item->text(HbInputButton::ButtonTextIndexSecondaryFirstRow).isEmpty() &&
            item->icon(HbInputButton::ButtonIconIndexSecondaryFirstRow).isNull()) {
            textContent[index] += item->text(HbInputButton::ButtonTextIndexSecondaryFirstRow);
            textContent[index] += QChar(QChar::LineSeparator);
        }

        if (!item->text(HbInputButton::ButtonTextIndexSecondarySecondRow).isEmpty() &&
            item->icon(HbInputButton::ButtonIconIndexSecondarySecondRow).isNull()) {
            textContent[index] += item->text(HbInputButton::ButtonTextIndexSecondarySecondRow);
            textContent[index] += QChar(QChar::LineSeparator);
        }
    }

    // Create text layouts for each text content
    for (int index = 0; index < HbTextLayoutCount; ++index) {
        if (textContent.contains(index)) {
            int textIndex = index / HbTextTypeCount;
            if (textIndex == 0) {
                createPrimaryTextLayout(index, textContent, size);
            } else if (textIndex == 1) {
                createPrimarySingleTextLayout(index, textContent, size);
            } else {
                createSecondaryTextLayout(index, textContent, size);
            }
        }
    }
}

void HbInputButtonGroupPrivate::updateCustomActions()
{
    for (int i = 0; i < mUsedCustomButtons.count(); ++i) {
        if (i < mButtonData.count()) {
            HbInputButton *item = mButtonData.at(mUsedCustomButtons.at(i));

            item->setIcon(HbIcon(), HbInputButton::ButtonIconIndexPrimary);
            item->setText(QString(), HbInputButton::ButtonTextIndexPrimary);
            item->setText(QString(), HbInputButton::ButtonTextIndexSecondaryFirstRow);
            item->setText(QString(), HbInputButton::ButtonTextIndexSecondarySecondRow);
            item->setState(HbInputButton::ButtonStateReleased);
        }
    }
    mUsedCustomButtons.clear();

    for (int i = 0; i < mButtonData.count(); ++i) {
        HbInputButton *item = mButtonData.at(i);

        int actionIndex = item->keyCode() - HbInputButton::ButtonKeyCodeCustom;

        if (actionIndex >= 0 && actionIndex < mCustomActions.count()) {
            item->setIcon(mCustomActions.at(actionIndex)->icon(), HbInputButton::ButtonIconIndexPrimary);
            if (!mCustomActions.at(actionIndex)->isEnabled()) {
                item->setState(HbInputButton::ButtonStateDisabled);
            }
            mUsedCustomButtons.append(i);
        }
    }
}

void HbInputButtonGroupPrivate::updateButtonGrid(const QSizeF &size)
{
    mButtonGridPositions.clear();

    if (!size.width() && !size.height()) {
        return;
    }

    qreal cellWidth = size.width() / mGridSize.width();
    qreal cellHeight = size.height() / mGridSize.height();

    for (int i = 0; i < mButtonData.count(); ++i) {
        HbInputButton *item = mButtonData.at(i);

        for (int y = 0; y < item->size().height(); ++y) {
            for (int x = 0; x < item->size().width(); ++x) {
                QPair<int, int> position = QPair<int, int>(item->position().x() + x, item->position().y() + y);
                mButtonGridPositions.insert(position, i);
            }
        }
        QRectF rect = QRectF(item->position().x() * cellWidth, item->position().y() * cellHeight,
                             item->size().width() * cellWidth, item->size().height() * cellHeight);
        item->setBoundingRect(rect);
    }
}

void HbInputButtonGroupPrivate::updateColorArray()
{
    mColors.clear();
    for (int i = 0; i < HbTextTypeCount; ++i) {
        HbInputButton::HbInputButtonType type = static_cast<HbInputButton::HbInputButtonType>(i / HbInputButton::ButtonStateCount);
        HbInputButton::HbInputButtonState state = static_cast<HbInputButton::HbInputButtonState>(i % HbInputButton::ButtonStateCount);
        mColors.append(HbColorScheme::color(buttonColor(type, state)));
    }
}

void HbInputButtonGroupPrivate::showButtonPreview(HbInputButton *const item)
{
    Q_Q(HbInputButtonGroup);

    // Button preview should be shown only for non-function buttons if preview
    // is enabled for this button group. Space key is handled as a special case
    // and no preview will be shown for that although it is not function button.
    int index = mButtonData.indexOf(item);
    if (mButtonPreviewEnabled && !mButtonPreview.contains(index) &&
        item->type() != HbInputButton::ButtonTypeFunction &&
        item->keyCode() != HbInputButton::ButtonKeyCodeSpace) {

        HbInputButtonGroup *group = new HbInputButtonGroup(QSize(1, 1));
        mButtonPreview.insert(index, group);

        // Create preview button and add it to the new button group
        QList<HbInputButton *> buttons;
        HbInputButton *previewItem = 0;
        if (!item->icon(HbInputButton::ButtonIconIndexPrimary).isNull()) {
            int keyCode = -1;
            if (!item->text(HbInputButton::ButtonTextIndexPrimary).isEmpty()) {
                keyCode = item->text(HbInputButton::ButtonTextIndexPrimary).at(0).unicode();
            }
            previewItem = new HbInputButton(keyCode, QPoint(0, 0));
            previewItem->setIcon(item->icon(HbInputButton::ButtonIconIndexPrimary), HbInputButton::ButtonIconIndexPrimary);
        } else if (!item->text(HbInputButton::ButtonTextIndexPrimary).isEmpty()) {
            previewItem = new HbInputButton(item->text(HbInputButton::ButtonTextIndexPrimary).at(0).unicode(), QPoint(0, 0));
            previewItem->setText(item->text(HbInputButton::ButtonTextIndexPrimary), HbInputButton::ButtonTextIndexPrimary);
        }
        previewItem->setType(HbInputButton::ButtonTypeLabel);
        buttons.append(previewItem);
        group->setButtons(buttons);

        qreal cellWidth = q->boundingRect().width() / mGridSize.width();
        qreal cellHeight = q->boundingRect().height() / mGridSize.height();

        Qt::Orientation orientation = Qt::Horizontal;
        if (q->mainWindow()) {
            orientation = q->mainWindow()->orientation();
        }

        // Calculate text size
        QFont font = HbFontSpec(HbFontSpec::Primary).font();
        font.setPixelSize(int(group->fontSize(HbInputButtonGroup::ButtonTextTypeLabel)));
        QFontMetricsF fontMetrics(font);
        qreal textWidth = fontMetrics.width(item->text(HbInputButton::ButtonTextIndexPrimary));

        // Calculate preview size
        qreal width = textWidth + HbPreviewMarginInUnits * mUnitValue;
        if (!item->icon(HbInputButton::ButtonIconIndexPrimary).isNull()) {
            width = item->boundingRect().width();
        } else if (width < HbPreviewWidthInUnits * mUnitValue) {
            width = HbPreviewWidthInUnits * mUnitValue;
        }
        qreal height = HbPreviewHeightInUnits * mUnitValue;
        qreal x = (item->position().x() + 0.5 * item->size().width()) * cellWidth - 0.5 * width;
        if (x < 0) {
            x = 0;
        } else if (x + width > q->boundingRect().width()) {
            x = q->boundingRect().width() - width;
        }
        qreal y = item->position().y() * cellHeight - height;
        if (y < 0 && 
            orientation == Qt::Horizontal &&
            q->objectName() == HbHwrSctKeyboard) {
            y = (item->position().y() + 1)* cellHeight;
        }
        group->setGeometry(QRectF(q->mapToScene(x, y), QSizeF(width, height)));
        if (q->parentItem()) {
            group->setZValue(q->parentItem()->zValue() + 1);
        }

        group->setButtonBorderSize(0);
        HbFrameDrawer *drawer = HbFrameDrawerPool::get(HbPreviewBackground, HbFrameDrawer::ThreePiecesHorizontal, QSizeF(width, height));
        drawer->setFillWholeRect(true);
        group->setBackground(drawer);
        q->mainWindow()->scene()->addItem(group);
    }
}

void HbInputButtonGroupPrivate::hideButtonPreview(HbInputButton *const item)
{
    int index = mButtonData.indexOf(item);
    if (mButtonPreview.contains(index)) {
        delete mButtonPreview.take(index);
    }
}

void HbInputButtonGroupPrivate::showCharacterSelectionPreview(HbInputButton *const item)
{
    Q_Q(HbInputButtonGroup);

    // Character preview should be shown only for non-function buttons which contains more
    // than one character if preview is enabled for this button group
    if (mCharacterSelectionPreviewEnabled && item->type() != HbInputButton::ButtonTypeFunction &&
        item->mappedCharacters().count() > 1) {

        if (item->type() == HbInputButton::ButtonTypeFunction) {
            HbWidgetFeedback::triggered(q, Hb::InstantLongPressed, Hb::ModifierInputFunctionButton);
        } else {
            HbWidgetFeedback::triggered(q, Hb::InstantLongPressed);
        }

        mProbabilities.clear();
        q->cancelButtonPress();

        // Create and initialize character preview
        if (!mCharacterSelectionPreview) {
            mCharacterSelectionPreview = new HbInputPopupBase();
            HbInputRegionCollector::instance()->attach(mCharacterSelectionPreview);
            mCharacterSelectionPreview->setModal(true);
            mCharacterSelectionPreview->setBackgroundFaded(false);
            mCharacterSelectionPreview->setTimeout(HbPopup::NoTimeout);
            mCharacterSelectionPreview->setDismissPolicy(HbPopup::TapAnywhere);
            qreal margin = HbPreviewMarginInUnits * mUnitValue * 0.5;
            mCharacterSelectionPreview->setContentsMargins(margin, 0, margin, 0);
        }

        HbInputButtonGroup *group = new HbInputButtonGroup(QSize(item->mappedCharacters().count(), 1));
        QObject::connect(group, SIGNAL(buttonPressed(const QKeyEvent &)), q, SLOT(emitButtonPressed(const QKeyEvent &)));
        QObject::connect(group, SIGNAL(buttonDoublePressed(const QKeyEvent &)), q, SLOT(emitButtonDoublePressed(const QKeyEvent &)));
        QObject::connect(group, SIGNAL(buttonReleased(const QKeyEvent &)), q, SLOT(emitButtonReleased(const QKeyEvent &)));
        QObject::connect(group, SIGNAL(buttonLongPressed(const QKeyEvent &)), q, SLOT(emitButtonLongPressed(const QKeyEvent &)));
        QObject::connect(group, SIGNAL(pressedButtonChanged(const QKeyEvent &, const QKeyEvent &)), q, SLOT(emitPressedButtonChanged(const QKeyEvent &, const QKeyEvent &)));

        qreal cellWidth = q->boundingRect().width() / mGridSize.width();
        qreal cellHeight = q->boundingRect().height() / mGridSize.height();

        // Calculate text size
        QFont font = HbFontSpec(HbFontSpec::Primary).font();
        font.setPixelSize(int(fontSize(HbInputButtonGroup::ButtonTextTypeLabel)));
        QFontMetricsF fontMetrics(font);
        qreal textWidth = fontMetrics.width(item->mappedCharacters());

        // Calculate preview size
        qreal width = textWidth + HbPreviewMarginInUnits * mUnitValue * (item->mappedCharacters().count() - 1);
        qreal height = HbPreviewHeightInUnits * mUnitValue;
        qreal x = q->scenePos().x() + (item->position().x() + 0.5 * item->size().width()) * cellWidth;
        qreal y = q->scenePos().y() + item->position().y() * cellHeight;

        // Create character preview button and add it to the created button group
        QList<HbInputButton *> buttons;
        for (int i = 0; i < item->mappedCharacters().count(); ++i) {
            HbInputButton *previewItem = new HbInputButton(item->keyCode(), QPoint(i, 0));
            previewItem->setType(HbInputButton::ButtonTypeLabel);
            previewItem->setText(item->mappedCharacters().at(i), HbInputButton::ButtonTextIndexPrimary);
            buttons.append(previewItem);
        }
        group->setButtons(buttons);
        group->setButtonBorderSize(0);

        group->setPreferredSize(QSizeF(width, height));
        mCharacterSelectionPreview->setPreferredPos(QPointF(x, y), HbPopup::BottomEdgeCenter);
        mCharacterSelectionPreview->setContentWidget(group);

        HbFrameDrawer *drawer = HbFrameDrawerPool::get(HbPreviewBackground, HbFrameDrawer::ThreePiecesHorizontal, QSizeF(width, height));
        drawer->setFillWholeRect(true);
        mCharacterSelectionPreview->setBackgroundItem(new HbFrameItem(drawer));

        mCharacterSelectionPreview->show();
    }
}

void HbInputButtonGroupPrivate::pressEvent(const QPointF &position, bool emitSignal)
{
    Q_Q(HbInputButtonGroup);

    // Ignore press events outside button group
    if (!(position.x() >= 0 && position.x() < q->boundingRect().width() &&
          position.y() >= 0 && position.y() < q->boundingRect().height())) {
        return;
    }

    int column = static_cast<int>(position.x() / (q->boundingRect().width() / mGridSize.width()));
    int row = static_cast<int>(position.y() / (q->boundingRect().height() / mGridSize.height()));

    int index = mButtonGridPositions.value(QPair<int, int>(column, row), -1);

    if (index >= 0 && index < mButtonData.count()) {
        HbInputButton *item = mButtonData.at(index);

        // Check whether we are actually supposed to handle the event.
        if ((item->state() != HbInputButton::ButtonStateReleased &&
             item->state() != HbInputButton::ButtonStateLatched) ||
            (mCharacterSelectionPreview && mCharacterSelectionPreview->isVisible())) {
            if (item->state() == HbInputButton::ButtonStateDisabled) {
                startLongPress(index);
                mActiveButtons.append(index);        
                item->setLastTriggeredPosition(position);
            }
            return;
        }

        mActiveButtons.append(index);        
        item->setLastTriggeredPosition(position);
        
        if (item->type() == HbInputButton::ButtonTypeFunction) {
            HbWidgetFeedback::triggered(q, Hb::InstantPressed, Hb::ModifierInputFunctionButton);
        } else {
            HbWidgetFeedback::triggered(q, Hb::InstantPressed);        
        }

        item->setState(HbInputButton::ButtonStatePressed);
        updateGraphics(QSizeF(q->boundingRect().width(), q->boundingRect().height()));
        updateTextLayouts(QSizeF(q->boundingRect().width(), q->boundingRect().height()));
        q->update();

        showButtonPreview(item);

        startLongPress(index);

        if (emitSignal) {
            item->setTouchPointPosition(position);
            if (!mUsedCustomButtons.contains(index)) {
                QString text;
                if (item->type() == HbInputButton::ButtonTypeLabel) {
                    text = item->text(HbInputButton::ButtonTextIndexPrimary);
                }
                QKeyEvent event(QEvent::KeyPress, item->keyCode(), Qt::NoModifier, text);
                q->emitButtonPressed(event);
            }
        }
    }
}

void HbInputButtonGroupPrivate::doublePressEvent(const QPointF &position, bool emitSignal)
{
    Q_Q(HbInputButtonGroup);

    // Ignore double press events outside button group
    if (!(position.x() >= 0 && position.x() < q->boundingRect().width() &&
          position.y() >= 0 && position.y() < q->boundingRect().height())) {
        return;
    }

    int column = static_cast<int>(position.x() / (q->boundingRect().width() / mGridSize.width()));
    int row = static_cast<int>(position.y() / (q->boundingRect().height() / mGridSize.height()));

    int index = mButtonGridPositions.value(QPair<int, int>(column, row), -1);

    if (index >= 0 && index < mButtonData.count()) {
        HbInputButton *item = mButtonData.at(index);

        // Check whether we are actually supposed to handle the event.
        if ((item->state() != HbInputButton::ButtonStateReleased &&
             item->state() != HbInputButton::ButtonStateLatched) ||
            (mCharacterSelectionPreview && mCharacterSelectionPreview->isVisible())) {
            if (item->state() == HbInputButton::ButtonStateDisabled) {
                startLongPress(index);
                mActiveButtons.append(index);        
                item->setLastTriggeredPosition(position);
            }
            return;
        }

        mActiveButtons.append(index);        
        item->setLastTriggeredPosition(position);

        if (item->type() == HbInputButton::ButtonTypeFunction) {
            HbWidgetFeedback::triggered(q, Hb::InstantPressed, Hb::ModifierInputFunctionButton);
        } else {
            HbWidgetFeedback::triggered(q, Hb::InstantPressed);        
        }

        item->setState(HbInputButton::ButtonStatePressed);
        updateGraphics(QSizeF(q->boundingRect().width(), q->boundingRect().height()));
        updateTextLayouts(QSizeF(q->boundingRect().width(), q->boundingRect().height()));
        q->update();

        showButtonPreview(item);

        startLongPress(index);

        if (!mUsedCustomButtons.contains(index)) {
            if (emitSignal) {
                QString text;
                if (item->type() == HbInputButton::ButtonTypeLabel) {
                    text = item->text(HbInputButton::ButtonTextIndexPrimary);
                }
                QKeyEvent event(QEvent::KeyPress, item->keyCode(), Qt::NoModifier, text);
                q->emitButtonDoublePressed(event);
            }
        }
    }
}

void HbInputButtonGroupPrivate::moveEvent(const QPointF &oldPosition, const QPointF &newPosition)
{
    Q_Q(HbInputButtonGroup);

    int itemIndex = activeButtonIndex(oldPosition);
    if (itemIndex >= 0) {
        HbInputButton* activeItem = mButtonData.at(itemIndex);
        // If move event happens within short timeout near the original touch point 
        // or after timeout inside button's touch area the move event needs to be ignored
        if (activeItem && activeItem->suppressMoveEvent(newPosition)) {
            return;
        }
    }
    
    int newColumn = static_cast<int>(newPosition.x() / (q->boundingRect().width() / mGridSize.width()));
    int newRow = static_cast<int>(newPosition.y() / (q->boundingRect().height() / mGridSize.height()));

    int newIndex = mButtonGridPositions.value(QPair<int, int>(newColumn, newRow), -1);

    // Check if movement happens inside button group and both oldIndex and newIndex are valid
    if (newPosition.x() >= 0 && newPosition.x() < q->boundingRect().width() &&
        newPosition.y() >= 0 && newPosition.y() < q->boundingRect().height() &&
        oldPosition.x() >= 0 && oldPosition.x() < q->boundingRect().width() &&
        oldPosition.y() >= 0 && oldPosition.y() < q->boundingRect().height() &&
        itemIndex >= 0 && newIndex >= 0) {

        // When user moves a finger on the keyboard, currently active button(s) should consume events
        // as long as the finger position is within the touch area of the button. Button should be removed
        // from the mActiveButtons, when finger moves outside the button's touch area.

        // The button, this event belongs to, can be determined by checking which button handled the
        // 'oldPosition' coordinate in previous run of either pressEvent or moveEvent.
        // If button is no longer active just return.
        int activeItemIndex = activeButtonIndex(oldPosition);
        if (activeItemIndex < 0) {
            return;
        }
        HbInputButton* activeItem = mButtonData.at(activeItemIndex);        

        // In case user has moved finger far enough away from the original button, old active
        // item needs to be removed and the new one should be added to active list.
        if (!mActiveButtons.contains(newIndex)) {
            releaseEvent(oldPosition, false);
            pressEvent(newPosition, false);

            QString text;
            if (activeItem->type() == HbInputButton::ButtonTypeLabel) {
                text = activeItem->text(HbInputButton::ButtonTextIndexPrimary);
            }
            QKeyEvent releaseEvent(QEvent::KeyRelease, activeItem->keyCode(), Qt::NoModifier, text);

            HbInputButton *newItem = mButtonData.at(newIndex);
            if (newItem->type() == HbInputButton::ButtonTypeLabel) {
                text = newItem->text(HbInputButton::ButtonTextIndexPrimary);
            }
            QKeyEvent pressEvent(QEvent::KeyPress, newItem->keyCode(), Qt::NoModifier, text);

            q->emitPressedButtonChanged(releaseEvent, pressEvent);
        } else {
            // Even though we do nothing with this button this time, update the status
            // of the item, so next time this function is invoked, this button is correctly
            // detected as the owner of the new event.
            activeItem->setLastTriggeredPosition(newPosition);
        }
    } else {
        // Move event came from outside button group or one of the positions does not contain
        // button so create new release and press events for old and new position.
        // If one of the positions is inside button group a new key event will get emitted automatically.
        releaseEvent(oldPosition, false);
        pressEvent(newPosition);
    }
}

void HbInputButtonGroupPrivate::releaseEvent(const QPointF &position, bool emitSignal)
{
    Q_Q(HbInputButtonGroup);

    int activeIndex = activeButtonIndex(position);
    if (activeIndex >= 0) {
        HbInputButton *item = mButtonData.at(activeIndex);

        cancelLongPress(activeIndex);

        // Check if we need to handle this event
        if (item->state() != HbInputButton::ButtonStatePressed) {
            return;
        }

        // This item is pressed, so release can happen for this.
        mActiveButtons.removeAll(activeIndex);
        QPointF releasePosition = item->currentTouchPointPosition();

        if (item->type() == HbInputButton::ButtonTypeFunction) {
            HbWidgetFeedback::triggered(q, Hb::InstantReleased, Hb::ModifierInputFunctionButton);
        } else {
            HbWidgetFeedback::triggered(q, Hb::InstantReleased);
        }

        item->setState(HbInputButton::ButtonStateReleased);
        updateGraphics(QSizeF(q->boundingRect().width(), q->boundingRect().height()));
        updateTextLayouts(QSizeF(q->boundingRect().width(), q->boundingRect().height()));
        q->update();

        if (mCharacterSelectionPreview && mCharacterSelectionPreview->isVisible()) {
            return;
        }

        hideButtonPreview(item);

        if (emitSignal) {
            if (item->type() == HbInputButton::ButtonTypeFunction) {
                HbWidgetFeedback::triggered(q, Hb::InstantClicked, Hb::ModifierInputFunctionButton);
            } else {
                HbWidgetFeedback::triggered(q, Hb::InstantClicked);
            }
            int actionIndex = item->keyCode() - HbInputButton::ButtonKeyCodeCustom;
            if (actionIndex >= 0 && actionIndex < mCustomActions.count()) {
                emit q->aboutToActivateCustomAction(mCustomActions.at(actionIndex));
                mCustomActions.at(actionIndex)->activate(QAction::Trigger);
            } else {
                calculateButtonProbabilities(releasePosition);

                QString text;
                if (item->type() == HbInputButton::ButtonTypeLabel) {
                    text = item->text(HbInputButton::ButtonTextIndexPrimary);
                }
                QKeyEvent event(QEvent::KeyRelease, item->keyCode(), Qt::NoModifier, text);
                q->emitButtonReleased(event);
            }
        }
    }
}

void HbInputButtonGroupPrivate::longPressEvent()
{
    Q_Q(HbInputButtonGroup);

    int index = mLongPressButtons.at(0);
    mLongPressButtons.removeAt(0);
    QTimer *timer = mLongPressTimers.at(0);
    mLongPressTimers.removeAt(0);    

    if (index >= 0 && index < mButtonData.count()) {
        HbInputButton *item = mButtonData.at(index);

        // Handle autorepeating buttons
        if (item->autoRepeat() &&
            (item->state() == HbInputButton::ButtonStatePressed ||
             item->state() == HbInputButton::ButtonStateLatched)) {
            mLongPressButtons.append(index);
            mLongPressTimers.append(timer);
            timer->start(HbAutoRepeatTimeout);

            if (item->type() == HbInputButton::ButtonTypeFunction) {
                HbWidgetFeedback::triggered(q, Hb::InstantKeyRepeated, Hb::ModifierInputFunctionButton);
            } else {
                HbWidgetFeedback::triggered(q, Hb::InstantKeyRepeated);
            }

            QString text;
            if (item->type() == HbInputButton::ButtonTypeLabel) {
                text = item->text(HbInputButton::ButtonTextIndexPrimary);
            }
            int keycode = item->keyCode();
            QKeyEvent releaseEvent(QEvent::KeyRelease, keycode, Qt::NoModifier, text, true);
            q->emitButtonReleased(releaseEvent);
            QKeyEvent pressEvent(QEvent::KeyPress, keycode, Qt::NoModifier, text, true);
            q->emitButtonPressed(pressEvent);
        } else {
            // Buttons that doesn't support autorepeat can either show character preview
            // or generate a long press
            if (mCharacterSelectionPreviewEnabled && item->type() != HbInputButton::ButtonTypeFunction
                && item->mappedCharacters().count() > 1) {
                showCharacterSelectionPreview(item);
            } else {
                if (item->type() == HbInputButton::ButtonTypeFunction) {
                    HbWidgetFeedback::triggered(q, Hb::InstantLongPressed, Hb::ModifierInputFunctionButton);
                } else {
                    HbWidgetFeedback::triggered(q, Hb::InstantLongPressed);
                }

                QString text;
                if (item->type() == HbInputButton::ButtonTypeLabel) {
                    text = item->text(HbInputButton::ButtonTextIndexPrimary);
                }
                QKeyEvent event(QEvent::KeyPress, item->keyCode(), Qt::NoModifier, text, true);
                q->emitButtonLongPressed(event);
            }
            delete timer;
        }
    }
}

void HbInputButtonGroupPrivate::calculateButtonProbabilities(const QPointF &position)
{
    Q_Q(HbInputButtonGroup);

    mProbabilities.clear();

    qreal cellWidth = q->boundingRect().width() / mGridSize.width();
    qreal cellHeight = q->boundingRect().height() / mGridSize.height();

    QRectF touchArea = QRectF(position.x() - 0.5 * cellWidth, position.y() - 0.5 * cellHeight,
                              HbTouchAreaSizeInUnits * mUnitValue, HbTouchAreaSizeInUnits * mUnitValue);

    // Calculate probabilities based on the intersection area of "touch area" and button area
    qreal probabilities = 0;
    foreach(HbInputButton *button, mButtonData) {
        QRectF intersection = button->boundingRect().intersected(touchArea);

        if (intersection.isValid()) {
            qreal probability = intersection.width() * intersection.height() / (touchArea.width() * touchArea.height());
            probabilities += probability;

            HbKeyPressProbability probableKey;
            probableKey.keycode = button->keyCode();
            probableKey.probability = probability;
            mProbabilities.append(probableKey);
        }
    }

    // Normalize
    for (int i = 0; i < mProbabilities.count(); ++i) {
        mProbabilities[i].probability /= probabilities;
    }
}

/*!
\internal
\brief Search active button based on position.

Locates active button based on its last interaction point.

\param position Location of triggered user action.
\return NULL, when no button has reacted to given coordinates.
\return Pointer to active button.
*/
int HbInputButtonGroupPrivate::activeButtonIndex(const QPointF &position)
{
    // Here we get the position, where last interaction occurred.
    // Start searching from the end of the list, because it is likely,
    // that the triggered position happens on the button that was added last.
    for (int i = mActiveButtons.count() - 1; i >= 0; --i) {
        int activeIndex = mActiveButtons[i];
        if (mButtonData[activeIndex]->wasTriggeredAt(position)) {
            return activeIndex;
        }
    }

    // Position is not inside any single active button.
    return -1;
}

void HbInputButtonGroupPrivate::createPrimarySingleTextLayout(int index, const QHash<int, QString> &textContent, const QSizeF &size)
{
    qreal cellWidth = size.width() / mGridSize.width();
    qreal cellHeight = size.height() / mGridSize.height();

    QFont font = HbFontSpec(HbFontSpec::Primary).font();

    int typeIndex = index % HbTextTypeCount / HbInputButton::ButtonStateCount;
    if (typeIndex == HbInputButton::ButtonTypeLabel) {
        font.setPixelSize(int(fontSize(HbInputButtonGroup::ButtonTextTypeLabel)));
    } else {
        font.setPixelSize(int(fontSize(HbInputButtonGroup::ButtonTextTypeSingle)));
    }

    mTextLayouts[index] = new QTextLayout(textContent.value(index), font);

    // Create text line for each button with primary text and correct type and state. Layout it
    // to correct position
    mTextLayouts.at(index)->beginLayout();
    foreach(HbInputButton *item, mButtonData) {
        int layoutIndex = item->type() * HbInputButton::ButtonStateCount + item->state() + HbTextTypeCount;
        if (!mEnabled) {
            layoutIndex = item->type() * HbInputButton::ButtonStateCount + HbInputButton::ButtonStateDisabled + HbTextTypeCount;
        }
        if (index == layoutIndex && item->size().isValid() &&
            !item->text(HbInputButton::ButtonTextIndexPrimary).isEmpty() &&
            item->icon(HbInputButton::ButtonIconIndexPrimary).isNull() &&
            item->text(HbInputButton::ButtonTextIndexSecondaryFirstRow).isEmpty() &&
            item->icon(HbInputButton::ButtonIconIndexSecondaryFirstRow).isNull() &&
            item->text(HbInputButton::ButtonTextIndexSecondarySecondRow).isEmpty() &&
            item->icon(HbInputButton::ButtonIconIndexSecondarySecondRow).isNull()) {

            QTextLine line = mTextLayouts.at(index)->createLine();
            line.setNumColumns(item->text(HbInputButton::ButtonTextIndexPrimary).length());
            qreal textWidth = line.naturalTextWidth();
            qreal textHeight = line.height();

            if (typeIndex == HbInputButton::ButtonTypeLabel) {
                layoutTextLine(HbInputButtonGroup::ButtonTextTypeLabel, item, QSizeF(cellWidth, cellHeight), line, QSizeF(textWidth, textHeight));
            } else {
                layoutTextLine(HbInputButtonGroup::ButtonTextTypeSingle, item, QSizeF(cellWidth, cellHeight), line, QSizeF(textWidth, textHeight));
            }
        }
    }
    mTextLayouts.at(index)->endLayout();
    mTextLayouts.at(index)->setCacheEnabled(true);
}

void HbInputButtonGroupPrivate::createPrimaryTextLayout(int index, const QHash<int, QString> &textContent, const QSizeF &size)
{
    qreal cellWidth = size.width() / mGridSize.width();
    qreal cellHeight = size.height() / mGridSize.height();

    QFont font = HbFontSpec(HbFontSpec::Primary).font();
    font.setPixelSize(int(fontSize(HbInputButtonGroup::ButtonTextTypePrimary)));

    mTextLayouts[index] = new QTextLayout(textContent.value(index), font);
    // Create text line for each button with primary text and correct type and state. Layout it
    // to correct position
    mTextLayouts.at(index)->beginLayout();
    foreach(HbInputButton *item, mButtonData) {
        int layoutIndex = item->type() * HbInputButton::ButtonStateCount + item->state();
        if (!mEnabled) {
            layoutIndex = item->type() * HbInputButton::ButtonStateCount + HbInputButton::ButtonStateDisabled;
        }
        if (index == layoutIndex && item->size().isValid() &&
            !item->text(HbInputButton::ButtonTextIndexPrimary).isEmpty() &&
            item->icon(HbInputButton::ButtonIconIndexPrimary).isNull() &&
            !(item->text(HbInputButton::ButtonTextIndexSecondaryFirstRow).isEmpty() &&
              item->icon(HbInputButton::ButtonIconIndexSecondaryFirstRow).isNull() &&
              item->text(HbInputButton::ButtonTextIndexSecondarySecondRow).isEmpty() &&
              item->icon(HbInputButton::ButtonIconIndexSecondarySecondRow).isNull())) {

            QTextLine line = mTextLayouts.at(index)->createLine();
            line.setNumColumns(item->text(HbInputButton::ButtonTextIndexPrimary).length());

            qreal textWidth = line.naturalTextWidth();
            qreal textHeight = line.height();

            layoutTextLine(HbInputButtonGroup::ButtonTextTypePrimary, item, QSizeF(cellWidth, cellHeight), line, QSizeF(textWidth, textHeight));
        }
    }
    mTextLayouts.at(index)->endLayout();
    mTextLayouts.at(index)->setCacheEnabled(true);
}

void HbInputButtonGroupPrivate::createSecondaryTextLayout(int index, const QHash<int, QString> &textContent, const QSizeF &size)
{
    QFont font = HbFontSpec(HbFontSpec::Primary).font();
    font.setPixelSize(int(fontSize(HbInputButtonGroup::ButtonTextTypeSecondaryFirstRow)));

    mTextLayouts[index] = new QTextLayout(textContent.value(index), font);

    // Create text line for each button with secondary first row or second row text and correct type and state.
    // Layout it to correct position
    mTextLayouts.at(index)->beginLayout();
    foreach(HbInputButton *item, mButtonData) {
        int layoutIndex = item->type() * HbInputButton::ButtonStateCount + item->state() + HbTextTypeCount * 2;
        if (!mEnabled) {
            layoutIndex = item->type() * HbInputButton::ButtonStateCount + HbInputButton::ButtonStateDisabled + HbTextTypeCount * 2;
        }
        if (index == layoutIndex && item->size().isValid()) {
            // Layout secondary text for first row
            layoutSecondaryText(index, item, size,
                                HbInputButton::ButtonTextIndexSecondaryFirstRow,
                                HbInputButton::ButtonIconIndexSecondaryFirstRow,
                                HbInputButton::ButtonTextIndexSecondarySecondRow,
                                HbInputButton::ButtonIconIndexSecondarySecondRow,
                                HbInputButtonGroup::ButtonTextTypeSecondaryFirstRow);
                                

            // Layout secondary text for second row
            layoutSecondaryText(index, item, size,
                                HbInputButton::ButtonTextIndexSecondarySecondRow,
                                HbInputButton::ButtonIconIndexSecondarySecondRow,
                                HbInputButton::ButtonTextIndexSecondaryFirstRow,
                                HbInputButton::ButtonIconIndexSecondaryFirstRow,
                                HbInputButtonGroup::ButtonTextTypeSecondarySecondRow);
        }
    }
    mTextLayouts.at(index)->endLayout();
    mTextLayouts.at(index)->setCacheEnabled(true);
}

void HbInputButtonGroupPrivate::layoutSecondaryText(int index, HbInputButton *item, const QSizeF &size,
                                                    HbInputButton::HbInputButtonTextIndex firstTextIndex,
                                                    HbInputButton::HbInputButtonIconIndex firstIconIndex,
                                                    HbInputButton::HbInputButtonTextIndex secondTextIndex,
                                                    HbInputButton::HbInputButtonIconIndex secondIconIndex,
                                                    HbInputButtonGroup::HbInputButtonTextType textType)
{
    qreal cellWidth = size.width() / mGridSize.width();
    qreal cellHeight = size.height() / mGridSize.height();

    if (!item->text(firstTextIndex).isEmpty() &&
        item->icon(firstIconIndex).isNull()) {

        QTextLine line = mTextLayouts.at(index)->createLine();
        line.setNumColumns(item->text(firstTextIndex).length());
        qreal textWidth = line.naturalTextWidth();
        qreal textHeight = line.height();

        if (item->text(HbInputButton::ButtonTextIndexPrimary).isEmpty() && 
            item->icon(HbInputButton::ButtonIconIndexPrimary).isNull()) {
            if (item->text(secondTextIndex).isEmpty() && 
                item->icon(secondIconIndex).isNull()) {
                layoutTextLine(HbInputButtonGroup::ButtonTextTypeSingle, item, QSizeF(cellWidth, cellHeight), line, QSizeF(textWidth, textHeight));
            } else {
                layoutTextLine(textType, item, QSizeF(cellWidth, cellHeight), line, QSizeF(textWidth, textHeight), true);
            }
        } else {
            layoutTextLine(textType, item, QSizeF(cellWidth, cellHeight), line, QSizeF(textWidth, textHeight));
        }
    }
}

void HbInputButtonGroupPrivate::layoutTextLine(HbInputButtonGroup::HbInputButtonTextType textType, const HbInputButton *button, const QSizeF &cellSize,
        QTextLine &textLine, const QSizeF &textSize, const bool centered)
{
    qreal textPositionX = 0.0;
    qreal textPositionY = 0.0;
    
    switch(textType) {
    case HbInputButtonGroup::ButtonTextTypeSingle:
    case HbInputButtonGroup::ButtonTextTypeLabel:
        textPositionX = (button->position().x() + 0.5 * button->size().width()) * cellSize.width() - 0.5 * textSize.width();
        textPositionY = (button->position().y() + 0.5 * button->size().height()) * cellSize.height() - 0.5 * textSize.height();
        break;

    case HbInputButtonGroup::ButtonTextTypePrimary:
        textPositionX = button->position().x() * cellSize.width() + HbHorizontalMarginInUnits * mUnitValue + mButtonBorderSize;
        textPositionY = (button->position().y() + 0.5 * button->size().height()) * cellSize.height() - 0.5 * textSize.height();
        break;

    case HbInputButtonGroup::ButtonTextTypeSecondaryFirstRow:
        if (centered) {
            textPositionX = (button->position().x() + 0.5 * button->size().width()) * cellSize.width() - 0.5 * textSize.width();
        } else {
            textPositionX = (button->position().x() + button->size().width()) * cellSize.width() -
                        textSize.width() - HbHorizontalMarginInUnits * mUnitValue - mButtonBorderSize;
        }
        textPositionY = (button->position().y() + button->size().height()) * cellSize.height() -
                        textSize.height() - HbVerticalMarginInUnits * mUnitValue - mButtonBorderSize;
        break;

    case HbInputButtonGroup::ButtonTextTypeSecondarySecondRow:
        if (centered) {
            textPositionX = (button->position().x() + 0.5 * button->size().width()) * cellSize.width() - 0.5 * textSize.width();
        } else {
            textPositionX = (button->position().x() + button->size().width()) * cellSize.width() -
                            textSize.width() - HbHorizontalMarginInUnits * mUnitValue - mButtonBorderSize;
        }
        textPositionY = button->position().y() * cellSize.height() + HbVerticalMarginInUnits * mUnitValue + mButtonBorderSize;
        break;
    default:
        break;
    }
    textLine.setPosition(QPointF(textPositionX, textPositionY));
}

QString HbInputButtonGroupPrivate::buttonGraphics(HbInputButton::HbInputButtonType type, HbInputButton::HbInputButtonState state)
{
    if (type == HbInputButton::ButtonTypeNormal) {
        if (state == HbInputButton::ButtonStateReleased) {
            return HbNormalBackground;
        } else if (state == HbInputButton::ButtonStatePressed) {
            return HbNormalPressedBackground;
        } else if (state == HbInputButton::ButtonStateLatched) {
            return HbNormalLatchedBackground;
        } else if (state == HbInputButton::ButtonStateDisabled) {
            return HbNormalInActiveBackground;
        }
    } else if (type == HbInputButton::ButtonTypeFunction) {
        if (state == HbInputButton::ButtonStateReleased) {
            return HbFunctionBackground;
        } else if (state == HbInputButton::ButtonStatePressed) {
            return HbFunctionPressedBackground;
        } else if (state == HbInputButton::ButtonStateLatched) {
            return HbFunctionLatchedBackground;
        } else if (state == HbInputButton::ButtonStateDisabled) {
            return HbFunctionInActiveBackground;
        }
    }
    return QString("");
}

QString HbInputButtonGroupPrivate::buttonColor(HbInputButton::HbInputButtonType type, HbInputButton::HbInputButtonState state)
{
    if (type == HbInputButton::ButtonTypeNormal) {
        if (state == HbInputButton::ButtonStateReleased) {
            return HbNormalColor;
        } else if (state == HbInputButton::ButtonStatePressed) {
            return HbNormalColorPressed;
        } else if (state == HbInputButton::ButtonStateLatched) {
            return HbNormalColorLatched;
        } else if (state == HbInputButton::ButtonStateDisabled) {
            return HbNormalColorInActive;
        }
    } else if (type == HbInputButton::ButtonTypeFunction) {
        if (state == HbInputButton::ButtonStateReleased) {
            return HbFunctionColor;
        } else if (state == HbInputButton::ButtonStatePressed) {
            return HbFunctionColorPressed;
        } else if (state == HbInputButton::ButtonStateLatched) {
            return HbFunctionColorLatched;
        } else if (state == HbInputButton::ButtonStateDisabled) {
            return HbFunctionColorInActive;
        }
    } else if (type == HbInputButton::ButtonTypeLabel) {
        if (mButtonData.count() == 1) {
            return HbButtonPreviewColor;
        } else {
            return HbCharacterSelectionPreviewColor;
        }
    }
    return QString("");
}

qreal HbInputButtonGroupPrivate::fontSize(HbInputButtonGroup::HbInputButtonTextType textType)
{
    return mFontSize[textType];
}

void HbInputButtonGroupPrivate::setFontSize(HbInputButtonGroup::HbInputButtonTextType textType,qreal size)
{
    mFontSize[textType] = size;
}

void HbInputButtonGroupPrivate::resetFontSizes()
{
    mFontSize[HbInputButtonGroup::ButtonTextTypeSingle] = HbTextSizeInUnits * mUnitValue;
    mFontSize[HbInputButtonGroup::ButtonTextTypePrimary] = HbPrimaryTextSizeInUnits * mUnitValue;
    mFontSize[HbInputButtonGroup::ButtonTextTypeSecondaryFirstRow] = HbSecondaryTextSizeInUnits * mUnitValue;
    mFontSize[HbInputButtonGroup::ButtonTextTypeSecondarySecondRow] = HbSecondaryTextSizeInUnits * mUnitValue;
    mFontSize[HbInputButtonGroup::ButtonTextTypeLabel] = HbLabelTextSizeInUnits * mUnitValue;
}

void HbInputButtonGroupPrivate::startLongPress(int index)
{
    Q_Q(HbInputButtonGroup);
    if (!mUsedCustomButtons.contains(index)) {
        mLongPressButtons.append(index);
        mLongPressTimers.append(new QTimer());
        mLongPressTimers.back()->setSingleShot(true);
        QObject::connect(mLongPressTimers.back(), SIGNAL(timeout()), q, SLOT(longPressEvent()));
        mLongPressTimers.back()->start(HbLongPressTimeout);
    }
}

void HbInputButtonGroupPrivate::cancelLongPress(int index)
{
    if (mLongPressButtons.contains(index)) {
        int listIndex = mLongPressButtons.indexOf(index);
        delete mLongPressTimers.at(listIndex);
        mLongPressTimers.removeAt(listIndex);
        mLongPressButtons.removeAt(listIndex);
    }
}

void HbInputButtonGroupPrivate::_q_customActionDestroyed(QObject *object)
{
    Q_Q(HbInputButtonGroup);

    HbAction *action = static_cast<HbAction *>(object);
    mCustomActions.removeAll(action);
    q->updateCustomButtons();
}

/// @endcond

/*!
Constructor
*/
HbInputButtonGroup::HbInputButtonGroup(QGraphicsItem *parent)
    : HbWidget(*new HbInputButtonGroupPrivate, parent)
{
    Q_D(HbInputButtonGroup);

    d->mUnitValue = HbDeviceProfile::profile(mainWindow()).unitValue();

    resetFontSizes();

    setAcceptedMouseButtons(Qt::LeftButton);
}

/*!
Constructor
*/
HbInputButtonGroup::HbInputButtonGroup(HbInputButtonGroupPrivate &dd, QGraphicsItem *parent)
    : HbWidget(dd, parent)
{
    Q_D(HbInputButtonGroup);

    d->mUnitValue = HbDeviceProfile::profile(mainWindow()).unitValue();

    resetFontSizes();

    setAcceptedMouseButtons(Qt::LeftButton);
}

/*!
Constructor
*/
HbInputButtonGroup::HbInputButtonGroup(const QSize &size, QGraphicsItem *parent)
    : HbWidget(*new HbInputButtonGroupPrivate, parent)
{
    Q_D(HbInputButtonGroup);

    d->mUnitValue = HbDeviceProfile::profile(mainWindow()).unitValue();

    resetFontSizes();

    setAcceptedMouseButtons(Qt::LeftButton);

    setGridSize(size);
}

/*!
Constructor
*/
HbInputButtonGroup::HbInputButtonGroup(HbInputButtonGroupPrivate &dd, const QSize &size, QGraphicsItem *parent)
    : HbWidget(dd, parent)
{
    Q_D(HbInputButtonGroup);

    d->mUnitValue = HbDeviceProfile::profile(mainWindow()).unitValue();

    resetFontSizes();

    setAcceptedMouseButtons(Qt::LeftButton);

    setGridSize(size);
}

/*!
Destructor
*/
HbInputButtonGroup::~HbInputButtonGroup()
{
}

/*!
Sets new grid size and updates button group content based on the new grid size.

\sa gridSize
*/
void HbInputButtonGroup::setGridSize(const QSize &size)
{
    Q_D(HbInputButtonGroup);

    d->mGridSize = size;
    if (d->mGridSize.width() < 1) {
        d->mGridSize.setWidth(1);
    }
    if (d->mGridSize.height() < 1) {
        d->mGridSize.setHeight(1);
    }

    setButtonBorderSize(d->mButtonBorderSize);
    d->updateButtonGrid(QSizeF(boundingRect().width(), boundingRect().height()));
    d->updateGraphics(QSizeF(boundingRect().width(), boundingRect().height()));
    d->updateTextLayouts(QSizeF(boundingRect().width(), boundingRect().height()));
    update();
}

/*!
Returns current grid size.

\sa setGridSize
*/
QSize HbInputButtonGroup::gridSize() const
{
    Q_D(const HbInputButtonGroup);

    return d->mGridSize;
}

/*!
Sets the button data and updates button group based on the new data.
Takes ownership of the button items. Button items that are not in the new list
will be destroyed.

\sa buttons
\sa button
*/
void HbInputButtonGroup::setButtons(const QList<HbInputButton *> &data)
{
    Q_D(HbInputButtonGroup);

    for (int i = 0; i < d->mButtonData.count(); ++i) {
        if (!data.contains(d->mButtonData.at(i))) {
            delete d->mButtonData.at(i);
            d->cancelLongPress(i);
        }
    }
    d->mButtonData = data;

    d->updateButtonGrid(QSizeF(boundingRect().width(), boundingRect().height()));
    d->updateCustomActions();
    d->updateColorArray();
    d->updateGraphics(QSizeF(boundingRect().width(), boundingRect().height()));
    d->updateTextLayouts(QSizeF(boundingRect().width(), boundingRect().height()));
    update();
}

/*!
Sets the button item and updates button group based on the new data.
Takes ownership of the button item. Old button item in the given index will be destroyed.
If null item is passed then the item from the given index will be removed.
If index is out of bounds then the item is inserted to the end of button list.

\sa buttons
\sa button
*/
void HbInputButtonGroup::setButton(HbInputButton *data, int index)
{
    Q_D(HbInputButtonGroup);

    if (index >= 0 && index < d->mButtonData.count()) {
        if (data != d->mButtonData.at(index)) {
            delete d->mButtonData.at(index);
            d->cancelLongPress(index);
        }
        if (data) {
            d->mButtonData.replace(index, data);
        } else {
            d->mButtonData.removeAt(index);
        }
    } else {
        if (data) {
            d->mButtonData.append(data);
        }
    }
    setButtons(d->mButtonData);
}

/*!
Sets the button item and updates button group based on the new data.
Takes ownership of the button item. Old button item in the given position will be destroyed.
If null item is passed then the item from the given position will be removed.
If the given position doesn't contain a button then the item is inserted to the end of button list.

\sa buttons
\sa button
*/
void HbInputButtonGroup::setButton(HbInputButton *data, int column, int row)
{
    Q_D(HbInputButtonGroup);

    int index = -1;
    if (d->mButtonGridPositions.contains(QPair<int, int>(column, row))) {
        index = d->mButtonGridPositions.value(QPair<int, int>(column, row), -1);
    }
    setButton(data, index);
}

/*!
Sets the button item and updates button group based on the new data.
Takes ownership of the button item. Old button item with given key code will be destroyed.
If null item is passed then the item with given key code will be removed.
If group contains multiple buttons with same key code then the first one will be replaced.
If there isn't any button with given key code then the button will be added to the end of button list.

\sa buttons
\sa button
*/
void HbInputButtonGroup::setButton(HbInputButton *data, HbInputButton::HbInputButtonKeyCode keyCode)
{
    Q_D(HbInputButtonGroup);

    int index = 0;
    for (; index < d->mButtonData.count(); ++index) {
        HbInputButton *item = d->mButtonData.at(index);
        if (item->keyCode() == keyCode) {
            break;
        }
    }

    setButton(data, index);
}

/*!
Returns button data.
Ownership of the returned button items is not transferred.

\sa setButtons
\sa setButton
*/
QList<HbInputButton *> HbInputButtonGroup::buttons() const
{
    Q_D(const HbInputButtonGroup);

    return d->mButtonData;
}

/*!
Returns button item.
Ownership of the returned button item is not transferred.

\sa setButtons
\sa setButton
*/
HbInputButton *HbInputButtonGroup::button(int index) const
{
    Q_D(const HbInputButtonGroup);

    if (index >= 0 && index < d->mButtonData.count()) {
        return d->mButtonData.at(index);
    }
    return 0;
}

/*!
Returns button data.
Ownership of the button items is not transferred.

\sa setButtons
\sa setButton
*/
HbInputButton *HbInputButtonGroup::button(int column, int row) const
{
    Q_D(const HbInputButtonGroup);

    int index = -1;
    if (d->mButtonGridPositions.contains(QPair<int, int>(column, row))) {
        index = d->mButtonGridPositions.value(QPair<int, int>(column, row), -1);
    }
    return button(index);
}

/*!
Returns button data.
Ownership of the button items is not transferred.

\sa setButtons
\sa setButton
*/
HbInputButton *HbInputButtonGroup::button(HbInputButton::HbInputButtonKeyCode keyCode) const
{
    Q_D(const HbInputButtonGroup);

    foreach(HbInputButton *button, d->mButtonData) {
        if (button->keyCode() == keyCode) {
            return button;
        }
    }
    return 0;
}

/*!
Sets custom actions and updates button group based on the new data. Custom actions
are mapped to buttons which keycode is equal or bigger than HbInputButtonKeyCustom.
Ownership of the actions is not transferred.

\sa customButtonActions
*/
void HbInputButtonGroup::setCustomButtonActions(const QList<HbAction *> &actions)
{
    Q_D(HbInputButtonGroup);

    disconnect(this, SLOT(updateCustomButtons()));
    disconnect(this, SLOT(_q_customActionDestroyed(QObject *)));

    d->mCustomActions = actions;

    foreach(HbAction *action, d->mCustomActions) {
        connect(action, SIGNAL(changed()), this, SLOT(updateCustomButtons()));
        connect(action, SIGNAL(destroyed(QObject *)), this, SLOT(_q_customActionDestroyed(QObject *)));
    }

    d->updateCustomActions();
    d->updateGraphics(QSizeF(boundingRect().width(), boundingRect().height()));
    d->updateTextLayouts(QSizeF(boundingRect().width(), boundingRect().height()));
    update();
}

/*!
Returns current custom actions.
Ownership of the actions is not transferred.

\sa setCustomButtonActions
*/
QList<HbAction *> HbInputButtonGroup::customButtonActions() const
{
    Q_D(const HbInputButtonGroup);

    return d->mCustomActions;
}

/*!
Sets the border size for the buttons and updates button group based on the new value.
Size must be in a range between 0 to half of button size.

\sa buttonBorderSize
*/
void HbInputButtonGroup::setButtonBorderSize(qreal borderSize)
{
    Q_D(HbInputButtonGroup);

    if (borderSize < 0.0) {
        borderSize = 0.0;
    } else if (boundingRect().width() != 0.0 && boundingRect().height() != 0.0) {
        qreal maxSize = boundingRect().width() / d->mGridSize.width() * 0.5;
        if (boundingRect().width() > boundingRect().height()) {
            maxSize = boundingRect().height() / d->mGridSize.height() * 0.5;
        }
        if (borderSize > maxSize) {
            borderSize = maxSize;
        }
    }
    d->mButtonBorderSize = borderSize;

    d->updateGraphics(QSizeF(boundingRect().width(), boundingRect().height()));
    update();
}

/*!
Returns the border size.

\sa setButtonBorderSize
*/
qreal HbInputButtonGroup::buttonBorderSize() const
{
    Q_D(const HbInputButtonGroup);

    return d->mButtonBorderSize;
}

/*!
Sets button preview state.

\sa isButtonPreviewEnabled
*/
void HbInputButtonGroup::setButtonPreviewEnabled(bool enabled)
{
    Q_D(HbInputButtonGroup);

    d->mButtonPreviewEnabled = enabled;
}

/*!
Returns preview state.

\sa setButtonPreviewEnabled
*/
bool HbInputButtonGroup::isButtonPreviewEnabled() const
{
    Q_D(const HbInputButtonGroup);

    return d->mButtonPreviewEnabled;
}

/*!
Sets button specific character selection preview state.

\sa isCharacterSelectionPreviewEnabled
*/
void HbInputButtonGroup::setCharacterSelectionPreviewEnabled(bool enabled)
{
    Q_D(HbInputButtonGroup);

    d->mCharacterSelectionPreviewEnabled = enabled;
}

/*!
Returns button specific character selection preview state.

\sa setCharacterSelectionPreviewEnabled
*/
bool HbInputButtonGroup::isCharacterSelectionPreviewEnabled() const
{
    Q_D(const HbInputButtonGroup);

    return d->mCharacterSelectionPreviewEnabled;
}


/*!
If given parameter is true more that one touch points are accepted
simultaneously.

\sa isMultiTouchEnabled
*/
void HbInputButtonGroup::setMultiTouchEnabled(bool enabled)
{
    Q_D(HbInputButtonGroup);

    d->mMultiTouchEnabled = enabled;
    setAcceptTouchEvents(enabled);
}

/*!
Returns true if more than one touch points are accepted simultaneously.

\sa setMultiTouchEnabled
*/
bool HbInputButtonGroup::isMultiTouchEnabled() const
{
    Q_D(const HbInputButtonGroup);

    return d->mMultiTouchEnabled;
}

/*!
Sets button group background graphics.
Takes ownership of the background object.
*/
void HbInputButtonGroup::setBackground(HbFrameDrawer *background)
{
    Q_D(HbInputButtonGroup);

    HbFrameDrawerPool::release(d->mBackground);
    d->mBackground = background;
}

/*!
Returns font size for given text type
*/
qreal HbInputButtonGroup::fontSize(HbInputButtonTextType textType)
{
    Q_D(HbInputButtonGroup);

    return d->fontSize(textType);
}

/*!
Set font size for given text type
*/
void HbInputButtonGroup::setFontSize(HbInputButtonTextType textType,qreal size)
{
    Q_D(HbInputButtonGroup);

    return d->setFontSize(textType,size);
}

/*!
Reset font size for all text type
*/
void HbInputButtonGroup::resetFontSizes()
{
    Q_D(HbInputButtonGroup);

    return d->resetFontSizes();
}

/*!
Returns all possible buttons the user could have intended to press
for the last registered touch along with their corresponding probabilities.
*/
QList<HbKeyPressProbability> HbInputButtonGroup::buttonProbabilities() const
{
    Q_D(const HbInputButtonGroup);

    return d->mProbabilities;
}

/*!
Sets the button group enabled or disabled.

\sa isEnabled
*/
void HbInputButtonGroup::setEnabled(bool enabled)
{
    Q_D(HbInputButtonGroup);

    d->mEnabled = enabled;
    d->updateGraphics(QSizeF(boundingRect().width(), boundingRect().height()));
    d->updateTextLayouts(QSizeF(boundingRect().width(), boundingRect().height()));
    update();
}

/*!
Returns the button group state.

\sa setEnabled
*/
bool HbInputButtonGroup::isEnabled() const
{
    Q_D(const HbInputButtonGroup);

    return d->mEnabled;
}

/*!
Draws the button group.
*/
void HbInputButtonGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    Q_D(HbInputButtonGroup);

    // Draw button group background
    if (d->mBackground) {
        d->mBackground->paint(painter, QRectF(0, 0, boundingRect().width(), boundingRect().height()));
    }

    qreal cellWidth = boundingRect().width() / d->mGridSize.width();
    qreal cellHeight = boundingRect().height() / d->mGridSize.height();

    for (int i = 0; i < d->mButtonData.count(); ++i) {
        HbInputButton *item = d->mButtonData.at(i);

        // Draw button backgrounds
        if (d->mButtonDrawers.at(i)) {
            qreal x = item->position().x() * cellWidth + d->mButtonBorderSize;
            qreal y = item->position().y() * cellHeight + d->mButtonBorderSize;
            qreal width = item->size().width() * cellWidth - 2 * d->mButtonBorderSize;
            qreal height = item->size().height() * cellHeight - 2 * d->mButtonBorderSize;

            painter->save();
            painter->translate(x, y);
            d->mButtonDrawers.at(i)->paint(painter, QRectF(0, 0, width, height));
            painter->restore();
        }

        // Draw primary icons
        if (!item->icon(HbInputButton::ButtonIconIndexPrimary).isNull()) {
            qreal x = item->position().x() * cellWidth;
            qreal y = item->position().y() * cellHeight;
            qreal width = item->size().width() * cellWidth;
            qreal height = item->size().height() * cellHeight;

            if (!item->text(HbInputButton::ButtonTextIndexSecondaryFirstRow).isEmpty() ||
                !item->text(HbInputButton::ButtonTextIndexSecondarySecondRow).isEmpty() ||
                !item->icon(HbInputButton::ButtonIconIndexSecondaryFirstRow).isNull() ||
                !item->icon(HbInputButton::ButtonIconIndexSecondarySecondRow).isNull()) {
                x += HbHorizontalMarginInUnits * d->mUnitValue + d->mButtonBorderSize;
                y += 0.5 * (item->size().height() * cellHeight - HbPrimaryIconSizeInUnits * d->mUnitValue);
                width = HbPrimaryIconSizeInUnits * d->mUnitValue;
                height = HbPrimaryIconSizeInUnits * d->mUnitValue;
            }
            item->icon(HbInputButton::ButtonIconIndexPrimary).paint(painter, QRectF(x, y, width, height));
        }

        // Draw secondary icons on first row
        if (!item->icon(HbInputButton::ButtonIconIndexSecondaryFirstRow).isNull()) {
            qreal x = (item->position().x() + item->size().width()) * cellWidth -
                      HbSecondaryIconSizeInUnits * d->mUnitValue - HbHorizontalMarginInUnits * d->mUnitValue - d->mButtonBorderSize;
            qreal y = (item->position().y() + item->size().height()) * cellHeight -
                      HbSecondaryIconSizeInUnits * d->mUnitValue - HbVerticalMarginInUnits * d->mUnitValue - d->mButtonBorderSize;
            qreal width = HbSecondaryIconSizeInUnits * d->mUnitValue;
            qreal height = HbSecondaryIconSizeInUnits * d->mUnitValue;

            Qt::Alignment alignment = static_cast<Qt::Alignment>(Qt::AlignVCenter | Qt::AlignRight);
            item->icon(HbInputButton::ButtonIconIndexSecondaryFirstRow).paint(painter, QRectF(x, y, width, height), Qt::KeepAspectRatio, alignment);
        }

        // Draw secondary icons on second row
        if (!item->icon(HbInputButton::ButtonIconIndexSecondarySecondRow).isNull()) {
            qreal x = (item->position().x() + item->size().width()) * cellWidth -
                      HbSecondaryIconSizeInUnits * d->mUnitValue - HbHorizontalMarginInUnits * d->mUnitValue - d->mButtonBorderSize;
            qreal y = item->position().y() * cellHeight + HbVerticalMarginInUnits * d->mUnitValue + d->mButtonBorderSize;
            qreal width = HbSecondaryIconSizeInUnits * d->mUnitValue;
            qreal height = HbSecondaryIconSizeInUnits * d->mUnitValue;

            Qt::Alignment alignment = static_cast<Qt::Alignment>(Qt::AlignVCenter | Qt::AlignRight);
            item->icon(HbInputButton::ButtonIconIndexSecondaryFirstRow).paint(painter, QRectF(x, y, width, height), Qt::KeepAspectRatio, alignment);
        }

    }

    // Draw button texts
    for (int i = 0; i < HbTextLayoutCount; ++i) {
        painter->save();
        painter->setPen(d->mColors.at(i % HbTextTypeCount));

        if (d->mTextLayouts.at(i)) {
            d->mTextLayouts.at(i)->draw(painter, QPointF(0, 0));
        }
        painter->restore();
    }
}

/*!
\reimp

Handles touch events.
*/
bool HbInputButtonGroup::sceneEvent(QEvent *event)
{
    Q_D(HbInputButtonGroup);

    // In case disabled or there is a preview popup open for any button, ignore events in this group.
    if (!d->mEnabled || (d->mCharacterSelectionPreview && d->mCharacterSelectionPreview->isVisible())) {
        event->ignore();
        return false;
    }

    switch(event->type()) {
    case QEvent::TouchBegin: {            
        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
        foreach(const QTouchEvent::TouchPoint &point, touchEvent->touchPoints()) {
            if (!point.isPrimary() && d->mMultiTouchEnabled) {
                d->pressEvent(point.pos());
            }
        }
        break;
    }

    case QEvent::TouchUpdate: {
        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
        foreach(const QTouchEvent::TouchPoint &point, touchEvent->touchPoints()) {
            if (!point.isPrimary() && d->mMultiTouchEnabled) {
                if (point.state() & Qt::TouchPointPressed) {
                    d->pressEvent(point.pos());
                } else if (point.state() & Qt::TouchPointMoved) {
                    d->moveEvent(point.lastPos(), point.pos());
                } else if (point.state() & Qt::TouchPointReleased) {
                    d->moveEvent(point.lastPos(), point.pos());
                    d->releaseEvent(point.pos());
                }
            }
        }
        break;
    }

    case QEvent::TouchEnd: {
        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
        foreach(const QTouchEvent::TouchPoint &point, touchEvent->touchPoints()) {
            if (!point.isPrimary() && d->mMultiTouchEnabled) {
                d->moveEvent(point.lastPos(), point.pos());
                d->releaseEvent(point.pos());
            }
        }
        break;
    }

    case QEvent::GraphicsSceneMousePress: {
        QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
        d->pressEvent(mouseEvent->pos());
        break;
    }

    case QEvent::GraphicsSceneMouseDoubleClick: {
        QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
        d->doublePressEvent(mouseEvent->pos());
        break;
    }

    case QEvent::GraphicsSceneMouseMove: {
        QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
        d->moveEvent(mouseEvent->lastPos(), mouseEvent->pos());
        break;
    }

    case QEvent::GraphicsSceneMouseRelease: {            
        QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
        d->moveEvent(mouseEvent->lastPos(), mouseEvent->pos());
        d->releaseEvent(mouseEvent->pos());
        break;
    }

    case QEvent::GrabMouse:
        d->mHasMouseGrab = true;
        break;

    case QEvent::UngrabMouse:
        d->mHasMouseGrab = false;
        break;

    default:
        return HbWidget::event(event);
    }
    return true;
}

/*!
\reimp

Updates button group geometry.
*/
void HbInputButtonGroup::setGeometry(const QRectF &rect)
{
    Q_D(HbInputButtonGroup);

    HbWidget::setGeometry(rect);

    setButtonBorderSize(d->mButtonBorderSize);
    d->updateButtonGrid(QSizeF(rect.width(), rect.height()));
    d->updateGraphics(QSizeF(rect.width(), rect.height()));
    d->updateTextLayouts(QSizeF(rect.width(), rect.height()));
}

/*!
\reimp

Updates theme graphics
 */
void HbInputButtonGroup::changeEvent(QEvent *event)
{
    Q_D(HbInputButtonGroup);

    if (event->type() == HbEvent::ThemeChanged) {
        if (d->mBackground) {
            d->mBackground->themeChanged();
        }

        foreach(HbFrameDrawer *drawer, d->mButtonDrawers) {
            drawer->themeChanged();
        }

        d->updateColorArray();

        for (int i = 0; i < d->mButtonData.count(); ++i) {
            HbInputButton *item = d->mButtonData.at(i);

            QColor color;
            if (d->mEnabled) {
                color = d->mColors.at(item->type() * HbInputButton::ButtonStateCount + item->state());
            } else {
                color = d->mColors.at(item->type() * HbInputButton::ButtonStateCount + HbInputButton::ButtonStateDisabled);
            }

            if (!item->icon(HbInputButton::ButtonIconIndexPrimary).isNull()) {
                HbIcon icon = item->icon(HbInputButton::ButtonIconIndexPrimary);
                icon.setColor(color);
                item->setIcon(icon, HbInputButton::ButtonIconIndexPrimary);
            }
        }
    }
    HbWidget::changeEvent(event);
}

/*!
\reimp
*/
void HbInputButtonGroup::showEvent(QShowEvent *event)
{
    Q_D(HbInputButtonGroup);

    setAcceptTouchEvents(d->mMultiTouchEnabled);

    if (qApp) {
        qApp->installEventFilter(this);
    }

    HbWidget::showEvent(event);
}

/*!
\reimp

Releases all pressed buttons when hidden.
*/
void HbInputButtonGroup::hideEvent(QHideEvent *event)
{
    cancelButtonPress();

    if (qApp) {
        qApp->removeEventFilter(this);
    }

    HbWidget::hideEvent(event);
}

/*!
Releases all pressed buttons when deactivated.
*/
bool HbInputButtonGroup::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if (event->type() == QEvent::ApplicationDeactivate) {
        cancelButtonPress();
    }
    return false;
}

/*!
Emits buttonPressed signal.
*/
void HbInputButtonGroup::emitButtonPressed(const QKeyEvent &event)
{
    emit buttonPressed(event);
}

/*!
Emits buttonDoublePressed signal.
*/
void HbInputButtonGroup::emitButtonDoublePressed(const QKeyEvent &event)
{
    emit buttonDoublePressed(event);
}

/*!
Emits buttonReleased signal.
*/
void HbInputButtonGroup::emitButtonReleased(const QKeyEvent &event)
{
    Q_D(HbInputButtonGroup);

    if (d->mCharacterSelectionPreview) {
        d->mCharacterSelectionPreview->hide();
    }

    emit buttonReleased(event);
}

/*!
Emits buttonLongPressed signal.
*/
void HbInputButtonGroup::emitButtonLongPressed(const QKeyEvent &event)
{
    emit buttonLongPressed(event);
}

/*!
Emits pressedButtonChanged signal.
*/
void HbInputButtonGroup::emitPressedButtonChanged(const QKeyEvent &releaseEvent, const QKeyEvent &pressEvent)
{
    emit pressedButtonChanged(releaseEvent, pressEvent);
}

/*!
Cancels current button press and releases all buttons.
*/
void HbInputButtonGroup::cancelButtonPress()
{
    Q_D(HbInputButtonGroup);

    if (d->mHasMouseGrab) {
        ungrabMouse();
    }

    d->mActiveButtons.clear();

    // Cancel long press timers
    d->mLongPressButtons.clear();
    foreach (QTimer *timer, d->mLongPressTimers) {
        delete timer;
    }
    d->mLongPressTimers.clear();

    // Release all buttons and close previews
    foreach (HbInputButton *button, d->mButtonData) {
        if (button->state() == HbInputButton::ButtonStatePressed) {
            button->setState(HbInputButton::ButtonStateReleased);
        }
        d->hideButtonPreview(button);
    }
    if (d->mCharacterSelectionPreview) { 
        d->mCharacterSelectionPreview->hide();
    }
    
    d->updateGraphics(QSizeF(boundingRect().width(), boundingRect().height()));
    d->updateTextLayouts(QSizeF(boundingRect().width(), boundingRect().height()));
    update();
}

/*!
Called when long press occurs.
*/
void HbInputButtonGroup::longPressEvent()
{
    Q_D(HbInputButtonGroup);

    d->longPressEvent();
}

/*!
Updates custom buttons with custom actions.

\sa setCustomButtonActions
*/
void HbInputButtonGroup::updateCustomButtons()
{
    Q_D(HbInputButtonGroup);

    d->updateCustomActions();
    d->updateGraphics(QSizeF(boundingRect().width(), boundingRect().height()));
    d->updateTextLayouts(QSizeF(boundingRect().width(), boundingRect().height()));
    update();
}

#include "moc_hbinputbuttongroup.cpp"

// End of file
