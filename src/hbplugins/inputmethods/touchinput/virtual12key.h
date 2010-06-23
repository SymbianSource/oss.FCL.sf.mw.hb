/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbPlugins module of the UI Extensions for Mobile.
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

#ifndef _Virtual12Key_H
#define _Virtual12Key_H

#include <QPointer>
#include <hbinputabstractbase.h>
#include <hbinputvirtualrocker.h>
#include <hbinputvkbwidget.h>

class HbKeymap;
class QTimer;
class HbInputVkbWidget;
class Hb12KeyTouchKeyboard;
class HbSctKeyboard;
class HbCandidateList;
class HbPredictionEngine;
class HbAction;

    // input mode handlers.
class HbInputModeHandler;
class HbInputPrediction12KeyHandler;
class HbInputBasic12KeyHandler;
class HbInputNumeric12KeyHandler;

class HbVirtual12Key : public HbInputAbstractMethod
{
    Q_OBJECT

    enum {
        EFinger12Abc,
        EFinger12Sct,
        EFinger12Numeric
    };

public:
    HbVirtual12Key();
    ~HbVirtual12Key();

public: // From QInputContext
    QString identifierName();
    bool isComposing() const;
    QString language();
    void reset();
    void mouseHandler(int cursorPosition, QMouseEvent* event);

public: // From HbInputMethod
    void focusLost(bool focusSwitch);
    void focusReceived();
    void inputLanguageChanged(const HbInputLanguage &newLanguage);
    void inputStateActivated(const HbInputState& newState);
    int displaySpecialCharacterTable(QObject* receiver);
    bool filterEvent(const QEvent* event);
    void secondaryInputLanguageChanged(const HbInputLanguage &newLanguage);

public:
    void initializePredictiveMode();
    void closeCandidatePopup();
    void showAutoCompletionFieldCandidates();
    void initializeModeHandlers();
    bool isSctModeActive() const;
    void launchCandidatePopup(const QStringList& candidates);
    void closeKeypad();
    void selectSpecialCharacterTableMode();
    void launchAutoCompletionPopup(const QStringList &candidates);
    void closeAutoCompletionPopup();
    void switchMode(int keyCode);
    void starKeySelected();
    HbKeyboardType currentKeyboardType() const;
private:
    void openKeypad(HbInputVkbWidget * keypadToOpen,bool inMinimizedMode = false);
    void switchToAlphaMode(bool openAlphaInMinimizedMode = false);
    void setSpecialCharacters();
    void loadKeymap(const HbInputLanguage &newLanguage);
    Hb12KeyTouchKeyboard *construct12Keyboard();
    bool usePrediction() const;

public slots:
    void keypadClosed();
    void keypadOpened();
    void keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod vkbCloseMethod);
    void rockerDirection(int direction, HbInputVirtualRocker::RockerSelectionMode selectionMode);
    void orientationAboutToChange();
    void candidatePopupClosed(int closingKey, const QString &candidate);
    void flickEvent(HbInputVkbWidget::HbFlickDirection direction);
    void mouseMovedOutOfButton();
    void smileySelected(QString smiley);
    void predictiveInputStateChanged(HbKeyboardSettingFlags keyboardType, bool newState);
    void aboutToActivateCustomAction(HbAction *custAction);
    void autocompletionStateChanged(HbKeyboardSettingFlags keyboardType, bool newState);

private:
    // mCurrentKeypad contains currently active keypad, we don't need to have
    // anyother variables to tell us which is current keypad
    QPointer<HbInputVkbWidget> mCurrentKeypad;
    // contains itut keypad
    QPointer<Hb12KeyTouchKeyboard> mItutKeypad;
    // contains sct keypad
    QPointer<HbSctKeyboard> mSctKeypad;

    //Owned by the keymap factory
    const HbKeymap* mKeymap;
    bool mOrientationAboutToChange;

    // Owned
    HbCandidateList* mCandidatePopup;
    HbInputFocusObject* mCurrentlyFocused;

    // input mode handlers.
    HbInputModeHandler *mActiveModeHandler;
    HbInputPrediction12KeyHandler *mPredictionModeHandler;
    HbInputBasic12KeyHandler *mBasicModeHandler;
    HbInputNumeric12KeyHandler *mNumericModeHandler;
	
    QPointer<HbVkbHost> mVkbHost;
    bool mKeyboardChangeAlreadyInprogress;
};

#endif // _Virtual12Key_H

// End of file
