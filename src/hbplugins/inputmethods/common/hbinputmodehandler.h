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
#ifndef hb_input_mode_handler
#define hb_input_mode_handler

#include <QObject>
#include <QStringList>
#include <QChar>

#include <hbinputmodeproperties.h>
#include <hbinputdef.h>
#include <hbinputpredictioncallback.h>

class HbInputAbstractMethod;
class QMouseEvent;
class QTimer;
class HbKeymap;
class QKeyEvent;

class HbInputModeHandlerPrivate;
class HbInputModeHandler: public QObject
{
    Q_OBJECT
public:
    // Set of actions.
    enum HbInputModeAction {
        // mode specific actions
        HbInputModeActionInit, // keymapping initialization, signal connections etc.
        HbInputModeActionReset, // resetting the state of the modehandler
        HbInputModeActionClose, //cleanups like disconnection..!

        // commit actions
        HbInputModeActionCommit, // commits the inline characters if any
        HbInputModeActionDeleteAndCommit,// deletes one character and then commits
        
        HbInputModeActionPrimaryLanguageChanged, 
        HbInputModeActionSecondaryLanguageChanged, 

        // prediction
        HbInputModeActionSetCandidateList, // sets the candidate list
        HbInputModeActionSetKeypad,//sets the keypad
        HbInputModeActionLaunchCandidatePopup, // launched the candidate popup
        HbInputModeActionHideTail, //Hides the prediction tail

        // autocompletion
        HbInputModeActionSetupAutoCompletion, // setting up of autocompletion

        // focus change
        HbInputModeActionFocusRecieved, // focus recived state
        HbInputModeActionFocusLost, // focus lost state
        HbInputModeActionCancelButtonPress
        // more..
    };

    virtual ~HbInputModeHandler();

    // HbInputMethod specific operations.
    virtual bool isComposing() const {return false;}
    virtual void listInputModes(QVector<HbInputModeProperties>& modes) const = 0 ;
    virtual void mouseHandler(int x, QMouseEvent* mouseEvent);
    virtual bool filterEvent(const QEvent * event);

    // Utility functions.
    void commitFirstMappedNumber(int key);
    void getAndFilterCharactersBoundToKey(QStringList &list, Qt::Key key);
    QChar getNthCharacterInKey(int &index, int key);
    virtual void commitAndAppendString(const QString& string);
    virtual void commitAndUpdate(const QString& string, int replaceFrom = 0, int replaceLength = 0, bool isAsync = false);
    void sendAndUpdate(QEvent &event);
    virtual void setKeymap(const HbKeymap* keymap);
    virtual void characterPreviewAvailable(bool available);

signals:
    // incase one mode handler is not capable of processing the events.
    // it can pass the event by sending following signals. And then it is 
    // upto the plugin to decide which two mode handlers needs to be connected.
    void passFilterEvent(const QKeyEvent *event);
    void passActionHandler(HbInputModeAction action);

public slots:    
    virtual void sctCharacterSelected(QString character);
    // filterEvent function
    virtual bool filterEvent(const QKeyEvent * event);
    // Action handler for HbInputModeAction actions.
    virtual bool actionHandler(HbInputModeAction action);
    virtual void smileySelected(QString smiley);
    virtual void cursorPositionChanged(int oldPos, int newPos);
protected:
    HbInputModeHandlerPrivate* const d_ptr;
    HbInputModeHandler(HbInputModeHandlerPrivate &dd, HbInputAbstractMethod* inputMethod);
private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputModeHandler)
    Q_DISABLE_COPY(HbInputModeHandler)
    Q_PRIVATE_SLOT(d_func(), virtual void _q_timeout())
};

// long press constant.
const int HbLongPressTimerTimeout = 600;
// multi tap constant
const int HbMultiTapTimerTimeout = 600;

// shift key states
enum HbInputShiftKeyState {
    EKeyPressedNone,
    EShiftKeyPressed
};

const QString HbAutoCompleteVendorIdString("Nokia HbAutoComplete");

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// LATIN BASIC //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class HbInputBasicHandlerPrivate;
class HbInputBasicHandler: public HbInputModeHandler
{
    Q_OBJECT
public:
    HbInputBasicHandler(HbInputAbstractMethod* inputMethod);
    ~HbInputBasicHandler();

    bool actionHandler(HbInputModeAction action);
    bool filterEvent(const QKeyEvent * event);

    // automatic completion apis;
    void setUpAutoCompleter();
    void refreshAutoCompleter();
    void deleteCharacterInAutoCompleter();
    void appendUnicodeCharacterInAutoCompleter(QChar character);
    void addWordInAutoCompleter();

public slots:
    void autoCompletionPopupClosed(QString activatedWord, int closingKey);
    void sctCharacterSelected(QString character);
    void smileySelected(QString character);
protected:
    HbInputBasicHandler(HbInputBasicHandlerPrivate &dd, HbInputAbstractMethod* inputMethod);
private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputBasicHandler)
    Q_DISABLE_COPY(HbInputBasicHandler)
};


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// PREDICTION //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
class HbInputPredictionHandlerPrivate;
class HbInputPredictionHandler: public HbInputModeHandler
{
    Q_OBJECT
public:
    ~HbInputPredictionHandler();

    bool isComposing();
    void mouseHandler(int cursorPosition, QMouseEvent* mouseEvent);
    bool filterEvent(const QKeyEvent * event);

    // engine related information.
    QList<HbInputLanguage> supportedLanguages() const;
    bool actionHandler(HbInputModeAction action);

    // some utility functions
    void appendUnicodeCharacter(QChar character);
    void commitAndAppendCharacter(QChar character);
    virtual void deleteOneCharacter();
    virtual void processExactWord(QString exactWord);
    void commitExactWord();
    virtual void processCustomWord(QString customWord);
    virtual void candidatePopupClosed(QString activatedWord, int closingKey);
    virtual void showExactWordPopupIfNeeded();
public slots:
    // different utility popup callbacks
    virtual void inputQueryPopupClosed(QString activatedWord, int closingKey);    
    void sctCharacterSelected(QString character);
    void smileySelected(QString smiley);
signals://some useful signals related to popups
    void launchInputQueryPopup(QString editWord);

protected:
    HbInputPredictionHandler(HbInputPredictionHandlerPrivate &dd, HbInputAbstractMethod* inputMethod);
private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputPredictionHandler)
    Q_DISABLE_COPY(HbInputPredictionHandler)
};


/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Numeric //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class HbInputNumericHandlerPrivate;
class HbInputNumericHandler: public HbInputModeHandler
{
    Q_OBJECT
public:
    ~HbInputNumericHandler();

    bool actionHandler(HbInputModeAction action);
    bool filterEvent(const QKeyEvent * event);

protected:
    HbInputNumericHandler(HbInputNumericHandlerPrivate &dd, HbInputAbstractMethod* inputMethod);
private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputNumericHandler)
    Q_DISABLE_COPY(HbInputNumericHandler)
};

#endif //hb_input_mode_handler
