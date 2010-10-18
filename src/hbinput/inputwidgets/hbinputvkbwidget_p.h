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

#ifndef HB_INPUT_VKB_WIDGET_PRIVATE_H
#define HB_INPUT_VKB_WIDGET_PRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb Inputs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
#include <QSignalMapper>
#include <QPointer>
#include <QTime>
#include <QTimeLine>

#include "hbinputdef.h"
#include "hbwidget_p.h"
#include "hbinputvkbwidget.h"

const QString settingsIcon("qtg_mono_settings");
const QString inputMethodIcon("qtg_mono_virtual_input");
const QString predictionOnIcon("qtg_mono_predictive_text_on");
const QString predictionOffIcon("qtg_mono_predictive_text_off");

class QTimeLine;
class QBitmap;
class QPixmap;
class QGraphicsGridLayout;
class HbPushButton;
class HbFrameDrawer;
class QAction;
class HbStackedWidget;
class HbVkbHost;
class HbTouchKeypadButton;
class HbInputModeIndicator;
class HbKeymap;
class HbInputSmileyPicker;
class HbInputMethod;
class QGraphicsLinearLayout;
class HbInputScreenshotWidget;
class HbInputFocusObject;
class HbInputSettingList;
class HbInputMethodDescriptor;
class HbInputSettingWidget;
class HbInputSettingPopup;

const qreal VerticalSpacing = 0.0, HorizontalSpacing = 0.0; //vertical and horizontal spacing for buttons in layout

class HB_INPUT_PRIVATE_EXPORT HbInputVkbWidgetPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbInputVkbWidget)

public:
    enum HbQwertyKeyboardSize {
        HbQwerty4x10,
        HbQwerty4x11
    };
    HbInputVkbWidgetPrivate();
    virtual ~HbInputVkbWidgetPrivate();

    virtual void initLayout();
    virtual void init();

    virtual int keyCode(int buttonId);
    virtual int keyCode(HbTouchKeypadButton *button);

    virtual void handleStandardButtonPress(int aButtonId);
    virtual void handleStandardButtonRelease(int aButtonId);

    virtual void addCustomButtonToLayout(HbTouchKeypadButton *button,
                                         int index);

    void redirectMousePressEvent(QGraphicsSceneMouseEvent *aEvent);
    void redirectMouseReleaseEvent(QGraphicsSceneMouseEvent *aEvent);

    virtual void applyEditorConstraints();
    virtual void updateKeyCodes();
    virtual void updateButtons();
    virtual void settingListPosition(QPointF &position, HbPopup::Placement &placement);

    void captureScreenshot();
    void updateMouseHitItem(HbTouchKeypadButton *button, QPointF position);
    void normalizeProbabilities(QList<HbKeyPressProbability> &allProbableKeys);

    bool isSmileysEnabled();
    bool isKeyboardDimmed();

    void showInputMethodSelectionDialog();
    void _q_activateInputMethod(const HbInputMethodDescriptor &descriptor, const QByteArray &customData);
    void _q_settingsClosed(HbAction *action);
    void _q_smileyPickerClosed();

    virtual QChar numberCharacterBoundToKey(int key);
    friend class HbTouchKeypadButton;
    friend class HbInputUsedSymbolPane;

public:
    HbInputVkbWidget *q_ptr;
    HbInputMethod *mOwner;
    bool mIsKeypadOn;

    HbKeypadMode mMode;
    const HbKeymap *mKeymap;

    HbModifiers mModifiers;

    HbInputModeIndicator *mInputModeIndicator;

    QPointer<HbTouchKeypadButton> mApplicationButton;
    QPointer<HbAction> mApplicationButtonAction;
    QPointer<HbTouchKeypadButton> mSettingsButton;
    QPointer<HbInputSettingList> mSettingList;

    QGraphicsGridLayout *mButtonLayout;

    QSignalMapper *mPressMapper;
    QSignalMapper *mReleaseMapper;
    QSignalMapper *mActionMapper;

    HbFrameDrawer *mBackgroundDrawer;
    HbFrameDrawer *mIconDrawer;

    bool mMainWinConnected;
    QGraphicsLinearLayout *mLayout;
    QPointer<HbVkbHost> mCurrentHost;
    bool mDrawbackground;
    QTime mMousePressTime;
    bool mMouseButtonPressedDown;
    HbInputVkbWidget::HbFlickDirection mFlickDirection;
    HbInputSmileyPicker *mSmileyPicker;

    HbInputScreenshotWidget *mScreenshotWidget;
    QTimeLine mScreenshotTimeLine;
    HbTouchKeypadButton *mMostRecentlyAccessedButton;
    QPointF mMostRecentlyClickedLocation;
    HbInputFocusObject  *mFocusedObject;
    QObject *mCurrentFocusedObject;
    bool mFlickAnimation;
    int mSettingsDialogsOpen;
    bool mAnimateWhenDialogCloses;
    HbQwertyKeyboardSize mKeyboardSize;
    int mCloseHandleHeight;
    int mCloseHandleWidth;
    QGraphicsWidget *mCloseHandle;
    bool mKeyboardDimmed;
    HbInputSettingPopup *mSettingPopup;
};

#endif //HB_INPUT_VKB_WIDGET_PRIVATE_H

// End of file
