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
#include "hbinputvkbwidget.h"
#include "hbinputvkbwidget_p.h"

#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QGraphicsItemAnimation>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsProxyWidget>
#include <QTimeLine>
#include <QGraphicsScene>
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>

#include <hbapplication.h>
#include <hbmainwindow.h>
#include <hbaction.h>
#include <hbview.h>
#include <hbwidget.h>
#include <hbdialog.h>
#include <hbeffect.h>
#include <hbframedrawer.h>
#include <hbevent.h>
#include <hbdataform.h>
#include <hbinputregioncollector_p.h>
#include <hbfeedbackmanager.h>

#include <hbinputmethod.h>
#include <hbinputsettingproxy.h>
#include <hbinpututils.h>
#include <hbinputdef.h>
#include <hbinputvkbhost.h>
#include <hbinputvkbhostbridge.h>
#include <hbinputsettingwidget.h>
#include <hbinputcommondialogs.h>
#include <hbinputkeymap.h>
#include <hbinputkeymapfactory.h>
#include <hbwidgetfeedback.h>
#include <hbinputpredictionfactory.h>
#include <hbinputbuttongroup.h>
#include <hbinputbutton.h>
#include <HbSwipeGesture>
#include <HbTapGesture>
#include <HbSelectionDialog>
#include <HbListWidgetItem>

#include "hbinputsettinglist.h"
#include "hbinputmodeindicator.h"
#include "hbinputsmileypicker.h"
#include "hbinputscreenshotwidget.h"

const int HB_DIGIT_LATIN_START_VALUE          = 0x0030;
const int HB_DIGIT_ARABIC_INDIC_START_VALUE   = 0x0660;
const int HB_DIGIT_EASTERN_ARABIC_START_VALUE = 0x06F0;
const int HB_DIGIT_DEVANAGARI_START_VALUE     = 0x0966;

const qreal HbPortraitSmileyPickerHeightInUnits = 43.7;
const qreal HbPortraitSmileyPickerWidthInUnits = 47.8;
const qreal HbLandscapeSmileyPickerHeightInUnits = 31.9;
const qreal HbLandscapeSmileyPickerWidthInUnits = 83.4;
const qreal HbSmileyPickerMarginInUnits = 0.9;


/*!
@proto
@hbinput
\class HbInputVkbWidget
\brief A base class for touch keypads.

This class implements default mechanisms for opening and closing touch keypads.
It know how to operate in landscape and in portait modes and it know how
implement split view -mechasnism for S60 QT UI's Hb library. It also implements
closing mechansim, where used is able to close the touch keypad by sliding it downwards
with a finger. This class also implements background drawing for touch keypads.
*/

/// @cond

inline HbWidget *hbwidget_cast(QGraphicsItem *item)
{
    if (item->isWidget() && static_cast<QGraphicsWidget *>(item)->inherits("HbWidget")) {
        return static_cast<HbWidget *>(item);
    }
    return 0;
}

HbInputVkbWidgetPrivate::HbInputVkbWidgetPrivate()
    : mOwner(0),
      mMode(EModeAbc),
      mKeymap(0),
      mModifiers(0),
      mInputModeIndicator(0),
      mSettingList(0),
      mButtonLayout(0),
      mBackgroundDrawer(0),
      mIconDrawer(0),
      mMainWinConnected(false),
      mLayout(0),
      mCurrentHost(0),
      mDrawbackground(true),
      mMouseButtonPressedDown(false),
      mFlickDirection(HbInputVkbWidget::HbFlickDirectionNone),
      mSmileyPicker(0),
      mScreenshotWidget(0),
      mScreenshotTimeLine(250),
      mMostRecentlyAccessedButton(0),
      mMostRecentlyClickedLocation(0.0, 0.0),
      mFocusedObject(0),
      mCurrentFocusedObject(0),
      mFlickAnimation(false),
      mSettingsListOpen(false),
      mAnimateWhenDialogCloses(false),
      mKeyboardSize(HbQwerty4x10),
      mCloseHandleHeight(0),
      mCloseHandle(0),
      mSettingView(0),
      mCurrentView(0),
      mKeyboardDimmed(false),
	  mSettingWidget(0)
{
    mScreenshotTimeLine.setUpdateInterval(16);
}


HbInputVkbWidgetPrivate::~HbInputVkbWidgetPrivate()
{
    delete mSettingList;
    delete mBackgroundDrawer;
    delete mIconDrawer;
    delete mSmileyPicker;
    delete mScreenshotWidget;
}

void HbInputVkbWidgetPrivate::initLayout()
{
    Q_Q(HbInputVkbWidget);

    mLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mLayout->setContentsMargins(0.0, 0.0, 0.0, 0.0);
    mLayout->setSpacing(0.0);

    qreal unitValue = HbDeviceProfile::profile(q->mainWindow()).unitValue();
    mCloseHandleHeight = (int)(HbCloseHandleHeightInUnits * unitValue);
    mCloseHandleWidth = (int)(HbCloseHandleWidthInUnits * unitValue);

    mCloseHandle = new QGraphicsWidget();
    mCloseHandle->setObjectName("vkbHandle");
    mCloseHandle->setMinimumHeight(mCloseHandleHeight);
    mCloseHandle->setMaximumHeight(mCloseHandleHeight);
    mLayout->addItem(mCloseHandle);

    q->setContentItem(new HbInputButtonGroup());
    q->setLayout(mLayout);
}

void HbInputVkbWidgetPrivate::init()
{
    Q_Q(HbInputVkbWidget);
    QGraphicsItem::GraphicsItemFlags itemFlags = q->flags();
#if QT_VERSION >= 0x040600
    itemFlags |= QGraphicsItem::ItemSendsGeometryChanges;
#endif
    // Make sure the keypad never steals focus.
    itemFlags |= QGraphicsItem::ItemIsPanel;
    q->setFlags(itemFlags);

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup *>(q->contentItem());
    QObject::connect(buttonGroup, SIGNAL(aboutToActivateCustomAction(HbAction *)),
                     q, SIGNAL(aboutToActivateCustomAction(HbAction *)));

    mBackgroundDrawer = new HbFrameDrawer();
    mBackgroundDrawer->setFrameGraphicsName(backgroundGraphics);
    mBackgroundDrawer->setFrameType(HbFrameDrawer::ThreePiecesVertical);
    mBackgroundDrawer->setFillWholeRect(true);
    mBackgroundDrawer->setBorderWidths(0.0, mCloseHandleHeight, 0.0, mCloseHandleHeight);

    mIconDrawer = new HbFrameDrawer();
    mIconDrawer->setFrameType(HbFrameDrawer::OnePiece);
    mIconDrawer->setFrameGraphicsName(HbInputVkbHandleIcon);

    mReleaseMapper = new QSignalMapper(q);
    mPressMapper = new QSignalMapper(q);
    mActionMapper = new QSignalMapper(q);

    q->grabGesture(Qt::SwipeGesture);

    // eating gestures below the panel (remove when panel starts to do this)
    q->grabGesture(Qt::TapGesture);
    q->grabGesture(Qt::PanGesture);

    HbInputRegionCollector::instance()->attach(q);
}

// re-implemented by inherited keyboards
int HbInputVkbWidgetPrivate::keyCode(int buttonId)
{
    Q_UNUSED(buttonId);
    return -1;
}

// re-implemented by inherited keyboards
int HbInputVkbWidgetPrivate::keyCode(HbTouchKeypadButton *button)
{
    Q_UNUSED(button);
    return -1;
}

void HbInputVkbWidgetPrivate::handleStandardButtonPress(int buttonid)
{
    int keycode = buttonid;
    if (buttonid >= 0) {
        keycode = keyCode(buttonid);
    }

    QKeyEvent event(QEvent::KeyPress, keycode, Qt::NoModifier);
    if (mOwner) {
        mOwner->filterEvent(&event);
    }
}

void HbInputVkbWidgetPrivate::handleStandardButtonRelease(int buttonid)
{
    int keycode = buttonid;
    if (buttonid >= 0) {
        keycode = keyCode(buttonid);
    }

    QKeyEvent event(QEvent::KeyRelease, keycode, Qt::NoModifier);
    if (mOwner) {
        mOwner->filterEvent(&event);
    }
}

void HbInputVkbWidgetPrivate::addCustomButtonToLayout(HbTouchKeypadButton *button,
        int index)
{
    Q_UNUSED(button);
    Q_UNUSED(index);
}

void HbInputVkbWidgetPrivate::redirectMousePressEvent(QGraphicsSceneMouseEvent *aEvent)
{
    q_ptr->mousePressEvent(aEvent);
}

void HbInputVkbWidgetPrivate::redirectMouseReleaseEvent(QGraphicsSceneMouseEvent *aEvent)
{
    q_ptr->mouseReleaseEvent(aEvent);
}

void HbInputVkbWidgetPrivate::applyEditorConstraints()
{
    // no default implementaiton as of now.
}

void HbInputVkbWidgetPrivate::updateKeyCodes()
{
    Q_Q(HbInputVkbWidget);

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup *>(q->contentItem());
    if (buttonGroup) {
        int key = 0;
        QList<HbInputButton *> buttons = buttonGroup->buttons();
        for (int i = 0; i < buttons.count(); ++i) {
            if (keyCode(i) == HbInputButton::ButtonKeyCodeCharacter) {
                HbInputButton *item = buttons.at(i);

                const HbKeyboardMap *keyboardMap = mKeymap->keyboard(q->keyboardType());
                if (keyboardMap && key < keyboardMap->keys.count()) {
                    item->setKeyCode(keyboardMap->keys.at(key)->keycode.unicode());
                } else {
                    item->setKeyCode(-1);
                }
                ++key;
            }
        }
    }
}

void HbInputVkbWidgetPrivate::updateButtons()
{
    Q_Q(HbInputVkbWidget);

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup *>(q->contentItem());
    if (buttonGroup) {
        int key = 0;
        QList<HbInputButton *> buttons = buttonGroup->buttons();
        for (int i = 0; i < buttons.count(); ++i) {
            if (keyCode(i) == HbInputButton::ButtonKeyCodeCharacter) {
                HbInputButton *item = buttons.at(i);

                const HbKeyboardMap *keyboardMap = mKeymap->keyboard(q->keyboardType());
                if (keyboardMap && key < keyboardMap->keys.count() && keyboardMap->keys.at(key)->characters(mModifiers) != QString("")) {
                    QString keydata = keyboardMap->keys.at(key)->characters(mModifiers);
                    item->setText(keydata.at(0), HbInputButton::ButtonTextIndexPrimary);

                    QString mappedCharacters;
                    keydata.append(keyboardMap->keys.at(key)->characters(mModifiers | HbModifierFnPressed));
                    for (int i = 0; i < keydata.length(); ++i) {
                        if (mOwner->focusObject() &&
                            mOwner->focusObject()->characterAllowedInEditor(keydata.at(i))) {
                            mappedCharacters.append(keydata.at(i));
                        }
                    }
                    item->setMappedCharacters(mappedCharacters);
                } else {
                    item->setText("", HbInputButton::ButtonTextIndexPrimary);
                    item->setMappedCharacters("");
                }
                ++key;
            }
        }
        buttonGroup->setButtons(buttons);
    }
}

void HbInputVkbWidgetPrivate::settingListPosition(QPointF &position, HbPopup::Placement &placement)
{
    Q_Q(HbInputVkbWidget);

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup *>(q->contentItem());
    if (buttonGroup) {
        HbInputButton *item = buttonGroup->button(HbInputButton::ButtonKeyCodeSettings);
        if (item) {
            position.setX(buttonGroup->scenePos().x() + item->boundingRect().x() + item->boundingRect().width());
            position.setY(buttonGroup->scenePos().y() + item->boundingRect().y());
            placement = HbPopup::BottomRightCorner;
        }
    }
}

void HbInputVkbWidgetPrivate::captureScreenshot()
{
    Q_Q(HbInputVkbWidget);

    if (!mScreenshotWidget) {
        mScreenshotWidget = new HbInputScreenshotWidget();
        mScreenshotWidget->setZValue(q->zValue());
        mScreenshotWidget->setGeometry(q->geometry());
        q->mainWindow()->scene()->addItem(mScreenshotWidget);
    }

    QPointF position = q->pos();
    QRectF rect = QRectF(position.x(), position.y() + mCloseHandleHeight, q->boundingRect().width(), q->boundingRect().height() - mCloseHandleHeight);
    QTransform rotateTrans;
    rotateTrans = q->mainWindow()->viewportTransform();
    QRectF transRect = rotateTrans.mapRect(rect);
    QPixmap pixmap;
    pixmap = QPixmap::grabWidget(q->mainWindow(), (int)transRect.x(), (int)transRect.y(), (int)transRect.width(), (int)transRect.height());
    pixmap = pixmap.transformed(rotateTrans.inverted());
    mScreenshotWidget->setScreenshot(pixmap);
}


void HbInputVkbWidgetPrivate::updateMouseHitItem(HbTouchKeypadButton *button, QPointF position)
{
    mMostRecentlyAccessedButton = button;
    mMostRecentlyClickedLocation = position;
}

void HbInputVkbWidgetPrivate::normalizeProbabilities(QList<HbKeyPressProbability> &allProbableKeys)
{
    qreal sum = 0.0;
    foreach(const HbKeyPressProbability &key, allProbableKeys) {
        sum += key.probability;
    }

    for (int count = 0; count < allProbableKeys.size(); count++) {
        allProbableKeys[count].probability = allProbableKeys[count].probability / sum;
    }
}

bool HbInputVkbWidgetPrivate::isKeyboardDimmed()
{
    return mKeyboardDimmed;
}

bool HbInputVkbWidgetPrivate::isSmileysEnabled()
{
    bool ret = false;
    if (!mOwner || !mOwner->focusObject()) {
        return ret;
    }
    if (mOwner->focusObject()->editorInterface().editor()->inherits("HbAbstractEdit")) {
        if (!mOwner->focusObject()->editorInterface().smileyTheme().isNull()) {
            ret = true;
        }
    }

    return ret;
}

void HbInputVkbWidgetPrivate::_q_activateInputMethod(const HbInputMethodDescriptor &descriptor, const QByteArray &customData)
{
    Q_Q(HbInputVkbWidget);

    if (!descriptor.isEmpty() && mOwner) {
        // Set as active input method.
        HbInputSettingProxy::instance()->setPreferredInputMethod(q->mainWindow()->orientation(), descriptor, customData);
        // Activate immediately.
        mOwner->activateInputMethod(descriptor);
    }
}

void HbInputVkbWidgetPrivate::_q_smileyPickerClosed()
{
    Q_Q(HbInputVkbWidget);

    q->setKeyboardDimmed(false);
}

QChar HbInputVkbWidgetPrivate::numberCharacterBoundToKey(int key)
{
    QChar numChr;
    if (!mKeymap || !mOwner) {
        return numChr;
    }

    HbInputFocusObject *focusObject = mOwner->focusObject();
    if (!focusObject) {
        return numChr;
    }
    HbInputLanguage language = mKeymap->language();
    if (language.language()  != (QLocale::Language)0) {
        HbInputDigitType digitType = HbInputUtils::inputDigitType(language);

        // In number editors, show the native digits only when both device and writing languages are same,
        // else show latin digits
        if (focusObject->editorInterface().isNumericEditor()) {
            QLocale::Language systemLanguage = QLocale::system().language();
            if (language.language() != systemLanguage) {
                digitType = HbDigitTypeLatin;
            }
        }

        HbKeyboardType keyboardType = mOwner->inputState().keyboard();

        if (keyboardType == HbKeyboardVirtual12Key) {
            numChr = HbInputUtils::findFirstNumberCharacterBoundToKey(
                         mKeymap->keyboard(keyboardType)->keys.at(key),
                         language, digitType);
        } else if (keyboardType == HbKeyboardVirtualQwerty) {
            switch (digitType) {
            case HbDigitTypeLatin:
                numChr = HB_DIGIT_LATIN_START_VALUE + key;
                break;
            case HbDigitTypeArabicIndic:
                numChr = HB_DIGIT_ARABIC_INDIC_START_VALUE + key;
                break;
            case HbDigitTypeEasternArabic:
                numChr = HB_DIGIT_EASTERN_ARABIC_START_VALUE + key;
                break;
            case HbDigitTypeDevanagari:
                numChr = HB_DIGIT_DEVANAGARI_START_VALUE + key;
                break;
            default:
                break;
            }
        }
    }
    return numChr;
}

void HbInputVkbWidgetPrivate::_q_settingsClosed(HbAction* action)
{
    Q_UNUSED(action);
    Q_Q(HbInputVkbWidget);

    q->settingsClosed();
}

/// @endcond

/*!
Costructs the object.
*/
HbInputVkbWidget::HbInputVkbWidget(QGraphicsItem *parent)
    : HbWidget(*new HbInputVkbWidgetPrivate, parent)
{
    Q_D(HbInputVkbWidget);
    d->q_ptr = this;
    d->initLayout();
    d->init();

    setFocusPolicy(Qt::ClickFocus);
    setPos(QPointF(0, 0));

#ifdef HB_EFFECTS
    HbEffect::disable(this);
#endif // HB_EFFECTS

    setActive(false);

    if (!d->mOwner) {
        d->mOwner = HbInputMethod::activeInputMethod();
    }
}

/*!
Constructs the object.
*/
HbInputVkbWidget::HbInputVkbWidget(HbInputVkbWidgetPrivate &dd, QGraphicsItem *parent)
    : HbWidget(dd, parent)
{
    Q_D(HbInputVkbWidget);
    d->q_ptr = this;
    d->initLayout();
    d->init();

    setFocusPolicy(Qt::ClickFocus);
    setPos(QPointF(0, 0));

#ifdef HB_EFFECTS
    HbEffect::disable(this);
#endif // HB_EFFECTS

    setActive(false);
}

/*!
Destructs the object.
*/
HbInputVkbWidget::~HbInputVkbWidget()
{
}

/*!
Vkb host calls this handler when the keypad open animation finishes.
*/
void HbInputVkbWidget::keyboardOpened(HbVkbHost *host)
{
    Q_D(HbInputVkbWidget);

    d->mCurrentHost = host;
    d->mFlickDirection = HbFlickDirectionNone;
}

/*!
Vkb host calls this handler when the keyboard close animation has finished.

\sa HbVkbHost
*/
void HbInputVkbWidget::keyboardClosed(HbVkbHost *host)
{
    Q_UNUSED(host);
    Q_D(HbInputVkbWidget);

    d->mFlickDirection = HbFlickDirectionNone;
}

/*!
Vkb host calls this handler when the keyboard minimize animation has finished.

\sa HbVkbHost
*/
void HbInputVkbWidget::keyboardMinimized(HbVkbHost *host)
{
    Q_UNUSED(host);
    Q_D(HbInputVkbWidget);
    d->mFlickDirection = HbFlickDirectionNone;
}

/*!
The paint method. Draws the widget.
*/
void HbInputVkbWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    Q_D(HbInputVkbWidget);

    QRectF rect = boundingRect();
    if (d->mDrawbackground) {
        d->mBackgroundDrawer->paint(painter, rect);
    }

    painter->save();
    painter->translate(rect.width() / 2 - d->mCloseHandleWidth / 2, 0);
    rect.setWidth(d->mCloseHandleWidth);
    rect.setHeight(d->mCloseHandleHeight);
    d->mIconDrawer->paint(painter, rect);
    painter->restore();
}

/*!
Sets the content item which will fill the content area of this widget. Content item is
a single input button group by default.
If null is given then the old content item is deleted and content area is cleared.
Takes ownership of the given item.

\sa updateKeyCodes
\sa updateButtons
\sa setKeyboardDimmed
\sa probableKeypresses
*/
void HbInputVkbWidget::setContentItem(QGraphicsLayoutItem *item)
{
    Q_D(HbInputVkbWidget);
    if (d->mLayout->count() > 1) {
        QGraphicsLayoutItem *oldItem = d->mLayout->itemAt(d->mLayout->count() - 1);
        d->mLayout->removeItem(oldItem);
        delete oldItem;
    }
    if (item) {
        d->mLayout->addItem(item);
    }
}

/*!
Returns current content item or null if not set.
Ownership is not transferred.
*/
QGraphicsLayoutItem *HbInputVkbWidget::contentItem() const
{
    Q_D(const HbInputVkbWidget);

    QGraphicsLayoutItem *item(0);
    if (d->mLayout->count() > 1) {
        item = d->mLayout->itemAt(d->mLayout->count() - 1);
    }
    return item;
}

/*!
\deprecated HbInputVkbWidget::setRockerVisible(bool)
    is deprecated.

Sets virtual rocker visibility.
*/
void HbInputVkbWidget::setRockerVisible(bool visible)
{
    Q_UNUSED(visible);
}

/*!
\deprecated HbInputVkbWidget::isRockerVisible() const
    is deprecated.

Returns true if virtual rocker is allowed to be visible.
*/
bool HbInputVkbWidget::isRockerVisible() const
{
    return false;
}

/*!
Returns active keypad mode. Possible values are EModeAbc, EModeNumeric and EModeSct.
*/
HbKeypadMode HbInputVkbWidget::mode() const
{
    Q_D(const HbInputVkbWidget);
    return d->mMode;
}

/*!
Returns active keypad modifiers.
*/
HbModifiers HbInputVkbWidget::modifiers() const
{
    Q_D(const HbInputVkbWidget);
    return d->mModifiers;
}

/*!
Sets the keypad to given mode. Possible values are EModeAbc, EModeNumeric and EModeSct.
*/
void HbInputVkbWidget::setMode(HbKeypadMode mode, HbModifiers modifiers)
{
    Q_D(HbInputVkbWidget);
    d->mMode = mode;
    d->mModifiers = modifiers;

    d->updateButtons();
    d->updateKeyCodes();
    d->applyEditorConstraints();

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup *>(contentItem());
    if (buttonGroup && d->mOwner->focusObject()) {
        buttonGroup->setCustomButtonActions(d->mOwner->focusObject()->editorInterface().actions());
    }

    if (d->mInputModeIndicator) {
        d->mInputModeIndicator->updateIndicator();
    }
}

/*!
Sets key map data object. Given key map data will be used as a source for button titles.
Usually the key map data for active input language is used.
*/
void HbInputVkbWidget::setKeymap(const HbKeymap *keymap)
{
    Q_D(HbInputVkbWidget);
    if (keymap) {
        d->mKeymap = keymap;
        d->updateKeyCodes();

        HbInputState newState = d->mOwner->inputState();
        if (newState.textCase() == HbTextCaseUpper || newState.textCase() == HbTextCaseAutomatic) {
            setMode(d->mMode, HbModifierShiftPressed);
        } else {
            setMode(d->mMode, HbModifierNone);
        }
    }
}

/*!
\reimp
*/
void HbInputVkbWidget::aboutToOpen(HbVkbHost *host)
{
    Q_D(HbInputVkbWidget);

    d->mCurrentHost = host;

    if (d->mOwner && d->mOwner->focusObject()) {
        qreal vkbZValue = d->mOwner->focusObject()->findVkbZValue();
        setZValue(vkbZValue);
    }

    show();
}

/*!
\reimp
*/
void HbInputVkbWidget::aboutToClose(HbVkbHost *host)
{
    Q_D(HbInputVkbWidget);

    d->mCurrentHost = host;

    if (d->mSmileyPicker && d->mSmileyPicker->isVisible()) {
        d->mSmileyPicker->hide();
    }
    if (d->mSettingList) {
        d->mSettingList->close();
    }
}

/*!
Enables or disables all buttons in the keyboard that have not been disabled directly.
*/
void HbInputVkbWidget::setKeyboardDimmed(bool dimmed)
{
    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup *>(contentItem());
    if (buttonGroup) {
        buttonGroup->setEnabled(!dimmed);
    }
}

/*!
Shows settings list
*/
void HbInputVkbWidget::showSettingList()
{
    Q_D(HbInputVkbWidget);
    HbPredictionFactory *predFactory = HbPredictionFactory::instance();

    d->mSettingsListOpen = true;
    d->captureScreenshot();

    if (!d->mSettingList) {
        d->mSettingList = new HbInputSettingList();
        connect(d->mSettingList, SIGNAL(inputSettingsButtonClicked()), this, SLOT(showSettingsView()));
        connect(d->mSettingList, SIGNAL(inputMethodSelected(const HbInputMethodDescriptor &, const QByteArray &)),
                this, SLOT(_q_activateInputMethod(const HbInputMethodDescriptor &, const QByteArray &)));
    }

    HbInputFocusObject *focusObject = d->mOwner->focusObject();
    if (focusObject) {
        if (focusObject->editorInterface().inputConstraints() & HbEditorConstraintLatinAlphabetOnly) {
            d->mSettingList->setLanguageSelectionEnabled(false);
        } else {
            d->mSettingList->setLanguageSelectionEnabled(true);
        }

        if (focusObject->editorInterface().isPredictionAllowed() &&
            predFactory->predictionEngineForLanguage(HbInputSettingProxy::instance()->globalInputLanguage())) {
            d->mSettingList->setPredictionSelectionEnabled(true);
        } else {
            d->mSettingList->setPredictionSelectionEnabled(false);
        }
    }

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup *>(contentItem());
    if (buttonGroup) {
        HbInputButton *item = buttonGroup->button(HbInputButton::ButtonKeyCodeSettings);
        if (item) {
            item->setState(HbInputButton::ButtonStateLatched);
            buttonGroup->setButton(item, HbInputButton::ButtonKeyCodeSettings);
        }
    }

    QPointF position;
    HbPopup::Placement placement;
    d->settingListPosition(position, placement);
    d->mSettingList->setPreferredPos(position, placement);
    d->mSettingList->updateSettingList();
    d->mSettingList->open(this, SLOT(_q_settingsClosed(HbAction*)));
}

/*!
Slot which is called when settings list is closed.
*/
void HbInputVkbWidget::settingsClosed()
{
    Q_D(HbInputVkbWidget);

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup *>(contentItem());
    if (buttonGroup) {
        HbInputButton *item = buttonGroup->button(HbInputButton::ButtonKeyCodeSettings);
        if (item) {
            item->setState(HbInputButton::ButtonStateReleased);
            buttonGroup->setButton(item, HbInputButton::ButtonKeyCodeSettings);
        }
    }

    d->mSettingsListOpen = false;
    if (d->mAnimateWhenDialogCloses) {
        animKeyboardChange();
        d->mAnimateWhenDialogCloses = false;
    } else if (d->mScreenshotTimeLine.state() != QTimeLine::Running) {
        keypadLanguageChangeFinished();
    }
}

/*!
Closes settings list
*/
void HbInputVkbWidget::closeSettingList()
{
    Q_D(HbInputVkbWidget);
    d->mSettingList->close();
}

/*!
Shows settings view
*/
void HbInputVkbWidget::showSettingsView()
{
    Q_D(HbInputVkbWidget);

    /*
    Added for vanilla input
    When settings dialog is launched, keypad is not closed.
    */
    HbInputFocusObject *focusObject = 0;
    if (!d->mOwner || !(focusObject = d->mOwner->focusObject())) {
        return;
    }
    HbVkbHost *vkbHost = focusObject->editorInterface().vkbHost();
    if (vkbHost && vkbHost->keypadStatus() != HbVkbHost::HbVkbStatusClosed) {
        vkbHost->closeKeypad();
    }
    d->mCurrentFocusedObject = focusObject->object();

    closeSettingList();
    if(!d->mSettingView) {
        d->mSettingView = new HbView(this);
        HbAction *backAction = new HbAction(Hb::BackNaviAction, d->mSettingView);
        backAction->setText(tr("Back"));
        connect(backAction, SIGNAL(triggered(bool)), this, SLOT(closeSettingsView()));
        d->mSettingView->setNavigationAction(backAction);
        HbDataForm *dataForm = new HbDataForm();
        d->mSettingView->setWidget(dataForm);
        d->mSettingWidget = new HbInputSettingWidget(dataForm, d->mSettingView);        
    }    
    d->mSettingWidget->initializeWidget();
    HbInputRegionCollector::instance()->attach(d->mSettingView);
    d->mSettingView->setTitle(tr("Input Settings"));
    mainWindow()->addView(d->mSettingView);
    d->mCurrentView = mainWindow()->currentView();
    mainWindow()->setCurrentView(d->mSettingView);
}

/*!
Closes settings view and returns to previous view
*/
void HbInputVkbWidget::closeSettingsView()
{
    Q_D(HbInputVkbWidget);

    mainWindow()->setCurrentView(d->mCurrentView);
    mainWindow()->removeView(d->mSettingView);
    if (d->mSettingView->scene()) {
        d->mSettingView->scene()->removeItem(d->mSettingView);
    }
    HbInputRegionCollector::instance()->detach(d->mSettingView);
    d->mSettingWidget->resetWidget();

    if (d->mCurrentFocusedObject) {
        HbInputFocusObject *focusObject = new HbInputFocusObject(d->mCurrentFocusedObject);
        d->mOwner->setFocusObject(focusObject);
        d->mCurrentFocusedObject = 0;
    }
}

/*!
\deprecated HbInputVkbWidget::executeMethodDialog()
    is deprecated.
Executes input method selection dialog
*/
void HbInputVkbWidget::executeMethodDialog()
{
}

/*!
\reimp
*/
QWidget *HbInputVkbWidget::asWidget()
{
    return HbInputUtils::createWrapperWidget(this);
}

/*!
\reimp
*/
QGraphicsWidget *HbInputVkbWidget::asGraphicsWidget()
{
    return this;
}

/*!
\reimp
*/
QSizeF HbInputVkbWidget::preferredKeyboardSize()
{
    Q_D(HbInputVkbWidget);

    if (d->mCurrentHost) {
        // Just rely on the host and return what it suggests.
        QSizeF rect = d->mCurrentHost->keyboardArea();
        return rect;
    }

    return QSizeF(0.0, 0.0);
}

/*!
\reimp
*/
void HbInputVkbWidget::keyboardAnimationFrame(HbVkbAnimationType type, qreal x)
{
    Q_UNUSED(type);
    Q_UNUSED(x);
}

/*!
Returns the size of the keypad button area.
*/
QSizeF HbInputVkbWidget::keypadButtonAreaSize()
{
    Q_D(HbInputVkbWidget);
    QSizeF ret = preferredKeyboardSize();
    if (ret.height() >  d->mCloseHandleHeight) {
        ret.setHeight(ret.height() - d->mCloseHandleHeight);
    }

    return ret;
}

/*!
\deprecated HbInputVkbWidget::rockerPosition()
    is deprecated.

Returns the virtual rocker position. The default position is in the middle
of keypad button area.
*/
QPointF HbInputVkbWidget::rockerPosition()
{
    return QPointF();
}

/*!
Sets the status of the background drawing. This method can be used to
optimize vkb widget drawing. If it is known that the widget will cover whole
vkb area and there are no places where the background shows through, then the background
drawing can be turned off to speed up paint method.
*/
void HbInputVkbWidget::setBackgroundDrawing(bool backgroundEnabled)
{
    Q_D(HbInputVkbWidget);
    d->mDrawbackground = backgroundEnabled;
}


/*!
Returns all possible keys that the user could have intended to press
for the last registered touch along with their corresponding probability.
*/
QList<HbKeyPressProbability> HbInputVkbWidget::probableKeypresses()
{
    QList<HbKeyPressProbability> probabilities;
    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup *>(contentItem());
    if (buttonGroup) {
        probabilities = buttonGroup->buttonProbabilities();
    }
    return probabilities;
}

/*!
Refines the bounding rect. This function is used for collision detection
and hit test.
*/
QPainterPath HbInputVkbWidget::shape() const
{
    QRectF rect = boundingRect();
    QPainterPath path;
    path.addRect(rect);
    return path;
}

/*!
\reimp
*/
QSizeF HbInputVkbWidget::minimizedKeyboardSize()
{
    Q_D(HbInputVkbWidget);
    return QSizeF(0.0, d->mCloseHandleHeight);
}

/*!
Shows smiley picker widget.
*/
void HbInputVkbWidget::showSmileyPicker(int rows, int columns)
{
    Q_D(HbInputVkbWidget);
    if (!d->mOwner || !d->mOwner->focusObject()) {
        return;
    }
    // check whether the smiley recognition is enabled  in the currently focused editor.
    if (!d->isSmileysEnabled()) {
        return;
    }
    HbInputFocusObject *focusObject = d->mOwner->focusObject();

    if (!d->mSmileyPicker || d->mFocusedObject != focusObject) {
        d->mFocusedObject = focusObject;
        if (d->mSmileyPicker) {
            delete d->mSmileyPicker;
        }
        // get the smiley list from editor interface smiley theme.
        QStringList smileys = focusObject->editorInterface().smileyTheme().smileys();

        if (!smileys.isEmpty()) {
            d->mSmileyPicker = new HbInputSmileyPicker(rows, columns, 0, smileys);
            d->mSmileyPicker->setObjectName("vkbwidget_smiley_picker");
            connect(d->mSmileyPicker, SIGNAL(selected(QString)), this, SIGNAL(smileySelected(QString)));
        }
    }

    if (d->mSmileyPicker) {
        qreal unitValue = HbDeviceProfile::profile(mainWindow()).unitValue();
        QSizeF screenSize = HbDeviceProfile::profile(mainWindow()).logicalSize();

        qreal width = HbPortraitSmileyPickerWidthInUnits * unitValue;
        qreal height = HbPortraitSmileyPickerHeightInUnits * unitValue;
        if (mainWindow()->orientation() == Qt::Horizontal) {
            width = HbLandscapeSmileyPickerWidthInUnits * unitValue;
            height = HbLandscapeSmileyPickerHeightInUnits * unitValue;
        }

        d->mSmileyPicker->setPreferredSize(QSizeF(width, height));
        d->mSmileyPicker->setPos((screenSize.width() - width) * 0.5,
                                 screenSize.height() - height - HbSmileyPickerMarginInUnits * unitValue);
        d->mSmileyPicker->open(this, SLOT(_q_smileyPickerClosed()));
        setKeyboardDimmed(true);

        HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup *>(contentItem());
        if (buttonGroup) {
            buttonGroup->cancelButtonPress();
        }
    }
}

HbInputVkbWidget::HbFlickDirection HbInputVkbWidget::flickDirection()
{
    Q_D(HbInputVkbWidget);
    return d->mFlickDirection;
}

/*!
Sends key event to owning input method.
*/
void HbInputVkbWidget::sendKeyPressEvent(const QKeyEvent &event)
{
    Q_D(HbInputVkbWidget);

    if (d->mOwner) {
        d->mOwner->filterEvent(&event);
    }
}

/*!
Sends key event to owning input method.
*/
void HbInputVkbWidget::sendKeyDoublePressEvent(const QKeyEvent &event)
{
    Q_D(HbInputVkbWidget);

    if (d->mOwner) {
        d->mOwner->filterEvent(&event);
    }
}

/*!
Sends key event to owning input method.
*/
void HbInputVkbWidget::sendKeyReleaseEvent(const QKeyEvent &event)
{
    Q_D(HbInputVkbWidget);

    if (d->mOwner && event.key() > 0) {
        d->mOwner->filterEvent(&event);
    }

    if (event.key() == HbInputButton::ButtonKeyCodeSettings) {
        showSettingList();
    }
}

/*!
Sends key event to owning input method.
*/
void HbInputVkbWidget::sendLongPressEvent(const QKeyEvent &event)
{
    Q_D(HbInputVkbWidget);

    if (d->mOwner) {
        d->mOwner->filterEvent(&event);
    }
}

/*!
Sends key event to owning input method.
Releae event is ignored.
*/
void HbInputVkbWidget::sendKeyChangeEvent(const QKeyEvent &releaseEvent, const QKeyEvent &pressEvent)
{
    Q_D(HbInputVkbWidget);
    Q_UNUSED(releaseEvent);

    if (d->mOwner) {
        d->mOwner->filterEvent(&pressEvent);
    }
}

void HbInputVkbWidget::keypadLanguageChangeAnimationUpdate(qreal value)
{
    Q_D(HbInputVkbWidget);

    int direction = 1;
    if (d->mFlickDirection == HbFlickDirectionLeft) {
        direction = -1;
    }

    QRectF rect = boundingRect();
    QPointF position = pos();
    position.setX(direction *(-rect.width() + rect.width() * value));
    if (d->mScreenshotWidget) {
        d->mScreenshotWidget->setPos(position.x() + direction * rect.width(), position.y());
        setPos(position);
    }
}

void HbInputVkbWidget::keypadLanguageChangeFinished()
{
    Q_D(HbInputVkbWidget);
    delete d->mScreenshotWidget;
    d->mScreenshotWidget = 0;
    d->mFlickDirection = HbFlickDirectionNone;
}

void HbInputVkbWidget::animKeyboardChange()
{
    Q_D(HbInputVkbWidget);
    if (mainWindow()) {
        if (d->mSettingsListOpen) {
            d->mAnimateWhenDialogCloses = true;
        } else {
            if (!d->mAnimateWhenDialogCloses) {
                d->captureScreenshot();
            }
            connect(&d->mScreenshotTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(keypadLanguageChangeAnimationUpdate(qreal)));
            connect(&d->mScreenshotTimeLine, SIGNAL(finished()), this, SLOT(keypadLanguageChangeFinished()));
            d->mScreenshotTimeLine.start();
        }
    }
}

/*!
\reimp
*/
QSizeF HbInputVkbWidget::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(constraint);
    Q_D(const HbInputVkbWidget);

    QSizeF sh;
    switch (which) {
    case Qt::MinimumSize:
        sh = QSizeF(0, 0);
        break;
    case Qt::PreferredSize:
        if (d->mCurrentHost) {
            sh = d->mCurrentHost->keyboardArea();
        }
        break;
    case Qt::MaximumSize:
        sh = QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        break;
    default:
        qWarning("HbInputVkbWidget::sizeHint(): Don't know how to handle the value of 'which'");
        break;
    }
    return sh;
}

/*!
\reimp
 */
void HbInputVkbWidget::changeEvent(QEvent *event)
{
    Q_D(HbInputVkbWidget);
    if (event->type() == HbEvent::ThemeChanged) {
        d->mBackgroundDrawer->themeChanged();
        d->mIconDrawer->themeChanged();
    }
    HbWidget::changeEvent(event);
}

/*!
\reimp
*/
void HbInputVkbWidget::gestureEvent(QGestureEvent *event)
{
    Q_D(HbInputVkbWidget);

    if (HbSwipeGesture *gesture = qobject_cast<HbSwipeGesture *>(event->gesture(Qt::SwipeGesture))) {
        if (gesture->state() == Qt::GestureFinished) {
            HbWidgetFeedback::triggered(this, Hb::InstantFlicked);
            // vertical swipes
            if (gesture->sceneVerticalDirection() == QSwipeGesture::Down) {
                d->mFlickDirection = HbFlickDirectionDown;
                emit flickEvent(d->mFlickDirection);
                emit keypadCloseEventDetected(HbVkbCloseMethodCloseGesture);
            } else if (gesture->sceneVerticalDirection() == QSwipeGesture::Up) {
                d->mFlickDirection = HbFlickDirectionUp;
                emit flickEvent(d->mFlickDirection);
                d->mCurrentHost->openKeypad(d->mCurrentHost->activeKeypad(), d->mOwner);
            } else {
                d->mFlickDirection = (HbInputVkbWidget::HbFlickDirection)gesture->sceneHorizontalDirection();
                // horizontal swipes
                if (d->mFlickAnimation) {
                    animKeyboardChange();
                }
                emit flickEvent(d->mFlickDirection);
            }
        }
    } else if (HbTapGesture *gesture = qobject_cast<HbTapGesture *>(event->gesture(Qt::TapGesture))) {
        if (gesture->state() == Qt::GestureFinished) {
            // if keypad is minimized, open it
            if (d->mCurrentHost && d->mCurrentHost->keypadStatus() == HbVkbHost::HbVkbStatusMinimized) {
                d->mCurrentHost->openKeypad(this, d->mOwner);
            }
        }
    }
}

#include "moc_hbinputvkbwidget.cpp"

// End of file
