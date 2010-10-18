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
#include "hbinputmethod.h"
#include "hbinputmethod_p.h"

#include <QInputMethodEvent>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QLocale>

#include "hbinputmodecache_p.h"
#include "hbinputsettingproxy.h"
#include "hbinputcontextproxy_p.h"
#include "hbinputfilter.h"
#include "hbinputmethodnull_p.h"
#include "hbinputpredictionfactory.h"
#include "hbinputextradictionaryfactory.h"
#include "hbinputstandardfilters.h"
#include "hbinpututils.h"
#include "hbinputvkbhost.h"
#include "hbinputvkbhostbridge.h"

/*!
@stable
@hbcore
\class HbInputMethod
\brief A base class for input method implementations.

HbInputMethod is the base class for input method implementations. It inherits from QInputContext,
connects to the input framework behind the scenes and resolves correct input state handler when
an editor widget is focused.

An internal framework class called HbInputModeCache scans through the system and looks for
available HbInputMethod instances. It then forms a list of input methods based on language
and keyboard type. Input method plugin reports (as plugin meta-data) which languages,
keyboards and input modes that plugin instance implements. Input mode cache then activates matching
HbInputMethod depending on the editor properties. It switches the active input method on the fly
when the focus switches between editors if the previous input method is unable to
support newly focused editor.

Custom input methods are a special class of input methods. Once a custom input method is
activated from UI, input mode cache stops resolving input methods upon focus operations
and the custom input is active in all editors until it is deactivated.

\bold The input framework program flow

1. An editor gains input focus.<BR>
2. Editor sets requestSoftwareInputPanel event to active input context.<BR>
3. The input framework creates input state based on editor attributes and the input mode cache <BR>
   resolves correct mode handler and activates it.<BR>
4. A virtual function HbInputMethod::focusReceived is called. At this point the input method<BR>
   initializes whatever it needs to initialize in order to start the input operation (for example,<BR>
   opens the virtual keyboard by using HbVkbHost API) and waits for user actions.<BR>
5. Text is written. The input method delivers results to the editor buffer by using HbInputFocusObject API.<BR>
   It can access editor attributes via HbEditorInterface API.<BR>
6. The active editor loses focus. At this point the input method receives a call to virtual function<BR>
   HbInputMethod::focusLost and is expected to conclude any ongoing input operations and shut down active<BR>
   UI elements (such as the virtual keyboard).<BR>
7. The input method waits for next focusReceived() call. <BR>

\bold Input method resolving

The input framework resolves correct input state handler based on three attributes: input mode,
keyboard type and input language.

The framework recognizes three different input modes: default, numeric and custom.
Default and numeric input modes are something that the framework resolves for active
input language and keyboard type. Custom input mode is something that the framework never
activates automatically, but it is activated from UI or directly from application
(or input method) code.

Numeric input mode is something that can handle those editors that are configured to only accept
numeric data (for example those that have Qt::ImhDigitsOnly hints on).

Language attribute is matched either as a direct match or as a language range. Language range
means the input method is capable of handling all the languages that
HbKeymapFactory::availableLanguages() returns. Direct match always wins so it is possible
to override language range for specific languages.

The keyboard attribute is always matched directly. Note that event though the constant
has term "keyboard" in it, it covers all input devices such as hand writing recognition widgets
that don't utilize traditional keys. it is up the the input method what that constant
really means.

The input method resolving parameters are part of input plugin meta data and they are returned
by QInputContextPlugin::languages() method. A single entry in the returned string list
is formed by packing resolving parameters into HbInputModeProperties structure and calling
HbInputModeProperties::asString() method.

Following code snippet shows how the input context plugin returns resolving parameters

\snippet{unittest_hbinputmethod/unittest_hbinputmethod.cpp,1}

\bold Input method resolving example

Say that we have implemented touch input methods for 12 key portrait mode and qwerty landscape mode.
Then we have Chinese touch input method for both portrait and landscape orientations and also
Chinese handwriting recognition input mode for portrait mode.

Touch input methods resolve to language range, which means that they will handle all
the other languages, except Chinese, which has its own designated input method.

Touch input methods also implement support for numeric mode. Because Chinese language uses
same numeric system as "latin" based languages, we only want to implement numeric mode
handling in one input method and arrange resolving parameters so that the numeric mode
is handled by default touch input method even when the global input language is Chinese.

Chinese handwriting input method is something that is a custom mode, and will be activated
from UI.

Following example shows how the resolving attributes are set up to achieve above configuration.

Portait touch input method returns following attributes

\snippet{unittest_hbinputmethod/unittest_hbinputmethod.cpp,2}

Landscape touch input method returns following attributes

\snippet{unittest_hbinputmethod/unittest_hbinputmethod.cpp,3}

Chinese portrait input method returns following attributes

\snippet{unittest_hbinputmethod/unittest_hbinputmethod.cpp,4}

Chinese landscape input method returns following attributes

\snippet{unittest_hbinputmethod/unittest_hbinputmethod.cpp,5}

Chinese handwriting recognition input method returns following attributes
(note use of HbInputModeCustom, in this example HWR is something that we
want to active from UI separately).

\snippet{unittest_hbinputmethod/unittest_hbinputmethod.cpp,6}

\sa QInputContext
\sa HbInputFocusObject
\sa HbEditorInterface
\sa HbVkbHost
*/

/*!
Constructs the object
*/
HbInputMethod::HbInputMethod() : d_ptr(new HbInputMethodPrivate(this))
{
    HbInputSettingProxy::instance()->connectObservingObject(this);
}

/*!
Destructs the object
*/
HbInputMethod::~HbInputMethod()
{
    HbInputSettingProxy::instance()->disconnectObservingObject(this);

    delete d_ptr;
}

/*!
Initializes the input framework.
*/
bool HbInputMethod::initializeFramework(QApplication &app)
{
    // Activate singleton shutdown.
    connect(&app, SIGNAL(aboutToQuit()), HbInputModeCache::instance(), SLOT(shutdown()));
    connect(&app, SIGNAL(aboutToQuit()), HbInputSettingProxy::instance(), SLOT(shutdown()));
    connect(&app, SIGNAL(aboutToQuit()), HbPredictionFactory::instance(), SLOT(shutDown()));
    connect(&app, SIGNAL(aboutToQuit()), HbExtraDictionaryFactory::instance(), SLOT(shutdown()));

    HbInputMethodNull *master = HbInputMethodNull::Instance();

    if (!master) {
        return false;
    }

    master->d_ptr->mIsActive = true;

    // Finally set application input context.
    QInputContext *proxy = master->d_ptr->proxy();
    // A check required so that Qt does not delete inputcontext
    // which we are passing.
    if (proxy != app.inputContext()) {
        app.setInputContext(proxy);
    }

    if (master->delayedPanelRequest()) {
        master->setDelayedPanelRequest(false);
        QInputContext* ic = qApp->inputContext();
        if (ic) {
            QEvent *openEvent = new QEvent(QEvent::RequestSoftwareInputPanel);
            ic->filterEvent(openEvent);
            delete openEvent;
        }
    }

    return true;
}

/*!
Returns the active instance of HbInputMethod. There is always active HbInputMethod instance after
InitializeFramework method has been called, even when there is no focused editor (in some cases it may
be so called null input method).
*/
HbInputMethod *HbInputMethod::activeInputMethod()
{
    // First try, try app input context directly. It is possible that it is an instance
    // of HbInputMethod that is installed directly there without framework knowing about it
    // (that shouldn't be done, but it is possible). That's why we rely on app input context as
    // a primary source instead of mode cache.
    QInputContext *context = qApp->inputContext();
    if (context && context->inherits("HbInputMethod")) {
        HbInputMethod *active = static_cast<HbInputMethod *>(context);
        return active;
    }

    // Then check if the 'null' is active.
    HbInputMethod *nullInstance = HbInputMethodNull::Instance();
    if (nullInstance && nullInstance->isActiveMethod()) {
        return nullInstance;
    }

    // No it wasn't, then go through the methods in the cache and see which one is
    // active.
    return HbInputModeCache::instance()->activeMethod();
}

/*!
Lists all custom input methods.
*/
QList<HbInputMethodDescriptor> HbInputMethod::listCustomInputMethods()
{
    return HbInputModeCache::instance()->listCustomInputMethods();
}

/*!
Lists custom input methods for given parameters.
*/
QList<HbInputMethodDescriptor> HbInputMethod::listCustomInputMethods(Qt::Orientation orientation, const HbInputLanguage &language)
{
    return HbInputModeCache::instance()->listCustomInputMethods(orientation, language);
}

/*!
Returns default input method for given orientation.
*/
HbInputMethodDescriptor HbInputMethod::defaultInputMethod(Qt::Orientation orientation)
{
    return HbInputModeCache::instance()->defaultInputMethod(orientation);
}

/*!
Activates given input method. Returns false if input method was not found
or the framework was not able to activate it.
*/
bool HbInputMethod::activateInputMethod(const HbInputMethodDescriptor &inputMethod)
{
    Q_D(HbInputMethod);

    if (!inputMethod.isEmpty()) {
        if (inputMethod.isDefault()) {
            d->setFocusCommon();
            return true;
        } else {
            HbInputMethod *customMethod = HbInputModeCache::instance()->loadInputMethod(inputMethod);
            if (customMethod && HbInputModeCache::instance()->acceptsState(customMethod, d->mInputState)) {
                d->contextSwitch(customMethod);
                return true;
            }
        }
    }

    return false;
}

/*!
This slot is called when the input language changes. The framework connects it
to the input setting proxy. When the signal is received, the input method implementation
is notified by calling inputLanguageChanged.

\sa inputLanguageChanged
\sa HbInputSettingProxy
*/
void HbInputMethod::globalInputLanguageChanged(const HbInputLanguage &newLanguage)
{
    Q_D(HbInputMethod);

    inputLanguageChanged(newLanguage);

    if (!isActiveMethod()) {
        // Notify non-active input methods of language change, but check
        // if the method has promised to handle the new language only
        // in the active method
        return;
    }

    // Just behave as if this was the first focus operation
    // to this editor.
    if (d->mFocusObject) {
        HbInputState state;
        editorRootState(state);
        activateState(state);
    }
}

/*!
This slot is called when the secondary input language changes. The framework connects it
to the input setting proxy. When the signal is received, the input method implementation
is notified by calling secondaryInputLanguageChanged.

\sa secondaryInputLanguageChanged
\sa HbInputSettingProxy
*/
void HbInputMethod::globalSecondaryInputLanguageChanged(const HbInputLanguage &newLanguage)
{
    secondaryInputLanguageChanged(newLanguage);
}

/*!
\deprecated HbInputMethod::activeKeyboardChanged(HbKeyboardType)
    is deprecated. No need to call this outside of framework.
*/
void HbInputMethod::activeKeyboardChanged(HbKeyboardType newKeyboard)
{
    Q_UNUSED(newKeyboard);
}

/*!
The framework calls this method when an input capable widget receives UI focus. This is empty
default implementation and the inheriting class should override it.

\sa focusLost
*/
void HbInputMethod::focusReceived()
{
    // Empty default implementation, do nothing.
}

/*!
The framework calls this method when an active editor loses focus. The parameter focusSwitch
is true if the focus is moving from one editor to another and false if the input focus
going to be lost completely.

\sa focusReceived
*/
void HbInputMethod::focusLost(bool focusSwitch)
{
    Q_UNUSED(focusSwitch);
    // Empty default implementation, do nothing.
}

/*!
Returns pointer to active focus object.
*/
HbInputFocusObject *HbInputMethod::focusObject() const
{
    Q_D(const HbInputMethod);
    return d->mFocusObject;
}

/*!
QWidget based editors notify focus changes through this method. This is the
default focus handling mechanism for QInputContext system. Input method
implementation should never override this method unless it knows what it is doing.

\sa setFocusObject
*/
void HbInputMethod::setFocusWidget(QWidget *widget)
{
    Q_D(HbInputMethod);

    if (d->mFocusLocked) {
        return;
    }

    if (!widget) {
        bool unfocus = true;

        if (d->mFocusObject) {
            // If the input focus is inside HbGraphicsScene then do not unfocus automatically.
            if (d->ignoreFrameworkFocusRelease(d->mFocusObject->object())) {
                unfocus = false;
            }
        }

        // Losing focus.
        if (d->mFocusObject && unfocus) {
            bool isNextFocusToEditor = false;
            //The Qt framework sets the focused widget twice as and when there is a change
            //of focus. First time it sets the focus to NULL and second time it sets the focus
            //to the actual widget. But, we need to know if there is a focus switch to an editor
            //And we do a focusLost(false) only if the current focused widget is NULL.
            QWidget *newFocusedWidget = QApplication::focusWidget();
            if (newFocusedWidget && newFocusedWidget->testAttribute(Qt::WA_InputMethodEnabled)) {
                if (newFocusedWidget->testAttribute(Qt::WA_WState_Created)
                    && newFocusedWidget->isEnabled()) {
                        isNextFocusToEditor = true;
                    }
            }
            // inform plugin about the focus shift.
            focusLost(isNextFocusToEditor);
            // hide mainwindow only incase qApp has focus on a non-editor field.
            if (!isNextFocusToEditor)
                d->hideMainWindow();

            delete d->mFocusObject;
            d->mFocusObject = 0;
            d->proxy()->QInputContext::setFocusWidget(0);
        }

        return;
    }
        
    // attach focuswidget to prxoy inputcontext as proxy is 
    // the only inputcotext known to qt framework.
    d->proxy()->QInputContext::setFocusWidget(widget);

    QGraphicsView *gView = qobject_cast<QGraphicsView *>(widget);
    if (gView) {
        // We don't want to focus to graphics view but the items inside the scene, so just return
        return;
    }

    // Check whether the editor has read-only constraint and reject focus
    // if so.
    bool readOnly = false;
    if (HbEditorInterface::isConnected(widget)) {
        HbEditorInterface eInt(widget);
        if (eInt.inputConstraints() & HbEditorConstraintIgnoreFocus) {
            readOnly = true;
        }
    }

    // Focusing widget doesn't have input capabilities
    // or it is read-only.
    if (readOnly && HbInputFocusObject::isReadOnlyWidget(widget)) {
        if (d->mFocusObject) {
            focusLost();
            d->hideMainWindow();
        }
        return;
    }

    if (d->mFocusObject) {
        if (d->mFocusObject->object() == widget) {
            // Focus remains in same widget, do nothing.
            return;
        } else {
            // Lose editor focus unless we are focusing back to the editor from e.g. a vkb
            focusLost();
        }
    }

    bool refreshHost = false;

    // Delete previous focus object.
    if (d->mFocusObject) {
        refreshHost = true;
        disconnect(d->mFocusObject->object(), SIGNAL(destroyed(QObject *)), this, SLOT(editorDeleted(QObject *)));
    }
    delete d->mFocusObject;
    d->mFocusObject = 0;

    // Attach focus.
    d->mFocusObject = d->createAndSetupFocusObject(widget);
    connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(editorDeleted(QObject *)));
    connect(d->mFocusObject, SIGNAL(aboutToChangeOrientation()), this, SLOT(_q_startOrientationSequence()));
    connect(d->mFocusObject, SIGNAL(orientationChanged()), this, SLOT(_q_endOrientationSequence()));

    d->setFocusCommon();

    // The focus jumped from one editor to another. Make sure that vkb host
    // updates the situation correctly.
    if (refreshHost && d->mFocusObject) {
        HbVkbHost *vkbHost = d->mFocusObject->editorInterface().vkbHost();
        if (vkbHost) {
            vkbHost->refresh();
        }
    }
}

/*!
Checks if the destroyed widget is currently focused and clears the focus
if needed. This method should not be overridden.
*/
void HbInputMethod::widgetDestroyed(QWidget *widget)
{
    Q_D(HbInputMethod);

    if (d->proxy()->focusWidget() == widget) {
        delete d->mFocusObject;
        d->mFocusObject = 0;
        // update QInputContext internal focusWidget pointer.
        d->proxy()->QInputContext::widgetDestroyed(widget);
        d->proxy()->QInputContext::setFocusWidget(0);
    }
}

/*!
Graphics item based editors (or any other object that implements
HbInputFocusObject) send their focus events notifications through this method.

Typically this method is called by HbInputContextProxy when it receives
requestSoftwareInputPanel event.

\sa setFocusWidget
\sa HbInputFocusObject
*/
void HbInputMethod::setFocusObject(HbInputFocusObject *focusObject)
{
    Q_D(HbInputMethod);

    if (d->mFocusLocked) {
        return;
    }

    if (focusObject == 0) {
        // Losing focus.
        if (d->mFocusObject != 0) {
            disconnect(d->mFocusObject->object(), SIGNAL(destroyed(QObject *)), this, SLOT(editorDeleted(QObject *)));
            focusLost(false);
            d->hideMainWindow();
            delete d->mFocusObject;
            d->mFocusObject = 0;
        }
        return;
    }

    if (d->compareWithCurrentFocusObject(focusObject)) {
        // The incoming focus object is either same or points to same
        // widget that the framework is already focused to and nothing needs to be done here.      
        return;
    }

    bool refreshHost = false;

    // Delete previous focus object.
    if (d->mFocusObject) {
        refreshHost = true;
        focusLost(true);
        disconnect(d->mFocusObject->object(), SIGNAL(destroyed(QObject *)), this, SLOT(editorDeleted(QObject *)));
        delete d->mFocusObject;
        d->mFocusObject = 0;
    }

    // Attach focus.
    d->mFocusObject = focusObject;
    connect(d->mFocusObject->object(), SIGNAL(destroyed(QObject *)), this, SLOT(editorDeleted(QObject *)));
    connect(d->mFocusObject, SIGNAL(aboutToChangeOrientation()), this, SLOT(_q_startOrientationSequence()));
    connect(d->mFocusObject, SIGNAL(orientationChanged()), this, SLOT(_q_endOrientationSequence()));

    d->setFocusCommon();

    // The focus jumped from one editor to another. Make sure that vkb host
    // updates the situation correctly.
    if (refreshHost && d->mFocusObject) {
        HbVkbHost *vkbHost = d->mFocusObject->editorInterface().vkbHost();
        if (vkbHost) {
            vkbHost->refresh();
        }
    }
}

/*!
The framework calls this method when device-wide input language changes.
The base class implementation is empty so any input method interested in
language switch events should implement it. Note that this method reports
change in system wide input language. Local input language in active editor
may still remain same.

\sa activeLanguage
*/
void HbInputMethod::inputLanguageChanged(const HbInputLanguage &newLanguage)
{
    Q_UNUSED(newLanguage);
    // Empty default implementation.
}

/*!
The framework calls this method when device-wide secondary input language changes.
The base class implementation is empty so any input method interested in language switch
events should override it in the actual inputmethod.
*/
void HbInputMethod::secondaryInputLanguageChanged(const HbInputLanguage &aNewLanguage)
{
    // Empty default implementation.
    Q_UNUSED(aNewLanguage);
}

/*!
Returns true if given input mode is allowed in active editor.
*/
bool HbInputMethod::modeAllowedInEditor(HbInputModeType mode) const
{
    Q_D(const HbInputMethod);
    return d->modeAllowedInEditor(mode);
}

/*!
Returns true if this instance is currently active QInputContext in QApplication.
*/
bool HbInputMethod::isActiveMethod() const
{
    Q_D(const HbInputMethod);
    return d->mIsActive;
}

/*!
The framework calls this method every time the input state changes. This is an empty default
implementation and the inheriting class should override it.
*/
void HbInputMethod::inputStateActivated(const HbInputState &newState)
{
    Q_UNUSED(newState);
    // Empty default implementation.
    if (this != HbInputMethodNull::Instance()) {
        qWarning("WARNING: inputStateActivated() default implementation called: Is that ok?");
    }
}

/*!
Returns active input state.

\sa activateState
\sa activateNextState
*/
HbInputState HbInputMethod::inputState() const
{
    Q_D(const HbInputMethod);
    return d->mInputState;
}

/*!
Returns the first input state that should be activated when an editor is
focused for the first time. The state is constructed from edirtor attributes and
input settings.
*/
void HbInputMethod::editorRootState(HbInputState &result) const
{
    Q_D(const HbInputMethod);
    d->editorRootState(result);
}

/*!

*/
void HbInputMethod::constructLatinState(HbInputState &result) const
{
    Q_D(const HbInputMethod);
    d->constructLatinState(result);
}

/*!
Activates given input state. State handler must be among cached input methods, this
method doesn't resolve it from non-cached plugins. Returns true if the framework was
able to find handler for the new state. A context switch to another input method
instance may occur.

\sa activateNextState
\sa InputState
*/
bool HbInputMethod::activateState(const HbInputState &state)
{
    Q_D(HbInputMethod);

    if (!d->stateAllowedInEditor(state)) {
        return false;
    }

    d->mStateChangeInProgress = true;

    HbInputMethod *stateHandler = HbInputModeCache::instance()->findStateHandler(state);

    if (!stateHandler) {
        stateHandler = HbInputMethodNull::Instance();
    }

    d->mInputState = state;

    if (stateHandler != this) {
        stateHandler->d_ptr->mStateChangeInProgress = true;
        // Context switch needed.
        d->inputStateToEditor(d->mInputState);
        d->contextSwitch(stateHandler);
        stateHandler->d_ptr->mStateChangeInProgress = false;
    } else {
        // Same method handles new state, just report the state change.
        d->inputStateToEditor(d->mInputState);
        inputStateActivated(d->mInputState);
    }

    d->mStateChangeInProgress = false;

    return true;
}

/*!
Updates the input state to automatic or lower case, with a check on editor contraints.
*/
void HbInputMethod::updateState()
{
    Q_D(HbInputMethod);

    if (!d->textCaseApplies()) {
        return;
    }

    bool autoCaseNeeded = d->automaticTextCaseNeeded();
    HbTextCase currentTextCase = d->mInputState.textCase();

    bool refresh = false;

    if (currentTextCase == HbTextCaseAutomatic) {
        if (!autoCaseNeeded) {
            currentTextCase = HbTextCaseLower;
            refresh = true;
        }
    } else if (autoCaseNeeded && currentTextCase != HbTextCaseUpper) {
        if (!d->isFixedCaseEditor()) {
            currentTextCase = HbTextCaseAutomatic;
            refresh = true;
        } else {
            return;
        }
    }

    if (refresh) {
        d->mInputState.setTextCase(currentTextCase);
        d->inputStateToEditor(d->mInputState);
        inputStateActivated(d->mInputState);
    }
}

/*!
\deprecated HbInputMethod::orientationChanged(Qt::Orientation)
    is deprecated. No need to call this from outside of the framework.
*/
void HbInputMethod::orientationChanged(Qt::Orientation orientation)
{
    Q_UNUSED(orientation);
}

/*!
\deprecated HbInputMethod::orientationAboutToChange()
    is deprecated. No need to call this from outside of the framework.
*/
void HbInputMethod::orientationAboutToChange()
{
}

/*!
Returns active input language. Unlike setting proxy's global input language,
this method takes into account input state language and possible editor local language,
so the return value reflects real situation in currently active editor instead of
the global setting. If input state defines language, then that is used. Otherwise
editor's local input language is checked first and if that is not specified, then
global input language is returned.
*/
HbInputLanguage HbInputMethod::activeLanguage() const
{
    Q_D(const HbInputMethod);
    return d->activeLanguage();
}

/*!
Once this method is called, the framework will ignore all the incoming focus events completely
until focus is unlocked. Context switch operation will unlock it
automatically. Usually this feature is not needed, but it may come handy in some cases
where an input method implementation knows that is is going to do something on the UI-level
that will steal the focus, but wants to ensure that the framework keeps its
focus in the original editor during that time.

\sa unlockFocus
*/
void HbInputMethod::lockFocus()
{
    Q_D(HbInputMethod);
    d->mFocusLocked = true;
}

/*!
Unlocks the focus. After calling this method the framework will start to receive
focus events again, in case they where locked out before.

\sa lockFocus
*/
void HbInputMethod::unlockFocus()
{
    Q_D(HbInputMethod);
    d->mFocusLocked = false;
}

/*!
Returns true if focus is locked. If focus is locked, input framework will ignore 
all the incoming focus events completely until focus is unlocked.

\sa lockFocus
\sa unlockFocus
*/
bool HbInputMethod::isFocusLocked()
{
    Q_D(HbInputMethod);
    return d->mFocusLocked;
}

/*!
Removes input method focus and asks active input plugin to close its active UI-components
(such as touch keypads). This may be needed in some special cases where the underlying
application wants to make sure that there are no input related elements on the screen.

This is a if-all-else fails backup method. Same can be done by doing
following.

\code
QInputContext* ic = qApp->inputContext();
if (ic) {
    QEvent *closeEvent = new QEvent(QEvent::CloseSoftwareInputPanel);
    ic->filterEvent(closeEvent);
    delete closeEvent;
}
\endcode
*/
void HbInputMethod::forceUnfocus()
{
    HbInputMethod *active = activeInputMethod();

    if (active) {
        active->focusLost(false);
        delete active->d_ptr->mFocusObject;
        active->d_ptr->mFocusObject = 0;
    }
}

/*!
Returns true if automatic condititions for automatic upper case are met at
current cursor position. Returns false if there isn't active editor.
*/
bool HbInputMethod::automaticTextCaseNeeded() const
{
    Q_D(const HbInputMethod);
    return d->automaticTextCaseNeeded();
}

/*!
Deep copies the input state back to editor interface.
*/
void HbInputMethod::inputStateToEditor(const HbInputState &source)
{
    Q_D(HbInputMethod);
    d->inputStateToEditor(source);
}

/*!
Returns true if state change operation is in progress. This is useful in those
cases where focus operations should behave differently during state change
than in normal focus-in / focus-out operations. For example, we may not want
to run virtual keyboard animations during state change but just switch to another
vkb immediately.
*/
bool HbInputMethod::stateChangeInProgress() const
{
    Q_D(const HbInputMethod);
    return d->mStateChangeInProgress;
}

/*!
When an editor becomes focused, the framework connects its destroyed signal
to this slot and it takes care of closing the input and resetting the input method.
*/
void HbInputMethod::editorDeleted(QObject *obj)
{
    Q_D(HbInputMethod);
    Q_UNUSED(obj);

    focusLost(false);
    d->hideMainWindow();
    delete d->mFocusObject;
    d->mFocusObject = 0;
    d->mFocusLocked = false;

    reset();
}

/*!
Returns the input method descriptor the framework used for loading this plugin.
Returns empty descriptor if the framework doesn't recognize this input method
(ie. it was not resolved by input mode cache).
*/
HbInputMethodDescriptor HbInputMethod::descriptor() const
{
    return HbInputModeCache::instance()->descriptor(this);
}

#include "moc_hbinputmethod.cpp"

// End of file

