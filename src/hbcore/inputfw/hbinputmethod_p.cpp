/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
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
#include <QInputMethodEvent>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QLocale>
#include <QClipboard>

#include "hbinputmethod.h"
#include "hbinputmethod_p.h"
#include "hbinputmodecache_p.h"
#include "hbinputsettingproxy.h"
#include "hbinputcontextproxy_p.h" 
#include "hbinputfilter.h"
#include "hbinputmethodnull_p.h"
#include "hbinpututils.h"
#include "hbinputstandardfilters.h"

/// @cond

/*!
\internal
Destructs the object.
*/
HbInputMethodPrivate::~HbInputMethodPrivate()
{
    delete mFocusObject;
}

/*!
\internal
Returns input filter of the focused editor.
*/
HbInputFilter *HbInputMethodPrivate::editorFilter() const
{
    if (mFocusObject) {
        return mFocusObject->editorInterface().filter();
    }

    return 0;
}

/*!
\internal
Returns constraint flags of the focused editor.
*/
int HbInputMethodPrivate::editorConstraints() const
{
    if (mFocusObject) {
        return mFocusObject->editorInterface().inputConstraints();
    }

    return 0;
}

/*!
\internal
Reads input state information from focused editor using editor interface and creates a local copy of it.
Finds out correct values for those fields that have not been initialized by the client application.
*/
void HbInputMethodPrivate::inputStateFromEditor(HbInputState& result)
{   
    if (mFocusObject) {
        HbEditorInterface& editorInterface = mFocusObject->editorInterface();
        editorInterface.lastFocusedState(result);

        if (result != HbInputState()) {    
            // The editor has been focused before. Let's see if the input language has changed since the editor
            // was last focused. 
            HbInputLanguage language = findStateLanguage();
            if (language != result.language()) {
                // Reconstruct the state since we don't know if the last one is valid for the new language. 
                editorRootState(result);  
                return;
            }

            // Keyboard may have changed since the last focus. Re-initialize it.
           result.setKeyboard(activeKeyboard());
           return;
        } 

        // this editor has not been focused before, return the root state.
        editorRootState(result);

        // See if the editor prefers numeric mode when focused for the first time.
        Qt::InputMethodHints hints = mFocusObject->inputMethodHints();
        if (hints & Qt::ImhPreferNumbers) {
            result.setInputMode(HbInputModeNumeric);
        }
    } else {
        result = HbInputState();
    }
}

/*!
\internal
Transfers local copy of the input state back to the editor using editor interface.
*/
void HbInputMethodPrivate::inputStateToEditor(const HbInputState& source)
{
    if (mFocusObject) {
        mFocusObject->editorInterface().setLastFocusedState(source);
        mFocusObject->editorInterface().setMode(source.inputMode());
        mFocusObject->editorInterface().setTextCase(source.textCase());
    }
}

/*!
\internal
Returns input language for current input state. Ususally it is same as the global input language but
in case of latin only -editors it may be english if the global input language isn't valid for latin only -editors.
*/
HbInputLanguage HbInputMethodPrivate::activeLanguage() const
{
    HbInputLanguage ret = mInputState.language();

    if (ret.undefined()) {
        ret = HbInputSettingProxy::instance()->globalInputLanguage();
        if (mFocusObject) {
            if ((mFocusObject->editorInterface().inputConstraints() & HbEditorConstraintLatinAlphabetOnly) &&
                !ret.isLatinAlphabetLanguage()) {
                // This is latin alphabet flagged editor, but the language isn't
                // latin alphabet language. Switch to english locally.
               ret = QLocale::English;
           }
        }
    }

    return HbInputLanguage(ret);
}

/*!
\internal
Returns true if given input mode is valid in focused editor.
*/
bool HbInputMethodPrivate::modeAllowedInEditor(HbInputModeType mode) const
{
    const int constraints = editorConstraints();

    if (mFocusObject &&
        mFocusObject->editorInterface().isNumericEditor() &&
        (mode & HbNumericModeMask) == 0) {
        // This is number only -editor but the proposed mode is not number mode, reject.
        return false;
    }

    if (constraints & HbEditorConstraintFixedInputMode){
        if (!mFocusObject || mFocusObject->editorInterface().mode() != mode) {
            // This is fixed mode editor but proposed mode is something else.
            // Reject.
            return false;
        }
    }

    if ((constraints & HbEditorConstraintLatinAlphabetOnly) && (mode & HbChineseModeMask)) {
        // Editor is flagged to be latin-alphabet only but
        // the input mode indicates non-latin mode. Reject.
        return false; 
    }
    return true;
}

/*!
\internal
Returns true if given state is valid in focused editor.
*/
bool HbInputMethodPrivate::stateAllowedInEditor(const HbInputState& state)
{
    if (!modeAllowedInEditor(state.inputMode())) {
        return false;
    }

    if ((state.inputMode() & HbChineseModeMask) == 0) {
        HbEditorConstraints constraints = (HbEditorConstraints)editorConstraints();
        if ((state.textCase() == HbTextCaseUpper || state.textCase() == HbTextCaseAutomatic)
            && isLowerCaseOnlyEditor()) {
            return false;
        }

        if (state.textCase() == HbTextCaseLower
            && isUpperCaseOnlyEditor()
            && !(state.inputMode() & HbNumericModeMask)) {
            return false;
        }
    }

    return true;
}

/*!
\internal
Finds state handler for given input state.
*/
HbInputMethod* HbInputMethodPrivate::findStateHandler(HbInputState& state)
{  
    HbInputMethod *stateHandler = 0;

    if (mFocusObject &&
        (mFocusObject->editorInterface().inputConstraints() & HbEditorConstraintIgnoreFocus)) {
        return HbInputMethodNull::Instance();
    }

    if (stateAllowedInEditor(state)) {
        stateHandler = HbInputModeCache::instance()->findStateHandler(state);
        if (!stateHandler &&
             state.inputMode() == HbInputModeNumeric &&
             state.language() != QLocale::English &&
             mFocusObject &&
             (mFocusObject->editorInterface().inputConstraints() & HbEditorConstraintFixedInputMode)) {
             // This is number only editor but there was no numeric handler
             // for specified language. Use default numeric hanlder
             // as a fallback.
             state.setLanguage(QLocale::English);
             stateHandler = HbInputModeCache::instance()->findStateHandler(state);
        }
    }

    return stateHandler;
}

/*!
\internal
Returns input language for current input state. Ususally it is same as the global input language but
in case of latin only -editors it may be english if the global input language isn't valid for latin only -editors.
*/
HbInputLanguage HbInputMethodPrivate::findStateLanguage() const
{
    HbInputLanguage lang = HbInputSettingProxy::instance()->globalInputLanguage();
    if ((editorConstraints() & HbEditorConstraintLatinAlphabetOnly) &&
        !lang.isLatinAlphabetLanguage()) {
        // This is latin alphabet flagged editor, but the language isn't
        // latin alphabet language. Switch to english locally.
        lang = QLocale::English;
    }   
    
    return lang;
}

/*!
\internal
Returns true if auto-capitalisation is needed in current cursor position.
*/
bool HbInputMethodPrivate::automaticTextCaseNeeded() const
{
    // If active keyboard is qwerty based and settings do not allow
    // automatic text case to be used with it, return false.
    if (mInputState.keyboard() & HbQwertyKeyboardMask
        && !HbInputSettingProxy::instance()->automaticTextCasingForQwerty()) {
        return false;
    }

    if (mFocusObject) {
        if (mFocusObject->inputMethodHints() & (Qt::ImhNoAutoUppercase | Qt::ImhPreferLowercase | Qt::ImhPreferUppercase)) {
            // Input method hint forbids auto-capitalisation or prefers either case.
            return false;
        }

        if (isFixedCaseEditor()) {
            // This is "uper case only" or "lower case only" editor. No automatic casing needed.
            return false;
        }

        if (mFocusObject->preEditString().size() > 0) {
            // If these is active word in inline edit state,
            // then for sure auto-casing is not needed.
            return false;
        }

        QString sText = mFocusObject->editorSurroundingText();
        int cursorPosition = mFocusObject->editorCursorPosition();
        if(cursorPosition >= 2) {
            QString previousChar = sText.mid(cursorPosition-1, 1);
            QString previousToPreviousChar = sText.mid(cursorPosition-2, 1);

            if (QString::compare( previousChar, " " ) == 0) {
                if (QString::compare( previousToPreviousChar, "." ) == 0) {
                    return true;
                }
                if (QString::compare( previousToPreviousChar, "!" ) == 0) {
                    return true;
                }
                if (QString::compare( previousToPreviousChar, "?" ) == 0) {
                    return true;
                }
            }
        } else if(cursorPosition == 0) {
            // when the cursor is at the beginning of the editor, auto-capitalisation is needed
            return true;
        }
    }

    return false;
}

/*!
\internal
Returns true if the concept of text case applies to current input language and input state.
*/
bool HbInputMethodPrivate::textCaseApplies() const
{
    HbInputLanguage language = activeLanguage();

    if (!language.isCaseSensitiveLanguage()) {
        return false;
    }

    if (!HbInputUtils::isCaseSensitiveMode(mInputState.inputMode())) {
        // Text case doesn't apply.
        return false;
    }

    return true;
}

/*!
\internal
Returns the active keyboard.
*/
HbKeyboardType HbInputMethodPrivate::activeKeyboard() const
{
    // We assume here that if touch keyboard value is available
    // in the setting proxy, then this is touch input device.
    // Otherwise return hw keyboard value. Later we need to add support
    // for hybrid devices where both hw and touch keypads can be active
    // at the same time.

    return HbInputSettingProxy::instance()->activeKeyboard();
}

/*!
\internal
Takes care of the part of focus in operation that is common to both QWidgets and QGraphicsWidgets.
*/
void HbInputMethodPrivate::setFocusCommon()
{
    Q_Q(HbInputMethod);

    if (mFocusObject) {
        Qt::InputMethodHints hints = mFocusObject->inputMethodHints();
        if (hints & Qt::ImhDialableCharactersOnly) {
            setUpFocusedObjectAsPhoneNumberEditor();
        } else if (hints & Qt::ImhFormattedNumbersOnly) {
            setUpFocusedObjectAsFormattedNumberEditor();
        } else if (hints & Qt::ImhDigitsOnly) {
            setUpFocusedObjectAsDigitsOnlyEditor();
        } else if (hints & Qt::ImhUrlCharactersOnly) {
            setUpFocusedObjectAsUrlEditor();
        } else if (hints & Qt::ImhEmailCharactersOnly) {
            setUpFocusedObjectAsEmailEditor();
        } 

        if (mFocusObject->editorInterface().editorClass() != HbInputEditorClassUnknown &&
            mFocusObject->editorInterface().extraDictionaryId() == 0) {
            // Editor class is set, but no dictionary id. Set it automatically here.
            mFocusObject->editorInterface().setExtraDictionaryId(mFocusObject->editorInterface().editorClass());
        }
    }

    // Create input state.  
    inputStateFromEditor(mInputState);

    // Find state handler
    HbInputMethod* stateHandler = 0;
    HbInputMethodDescriptor activeMethod = HbInputSettingProxy::instance()->activeCustomInputMethod();
    if (!activeMethod.isEmpty() && !activeMethod.isDefault()) {
        // A custom method is active. Don't resolve, just try to load it.
        if ((editorConstraints() & HbEditorConstraintIgnoreFocus) == 0) {
            stateHandler = HbInputModeCache::instance()->loadInputMethod(activeMethod);
        }
    }

    if (!stateHandler) {
        // It either wasn't a custom method or we were not able to load the custom method.
        // Resolve normally.
         stateHandler = findStateHandler(mInputState);
    }

    if (stateHandler == 0) {
        // No state handler found (this should never happen under normal circumstances).
        // Fall back to null method.
        stateHandler = HbInputMethodNull::Instance();
    }

    if (stateHandler != q_ptr) {
        // This method cannot handle requested input state. Switch to another one.
        contextSwitch(stateHandler);
        return;
    }

    // Deep copy possibly modified input state back to editor at this point.
    inputStateToEditor(mInputState);

    q->focusReceived();
    refreshState();
}

/*!
\internal
Refreshes input state. Stores local edit state back to the editor and calls child class' inputMethodActived method.
*/
void HbInputMethodPrivate::refreshState()
{
    Q_Q(HbInputMethod);

    inputStateToEditor(mInputState);
    q->inputStateActivated(mInputState);
}

/*!
\internal
Returns true if given focus object is same as currently focused or points to same editor instance.
*/
bool HbInputMethodPrivate::compareWithCurrentFocusObject(HbInputFocusObject* focusObject) const
{
    // both null pointer
    if ( !mFocusObject ) {
        return false;
    }

    if ( !focusObject ) {
        return false;
    }

    if (mFocusObject == focusObject) {
        return true;
    }

    return mFocusObject->object() == focusObject->object();
}

/*!
\internal
Creates and returns new input context proxy.
*/
QInputContext* HbInputMethodPrivate::newProxy()
{
    return new HbInputContextProxy(q_ptr);
}

/*!
\internal
Returns true if currently focused editor is fixed text case editor. 
*/
bool HbInputMethodPrivate::isFixedCaseEditor() const
{
  if (mFocusObject) {
        return (mFocusObject->inputMethodHints() & (Qt::ImhLowercaseOnly | Qt::ImhUppercaseOnly));
    }

    return false;
}

/*!
\internal
Returns true if focused editor is lower case -only editor.
*/
bool HbInputMethodPrivate::isLowerCaseOnlyEditor() const
{
    if (mFocusObject) {
       return (mFocusObject->inputMethodHints() & Qt::ImhLowercaseOnly);    
    }

    return false;
}

/*!
\internal
Returns true if focused editor is upper case -only editor.
*/
bool HbInputMethodPrivate::isUpperCaseOnlyEditor() const
{
    if (mFocusObject) {
       return (mFocusObject->inputMethodHints() & Qt::ImhUppercaseOnly);
    }

    return false;
}

/*!
\internal
This method is needed during context switch operation. It transfers relevant parts of input
method's internal state to the input method that is about to assume control.
*/
void HbInputMethodPrivate::transfer(HbInputMethod* source)
{
    Q_Q(HbInputMethod);

    if (source) {
        // Delete old focus object.
        delete mFocusObject;
        mFocusObject = 0;

        // Switch to new focus object.
        mFocusObject = source->d_ptr->mFocusObject;
        source->d_ptr->mFocusObject = 0;
        if (mFocusObject) {
            q->disconnect(mFocusObject->object(), SIGNAL(destroyed(QObject*)), source, SLOT(editorDeleted(QObject*)));
            q->connect(mFocusObject->object(), SIGNAL(destroyed(QObject*)), q, SLOT(editorDeleted(QObject*)));
        }

        // Makes sure there isn't focus lock.
        mFocusLocked = false;

        // Transfer state.
        mInputState = source->d_ptr->mInputState;

        // Set this one active.
        mIsActive = true;
        source->d_ptr->mIsActive = false;
    }
}

/*!
\internal
Passes control to another input method instance.  Context switch
happens when there is a need to change input state but currently active input method
is not able to handle new input state. Framework then finds a new state handler and
calls this method to switch active input method. After successful call, the given input method will
be application's active input context.
*/
void HbInputMethodPrivate::contextSwitch(HbInputMethod* toBeActive)
{
    Q_Q(HbInputMethod);

    if (q == toBeActive) {
        return;
    }

    // Deactivate before focus event. That way input method is able to
    // recognize in focus handler that the focus lost event was a result of
    // context switch and close all its open UI-elements. This needs to be done
    // even there is no active focus object (to make sure that virtual keyboard & 
    // other potential UI elements will be removed).
    mIsActive = false;
    q->focusLost(false);

    // Release possible focus-lock.
    mFocusLocked = false;

    // Then transfer state to to-be-focused method.
    toBeActive->d_ptr->transfer(q);

    // Active new context.
    QInputContext* proxy = toBeActive->d_ptr->newProxy();
    qApp->setInputContext(proxy);

    if (toBeActive->focusObject()) {
        // Notify focus change.
        toBeActive->focusReceived();

        // Notify input state change.
        toBeActive->d_ptr->refreshState();
    }
}

/*!
\internal
Constructs the first input state for an editor that hasn't been focused before.
*/
void HbInputMethodPrivate::editorRootState(HbInputState &result) const
{ 

   if (mFocusObject) {
        HbInputLanguage language = findStateLanguage();
        HbInputModeType inputMode = initialInputMode(language);     
        result = HbInputState(inputMode,
                              initialTextCase(inputMode),
                              activeKeyboard(),
                              language);     
    } else {
        result = HbInputState();
    }
}

/*!
\internal
Reuturns initial text case for an editor that hasn't been focused before.
*/
HbTextCase HbInputMethodPrivate::initialTextCase(HbInputModeType inputMode) const
{
    HbTextCase ret = HbTextCaseNone;

    if (mFocusObject) {
        Qt::InputMethodHints hints = mFocusObject->inputMethodHints();

        // Then handle the text case: find out what it should be and and if
        // automatic text case is needed.
        ret = (HbTextCase)mFocusObject->editorInterface().textCase();
        if (!HbInputUtils::isCaseSensitiveMode(inputMode)) {
            // Text case doesn't apply.
            ret = HbTextCaseNone;
        } else {
            if (inputMode & HbNumericModeMask) {
                // Set text case to lower case for numeric mode (it doesn not really apply but just in case).
                ret = HbTextCaseLower;
            } else if (ret == HbTextCaseNone) {
                // There was no initial text case. Figure it out.
                ret = HbTextCaseLower;
                // Set automatic text case if suitable conditions are met and there are no constraints
                // preventing it.
                if (!isFixedCaseEditor() && automaticTextCaseNeeded()) {
                    ret = HbTextCaseAutomatic;
                }

                // Then see if there are exceptions that override previous decisions.
                if (hints & Qt::ImhPreferUppercase) {
                    ret = HbTextCaseUpper;
                }

                if (isUpperCaseOnlyEditor()) {
                    ret = HbTextCaseUpper;
                }
            }
        }
    }

    return ret;
} 

/*!
\internal
Finds the first input mode for an editor that hasn't been focus before.
*/
HbInputModeType HbInputMethodPrivate::initialInputMode(const HbInputLanguage &language) const
{ 
    HbInputModeType ret = HbInputModeNone;

    if (mFocusObject) {        
        if (mFocusObject->editorInterface().inputConstraints() & HbEditorConstraintFixedInputMode) {
            // This is fixed mode editor, always trust what editor interface gives us.
            ret = (HbInputModeType)mFocusObject->editorInterface().mode();
        } else {
            // Editor doesn't have mode asigned. Propose default mode.                  
            Qt::InputMethodHints hints = mFocusObject->inputMethodHints();
            if (mFocusObject->editorInterface().isNumericEditor()) {
                // It is fixed numeric editor.
                ret = HbInputModeNumeric;
            } else {
                ret = defaultInputMode(language);        
            }
        }
    }

    return ret;
}

/*!
\internal
Constructs latin input state. Non-latin input methods that wish to switch to latin mode
can use this input state as a parameter for activateState() method.
*/
void HbInputMethodPrivate::constructLatinState(HbInputState &result) const
{
     result.setLanguage(HbInputLanguage(QLocale::English));
     result.setInputMode(HbInputModeDefault);
     result.setKeyboard(activeKeyboard()); 
     result.setTextCase(initialTextCase(HbInputModeDefault));     
}

/*!
\internal
Returns the default input mode for given language.
*/
HbInputModeType HbInputMethodPrivate::defaultInputMode(const HbInputLanguage &inputLanguage) const
{
    if (inputLanguage == HbInputLanguage(QLocale::Chinese, QLocale::China)) {
        return HbInputModePinyin; 
        }
    if (inputLanguage == HbInputLanguage(QLocale::Chinese, QLocale::HongKong)) {
        return HbInputModeStroke; 
        }
    if (inputLanguage == HbInputLanguage(QLocale::Chinese, QLocale::Taiwan)) {
        return HbInputModeZhuyin; 
        }

    return HbInputModeDefault;
}

/*!
\internal
A convenience method for setting up the editor as digits only editor.
*/
void HbInputMethodPrivate::setUpFocusedObjectAsDigitsOnlyEditor()
{
    if(mFocusObject) {
        mFocusObject->editorInterface().setMode(HbInputModeNumeric);
        mFocusObject->editorInterface().setInputConstraints(HbEditorConstraintFixedInputMode);
        if(!mFocusObject->editorInterface().filter()) {
            mFocusObject->editorInterface().setFilter(HbDigitsOnlyFilter::instance());
        }
        mFocusObject->setInputMethodHints(Qt::ImhDigitsOnly | Qt::ImhNoPredictiveText);
    }
}

/*!
\internal
A convenience method for setting up the editor as formatted only editor
*/
void HbInputMethodPrivate::setUpFocusedObjectAsFormattedNumberEditor()
{
    if(mFocusObject) {
        mFocusObject->editorInterface().setMode(HbInputModeNumeric);
        mFocusObject->editorInterface().setInputConstraints(HbEditorConstraintFixedInputMode);
        if(!mFocusObject->editorInterface().filter()) {
            mFocusObject->editorInterface().setFilter(HbFormattedNumbersFilter::instance());
        }
        mFocusObject->setInputMethodHints(Qt::ImhFormattedNumbersOnly | Qt::ImhNoPredictiveText);
    }
}

/*!
\internal
A convenience method for setting up the editor as phone number editor
*/
void HbInputMethodPrivate::setUpFocusedObjectAsPhoneNumberEditor()
{
    if(mFocusObject) {
        mFocusObject->editorInterface().setMode(HbInputModeNumeric);
        mFocusObject->editorInterface().setInputConstraints(HbEditorConstraintFixedInputMode);
        if(!mFocusObject->editorInterface().filter()) {
            mFocusObject->editorInterface().setFilter(HbPhoneNumberFilter::instance());
        }
        mFocusObject->setInputMethodHints(Qt::ImhDialableCharactersOnly | Qt::ImhNoPredictiveText);
    }
}

/*!
\internal
A convenience method for setting up the editor as email editor
*/
void HbInputMethodPrivate::setUpFocusedObjectAsEmailEditor()
{
    if(mFocusObject) {
        mFocusObject->editorInterface().setMode(HbInputModeNone);
        mFocusObject->editorInterface().setInputConstraints(HbEditorConstraintLatinAlphabetOnly);
        if(!mFocusObject->editorInterface().filter()) {
            mFocusObject->editorInterface().setFilter(HbEmailAddressFilter::instance());
        }
        mFocusObject->setInputMethodHints(Qt::ImhEmailCharactersOnly | Qt::ImhNoPredictiveText 
            | Qt::ImhPreferLowercase);
    }
}
/*!
\internal
A convenience method for setting up the editor as url editor
*/
void HbInputMethodPrivate::setUpFocusedObjectAsUrlEditor()
{
    if(mFocusObject) {
        mFocusObject->editorInterface().setMode(HbInputModeNone);
        mFocusObject->editorInterface().setInputConstraints(HbEditorConstraintLatinAlphabetOnly);
        if(!mFocusObject->editorInterface().filter()) {
            mFocusObject->editorInterface().setFilter(HbUrlFilter::instance());
        }
        mFocusObject->setInputMethodHints(Qt::ImhUrlCharactersOnly | Qt::ImhNoPredictiveText 
            | Qt::ImhPreferLowercase);
    }
}

/// @endcond

// End of file


