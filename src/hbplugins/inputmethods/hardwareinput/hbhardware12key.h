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
#ifndef _HbHardware12key_H 
#define _HbHardware12key_H 

#include <hbinputmethod.h>
#include <hbinputpredictioncallback.h>


class HbKeyMapFactory;
class HbKeyMapData;
class QTimer;
class HbInputSctPortrait;
class HbCandidateList;
class HbPredictionEngine;
class HbAction;


// input mode handlers.
class HbInputModeHandler;
class HbHardwareInputPrediction12KeyHandler;
class HbHardwareInputBasic12KeyHandler;
class HbHardwareInputNumeric12KeyHandler;


class HbHardware12key : public HbInputMethod
{
    Q_OBJECT
public:
    HbHardware12key();
    ~HbHardware12key();

public: // From QInputContext
    QString identifierName();
    bool isComposing() const;
    QString language();
    void reset();
    void mouseHandler(int cursorPosition, QMouseEvent* event);
    bool usePrediction() const;
public: // From HbInputMethod    
    void focusReceived();
        void focusLost(bool focusSwitch);
    void inputLanguageChanged(int newLanguage); 
    void inputStateActivated(const HbInputState& newState);
    bool filterEvent(const QEvent *event);

public: 
    void initializeModeHandlers();
    void closeCandidatePopup();
    void launchCandidatePopup(QStringList *candidates);
    int switchSpecialCharacterTable();
    void launchAutoCompletionPopup(QStringList *candidates);
    void closeAutoCompletionPopup();
    void switchMode(int keyCode);
    
protected:
    void predictiveInputStatusChanged(int newStatus);

public slots:
    void sctCharacterSelected(QChar character);
    void candidatePopupClosed(int closingKey);

private:
    void switchToSctMode();
    void setSpecialCharacters();
    QPoint getCandidatePosition();

private:
    HbKeyMapData*    mKeyData;
    HbInputModeType mInputMode;
    HbInputState mPrevInputState;
    HbInputSctPortrait* mSctKeypad;

    // Owned
    HbCandidateList* mCandidatePopup;
    HbInputFocusObject* mCurrentlyFocused;

    // Input mode handlers.
    HbInputModeHandler *mActiveModeHandler;
    HbHardwareInputPrediction12KeyHandler *mPredictionModeHandler;
    HbHardwareInputBasic12KeyHandler *mBasicModeHandler;
    HbHardwareInputNumeric12KeyHandler *mNumericModeHandler;
};

#endif // _HbHardware12key_H

// End of file 

