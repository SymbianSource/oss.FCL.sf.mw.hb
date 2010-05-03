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

#ifndef _VirtualQwerty_H
#define _VirtualQwerty_H

#include <QPointer>
#include <hbinputabstractbase.h>
#include <hbinputpredictioncallback.h>
#include <hbinputvirtualrocker.h>
#include <hbinputvkbwidget.h>

class QTimer;
class HbQwertyKeyboard;
class HbKeyMapData;
class HbPredictionEngine;
class HbCandidateList;
class HbExactWordPopup;
class HbInputSctLandscape;

class HbInputModeHandler;
class HbInputBasicQwertyHandler;
class HbInputPredictionQwertyHandler;
class HbInputNumericQwertyHandler;

class HbVirtualQwerty : public HbInputAbstractMethod
{
    Q_OBJECT

public:
    HbVirtualQwerty();
    ~HbVirtualQwerty();

public: // From QInputContext
    QString identifierName();
    bool isComposing() const;
    QString language();
    void reset();
    void mouseHandler(int x, QMouseEvent* aEvent);
    bool filterEvent(const QEvent* event);

public slots:
    void sctCharacterSelected(QString aChar);
    void orientationAboutToChange();
    void rockerDirection(int aDirection, HbInputVirtualRocker::RockerSelectionMode aSelectionMode);
    // keypad related slots
    void keypadClosed();
    void keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod vkbCloseMethod);
    void flickEvent(HbInputVkbWidget::HbFlickDirection direction);
    void mouseMovedOutOfButton();
    void smileySelected(QString smiley);
    void predictiveInputStateChanged(HbKeyboardSettingFlags keyboardType, bool newState);

public: // From HbInputMethod
    void focusReceived();
    void focusLost(bool focusSwitch);
    void inputStateActivated(const HbInputState& aNewState);
    void candidatePopupClosed(int closingKey);
    void inputLanguageChanged(const HbInputLanguage &aNewLanguage);
    void secondaryInputLanguageChanged(const HbInputLanguage &newLanguage);
    int displaySpecialCharacterTable(QObject* aReceiver);
    bool usePrediction() const;
public: //From HbAbstractQwerty
    QList<HbKeyPressProbability> probableKeypresses();
signals:
    void autoCompletionPopupClosed(QString word, int closingkey);

public:
    void initializeModeHandlers();
    void launchCandidatePopup(const QStringList &candidates);

    void closeKeypad();
    void launchCharacterPreviewPane(int key);
    void switchSpecialCharacterTable();
    void selectSpecialCharacterTableMode();
    void closeExactWordPopup();
    void launchExactWordPopup(QString exactWord);
    void closeAutoCompletionPopup();
    void launchAutoCompletionPopup(const QStringList &candidates);

protected:
    void predictiveInputStatusChanged(int newStatus);

private slots:
    void candidatePopupCancelled();

private:
    void initializePredictiveMode();
    void updateEditor();
    bool selectWord(bool selectFromLeft = true);
    void launchCandidatePopup();
    QPointF getCursorCoordinatePosition();
    void loadKeymap(const HbInputLanguage &newLanguage);

private:
    void openKeypad(HbInputVkbWidget * keypadToOpen, bool inMinimizedMode = false);
    HbQwertyKeyboard* constructKeypad(HbKeypadMode currentInputType);

private:
    // mCurrentKeypad contains currently active keypad, we dont need to have
    // anyother variables to tell us which is current keypad
    QPointer<HbInputVkbWidget> mCurrentKeypad;
    // contains qwerty alpha keypad
    QPointer<HbQwertyKeyboard> mQwertyAlphaKeypad;
    // contains qwerty numeric keypad
    QPointer<HbQwertyKeyboard> mQwertyNumericKeypad;
    // contains sct keypad
    QPointer<HbInputSctLandscape> mSctKeypad;

    const HbKeymap *mKeymap;
    HbFnState mFnState;
    HbInputState mPreviousInputState;

    HbExactWordPopup *mExactWordPopup;
    HbCandidateList *mCandidatePopup;
    bool mOrientationAboutToChange;
    HbInputVkbWidget::HbSctView mSctMode;
    int mShiftKeyState;
    // mode handlers
    HbInputModeHandler *mActiveModeHandler;
    HbInputBasicQwertyHandler *mBasicModeHandler;
    HbInputPredictionQwertyHandler *mPredictionModeHandler;
    HbInputNumericQwertyHandler *mNumericModeHandler;
    // vkbhost
    QPointer<HbVkbHost> mVkbHost;
};

#endif // _VirtualQwerty_H

// End of file
