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
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QGraphicsItemAnimation>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsProxyWidget>
#include <QTimeLine>
#include <QGraphicsGridLayout>
#include <QGraphicsScene>
#include <QGraphicsLinearLayout>

#include <hbapplication.h>
#include <hbmainwindow.h>
#include <hbaction.h>
#include <hbview.h>
#include <hbwidget.h>
#include <hbpushbutton.h>
#include <hbinputsettingproxy.h>
#include <hbdialog.h>
#include <hbeffect.h>
#include <hbstackedwidget.h>
#include <hbframedrawer.h>
#include <hbevent.h>
#include <hbdataform.h>

#include <hbinputmethod.h>
#include <hbinputsettingproxy.h>
#include <hbinpututils.h>
#include <hbinputdef.h>
#include <hbinputvkbhost.h>
#include <hbinputsettingwidget.h>
#include <hbinputcommondialogs.h>
#include <hbinputkeymap.h>
#include <hbinputkeymapfactory.h>
#include <hbwidgetfeedback.h>
#include <hbinputpredictionfactory.h>
#include <HbSwipeGesture>
#include <HbTapGesture>
#include "hbinputvirtualrocker.h"
#include "hbinputvkbwidget.h"
#include "hbinputvkbwidget_p.h"
#include "hbinputtouchkeypadbutton.h"
#include "hbinputsettinglist.h"
#include "hbinputmodeindicator.h"
#include <hbfeedbackmanager.h>
#include "hbinputsmileypicker.h"
#include "hbinputscreenshotwidget.h"
const qreal HbMouseDragDelta = 0.4;
const qreal HbRockerWidth = 50.0;

const int MaxSweepTime = 500;
const int SweepLength = 150;


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

inline HbWidget* hbwidget_cast(QGraphicsItem *item)
{
    if( item->isWidget() && static_cast<QGraphicsWidget*>(item)->inherits("HbWidget") ) {
        return static_cast<HbWidget*>(item);
    }
    return 0;
}

HbInputVkbWidgetPrivate::HbInputVkbWidgetPrivate()
: mOwner(0),
mMode(EModeAbc),
mKeymap(0),
mModifiers(0),
mInputModeIndicator(0),
mApplicationButton(0),
mSettingsButton(0),
mSettingList(0),
mButtonLayout(0),
mRocker(0),
mBackgroundDrawer(0),
mIconDrawer(0),
mMainWinConnected(false),
mShowRocker(false),
mLayout(0),
mCurrentHost(0),
mDrawbackground(true),
mMouseButtonPressedDown(false),
mFlickDirection(HbInputVkbWidget::HbFlickDirectionNone),
mSmileyPicker(0),
mScreenshotWidget(0),
mScreenshotTimeLine(250),
mMostRecentlyAccessedButton(0),
mMostRecentlyClickedLocation(0.0,0.0),
mFocusedObject(0),
mFlickAnimation(false),
mSettingsListOpen(false),
mAnimateWhenDialogCloses(false),
mKeyboardSize(HbQwerty4x10),
mCloseHandleHeight(0),
mCloseHandle(0),
mSettingView(0),
mCurrentView(0),
mKeyboardDimmed(false)
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

void HbInputVkbWidgetPrivate::init()
{
    Q_Q(HbInputVkbWidget);

    mRocker = new HbInputVirtualRocker(q);
    mRocker->setObjectName("VirtualRocker");
    QSizeF rockerSize(HbRockerWidth, HbRockerWidth);
    mRocker->resize(rockerSize);
    mRocker->setMinimumSize(HbRockerWidth, HbRockerWidth);
    mRocker->setMaximumSize(HbRockerWidth*20, HbRockerWidth*20);

    QObject::connect(mRocker, SIGNAL(rockerDirection(int, HbInputVirtualRocker::RockerSelectionMode)),
        q, SIGNAL(rockerDirection(int, HbInputVirtualRocker::RockerSelectionMode)));

    mRocker->setVisible(false);

    mBackgroundDrawer = new HbFrameDrawer();
    mBackgroundDrawer->setFrameGraphicsName(backgroundGraphics);
    mBackgroundDrawer->setFrameType(HbFrameDrawer::ThreePiecesVertical);
    mBackgroundDrawer->setFillWholeRect(true);
    mBackgroundDrawer->setBorderWidths(0.0, HbCloseHandleHeight, 0.0, HbCloseHandleHeight);

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

void HbInputVkbWidgetPrivate::addCustomButtonToLayout( HbTouchKeypadButton* button,
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

void HbInputVkbWidgetPrivate::applyEditorConstraints() {
    // no default implementaiton as of now.
}

void HbInputVkbWidgetPrivate::setRockerPosition()
{
    Q_Q(HbInputVkbWidget);

    // Set rocker position.
    QSizeF padArea = q->keypadButtonAreaSize();
    QPointF point((padArea.width() * 0.5) - (mRocker->size().width() * 0.5),
        (padArea.height() * 0.5) - (mRocker->size().height() * 0.5));
    point.setY(point.y() + mCloseHandleHeight);

    if (q->keypadLayout() && q->keypadLayout()->geometry().height()) {
        point.setX(((padArea.width() * 0.5) - (mRocker->size().width() * 0.5)));
        point.setY(((q->keypadLayout()->geometry().height() * 0.5) - (mRocker->size().height() * 0.5) + mCloseHandleHeight));
    }
    mRocker->setPos(point);
}

void HbInputVkbWidgetPrivate::captureScreenshot()
{
    Q_Q(HbInputVkbWidget);

    if (!mScreenshotWidget) {
        mScreenshotWidget = new HbInputScreenshotWidget();
        mScreenshotWidget->setGeometry(q->geometry());
        q->mainWindow()->scene()->addItem(mScreenshotWidget);
    }

    QPointF position = q->pos();
    QRectF rect = QRectF(position.x(), position.y()+ mCloseHandleHeight, q->boundingRect().width(), q->boundingRect().height()- mCloseHandleHeight);
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
    foreach (HbKeyPressProbability key, allProbableKeys) {
        sum += key.probability;
    }

    for (int count=0;count<allProbableKeys.size();count++) {
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
/// @endcond

/*!
Costructs the object.
*/
HbInputVkbWidget::HbInputVkbWidget(QGraphicsItem* parent)
    : HbWidget(*new HbInputVkbWidgetPrivate, parent)
{
    Q_D(HbInputVkbWidget);
    d->q_ptr = this;
    d->init();

    setFocusPolicy(Qt::ClickFocus);
    setPos(QPointF(0,0));

#ifdef HB_EFFECTS
    HbEffect::disable(this);
#endif // HB_EFFECTS

    // Make sure the keypad never steals focus.
    setFlag(QGraphicsItem::ItemIsPanel, true);
    setActive(false);
}

/*!
Constructs the object.
*/
HbInputVkbWidget::HbInputVkbWidget(HbInputVkbWidgetPrivate& dd, QGraphicsItem* parent)
  : HbWidget(dd, parent)
{
    Q_D(HbInputVkbWidget);
    d->q_ptr = this;
    d->init();

    setFocusPolicy(Qt::ClickFocus);
    setPos(QPointF(0,0));

#ifdef HB_EFFECTS
    HbEffect::disable(this);
#endif // HB_EFFECTS

    // Make sure the keypad never steals focus.
    setFlag(QGraphicsItem::ItemIsPanel, true);
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
    d->mRocker->setVisible(d->mShowRocker);
    d->setRockerPosition();
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

    d->mRocker->setVisible(false);
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
\deprecated HbInputVkbWidget::mousePressEvent(QGraphicsSceneMouseEvent*)
    is deprecated.
*/
void HbInputVkbWidget::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);
}

/*!
\deprecated HbInputVkbWidget::mappedKeyPress(int)
    is deprecated.
*/
void HbInputVkbWidget::mappedKeyPress(int buttonid)
{
    Q_D(HbInputVkbWidget);
    d->handleStandardButtonPress(buttonid);
}

/*!
\deprecated HbInputVkbWidget::mappedKeyRelease(int)
    is deprecated.
*/
void HbInputVkbWidget::mappedKeyRelease(int buttonid)
{
    Q_D(HbInputVkbWidget);
    d->handleStandardButtonRelease(buttonid);
}


/*!
The paint method. Draws the widget.
*/
void HbInputVkbWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    Q_D(HbInputVkbWidget);

    QRectF rect = boundingRect();
    if (d->mDrawbackground) {
        d->mBackgroundDrawer->paint(painter, rect);
    }

    rect.setLeft(rect.width()/2 - d->mCloseHandleHeight*3);
    rect.setWidth(d->mCloseHandleHeight*6);
    rect.setHeight(d->mCloseHandleHeight);
    d->mIconDrawer->paint(painter, rect);
}

/*!
Sets virtual rocker visibility.
*/
void HbInputVkbWidget::setRockerVisible(bool visible)
{
    Q_D(HbInputVkbWidget);
    d->mShowRocker = visible;
}

/*!
Returns true if virtual rocker is allowed to be visible.
*/
bool HbInputVkbWidget::isRockerVisible() const
{
    Q_D(const HbInputVkbWidget);
    if (d->mShowRocker) {
        return d->mRocker->isVisible();
    } else {
        return false;
    }
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
    d->mModifiers = modifiers;
    d->mMode = mode;
}

/*!
Sets key map data object. Given key map data will be used as a source for button titles.
Usually the key map data for active input language is used.
*/
void HbInputVkbWidget::setKeymap(const HbKeymap* keymap)
{
    Q_D(HbInputVkbWidget);
    if (keymap) {
        d->mKeymap = keymap;
    }
}

/*!
\reimp
*/
void HbInputVkbWidget::aboutToOpen(HbVkbHost *host)
{
    Q_D(HbInputVkbWidget);

    d->mCurrentHost = host;

    if (!d->mLayout) {
        // get preferred size from vkbhost and set it to vkb.
        // Keypad buttons will flicker while vkb opening (when we open keypad for first time )
        // if we dont set size to vkb before seting layout to vkb.
        resize(preferredKeyboardSize());
        d->mLayout = new QGraphicsLinearLayout(Qt::Vertical);
        d->mLayout->setContentsMargins(0.0, 0.0, 0.0, 0.0);
        d->mLayout->setSpacing(0.0);

        setLayout(d->mLayout);

        d->mCloseHandle = new QGraphicsWidget();
        d->mCloseHandle->setObjectName("vkbHandle");
        d->mCloseHandleHeight = (int)HbCloseHandleHeight;
        d->mCloseHandle->setMinimumHeight(d->mCloseHandleHeight);
        d->mCloseHandle->setMaximumHeight(d->mCloseHandleHeight);

        d->mLayout->addItem(d->mCloseHandle);
        d->mLayout->addItem(keypadLayout());
    }


    if (d->mOwner && d->mOwner->focusObject()) {
        qreal vkbZValue = d->mOwner->focusObject()->findVkbZValue();
        setZValue(vkbZValue);
        d->mRocker->setZValue(vkbZValue+0.5);
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

    d->mRocker->setVisible(false);
    if (d->mSettingList) {
        d->mSettingList->close();
    }
}

/*!
Enables or disabled all buttons in the keyboard that have not been disabled directly.
*/
void HbInputVkbWidget::setKeyboardDimmed(bool dimmed)
{
    Q_D(HbInputVkbWidget);
    d->mKeyboardDimmed = dimmed;
    if (keypadLayout()) {
        int itemCount = keypadLayout()->count();
        for (int i=0; i<itemCount; i++) {
            HbTouchKeypadButton* button = static_cast<HbTouchKeypadButton*>(keypadLayout()->itemAt(i));
            button->setFade(dimmed);
        }
    }
    if (!dimmed) {
        // when we undimmed the keyboard, all the buttons will be enabled by default.
        // we need to call applyEditorConstraints on the keyboard to apply constraints
        d->applyEditorConstraints();
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
        connect(d->mSettingList, SIGNAL(inputMethodsButtonClicked()), this, SLOT(executeMethodDialog()));
    }

    HbInputFocusObject *focusObject = d->mOwner->focusObject();
    if (focusObject &&
        focusObject->editorInterface().isPredictionAllowed() &&
        predFactory->predictionEngineForLanguage(HbInputSettingProxy::instance()->globalInputLanguage())) {
        d->mSettingList->setPredictionSelectionEnabled(true);
    } else {
        d->mSettingList->setPredictionSelectionEnabled(false);
    }

    d->mSettingsButton->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonLatched);
    qreal x = d->mSettingsButton->scenePos().x() + d->mSettingsButton->rect().width();
    qreal y = d->mSettingsButton->scenePos().y();
    d->mSettingList->setPreferredPos(QPointF(x, y), HbPopup::BottomRightCorner);
    d->mSettingList->updateSettingList();
    d->mSettingList->open(this, SLOT(settingsClosed()));
}

/*!
Slot which is called when settings list is closed.
*/
void HbInputVkbWidget::settingsClosed()
{
    Q_D(HbInputVkbWidget);

    d->mSettingsButton->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);

    d->mSettingsListOpen = false;
    if ( d->mAnimateWhenDialogCloses ) {
        animKeyboardChange();
        d->mAnimateWhenDialogCloses = false;
    } else if(d->mScreenshotTimeLine.state() != QTimeLine::Running) {
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
    d->mSettingsButton->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
}

/*!
\deprecated HbInputVkbWidget::togglePredictionStatus()
    is deprecated.
*/
void HbInputVkbWidget::togglePredictionStatus()
{
    closeSettingList();
    HbInputSettingProxy::instance()->togglePrediction();
    update();
}

/*!
\deprecated HbInputVkbWidget::executeSettingsDialog()
    is deprecated. Use showSettingsView instead.
*/
void HbInputVkbWidget::executeSettingsDialog()
{
}

/*!
Shows settings view
*/
void HbInputVkbWidget::showSettingsView()
{
    Q_D(HbInputVkbWidget);

    closeSettingList();

    d->mSettingView = new HbView(this);
    d->mSettingView->setTitle(tr("Input Settings"));
    mainWindow()->addView(d->mSettingView);

    HbAction *backAction = new HbAction(Hb::BackNaviAction, d->mSettingView);
    backAction->setText(tr("Back"));
    connect(backAction, SIGNAL(triggered(bool)), this, SLOT(closeSettingsView()));
    d->mSettingView->setNavigationAction(backAction);

    HbDataForm *dataForm = new HbDataForm();
    d->mSettingView->setWidget(dataForm);
    HbInputSettingWidget *settingWidget = new HbInputSettingWidget(dataForm, d->mSettingView);
    settingWidget->initializeWidget();

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
    delete d->mSettingView;
    d->mSettingView = 0;
}

/*!
Executes input method selection dialog
*/
void HbInputVkbWidget::executeMethodDialog()
{
    Q_D(HbInputVkbWidget);

    closeSettingList();
    HbInputMethodDescriptor method
        = HbInputCommonDialogs::showCustomInputMethodSelectionDialog(HbInputSettingProxy::instance()->globalInputLanguage());
    if (!method.isEmpty() && d->mOwner) {
        d->mOwner->activateInputMethod(method);
    }
}

/*!
\deprecated HbInputVkbWidget::keypadLayout()
    is deprecated.
*/
QGraphicsLayout *HbInputVkbWidget::keypadLayout()
{
    Q_D(HbInputVkbWidget);
    return d->mButtonLayout;
}

/*!
\reimp
*/
QWidget* HbInputVkbWidget::asWidget()
{
    return HbInputUtils::createWrapperWidget(this);
}

/*!
\reimp
*/
QGraphicsWidget* HbInputVkbWidget::asGraphicsWidget()
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

    Q_D(HbInputVkbWidget);
    d->setRockerPosition();
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
Returns all possible keys those the user could have intended to press
for the last registered touch along with their corresponding probability.
One issue of thecurrent API implementation is that it does not always
sum up the probabilities to 1.0, but sometimes it returns.999999899 etc.
Need to be careful about it!
*/
QList<HbKeyPressProbability> HbInputVkbWidget::probableKeypresses()
{
    Q_D(HbInputVkbWidget);

    QList<HbKeyPressProbability> probableKeys;
    int totalItems = d->mButtonLayout->count();
    QRectF buttonRect = d->mMostRecentlyAccessedButton->geometry();
    //The overlaying rectangle is the test rectangle that is used for finding
    //intersactions with other buttons.
    QRectF overlayingRect(d->mMostRecentlyClickedLocation.x()-buttonRect.width()/2,d->mMostRecentlyClickedLocation.y()-buttonRect.height()/2, buttonRect.width(), buttonRect.height());
    QPainterPath path(overlayingRect.topLeft());
    path.addRect(overlayingRect);
    for(int count=0; count < totalItems; count++) {
        QGraphicsItem *item = d->mButtonLayout->itemAt(count)->graphicsItem();
        QPainterPath testPath = item->mapFromScene(path);
        HbTouchKeypadButton *buttonItem = 0;
        if(item->isWidget()){
            buttonItem =  qobject_cast<HbTouchKeypadButton *>(static_cast<QGraphicsWidget *>(item));
        }
        if(!buttonItem) {
            continue;
        }

        //Checkif the button collides with the path, if yes,it means that the colliding
        //key also could have been clicked by the user. The probability of the button being
        //clickedin that case willdependon the area of the intersaction rectangle.
        if(buttonItem->collidesWithPath(testPath)) {
            //Initiallylet the intersaction be same as the overlaying rectangle,later we will
            //shrink the rectangle and find the actual intersected rectangle.
            QRectF intersactionRect = overlayingRect;
            //The overlaying rectangle is in scene coordinates, map it to the item coordinates.
            intersactionRect.moveTopLeft(buttonItem->mapFromScene(overlayingRect.topLeft()));
            int width = (int)intersactionRect.width();
            int height = (int)intersactionRect.height();
            qreal probability = 0.0;
            //Shrink based on the size of the intersaction
            if (intersactionRect.topLeft().x() > 0) {
                width -= (int)intersactionRect.topLeft().x();
            } else {
                width += (int)intersactionRect.topLeft().x();
            }
            if (intersactionRect.topLeft().y() > 0) {
                height -= (int)intersactionRect.topLeft().y();
            } else {
                height += (int)intersactionRect.topLeft().y();
            }
            //The probabilty of the key is based on the intersaction area.
            probability = (height * width) / (intersactionRect.width()* intersactionRect.height());
            HbKeyPressProbability probablekey;
            probablekey.keycode = d->keyCode(buttonItem);
            probablekey.probability = probability;

            if(probablekey.keycode && (probablekey.probability>0)) {
                probableKeys.append(probablekey);
            }
        }
    }
    //Normalize makes sure that all probability summation is 1.0.
    d->normalizeProbabilities(probableKeys);
    return probableKeys;
}

/*!
\deprecated HbInputVkbWidget::setupToolCluster()
    is deprecated.
*/
void HbInputVkbWidget::setupToolCluster()
{
    Q_D(HbInputVkbWidget);
    if(!d->mOwner || !d->mOwner->focusObject()) {
        return;
    }

    // Create settings button if it does not exist
    if (!d->mSettingsButton) {
        d->mSettingsButton = new HbTouchKeypadButton(this, QString(""));
        d->mInputModeIndicator = new HbInputModeIndicator(*d->mSettingsButton, this);
        d->mSettingsButton->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
        d->mSettingsButton->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);

        connect(d->mSettingsButton, SIGNAL(clicked()), this, SLOT(showSettingList()));

        connect(d->mSettingsButton, SIGNAL(pressed()), d->mPressMapper, SLOT(map()));
        connect(d->mSettingsButton, SIGNAL(released()), d->mReleaseMapper, SLOT(map()));
        d->mPressMapper->setMapping(d->mSettingsButton, -1);
        d->mReleaseMapper->setMapping(d->mSettingsButton, -1);
    } else {
        d->mInputModeIndicator->updateIndicator();
    }

    // If there's a application specific button defined, create new button with the properties
    // or update the existing one. Otherwise create an empty button or clean the properties of an existing one.
    if (!d->mOwner->focusObject()->editorInterface().actions().isEmpty()) {
        QList<HbAction*> actions = d->mOwner->focusObject()->editorInterface().actions();
        if (d->mApplicationButtonAction != actions.first()) {
            if (d->mApplicationButton) {
                d->mApplicationButton->setText(actions.first()->text());
                d->mApplicationButton->setIcon(actions.first()->icon());
                d->mApplicationButton->disconnect(SIGNAL(clicked()));
                d->mApplicationButton->disconnect(SIGNAL(pressed()));
                d->mApplicationButton->disconnect(SIGNAL(released()));

                // disconnects old signal
                disconnect(d->mApplicationButtonAction, SIGNAL(changed()), this, SLOT(refreshApplicationButton()));
                disconnect(d->mApplicationButton, SIGNAL(clicked()), d->mApplicationButtonAction, SLOT(trigger()));
            } else {
                d->mApplicationButton = new HbTouchKeypadButton(this, actions.first()->icon(), actions.first()->text());
            }
            d->mApplicationButtonAction = actions.first();
            // Connect to enabling signal and check its value
            connect(actions.first(), SIGNAL(changed()), this, SLOT(refreshApplicationButton()));

            if (actions.first()->isEnabled()) {
                // action is enabled
                connect(d->mApplicationButton, SIGNAL(clicked()), d->mApplicationButtonAction, SLOT(trigger()));

                connect(d->mApplicationButton, SIGNAL(pressed()), d->mPressMapper, SLOT(map()));
                connect(d->mApplicationButton, SIGNAL(released()), d->mReleaseMapper, SLOT(map()));
                d->mPressMapper->setMapping(d->mApplicationButton, -1);
                d->mReleaseMapper->setMapping(d->mApplicationButton, -1);

                d->mApplicationButton->setToolTip(actions.first()->toolTip());
                d->mApplicationButton->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
                d->mApplicationButton->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
            } else {
                // action is disabled
                d->mApplicationButton->setButtonType(HbTouchKeypadButton::HbTouchButtonFnInActive);
                d->mApplicationButton->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonPressed);
            }
            d->mApplicationButton->setToolTip(actions.first()->toolTip());
        }
    } else {
        if (d->mApplicationButton) {
            if (d->mApplicationButtonAction) {
                disconnect(d->mApplicationButtonAction, SIGNAL(changed()), this, SLOT(refreshApplicationButton()));
            }
            d->mApplicationButton->disconnect(SIGNAL(clicked()));
            d->mApplicationButton->disconnect(SIGNAL(pressed()));
            d->mApplicationButton->disconnect(SIGNAL(released()));
            d->mApplicationButton->setText(QString());
            d->mApplicationButton->setIcon(HbIcon());
            d->mApplicationButton->setToolTip(QString());
            d->mApplicationButtonAction = 0;
        } else {
            d->mApplicationButton = new HbTouchKeypadButton(this, QString());
            d->mApplicationButton->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
            d->mApplicationButton->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
        }
        d->mApplicationButtonAction = 0;
    }
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
        d->mSmileyPicker->setGeometry(QRectF(0, pos().y(), geometry().width(),
            geometry().height()));
        d->mSmileyPicker->show();
    }
}

/*!
\deprecated HbInputVkbWidget::flickDirection()
    is deprecated.
*/
HbInputVkbWidget::HbFlickDirection HbInputVkbWidget::flickDirection()
{
    Q_D(HbInputVkbWidget);
    return d->mFlickDirection;
}

/*!
\deprecated HbInputVkbWidget::refreshApplicationButton()
    is deprecated.
*/
void HbInputVkbWidget::refreshApplicationButton()
{
    Q_D(HbInputVkbWidget);

    d->mApplicationButton->setText(d->mApplicationButtonAction->text());
    d->mApplicationButton->setIcon(d->mApplicationButtonAction->icon());

    if (d->mApplicationButton->getButtonType() == HbTouchKeypadButton::HbTouchButtonFnInActive
        && d->mApplicationButtonAction->isEnabled()) {
        // action has been enabled
        connect(d->mApplicationButton, SIGNAL(clicked()), d->mApplicationButtonAction, SLOT(trigger()));
        d->mApplicationButton->setToolTip(d->mApplicationButtonAction->toolTip());
        d->mApplicationButton->setFade(false);
    } else if (d->mApplicationButton->getButtonType() == HbTouchKeypadButton::HbTouchButtonFunction
        && !d->mApplicationButtonAction->isEnabled()) {
        // action has been disabled
        d->mApplicationButton->disconnect(SIGNAL(clicked()));
        d->mApplicationButton->setFade(true);
    }
}

void HbInputVkbWidget::keypadLanguageChangeAnimationUpdate(qreal value)
{
    Q_D(HbInputVkbWidget);

    int direction = 1;
    if (flickDirection() == HbFlickDirectionLeft) {
        direction = -1;
    }

    QRectF rect = boundingRect();
    QPointF position = pos();
    position.setX(direction * (-rect.width() + rect.width() * value));
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
        if (d->mSettingsListOpen){
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

    if(HbSwipeGesture *gesture = qobject_cast<HbSwipeGesture *>(event->gesture(Qt::SwipeGesture))) {
        if (gesture->state() == Qt::GestureFinished) {
            HbWidgetFeedback::triggered(this, Hb::InstantFlicked);
            // vertical swipes
            if (gesture->sceneSwipeAngle() > 250 && gesture->sceneSwipeAngle() < 290 &&
                    gesture->sceneVerticalDirection() == QSwipeGesture::Down) {
                d->mFlickDirection = HbFlickDirectionDown;
                emit keypadCloseEventDetected(HbVkbCloseMethodCloseGesture);
            } else if (gesture->sceneSwipeAngle() > 70 && gesture->sceneSwipeAngle() < 110 &&
                           gesture->sceneVerticalDirection() == QSwipeGesture::Up) {
                d->mFlickDirection = HbFlickDirectionUp;
                d->mCurrentHost->openKeypad(d->mCurrentHost->activeKeypad(), d->mOwner);
            } else {
                d->mFlickDirection = (HbInputVkbWidget::HbFlickDirection)gesture->sceneHorizontalDirection();
                // horizontal swipes
                if (d->mFlickAnimation){
                    animKeyboardChange();
                }
                emit flickEvent(d->mFlickDirection);
            }
        }
    } else if(HbTapGesture *gesture = qobject_cast<HbTapGesture *>(event->gesture(Qt::TapGesture))) {
        if (gesture->state() == Qt::GestureFinished) {        
            // if keypad is minimized, open it 
            if ( d->mCurrentHost->keypadStatus() == HbVkbHost::HbVkbStatusMinimized ) {
                d->mCurrentHost->openKeypad(this, d->mOwner);
            }
        }
    }
}

// End of file
