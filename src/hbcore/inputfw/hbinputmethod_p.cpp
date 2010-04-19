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
Destructs the object.
*/
HbInputMethodPrivate::~HbInputMethodPrivate()
{
    delete mFocusObject;
}

/*!
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
Returns constraint flags of the focused editor.
*/
int HbInputMethodPrivate::editorConstraints() const
{
    if (mFocusObject) {
        return mFocusObject->editorInterface().constraints();
    }

    return 0;
}

/*!
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
    } else {
        result = HbInputState();
    }
}

/*!
Transfers local copy of the input state back to the editor using editor interface.
*/
void HbInputMethodPrivate::inputStateToEditor(const HbInputState& source)
{
    if (mFocusObject) {
        mFocusObject->editorInterface().setLastFocusedState(source);
        mFocusObject->editorInterface().setInputMode(source.inputMode());
        mFocusObject->editorInterface().setTextCase(source.textCase());
    }
}

/*!
Returns input language for current input state. Ususally it is same as the global input language but
in case of latin only -editors it may be english if the global input language isn't valid for latin only -editors.
*/
HbInputLanguage HbInputMethodPrivate::activeLanguage() const
{
    HbInputLanguage ret = mInputState.language();

    if (ret.undefined()) {
        ret = HbInputSettingProxy::instance()->globalInputLanguage();
        if (mFocusObject) {
            if ((mFocusObject->editorInterface().constraints() & HbEditorConstraintLatinAlphabetOnly) &&
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
        if (!mFocusObject || mFocusObject->editorInterface().inputMode() != mode) {
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
Goes through the given list of input modes, generates input state for each mode and finds
out if there is state handler for that state. If a handler is found, returns it and 
fills 'state'parameter with values from generated state.
*/
HbInputMethod* HbInputMethodPrivate::findInitialStateHandler(const QVector<HbInputModeProperties>& modes,
                                                             HbInputState& state)
{
    state = HbInputState();
    HbInputMethod* master = 0;

    HbInputState inState;

    foreach (HbInputModeProperties mode, modes) {
        if (modeAllowedInEditor(mode.iMode)) {
            stateFromMode(mode, inState);
            master = HbInputModeCache::instance()->findStateHandler(inState);
            if (master) {
                state = inState;
                break;
            }
        }
    }

    return master;
} 

/*!
Creates input state from input mode.
*/
void HbInputMethodPrivate::stateFromMode(const HbInputModeProperties& mode, HbInputState& state)
{
    state.setKeyboard(activeKeyboard());
    state.setInputMode(mode.iMode);
    state.setLanguage(mode.iLanguage);

    if (state.language().isCaseSensitiveLanguage() && HbInputUtils::isCaseSensitiveMode(mode.iMode)) {
        if (automaticTextCaseNeeded()) {
            state.setTextCase(HbTextCaseAutomatic);
        } else {
            state.setTextCase(HbTextCaseLower);
        }
    } else {
        state.setTextCase(HbTextCaseNone);
    }
}

/*!
Finds state handler for given input state.
*/
HbInputMethod* HbInputMethodPrivate::findStateHandler(HbInputState& state)
{  
    if (stateAllowedInEditor(state)) {
        HbInputMethod* stateHandler = HbInputModeCache::instance()->findStateHandler(state);
        if (stateHandler) {
            return stateHandler;
        }
    }

    return 0;
}

/*!
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
    
    return HbInputLanguage(lang);
}

/*!
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
Takes care of the part of focus in operation that is common to both QWidgets and QGraphicsWidgets.
*/
void HbInputMethodPrivate::setFocusCommon()
{
    Q_Q(HbInputMethod);

    if (mFocusObject) {
        if (mFocusObject->editorInterface().filter() == 0) {
            // If input method hints suggest certain input method filter but none is set,
            // provide suitable one with compliments.
            Qt::InputMethodHints hints = mFocusObject->inputMethodHints();
            if (hints & Qt::ImhDialableCharactersOnly) {
                mFocusObject->editorInterface().setFilter(HbPhoneNumberFilter::instance());
            } else if (hints & Qt::ImhFormattedNumbersOnly) {
                mFocusObject->editorInterface().setFilter(HbFormattedNumbersFilter::instance());
            } else if (hints & Qt::ImhDigitsOnly) {
                mFocusObject->editorInterface().setFilter(HbDigitsOnlyFilter::instance());
            } else if (hints & Qt::ImhUrlCharactersOnly) {
                mFocusObject->editorInterface().setFilter(HbUrlFilter::instance());
            } else if (hints & Qt::ImhEmailCharactersOnly) {
                mFocusObject->editorInterface().setFilter(HbEmailAddressFilter::instance());
            } 
        }
    }

    // Create input state.
    if (mTrustLocalState) {
        // This focus operation is a direct result from UI operation that modified
        // input state (and probably caused context switch) while focus was away.
        // Therefore it was not possible to store new state to editor interface,
        // but instead input method was requested to trust the local state instead
        // of reading it from the editor. We assume that whatever part of code set this flag
        // knows what it is doing.
        mTrustLocalState = false;
    } else {
        inputStateFromEditor(mInputState);
    }
   
    // Find state handler
    HbInputMethod* stateHandler = 0;
    HbInputMethodDescriptor activeMethod = HbInputSettingProxy::instance()->activeCustomInputMethod();
    if (!activeMethod.isEmpty() && !activeMethod.isDefault()) {
        // A custom method is active. Don't resolve, just try to load it.
        stateHandler = HbInputModeCache::instance()->loadInputMethod(activeMethod);
    }

    if (!stateHandler) {
        // It either wasn't a custom method or we were not able to load the custom method.
        // Resolve normally.
         stateHandler = findStateHandler(mInputState);
    }

    if (editorConstraints() & HbEditorConstraintIgnoreFocus) {
        // The editor requests us to ignore the focus.
        stateHandler = 0;
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
Refreshes input state. Stores local edit state back to the editor and calls child class' inputMethodActived method.
*/
void HbInputMethodPrivate::refreshState()
{
    Q_Q(HbInputMethod);

    inputStateToEditor(mInputState);
    q->inputStateActivated(mInputState);
}

/*!
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
Creates and returns new input context proxy.
*/
QInputContext* HbInputMethodPrivate::newProxy()
{
    return new HbInputContextProxy(q_ptr);
}

/*!
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
Cosntructs the first input state for an editor that hasn't been focused before.
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
Reutunrs initial text case for an editor that hasn't been focused before.
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
Finds the first input mode for an editor that hasn't been focus before.
*/
HbInputModeType HbInputMethodPrivate::initialInputMode(const HbInputLanguage &language) const
{ 
    HbInputModeType ret = HbInputModeNone;

    if (mFocusObject) {        
        if (mFocusObject->editorInterface().constraints() & HbEditorConstraintFixedInputMode) {
            // This is fixed mode editor, always trust what editor interface gives us.
            ret = (HbInputModeType)mFocusObject->editorInterface().inputMode();
        } else {
            // Editor doesn't have mode asigned. Propose default mode.                  
            Qt::InputMethodHints hints = mFocusObject->inputMethodHints();
            if (mFocusObject->editorInterface().isNumericEditor() || (hints & Qt::ImhPreferNumbers)) {
                // It is either fixed numeric or prefers numeric.
                ret = HbInputModeNumeric;
            } else {
                ret = defaultInputMode(language);        
            }
        }
    }

    return ret;
}

/*!
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

/// @endcond

// End of file


